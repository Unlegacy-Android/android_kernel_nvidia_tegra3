/*
 * Lite-On AL3010 Ambient Light Sensor driver
 *
 * Copyright(c) 2016 Dániel Járai
 *
 * Derived from the Google Nexus 7 kernel driver
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ktime.h>
#include <linux/input.h>

#define AL3010_NUM_CACHEABLE_REGS	9

#define	AL3010_ALS_COMMAND	4
#define	AL3010_RAN_MASK		0x70
#define	AL3010_RAN_SHIFT	(4)

#define AL3010_MODE_COMMAND	0

#define AL3010_POW_MASK		0x01
#define AL3010_POW_UP		0x01
#define AL3010_POW_DOWN		0x00
#define AL3010_POW_SHIFT	(0)

#define	AL3010_ADC_LSB	0x0c
#define	AL3010_ADC_MSB	0x0d

#define REVISE_LUX_TIMES	2
#define CALIBRATION_BASE_LUX	1000

static int calibration_regs = 880; // default K value 880 is average K value of PR devices

static bool is_poweron_after_resume = false;

static u8 al3010_reg[AL3010_NUM_CACHEABLE_REGS] =
	{0x00, 0x01, 0x0c, 0x0d, 0x10, 0x1a, 0x1b, 0x1c, 0x1d};

struct al3010_data {
	struct i2c_client *client;
	struct mutex lock;
	struct mutex input_lock;
	struct input_dev *light_input_dev;
	struct work_struct work_light;
	struct hrtimer timer;
	struct workqueue_struct *wq;
	ktime_t light_poll_delay;
	bool light_poll_enabled;
	int power_state;
	u8 reg_cache[AL3010_NUM_CACHEABLE_REGS];
};

static bool al3010_hardware_fail = false;

static int al3010_chip_resume(struct al3010_data *data);

static int __al3010_write_reg(struct i2c_client *client,
				u32 reg, u8 mask, u8 shift, u8 val)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	int ret = 0;
	u8 tmp;

	if (reg >= AL3010_NUM_CACHEABLE_REGS)
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

static int al3010_get_power_state(struct i2c_client *client)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	u8 cmdreg;

	mutex_lock(&data->lock);
	cmdreg = i2c_smbus_read_byte_data(client, AL3010_MODE_COMMAND);
	mutex_unlock(&data->lock);

	return (cmdreg & AL3010_POW_MASK) >> AL3010_POW_SHIFT;
}

static int al3010_set_power_state(struct i2c_client *client, int state)
{
	if (al3010_get_power_state(client) == state)
		return 0;

	dev_dbg(&client->dev, "power %s\n", state ? "on" : "off");

	return __al3010_write_reg(client, AL3010_MODE_COMMAND,
				AL3010_POW_MASK, AL3010_POW_SHIFT,
				state ? AL3010_POW_UP : AL3010_POW_DOWN);
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

	return (u32)((((msb << 8) | lsb) * CALIBRATION_BASE_LUX) / calibration_regs);
}

static void al3010_light_enable(struct al3010_data *data)
{
	dev_dbg(&data->client->dev, "starting poll timer, delay %lldns\n",
		    ktime_to_ns(data->light_poll_delay));
	hrtimer_start(&data->timer, data->light_poll_delay, HRTIMER_MODE_REL);
}

static void al3010_light_disable(struct al3010_data *data)
{
	dev_dbg(&data->client->dev, "stopping poll timer\n");
	hrtimer_cancel(&data->timer);
	cancel_work_sync(&data->work_light);
}

static enum hrtimer_restart al3010_timer_func(struct hrtimer *timer)
{
	struct al3010_data *data = container_of(timer, struct al3010_data, timer);
	queue_work(data->wq, &data->work_light);
	hrtimer_forward_now(&data->timer, data->light_poll_delay);
	return HRTIMER_RESTART;
}

static void al3010_work_func_light(struct work_struct *work)
{
	struct al3010_data *data = container_of(work, struct al3010_data,
					      work_light);
	int lux = al3010_get_adc_value(data->client) * REVISE_LUX_TIMES;
	input_report_abs(data->light_input_dev, ABS_MISC, lux);
	input_sync(data->light_input_dev);
}

static ssize_t al3010_show_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct al3010_data *data = dev_get_drvdata(dev);

	/* No LUX data if not operational */
	if (al3010_get_power_state(data->client) != 1)
		return -EBUSY;

	return sprintf(buf, "%d\n", al3010_get_adc_value(data->client));
}

static ssize_t al3010_show_revise_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct al3010_data *data = dev_get_drvdata(dev);

	if (al3010_hardware_fail)
		return sprintf(buf, "%d\n", -1);

	/* No LUX data if not operational */
	if (al3010_get_power_state(data->client) != 1)
		return -EBUSY;

	//+++ wait al3010 wake up
	if (is_poweron_after_resume) {
		msleep(200);
		is_poweron_after_resume = false;
	}

	return sprintf(buf, "%d\n", (al3010_get_adc_value(data->client) * REVISE_LUX_TIMES));
}

static ssize_t al3010_power_on(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct al3010_data *data = dev_get_drvdata(dev);
	int ret = al3010_chip_resume(data);

	return sprintf(buf, "%d\n", ret);
}

static ssize_t al3010_show_poll_delay(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct al3010_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%lld\n", ktime_to_ns(data->light_poll_delay));
}

static ssize_t al3010_store_poll_delay(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct al3010_data *data = dev_get_drvdata(dev);
	int64_t new_delay;
	int err;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	mutex_lock(&data->input_lock);
	dev_dbg(dev, "new poll delay = %lldns, old poll delay = %lldns\n",
	    new_delay, ktime_to_ns(data->light_poll_delay));
	if (new_delay != ktime_to_ns(data->light_poll_delay)) {
		data->light_poll_delay = ns_to_ktime(new_delay);
		if (data->light_poll_enabled) {
			al3010_light_disable(data);
			al3010_light_enable(data);
		}
	}
	mutex_unlock(&data->input_lock);

	return size;
}

static ssize_t al3010_show_enable(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct al3010_data *data = dev_get_drvdata(dev);

	if (al3010_hardware_fail)
		return sprintf(buf, "%d\n", 0);

	return sprintf(buf, "%d\n", al3010_get_power_state(data->client));
}

static ssize_t al3010_store_enable(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct al3010_data *data = dev_get_drvdata(dev);
	bool new_value;

	if (sysfs_streq(buf, "1")) {
		new_value = true;
	} else if (sysfs_streq(buf, "0")) {
		new_value = false;
	} else {
		dev_err(dev, "%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&data->input_lock);
	dev_dbg(dev, "polling enable = %d, old state = %d\n",
		    new_value, data->light_poll_enabled);
	if (new_value && !data->light_poll_enabled) {
		al3010_chip_resume(data);
		data->light_poll_enabled = true;
		al3010_light_enable(data);
	} else if (!new_value && data->light_poll_enabled) {
		al3010_light_disable(data);
		data->light_poll_enabled = false;
		al3010_set_power_state(data->client, 0);
	}
	mutex_unlock(&data->input_lock);
	return count;
}

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

static DEVICE_ATTR(show_lux, 0444, al3010_show_lux, NULL);
static DEVICE_ATTR(show_revise_lux, 0444, al3010_show_revise_lux, NULL);
static DEVICE_ATTR(power_on, 0444, al3010_power_on, NULL);
static DEVICE_ATTR(enable, 0644, al3010_show_enable, al3010_store_enable);
static DEVICE_ATTR(poll_delay, 0644, al3010_show_poll_delay, al3010_store_poll_delay);
static DEVICE_ATTR(calibration, 0644, al3010_show_calib, al3010_store_calib);

static struct attribute *al3010_attributes[] = {
	&dev_attr_show_lux.attr,
	&dev_attr_show_revise_lux.attr,
	&dev_attr_power_on.attr,
	&dev_attr_enable.attr,
	&dev_attr_poll_delay.attr,
	&dev_attr_calibration.attr,
	NULL
};

static const struct attribute_group al3010_attr_group = {
	.attrs = al3010_attributes,
};

/* restore registers from cache */
static int al3010_chip_resume(struct al3010_data *data)
{
	int ret = 0, i;

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
	}

	return ret;
}

static int al3010_init_client(struct i2c_client *client)
{
	int err;

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

	err = al3010_set_power_state(client, 0);
	if (err) {
		dev_err(&client->dev, "%s: al3010_set_power_state returned error %d\n", __func__, err);
		return err;
	}

	return 0;
}

static int __devinit al3010_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct al3010_data *data;
	int err;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	data = kzalloc(sizeof(struct al3010_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;
	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);
	mutex_init(&data->input_lock);

	/* initialize the AL3010 chip */
	err = al3010_init_client(client);
	if (err) {
		dev_err(&client->dev, "hardware fail\n");
		dev_err(&client->dev, "keep al3010 driver alive\n");
		err = 0;
		al3010_hardware_fail = true;
	}
	//re-init , workaround to fix init fail when i2c arbitration lost
	if (al3010_hardware_fail) {
		err = al3010_init_client(client);
		if (err) {
			dev_err(&client->dev, "re-init fail\n");
			dev_err(&client->dev, "keep al3010 driver alive\n");
		} else {
			dev_info(&client->dev, "re-init success\n");
			al3010_hardware_fail = false;
		}
		err = 0;
	}
	/* register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &al3010_attr_group);
	if (err) {
		dev_err(&client->dev, "sysfs_create_group returned error %d\n", err);
		goto exit_kfree;
	}

	/* allocate light_sensor input device */
	data->light_input_dev = input_allocate_device();
	if (!data->light_input_dev) {
		dev_err(&client->dev, "could not allocate input device\n");
		err = -ENOMEM;
		goto err_input;
	}
	input_set_drvdata(data->light_input_dev, data);
	data->light_input_dev->name = "light_sensor";
	input_set_capability(data->light_input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(data->light_input_dev, ABS_MISC, 0, 1, 0, 0);

	err = input_register_device(data->light_input_dev);
	if (err < 0) {
		dev_err(&client->dev, "could not register input device\n");
		input_free_device(data->light_input_dev);
		goto err_input;
	}
	data->light_input_dev = data->light_input_dev;
	err = sysfs_create_group(&data->light_input_dev->dev.kobj,
				 &al3010_attr_group);
	if (err) {
		dev_err(&client->dev, "could not create sysfs group\n");
		goto err_sysfs_create_group_input;
	}

	/* hrtimer settings - we poll for light values using a timer */
	hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->light_poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	data->timer.function = al3010_timer_func;

	data->wq = create_singlethread_workqueue("al3010_wq");
	if (!data->wq) {
		err = -ENOMEM;
		dev_err(&client->dev, "could not create workqueue\n");
		goto err_create_workqueue;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&data->work_light, al3010_work_func_light);

	dev_info(&client->dev, "probed\n");

	return 0;

err_create_workqueue:
	sysfs_remove_group(&data->light_input_dev->dev.kobj, &al3010_attr_group);
err_sysfs_create_group_input:
	input_unregister_device(data->light_input_dev);
err_input:
	sysfs_remove_group(&client->dev.kobj, &al3010_attr_group);
exit_kfree:
	kfree(data);
	return err;
}

static int __devexit al3010_remove(struct i2c_client *client)
{
	struct al3010_data *data = i2c_get_clientdata(client);
	sysfs_remove_group(&data->light_input_dev->dev.kobj, &al3010_attr_group);
	sysfs_remove_group(&client->dev.kobj, &al3010_attr_group);
	destroy_workqueue(data->wq);
	input_unregister_device(data->light_input_dev);
	al3010_set_power_state(client, 0);
	mutex_destroy(&data->lock);
	mutex_destroy(&data->input_lock);
	kfree(data);
	dev_dbg(&client->dev, "removed\n");

	return 0;
}

#ifdef CONFIG_PM
static int al3010_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3010_data *data = dev_get_drvdata(dev);

	if (al3010_hardware_fail)
		return 0;

	data->power_state = al3010_get_power_state(client);

	dev_dbg(dev, "%s\n", __func__);

	return al3010_set_power_state(to_i2c_client(dev), 0);
}

static int al3010_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3010_data *data = dev_get_drvdata(dev);

	// only turn on power if it was on before suspending
	if (!data->power_state || al3010_hardware_fail)
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

module_i2c_driver(al3010_driver);

MODULE_DESCRIPTION("AL3010 Ambient Light Sensor driver");
MODULE_LICENSE("GPL");
