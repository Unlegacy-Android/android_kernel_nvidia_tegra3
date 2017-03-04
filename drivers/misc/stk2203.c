/*
 *  stk2203.c - Linux kernel modules for stk220x ambient light sensor with interrupt
 *
 *  Copyright (C) 2011 Patrick Chang / SenseTek <patrick_chang@sitronix.com.tw>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <linux/module.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/stk_lk_defs.h>
#include <linux/stk_i2c_als_220xgeneric.h>
#include "../../arch/arm/mach-tegra/board-acer-t30.h"
#include <linux/i2c/at24.h>

extern int acer_board_id;
extern int acer_board_type;

#define STK_DRIVER_VER	"1.6.2"
#define ALS_NAME	"lightsensor-level"
#define DEFAULT_ALS_TRANSMITTANCE	1000

#define STKALS_DRV_NAME	"stk_als"
#define DEVICE_NAME		"stk-oss"

#define stk_writeb(x,y) i2c_smbus_write_byte_data(pStkAlsData->client,x,y)
#define stk_readb(x) i2c_smbus_read_byte_data(pStkAlsData->client,x)
#define ABS(x) ((x)>=0? (x):(-x))
#define STK_LOCK0 mutex_unlock(&stkals_io_lock)
#define STK_LOCK1 mutex_lock(&stkals_io_lock)

static int32_t init_all_setting(void);
static int32_t get_lux(void);
static int32_t set_it(uint32_t it);
static int32_t set_gain(uint32_t gain);
static int32_t enable_als(uint32_t enable);
static int32_t set_power_state(uint32_t nShutdown);

static struct mutex stkals_io_lock;
struct stkals_data* pStkAlsData = NULL;
static struct workqueue_struct *stk_oss_work_queue = NULL;
static int32_t als_transmittance = DEFAULT_ALS_TRANSMITTANCE;
static char buff[LENGTH_L_SENSOR];

inline void report_event(struct input_dev* dev,int32_t report_value)
{
	input_report_abs(dev, ABS_MISC, report_value);
	input_sync(dev);
	INFO("STK ALS : als input event %d lux\n",report_value);
}

inline int32_t get_reading(void)
{
	return STK_ALS_DATA(stk_readb(STK_ALS_DT1_REG),stk_readb(STK_ALS_DT2_REG));
}

inline int32_t alscode2lux(int32_t alscode)
{
	if (acer_board_type == BOARD_PICASSO_M) {
		if (acer_board_id == BOARD_PVT1) {
			if ((buff[1]-48) == 1) {
				if ((buff[6]-48) >= 2 && (buff[7]-48) >= 4) {
					alscode<<=11;
				} else if ((buff[6]-48) == 0 && (buff[7]-48) <= 5) {
					alscode<<=9;
				} else {
					alscode<<=10;
				}
			} else {
				alscode<<=10;
			}
		} else {
			alscode<<=9;
		}
	} else if (acer_board_type == BOARD_PICASSO_MF) {
		if ((buff[1]-48) == 1 && (buff[6]-48) >= 2 && (buff[7]-48) >= 4) {
			alscode<<=11;
		} else if ((buff[1]-48) == 1 && (buff[6]-48) == 0 && (buff[7]-48) <= 5) {
			alscode<<=9;
		} else {
			alscode<<=10;
		}
	} else {
		alscode<<=10;
	}
	return alscode/als_transmittance;
}

static int32_t get_lux()
{
	return  alscode2lux(get_reading());
}

static int32_t set_it(uint32_t it)
{
	int32_t val;

	val = stk_readb(STK_ALS_CMD_REG);
	val &= (~STK_ALS_CMD_IT_MASK);
	val |= STK_ALS_CMD_IT(it);

	return stk_writeb(STK_ALS_CMD_REG,val);
}

static int32_t set_gain(uint32_t gain)
{
	int32_t val;

	val = stk_readb(STK_ALS_CMD_REG);
	val &= (~STK_ALS_CMD_GAIN_MASK);
	val |= STK_ALS_CMD_GAIN(gain);

	return stk_writeb(STK_ALS_CMD_REG,val);
}

static int32_t set_power_state(uint32_t nShutdown)
{
	int32_t val;

	val = stk_readb(STK_ALS_CMD_REG);
	val &= (~STK_ALS_CMD_SD_MASK);
	val |= STK_ALS_CMD_SD(nShutdown);

	return stk_writeb(STK_ALS_CMD_REG,val);
}


static int32_t enable_als(uint32_t enable)
{
	int32_t ret;

	pStkAlsData->enable_als_irq = 0;

	ret = set_power_state(enable?0:1);
	if (enable) {
		pStkAlsData->enable_als_irq = 1;
		enable_irq(pStkAlsData->irq);
	} else {
		disable_irq(pStkAlsData->irq);
	}

	return ret;
}


static int32_t init_all_setting()
{
	uint32_t val;

	enable_als(0);
	if (acer_board_type == BOARD_PICASSO_M) {
		if (acer_board_id == BOARD_PVT1) {
			if ((buff[1]-48) == 1) {
				if ((buff[6]-48) >= 2 && (buff[7]-48) >= 4) {
					set_gain(0);
				} else if ((buff[6]-48) == 0 && (buff[7]-48) <= 5) {
					set_gain(2);
				} else {
					set_gain(1);
				}
			} else {
				set_gain(1);
			}
		} else {
			set_gain(2);
		}
	} else if (acer_board_type == BOARD_PICASSO_MF) {
		if ((buff[1]-48) == 1 && (buff[6]-48) >= 2 && (buff[7]-48) >= 4) {
			set_gain(0);
		} else if ((buff[1]-48) == 1 && (buff[6]-48) == 0 && (buff[7]-48) <= 5) {
			set_gain(2);
		} else {
			set_gain(1);
		}
	} else {
		set_gain(1);
	}
	set_it(1);
	val = stk_readb(STK_ALS_CMD_REG);
	INFO("Init ALS Setting --> CMDREG = 0x%x\n",val);
	pStkAlsData->enable_als_irq = 1;

	val = stk_readb(STK_ALS_INT_REG);
	INFO("Init ALS Setting --> INTREG = 0x%x\n",val);

	if (val & STK_ALS_INT_FLAG_MASK) {
		val &= (~STK_ALS_INT_FLAG_MASK);
		stk_writeb(STK_ALS_INT_REG,val);
	}

	val = stk_readb(STK_ALS_INT_REG);
	val &= (~STK_ALS_INT_THD_MASK);
	val &= (~STK_ALS_INT_PRST_MASK);
	stk_writeb(STK_ALS_INT_REG,val);

	return 0;
}

static ssize_t lux_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	return sprintf(buf, "%d\n", alscode2lux((1<<12) -1));

}


static ssize_t als_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	int32_t enable;

	STK_LOCK(1);
	enable = pStkAlsData->enable_als_irq;
	STK_LOCK(0);

	return sprintf(buf, "%d\n", enable);
}


static ssize_t als_enable_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t len)
{
	uint32_t value = simple_strtoul(buf, NULL, 10);

	INFO("STK ALS Driver : Enable ALS : %d\n",value);
	STK_LOCK(1);
	enable_als(value);
	STK_LOCK(0);

	return len;
}


static ssize_t lux_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	int32_t lux;

	STK_LOCK(1);
	lux = pStkAlsData->als_lux_last;
	STK_LOCK(0);

	return sprintf(buf, "%d lux\n", lux);
}
static ssize_t lux_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t len)
{
	unsigned long value = simple_strtoul(buf, NULL, 10);

	STK_LOCK(1);
	report_event(pStkAlsData->input_dev,value);
	STK_LOCK(0);

	return len;
}
static ssize_t lux_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	return sprintf(buf, "1\n");
}

ssize_t stk_bin_sysfs_read(als_lux_range)
{
	uint32_t* pDst = (uint32_t*)buffer;

	*pDst = alscode2lux((1<<12)-1);

	return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(als_lux_resolution)
{
	uint32_t* pDst = (uint32_t*)buffer;

	*pDst = 1;

	return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(lux_bin)
{

	int32_t *pDst = (int32_t*)buffer;

	STK_LOCK(1);
	*pDst = pStkAlsData->als_lux_last;
	STK_LOCK(0);

	return sizeof(uint32_t);
}


ssize_t stk_bin_sysfs_read(als_enable)
{
	STK_LOCK(1);
	STK_LOCK(0);

	return sizeof(uint8_t);
}

ssize_t  stk_bin_sysfs_write(als_enable)
{
	STK_LOCK(1);
	enable_als(buffer[0]);
	STK_LOCK(0);

	return count;
}


ssize_t stk_bin_sysfs_read(als_min_delay)
{
	*((uint32_t*)buffer) = ALS_MIN_DELAY;

	return sizeof(uint32_t);
}

static struct kobj_attribute lux_range_attribute = (struct kobj_attribute)__ATTR_RO(lux_range);
static struct kobj_attribute lux_attribute = (struct kobj_attribute)__ATTR(lux,0644,lux_show,lux_store);
static struct kobj_attribute als_enable_attribute = (struct kobj_attribute)__ATTR(als_enable,0644,als_enable_show,als_enable_store);
static struct kobj_attribute als_lux_res_attribute = (struct kobj_attribute)__ATTR_RO(lux_res);

static struct attribute* sensetek_optical_sensors_attrs [] =
{
	&lux_range_attribute.attr,
	&lux_attribute.attr,
	&als_enable_attribute.attr,
	&als_lux_res_attribute.attr,
	NULL,
};



static struct platform_device *stk_oss_dev = NULL;

static int stk_sysfs_create_files(struct kobject *kobj,struct attribute** attrs)
{
	int err;

	while (*attrs!=NULL) {
		err = sysfs_create_file(kobj,*attrs);
		if (err)
			return err;
		attrs++;
	}

	return 0;
}

static void stk_oss_wq_function(struct work_struct *work)
{
	int32_t lux;
	uint32_t val;

	STK_LOCK(1);
	lux = get_lux();
	pStkAlsData->als_lux_last = lux;
	report_event(pStkAlsData->input_dev,lux);
	STK_LOCK(0);

	if (pStkAlsData->enable_als_irq)
		enable_irq(pStkAlsData->irq);
	else
		disable_irq(pStkAlsData->irq);

	val = stk_readb(STK_ALS_INT_REG);
	val &= (~STK_ALS_INT_FLAG_MASK);
	stk_writeb(STK_ALS_INT_REG,val);

}

static irqreturn_t stk_oss_irq_handler(int irq, void *data)
{
	struct stkals_data *pData = data;

	disable_irq(pData->irq);
	queue_work(stk_oss_work_queue,&pData->work);

	return IRQ_HANDLED;
}

static int stk_als_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int32_t err= -EINVAL;
	struct stkals_data*  als_data;
	int result = 0;

	if (i2c_smbus_read_byte_data(client,STK_ALS_CMD_REG)<0) {
		ERR("STKALS : no device found\n");
		return -ENODEV;
	}

	INFO("STK ALS : als i2c slave address = 0x%x\n",client->addr);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		ERR("STKALS -- No Support for I2C_FUNC_SMBUS_BYTE_DATA\n");
		return -ENODEV;
	}

	als_data = kzalloc(sizeof(struct stkals_data),GFP_KERNEL);
	if (unlikely(als_data == 0)) {
		ERR("STKALS -- No enough memory\n");
		return -ENOMEM;
	}

	als_data->client = client;
	i2c_set_clientdata(client,als_data);
	mutex_init(&stkals_io_lock);

	pStkAlsData = als_data;
	stk_oss_work_queue = create_workqueue("stk_oss_wq");
	if (stk_oss_work_queue)
		INIT_WORK(&als_data->work, stk_oss_wq_function);
	else {
		ERR("%s:create_workqueue error\n", __func__);
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}

	err = set_power_state(0);
	if (err < 0) {
		ERR("%s: set_power_state error\n", __func__);
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}

	memset(buff,'\0',sizeof(buff));
	result = Get_Light_Sensor(buff);
	if (result != 0)
		printk("LightBuffer = %s \n", buff);

	init_all_setting();

	pStkAlsData->input_dev = input_allocate_device();
	if (pStkAlsData->input_dev==NULL) {
		ERR("STK ALS : can not allocate als input device\n");
		free_irq(client->irq, pStkAlsData);
		mutex_destroy(&stkals_io_lock);
		kfree(pStkAlsData);
		pStkAlsData = NULL;
		return -ENOMEM;
	}
	pStkAlsData->input_dev->name = ALS_NAME;
	set_bit(EV_ABS, pStkAlsData->input_dev->evbit);
	input_set_abs_params(pStkAlsData->input_dev, ABS_MISC, 0, alscode2lux((1<<12)-1), 0, 0);
	err = input_register_device(pStkAlsData->input_dev);
	if (err<0) {
		ERR("STK ALS : can not register als input device\n");
		free_irq(client->irq, pStkAlsData);
		mutex_destroy(&stkals_io_lock);
		input_free_device(pStkAlsData->input_dev);
		kfree(pStkAlsData);
		pStkAlsData = NULL;
		return err;
	}

	err = request_irq(client->irq, stk_oss_irq_handler, STK_IRQF_MODE, DEVICE_NAME, als_data);
	if (err < 0) {
		ERR("%s: request_irq(%d) failed for (%d)\n",
			__func__, client->irq, err);
		gpio_free(158);
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}

	INFO("STK ALS : register als input device OK\n");

	return 0;
}

static int stk_als_remove(struct i2c_client *client)
{
	platform_device_put(stk_oss_dev);
	gpio_free(158);
	mutex_destroy(&stkals_io_lock);

	if (pStkAlsData) {
		if (stk_oss_work_queue) {
			flush_workqueue(stk_oss_work_queue);
			destroy_workqueue(stk_oss_work_queue);
		}
		input_unregister_device(pStkAlsData->input_dev);
		input_free_device(pStkAlsData->input_dev);
		free_irq(client->irq, pStkAlsData);
		//kfree(pStkAlsData);
		pStkAlsData = 0;
	}

	return 0;
}

static const struct i2c_device_id stk_als_id[] =
{
	{ "stk_als", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, stk_als_id);

static struct i2c_driver stk_als_driver =
{
	.driver =
	{
		.name = STKALS_DRV_NAME,
	},
	.probe = stk_als_probe,
	.remove = stk_als_remove,
	.id_table = stk_als_id,
};

static int __init stk_i2c_als_init(void)
{
	int ret;

	ret = i2c_add_driver(&stk_als_driver);
	if (ret)
		return ret;

	if (pStkAlsData == NULL)
		return -EINVAL;

	stk_oss_dev = platform_device_alloc(DEVICE_NAME,-1);
	if (!stk_oss_dev) {
		i2c_del_driver(&stk_als_driver);
		return -ENOMEM;
	}
	if (platform_device_add(stk_oss_dev)) {
		i2c_del_driver(&stk_als_driver);
		return -ENOMEM;
	}

	ret = stk_sysfs_create_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_attrs);
	if (ret) {
		i2c_del_driver(&stk_als_driver);
		return -ENOMEM;
	}
	INFO("STK ALS Module initialized.\n");

	return 0;
}

static void __exit stk_i2c_als_exit(void)
{
	i2c_del_driver(&stk_als_driver);
}

MODULE_AUTHOR("Patrick Chang <patrick_chang@sitronix.com>");
MODULE_DESCRIPTION("SenseTek Ambient Light Sensor driver");
MODULE_LICENSE("GPL");

module_init(stk_i2c_als_init);
module_exit(stk_i2c_als_exit);
