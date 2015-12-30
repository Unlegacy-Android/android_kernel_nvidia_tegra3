#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/kernel.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>

#define DRIVER_VERSION	"1.0"

#define AL3010_NUM_CACHABLE_REGS	9

#define	AL3010_ALS_COMMAND	4
#define	AL3010_RAN_MASK	0x70
#define	AL3010_RAN_SHIFT	(4)

#define AL3010_MODE_COMMAND	0

#define AL3010_POW_MASK		0x01
#define AL3010_POW_UP		0x01
#define AL3010_POW_DOWN		0x00
#define AL3010_POW_SHIFT	(0)

#define	AL3010_ADC_LSB	0x0c
#define	AL3010_ADC_MSB	0x0d

static int calibration_base_lux = 1000;
static int calibration_regs = 880; // default K value 880 is average K value of PR devices
static int default_calibration_regs = 880;

static bool is_poweron_after_resume = false;
static struct timeval t_poweron_timestamp;

#define AL3010_IOC_MAGIC 0xF3
#define AL3010_IOC_MAXNR 2
#define AL3010_POLL_DATA _IOR(AL3010_IOC_MAGIC,2,int)

#define AL3010_IOCTL_START_HEAVY 2
#define AL3010_IOCTL_START_NORMAL 1
#define AL3010_IOCTL_END 0

#define START_NORMAL	(HZ)
#define START_HEAVY	(HZ)

static int poll_mode = 0;
struct delayed_work al3010_poll_data_work;
static struct workqueue_struct *sensor_work_queue;
struct i2c_client *al3010_client;


static u8 al3010_reg[AL3010_NUM_CACHABLE_REGS] = 
	{0x00, 0x01, 0x0c, 0x0d, 0x10, 0x1a, 0x1b, 0x1c, 0x1d};

struct al3010_data {
	struct i2c_client *client;
	struct mutex lock;
	struct miscdevice misc_dev;
	u8 reg_cache[AL3010_NUM_CACHABLE_REGS];
	u8 power_state_before_suspend;
};

static int revise_lux_times = 2;
static bool al3010_hardware_fail = false;

static int al3010_chip_resume(struct al3010_data *data);

static int __al3010_write_reg(struct i2c_client *client,
				u32 reg, u8 mask, u8 shift, u8 val)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	int ret = 0;
	u8 tmp;

	if (reg >= AL3010_NUM_CACHABLE_REGS)
		return -EINVAL;

	mutex_lock(&data->lock);

	tmp = data->reg_cache[reg];
	tmp &= ~mask;
	tmp |= val << shift;

	ret = i2c_smbus_write_byte_data(client, al3010_reg[reg], tmp);
	if (!ret)
		data->reg_cache[reg] = tmp;

	mutex_unlock(&data->lock);
	return ret;
}

static int al3010_set_range(struct i2c_client *client, int range)
{
	return __al3010_write_reg(client, AL3010_ALS_COMMAND, 
							AL3010_RAN_MASK, AL3010_RAN_SHIFT, range);
}

/* power_state */
static int al3010_set_power_state(struct i2c_client *client, int state)
{
	return __al3010_write_reg(client, AL3010_MODE_COMMAND,
				AL3010_POW_MASK, AL3010_POW_SHIFT, 
				state ? AL3010_POW_UP : AL3010_POW_DOWN);
}

static int al3010_get_power_state(struct i2c_client *client)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	u8 cmdreg;

	mutex_lock(&data->lock);
	cmdreg = i2c_smbus_read_byte_data(client, AL3010_MODE_COMMAND);
	mutex_unlock(&data->lock);

	return (cmdreg & AL3010_POW_MASK) >> AL3010_POW_SHIFT;
}

static int al3010_get_adc_value(struct i2c_client *client)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	int lsb, msb;

	mutex_lock(&data->lock);
	lsb = i2c_smbus_read_byte_data(client, AL3010_ADC_LSB);

	if (lsb < 0) {
		mutex_unlock(&data->lock);
		return lsb;
	}

	msb = i2c_smbus_read_byte_data(client, AL3010_ADC_MSB);
	mutex_unlock(&data->lock);

	if (msb < 0)
		return msb;

	return (u32)((((msb << 8) | lsb) * calibration_base_lux) / calibration_regs);
}

static int al3010_get_reg_value(struct i2c_client *client)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	int lsb, msb;

	mutex_lock(&data->lock);
	lsb = i2c_smbus_read_byte_data(client, AL3010_ADC_LSB);

	if (lsb < 0) {
		mutex_unlock(&data->lock);
		return lsb;
	}

	msb = i2c_smbus_read_byte_data(client, AL3010_ADC_MSB);
	mutex_unlock(&data->lock);

	if (msb < 0)
		return msb;

	return (u16)((msb << 8) | lsb);
}

/*
 * sysfs layer
 */
static ssize_t al3010_show_power_state(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (al3010_hardware_fail)
		return sprintf(buf, "%d\n", 0);

	return sprintf(buf, "%d\n", al3010_get_power_state(client));
}

static ssize_t al3010_show_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	/* No LUX data if not operational */
	if (al3010_get_power_state(client) != 0x01)
		return -EBUSY;

	return sprintf(buf, "%d\n", al3010_get_adc_value(client));
}

static ssize_t al3010_show_reg(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	/* No LUX data if not operational */
	if (al3010_get_power_state(client) != 0x01)
		return -EBUSY;

	return sprintf(buf, "%d\n", al3010_get_reg_value(client));
}

static ssize_t al3010_show_revise_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int require_wait_time = 200;
	struct i2c_client *client = to_i2c_client(dev);

	if (al3010_hardware_fail)
		return sprintf(buf, "%d\n", -1);

	/* No LUX data if not operational */
	if (al3010_get_power_state(client) != 0x01)
		return -EBUSY;

	//+++ wait al3010 wake up
	if (is_poweron_after_resume) {
		msleep(require_wait_time);
		is_poweron_after_resume = false;
	}

	return sprintf(buf, "%d\n", (al3010_get_adc_value(client) * revise_lux_times));
}

static ssize_t al3010_show_default_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int show_lux_value = al3010_get_adc_value(client);
	int show_default_lux_value = (show_lux_value * calibration_regs) / default_calibration_regs;

	return sprintf(buf, "%d\n", show_default_lux_value);
}

static ssize_t al3010_power_on(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3010_data *data = i2c_get_clientdata(client);
	int ret = al3010_chip_resume(data);

	return sprintf(buf, "%d\n", ret);
}

/* calibration */
static ssize_t al3010_show_calib(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", calibration_regs);
}

static ssize_t al3010_store_calib(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long val;

	int err = kstrtoul(buf, 10, &val);
	if (err < 0)
		return err;

	calibration_regs = val;

	dev_info(dev, "calibration data successfully loaded\n");

	return count;
}

static SENSOR_DEVICE_ATTR(show_reg, 0644, al3010_show_reg, NULL, 1);
static SENSOR_DEVICE_ATTR(show_lux, 0644, al3010_show_lux, NULL, 2);
static SENSOR_DEVICE_ATTR(lightsensor_status, 0644, al3010_show_power_state, NULL, 3);
static SENSOR_DEVICE_ATTR(show_revise_lux, 0644, al3010_show_revise_lux, NULL, 4);
static SENSOR_DEVICE_ATTR(show_default_lux, 0644, al3010_show_default_lux, NULL, 5);
static SENSOR_DEVICE_ATTR(power_on, 0644, al3010_power_on, NULL, 6);
static SENSOR_DEVICE_ATTR(calibration, 0644, al3010_show_calib, al3010_store_calib, 7);

static struct attribute *al3010_attributes[] = {
	&sensor_dev_attr_show_reg.dev_attr.attr,
	&sensor_dev_attr_show_lux.dev_attr.attr,
	&sensor_dev_attr_lightsensor_status.dev_attr.attr,
	&sensor_dev_attr_show_revise_lux.dev_attr.attr,
	&sensor_dev_attr_show_default_lux.dev_attr.attr,
	&sensor_dev_attr_power_on.dev_attr.attr,
	&sensor_dev_attr_calibration.dev_attr.attr,
	NULL
};

static const struct attribute_group al3010_attr_group = {
	.attrs = al3010_attributes,
};

/* restore registers from cache */
static int al3010_chip_resume(struct al3010_data *data)
{
	int ret = 0, i = 0;

	if (al3010_get_power_state(data->client) == 0) {
		mutex_lock(&data->lock);
		for (i = 0; i < ARRAY_SIZE(data->reg_cache); i++) {
			if (i2c_smbus_write_byte_data(data->client, i, data->reg_cache[i])) {
				mutex_unlock(&data->lock);
				return -EIO;
			}
		}
		mutex_unlock(&data->lock);
		ret = al3010_set_power_state(data->client, 1);
		is_poweron_after_resume = true;
		do_gettimeofday(&t_poweron_timestamp);
	}

	return ret;
}

static int al3010_init_client(struct i2c_client *client)
{
	int err = 0;

	err = al3010_set_power_state(client, 1);
	if (err) {
		dev_err(&client->dev, "%s: al3010_set_power_state returned error %d\n", __func__, err);
		return err;
	}

	//set sensor range to 4863 lux.
	//(If panel luminousness is 10% , the range of pad is 0 ~ 48630 lux.)
	err = al3010_set_range(client, 2);
	if (err) {
		dev_err(&client->dev, "%s: al3010_set_range returned error %d\n", __func__, err);
		return err;
	}

	return 0;
}

/**
 * i2c stress test
 */
int al3010_open(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);
	return 0;
}

int al3010_release(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);
	return 0;
}

long al3010_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 1;

	if (_IOC_TYPE(cmd) != AL3010_IOC_MAGIC || _IOC_NR(cmd) > AL3010_IOC_MAXNR)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;

	switch (arg) {
		case AL3010_IOCTL_START_HEAVY:
			printk("%s: ioctl heavy\n", __func__);
			poll_mode = START_HEAVY;
			queue_delayed_work(sensor_work_queue, &al3010_poll_data_work, poll_mode);
			break;
		case AL3010_IOCTL_START_NORMAL:
			printk("%s: ioctl normal\n", __func__);
			poll_mode = START_NORMAL;
			queue_delayed_work(sensor_work_queue, &al3010_poll_data_work, poll_mode);
			break;
		case AL3010_IOCTL_END:
			printk("%s: ioctl end\n", __func__);
			cancel_delayed_work_sync(&al3010_poll_data_work);
			break;
		default:
			return -ENOTTY;
	}

	return 0;
}

struct file_operations al3010_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = al3010_ioctl,
	.open =	al3010_open,
	.release = al3010_release,
};

static void al3010_poll_data(struct work_struct * work)
{
	if (poll_mode == 0)
		msleep(5);

	queue_delayed_work(sensor_work_queue, &al3010_poll_data_work, poll_mode);
}

/*
 * I2C layer
 */
static int __devinit al3010_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct al3010_data *data;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	data = kzalloc(sizeof(struct al3010_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;
	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);

	/* initialize the AL3010 chip */
	err = al3010_init_client(client);
	if (err) {
		dev_err(&client->dev, "%s: hardware fail\n", __func__);
		dev_err(&client->dev, "%s: keep al3010 driver alive\n", __func__);
		err = 0;
		al3010_hardware_fail = true;
	}
	//re-init , workaround to fix init fail when i2c arbitration lost
	if (al3010_hardware_fail) {
		err = al3010_init_client(client);
		if (err) {
			dev_err(&client->dev, "%s: re-init fail\n", __func__);
			dev_err(&client->dev, "%s: keep al3010 driver alive\n", __func__);
		} else {
			dev_info(&client->dev, "%s: re-init success\n", __func__);
			al3010_hardware_fail = false;
		}
		err = 0;
	}
	/* register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &al3010_attr_group);
	if (err) {
		dev_err(&client->dev, "%s: sysfs_create_group returned error %d\n", __func__, err);
		goto exit_kfree;
	}

	/* register device node */
	dev_info(&client->dev, "%s: initialized\n", __func__);
	dev_info(&client->dev, "driver version %s enabled\n", DRIVER_VERSION);

	/* init for i2c stress test */
	sensor_work_queue = create_singlethread_workqueue("i2c_lightsensor_wq");
	if (!sensor_work_queue) {
		dev_err(&client->dev, "%s: create_singlethread_workqueue failed\n", __func__);
		goto exit_kfree;
	}
	INIT_DELAYED_WORK(&al3010_poll_data_work, al3010_poll_data);
	al3010_client = client;
	data->misc_dev.minor = MISC_DYNAMIC_MINOR;
	data->misc_dev.name = "lightsensor";
	data->misc_dev.fops = &al3010_fops;
	err = misc_register(&data->misc_dev);
	if (err) {
		dev_err(&client->dev, "%s: unable to register misc device %s\n", __func__, data->misc_dev.name);
		goto exit_kfree;
	}

	return 0;

exit_kfree:
	kfree(data);
	return err;
}

static int __devexit al3010_remove(struct i2c_client *client)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	misc_deregister(&data->misc_dev);

	sysfs_remove_group(&client->dev.kobj, &al3010_attr_group);
	al3010_set_power_state(client, 0);
	kfree(i2c_get_clientdata(client));
	dev_info(&client->dev, "%s: remove succeeded\n", __func__);

	return 0;
}

#ifdef CONFIG_PM
static int al3010_suspend(struct device *dev)
{
	if (al3010_hardware_fail)
		return 0;

	dev_dbg(dev, "%s\n", __func__);

	return al3010_set_power_state(to_i2c_client(dev), 0);
}

static int al3010_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (al3010_hardware_fail)
		return 0;

	// delay to avoid suspend and resume too close as it could cause power on fail
	mdelay(5);

	dev_dbg(dev, "%s\n", __func__);

	return al3010_chip_resume(i2c_get_clientdata(client));
}

static SIMPLE_DEV_PM_OPS(al3010_pm, al3010_suspend, al3010_resume);
#endif

static const struct i2c_device_id al3010_id[] = {
	{ "al3010", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, al3010_id);

static struct i2c_driver al3010_driver = {
	.driver = {
		.name	= "al3010",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm		= &al3010_pm,
#endif
	},
	.probe	= al3010_probe,
	.remove	= __devexit_p(al3010_remove),
	.id_table = al3010_id,
};

static int __init al3010_init(void)
{
	return i2c_add_driver(&al3010_driver);
}

static void __exit al3010_exit(void)
{
	printk("%s\n", __func__);
	i2c_del_driver(&al3010_driver);
}

MODULE_AUTHOR("yc");
MODULE_DESCRIPTION("test version v1.0");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRIVER_VERSION);

module_init(al3010_init);
module_exit(al3010_exit);
