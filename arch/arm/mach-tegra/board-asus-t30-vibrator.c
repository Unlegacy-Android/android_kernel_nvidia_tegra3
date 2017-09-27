/*
 * arch/arm/mach-tegra/board-asus-t30-vibrator.c
 *
 * Copyright (C) 2011-2012 ASUSTek Computer Incorporation
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <mach/board-asus-t30-misc.h>
#include "gpio-names.h"
#include "../../../drivers/staging/android/timed_output.h"
#include <linux/timer.h>

static struct timer_list v_timer;
//The vibrate events are asynchronously triggered from user
static void vibrator_enable(struct timed_output_dev *dev, int value)
{
        if (value) {
		pr_info("[VIB]: vibrator_enable: %d\n",value);
		del_timer_sync(&v_timer);//delete the last timer.
		v_timer.expires = jiffies + msecs_to_jiffies(value);
		add_timer(&v_timer);
		gpio_set_value(TEGRA_GPIO_PH7, 1);
        } else {
		gpio_set_value(TEGRA_GPIO_PH7, 0);
	}

}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	/* Always return 0, there is no related user space
	* vibrator API would call this function.*/
        return 0;
}

static struct timed_output_dev tegra_vibrator = {
        .name           = "vibrator",
        .get_time       = vibrator_get_time,
        .enable         = vibrator_enable,
};

static void stop_vibrator(unsigned long trigger)
{
	gpio_direction_output(TEGRA_GPIO_PH7, trigger);
}

static int __init vibrator_init(void)
{
	int ret;
	/*
	 * Use GMI_AD15 pin as a software-controlled GPIO
	 * to control vibrator
	 */
	u32 project_info = tegra3_get_project_id();
        if(project_info != TEGRA3_PROJECT_TF201 &&
	    project_info != TEGRA3_PROJECT_TF700T)
		return 0;

	printk(KERN_INFO "%s+ #####\n", __func__);
	ret = gpio_request(TEGRA_GPIO_PH7, "ENB_VIB");
	if (ret) {
		pr_info("[VIB]: gpio_request failed.\n");
		gpio_free(TEGRA_GPIO_PH7);
		return ret;
	}

	/* Turn off vibrator in default*/
	gpio_direction_output(TEGRA_GPIO_PH7, 0);

	init_timer(&v_timer);
	v_timer.expires = jiffies;
	v_timer.data = 0;
	v_timer.function = stop_vibrator;
	ret = timed_output_dev_register(&tegra_vibrator);

	if (ret)
		pr_info("[VIB]: timed_output_dev_register failed.\n");
	printk(KERN_INFO "%s- #####\n", __func__);
	return ret;
}

static void __exit vibrator_exit(void)
{
	del_timer_sync(&v_timer);
	stop_vibrator(0);
	timed_output_dev_unregister(&tegra_vibrator);
}

module_init(vibrator_init);
module_exit(vibrator_exit);

MODULE_DESCRIPTION("timed output vibrator device");
MODULE_LICENSE("GPL");
