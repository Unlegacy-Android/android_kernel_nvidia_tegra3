/*
 * ASUS LID driver
 *
 * Copyright (c) 2012, ASUSTek Corporation.
 * Copyright (c) 2018, Svyatoslav Ryhel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/gpio_event.h>

#include <asm/gpio.h>
#include <../gpio-names.h>

#include <mach/board-transformer-misc.h>

#include "asusec.h"

static unsigned int hall_sensor_gpio = TEGRA_GPIO_PS6;
struct delayed_work lid_hall_sensor_work;
static struct workqueue_struct *lid_wq;

static struct input_dev	*lid_indev;
static struct platform_device *lid_dev; /* Device structure */

static ssize_t show_lid_status(struct device *class, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gpio_get_value(hall_sensor_gpio));
}
static DEVICE_ATTR(lid_status, S_IWUSR | S_IRUGO, show_lid_status, NULL);

/* Attribute Descriptor */
static struct attribute *lid_attrs[] = {
	&dev_attr_lid_status.attr,
	NULL
};

/* Attribute group */
static struct attribute_group lid_attr_group = {
	.attrs = lid_attrs,
};

static irqreturn_t lid_interrupt_handler(int irq, void *dev_id)
{
	if (irq == gpio_to_irq(hall_sensor_gpio))
		queue_delayed_work(lid_wq, &lid_hall_sensor_work, 0);

	return IRQ_HANDLED;
}

static int lid_irq_hall_sensor(void)
{
	int rc = 0;
	unsigned gpio = hall_sensor_gpio;
	unsigned irq = gpio_to_irq(hall_sensor_gpio);
	const char* label = "hall_sensor";

	rc = gpio_request(gpio, label);
	if (rc) {
		ASUSEC_ERR("gpio_request failed for input %d\n", gpio);
	}

	rc = gpio_direction_input(gpio);
	if (rc) {
		ASUSEC_ERR("gpio_direction_input failed for input %d\n", gpio);
		goto err_gpio_direction_input_failed;
	}

	rc = request_irq(irq, lid_interrupt_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, label, lid_indev);
	if (rc < 0) {
		ASUSEC_ERR("Could not register for %s interrupt, irq = %d, rc = %d\n", label, irq, rc);
		rc = -EIO;
		goto err_gpio_request_irq_fail;
	}

	enable_irq_wake(irq);

	if (gpio_get_value(gpio)){
		ASUSEC_NOTICE("LID open\n");
	} else {
		ASUSEC_NOTICE("LID close\n");
	}

	return 0;

err_gpio_request_irq_fail:
	gpio_free(gpio);
err_gpio_direction_input_failed:
	return rc;
}

static void lid_report_function(struct work_struct *dat)
{
	int value = 0;

	if (lid_indev == NULL){
		ASUSEC_ERR("LID input device doesn't exist\n");
		return;
	}
	msleep(DELAY_TIME_MS);
	value = gpio_get_value(hall_sensor_gpio);

	if (value)
		input_report_switch(lid_indev, SW_LID, 0);
	else
		input_report_switch(lid_indev, SW_LID, 1);

	input_sync(lid_indev);
}

static int lid_input_device_create(void)
{
	int err = 0;

	lid_indev = input_allocate_device();
	if (!lid_indev) {
		ASUSEC_ERR("lid_indev allocation fails\n");
		err = -ENOMEM;
		goto exit;
	}

	lid_indev->name = "lid_input";
	lid_indev->phys = "/dev/input/lid_indev";

	set_bit(EV_SW, lid_indev->evbit);
	set_bit(SW_LID, lid_indev->swbit);

	err = input_register_device(lid_indev);
	if (err) {
		ASUSEC_ERR("lid_indev registration fails\n");
		goto exit_input_free;
	}
	return 0;

exit_input_free:
	input_free_device(lid_indev);
	lid_indev = NULL;
exit:
	return err;

}

static int __init lid_init(void)
{
	int err = 0;

	lid_dev = platform_device_register_simple("LID", -1, NULL, 0);
	if (!lid_dev){
		pr_err("LID_init: error\n");
		return -ENOMEM;
	}

	err = sysfs_create_group(&lid_dev->dev.kobj, &lid_attr_group);
	if (err) {
		ASUSEC_ERR("Unable to create the sysfs\n");
		return err;
	}

	err = lid_input_device_create();
	if (err != 0)
		return err;

	lid_wq = create_singlethread_workqueue("lid_wq");
	INIT_DELAYED_WORK_DEFERRABLE(&lid_hall_sensor_work, lid_report_function);

	lid_irq_hall_sensor();

	pr_info("LID sensor initiated\n");

	return 0;
}
module_init(lid_init);

static void __exit lid_exit(void)
{
	input_unregister_device(lid_indev);
	sysfs_remove_group(&lid_dev->dev.kobj, &lid_attr_group);
	platform_device_unregister(lid_dev);
}
module_exit(lid_exit);

MODULE_DESCRIPTION("ASUS Hall Sensor Driver");
MODULE_LICENSE("GPL");
