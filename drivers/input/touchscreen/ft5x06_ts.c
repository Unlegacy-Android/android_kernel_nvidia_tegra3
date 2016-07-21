/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
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
#define DEBUG
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/i2c/ft5x06_ts.h>

#include <mach/pinmux.h>
#include "../../../arch/arm/mach-tegra/gpio-names.h"

#include "ft5x06_ex_fun.h"

#include <mach/fb.h>

extern int ft5x0x_create_sysfs(struct i2c_client *client);
extern int fts_ctpm_auto_upgrade(struct i2c_client *client);
enum {
	FT_SUSPEND,
};

struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x0x_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	struct ft5x0x_platform_data *pdata;
	unsigned long	flags;
};

/*
*ft5x0x_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}

/*release the point*/
static void ft5x0x_ts_release(struct ft5x0x_ts_data *data)
{
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_sync(data->input_dev);
}

/*Read touch point information when the interrupt  is asserted.*/
static int ft5x0x_read_Touchdata(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft5x0x_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;

		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;

		if ( event->au8_touch_event[i] == 0 ||
				event->au8_touch_event[i] == 2)
			event->touch_point++;

		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;

	}
	event->pressure = FT_PRESS;
	return 0;
}

/*
*report the point information
*/
static void ft5x0x_report_value(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i = 0;

	for (i = 0; i < event->touch_point; i++) {
		/* LCD view area */
		if (event->au16_x[i] < data->x_max
		    && event->au16_y[i] < data->y_max) {
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					 event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					 event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 event->pressure);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
					 event->au8_finger_id[i]);
			if (event->au8_touch_event[i] == 0
			    || event->au8_touch_event[i] == 2)
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR,
						 event->pressure);
			else
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR, 0);
		}
		input_mt_sync(data->input_dev);
	}
	input_report_key(data->input_dev, BTN_TOUCH, event->touch_point > 0);
	input_sync(data->input_dev);

	if (event->touch_point == 0)
		ft5x0x_ts_release(data);
}

/*The ft5x0x device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;
	int ret = 0;

	if (test_bit(FT_SUSPEND, &ft5x0x_ts->flags))
		return IRQ_HANDLED;

	disable_irq_nosync(ft5x0x_ts->irq);

	ret = ft5x0x_read_Touchdata(ft5x0x_ts);
	if (ret == 0)
		ft5x0x_report_value(ft5x0x_ts);

	enable_irq(ft5x0x_ts->irq);

	return IRQ_HANDLED;
}

#if defined(CONFIG_PM)
static int ft5x0x_ts_suspend(struct device *dev)
{
	struct ft5x0x_ts_data *data = dev_get_drvdata(dev);

	disable_irq(data->client->irq);

	//set_bit(FT_SUSPEND, &data->flags);
	gpio_set_value(data->pdata->reset, 0);

/*++++20120606 JimmySu add for touch power consumption*/
	gpio_request(TEGRA_GPIO_PT5, "ft5x0x-i2c-scl-sleep");
	gpio_direction_output(TEGRA_GPIO_PT5, 0);
	gpio_set_value(TEGRA_GPIO_PT5, 0);

	gpio_request(TEGRA_GPIO_PT6, "ft5x0x-i2c-sda-sleep");
	gpio_direction_output(TEGRA_GPIO_PT6, 0);
	gpio_set_value(TEGRA_GPIO_PT6, 0);
/*----20120606 JimmySu add for touch power consumption*/

	return 0;
}

static int ft5x0x_ts_resume(struct device *dev)
{
	//u8 chip_id;
	struct ft5x0x_ts_data *data = dev_get_drvdata(dev);

/*++++20120606 JimmySu add for touch power consumption*/
	gpio_free(TEGRA_GPIO_PT5);
	gpio_free(TEGRA_GPIO_PT6);
/*----20120606 JimmySu add for touch power consumption*/

	gpio_set_value_cansleep(data->pdata->reset, 1);
	mdelay(10);
	gpio_set_value_cansleep(data->pdata->reset, 0);
	mdelay(10);
	gpio_set_value_cansleep(data->pdata->reset, 1);
	msleep(100);

/*	if(ft5x0x_read_reg(ts->client, 0xA3, &chip_id) < 0){
		dev_dbg(&ts->client->dev, "[FTS]ft5x0x chip ID read error reset once.\n");
		gpio_set_value(ts->pdata->reset, 0);
		mdelay(10);
	gpio_set_value(ts->pdata->reset, 1);
	}*/

	//clear_bit(FT_SUSPEND, &ts->flags);
	enable_irq(data->client->irq);

	return 0;
}

static SIMPLE_DEV_PM_OPS(ft5x0x_ts_pm, ft5x0x_ts_suspend, ft5x0x_ts_resume);
#endif



static void ft5x0x_ts_shutdown(struct i2c_client *client)
{
//		pr_info("%s++\n", __func__);


	disable_irq_nosync(client->irq);

	/* power down LCD, add use a black screen for HDMI */

	gpio_request(TEGRA_GPIO_PT6, "ft5x0x-i2c-sda-sleep");
	gpio_direction_output(TEGRA_GPIO_PT6, 0);
	gpio_set_value(TEGRA_GPIO_PT6, 0);

	gpio_request(TEGRA_GPIO_PT5, "ft5x0x-i2c-scl-sleep");
	gpio_direction_output(TEGRA_GPIO_PT5, 0);
	gpio_set_value(TEGRA_GPIO_PT5, 0);

	return ;

}

static int ft5x0x_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct ft5x0x_platform_data *pdata =
	    (struct ft5x0x_platform_data *)client->dev.platform_data;
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;

	pr_info("%s++\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);

	if (!ft5x0x_ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	i2c_set_clientdata(client, ft5x0x_ts);
	ft5x0x_ts->irq = client->irq;
	ft5x0x_ts->client = client;
	ft5x0x_ts->pdata = pdata;
	ft5x0x_ts->x_max = pdata->x_max - 1;
	ft5x0x_ts->y_max = pdata->y_max - 1;
#ifdef CONFIG_PM
	err = gpio_request(pdata->reset, "ft5x0x reset");
	if (err < 0) {
		dev_err(&client->dev, "%s:failed to set gpio reset.\n",
			__func__);
		goto exit_request_reset;
	}

/*++++20120606, JimmySu add reset function for initial setting*/
	gpio_direction_output(pdata->reset, 1);
	gpio_set_value(pdata->reset, 1);
	mdelay(3);
	gpio_set_value(pdata->reset, 0);
	mdelay(10);
	gpio_set_value(pdata->reset, 1);
//	gpio_export(pdata->reset, 0);
/*----20120606, JimmySu add reset function for initial setting*/
#endif

	err = request_threaded_irq(client->irq, NULL, ft5x0x_ts_interrupt,
				   pdata->irqflags, client->dev.driver->name,
				   ft5x0x_ts);
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	disable_irq(client->irq);

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, ft5x0x_ts->x_max, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, ft5x0x_ts->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TRACKING_ID, 0, CFG_MAX_TOUCH_POINTS, 0, 0);

	input_dev->name = FT5X0X_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"ft5x0x_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}
	/*make sure CTP already finish startup process */
	msleep(150);

	/*get some register information */
	uc_reg_addr = FT5x0x_REG_FW_VER;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT5x0x_REG_POINT_RATE;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] report rate is %dHz.\n",
		uc_reg_value * 10);

	uc_reg_addr = FT5X0X_REG_THGROUP;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n",
		uc_reg_value * 4);

	ft5x0x_create_sysfs(client);  // 20120529 , JimmySu add attribute for check fireware version
	//fts_ctpm_auto_upgrade(client);

	if(ft5x0x_read_reg(client, 0xA3, &uc_reg_value) < 0){
		dev_dbg(&client->dev, "[FTS]ft5x0x chip ID read error reset once.\n");
		gpio_set_value(pdata->reset, 0);
		mdelay(10);
		gpio_set_value(pdata->reset, 1);
	}
	dev_dbg(&client->dev, "[FTS]ft5x0x chip ID = 0x%x.\n", uc_reg_value);

/*#if defined(CONFIG_HAS_EARLYSUSPEND)
	ft5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ft5x0x_ts->early_suspend.suspend = ft5x0x_ts_suspend;
	ft5x0x_ts->early_suspend.resume = ft5x0x_ts_resume;
	register_early_suspend(&ft5x0x_ts->early_suspend);
#endif*/

	clear_bit(FT_SUSPEND, &ft5x0x_ts->flags);
	enable_irq(client->irq);
	return 0;

exit_input_register_device_failed:
	pr_info("%s:%d--\n", __func__, __LINE__);
	input_free_device(input_dev);

exit_input_dev_alloc_failed:
	pr_info("%s:%d--\n", __func__, __LINE__);
	free_irq(client->irq, ft5x0x_ts);
#ifdef CONFIG_PM
exit_request_reset:
	pr_info("%s:%d--\n", __func__, __LINE__);
	gpio_free(ft5x0x_ts->pdata->reset);
#endif

exit_irq_request_failed:
	pr_info("%s:%d--\n", __func__, __LINE__);
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);

exit_alloc_data_failed:
	pr_info("%s:%d--\n", __func__, __LINE__);
exit_check_functionality_failed:
	pr_info("%s:%d--\n", __func__, __LINE__);
	return err;
}

static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	ft5x0x_ts = i2c_get_clientdata(client);
	input_unregister_device(ft5x0x_ts->input_dev);
	#ifdef CONFIG_PM
	gpio_free(ft5x0x_ts->pdata->reset);
	#endif
	free_irq(client->irq, ft5x0x_ts);
	kfree(ft5x0x_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{FT5X0X_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe = ft5x0x_ts_probe,
	.remove = __devexit_p(ft5x0x_ts_remove),
	.shutdown = ft5x0x_ts_shutdown,  //20121004, JimmySu register shutdown function to prevent warm-reset erase FW
	.id_table = ft5x0x_ts_id,
	.driver = {
		   .name = FT5X0X_NAME,
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM
		   .pm = &ft5x0x_ts_pm,
#endif
		   },
};

static int __init ft5x0x_ts_init(void)
{
	int ret;
	printk("%s+++\n", __func__);
	ret = i2c_add_driver(&ft5x0x_ts_driver);
	if (ret) {
		printk(KERN_WARNING "Adding ft5x0x driver failed "
		       "(errno = %d)\n", ret);
	} else {
		pr_info("Successfully added driver %s\n",
			ft5x0x_ts_driver.driver.name);
	}
	printk("%s---\n", __func__);
	return ret;
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
