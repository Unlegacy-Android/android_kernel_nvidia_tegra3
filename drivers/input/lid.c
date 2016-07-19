/*
 * ASUS LID driver
 *
 * Copyright (c) 2012, ASUSTek Corporation.
 * Copyright (c) 2016, André Pinela
 *
 */
 
#define DEBUG 0
 
#include <linux/device.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/gpio_event.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include <linux/input/lid.h>

struct lid_sensor
{
	const struct lid_sensor_platform_data *pdata;
	struct platform_device *pdev;
	struct input_dev *lid_input_device;
	unsigned int state;
	unsigned int irq;
	struct delayed_work work;
	struct workqueue_struct *wq;
};

static void lid_update_state(struct platform_device *pdev)
{
	struct lid_sensor *lid_sensor = platform_get_drvdata(pdev);
	int gpio_value;

	// Wait to stabilize gpio
	msleep(CONVERSION_TIME_MS);

	// Get gpio value (we use this value negated)
	gpio_value = gpio_get_value(lid_sensor->pdata->irq_gpio);
	if(gpio_value) {
		lid_sensor->state = 0;
	} else {
		lid_sensor->state = 1;
	}
	dev_dbg(&lid_sensor->pdev->dev, "Sensor state: %d\n", lid_sensor->state);
}

static ssize_t show_lid_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct lid_sensor *lid_sensor = platform_get_drvdata(pdev);
	
	lid_update_state(pdev);
	return sprintf(buf, "%d\n", lid_sensor->state);
}
static DEVICE_ATTR(lid_status, S_IWUSR | S_IRUGO, show_lid_status, NULL);

static struct attribute *lid_attrs[] = {
        &dev_attr_lid_status.attr,
        NULL
};

static struct attribute_group lid_attr_group = {
        .attrs = lid_attrs,
};

static irqreturn_t lid_interrupt_handler(int irq, void *devid)
{
	struct platform_device *pdev = to_platform_device(devid);
	struct lid_sensor *lid_sensor = platform_get_drvdata(pdev);

	dev_dbg(&pdev->dev, "Interrupt handler... gpio_get_value(%d)=%d\n",
		lid_sensor->pdata->irq_gpio,
		gpio_get_value(lid_sensor->pdata->irq_gpio));
	queue_delayed_work(lid_sensor->wq, &lid_sensor->work, 0);

	return IRQ_HANDLED;
}

static void lid_report_function(struct delayed_work *work)
{
	struct lid_sensor *lid_sensor = container_of(work, struct lid_sensor, work);

	if (lid_sensor->lid_input_device == NULL){
		dev_err(&lid_sensor->pdev->dev, "Input device doesn't exist\n");
		return;
	}

	// Update internal driver state getting gpio value
	lid_update_state(lid_sensor->pdev);

	// Update input SW_LID device state
	input_report_switch(lid_sensor->lid_input_device, SW_LID, lid_sensor->state);

	// Sync input device
	input_sync(lid_sensor->lid_input_device);

	dev_info(&lid_sensor->pdev->dev, "SW_LID report value = %d\n", lid_sensor->state);
}

static int __devinit lid_sensor_probe(struct platform_device *pdev)
{
	const struct lid_sensor_platform_data *pdata = pdev->dev.platform_data;
	struct lid_sensor *lid_sensor;
	int ret;
	int irq;

	if (!pdata) {
		dev_err(&pdev->dev, "No platform data\n");
		return -EINVAL;
	}

	if (!gpio_is_valid(pdata->irq_gpio)) {
		dev_err(&pdev->dev, "Invalid gpio pin\n");
		return -EINVAL;
	}

	dev_dbg(&pdev->dev, "Allocating driver structure\n");
	lid_sensor = kzalloc(sizeof(*lid_sensor), GFP_KERNEL);
	if (!lid_sensor) {
		dev_err(&pdev->dev, "Failed to alloc driver structure\n");
		return -ENOMEM;
	}
	lid_sensor->pdev = pdev;

	dev_dbg(&pdev->dev, "Creating workqueue thread \n");
	lid_sensor->wq = create_singlethread_workqueue(dev_name(&pdev->dev));
	if(!lid_sensor->wq){
		dev_err(&pdev->dev, "Failed to create workqueue structure\n");
		ret = -ENOMEM;
		goto err_free;
	}

	dev_dbg(&pdev->dev, "Allocating lid_input_device\n");
	lid_sensor->lid_input_device = input_allocate_device();
	if (!lid_sensor->lid_input_device) {
		dev_err(&pdev->dev, "Failed to alloc lid_input_device\n");
		ret = -ENOMEM;
		goto err_free;
	}

	dev_dbg(&pdev->dev, "Defining lid_input_device name, phys and setting bits\n");
	lid_sensor->lid_input_device->name = "lid_input";
	lid_sensor->lid_input_device->phys = "/dev/input/lid_indev";
	set_bit(EV_SW, lid_sensor->lid_input_device->evbit);
	set_bit(SW_LID, lid_sensor->lid_input_device->swbit);

	dev_dbg(&pdev->dev, "Requesting gpio pin: %d\n",pdata->irq_gpio);
	ret = gpio_request(pdata->irq_gpio, dev_name(&pdev->dev));
	if (ret) {
		dev_err(&pdev->dev, "Failed to request gpio pin: %d\n", ret);
		goto err_free;
	}

	dev_dbg(&pdev->dev, "Setting gpio_direction_input\n");
	ret = gpio_direction_input(pdata->irq_gpio);
	if (ret) {
		dev_err(&pdev->dev, "Failed to set gpio to input: %d\n", ret);
		goto err_gpio_free;
	}
	lid_sensor->state = 1;
	lid_sensor->pdata = pdata;

	dev_dbg(&pdev->dev, "Registering lid_input_device\n");
	ret = input_register_device(lid_sensor->lid_input_device);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register lid_input_device\n");
		goto err_gpio_free;
	}

	dev_dbg(&pdev->dev, "Creating sysfs group\n");
	ret = sysfs_create_group((struct kobject*)&pdev->dev.kobj, &lid_attr_group);
	if(ret) {
		dev_err(&pdev->dev, "Failed to create sysfs group\n");
		goto exit_input_free;
	}

	dev_dbg(&pdev->dev, "Requesting irq\n");
	irq = gpio_to_irq(pdata->irq_gpio);
	dev_dbg(&pdev->dev, "irq: %d\n",irq);
	if (irq > 0) {
		ret = request_irq(irq, lid_interrupt_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				dev_name(&pdev->dev), &pdev->dev);
		if (ret < 0)
			dev_warn(&pdev->dev, "Failed to request irq: %d\n", ret);
		else
			lid_sensor->irq = irq;
	}
	dev_dbg(&pdev->dev, "Enabling irq wake\n");
	enable_irq_wake(irq);
	
	platform_set_drvdata(pdev, lid_sensor);
	
	INIT_DELAYED_WORK_DEFERRABLE(&lid_sensor->work, lid_report_function);

	return 0;

exit_input_free:
	input_free_device(lid_sensor->lid_input_device);
err_gpio_free:
	gpio_free(pdata->irq_gpio);
err_free:
	kfree(lid_sensor);
	return ret;
}

static int __devexit lid_sensor_remove(struct platform_device *pdev)
{
	struct lid_sensor *lid_sensor = platform_get_drvdata(pdev);

	if (lid_sensor->irq)
		free_irq(lid_sensor->irq, &lid_sensor->lid_input_device);

	sysfs_remove_group(&pdev->dev.kobj, &lid_attr_group);

	input_unregister_device(lid_sensor->lid_input_device);

	gpio_free(lid_sensor->pdata->irq_gpio);

	platform_set_drvdata(pdev, NULL);
	kfree(lid_sensor);

	return 0;
}

static SIMPLE_DEV_PM_OPS(lid_sensor_pm_ops, NULL, NULL);

static struct platform_driver lid_sensor_driver = {
	.probe = lid_sensor_probe,
	.remove = __devexit_p(lid_sensor_remove),
	.driver = {
		.name = "lid-sensor",
		.owner = THIS_MODULE,
		.pm = &lid_sensor_pm_ops,
	},
};

module_platform_driver(lid_sensor_driver);

MODULE_DESCRIPTION("ASUS Hall Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:lid-sensor");
