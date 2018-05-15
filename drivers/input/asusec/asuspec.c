/*
 * ASUS EC driver.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/cdev.h>
#include <linux/gpio_event.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/power_supply.h>
#include <linux/statfs.h>

#include <asm/gpio.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>

#include <../gpio-names.h>

#include <mach/board-transformer-misc.h>

#include "asuspec.h"

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/*
 * functions declaration
 */
static int asuspec_dockram_write_data(int cmd, int length);
static int asuspec_dockram_read_data(int cmd);
static int asuspec_dockram_read_battery(int cmd);
static int asuspec_i2c_write_data(struct i2c_client *client, u16 data);
static int asuspec_i2c_read_data(struct i2c_client *client);
static int asuspec_chip_init(struct i2c_client *client);
static void asuspec_send_ec_req(void);
static void asuspec_enter_s3_timer(unsigned long data);
static void asuspec_enter_s3_work_function(struct work_struct *dat);
static void asuspec_fw_update_work_function(struct work_struct *dat);
static void asuspec_work_function(struct work_struct *dat);
static int __devinit asuspec_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
static int __devexit asuspec_remove(struct i2c_client *client);
static ssize_t asuspec_status_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_info_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_version_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_battery_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_control_flag_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_send_ec_req_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_charging_led_store(struct device *class,
		struct device_attribute *attr,const char *buf, size_t count);
static ssize_t asuspec_led_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_enter_normal_mode_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_switch_hdmi_show(struct device *class,
		struct device_attribute *attr,char *buf);
static ssize_t asuspec_win_shutdown_show(struct device *class,
		struct device_attribute *attr,char *buf);
//static ssize_t asuspec_cmd_data_store(struct device *class,
//		struct device_attribute *attr,const char *buf, size_t count);
//static ssize_t asuspec_return_data_show(struct device *class,
//		struct device_attribute *attr,char *buf);
static ssize_t asuspec_switch_name(struct switch_dev *sdev, char *buf);
static ssize_t asuspec_switch_state(struct switch_dev *sdev, char *buf);
static ssize_t apower_switch_name(struct switch_dev *sdev, char *buf);
static ssize_t apower_switch_state(struct switch_dev *sdev, char *buf);
static int asuspec_suspend(struct device *dev);
static int asuspec_resume(struct device *dev);
static int asuspec_open(struct inode *inode, struct file *flip);
static int asuspec_release(struct inode *inode, struct file *flip);
static long asuspec_ioctl(struct file *flip, unsigned int cmd, unsigned long arg);
static void asuspec_switch_apower_state(int state);
static void asuspec_switch_hdmi(void);
static void asuspec_win_shutdown(void);
//static void asuspec_storage_info_update(void);
static void asuspec_enter_normal_mode(void);
static ssize_t ec_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static ssize_t ec_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static void BuffPush(char data);

/*
* extern variable
*/
extern unsigned int factory_mode;

/*
 * global variable
 */
static unsigned int asuspec_apwake_gpio = TEGRA_GPIO_PS2;
static unsigned int asuspec_ecreq_gpio = TEGRA_GPIO_PQ1;

static char host_to_ec_buffer[EC_BUFF_LEN];
static char ec_to_host_buffer[EC_BUFF_LEN];
static int h2ec_count;
static int buff_in_ptr;	  // point to the next free place
static int buff_out_ptr;	  // points to the first data
int reg_addr = -1;

struct i2c_client dockram_client;
static struct class *asuspec_class;
static struct device *asuspec_device ;
static struct asuspec_chip *ec_chip;

struct cdev *asuspec_cdev ;
static dev_t asuspec_dev ;
static int asuspec_major = 0 ;
static int asuspec_minor = 0 ;

static struct workqueue_struct *asuspec_wq;
struct delayed_work asuspec_stress_work;

static const struct i2c_device_id asuspec_id[] = {
	{"asuspec", 0},
	{}
};


MODULE_DEVICE_TABLE(i2c, asuspec_id);

struct file_operations asuspec_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = asuspec_ioctl,
	.open = asuspec_open,
	.write = ec_write,
	.read = ec_read,
	.release = asuspec_release,
};

static struct dev_pm_ops asuspec_dev_pm_ops ={
	.suspend = asuspec_suspend,
	.resume = asuspec_resume,
};

static struct i2c_driver asuspec_driver = {
	.class	= I2C_CLASS_HWMON,
	.driver	 = {
		.name = "asuspec",
		.owner = THIS_MODULE,
		.pm = &asuspec_dev_pm_ops,
	},
	.probe	 = asuspec_probe,
	.remove	 = __devexit_p(asuspec_remove),
	.id_table = asuspec_id,
};

static DEVICE_ATTR(ec_status, S_IWUSR | S_IRUGO, asuspec_status_show,NULL);
static DEVICE_ATTR(ec_info, S_IWUSR | S_IRUGO, asuspec_info_show,NULL);
static DEVICE_ATTR(ec_version, S_IWUSR | S_IRUGO, asuspec_version_show,NULL);
static DEVICE_ATTR(ec_battery, S_IWUSR | S_IRUGO, asuspec_battery_show,NULL);
static DEVICE_ATTR(ec_control_flag, S_IWUSR | S_IRUGO, asuspec_control_flag_show,NULL);
static DEVICE_ATTR(ec_request, S_IWUSR | S_IRUGO, asuspec_send_ec_req_show,NULL);
static DEVICE_ATTR(ec_led, S_IWUSR | S_IRUGO, asuspec_led_show,NULL);
static DEVICE_ATTR(ec_charging_led, S_IWUSR | S_IRUGO, NULL, asuspec_charging_led_store);
static DEVICE_ATTR(ec_normal_mode, S_IWUSR | S_IRUGO, asuspec_enter_normal_mode_show,NULL);
static DEVICE_ATTR(ec_switch_hdmi, S_IWUSR | S_IRUGO, asuspec_switch_hdmi_show,NULL);
static DEVICE_ATTR(ec_win_shutdown, S_IWUSR | S_IRUGO, asuspec_win_shutdown_show,NULL);

static struct attribute *asuspec_smbus_attributes[] = {
	&dev_attr_ec_status.attr,
	&dev_attr_ec_info.attr,
	&dev_attr_ec_version.attr,
	&dev_attr_ec_battery.attr,
	&dev_attr_ec_control_flag.attr,
	&dev_attr_ec_request.attr,
	&dev_attr_ec_led.attr,
	&dev_attr_ec_charging_led.attr,
	&dev_attr_ec_normal_mode.attr,
	&dev_attr_ec_switch_hdmi.attr,
	&dev_attr_ec_win_shutdown.attr,
NULL
};

static const struct attribute_group asuspec_smbus_group = {
	.attrs = asuspec_smbus_attributes,
};

#if ASUSPEC_DEBUG
int dbg_counter = 0;
#endif
/*
 * functions definition
 */
int asuspec_audio_recording(int record_enable){
	if (record_enable)
		asuspec_send_ec_req();
	ec_chip->audio_recording = record_enable;
	ASUSPEC_NOTICE("audio_recording = %d\n", ec_chip->audio_recording);
	return 0;
}
EXPORT_SYMBOL(asuspec_audio_recording);

int asuspec_is_usb_charger(int charger_enable){
	int ret = 0;

	if (ec_chip->ec_in_s3){
		asuspec_send_ec_req();
		msleep(200);
	}
	ret = asuspec_dockram_read_data(0x0A);
	if (ret < 0){
		ASUSPEC_ERR("Fail to access control flag info.\n");
		return ret;
	}

	ec_chip->i2c_dm_data[0] = 8;
	if (charger_enable){
		ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[5] | 0x01;
	} else {
		ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[5] & 0xFE;
	}
	ret = asuspec_dockram_write_data(0x0A,9);
	mod_timer(&ec_chip->asuspec_timer,jiffies+(HZ * 1));
	return ret;
}
EXPORT_SYMBOL(asuspec_is_usb_charger);

int asuspec_battery_monitor(char *cmd){
	int ret_val = 0;

	if (ec_chip->ec_in_s3){
		asuspec_send_ec_req();
		msleep(200);
	}
	ret_val = asuspec_dockram_read_battery(0x14);

	if (ret_val == -1){
		ASUSPEC_ERR("Fail to access battery info.\n");
		return -1;
	}
	else {
                if(ec_chip->audio_recording == 0){
                        mod_timer(&ec_chip->asuspec_timer,jiffies+(HZ * 1));
                }
		if (!strcmp(cmd, "status"))
			ret_val = (ec_chip->i2c_dm_battery[2] << 8 ) | ec_chip->i2c_dm_battery[1];
		else if (!strcmp(cmd, "temperature"))
			ret_val = (ec_chip->i2c_dm_battery[8] << 8 ) | ec_chip->i2c_dm_battery[7];
		else if (!strcmp(cmd, "voltage"))
			ret_val = (ec_chip->i2c_dm_battery[10] << 8 ) | ec_chip->i2c_dm_battery[9];
		else if (!strcmp(cmd, "current"))
			ret_val = (ec_chip->i2c_dm_battery[12] << 8 ) | ec_chip->i2c_dm_battery[11];
		else if (!strcmp(cmd, "capacity"))
			ret_val = (ec_chip->i2c_dm_battery[14] << 8 ) | ec_chip->i2c_dm_battery[13];
		else if (!strcmp(cmd, "remaining_capacity"))
			ret_val = (ec_chip->i2c_dm_battery[16] << 8 ) | ec_chip->i2c_dm_battery[15];
		else if (!strcmp(cmd, "avg_time_to_empty"))
			ret_val = (ec_chip->i2c_dm_battery[18] << 8 ) | ec_chip->i2c_dm_battery[17];
		else if (!strcmp(cmd, "avg_time_to_full"))
			ret_val = (ec_chip->i2c_dm_battery[20] << 8 ) | ec_chip->i2c_dm_battery[19];
		else {
			ASUSPEC_ERR("Unknown command\n");
			ret_val = -2;
		}
		ASUSPEC_INFO("cmd %s, return %d\n", cmd, ret_val);
		return ret_val;
	}
}
EXPORT_SYMBOL(asuspec_battery_monitor);

static void asuspec_dockram_init(struct i2c_client *client){
	dockram_client.adapter = client->adapter;
	dockram_client.addr = 0x17;
	dockram_client.detected = client->detected;
	dockram_client.dev = client->dev;
	dockram_client.driver = client->driver;
	dockram_client.flags = client->flags;
	strcpy(dockram_client.name,client->name);
	ec_chip->ec_ram_init = ASUSPEC_MAGIC_NUM;
}

static int asuspec_dockram_write_data(int cmd, int length)
{
	int ret = 0;

	if (ec_chip->ec_ram_init != ASUSPEC_MAGIC_NUM){
		ASUSPEC_ERR("DockRam is not ready.\n");
		return -1;
	}

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access dockram under FW update mode.\n");
		return -2;
	}

	if (ec_chip->i2c_err_count > ASUSPEC_I2C_ERR_TOLERANCE){
		return -3;
	}

	ret = i2c_smbus_write_i2c_block_data(&dockram_client, cmd, length, ec_chip->i2c_dm_data);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to write dockram data, status %d\n", ret);
	} else {
		ec_chip->i2c_err_count = 0;
	}
	return ret;
}

static int asuspec_dockram_read_data(int cmd)
{
	int ret = 0;

	if (ec_chip->ec_ram_init != ASUSPEC_MAGIC_NUM){
		ASUSPEC_ERR("DockRam is not ready.\n");
		return -1;
	}

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access dockram under FW update mode.\n");
		return -2;
	}

	if (ec_chip->i2c_err_count > ASUSPEC_I2C_ERR_TOLERANCE){
		return -3;
	}

	ret = i2c_smbus_read_i2c_block_data(&dockram_client, cmd, 32, ec_chip->i2c_dm_data);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to read dockram data, status %d\n", ret);
	} else {
		ec_chip->i2c_err_count = 0;
	}
	return ret;
}

#if 0
static int asuspec_dockram_write_storageinfo(int cmd, int length)
{
	int ret = 0;

	if (ec_chip->ec_ram_init != ASUSPEC_MAGIC_NUM){
		ASUSPEC_ERR("DockRam is not ready.\n");
		return -1;
	}

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access dockram under FW update mode.\n");
		return -2;
	}

	if (ec_chip->i2c_err_count > ASUSPEC_I2C_ERR_TOLERANCE){
		return -3;
	}

	ret = i2c_smbus_write_i2c_block_data(&dockram_client, cmd, length, ec_chip->i2c_dm_storage);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to write dockram data, status %d\n", ret);
	} else {
		ec_chip->i2c_err_count = 0;
	}
	return ret;
}

static int asuspec_dockram_read_storageinfo(int cmd)
{
	int ret = 0;

	if (ec_chip->ec_ram_init != ASUSPEC_MAGIC_NUM){
		ASUSPEC_ERR("DockRam is not ready.\n");
		return -1;
	}

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access dockram under FW update mode.\n");
		return -2;
	}

	if (ec_chip->i2c_err_count > ASUSPEC_I2C_ERR_TOLERANCE){
		return -3;
	}

	ret = i2c_smbus_read_i2c_block_data(&dockram_client, cmd, 32, ec_chip->i2c_dm_storage);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to read dockram data, status %d\n", ret);
	} else {
		ec_chip->i2c_err_count = 0;
	}
	return ret;
}
#endif

static int asuspec_dockram_read_battery(int cmd)
{
	int ret = 0;

	if (ec_chip->ec_ram_init != ASUSPEC_MAGIC_NUM){
		ASUSPEC_ERR("DockRam is not ready.\n");
		return -1;
	}

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access dockram under FW update mode.\n");
		return -2;
	}

	ret = i2c_smbus_read_i2c_block_data(&dockram_client, cmd, 32, ec_chip->i2c_dm_battery);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to read dockram battery, status %d\n", ret);
		ret = -1;
	} else {
		if (ec_chip->apwake_disabled){
			mutex_lock(&ec_chip->irq_lock);
			if (ec_chip->apwake_disabled){
				enable_irq(gpio_to_irq(asuspec_apwake_gpio));
				enable_irq_wake(gpio_to_irq(asuspec_apwake_gpio));
				ec_chip->apwake_disabled = 0;
				ASUSPEC_ERR("Enable pad apwake\n");
			}
			mutex_unlock(&ec_chip->irq_lock);
		}
		ec_chip->i2c_err_count = 0;
	}
	ASUSPEC_I2C_DATA(ec_chip->i2c_dm_battery, dbg_counter);
	return ret;
}

static int asuspec_i2c_write_data(struct i2c_client *client, u16 data)
{
	int ret = 0;

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access ec under FW update mode.\n");
		return -1;
	}

	if (ec_chip->i2c_err_count > ASUSPEC_I2C_ERR_TOLERANCE){
		return -3;
	}

	ret = i2c_smbus_write_word_data(client, 0x64, data);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to write data, status %d\n", ret);
	} else {
		ec_chip->i2c_err_count = 0;
	}
	return ret;
}

static int asuspec_i2c_read_data(struct i2c_client *client)
{
	int ret = 0;

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access ec under FW update mode.\n");
		return -1;
	}

	if (ec_chip->i2c_err_count > ASUSPEC_I2C_ERR_TOLERANCE){
		mutex_lock(&ec_chip->irq_lock);
		if(!ec_chip->apwake_disabled){
			disable_irq_nosync(gpio_to_irq(asuspec_apwake_gpio));
			disable_irq_wake(gpio_to_irq(asuspec_apwake_gpio));
			ec_chip->apwake_disabled = 1;
			ASUSPEC_ERR("Disable pad apwake\n");
		}
		mutex_unlock(&ec_chip->irq_lock);
		return -3;
	}

	ret = i2c_smbus_read_i2c_block_data(client, 0x6A, 8, ec_chip->i2c_data);
	if (ret < 0) {
		ASUSPEC_ERR("Fail to read data, status %d\n", ret);
		ec_chip->i2c_err_count++;
	} else {
		ec_chip->i2c_err_count = 0;
	}
	ASUSPEC_I2C_DATA(ec_chip->i2c_data, dbg_counter);
	return ret;
}

static int asuspec_i2c_test(struct i2c_client *client)
{
	return asuspec_i2c_write_data(client, 0x0000);
}

static int asuspec_chip_init(struct i2c_client *client)
{
	int ret_val = 0;
	int i = 0;

	ec_chip->op_mode = 0;

	for ( i = 0; i < 10; i++){
		ret_val = asuspec_i2c_test(client);
		if (ret_val < 0)
			msleep(300);
		else
			break;
	}

	if(ret_val < 0){
		goto fail_to_access_ec;
	}

	for ( i=0; i<8; i++){
		asuspec_i2c_read_data(client);
	}

	if (asuspec_dockram_read_data(0x01) < 0){
		goto fail_to_access_ec;
	}
	strcpy(ec_chip->ec_model_name, &ec_chip->i2c_dm_data[1]);
	ASUSPEC_NOTICE("Model Name: %s\n", ec_chip->ec_model_name);

	if (asuspec_dockram_read_data(0x02) < 0){
		goto fail_to_access_ec;
	}
	strcpy(ec_chip->ec_version, &ec_chip->i2c_dm_data[1]);
	ASUSPEC_NOTICE("EC-FW Version: %s\n", ec_chip->ec_version);

	if (asuspec_dockram_read_data(0x03) < 0){
		goto fail_to_access_ec;
	}
	ASUSPEC_INFO("EC-Config Format: %s\n", &ec_chip->i2c_dm_data[1]);

	if (asuspec_dockram_read_data(0x04) < 0){
		goto fail_to_access_ec;
	}
	strcpy(ec_chip->ec_pcba, &ec_chip->i2c_dm_data[1]);
	ASUSPEC_NOTICE("PCBA Version: %s\n", ec_chip->ec_pcba);

	asuspec_enter_normal_mode();

	ec_chip->status = 1;
	switch_set_state(&ec_chip->pad_sdev, !ec_chip->pad_sdev.state);
fail_to_access_ec:
	return 0;

}

static irqreturn_t asuspec_interrupt_handler(int irq, void *dev_id){

	ASUSPEC_INFO("interrupt irq = %d\n", irq);
	if (irq == gpio_to_irq(asuspec_apwake_gpio)){
		disable_irq_nosync(irq);
		if (ec_chip->op_mode){
			queue_delayed_work(asuspec_wq, &ec_chip->asuspec_fw_update_work, 0);
		} else {
			queue_delayed_work(asuspec_wq, &ec_chip->asuspec_work, 0);
		}
	}
	return IRQ_HANDLED;
}

static int asuspec_irq_ec_request(struct i2c_client *client)
{
	int rc = 0 ;
	unsigned gpio = asuspec_ecreq_gpio;
	const char* label = "asuspec_request" ;

	ASUSPEC_INFO("GPIO = %d , state = %d\n", gpio, gpio_get_value(gpio));

	rc = gpio_request(gpio, label);
	if (rc) {
		ASUSPEC_ERR("gpio_request failed for input %d\n", gpio);
		goto err_exit;
	}

	rc = gpio_direction_output(gpio, 1) ;
	if (rc) {
		ASUSPEC_ERR("gpio_direction_output failed for input %d\n", gpio);
		goto err_exit;
	}
	ASUSPEC_INFO("GPIO = %d , state = %d\n", gpio, gpio_get_value(gpio));
	return 0 ;

err_exit:
	return rc;
}


static int asuspec_irq_ec_apwake(struct i2c_client *client)
{
	int rc = 0 ;
	unsigned gpio = asuspec_apwake_gpio;
	int irq = gpio_to_irq(gpio);
	const char* label = "asuspec_apwake" ;

	ASUSPEC_INFO("GPIO = %d, irq = %d\n", gpio, irq);
	ASUSPEC_INFO("GPIO = %d , state = %d\n", gpio, gpio_get_value(gpio));

	rc = gpio_request(gpio, label);
	if (rc) {
		ASUSPEC_ERR("gpio_request failed for input %d\n", gpio);
		goto err_request_input_gpio_failed;
	}

	rc = gpio_direction_input(gpio) ;
	if (rc) {
		ASUSPEC_ERR("gpio_direction_input failed for input %d\n", gpio);
		goto err_gpio_direction_input_failed;
	}
	ASUSPEC_INFO("GPIO = %d , state = %d\n", gpio, gpio_get_value(gpio));

	rc = request_irq(irq, asuspec_interrupt_handler,/*IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING|IRQF_TRIGGER_HIGH|*/IRQF_TRIGGER_LOW, label, client);
	if (rc < 0) {
		ASUSPEC_ERR("Could not register for %s interrupt, irq = %d, rc = %d\n", label, irq, rc);
		rc = -EIO;
		goto err_gpio_request_irq_fail ;
	}

	enable_irq_wake(gpio_to_irq(asuspec_apwake_gpio));
	ASUSPEC_INFO("request irq = %d, rc = %d\n", irq, rc);

	return 0 ;

err_gpio_request_irq_fail:
	gpio_free(gpio);
err_gpio_direction_input_failed:
err_request_input_gpio_failed :
	return rc;

	return 0 ;
}

static void asuspec_enter_s3_timer(unsigned long data){
	queue_delayed_work(asuspec_wq, &ec_chip->asuspec_enter_s3_work, 0);
}

static void asuspec_send_ec_req(void){
	ASUSPEC_NOTICE("send EC_Request\n");
	gpio_set_value(asuspec_ecreq_gpio, 0);
	msleep(DELAY_TIME_MS);
	gpio_set_value(asuspec_ecreq_gpio, 1);
}

static void asuspec_smi(void){
	if (ec_chip->i2c_data[2] == ASUSPEC_SMI_HANDSHAKING){
		ASUSPEC_NOTICE("ASUSPEC_SMI_HANDSHAKING\n");
		if(ec_chip->status == 0){
			asuspec_chip_init(ec_chip->client);
		}
		ec_chip->ec_in_s3 = 0;
	} else if (ec_chip->i2c_data[2] == ASUSPEC_SMI_RESET){
		ASUSPEC_NOTICE("ASUSPEC_SMI_RESET\n");
		queue_delayed_work(asuspec_wq, &ec_chip->asuspec_init_work, 0);
	} else if (ec_chip->i2c_data[2] == ASUSPEC_SMI_WAKE){
		ASUSPEC_NOTICE("ASUSPEC_SMI_WAKE\n");
	} else if (ec_chip->i2c_data[2] == APOWER_SMI_S5){
		ASUSPEC_NOTICE("APOWER_POWEROFF\n");
		asuspec_switch_apower_state(APOWER_POWEROFF);
	} else if (ec_chip->i2c_data[2] == APOWER_SMI_NOTIFY_SHUTDOWN){
		ASUSPEC_NOTICE("APOWER_NOTIFY_SHUTDOWN\n");
		asuspec_switch_apower_state(APOWER_NOTIFY_SHUTDOWN);
	} else if (ec_chip->i2c_data[2] == APOWER_SMI_RESUME){
		ASUSPEC_NOTICE("APOWER_SMI_RESUME\n");
		asuspec_switch_apower_state(APOWER_RESUME);
	}
}

static void asuspec_enter_s3_work_function(struct work_struct *dat)
{
	int ret_val = 0;
	int i = 0;

	mutex_lock(&ec_chip->state_change_lock);

	if (ec_chip->op_mode){
		ASUSPEC_ERR("It's not allowed to access dockram under FW update mode.\n");
		mutex_unlock(&ec_chip->state_change_lock);
		return ;
	}

	ec_chip->ec_in_s3 = 1;
	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_read_data(0x0A);
		if (ret_val < 0){
			ASUSPEC_ERR("fail to get control flag\n");
			msleep(100);
		}
		else
			break;
	}

	ec_chip->i2c_dm_data[0] = 8;
	ec_chip->i2c_dm_data[5] = ec_chip->i2c_dm_data[5] | 0x02;

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_write_data(0x0A,9);
		if (ret_val < 0){
			ASUSPEC_ERR("Send s3 command fail\n");
			msleep(100);
		}
		else {
			ASUSPEC_NOTICE("EC in S3\n");
			break;
		}
	}
	mutex_unlock(&ec_chip->state_change_lock);
}

static void asuspec_init_work_function(struct work_struct *dat)
{
	asuspec_send_ec_req();
	msleep(200);
	asuspec_chip_init(ec_chip->client);
}

static void asuspec_stresstest_work_function(struct work_struct *dat)
{
	asuspec_i2c_read_data(ec_chip->client);
	queue_delayed_work(asuspec_wq, &asuspec_stress_work, HZ/ec_chip->polling_rate);
}

static void asuspec_fw_update_work_function(struct work_struct *dat)
{
	int smbus_data;
	int gpio = asuspec_apwake_gpio;
	int irq = gpio_to_irq(gpio);

	mutex_lock(&ec_chip->lock);
	smbus_data = i2c_smbus_read_byte_data(&dockram_client, 0);
	enable_irq(irq);
	BuffPush(smbus_data);
	mutex_unlock(&ec_chip->lock);
}

static void asuspec_work_function(struct work_struct *dat)
{
	int gpio = asuspec_apwake_gpio;
	int irq = gpio_to_irq(gpio);
	int ret_val = 0;

	ret_val = asuspec_i2c_read_data(ec_chip->client);
	enable_irq(irq);

	ASUSPEC_NOTICE("0x%x 0x%x 0x%x 0x%x\n", ec_chip->i2c_data[0],
		ec_chip->i2c_data[1], ec_chip->i2c_data[2], ec_chip->i2c_data[3]);

	if (ret_val < 0){
		return ;
	}

	if (ec_chip->i2c_data[1] & ASUSPEC_OBF_MASK){
		if (ec_chip->i2c_data[1] & ASUSPEC_SMI_MASK){
			asuspec_smi();
			return ;
		}
	}
}

static int __devinit asuspec_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0;

	ASUSPEC_INFO("asuspec probe\n");
	err = sysfs_create_group(&client->dev.kobj, &asuspec_smbus_group);
	if (err) {
		ASUSPEC_ERR("Unable to create the sysfs\n");
		goto exit;
	}

	ec_chip = kzalloc(sizeof (struct asuspec_chip), GFP_KERNEL);
	if (!ec_chip) {
		ASUSPEC_ERR("Memory allocation fails\n");
		err = -ENOMEM;
		goto exit;
	}

	ec_chip->pad_pid = tegra3_get_project_id();
	i2c_set_clientdata(client, ec_chip);
	ec_chip->client = client;
	ec_chip->client->driver = &asuspec_driver;
	ec_chip->client->flags = 1;

	init_timer(&ec_chip->asuspec_timer);
	ec_chip->asuspec_timer.function = asuspec_enter_s3_timer;

	wake_lock_init(&ec_chip->wake_lock, WAKE_LOCK_SUSPEND, "asuspec_wake");
	mutex_init(&ec_chip->lock);
	mutex_init(&ec_chip->irq_lock);
	mutex_init(&ec_chip->state_change_lock);

	ec_chip->ec_ram_init = 0;
	ec_chip->audio_recording = 0;
	ec_chip->status = 0;
	ec_chip->ec_in_s3 = 0;
	ec_chip->apwake_disabled = 0;
	ec_chip->storage_total = 0;
	ec_chip->storage_avail = 0;
	asuspec_dockram_init(client);
	cdev_add(asuspec_cdev,asuspec_dev,1) ;

	ec_chip->pad_sdev.name = PAD_SDEV_NAME;
	ec_chip->pad_sdev.print_name = asuspec_switch_name;
	ec_chip->pad_sdev.print_state = asuspec_switch_state;
	if(switch_dev_register(&ec_chip->pad_sdev) < 0){
		ASUSPEC_ERR("switch_dev_register for pad failed!\n");
	}
	switch_set_state(&ec_chip->pad_sdev, 0);

	ec_chip->apower_sdev.name = APOWER_SDEV_NAME;
	ec_chip->apower_sdev.print_name = apower_switch_name;
	ec_chip->apower_sdev.print_state = apower_switch_state;
	ec_chip->apower_state = 0;
	if(switch_dev_register(&ec_chip->apower_sdev) < 0){
		ASUSPEC_ERR("switch_dev_register for apower failed!\n");
	}
	switch_set_state(&ec_chip->apower_sdev, ec_chip->apower_state);

	asuspec_wq = create_singlethread_workqueue("asuspec_wq");
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asuspec_work, asuspec_work_function);
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asuspec_init_work, asuspec_init_work_function);
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asuspec_fw_update_work, asuspec_fw_update_work_function);
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asuspec_enter_s3_work, asuspec_enter_s3_work_function);
	INIT_DELAYED_WORK_DEFERRABLE(&asuspec_stress_work, asuspec_stresstest_work_function);

	asuspec_irq_ec_request(client);
	asuspec_irq_ec_apwake(client);
	queue_delayed_work(asuspec_wq, &ec_chip->asuspec_init_work, 0);

	return 0;

exit:
	return err;
}

static int __devexit asuspec_remove(struct i2c_client *client)
{
	struct asuspec_chip *chip = i2c_get_clientdata(client);

	dev_dbg(&client->dev, "%s()\n", __func__);
	input_unregister_device(chip->indev);
	kfree(chip);
	return 0;
}

static ssize_t asuspec_status_show(struct device *class,struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%d\n", ec_chip->status);
}

static ssize_t asuspec_info_show(struct device *class,struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%s\n", ec_chip->ec_version);
}

static ssize_t asuspec_version_show(struct device *class,struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%s\n", ec_chip->ec_version);
}

static ssize_t asuspec_battery_show(struct device *class,struct device_attribute *attr,char *buf)
{
	int bat_status, bat_temp, bat_vol, bat_current, bat_capacity, remaining_cap;
	int ret_val = 0;
	char temp_buf[64];

	bat_status = asuspec_battery_monitor("status");
	bat_temp = asuspec_battery_monitor("temperature");
	bat_vol = asuspec_battery_monitor("voltage");
	bat_current = asuspec_battery_monitor("current");
	bat_capacity = asuspec_battery_monitor("capacity");
	remaining_cap = asuspec_battery_monitor("remaining_capacity");

	if (ret_val < 0)
		return sprintf(buf, "fail to get battery info\n");
	else {
		sprintf(temp_buf, "status = 0x%x\n", bat_status);
		strcpy(buf, temp_buf);
		sprintf(temp_buf, "temperature = %d\n", bat_temp);
		strcat(buf, temp_buf);
		sprintf(temp_buf, "voltage = %d\n", bat_vol);
		strcat(buf, temp_buf);
		sprintf(temp_buf, "current = %d\n", bat_current);
		strcat(buf, temp_buf);
		sprintf(temp_buf, "capacity = %d\n", bat_capacity);
		strcat(buf, temp_buf);
		sprintf(temp_buf, "remaining capacity = %d\n", remaining_cap);
		strcat(buf, temp_buf);

		return strlen(buf);
	}
}

static ssize_t asuspec_control_flag_show(struct device *class,struct device_attribute *attr,char *buf)
{
	int ret_val = 0;
	int i = 0;
	char temp_buf[64];

	ret_val = asuspec_dockram_read_data(0x0A);
	if (ret_val < 0)
		return sprintf(buf, "fail to get pad ec control-flag info\n");
	else{
		sprintf(temp_buf, "byte[0] = 0x%x\n", ec_chip->i2c_dm_data[i]);
		strcpy(buf, temp_buf);
		for (i = 1; i < 9; i++){
			sprintf(temp_buf, "byte[%d] = 0x%x\n", i, ec_chip->i2c_dm_data[i]);
			strcat(buf, temp_buf);
		}
		return strlen(buf);
	}
}

static ssize_t asuspec_send_ec_req_show(struct device *class,struct device_attribute *attr,char *buf)
{
	asuspec_send_ec_req();
	return sprintf(buf, "EC_REQ is sent\n");
}


static ssize_t asuspec_charging_led_store(struct device *class,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret_val = 0;

	if (ec_chip->op_mode == 0){
		asuspec_dockram_read_data(0x0A);
		if (buf[0] == '0'){
			ec_chip->i2c_dm_data[0] = 8;
			ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[6] & 0xF9;
			ret_val = asuspec_dockram_write_data(0x0A,9);
			if (ret_val < 0)
				ASUSPEC_NOTICE("Fail to diable led test\n");
			else
				ASUSPEC_NOTICE("Diable led test\n");
		} else if (buf[0] == '1'){
			asuspec_dockram_read_data(0x0A);
			ec_chip->i2c_dm_data[0] = 8;
			ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[6] & 0xF9;
			ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[6] | 0x02;
			ret_val = asuspec_dockram_write_data(0x0A,9);
			if (ret_val < 0)
				ASUSPEC_NOTICE("Fail to enable orange led test\n");
			else
				ASUSPEC_NOTICE("Enable orange led test\n");
		} else if (buf[0] == '2'){
			asuspec_dockram_read_data(0x0A);
			ec_chip->i2c_dm_data[0] = 8;
			ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[6] & 0xF9;
			ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[6] | 0x04;
			ret_val = asuspec_dockram_write_data(0x0A,9);
			if (ret_val < 0)
				ASUSPEC_NOTICE("Fail to enable green led test\n");
			else
				ASUSPEC_NOTICE("Enable green led test\n");
		}
	} else {
		ASUSPEC_NOTICE("Fail to enter led test\n");
	}

	return count;
}

static ssize_t asuspec_led_show(struct device *class,struct device_attribute *attr,char *buf)
{
	int ret_val = 0;

	asuspec_dockram_read_data(0x0A);
	ec_chip->i2c_dm_data[0] = 8;
	ec_chip->i2c_dm_data[6] = ec_chip->i2c_dm_data[6] | 0x01;
	ret_val = asuspec_dockram_write_data(0x0A,9);
	if (ret_val < 0)
		return sprintf(buf, "Fail to EC LED Blink\n");
	else
		return sprintf(buf, "EC LED Blink\n");
}

static ssize_t asuspec_enter_normal_mode_show(struct device *class,struct device_attribute *attr,char *buf)
{
	asuspec_enter_normal_mode();
	return sprintf(buf, "Entering normal mode\n");
}

static ssize_t asuspec_switch_hdmi_show(struct device *class,struct device_attribute *attr,char *buf)
{
	asuspec_switch_hdmi();
	return sprintf(buf, "Switch hdmi\n");
}

static ssize_t asuspec_win_shutdown_show(struct device *class,struct device_attribute *attr,char *buf)
{
	asuspec_win_shutdown();
	return sprintf(buf, "Win shutdown\n");
}

#if 0
static ssize_t asuspec_cmd_data_store(struct device *class,struct device_attribute *attr,const char *buf, size_t count)
{
	int buf_len = strlen(buf);
	int data_len = (buf_len -1)/2;
	char chr[2], data_num[data_len];
	int i=0, j=0, idx=0, ret, data_cnt;
	u8 cmd;
	chr[2] = '\0';

	if (ec_chip->ec_in_s3){
		asuspec_send_ec_req();
		msleep(200);
	}

	memset(&ec_chip->i2c_dm_data, 0, 32);

	printk("buf_len=%d, data_len=%d \n",buf_len, data_len);

	if(!(buf_len&0x01) || !data_len){
		return -1;
	} else if(buf_len==3){
		reg_addr = (u8) simple_strtoul (buf,NULL,16);
		return EAGAIN;
	}
	for(i=0;i<buf_len-1;i++){
		chr[j] = *(buf+i);
		if(j==1){
			if (i == 1) {
				cmd = (u8) simple_strtoul (chr,NULL,16);
			} else
				data_num[idx++] = (u8) simple_strtoul (chr,NULL,16);
		}
		j++;
		if(j>1){
			j=0;
		}
	}
	data_num[idx] = '\0';
	data_cnt = data_len - 1;

	if(data_cnt > 32) {
		printk("Input data count is over length\n");
		return -1;
	}

	memcpy(&ec_chip->i2c_dm_data[1], data_num, data_cnt);
	ec_chip->i2c_dm_data[0] = data_len - 1;

	for(i=0; i<data_len; i++){
		if (i ==0) printk("I2c cmd=0x%x\n", cmd);
		printk("I2c_dm_data[%d]=0x%x\n",i, ec_chip->i2c_dm_data[i]);
	}
	ret = asuspec_dockram_write_data(cmd, data_len);
	if(ret <0)
		ASUSPEC_NOTICE("Fail to write data\n");
	return count;
}

static ssize_t asuspec_return_data_show(struct device *class,struct device_attribute *attr,char *buf)
{
	int i, ret_val;
	char temp_buf[64];

	if (reg_addr != -1) {

		if (ec_chip->ec_in_s3){
			asuspec_send_ec_req();
			msleep(200);
		}

		printk("Smbus read EC command=0x%02x\n", reg_addr);
		ret_val = asuspec_dockram_read_data(reg_addr);
		reg_addr = -1;

		if (ret_val < 0)
			return sprintf(buf, "Fail to read ec data\n");
		else{
			if (ec_chip->i2c_dm_data[0]> 32)
				return sprintf(buf, "EC return data length error\n");
			for (i = 1; i <= ec_chip->i2c_dm_data[0] ; i++){
				sprintf(temp_buf, "byte[%d] = 0x%x\n", i, ec_chip->i2c_dm_data[i]);
				strcat(buf, temp_buf);
			}
			return strlen(buf);
		}
	}
	return 0;
}
#endif

static ssize_t asuspec_switch_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "%s\n", ec_chip->ec_version);
}

static ssize_t asuspec_switch_state(struct switch_dev *sdev, char *buf)
{
	if (201) {
		return sprintf(buf, "%s\n", "0");
	}
}

static ssize_t apower_switch_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "%s\n", APOWER_SDEV_NAME);
}

static ssize_t apower_switch_state(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "%d\n", ec_chip->apower_state);
}

static int asuspec_suspend(struct device *dev)
{
	printk("asuspec_suspend+\n");
	del_timer_sync(&ec_chip->asuspec_timer);
	ec_chip->ec_in_s3 = 1;
	printk("asuspec_suspend-\n");
	return 0;
}

static int asuspec_resume(struct device *dev)
{
	printk("asuspec_resume+\n");
	ec_chip->i2c_err_count = 0;
	printk("asuspec_resume-\n");
	return 0;
}

static int asuspec_open(struct inode *inode, struct file *flip){
	ASUSPEC_NOTICE("\n");
	return 0;
}

static int asuspec_release(struct inode *inode, struct file *flip){
	ASUSPEC_NOTICE("\n");
	return 0;
}

static long asuspec_ioctl(struct file *flip,
					unsigned int cmd, unsigned long arg){
	int err = 1;

	if (_IOC_TYPE(cmd) != ASUSPEC_IOC_MAGIC)
	 return -ENOTTY;
	if (_IOC_NR(cmd) > ASUSPEC_IOC_MAXNR)
	return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	 switch (cmd) {
        case ASUSPEC_POLLING_DATA:
			if (arg == ASUSPEC_IOCTL_HEAVY){
				ASUSPEC_NOTICE("heavy polling\n");
				ec_chip->polling_rate = 80;
				queue_delayed_work(asuspec_wq, &asuspec_stress_work, HZ/ec_chip->polling_rate);
			}
			else if (arg == ASUSPEC_IOCTL_NORMAL){
				ASUSPEC_NOTICE("normal polling\n");
				ec_chip->polling_rate = 10;
				queue_delayed_work(asuspec_wq, &asuspec_stress_work, HZ/ec_chip->polling_rate);
			}
			else if  (arg == ASUSPEC_IOCTL_END){
				ASUSPEC_NOTICE("polling end\n");
		    	cancel_delayed_work_sync(&asuspec_stress_work) ;
			}
			else
				return -ENOTTY;
			break;
		case ASUSPEC_FW_UPDATE:
			ASUSPEC_NOTICE("ASUSPEC_FW_UPDATE\n");
			mutex_lock(&ec_chip->state_change_lock);
			asuspec_send_ec_req();
			msleep(200);
			buff_in_ptr = 0;
			buff_out_ptr = 0;
			h2ec_count = 0;
			memset(host_to_ec_buffer, 0, EC_BUFF_LEN);
			memset(ec_to_host_buffer, 0, EC_BUFF_LEN);
			ec_chip->status = 0;
			ec_chip->op_mode = 1;
			wake_lock_timeout(&ec_chip->wake_lock, 3*60*HZ);
			ec_chip->i2c_dm_data[0] = 0x02;
			ec_chip->i2c_dm_data[1] = 0x55;
			ec_chip->i2c_dm_data[2] = 0xAA;
			msleep(2400);
			i2c_smbus_write_i2c_block_data(&dockram_client, 0x40, 3, ec_chip->i2c_dm_data);
			msleep(1000);
			mutex_unlock(&ec_chip->state_change_lock);
			break;
		case ASUSPEC_INIT:
			ASUSPEC_NOTICE("ASUSPEC_INIT\n");
			msleep(500);
			ec_chip->status = 0;
			ec_chip->op_mode = 0;
			queue_delayed_work(asuspec_wq, &ec_chip->asuspec_init_work, 0);
			switch_set_state(&ec_chip->pad_sdev, !ec_chip->pad_sdev.state);
			msleep(2500);
			break;
		case ASUSPEC_FW_DUMMY:
			ASUSPEC_NOTICE("ASUSPEC_FW_DUMMY\n");
			ec_chip->i2c_dm_data[0] = 0x02;
			ec_chip->i2c_dm_data[1] = 0x55;
			ec_chip->i2c_dm_data[2] = 0xAA;
			i2c_smbus_write_i2c_block_data(&dockram_client, 0x40, 3, ec_chip->i2c_dm_data);
			break;
		case ASUSPEC_SWITCH_HDMI:
			ASUSPEC_NOTICE("ASUSPEC_SWITCH_HDMI\n");
			asuspec_switch_hdmi();
			break;
		case ASUSPEC_WIN_SHUTDOWN:
			ASUSPEC_NOTICE("ASUSPEC_WIN_SHUTDOWN\n");
			asuspec_win_shutdown();
			break;
        default: /* redundant, as cmd was checked against MAXNR */
            return -ENOTTY;
	}
    return 0;
}

static void asuspec_switch_apower_state(int state){
	ec_chip->apower_state = state;
	switch_set_state(&ec_chip->apower_sdev, ec_chip->apower_state);
	ec_chip->apower_state = APOWER_IDLE;
	switch_set_state(&ec_chip->apower_sdev, ec_chip->apower_state);
}

static void asuspec_win_shutdown(void){
	int ret_val = 0;
	int i = 0;

	if (ec_chip->ec_in_s3){
		asuspec_send_ec_req();
		msleep(200);
	}

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_read_data(0x0A);
		if (ret_val < 0){
			ASUSPEC_ERR("fail to get control flag\n");
			msleep(100);
		}
		else
			break;
	}

	ec_chip->i2c_dm_data[0] = 8;
	ec_chip->i2c_dm_data[8] = ec_chip->i2c_dm_data[8] | 0x40;

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_write_data(0x0A,9);
		if (ret_val < 0){
			ASUSPEC_ERR("Win shutdown command fail\n");
			msleep(100);
		}
		else {
			ASUSPEC_NOTICE("Win shutdown\n");
			break;
		}
	}
}

static void asuspec_switch_hdmi(void){
	int ret_val = 0;
	int i = 0;

	if (ec_chip->ec_in_s3){
		asuspec_send_ec_req();
		msleep(200);
	}

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_read_data(0x0A);
		if (ret_val < 0){
			ASUSPEC_ERR("fail to get control flag\n");
			msleep(100);
		}
		else
			break;
	}

	ec_chip->i2c_dm_data[0] = 8;
	ec_chip->i2c_dm_data[8] = ec_chip->i2c_dm_data[8] | 0x01;

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_write_data(0x0A,9);
		if (ret_val < 0){
			ASUSPEC_ERR("Switch hdmi command fail\n");
			msleep(100);
		}
		else {
			ASUSPEC_NOTICE("Switching hdmi\n");
			break;
		}
	}

}

#if 0
static void asuspec_storage_info_update(void){
	int ret_val = 0;
	int i = 0;
	struct kstatfs st_fs;
	unsigned long long block_size;
	unsigned long long f_blocks;
	unsigned long long f_bavail;
	unsigned long long mb;

	ret_val = user_statfs("/data", &st_fs);
	if (ret_val < 0){
		ASUSPEC_ERR("fail to get data partition size\n");
		ec_chip->storage_total = 0;
		ec_chip->storage_avail = 0;
	} else {
		block_size = st_fs.f_bsize;
		f_blocks = st_fs.f_blocks;
		f_bavail = st_fs.f_bavail;
		mb = MB;
		ec_chip->storage_total = block_size * f_blocks / mb;
		ec_chip->storage_avail = block_size * f_bavail / mb;
		ASUSPEC_NOTICE("Storage total size = %ld, available size = %ld\n", ec_chip->storage_total, ec_chip->storage_avail);
	}

	if (ec_chip->ec_in_s3){
		asuspec_send_ec_req();
		msleep(200);
	}

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_read_storageinfo(0x28);
		if (ret_val < 0){
			ASUSPEC_ERR("fail to get PadInfo\n");
			msleep(100);
		}
		else
			break;
	}

	ec_chip->i2c_dm_storage[0] = 8;
	ec_chip->i2c_dm_storage[1] = ec_chip->storage_total & 0xFF;
	ec_chip->i2c_dm_storage[2] = (ec_chip->storage_total >> 8) & 0xFF;
	ec_chip->i2c_dm_storage[3] = ec_chip->storage_avail & 0xFF;;
	ec_chip->i2c_dm_storage[4] = (ec_chip->storage_avail >> 8) & 0xFF;;

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_write_storageinfo(0x28,9);
		if (ret_val < 0){
			ASUSPEC_ERR("fail to write PadInfo\n");
			msleep(100);
		}
		else {
			ASUSPEC_NOTICE("Write PadInof Total[H][L]: 0x%x, 0x%x\n", ec_chip->i2c_dm_storage[2], ec_chip->i2c_dm_storage[1]);
			ASUSPEC_NOTICE("Write PadInof Avail[H][L]: 0x%x, 0x%x\n", ec_chip->i2c_dm_storage[4], ec_chip->i2c_dm_storage[3]);
			break;
		}
	}
}
#endif

static void asuspec_enter_normal_mode(void){

	int ret_val = 0;
	int i = 0;

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_read_data(0x0A);
		if (ret_val < 0){
			ASUSPEC_ERR("fail to get control flag\n");
			msleep(100);
		}
		else
			break;
	}

	ec_chip->i2c_dm_data[0] = 8;
	ec_chip->i2c_dm_data[5] = ec_chip->i2c_dm_data[5] & 0xBF;

	for ( i = 0; i < 3; i++ ){
		ret_val = asuspec_dockram_write_data(0x0A,9);
		if (ret_val < 0){
			ASUSPEC_ERR("Entering normal mode fail\n");
			msleep(100);
		}
		else {
			ASUSPEC_NOTICE("Entering normal mode\n");
			break;
		}
	}
}

static int BuffDataSize(void)
{
    int in = buff_in_ptr;
    int out = buff_out_ptr;

    if (in >= out){
        return (in - out);
    } else {
        return ((EC_BUFF_LEN - out) + in);
    }
}

static void BuffPush(char data)
{

    if (BuffDataSize() >= (EC_BUFF_LEN -1)){
        ASUSPEC_ERR("Error: EC work-buf overflow \n");
        return;
    }

    ec_to_host_buffer[buff_in_ptr] = data;
    buff_in_ptr++;
    if (buff_in_ptr >= EC_BUFF_LEN){
        buff_in_ptr = 0;
    }
}

static char BuffGet(void)
{
    char c = (char)0;

    if (BuffDataSize() != 0){
        c = (char) ec_to_host_buffer[buff_out_ptr];
        buff_out_ptr++;
         if (buff_out_ptr >= EC_BUFF_LEN){
             buff_out_ptr = 0;
         }
    }
    return c;
}

static ssize_t ec_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int i = 0;
    int ret;
    char tmp_buf[EC_BUFF_LEN];
	static int f_counter = 0;
	static int total_buf = 0;

	mutex_lock(&ec_chip->lock);
	mutex_unlock(&ec_chip->lock);

    while ((BuffDataSize() > 0) && count)
    {
        tmp_buf[i] = BuffGet();
		ASUSPEC_INFO("tmp_buf[%d] = 0x%x, total_buf = %d\n", i, tmp_buf[i], total_buf);
        count--;
        i++;
		f_counter = 0;
		total_buf++;
    }

    ret = copy_to_user(buf, tmp_buf, i);
    if (ret == 0)
    {
        ret = i;
    }

    return ret;
}

static ssize_t ec_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int err;
    int i;

    if (h2ec_count > 0)
    {                   /* There is still data in the buffer that */
        return -EBUSY;  /* was not sent to the EC */
    }
    if (count > EC_BUFF_LEN)
    {
        return -EINVAL; /* data size is too big */
    }

    err = copy_from_user(host_to_ec_buffer, buf, count);
    if (err)
    {
        ASUSPEC_ERR("ec_write copy error\n");
        return err;
    }

    h2ec_count = count;
    for (i = 0; i < count ; i++){
		i2c_smbus_write_byte_data(&dockram_client, host_to_ec_buffer[i],0);
    }
    h2ec_count = 0;
    return count;

}

static int __init asuspec_init(void)
{
	int err_code = 0;

	printk(KERN_INFO "%s+ #####\n", __func__);

	if (asuspec_major) {
		asuspec_dev = MKDEV(asuspec_major, asuspec_minor);
		err_code = register_chrdev_region(asuspec_dev, 1, "asuspec");
	} else {
		err_code = alloc_chrdev_region(&asuspec_dev, asuspec_minor, 1,"asuspec");
		asuspec_major = MAJOR(asuspec_dev);
	}

	ASUSPEC_NOTICE("cdev_alloc\n") ;
	asuspec_cdev = cdev_alloc() ;
	asuspec_cdev->owner = THIS_MODULE ;
	asuspec_cdev->ops = &asuspec_fops ;

	err_code=i2c_add_driver(&asuspec_driver);
	if(err_code){
		ASUSPEC_ERR("i2c_add_driver fail\n") ;
		goto i2c_add_driver_fail ;
	}
	asuspec_class = class_create(THIS_MODULE, "asuspec");
	if(asuspec_class <= 0){
		ASUSPEC_ERR("asuspec_class create fail\n");
		err_code = -1;
		goto class_create_fail ;
	}
	asuspec_device = device_create(asuspec_class, NULL, MKDEV(asuspec_major, asuspec_minor), NULL, "asuspec" );
	if(asuspec_device <= 0){
		ASUSPEC_ERR("asuspec_device create fail\n");
		err_code = -1;
		goto device_create_fail ;
	}

	ASUSPEC_INFO("return value %d\n", err_code) ;
	printk(KERN_INFO "%s- #####\n", __func__);

	return 0;

device_create_fail :
	class_destroy(asuspec_class) ;
class_create_fail :
	i2c_del_driver(&asuspec_driver);
i2c_add_driver_fail :
	printk(KERN_INFO "%s- #####\n", __func__);
	return err_code;

}

static void __exit asuspec_exit(void)
{
	device_destroy(asuspec_class,MKDEV(asuspec_major, asuspec_minor)) ;
	class_destroy(asuspec_class) ;
	i2c_del_driver(&asuspec_driver);
	unregister_chrdev_region(asuspec_dev, 1);
	switch_dev_unregister(&ec_chip->pad_sdev);
	switch_dev_unregister(&ec_chip->apower_sdev);
}

module_init(asuspec_init);
module_exit(asuspec_exit);
