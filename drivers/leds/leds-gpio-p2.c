#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/leds.h>
#include <linux/leds-gpio-p2.h>
#include <linux/power_supply.h>

static struct delayed_work gpio_led_wq;
static struct gpio_led_data *pdata;
static int already_LED_ON = 0;
static struct power_supply *psy;

#define LED_DELAY_TIME 5000

static int gpio_led_suspend(struct device *dev)
{
	int ret;
	union power_supply_propval battery_capacity;

	cancel_delayed_work(&gpio_led_wq);

	if (psy) {
		ret = psy->get_property(psy, POWER_SUPPLY_PROP_STATUS, &battery_capacity);
		if (ret == 0) {
			if (battery_capacity.intval == POWER_SUPPLY_STATUS_FULL) {
				if (already_LED_ON == 0) {
					gpio_direction_output(pdata->gpio, 1);
					already_LED_ON = 1;
					pr_info("[LED] driver LED_ON\n");
				}
			} else {
				gpio_direction_output(pdata->gpio, 0);
				already_LED_ON = 0;
				pr_info("[LED] driver LED_OFF\n");
			}
		} else {
			gpio_direction_output(pdata->gpio, 0);
		}
	} else {
		gpio_direction_output(pdata->gpio, 0);
	}

	return 0;
}

static int gpio_led_resume(struct device *dev)
{
	if (already_LED_ON == 0) {
		gpio_direction_output(pdata->gpio, 1);
		already_LED_ON = 1;
		pr_info("[LED] driver LED_ON\n");
		schedule_delayed_work(&gpio_led_wq, msecs_to_jiffies(LED_DELAY_TIME));
	}

	return 0;
}

static void gpio_led_work_func(struct work_struct *work)
{
	int ret;
	union power_supply_propval battery_capacity;

	if (psy) {
		ret = psy->get_property(psy, POWER_SUPPLY_PROP_STATUS, &battery_capacity);
		if (ret == 0) {
			if (battery_capacity.intval == POWER_SUPPLY_STATUS_FULL) {
				if (already_LED_ON == 0) {
					gpio_direction_output(pdata->gpio, 1);
					already_LED_ON = 1;
					pr_info("[LED] driver LED_ON\n");
				}
			} else {
				gpio_direction_output(pdata->gpio, 0);
				already_LED_ON = 0;
				pr_info("[LED] driver LED_OFF\n");
			}
		} else {
			gpio_direction_output(pdata->gpio, 0);
		}
	} else {
		gpio_direction_output(pdata->gpio, 0);
	}
}

static void gpio_whiteled_set(struct led_classdev *led_cdev,
                              enum led_brightness value)
{
	if (value) {
		if (already_LED_ON == 0) {
			gpio_direction_output(pdata->gpio,1);
			already_LED_ON = 1;
			pr_info("[LED] driver & sysfs LED_ON\n");
		}
	} else {
		gpio_direction_output(pdata->gpio,0);
		already_LED_ON = 0;
		pr_info("[LED] driver & sysfs LED_OFF\n");
	}
}

static struct led_classdev cdev_whiteled = {
	.name = "acer-leds",
	.brightness_set = gpio_whiteled_set,
};

static int gpio_led_probe(struct platform_device *pdev)
{
	int rc;

	pdata = pdev->dev.platform_data;

	rc = gpio_request(pdata->gpio, "LED_EN");
	if (rc) {
		pr_err("[LED] gpio_request failed on pin %d (rc=%d)\n", pdata->gpio, rc);
		goto err_gpio_request;
	}

	rc = gpio_direction_output(pdata->gpio, 0);
	if (rc) {
		pr_err("[LED] direction configuration failed %d\n", rc);
		goto err_gpio_direction_cfg;
	}

	rc = led_classdev_register(&pdev->dev, &cdev_whiteled);
	if (rc) {
		pr_err("[LED] led_classdev_register failed %d\n", rc);
		goto err_led_classdev_reg;
	}

	if (pdata->psy_name) {
		psy = power_supply_get_by_name(pdata->psy_name);
		if (!psy)
			pr_err("[LED] failed to get power supply\n");
	}

	INIT_DELAYED_WORK(&gpio_led_wq, gpio_led_work_func);

	pr_info("[LED] driver init done.\n");

	return 0;

err_led_classdev_reg:
err_gpio_direction_cfg:
	gpio_free(pdata->gpio);
err_gpio_request:
	return rc;
}

static int gpio_led_remove(struct platform_device *pdev)
{
	cancel_delayed_work_sync(&gpio_led_wq);
	led_classdev_unregister(&cdev_whiteled);
	gpio_free(pdata->gpio);
	kfree(pdata);

	return 0;
}

#ifdef CONFIG_PM
static SIMPLE_DEV_PM_OPS(gpio_leds_pm, gpio_led_suspend, gpio_led_resume);
#endif

static struct platform_driver gpio_led_driver = {
	.probe		= gpio_led_probe,
	.remove		= gpio_led_remove,
	.driver		= {
		.name	= "gpio-leds",
#ifdef CONFIG_PM
		.pm     = &gpio_leds_pm,
#endif
	},
};

module_platform_driver(gpio_led_driver);

MODULE_AUTHOR("Shawn Tu <Shawn_Tu@acer.com.tw>");
MODULE_DESCRIPTION("ACER LED driver");
MODULE_LICENSE("GPL");
