/* drivers/input/misc/jsa1127.c
 *
 * Copyright (c) 2011 SOLTEAM.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

#include <linux/jsa1127.h>

#ifdef CONFIG_MACH_CL2N
#include <mach/board-cl2n-misc.h>
#endif

/* debug level */
#define DEBUG_MASK 0
#if DEBUG_MASK

#define DBG(fmt, ...)  do { \
							printk(KERN_DEBUG "[JSA1127_D] %s(%d): " fmt "\n",\
							__FUNCTION__, __LINE__, ## __VA_ARGS__); \
						} while(0)
#else
#define DBG(fmt, ...)
#endif

#define INFO(fmt, ...)  do { \
							printk(KERN_INFO "[JSA1127_I] %s(%d): " fmt "\n",\
							__FUNCTION__, __LINE__, ## __VA_ARGS__); \
						} while(0)

#define ERR(fmt, ...)  do { \
							printk(KERN_ERR "[JSA1127_E] %s(%d): " fmt "\n",\
							__FUNCTION__, __LINE__, ## __VA_ARGS__); \
						} while(0)

#define I2C_RETRY_TIME 10
#define PRODUCT_NAME "jsa1127"

#define MAX_DELAY  2000 /* mSec */
#define MIN_DELAY  200  /* mSec */
#define delay_to_jiffies(d) ((d)?msecs_to_jiffies(d):1)

/* Kernel has no float point, it would convert it by this*/
/* RINT = 100, specification page 12, lux/count = 1.67 */
#define DEFAULT_RESOLUTION_R100K	1670 //evt/dvt
#define DEFAULT_RESOLUTION_R800K	210 //dvt2
#define BASE_VALUE			1000
#define INVALID_COUNT	0xFFFFFFFF

/* jsa1127 driver data struct */
struct jsa1127_drv_data
{
	struct delayed_work work;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct jsa1127_platform_data *pdev;
	struct mutex mutex;
	unsigned int delay;
	unsigned long lux;
	unsigned int enabled;
	unsigned long resolution;
	unsigned long compensate_rate;
	unsigned int first_boot;
};

static struct jsa1127_drv_data jsa1127_data = {
	.delay = MAX_DELAY,
	.lux = 0,
	.enabled = 0,
	.resolution = DEFAULT_RESOLUTION_R800K,
	.compensate_rate = 50,
	.first_boot = 1,
};

/* CMD definition */
#define CMD_SHUTDOWN_MODE			0x8C
#define CMD_ACTIVATE_MODE			0x0C
#define CMD_ACTIVATE_MODE_ONE_TIME		0x04
#define CMD_START_INTEGRATION_ONE_TIME		0x08
#define CMD_STOP_INTEGRATION_ONE_TIME		0x30

static int jsa1127_cmd_send(unsigned char cmd)
{
	int ret = -EIO;
	unsigned char wbuf[5];
	int retry = I2C_RETRY_TIME;

	wbuf[0] = cmd;

	printk("jsa1127_cmd_send now !\n");

	for (; retry > 0; retry --)
	{
		ret = i2c_master_send(jsa1127_data.client, wbuf, 1);
		if (ret < 0)
		{
			ERR("write cmd[0x%2X] failed!, retry(%d)",
				cmd, (I2C_RETRY_TIME - retry));
		}
		else
		{
			DBG("write cmd[0x%2X] success!", cmd);
			break;
		}
	}

	return ret;
}

/* Using internal integration timing to read lux value */
static unsigned long jsa1127_read_lux(void)
{
	int ret = -EIO;
	int retry = I2C_RETRY_TIME;
	unsigned char rbuf[5];
	unsigned long lux = INVALID_COUNT;

	/* start to read data of lux */
	retry = I2C_RETRY_TIME;
	for(; retry > 0; retry --)
	{
		ret = i2c_master_recv(jsa1127_data.client, rbuf, 2);
		if (ret < 0)
		{
			ERR("read failed!, retry(%d)", (I2C_RETRY_TIME - retry));
		}
		else
		{
			DBG("read success!");
			break;
		}
	}

	if (ret > 0)
	{
		if (rbuf[1] && 0x80)
		{
			lux = (unsigned long)((((int)rbuf[1]&0x7F) << 8) | (int)rbuf[0]);
			DBG("lux value is valid!, count = %ld", lux);
		}
		else
		{
			lux = INVALID_COUNT;
			INFO("lux value is invalid!");
		}
	}

	return lux;
}


/* Initial chips stage, check chip connect with board is fine,
 * using external integration timing */
static int initial_jsa1127_chip(void)
{
	int ret = 0;

	ret = jsa1127_cmd_send(CMD_ACTIVATE_MODE);
	if (ret < 0)
	{
		ERR("Send CMD activiate one time failed!");
		ret = -EIO;
		goto i2c_err;
	}

#if 0
	ret = jsa1127_cmd_send(CMD_ACTIVATE_MODE_ONE_TIME);
	if (ret < 0)
	{
		ERR("Send CMD activiate one time failed!");
		ret = -EIO;
		goto i2c_err;
	}

	ret = jsa1127_cmd_send(CMD_START_INTEGRATION_ONE_TIME);
	if (ret < 0)
	{
		ERR("Send CMD start command failed!");
		ret =  -EIO;
		goto i2c_err;
	}

	ret = jsa1127_cmd_send(CMD_STOP_INTEGRATION_ONE_TIME);
	if (ret < 0)
	{
		ERR("Send CMD stop command failed!");
		ret =  -EIO;
		goto i2c_err;
	}

	ret = jsa1127_cmd_send(CMD_SHUTDOWN_MODE);
	if (ret < 0)
	{
		ERR("Send CMD shutdown failed!");
		ret =  -EIO;
		goto i2c_err;
	}
#endif

	if (ret > 0)
		DBG("initial chip success!");

i2c_err:
	return ret;
}

/* allocate input event to pass to user space*/
static int jsa1127_input_register(void)
{
	int ret = 0;

	/* Allocate Input Device */
	if (!jsa1127_data.input_dev)
		jsa1127_data.input_dev = input_allocate_device();

	if (!jsa1127_data.input_dev)
	{
		ERR("Allocate input device failed.");
		ret = -ENOMEM;
		return ret;
	}

	jsa1127_data.input_dev->name = "jsa1127_als";
	jsa1127_data.input_dev->id.bustype = BUS_I2C;
	set_bit(EV_ABS, jsa1127_data.input_dev->evbit);
	input_set_capability(jsa1127_data.input_dev, EV_ABS, ABS_X);

	/* Register Input Device */
	ret = input_register_device(jsa1127_data.input_dev);
	if (ret)
	{
		ERR("Register input device failed.");
		input_free_device(jsa1127_data.input_dev);
		ret = -EIO;
	}

	return ret;
}

/* work queue for update current light senosr lux*/
static void jsa1127_work_func(struct work_struct *work)
{
	unsigned long delay = delay_to_jiffies(jsa1127_data.delay);
	unsigned long lux;

	lux = jsa1127_read_lux();
	if (lux != INVALID_COUNT)
	{
		lux = (lux * jsa1127_data.resolution * jsa1127_data.compensate_rate) / BASE_VALUE;
		jsa1127_data.lux = lux;
		DBG("udpdate lux: %ld", lux);
	}
	else
	{
		DBG("report the prevous lux value!");
		lux = jsa1127_data.lux;
	}

//	if (!jsa1127_data.input_dev)
	if (jsa1127_data.input_dev)
	{
		DBG("report lux input abs_x event (%ld)", lux);
		input_report_abs(jsa1127_data.input_dev, ABS_X, lux);
		input_sync(jsa1127_data.input_dev);
	}

	if (jsa1127_data.enabled)
	{
		DBG("still scheduling light sensor workqueue!");
		schedule_delayed_work(&jsa1127_data.work, msecs_to_jiffies(delay+1));
	}
}

static ssize_t sensor_data_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	DBG("lux: %ld", jsa1127_data.lux);
	return snprintf(buf, sizeof(buf), "%ld\n", jsa1127_data.lux);
}

static ssize_t sensor_delay_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	DBG("delay: %d", jsa1127_data.delay);
	return snprintf(buf, sizeof(buf), "%d\n", jsa1127_data.delay);
}

static ssize_t sensor_delay_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = simple_strtoul(buf, NULL, 10);

	if (value < MIN_DELAY)
		value = MIN_DELAY;

	if (value >= MAX_DELAY)
		value = MAX_DELAY;

	mutex_lock(&jsa1127_data.mutex);
	jsa1127_data.delay = value;
	mutex_unlock(&jsa1127_data.mutex);

	DBG("set delay time as %d ms", jsa1127_data.delay);

	return count;
}

static ssize_t sensor_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	DBG("enabled: %d", jsa1127_data.enabled);
	return snprintf(buf, sizeof(buf), "%d\n", jsa1127_data.enabled);
}

#ifdef CONFIG_MACH_CL2N
static int cl2n_get_attribut(const char *filename,  unsigned char *buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	loff_t pos;
	mm_segment_t old_fs;
	ssize_t			nread;

	pfile = filp_open(filename, O_RDONLY, 0);

	if (IS_ERR(pfile)) {
		pr_err("[%s]error occured while opening file %s.\n", __FUNCTION__,filename);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	nread = vfs_read(pfile, buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);
	buf[nread-1] = '\0';

	return 0;
}
#endif

static ssize_t sensor_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = simple_strtoul(buf, NULL, 10);
	unsigned long delay = delay_to_jiffies(jsa1127_data.delay);

#ifdef CONFIG_MACH_CL2N
	if(jsa1127_data.first_boot)
	{
		/* Check board ID*/
		int cl2n_board_id = cl2n_get_board_strap();
		printk("cl2n_board_id = %d\n", cl2n_board_id);

		if(cl2n_board_id==CL2N_BOARD_VER_A00)//EVT
		{
			jsa1127_data.compensate_rate = 50;
			jsa1127_data.resolution = DEFAULT_RESOLUTION_R100K ;
			printk("(%s)Check board ID:EVT \n",__FUNCTION__);
		}
		else if(cl2n_board_id==CL2N_BOARD_VER_B00)//DVT1/DVT2
		{
			char buf[10];
			cl2n_get_attribut("/sys/block/mmcblk0/device/name",buf);

			if(!strcmp(buf,"SEM16G"))
			{
				//DVT1
				jsa1127_data.compensate_rate = 50;
				jsa1127_data.resolution = DEFAULT_RESOLUTION_R100K ;
				printk("(%s)Check board ID:DVT1, %s \n",__FUNCTION__,buf);
			}
			else
			{
				//DVT2
				jsa1127_data.compensate_rate = 40;
				jsa1127_data.resolution = DEFAULT_RESOLUTION_R800K ;
				printk("(%s)Check board ID:DVT2, %s \n",__FUNCTION__,buf);
			}
		}
		else//PVT/MP
		{
			jsa1127_data.compensate_rate = 40;
			jsa1127_data.resolution = DEFAULT_RESOLUTION_R800K ;
			printk("(%s)Check board ID:PVT/MP \n",__FUNCTION__);
		}

		jsa1127_data.first_boot = 0;
	}
#endif

	mutex_lock(&jsa1127_data.mutex);

	if (value == 1 && jsa1127_data.enabled == 0)
	{
		if (jsa1127_cmd_send(CMD_ACTIVATE_MODE) < 0) //enable light sensor
		{
			ERR("enable jsa1127 failed!");
			return 0;
		}
		else
		{
			INFO("enable light sensor success");
			jsa1127_data.enabled = 1;
			schedule_delayed_work(&jsa1127_data.work, msecs_to_jiffies(delay+1));
		}
	}
	else if (jsa1127_data.enabled == 1 && value == 0)
	{
		if (jsa1127_cmd_send(CMD_SHUTDOWN_MODE) < 0) //disable light sensor
		{
			ERR("disable jsa1127 failed!");
		}

		jsa1127_data.enabled = 0;
		cancel_delayed_work_sync(&jsa1127_data.work);
	}
	else
	{
		//do nothing
		INFO("Do nothing at this operation time!");
	}

	mutex_unlock(&jsa1127_data.mutex);

	DBG("set enabled as %d", jsa1127_data.enabled);

	return count;
}

static ssize_t sensor_resolution_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	DBG("resolution: %ld", jsa1127_data.resolution);
	return snprintf(buf, sizeof(buf), "%ld\n", jsa1127_data.resolution);
}

static ssize_t sensor_resolution_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = simple_strtoul(buf, NULL, 10);

	if (value < 0)
		return 0;

	mutex_lock(&jsa1127_data.mutex);
	jsa1127_data.resolution = (unsigned long) value;
	mutex_unlock(&jsa1127_data.mutex);

	DBG("set resolution as %ld", jsa1127_data.resolution);

	return count;
}

///sys/class/input/input4/test
static ssize_t sensor_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = simple_strtoul(buf, NULL, 10);
	int ret = 0;
	unsigned long lux;

  switch(value)
  {
	case 0:
				ret = jsa1127_cmd_send(CMD_ACTIVATE_MODE_ONE_TIME);
				if (ret < 0)
				{
					printk("Send CMD activiate one time failed!");
				}
		break;
	case 1:
				ret = jsa1127_cmd_send(CMD_START_INTEGRATION_ONE_TIME);
				if (ret < 0)
				{
					printk("Send CMD start command failed!");
				}
		break;
	case 2:
				ret = jsa1127_cmd_send(CMD_STOP_INTEGRATION_ONE_TIME);
				if (ret < 0)
				{
					printk("Send CMD stop command failed!");
				}
		break;
	case 3:
				ret = jsa1127_cmd_send(CMD_SHUTDOWN_MODE);
				if (ret < 0)
				{
					printk("Send CMD shutdown failed!");
				}
		break;
	case 4:
				lux = jsa1127_read_lux();
				if (lux != INVALID_COUNT)
				{
					lux = (lux * jsa1127_data.resolution) / BASE_VALUE;
					printk("jsa1127_read_lux lux: %ld", lux);
				}
				else
				{
					printk("Send CMD shutdown failed!");
				}

		break;
	default:
		break;
  }

	return count;
}

static ssize_t sensor_compensate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	DBG("compensate: %d", jsa1127_data.compensate_rate);
	return snprintf(buf, sizeof(buf), "%lu\n", jsa1127_data.compensate_rate);
}

static ssize_t sensor_compensate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = simple_strtoul(buf, NULL, 10);

	mutex_lock(&jsa1127_data.mutex);
	jsa1127_data.compensate_rate = value;
	mutex_unlock(&jsa1127_data.mutex);

	DBG("set delay time as %lu ms", jsa1127_data.compensate_rate);

	return count;
}

static struct device_attribute sensor_dev_attr_data = __ATTR(data, S_IRUGO,
	sensor_data_show, NULL);
static struct device_attribute sensor_dev_attr_delay = __ATTR(delay,
	S_IRUGO|S_IWUSR|S_IWGRP,
	sensor_delay_show, sensor_delay_store);
static struct device_attribute sensor_dev_attr_enable = __ATTR(enable,
	S_IRUGO|S_IWUSR|S_IWGRP,
	sensor_enable_show, sensor_enable_store);
static struct device_attribute sensor_dev_attr_resolution = __ATTR(resolution,
	S_IRUGO|S_IWUSR|S_IWGRP,
	sensor_resolution_show, sensor_resolution_store);

static struct device_attribute sensor_dev_attr_test = __ATTR(test,
	S_IRUGO|S_IWUSR|S_IWGRP,
	NULL, sensor_test_store);
static struct device_attribute sensor_dev_attr_compensate = __ATTR(compensate,
	S_IRUGO|S_IWUSR|S_IWGRP,
	sensor_compensate_show, sensor_compensate_store);

static struct attribute *sensor_attributes[] = {
	&sensor_dev_attr_data.attr,
	&sensor_dev_attr_delay.attr,
	&sensor_dev_attr_enable.attr,
	&sensor_dev_attr_resolution.attr,
	&sensor_dev_attr_test.attr,
	&sensor_dev_attr_compensate.attr,
	NULL
};

static struct attribute_group sensor_attribute_group = {
	.attrs = sensor_attributes
};

static int __devinit jsa1127_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;

	/* Chcek I2C */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		ERR("Check I2C functionality failed.");
		ret = -EIO;
		goto err_check_functionality_failed;
	}

	/* Get driver data and i2c client */
	jsa1127_data.client = client;
	jsa1127_data.pdev = (struct jsa1127_platform_data *) client->dev.platform_data;
	if (!jsa1127_data.pdev)
	{
		ERR("Please check platform data!");
		ret = -EIO;
		goto err_platform_data;
	}

	/* initial platform configuration*/
	if (!jsa1127_data.pdev && !jsa1127_data.pdev->configure_platform)
	{
		INFO("initalize platform setting!");
		jsa1127_data.pdev->configure_platform();
	}

	/* Initial jsa1127 chip and check connect with i2c bus */

	ret = initial_jsa1127_chip();
	if (ret < 0)
	{
		ERR("initial chip error!");
		goto err_init_chip;
	}

	/* register input device and delay work queue for evnet polling */
	ret = jsa1127_input_register();
	if (ret < 0 )
	{
		ERR("jsa1127 input register error!");
		goto err_input_failed;
	}

	/* initial delay workqueue */
	mutex_init(&jsa1127_data.mutex);
	jsa1127_data.delay = MAX_DELAY;
	jsa1127_data.first_boot = 1;
	INIT_DELAYED_WORK(&jsa1127_data.work, jsa1127_work_func);
	schedule_delayed_work(&jsa1127_data.work, msecs_to_jiffies(MAX_DELAY));

	/* setup the attr for control by others */
	ret = sysfs_create_group(&jsa1127_data.input_dev->dev.kobj,
			&sensor_attribute_group);
	if (ret < 0)
	{
		ERR("register attributes failed!");
		goto err_sysfs_register;
	}

	INFO("Probe Done.");
	return 0;

err_sysfs_register:
err_input_failed:
err_init_chip:
err_platform_data:
err_check_functionality_failed:

	return ret;
}

static int jsa1127_remove(struct i2c_client *client)
{
	cancel_delayed_work_sync(&jsa1127_data.work);
	sysfs_remove_group(&jsa1127_data.input_dev->dev.kobj,
		&sensor_attribute_group);
	input_unregister_device(jsa1127_data.input_dev);
	input_free_device(jsa1127_data.input_dev);

	INFO("Remove JSA1127 Module Done.");
	return 0;
}

static const struct i2c_device_id jsa1127_id[] = {
	{PRODUCT_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, jsa1127_id);

static struct i2c_driver jsa1127_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = PRODUCT_NAME,
	},
	.id_table  = jsa1127_id,
	.probe     = jsa1127_probe,
	.remove    = jsa1127_remove,
};

/* driver initial part */
static int __init jsa1127_init(void)
{
	INFO("register jsa1127 i2c driver!");
	return i2c_add_driver(&jsa1127_driver);
}
//module_init(jsa1127_init);
late_initcall(jsa1127_init);

static void __exit jsa1127_exit(void)
{
	INFO("deregister jsa1127 i2c driver!");
	i2c_del_driver(&jsa1127_driver);
}
module_exit(jsa1127_exit);

MODULE_AUTHOR("<Solteam Corp.>");
MODULE_DESCRIPTION("Solteam JSA1127 Ambient Light Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.8");
