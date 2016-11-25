/*
 * at24c02c.c - handle at24c02c I2C EEPROM
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <linux/stddef.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include "../../../arch/arm/mach-tegra/gpio-names.h"

#define WRITE_PROTECT_GPIO    TEGRA_GPIO_PQ4

struct at24_data {
	struct at24_platform_data chip;
	struct mutex lock;
	u8 *writebuf;
	unsigned write_max;
	unsigned num_addresses;
	struct i2c_client *client[];
};

struct at24_data *g_at24;

static unsigned io_limit = 128;
module_param(io_limit, uint, 0);
MODULE_PARM_DESC(io_limit, "Maximum bytes per I/O (default 128)");

static unsigned write_timeout = 25;
module_param(write_timeout, uint, 0);
MODULE_PARM_DESC(write_timeout, "Time (in ms) to try writes (default 25)");

#define AT24_SIZE_BYTELEN 5
#define AT24_SIZE_FLAGS 8
#define AT24_BITMASK(x) (BIT(x) - 1)

static const struct i2c_device_id at24_ids[] = {
	{ "at24", 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(i2c, at24_ids);

struct i2c_client *i2c_client;
static bool is_eeprom_ready = false;

static struct i2c_client *at24_translate_offset(struct at24_data *at24,
		unsigned *offset)
{
	unsigned i;

	if (at24->chip.flags & AT24_FLAG_ADDR16) {
		i = *offset >> 16;
		*offset &= 0xffff;
	} else {
		i = *offset >> 8;
		*offset &= 0xff;
	}

	return at24->client[i];
}

static ssize_t at24_eeprom_read(struct at24_data *at24, char *buf,
		unsigned offset, size_t count)
{
	struct i2c_msg msg[2];
	u8 msgbuf[2];
	struct i2c_client *client;
	unsigned long timeout, read_time;
	int status, i;

	memset(msg, 0, sizeof(msg));

	client = at24_translate_offset(at24, &offset);

	if (count > io_limit)
		count = io_limit;

	i = 0;
	if (at24->chip.flags & AT24_FLAG_ADDR16)
		msgbuf[i++] = offset >> 8;

	msgbuf[i++] = offset;

	msg[0].addr = client->addr;
	msg[0].buf = msgbuf;
	msg[0].len = i;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = count;

	timeout = jiffies + msecs_to_jiffies(write_timeout);
	do {
		read_time = jiffies;

		status = i2c_transfer(client->adapter, msg, 2);
		if (status == 2)
			status = count;

		dev_dbg(&client->dev, "read %zu@%d --> %d (%ld)\n",
				count, offset, status, jiffies);

		if (status == count)
			return count;

		/* REVISIT: at HZ=100, this is sloooow */
		msleep(1);
	} while (time_before(read_time, timeout));

	return -ETIMEDOUT;
}

static ssize_t at24_read(struct at24_data *at24,
		char *buf, loff_t off, size_t count)
{
	ssize_t retval = 0;

	if (unlikely(!count))
		return count;

	mutex_lock(&at24->lock);

	while (count) {
		ssize_t	status;

		status = at24_eeprom_read(at24, buf, off, count);
		if (status <= 0) {
			if (retval == 0)
				retval = status;
			break;
		}
		buf += status;
		off += status;
		count -= status;
		retval += status;
	}

	mutex_unlock(&at24->lock);

	return retval;
}

static ssize_t at24_eeprom_write(struct at24_data *at24, const char *buf,
		unsigned offset, size_t count)
{
	struct i2c_client *client;
	struct i2c_msg msg;
	ssize_t status;
	unsigned long timeout, write_time;
	unsigned next_page;
	int i = 0;

	/* Get corresponding I2C address and adjust offset */
	client = at24_translate_offset(at24, &offset);

	/* write_max is at most a page */
	if (count > at24->write_max)
		count = at24->write_max;

	/* Never roll over backwards, to the start of this page */
	next_page = roundup(offset + 1, at24->chip.page_size);
	if (offset + count > next_page)
		count = next_page - offset;

	/* If we'll use I2C calls for I/O, set up the message */

	msg.addr = client->addr;
	msg.flags = 0;

	/* msg.buf is u8 and casts will mask the values */
	msg.buf = at24->writebuf;
	if (at24->chip.flags & AT24_FLAG_ADDR16)
		msg.buf[i++] = offset >> 8;

	msg.buf[i++] = offset;
	memcpy(&msg.buf[i], buf, count);
	msg.len = i + count;

	timeout = jiffies + msecs_to_jiffies(write_timeout);
	do {
		write_time = jiffies;

		status = i2c_transfer(client->adapter, &msg, 1);
		if (status == 1)
			status = count;

		dev_dbg(&client->dev, "write %zu@%d --> %zd (%ld)\n",
				count, offset, status, jiffies);

		if (status == count)
			return count;

		/* REVISIT: at HZ=100, this is sloooow */
		msleep(1);
	} while (time_before(write_time, timeout));

	return -ETIMEDOUT;
}

static ssize_t at24_write(struct at24_data *at24, const char *buf, int off,
			  size_t count)
{
	ssize_t retval = 0;

	if (unlikely(!count))
		return count;

	mutex_lock(&at24->lock);

	while (count) {
		ssize_t	status;

		status = at24_eeprom_write(at24, buf, off, count);
		msleep(1);

		if (status <= 0) {
			if (retval == 0)
				retval = status;
			break;
		}
		buf += status;
		off += status;
		count -= status;
		retval += status;
	}

	mutex_unlock(&at24->lock);

	return retval;
}

int SetWriteProtect(int i)
{
	if(i)
	{
		gpio_set_value(WRITE_PROTECT_GPIO, 0);
		udelay(5);
	} else {
		gpio_set_value(WRITE_PROTECT_GPIO, 1);
		udelay(5);
	}

	return 0;
}

static int Read_Data(char *buf, int addr, int length)
{
	ssize_t retval = 0;

	if (addr + length >= 256)
		return -1;

	retval = at24_read(g_at24, buf, (loff_t) addr, (size_t) length);

	return length;
}

static int Read_Data_String(char *buf, int addr, int length)
{
	int retval = 0;
	int i = 0;
	char *buf_hex;
	char *tmp;
	char * s = buf;

	buf_hex = kzalloc(length, GFP_KERNEL);
	tmp = kzalloc((2*length), GFP_KERNEL);

	retval = Read_Data(buf_hex, addr, length);

	for(i = 0; i < length ; i++)
	{
		tmp[i*2] = (buf_hex[i] & 0xF0) >> 4;
		tmp[i*2+1] = buf_hex[i] & 0x0F;
	}


	for(i=0;i<length*2;i++)
		s += sprintf(s, "%x",tmp[i]);

	kfree(buf_hex);
	kfree(tmp);

	return retval*2;
}

static int Write_Data(const char *buf, int addr, int length)
{
	ssize_t retval = 0;

	if (addr + length >= 256)
		return -1;

	SetWriteProtect(1);
	retval = at24_write(g_at24, buf, addr, (size_t) length);
	SetWriteProtect(0);

	return length;
}

int Get_UUID(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_UUID, LENGTH_UUID);
	return 0;
}

int Get_BT_MAC(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_BT_MAC, LENGTH_BT_MAC);
	return 0;
}

int Get_Serial_Number(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_SN, LENGTH_SN);
	return 0;
}

int Get_Board_ID(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_BOARD_ID, LENGTH_BOARD_ID);
	return 0;
}

int Get_Product_SKU(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_PRODUCT_SKU, LENGTH_PRODUCT_SKU);
	return 0;
}

int Get_SerialNumberwithoutBarcode(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_SNWB, LENGTH_SNWB);
	return 0;
}

int Get_IMEIwithBarcode(char * buf)
{
	int retval = 0;
	char * tmp;

	if(is_eeprom_ready)
	{
		tmp = kzalloc(LENGTH_IMEI*2, GFP_KERNEL);
		retval = Read_Data_String(tmp, ADDR_IMEI, LENGTH_IMEI);
		retval--;

		memcpy(buf, tmp, retval);
		memset(tmp, 'f', retval);

		if (!strncmp(buf, tmp, retval))
			memset(buf, '0', retval);

		kfree(tmp);
		return retval;
	}
	return 0;
}

int Get_Manufacture_Date(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_MANUFACTURE_DATE, LENGTH_MANUFACTURE_DATE);
	return 0;
}

int Get_G_Sensor(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf,ADDR_G_SENSOR, LENGTH_G_SENSOR);
	return 0;
}

int Get_Light_Sensor(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf,ADDR_L_SENSOR, LENGTH_L_SENSOR);
	return 0;
}

int Get_WV_Device_ID(char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_WV_Device_ID, LENGTH_WV_Device_ID);
	return 0;
}

int Set_UUID(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_UUID, LENGTH_UUID);
	return 0;
}

int Set_BT_MAC(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_BT_MAC, LENGTH_BT_MAC);
	return 0;
}

int Set_Serial_Number(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_SN, LENGTH_SN);
	return 0;
}

int Set_Board_ID(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_BOARD_ID, LENGTH_BOARD_ID);
	return 0;
}

int Set_Product_SKU(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_PRODUCT_SKU, LENGTH_PRODUCT_SKU);
	return 0;
}

int Set_SerialNumberwithoutBarcode(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_SNWB, LENGTH_SNWB);
	return 0;
}

int Set_IMEIwithBarcode(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_IMEI, LENGTH_IMEI);
	return 0;
}

int Set_Manufacture_Date(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_MANUFACTURE_DATE, LENGTH_MANUFACTURE_DATE);
	return 0;
}

int Set_G_Sensor(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_G_SENSOR, LENGTH_G_SENSOR);
	return 0;
}

int Set_Light_Sensor(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_L_SENSOR, LENGTH_L_SENSOR);
	return 0;
}

int Set_WV_Device_ID(char * buf)
{
	if(is_eeprom_ready)
		return Write_Data(buf, ADDR_WV_Device_ID, LENGTH_WV_Device_ID);
	return 0;
}

EXPORT_SYMBOL(Get_UUID);
EXPORT_SYMBOL(Get_BT_MAC);
EXPORT_SYMBOL(Get_Serial_Number);
EXPORT_SYMBOL(Get_Board_ID);
EXPORT_SYMBOL(Get_Product_SKU);
EXPORT_SYMBOL(Get_SerialNumberwithoutBarcode);
EXPORT_SYMBOL(Get_IMEIwithBarcode);
EXPORT_SYMBOL(Get_Manufacture_Date);
EXPORT_SYMBOL(Get_G_Sensor);
EXPORT_SYMBOL(Get_Light_Sensor);
EXPORT_SYMBOL(Get_WV_Device_ID);

EXPORT_SYMBOL(Set_UUID);
EXPORT_SYMBOL(Set_BT_MAC);
EXPORT_SYMBOL(Set_Serial_Number);
EXPORT_SYMBOL(Set_Board_ID);
EXPORT_SYMBOL(Set_Product_SKU);
EXPORT_SYMBOL(Set_SerialNumberwithoutBarcode);
EXPORT_SYMBOL(Set_IMEIwithBarcode);
EXPORT_SYMBOL(Set_Manufacture_Date);
EXPORT_SYMBOL(Set_G_Sensor);
EXPORT_SYMBOL(Set_Light_Sensor);
EXPORT_SYMBOL(Set_WV_Device_ID);

static ssize_t UUID_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_UUID, LENGTH_UUID);
	return 0;
}

static ssize_t UUID_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t BTMAC_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_BT_MAC, LENGTH_BT_MAC);
	return 0;
}

static ssize_t BTMAC_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t SerialNumber_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_SN, LENGTH_SN);
	return 0;
}

static ssize_t SerialNumber_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t BoardID_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_BOARD_ID, LENGTH_BOARD_ID);
	return 0;
}

static ssize_t BoardID_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t ProductSKU_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_PRODUCT_SKU, LENGTH_PRODUCT_SKU);
	return 0;
}

static ssize_t ProductSKU_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t SerialNumberwithoutBarcode_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_SNWB, LENGTH_SNWB);
	return 0;
}

static ssize_t SerialNumberwithoutBarcode_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return Write_Data(buf, ADDR_SNWB, LENGTH_SNWB);
}

static ssize_t IMEIwithBarcode_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	int retval = 0;
	char * tmp;

	if(is_eeprom_ready)
	{
		tmp = kzalloc(LENGTH_IMEI*2, GFP_KERNEL);
		retval = Read_Data_String(tmp, ADDR_IMEI, LENGTH_IMEI);
		retval--;

		memcpy(buf, tmp, retval);
		memset(tmp, 'f', retval);

		if (!strncmp(buf, tmp, retval))
			memset(buf, '0', retval);

		kfree(tmp);
		return retval;
	}
	return 0;
}

static ssize_t IMEIwithBarcode_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t ManufactureDate_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data_String(buf, ADDR_MANUFACTURE_DATE, LENGTH_MANUFACTURE_DATE);
	return 0;
}

static ssize_t ManufactureDate_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return Write_Data(buf, ADDR_MANUFACTURE_DATE, LENGTH_MANUFACTURE_DATE);
}

static ssize_t GSensor_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_G_SENSOR, LENGTH_G_SENSOR);
	return 0;
}

static ssize_t GSensor_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t LightSensor_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_L_SENSOR, LENGTH_L_SENSOR);
	return 0;
}

static ssize_t LightSensor_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static ssize_t WVDeviceID_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	if(is_eeprom_ready)
		return Read_Data(buf, ADDR_WV_Device_ID, LENGTH_WV_Device_ID);
	return 0;
}

static ssize_t WVDeviceID_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}

static int at24_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int at24_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long
at24_ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
	void __user *Kbuf = (void __user *)buf;

	char bufa[76];

	switch (cmd) {
		case AT24_IOCTL_WRITE_UUID:
			if (copy_from_user(&bufa, Kbuf, LENGTH_UUID))
				return -EFAULT;
			Write_Data(bufa, ADDR_UUID, LENGTH_UUID);
			break;

		case AT24_IOCTL_WRITE_BT_MAC:
			if (copy_from_user(&bufa, Kbuf, LENGTH_BT_MAC)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_BT_MAC, LENGTH_BT_MAC);
			break;

		case AT24_IOCTL_WRITE_SN:
			if (copy_from_user(&bufa, Kbuf, LENGTH_SN)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_SN, LENGTH_SN);
			break;

		case AT24_IOCTL_WRITE_SNWB:
			if (copy_from_user(&bufa, Kbuf, LENGTH_SNWB)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_SNWB, LENGTH_SNWB);
			break;

		case AT24_IOCTL_WRITE_MANUFACTURE_DATE:
			if (copy_from_user(&bufa, Kbuf, LENGTH_MANUFACTURE_DATE)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_MANUFACTURE_DATE, LENGTH_MANUFACTURE_DATE);
			break;

		case AT24_IOCTL_WRITE_IMEI:
			if (copy_from_user(&bufa, Kbuf, LENGTH_IMEI)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_IMEI, LENGTH_IMEI);
			break;

		case AT24_IOCTL_WRITE_BOARD_ID:
			if (copy_from_user(&bufa, Kbuf, LENGTH_BOARD_ID)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_BOARD_ID, LENGTH_BOARD_ID);
			break;

		case AT24_IOCTL_WRITE_PRODUCT_SKU:
			if (copy_from_user(&bufa, Kbuf, LENGTH_PRODUCT_SKU)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_PRODUCT_SKU, LENGTH_PRODUCT_SKU);
			break;

		case AT24_IOCTL_WRITE_L_SENSOR:
			if (copy_from_user(&bufa, Kbuf, LENGTH_L_SENSOR)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_L_SENSOR, LENGTH_L_SENSOR);
			break;

		case AT24_IOCTL_WRITE_G_SENSOR:
			if (copy_from_user(&bufa, Kbuf, LENGTH_G_SENSOR)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_G_SENSOR, LENGTH_G_SENSOR);
			break;

		case AT24_IOCTL_WRITE_WV_Device_ID:
			if (copy_from_user(&bufa, Kbuf, LENGTH_WV_Device_ID)) {
				return -EFAULT;
			}
			Write_Data(bufa, ADDR_WV_Device_ID, LENGTH_WV_Device_ID);
			break;

		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations at24_fops = {
	.owner = THIS_MODULE,
	.open = at24_open,
	.release = at24_release,
	.unlocked_ioctl = at24_ioctl,
};

static struct miscdevice at24_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "at24_dev",
	.fops = &at24_fops,
};

static struct kobject *eeprom_debug_kobj;
static struct kobject *eeprom_dev_info_kobj;

#define debug_attr(_name, _mode) \
	static struct kobj_attribute _name##_attr = { \
	.attr = { \
	.name = __stringify(_name), \
	.mode = _mode, \
	}, \
	.show = _name##_show, \
	.store = _name##_store, \
	}

debug_attr(UUID, 0644);
debug_attr(BTMAC, 0644);
debug_attr(SerialNumber, 0644);
debug_attr(BoardID, 0644);
debug_attr(ProductSKU, 0644);
debug_attr(SerialNumberwithoutBarcode, 0664);
debug_attr(IMEIwithBarcode, 0644);
debug_attr(ManufactureDate, 0664);
debug_attr(GSensor, 0644);
debug_attr(LightSensor, 0644);
debug_attr(WVDeviceID, 0644);

static struct attribute * group[] = {
	&UUID_attr.attr,
	&BTMAC_attr.attr,
	&SerialNumber_attr.attr,
	&BoardID_attr.attr,
	&ProductSKU_attr.attr,
	&SerialNumberwithoutBarcode_attr.attr,
	&IMEIwithBarcode_attr.attr,
	&ManufactureDate_attr.attr,
	&GSensor_attr.attr,
	&LightSensor_attr.attr,
	&WVDeviceID_attr.attr,
	NULL,
};

static struct attribute_group attr_group =
{
	.attrs = group,
};

static int at24_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct at24_platform_data chip;
	bool writable;
	struct at24_data *at24;
	int err;
	unsigned i, num_addresses;
	kernel_ulong_t magic;

	if (client->dev.platform_data) {
		chip = *(struct at24_platform_data *)client->dev.platform_data;
	} else {
		if (!id->driver_data) {
			err = -ENODEV;
			goto err_out;
		}
		magic = id->driver_data;
		chip.byte_len = BIT(magic & AT24_BITMASK(AT24_SIZE_BYTELEN));
		magic >>= AT24_SIZE_BYTELEN;
		chip.flags = magic & AT24_BITMASK(AT24_SIZE_FLAGS);
		/*
		 * This is slow, but we can't know all eeproms, so we better
		 * play safe. Specifying custom eeprom-types via platform_data
		 * is recommended anyhow.
		 */
		chip.page_size = 1;
		chip.setup = NULL;
		chip.context = NULL;
	}

	if (!is_power_of_2(chip.byte_len)) {
		dev_warn(&client->dev,
			"byte_len looks suspicious (no power of 2)!\n");
	}
	if (!is_power_of_2(chip.page_size)) {
		dev_warn(&client->dev,
			"page_size looks suspicious (no power of 2)!\n");
	}

	if (chip.flags & AT24_FLAG_TAKE8ADDR)
		num_addresses = 8;
	else
		num_addresses =	DIV_ROUND_UP(chip.byte_len,
			(chip.flags & AT24_FLAG_ADDR16) ? 65536 : 256);

	at24 = kzalloc(sizeof(struct at24_data) +
		num_addresses * sizeof(struct i2c_client *), GFP_KERNEL);

	if (!at24) {
		err = -ENOMEM;
		goto err_out;
	}

	mutex_init(&at24->lock);
	at24->chip = chip;
	at24->num_addresses = num_addresses;

	/*
	 * Export the EEPROM bytes through sysfs, since that's convenient.
	 * By default, only root should see the data (maybe passwords etc)
	 */

	writable = !(chip.flags & AT24_FLAG_READONLY);
	if (writable) {
		unsigned write_max = chip.page_size;
		if (write_max > io_limit)
			write_max = io_limit;
		at24->write_max = write_max;
		/* buffer (data + address at the beginning) */
		at24->writebuf = kmalloc(write_max + 2, GFP_KERNEL);
		if (!at24->writebuf) {
			err = -ENOMEM;
			goto err_struct;
		}
	}
	at24->client[0] = client;
	i2c_client = client;

	/* use dummy devices for multiple-address chips */
	for (i = 1; i < num_addresses; i++) {
		at24->client[i] = i2c_new_dummy(client->adapter,
					client->addr + i);
		if (!at24->client[i]) {
			dev_err(&client->dev, "address 0x%02x unavailable\n",
					client->addr + i);
			err = -EADDRINUSE;
			goto err_clients;
		}
	}
	i2c_set_clientdata(client, at24);

	dev_dbg(&client->dev,
		"page_size %d, num_addresses %d, write_max %d\n",
		chip.page_size, num_addresses,
		at24->write_max);

	gpio_request(WRITE_PROTECT_GPIO,"eeprom_write_protect");
	gpio_direction_output(WRITE_PROTECT_GPIO, 1);

	eeprom_debug_kobj = kobject_create_and_add("at24", NULL);
	if (eeprom_debug_kobj == NULL)
	{
		dev_err(&client->dev,"%s: subsystem_register failed\n", __FUNCTION__);
	}
	eeprom_dev_info_kobj = kobject_create_and_add("dev-info_eeprom", NULL);
	if (eeprom_dev_info_kobj == NULL)
	{
		dev_err(&client->dev,"%s: subsystem_register failed\n", __FUNCTION__);
	}
	err = sysfs_create_group(eeprom_debug_kobj, &attr_group);
	if(err)
	{
		dev_err(&client->dev,"%s: sysfs_create_group failed, %d\n", __FUNCTION__, __LINE__);
	}
	err = sysfs_create_group(eeprom_dev_info_kobj, &attr_group);
	if(err)
	{
		dev_err(&client->dev,"%s: sysfs_create_group failed, %d\n", __FUNCTION__, __LINE__);
	}
	err = misc_register(&at24_device);
	if (err) {
		printk(KERN_ERR "at24_probe: at24_device register failed\n");
	}
	is_eeprom_ready = true;
	g_at24 = at24;
	return 0;

err_clients:
	for (i = 1; i < num_addresses; i++)
		if (at24->client[i])
			i2c_unregister_device(at24->client[i]);

	kfree(at24->writebuf);
err_struct:
	kfree(at24);
err_out:
	dev_dbg(&client->dev, "probe error %d\n", err);
	return err;
}

static int __devexit at24_remove(struct i2c_client *client)
{
	struct at24_data *at24;
	int i;

	at24 = i2c_get_clientdata(client);
	misc_deregister(&at24_device);
	gpio_free(WRITE_PROTECT_GPIO);

	for (i = 1; i < at24->num_addresses; i++)
		i2c_unregister_device(at24->client[i]);

	kfree(at24->writebuf);
	kfree(at24);
	return 0;
}

static struct i2c_driver at24_driver = {
	.driver = {
		.name = "at24",
		.owner = THIS_MODULE,
	},
	.probe = at24_probe,
	.remove = __devexit_p(at24_remove),
	.id_table = at24_ids,
};

static int __init at24_init(void)
{
	io_limit = rounddown_pow_of_two(io_limit);
	return i2c_add_driver(&at24_driver);
}
module_init(at24_init);

static void __exit at24_exit(void)
{
	i2c_del_driver(&at24_driver);
}
module_exit(at24_exit);

MODULE_AUTHOR("Tab Chiang");
