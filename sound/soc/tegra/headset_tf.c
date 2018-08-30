/*
 * Headset device detection driver.
 *
 * Copyright (C) 2011 ASUSTek Corporation.
 *
 * Authors:
 * Jason Cheng <jason4_cheng@asus.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/timer.h>
#include <linux/switch.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <sound/soc.h>
#include "../gpio-names.h"
#include "../codecs/wm8903.h"
#include "../codecs/rt5631.h"
#include "../codecs/rt5640.h"
#include <mach/board-transformer-misc.h>
#include "../board-transformer.h"
#include <mach/pinmux-tegra30.h>
#include "../board.h"
MODULE_DESCRIPTION("Headset detection driver");
MODULE_LICENSE("GPL");

#define DEFAULT_PINMUX(_pingroup, _mux, _pupd, _tri, _io)       \
        {                                                       \
                .pingroup       = TEGRA_PINGROUP_##_pingroup,   \
                .func           = TEGRA_MUX_##_mux,             \
                .pupd           = TEGRA_PUPD_##_pupd,           \
                .tristate       = TEGRA_TRI_##_tri,             \
                .io             = TEGRA_PIN_##_io,              \
                .lock           = TEGRA_PIN_LOCK_DEFAULT,       \
                .od             = TEGRA_PIN_OD_DEFAULT,         \
                .ioreset        = TEGRA_PIN_IO_RESET_DEFAULT,   \
        }

/*----------------------------------------------------------------------------
 ** FUNCTION DECLARATION
 **----------------------------------------------------------------------------*/
static int __init 	headset_init(void);
static void __exit headset_exit(void);
static irqreturn_t 	detect_irq_handler(int irq, void *dev_id);
static void 		detection_work(struct work_struct *work);
static int		jack_config_gpio(void);
static void 		lineout_work_queue(struct work_struct *work);
static int              lineout_config_gpio(u32 project_info);
static void 		detection_work(struct work_struct *work);
static int              btn_config_gpio(void);
static int 		hs_micbias_power(int on);
static irqreturn_t 	lineout_irq_handler(int irq, void *dev_id);
/*----------------------------------------------------------------------------
 ** GLOBAL VARIABLES
 **----------------------------------------------------------------------------*/
#define JACK_GPIO		(TEGRA_GPIO_PW2)
#define LINEOUT_GPIO_TF		(TEGRA_GPIO_PX3)
#define LINEOUT_GPIO_NAKASI	(TEGRA_GPIO_PW3)
#define LINEOUT_GPIO_BACH	(TEGRA_GPIO_PX6)
#define HOOK_GPIO		(TEGRA_GPIO_PX2)
#define UART_HEADPHONE_SWITCH (TEGRA_GPIO_PS2)
#define ON	(1)
#define OFF	(0)

enum{
	NO_DEVICE = 0,
	HEADSET_WITH_MIC = 1,
	HEADSET_WITHOUT_MIC = 2,
};

enum{
	NO_LINEOUT = 0,
	LINEOUT_IN = 1,
};

struct headset_data {
	struct switch_dev sdev;
	struct switch_dev ldev;
	struct input_dev *input;
	unsigned int irq;
	struct hrtimer timer;
	ktime_t debouncing_time;
};

static struct headset_data *hs_data;
bool headset_alive = false;
EXPORT_SYMBOL(headset_alive);
bool lineout_alive;
EXPORT_SYMBOL(lineout_alive);

static struct workqueue_struct *g_detection_work_queue;
static DECLARE_WORK(g_detection_work, detection_work);

struct work_struct headset_work;
struct work_struct lineout_work;
static u32 lineout_gpio;

extern struct snd_soc_codec *rt5631_audio_codec;
extern struct snd_soc_codec *wm8903_codec;

static ssize_t lineout_name_show(struct switch_dev *ldev, char *buf)
{
	switch (switch_get_state(&hs_data->ldev)){
		case NO_LINEOUT:
			{
				return sprintf(buf, "%s\n", "No Device");
			}
		case LINEOUT_IN:
			{
				return sprintf(buf, "%s\n", "LINEOUT_IN");
			}
	}
	return -EINVAL;
}

static ssize_t lineout_state_show(struct switch_dev *ldev, char *buf)
{
	switch (switch_get_state(&hs_data->ldev)){
		case NO_LINEOUT:
			return sprintf(buf, "%d\n", 0);
		case LINEOUT_IN:
			return sprintf(buf, "%d\n", 1);
	}
	return -EINVAL;
}

/**********************************************************
 ** Function: LineOut detection interrupt handler
 ** Parameter: dedicated irq
 ** Return value: if sucess, then returns IRQ_HANDLED
 **
 ************************************************************/
static irqreturn_t lineout_irq_handler(int irq, void *dev_id)
{
	schedule_work(&lineout_work);

	return IRQ_HANDLED;
}

static ssize_t headset_name_show(struct switch_dev *sdev, char *buf)
{
	switch (switch_get_state(&hs_data->sdev)){
	case NO_DEVICE:{
		return sprintf(buf, "%s\n", "No Device");
		}
	case HEADSET_WITH_MIC:{
		return sprintf(buf, "%s\n", "HEADSET");
		}
	case HEADSET_WITHOUT_MIC:{
		return sprintf(buf, "%s\n", "HEADPHONE");
		}
	}
	return -EINVAL;
}

static ssize_t headset_state_show(struct switch_dev *sdev, char *buf)
{
	switch (switch_get_state(&hs_data->sdev)){
	case NO_DEVICE:
		return sprintf(buf, "%d\n", 0);
	case HEADSET_WITH_MIC:
		return sprintf(buf, "%d\n", 1);
	case HEADSET_WITHOUT_MIC:
		return sprintf(buf, "%d\n", 2);
	}
	return -EINVAL;
}

static void insert_headset(void)
{
    if(gpio_get_value(HOOK_GPIO)){ 
		pr_info("HEADSET: %s: headphone\n", __func__);
		switch_set_state(&hs_data->sdev, HEADSET_WITHOUT_MIC);
		hs_micbias_power(OFF);
		headset_alive = false;
	} else {
		pr_info("HEADSET: %s: headset\n", __func__);
		switch_set_state(&hs_data->sdev, HEADSET_WITH_MIC);
		hs_micbias_power(ON);
		headset_alive = true;
	}
	hs_data->debouncing_time = ktime_set(0, 100000000);  /* 100 ms */
}

static void remove_headset(void)
{
	switch_set_state(&hs_data->sdev, NO_DEVICE);
	hs_data->debouncing_time = ktime_set(0, 100000000);  /* 100 ms */
	headset_alive = false;
}

static void detection_work(struct work_struct *work)
{
	unsigned long irq_flags;
	int cable_in1;
	int mic_in = 0;

	hs_micbias_power(ON);

	/* Disable headset interrupt while detecting.*/
	local_irq_save(irq_flags);
	disable_irq(hs_data->irq);
	local_irq_restore(irq_flags);

	/* Delay 1000ms for pin stable. */
	msleep(1000);

	/* Restore IRQs */
	local_irq_save(irq_flags);
	enable_irq(hs_data->irq);
	local_irq_restore(irq_flags);

	if (gpio_get_value(JACK_GPIO) != 0) {
		/* Headset not plugged in */
			remove_headset();
		goto closed_micbias;
	}

	cable_in1 = gpio_get_value(JACK_GPIO);
	mic_in  = gpio_get_value(HOOK_GPIO);
	if (cable_in1 == 0) {
	    pr_info("HEADSET: HOOK_GPIO value: %d\n", mic_in);
		if(switch_get_state(&hs_data->sdev) == NO_DEVICE)
			insert_headset();
		else if ( mic_in == 1)
			goto closed_micbias;
	} else {
		pr_info("HEADSET: Jack-in GPIO is low, but not a headset \n");
		goto closed_micbias;
	}
	return;

closed_micbias:
	hs_micbias_power(OFF);
	return;
}

static enum hrtimer_restart detect_event_timer_func(struct hrtimer *data)
{
	queue_work(g_detection_work_queue, &g_detection_work);
	return HRTIMER_NORESTART;
}

/**********************************************************
**  Function: Jack detection-in gpio configuration function
**  Parameter: none
**  Return value: if sucess, then returns 0
**
************************************************************/
static int jack_config_gpio()
{
	int ret;

	pr_info("HEADSET: Config Jack-in detection gpio\n");
	hs_micbias_power(ON);
	ret = gpio_request(JACK_GPIO, "h2w_detect");
	ret = gpio_direction_input(JACK_GPIO);

	hs_data->irq = gpio_to_irq(JACK_GPIO);
	ret = request_irq(hs_data->irq, detect_irq_handler,
			  IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "h2w_detect", NULL);

	ret = irq_set_irq_wake(hs_data->irq, 1);

	msleep(1);

	if (gpio_get_value(JACK_GPIO) == 0){
		insert_headset();
	} else {
		hs_micbias_power(OFF);
		switch_set_state(&hs_data->sdev, NO_DEVICE);
		remove_headset();
	}

	return 0;
}

/**********************************************************
**  Function: Headset Hook Key Detection interrupt handler
**  Parameter: irq
**  Return value: IRQ_HANDLED
**  High: Hook button pressed
************************************************************/
static int btn_config_gpio()
{
	int ret;

	pr_info("HEADSET: Config Headset Button detection gpio\n");

	ret = gpio_request(HOOK_GPIO, "btn_INT");
	ret = gpio_direction_input(HOOK_GPIO);

	return 0;
}

static void lineout_work_queue(struct work_struct *work)
{
	msleep(300);
	if (gpio_get_value(lineout_gpio) == 0){
		pr_info("HEADSET: LINEOUT: LineOut inserted\n");
		lineout_alive = true;
	} else if(gpio_get_value(lineout_gpio)){
		pr_info("HEADSET: LINEOUT: LineOut removed\n");
		lineout_alive = false;
	}
}

/**********************************************************
**  Function: LineOut Detection configuration function
**  Parameter: none
**  Return value: IRQ_HANDLED
**
************************************************************/
static int lineout_config_gpio(u32 project_info)
{
	int ret;

	pr_info("HEADSET: Config LineOut detection gpio\n");
	lineout_gpio = LINEOUT_GPIO_TF;
	ret = gpio_request(lineout_gpio, "lineout_int");
	ret = gpio_direction_input(lineout_gpio);

	ret = request_irq(gpio_to_irq(lineout_gpio),
			&lineout_irq_handler,
			IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
			"lineout_int", 0);
	if (gpio_get_value(lineout_gpio) == 0){
		lineout_alive = true;
	} else {
		lineout_alive = false;
	}
	return 0;
}

/**********************************************************
**  Function: Headset jack-in detection interrupt handler
**  Parameter: dedicated irq
**  Return value: if sucess, then returns IRQ_HANDLED
**
************************************************************/
static irqreturn_t detect_irq_handler(int irq, void *dev_id)
{
	int value1, value2;
	int retry_limit = 10;

	do {
		value1 = gpio_get_value(JACK_GPIO);
		irq_set_irq_type(hs_data->irq, value1 ?
				IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING);
		value2 = gpio_get_value(JACK_GPIO);
	} while (value1 != value2 && retry_limit-- > 0);

	if ((switch_get_state(&hs_data->sdev) == NO_DEVICE) ^ value2){
		hrtimer_start(&hs_data->timer, hs_data->debouncing_time, HRTIMER_MODE_REL);
	}

	return IRQ_HANDLED;
}

static int codec_micbias_power(int on)
{
	switch (tegra3_get_project_id()) {
	case TEGRA3_PROJECT_TF201:
	case TEGRA3_PROJECT_TF300TG:
	case TEGRA3_PROJECT_TF300TL:
	case TEGRA3_PROJECT_TF700T:
		if(on){
			if(rt5631_audio_codec == NULL){
				pr_info("HEADSET: %s: No rt5631 rt5631_audio_codec - set micbias on fail\n", __func__);
				return 0;
			}
		} else {
			if(rt5631_audio_codec == NULL){
				pr_info("HEADSET: %s: No rt5631 rt5631_audio_codec - set micbias off fail\n", __func__);
				return 0;
			}
			snd_soc_update_bits(rt5631_audio_codec, RT5631_PWR_MANAG_ADD2, RT5631_PWR_MICBIAS1_VOL, 0); /* Disable MicBias1 */
		}
		break;
	case TEGRA3_PROJECT_TF300T:
		if(on){
			if(wm8903_codec == NULL){
					pr_info("HEADSET: %s: No wm8903_codec - set micbias on fail\n", __func__);
					return 0;
			}
		} else {
			if(wm8903_codec == NULL){
					pr_info("HEADSET: %s: No wm8903_codec - set micbias off fail\n", __func__);
					return 0;
			}
			snd_soc_update_bits(wm8903_codec, WM8903_MIC_BIAS_CONTROL_0, 0, 0); /* Disable MicBias1 */
		}
		break;
	}
	return 0;
}

static int hs_micbias_power(int on)
{
	static int nLastVregStatus = -1;

	if(on && nLastVregStatus!=ON){
		pr_info("HEADSET: Turn on micbias power\n");
		nLastVregStatus = ON;
		codec_micbias_power(ON);
	}
	else if(!on && nLastVregStatus!=OFF){
		pr_info("HEADSET: Turn off micbias power\n");
		nLastVregStatus = OFF;
		codec_micbias_power(OFF);
	}
	return 0;
}

/**********************************************************
**  Function: Headset driver init function
**  Parameter: none
**  Return value: none
**
************************************************************/
static int __init headset_init(void)
{
	int ret;
	u32 project_info = tegra3_get_project_id();

	pr_info("%s+ #####\n", __func__);

	pr_info("HEADSET: Headset detection init\n");

	hs_data = kzalloc(sizeof(struct headset_data), GFP_KERNEL);
	if (!hs_data)
		return -ENOMEM;

	hs_data->debouncing_time = ktime_set(0, 100000000);  /* 100 ms */
	hs_data->sdev.name = "h2w";
	hs_data->sdev.print_name = headset_name_show;
	hs_data->sdev.print_state = headset_state_show;

	hs_data->ldev.name = "lineout";
	hs_data->ldev.print_name = lineout_name_show;
	hs_data->ldev.print_state = lineout_state_show;

	ret = switch_dev_register(&hs_data->ldev);
	if (ret < 0)
		goto err_switch_dev_register;

	ret = switch_dev_register(&hs_data->sdev);
	if (ret < 0)
		goto err_switch_dev_register;

	g_detection_work_queue = create_workqueue("detection");

	hrtimer_init(&hs_data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hs_data->timer.function = detect_event_timer_func;

	pr_info("HEADSET: Headset detection mode\n");
	lineout_config_gpio(project_info);
	btn_config_gpio();/*Config hook detection GPIO*/
	jack_config_gpio();/*Config jack detection GPIO*/
	INIT_WORK(&lineout_work, lineout_work_queue);

	pr_info("%s- #####\n", __func__);
	return 0;

err_switch_dev_register:
	pr_err("Headset: Failed to register driver\n");

	return ret;
}

/**********************************************************
**  Function: Headset driver exit function
**  Parameter: none
**  Return value: none
**
************************************************************/
static void __exit headset_exit(void)
{
	pr_info("HEADSET: Headset exit\n");
	if (switch_get_state(&hs_data->sdev))
		remove_headset();
	gpio_free(JACK_GPIO);
	gpio_free(HOOK_GPIO);
	gpio_free(lineout_gpio);

	free_irq(hs_data->irq, 0);
	destroy_workqueue(g_detection_work_queue);
	switch_dev_unregister(&hs_data->sdev);
}

module_init(headset_init);
module_exit(headset_exit);
