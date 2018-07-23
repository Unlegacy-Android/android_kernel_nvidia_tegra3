/*
 * ASUS Dock EC driver.
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
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/gpio_event.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/power/pad_battery.h>

#include <asm/gpio.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>

#include <../gpio-names.h>

#include <mach/board-transformer-misc.h>

#include "asusec.h"
#include "elan_i2c_asus.h"

/*
 * global variable
 */
static unsigned int asusdec_apwake_gpio = TEGRA_GPIO_PS7;
static unsigned int asusdec_ecreq_gpio = TEGRA_GPIO_PQ6;
static unsigned int asusdec_dock_in_gpio = TEGRA_GPIO_PU4;
static unsigned int asusdec_hall_sensor_gpio = TEGRA_GPIO_PS6;

static struct i2c_client dockram_client;
static struct class *asusdec_class;
static struct device *asusdec_device;
static struct asusdec_chip *ec_chip;

static dev_t asusdec_dev;
static int asusdec_major = 0;
static int asusdec_minor = 0;

static struct workqueue_struct *asusdec_wq;

static int asusdec_kp_sci_table[]={0, KEY_SLEEP, KEY_WLAN, KEY_BLUETOOTH,
		ASUSDEC_KEY_TOUCHPAD, KEY_BRIGHTNESSDOWN, KEY_BRIGHTNESSUP, ASUSDEC_KEY_AUTOBRIGHT,
		KEY_CAMERA, -9, -10, -11,
		-12, -13, -14, -15,
		KEY_WWW, ASUSDEC_KEY_SETTING, KEY_PREVIOUSSONG, KEY_PLAYPAUSE,
		KEY_NEXTSONG, KEY_MUTE, KEY_VOLUMEDOWN, KEY_VOLUMEUP};

/* Function keys */
static int asusdec_kp_sci_table_fn[]={0, KEY_DELETE, KEY_F1, KEY_F2,
		KEY_F3, KEY_F4, KEY_F5, KEY_F6,
		KEY_F7, -9, -10, -11,
		-12, -13, -14, -15,
		KEY_F8, KEY_F9, KEY_F10, KEY_F11,
		KEY_F12, KEY_INSERT, KEY_VOLUMEDOWN, KEY_VOLUMEUP};

#define KEY_FLAGS_FN_LOCK 1		/* Fn keys without modifier */
#define KEY_FLAGS_BACK_AS_ESC 2
#define KEY_FLAGS_SEARCH_AS_LEFTALT 4
#define KEY_FLAGS_HOME_AS_LEFTMETA 8	/* aka Super_L or "Windows key" */

int key_flags = 0;
module_param(key_flags, int, 0644);

int key_autorepeat = 0;
static int key_autorepeat_set(const char *arg, const struct kernel_param *kp)
{
	int ret = param_set_int(arg, kp);

	if (ret == 0) {
		if (ec_chip && ec_chip->indev) {
			if (key_autorepeat)
				set_bit(EV_REP, ec_chip->indev->evbit);
			else
				clear_bit(EV_REP, ec_chip->indev->evbit);
		}
	}

	return ret;
}

static struct kernel_param_ops key_autorepeat_ops = {
	.set = key_autorepeat_set,
	.get = param_get_int,
};
module_param_cb(key_autorepeat, &key_autorepeat_ops, &key_autorepeat, 0644);

/*
 * functions definition
 */
static void asusdec_dockram_init(struct i2c_client *client)
{
	dockram_client.adapter = client->adapter;
	dockram_client.addr = 0x1b;
	dockram_client.detected = client->detected;
	dockram_client.dev = client->dev;
	dockram_client.driver = client->driver;
	dockram_client.flags = client->flags;
	strcpy(dockram_client.name, client->name);
}

static int asusdec_dockram_write_data(int cmd, int length)
{
	int ret = 0;

	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -1;

	ret = i2c_smbus_write_i2c_block_data(&dockram_client, cmd, length, ec_chip->i2c_dm_data);
	if (ret < 0)
		ASUSEC_ERR("Fail to read dockram data, status %d\n", ret);

	return ret;
}

static int asusdec_dockram_read_data(int cmd)
{
	int ret = 0;

	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -1;

	ret = i2c_smbus_read_i2c_block_data(&dockram_client, cmd, 32, ec_chip->i2c_dm_data);
	if (ret < 0)
		ASUSEC_ERR("Fail to read dockram data, status %d\n", ret);

	return ret;
}

static int asusdec_i2c_write_data(struct i2c_client *client, u16 data)
{
	int ret = 0;

	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -1;

	ret = i2c_smbus_write_word_data(client, 0x64, data);
	if (ret < 0)
		ASUSEC_ERR("Fail to write data, status %d\n", ret);

	return ret;
}

static int asusdec_i2c_read_data(struct i2c_client *client)
{
	int ret = 0;

	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -1;

	ret = i2c_smbus_read_i2c_block_data(client, 0x6A, 8, ec_chip->i2c_data);
	if (ret < 0)
		ASUSEC_ERR("Fail to read data, status %d\n", ret);

	return ret;
}

static int asusdec_keypad_get_response(struct i2c_client *client, int res)
{
	int retry = ASUSEC_RETRY_COUNT;

	while(retry-- > 0){
		asusdec_i2c_read_data(client);
		if ((ec_chip->i2c_data[1] & ASUSEC_OBF_MASK) &&
			(!(ec_chip->i2c_data[1] & ASUSEC_AUX_MASK))){
			if (ec_chip->i2c_data[2]  == res){
				goto get_asusdec_keypad_i2c;
			}
		}
		msleep(DELAY_TIME_MS/5);
	}
	return -1;

get_asusdec_keypad_i2c:
	return 0;

}

static int asusdec_keypad_enable(struct i2c_client *client)
{
	int retry = ASUSEC_RETRY_COUNT;

	while(retry-- > 0){
		asusdec_i2c_write_data(client, 0xF400);
		if(!asusdec_keypad_get_response(client, ASUSDEC_PS2_ACK)){
			goto keypad_enable_ok;
		}
	}
	ASUSEC_ERR("fail to enable keypad");
	return -1;

keypad_enable_ok:
	return 0;
}

static int asusdec_keypad_disable(struct i2c_client *client)
{
	int retry = ASUSEC_RETRY_COUNT;

	while(retry-- > 0){
		asusdec_i2c_write_data(client, 0xF500);
		if(!asusdec_keypad_get_response(client, ASUSDEC_PS2_ACK)){
			goto keypad_disable_ok;
		}
	}

	ASUSEC_ERR("fail to disable keypad");
	return -1;

keypad_disable_ok:
	return 0;
}

static void asusdec_keypad_led_on(struct work_struct *dat)
{
	ec_chip->kbc_value = 1;
	msleep(250);
	asusdec_i2c_write_data(ec_chip->client, 0xED00);
}


static void asusdec_keypad_led_off(struct work_struct *dat)
{
	ec_chip->kbc_value = 0;
	msleep(250);
	asusdec_i2c_write_data(ec_chip->client, 0xED00);
}


static int asusdec_touchpad_get_response(struct i2c_client *client, int res)
{
	int retry = ASUSEC_RETRY_COUNT;

	msleep(DELAY_TIME_MS);
	while(retry-- > 0){
		asusdec_i2c_read_data(client);
		if ((ec_chip->i2c_data[1] & ASUSEC_OBF_MASK) &&
			(ec_chip->i2c_data[1] & ASUSEC_AUX_MASK)){
			if (ec_chip->i2c_data[2] == res){
				goto get_asusdec_touchpad_i2c;
			}
		}
		msleep(DELAY_TIME_MS/5);
	}

	ASUSEC_ERR("fail to get touchpad response");
	return -1;

get_asusdec_touchpad_i2c:
	return 0;
}

static int asusdec_touchpad_enable(struct i2c_client *client)
{
	ec_chip->tp_wait_ack = 1;
	asusdec_i2c_write_data(client, 0xF4D4);
	return 0;
}

static int asusdec_touchpad_disable(struct i2c_client *client)
{
	int retry = 5;

	while(retry-- > 0){
		asusdec_i2c_write_data(client, 0xF5D4);
		if(!asusdec_touchpad_get_response(client, ASUSDEC_PS2_ACK)){
			goto touchpad_disable_ok;
		}
	}

	ASUSEC_ERR("fail to disable touchpad");
	return -1;

touchpad_disable_ok:
	return 0;
}

static int asusdec_i2c_test(struct i2c_client *client){
	return asusdec_i2c_write_data(client, 0x0000);
}

static void asusdec_reset_dock(void)
{
	ec_chip->dock_init = 0;
	ASUSEC_NOTICE("send EC_Request\n");
	gpio_set_value(asusdec_ecreq_gpio, 0);
	msleep(20);
	gpio_set_value(asusdec_ecreq_gpio, 1);
}

static int asusdec_is_init_running(void)
{
	int ret_val;

	mutex_lock(&ec_chip->dock_init_lock);
	ret_val = ec_chip->dock_init;
	ec_chip->dock_init = 1;
	mutex_unlock(&ec_chip->dock_init_lock);

	return ret_val;
}

static void asusdec_clear_i2c_buffer(struct i2c_client *client)
{
	int i;

	for (i = 0; i < 8; i++){
		asusdec_i2c_read_data(client);
	}
}

static void asusdec_dock_status_report(void){
	ASUSEC_INFO("dock_in = %d\n", !gpio_get_value(asusdec_dock_in_gpio));
#if BATTERY_DRIVER
	queue_delayed_work(asusdec_wq, &ec_chip->asusdec_pad_battery_report_work, 0);
#endif
}

static void asusdec_keypad_set_input_params(struct input_dev *dev)
{
	int i = 0;

	set_bit(EV_KEY, dev->evbit);

	for (i = 0; i < 246; i++)
		set_bit(i, dev->keybit);

	input_set_capability(dev, EV_LED, LED_CAPSL);
}

static int asusdec_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -ENOTTY;

	if ((type != EV_LED) || (code != LED_CAPSL))
		return -ENOTTY;

	if(value == 0)
		queue_delayed_work(asusdec_wq, &ec_chip->asusdec_led_off_work, 0);
	else
		queue_delayed_work(asusdec_wq, &ec_chip->asusdec_led_on_work, 0);

	return 0;
}

static int asusdec_input_device_create(struct i2c_client *client)
{
	int err = 0;

	if (ec_chip->indev)
		return 0;

	ec_chip->indev = input_allocate_device();
	if (!ec_chip->indev) {
		ASUSEC_ERR("input_dev allocation fails\n");
		err = -ENOMEM;
		goto exit;
	}

	ec_chip->indev->name = "asusdec";
	ec_chip->indev->phys = "/dev/input/asusdec";
	ec_chip->indev->dev.parent = &client->dev;
	ec_chip->indev->event = asusdec_event;

	asusdec_keypad_set_input_params(ec_chip->indev);
	err = input_register_device(ec_chip->indev);
	if (err) {
		ASUSEC_ERR("input registration fails\n");
		goto exit_input_free;
	}
	return 0;

exit_input_free:
	input_free_device(ec_chip->indev);
	ec_chip->indev = NULL;
exit:
	return err;
}

static int asusdec_chip_init(struct i2c_client *client)
{
	int ret_val = 0;
	int i;

	if(asusdec_is_init_running())
		return 0;

	wake_lock(&ec_chip->wake_lock);
	memset(ec_chip->ec_model_name, 0, 32);
	memset(ec_chip->ec_version, 0, 32);
	disable_irq_nosync(gpio_to_irq(asusdec_apwake_gpio));

	for (i = 0; i < 3; i++){
		ret_val = asusdec_i2c_test(client);
		if (ret_val < 0)
			msleep(1000);
		else
			break;
	}

	if(ret_val < 0)
		goto fail_to_access_ec;

	for (i = 0; i < 8; i++){
		asusdec_i2c_read_data(client);
	}

	if (asusdec_dockram_read_data(0x01) < 0)
		goto fail_to_access_ec;
	strcpy(ec_chip->ec_model_name, &ec_chip->i2c_dm_data[1]);
	ASUSEC_NOTICE("Model Name: %s\n", ec_chip->ec_model_name);

	if (asusdec_dockram_read_data(0x02) < 0)
		goto fail_to_access_ec;
	strcpy(ec_chip->ec_version, &ec_chip->i2c_dm_data[1]);
	ASUSEC_NOTICE("EC-FW Version: %s\n", ec_chip->ec_version);

	if (asusdec_dockram_read_data(0x03) < 0)
		goto fail_to_access_ec;
	ASUSEC_NOTICE("EC-Config Format: %s\n", &ec_chip->i2c_dm_data[1]);

	if (asusdec_dockram_read_data(0x04) < 0)
		goto fail_to_access_ec;
	strcpy(ec_chip->dock_pid, &ec_chip->i2c_dm_data[1]);
	ASUSEC_NOTICE("PID Version: %s\n", ec_chip->dock_pid);

	if(asusdec_input_device_create(client))
		goto fail_to_access_ec;

	if (ec_chip->init_success == 0)
		msleep(750);

	asusdec_clear_i2c_buffer(client);
	asusdec_touchpad_disable(client);
	asusdec_keypad_disable(client);

#if TOUCHPAD_MODE
	asusdec_clear_i2c_buffer(client);
	elantech_detect(ec_chip);
	elantech_init(ec_chip);
#endif

	ASUSEC_NOTICE("touchpad and keyboard init\n");
	ec_chip->d_index = 0;

	asusdec_keypad_enable(client);
	asusdec_clear_i2c_buffer(client);

	enable_irq(gpio_to_irq(asusdec_apwake_gpio));

	if (ec_chip->tp_enable)
		asusdec_touchpad_enable(client);

	ec_chip->init_success = 1;
	ec_chip->status = 1;
	asusdec_dock_status_report();
	wake_unlock(&ec_chip->wake_lock);
	return 0;

fail_to_access_ec:
	if (asusdec_dockram_read_data(0x00) < 0)
		ASUSEC_NOTICE("No EC detected\n");
	else
		ASUSEC_NOTICE("Need EC FW update\n");
	enable_irq(gpio_to_irq(asusdec_apwake_gpio));
	wake_unlock(&ec_chip->wake_lock);
	return -1;
}

static irqreturn_t asusdec_interrupt_handler(int irq, void *dev_id)
{
	if (irq == gpio_to_irq(asusdec_apwake_gpio)){
		disable_irq_nosync(irq);
		queue_delayed_work(asusdec_wq, &ec_chip->asusdec_work, 0);
	}

	if (irq == gpio_to_irq(asusdec_dock_in_gpio)){
		queue_delayed_work(asusdec_wq, &ec_chip->asusdec_dock_init_work, 0);
	}

	return IRQ_HANDLED;
}

static int asusdec_irq_dock_in(struct i2c_client *client)
{
	int rc = 0 ;
	unsigned gpio = asusdec_dock_in_gpio;
	unsigned irq = gpio_to_irq(asusdec_dock_in_gpio);
	const char* label = "asusdec_dock_in";

	/* Requesting gpio in case it wasn't requested before */
	rc = gpio_request(gpio, label);

	rc = gpio_direction_input(gpio);
	if (rc) {
		ASUSEC_ERR("gpio_direction_input failed for input %d\n", gpio);
		goto err_gpio_direction_input_failed;
	}

	rc = request_irq(irq, asusdec_interrupt_handler, IRQF_SHARED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, label, client);
	if (rc < 0) {
		ASUSEC_ERR("Could not register for %s interrupt, irq = %d, rc = %d\n", label, irq, rc);
		rc = -EIO;
		goto err_gpio_request_irq_fail;
	}

	return 0 ;

err_gpio_request_irq_fail :
	gpio_free(gpio);
err_gpio_direction_input_failed:
	return rc;
}

static int asusdec_irq(struct i2c_client *client)
{
	int rc = 0;
	unsigned gpio = asusdec_apwake_gpio;
	unsigned irq = gpio_to_irq(asusdec_apwake_gpio);
	const char* label = "asusdec_input";

	rc = gpio_request(gpio, label);
	if (rc) {
		ASUSEC_ERR("gpio_request failed for input %d\n", gpio);
		goto err_request_input_gpio_failed;
	}

	rc = gpio_direction_input(gpio) ;
	if (rc) {
		ASUSEC_ERR("gpio_direction_input failed for input %d\n", gpio);
		goto err_gpio_direction_input_failed;
	}

	rc = request_irq(irq, asusdec_interrupt_handler, IRQF_TRIGGER_LOW, label, client);
	if (rc < 0) {
		ASUSEC_ERR("Could not register for %s interrupt, irq = %d, rc = %d\n", label, irq, rc);
		rc = -EIO;
		goto err_gpio_request_irq_fail ;
	}
	enable_irq_wake(irq);

	return 0 ;

err_gpio_request_irq_fail:
	gpio_free(gpio);
err_gpio_direction_input_failed:
err_request_input_gpio_failed:
	return rc;
}

static int asusdec_irq_ec_request(struct i2c_client *client)
{
	int rc = 0;
	unsigned gpio = asusdec_ecreq_gpio;
	const char* label = "asusdec_request";

	rc = gpio_request(gpio, label);
	if (rc) {
		ASUSEC_ERR("gpio_request failed for input %d\n", gpio);
		goto err_exit;
	}

	rc = gpio_direction_output(gpio, 1);
	if (rc) {
		ASUSEC_ERR("gpio_direction_output failed for input %d\n", gpio);
		goto err_exit;
	}

	return 0;

err_exit:
	return rc;
}

static int asusdec_kp_key_mapping(int x)
{
	switch (x){
		case ASUSDEC_KEYPAD_ESC:
			if (key_flags & KEY_FLAGS_BACK_AS_ESC)
				return KEY_ESC;
			else
				return KEY_BACK;

		case ASUSDEC_KEYPAD_KEY_WAVE:
			return KEY_GRAVE;

		case ASUSDEC_KEYPAD_KEY_1:
			return KEY_1;

		case ASUSDEC_KEYPAD_KEY_2:
			return KEY_2;

		case ASUSDEC_KEYPAD_KEY_3:
			return KEY_3;

		case ASUSDEC_KEYPAD_KEY_4:
			return KEY_4;

		case ASUSDEC_KEYPAD_KEY_5:
			return KEY_5;

		case ASUSDEC_KEYPAD_KEY_6:
			return KEY_6;

		case ASUSDEC_KEYPAD_KEY_7:
			return KEY_7;

		case ASUSDEC_KEYPAD_KEY_8:
			return KEY_8;

		case ASUSDEC_KEYPAD_KEY_9:
			return KEY_9;

		case ASUSDEC_KEYPAD_KEY_0:
			return KEY_0;

		case ASUSDEC_KEYPAD_KEY_MINUS:
			return KEY_MINUS;

		case ASUSDEC_KEYPAD_KEY_EQUAL:
			return KEY_EQUAL;

		case ASUSDEC_KEYPAD_KEY_BACKSPACE:
			return KEY_BACKSPACE;

		case ASUSDEC_KEYPAD_KEY_TAB:
			return KEY_TAB;

		case ASUSDEC_KEYPAD_KEY_Q:
			return KEY_Q;

		case ASUSDEC_KEYPAD_KEY_W:
			return KEY_W;

		case ASUSDEC_KEYPAD_KEY_E:
			return KEY_E;

		case ASUSDEC_KEYPAD_KEY_R:
			return KEY_R;

		case ASUSDEC_KEYPAD_KEY_T:
			return KEY_T;

		case ASUSDEC_KEYPAD_KEY_Y:
			return KEY_Y;

		case ASUSDEC_KEYPAD_KEY_U:
			return KEY_U;

		case ASUSDEC_KEYPAD_KEY_I:
			return KEY_I;

		case ASUSDEC_KEYPAD_KEY_O:
			return KEY_O;

		case ASUSDEC_KEYPAD_KEY_P:
			return KEY_P;

		case ASUSDEC_KEYPAD_KEY_LEFTBRACE:
			return KEY_LEFTBRACE;

		case ASUSDEC_KEYPAD_KEY_RIGHTBRACE:
			return KEY_RIGHTBRACE;

		case ASUSDEC_KEYPAD_KEY_BACKSLASH:
			return KEY_BACKSLASH;

		case ASUSDEC_KEYPAD_KEY_CAPSLOCK:
			return KEY_CAPSLOCK;

		case ASUSDEC_KEYPAD_KEY_A:
			return KEY_A;

		case ASUSDEC_KEYPAD_KEY_S:
			return KEY_S;

		case ASUSDEC_KEYPAD_KEY_D:
			return KEY_D;

		case ASUSDEC_KEYPAD_KEY_F:
			return KEY_F;

		case ASUSDEC_KEYPAD_KEY_G:
			return KEY_G;

		case ASUSDEC_KEYPAD_KEY_H:
			return KEY_H;

		case ASUSDEC_KEYPAD_KEY_J:
			return KEY_J;

		case ASUSDEC_KEYPAD_KEY_K:
			return KEY_K;

		case ASUSDEC_KEYPAD_KEY_L:
			return KEY_L;

		case ASUSDEC_KEYPAD_KEY_SEMICOLON:
			return KEY_SEMICOLON;

		case ASUSDEC_KEYPAD_KEY_APOSTROPHE:
			return KEY_APOSTROPHE;

		case ASUSDEC_KEYPAD_KEY_ENTER:
			return KEY_ENTER;

		case ASUSDEC_KEYPAD_KEY_LEFTSHIFT:
			return KEY_LEFTSHIFT;

		case ASUSDEC_KEYPAD_KEY_Z:
			return KEY_Z;

		case ASUSDEC_KEYPAD_KEY_X:
			return KEY_X;

		case ASUSDEC_KEYPAD_KEY_C:
			return KEY_C;

		case ASUSDEC_KEYPAD_KEY_V:
			return KEY_V;

		case ASUSDEC_KEYPAD_KEY_B:
			return KEY_B;

		case ASUSDEC_KEYPAD_KEY_N:
			return KEY_N;

		case ASUSDEC_KEYPAD_KEY_M:
			return KEY_M;

		case ASUSDEC_KEYPAD_KEY_COMMA:
			return KEY_COMMA;

		case ASUSDEC_KEYPAD_KEY_DOT:
			return KEY_DOT;

		case ASUSDEC_KEYPAD_KEY_SLASH:
			return KEY_SLASH;

		case ASUSDEC_KEYPAD_KEY_RIGHTSHIFT:
			return KEY_RIGHTSHIFT;

		case ASUSDEC_KEYPAD_KEY_LEFT:
			return KEY_LEFT;

		case ASUSDEC_KEYPAD_KEY_RIGHT:
			return KEY_RIGHT;

		case ASUSDEC_KEYPAD_KEY_UP:
			return KEY_UP;

		case ASUSDEC_KEYPAD_KEY_DOWN:
			return KEY_DOWN;

		case ASUSDEC_KEYPAD_RIGHTWIN:
			if (key_flags & KEY_FLAGS_SEARCH_AS_LEFTALT)
				return KEY_LEFTALT;
			else
				return KEY_SEARCH;

		case ASUSDEC_KEYPAD_LEFTCTRL:
			return KEY_LEFTCTRL;

		case ASUSDEC_KEYPAD_LEFTWIN:
			if (key_flags & KEY_FLAGS_HOME_AS_LEFTMETA)
				return KEY_LEFTMETA;
			else
				return KEY_HOMEPAGE;

		case ASUSDEC_KEYPAD_LEFTALT:
			return KEY_LEFTALT;

		case ASUSDEC_KEYPAD_KEY_SPACE:
			return KEY_SPACE;

		case ASUSDEC_KEYPAD_RIGHTALT:
			return KEY_RIGHTALT;

		case ASUSDEC_KEYPAD_WINAPP:
			return KEY_MENU;

		case ASUSDEC_KEYPAD_RIGHTCTRL:
			return KEY_RIGHTCTRL;

		case ASUSDEC_KEYPAD_HOME:
			return KEY_HOME;

		case ASUSDEC_KEYPAD_PAGEUP:
			return KEY_PAGEUP;

		case ASUSDEC_KEYPAD_PAGEDOWN:
			return KEY_PAGEDOWN;

		case ASUSDEC_KEYPAD_END:
			return KEY_END;

		//--- JP keys
		case ASUSDEC_YEN:
			return KEY_YEN;

		case ASUSDEC_RO:
			return KEY_RO;

		case ASUSDEC_MUHENKAN:
			return KEY_MUHENKAN;

		case ASUSDEC_HENKAN:
			return KEY_HENKAN;

		case ASUSDEC_HIRAGANA_KATAKANA:
			return KEY_KATAKANAHIRAGANA;

		//--- UK keys
		case ASUSDEC_EUROPE_2:
			return KEY_102ND;

		default:
			return -1;
	}
}

static void asusdec_reset_counter(unsigned long data){
	ec_chip->d_index = 0;
}

#if TOUCHPAD_MODE
static void asusdec_tp_abs(void)
{
	unsigned char SA1,A1,B1,SB1,C1,D1;
	static unsigned char SA1_O=0,A1_O=0,B1_O=0,SB1_O=0,C1_O=0,D1_O=0;
	static int Null_data_times = 0;

	if (ec_chip->tp_enable){
		SA1= ec_chip->ec_data[0];
		A1 = ec_chip->ec_data[1];
		B1 = ec_chip->ec_data[2];
		SB1= ec_chip->ec_data[3];
		C1 = ec_chip->ec_data[4];
		D1 = ec_chip->ec_data[5];
//		ASUSEC_INFO("SA1=0x%x A1=0x%x B1=0x%x SB1=0x%x C1=0x%x D1=0x%x \n",SA1,A1,B1,SB1,C1,D1);
		if ( (SA1 == 0xC4) && (A1 == 0xFF) && (B1 == 0xFF) &&
		     (SB1 == 0x02) && (C1 == 0xFF) && (D1 == 0xFF)){
			Null_data_times ++;
			goto asusdec_tp_abs_end;
		}

		if(!(SA1 == SA1_O && A1 == A1_O && B1 == B1_O &&
		   SB1 == SB1_O && C1 == C1_O && D1 == D1_O)) {
			elantech_report_absolute_to_related(ec_chip, &Null_data_times);
		}

asusdec_tp_abs_end:
		SA1_O = SA1;
		A1_O = A1;
		B1_O = B1;
		SB1_O = SB1;
		C1_O = C1;
		D1_O = D1;
	}
}
#endif

static void asusdec_touchpad_processing(void){
	int i;
	int length = 0;
	int tp_start = 0;

#if TOUCHPAD_MODE
	length = ec_chip->i2c_data[0];
	if (ec_chip->tp_wait_ack){
		ec_chip->tp_wait_ack = 0;
		tp_start = 1;
		ec_chip->d_index = 0;
	} else {
		tp_start = 0;
	}

	for(i = tp_start; i < length - 1 ; i++){
		ec_chip->ec_data[ec_chip->d_index] = ec_chip->i2c_data[i+2];
		ec_chip->d_index++;
		if (ec_chip->d_index == 6){
			asusdec_tp_abs();
			ec_chip->d_index = 0;
		}
	}

	if (ec_chip->d_index)
		mod_timer(&ec_chip->asusdec_timer,jiffies+(HZ * 1/20));
#endif
}

static void asusdec_dock_init_work_function(struct work_struct *dat)
{
	int gpio = asusdec_dock_in_gpio;

	wake_lock(&ec_chip->wake_lock_init);
	mutex_lock(&ec_chip->input_lock);

	if (gpio_get_value(gpio)){
		ASUSEC_NOTICE("No dock detected\n");

		ec_chip->init_success = 0;
		ec_chip->tp_enable = 1;

		memset(ec_chip->ec_model_name, 0, 32);
		memset(ec_chip->ec_version, 0, 32);

		if (ec_chip->indev){
			input_unregister_device(ec_chip->indev);
			ec_chip->indev = NULL;
		}

		if (ec_chip->private->abs_dev){
			input_unregister_device(ec_chip->private->abs_dev);
			ec_chip->private->abs_dev = NULL;
		}

		asusdec_dock_status_report();
	} else {
		ASUSEC_NOTICE("Dock-in detected\n");
		if (gpio_get_value(asusdec_hall_sensor_gpio) || (!ec_chip->status)){
			if (ec_chip->init_success == 0){
				ec_chip->susb_on = 1;
				msleep(200);
				asusdec_reset_dock();
			}
		} else {
			ASUSEC_NOTICE("Keyboard is closed\n");
		}
	}
	mutex_unlock(&ec_chip->input_lock);
	wake_unlock(&ec_chip->wake_lock_init);
}

static void asusdec_kp_wake(void)
{
	if (asusdec_input_device_create(ec_chip->client)){
		return;
	}

	input_report_key(ec_chip->indev, KEY_MENU, 1);
	input_sync(ec_chip->indev);
	input_report_key(ec_chip->indev, KEY_MENU, 0);
	input_sync(ec_chip->indev);
}

static void asusdec_kp_smi(void)
{
	if (ec_chip->i2c_data[2] == ASUSEC_SMI_HANDSHAKING){
		ASUSEC_NOTICE("ASUSEC_SMI_HANDSHAKING\n");
		ec_chip->ec_in_s3 = 0;
		if (ec_chip->susb_on){
			asusdec_chip_init(ec_chip->client);
		}
	} else if (ec_chip->i2c_data[2] == ASUSEC_SMI_RESET){
		ASUSEC_NOTICE("ASUSEC_SMI_RESET\n");
		ec_chip->init_success = 0;
		asusdec_dock_init_work_function(NULL);
	} else if (ec_chip->i2c_data[2] == ASUSEC_SMI_WAKE){
		asusdec_kp_wake();
		ASUSEC_NOTICE("ASUSEC_SMI_WAKE\n");
	} else if (ec_chip->i2c_data[2] == ASUSEC_SMI_ADAPTER_EVENT){
		ASUSEC_NOTICE("ASUSEC_SMI_ADAPTER_EVENT\n");
#if DOCK_USB
		fsl_dock_ec_callback();
#endif
	} else if (ec_chip->i2c_data[2] == ASUSEC_SMI_BACKLIGHT_ON){
		ASUSEC_NOTICE("ASUSEC_SMI_BACKLIGHT_ON\n");
		ec_chip->susb_on = 1;
		asusdec_reset_dock();
#if DOCK_USB
		tegra_usb3_smi_backlight_on_callback();
#endif
	}
}

static void asusdec_kp_kbc(void)
{
	if (ec_chip->i2c_data[2] == ASUSDEC_PS2_ACK){
		if (ec_chip->kbc_value == 0){
			ASUSEC_INFO("i2c_write 0x0000\n");
			asusdec_i2c_write_data(ec_chip->client, 0x0000);
		} else {
			ASUSEC_INFO("i2c_write 0x0400\n");
			asusdec_i2c_write_data(ec_chip->client, 0x0400);
		}
	}
}

static int are_fn_keys_active(struct input_dev *dev)
{
	return test_bit(KEY_RIGHTALT, dev->key) ^ !!(key_flags & KEY_FLAGS_FN_LOCK);
}

static void asusdec_kp_sci(void)
{
	int ec_signal = ec_chip->i2c_data[2];

	if (are_fn_keys_active(ec_chip->indev))
		ec_chip->keypad_data.input_keycode = asusdec_kp_sci_table_fn[ec_signal];
	else
		ec_chip->keypad_data.input_keycode = asusdec_kp_sci_table[ec_signal];

	if(ec_chip->keypad_data.input_keycode > 0){
		ASUSEC_INFO("input_keycode = 0x%x\n", ec_chip->keypad_data.input_keycode);
		input_report_key(ec_chip->indev, ec_chip->keypad_data.input_keycode, 1);
		input_sync(ec_chip->indev);
		input_report_key(ec_chip->indev, ec_chip->keypad_data.input_keycode, 0);
		input_sync(ec_chip->indev);
	} else {
		ASUSEC_INFO("Unknown ec_signal = 0x%x\n", ec_signal);
	}
}
static void asusdec_kp_key(void)
{
	int scancode = 0;

	if (ec_chip->i2c_data[2] == ASUSDEC_KEYPAD_KEY_EXTEND){
		ec_chip->keypad_data.extend = 1;
		ec_chip->bc = 3;
	} else {
		ec_chip->keypad_data.extend = 0;
		ec_chip->bc = 2;
	}

	if(ec_chip->i2c_data[ec_chip->bc] == ASUSDEC_KEYPAD_KEY_BREAK){
		ec_chip->keypad_data.value = 0;
		ec_chip->bc++;
	} else {
		ec_chip->keypad_data.value = 1;
	}

	if (ec_chip->keypad_data.extend == 1){
		scancode = ((ASUSDEC_KEYPAD_KEY_EXTEND << 8) | ec_chip->i2c_data[ec_chip->bc]);
	} else {
		scancode = ec_chip->i2c_data[ec_chip->bc];
	}

	if (ec_chip->i2c_data[0] == 6){
		if ((ec_chip->i2c_data[2] == 0xE0) &&
			(ec_chip->i2c_data[3] == 0xF0) &&
			(ec_chip->i2c_data[4] == 0x12)){
			scancode = ec_chip->i2c_data[5] << 8 | ec_chip->i2c_data[6];
			ec_chip->keypad_data.value = 1;
		} else if ((ec_chip->i2c_data[2] == 0xE0) &&
			(ec_chip->i2c_data[3] == 0xF0) &&
			(ec_chip->i2c_data[4] == 0x59)){
			scancode = ec_chip->i2c_data[5] << 8 | ec_chip->i2c_data[6];
			ec_chip->keypad_data.value = 1;
		}
	}
//	ASUSEC_INFO("scancode = 0x%x\n", scancode);
	ec_chip->keypad_data.input_keycode = asusdec_kp_key_mapping(scancode);

	if(ec_chip->keypad_data.input_keycode > 0){
//		ASUSEC_INFO("input_keycode = 0x%x, input_value = %d\n",
//				ec_chip->keypad_data.input_keycode, ec_chip->keypad_data.value);
		input_report_key(ec_chip->indev,
			ec_chip->keypad_data.input_keycode, ec_chip->keypad_data.value);
		input_sync(ec_chip->indev);
	} else {
		ASUSEC_INFO("Unknown scancode = 0x%x\n", scancode);
	}
}

static void asusdec_keypad_processing(void)
{
	if (ec_chip->i2c_data[1] & ASUSEC_KBC_MASK)
		asusdec_kp_kbc();
	else if (ec_chip->i2c_data[1] & ASUSEC_SCI_MASK)
		asusdec_kp_sci();
	else
		asusdec_kp_key();
}

#if BATTERY_DRIVER
static void asusdec_pad_battery_report_function(struct work_struct *dat)
{
	int ret_val = 0;
	int dock_in = !gpio_get_value(asusdec_dock_in_gpio);

	ret_val = docking_callback(dock_in);
	if (ret_val < 0)
		queue_delayed_work(asusdec_wq, &ec_chip->asusdec_pad_battery_report_work, 2*HZ);
}
#endif

static void asusdec_work_function(struct work_struct *dat)
{
	int gpio = asusdec_apwake_gpio;
	int irq = gpio_to_irq(gpio);
	int ret_val = 0;

	ret_val = asusdec_i2c_read_data(ec_chip->client);
	enable_irq(irq);

	if (ret_val < 0)
		return;

	if (ec_chip->i2c_data[1] & ASUSEC_OBF_MASK){
		if (ec_chip->i2c_data[1] & ASUSEC_SMI_MASK){
			asusdec_kp_smi();
			return ;
		}
	}

	mutex_lock(&ec_chip->input_lock);
	if (ec_chip->indev == NULL){
		mutex_unlock(&ec_chip->input_lock);
		return;
	}
	if (ec_chip->i2c_data[1] & ASUSEC_OBF_MASK){
		if (ec_chip->i2c_data[1] & ASUSEC_AUX_MASK){
			if (ec_chip->private->abs_dev)
				asusdec_touchpad_processing();
		} else {
			asusdec_keypad_processing();
		}
	}
	mutex_unlock(&ec_chip->input_lock);
}

static int asusdec_dock_battery_get_capacity(union power_supply_propval *val)
{
	int bat_percentage = 0;
	int ret_val = 0;

	val->intval = -1;

	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -1;

	if (ec_chip->ec_in_s3 && ec_chip->status)
		msleep(200);

	ret_val = asusdec_dockram_read_data(0x14);
	if (ret_val < 0)
		return -1;

	bat_percentage = (ec_chip->i2c_dm_data[14] << 8 ) | ec_chip->i2c_dm_data[13];
	bat_percentage = ((bat_percentage >= 100) ? 100 : bat_percentage);

	if(bat_percentage >70 && bat_percentage <80)
		bat_percentage -= 1;
	else if(bat_percentage >60 && bat_percentage <=70)
		bat_percentage -= 2;
	else if(bat_percentage >50 && bat_percentage <=60)
		bat_percentage -= 3;
	else if(bat_percentage >30 && bat_percentage <=50)
		bat_percentage -= 4;
	else if(bat_percentage >=0 && bat_percentage <=30)
		bat_percentage -= 5;

	bat_percentage = ((bat_percentage <= 0) ? 0 : bat_percentage);
	val->intval = bat_percentage;
	return 0;
}

static int asusdec_dock_battery_get_status(union power_supply_propval *val)
{
	int ret_val = 0;

	val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;

	if (!gpio_get_value(asusdec_dock_in_gpio) == 0)
		return -1;

	if (ec_chip->ec_in_s3 && ec_chip->status)
		msleep(200);

	ret_val = asusdec_dockram_read_data(0x0A);
	if (ret_val < 0)
		return -1;

	if (ec_chip->i2c_dm_data[1] & 0x4)
		val->intval = POWER_SUPPLY_STATUS_CHARGING;

	return 0;
}

static int asusdec_dock_battery_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	switch (psp) {
		case POWER_SUPPLY_PROP_CAPACITY:
			if(asusdec_dock_battery_get_capacity(val) < 0)
				goto error;
			break;
		case POWER_SUPPLY_PROP_STATUS:
			if(asusdec_dock_battery_get_status(val) < 0)
				goto error;
			break;
		default:
			return -EINVAL;
	}
	return 0;

error:
	return -EINVAL;
}

static enum power_supply_property asusdec_dock_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CAPACITY,
};

static struct power_supply asusdec_power_supply[] = {
	{
		.name		= "dock_battery",
		.type		= POWER_SUPPLY_TYPE_DOCK_BATTERY,
		.properties	= asusdec_dock_properties,
		.num_properties	= ARRAY_SIZE(asusdec_dock_properties),
		.get_property	= asusdec_dock_battery_get_property,
	},
};

static int __devinit asusdec_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0;

	ec_chip = kzalloc(sizeof (struct asusdec_chip), GFP_KERNEL);
	if (!ec_chip) {
		ASUSEC_ERR("Memory allocation (asusdec_chip) fails\n");
		err = -ENOMEM;
		goto exit;
	}

	ec_chip->private = kzalloc(sizeof(struct elantech_data), GFP_KERNEL);
	if (!ec_chip->private) {
		ASUSEC_ERR("Memory allocation (elantech_data) fails\n");
		err = -ENOMEM;
		goto exit;
	}

	ec_chip->client = client;
	i2c_set_clientdata(client, ec_chip);

	mutex_init(&ec_chip->input_lock);
	mutex_init(&ec_chip->dock_init_lock);

	init_timer(&ec_chip->asusdec_timer);
	ec_chip->asusdec_timer.function = asusdec_reset_counter;

	wake_lock_init(&ec_chip->wake_lock, WAKE_LOCK_SUSPEND, "asusdec_wake");
	wake_lock_init(&ec_chip->wake_lock_init, WAKE_LOCK_SUSPEND, "asusdec_wake_init");

	ec_chip->status = 0;
	ec_chip->dock_init = 0;
	ec_chip->d_index = 0;
	ec_chip->suspend_state = 0;
	ec_chip->init_success = 0;
	ec_chip->tp_wait_ack = 0;
	ec_chip->tp_enable = 1;
	ec_chip->ec_in_s3 = 1;
	ec_chip->susb_on = 1;
	ec_chip->indev = NULL;
	ec_chip->private->abs_dev = NULL;

	asusdec_dockram_init(client);

	err = power_supply_register(&client->dev, &asusdec_power_supply[0]);
	if (err){
		ASUSEC_ERR("fail to register power supply for dock\n");
		goto exit;
	}

	asusdec_wq = create_singlethread_workqueue("asusdec_wq");

#if BATTERY_DRIVER
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asusdec_pad_battery_report_work, asusdec_pad_battery_report_function);
#endif
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asusdec_work, asusdec_work_function);
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asusdec_dock_init_work, asusdec_dock_init_work_function);
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asusdec_led_on_work, asusdec_keypad_led_on);
	INIT_DELAYED_WORK_DEFERRABLE(&ec_chip->asusdec_led_off_work, asusdec_keypad_led_off);

	asusdec_irq_dock_in(client);
	asusdec_irq_ec_request(client);
	asusdec_irq(client);

	queue_delayed_work(asusdec_wq, &ec_chip->asusdec_dock_init_work, 0);

	pr_info("asusdec: probed\n");
	return 0;

exit:
	return err;
}

static int __devexit asusdec_remove(struct i2c_client *client)
{
	struct asusdec_chip *chip = i2c_get_clientdata(client);

	input_unregister_device(chip->indev);
	kfree(chip);
	return 0;
}

static int asusdec_suspend(struct device *dev)
{
	int ret_val;

	ec_chip->susb_on = 0;
	flush_workqueue(asusdec_wq);

	if (!gpio_get_value(asusdec_dock_in_gpio) && (ec_chip->ec_in_s3 == 0)){
		ret_val = asusdec_i2c_test(ec_chip->client);
		if(ret_val < 0){
			goto fail_to_access_ec;
		}

		asusdec_dockram_read_data(0x0A);

		ec_chip->i2c_dm_data[0] = 8;
		ec_chip->i2c_dm_data[5] = ec_chip->i2c_dm_data[5] & 0xDF;
		ec_chip->i2c_dm_data[5] = ec_chip->i2c_dm_data[5] | 0x22;
		ec_chip->i2c_dm_data[5] = ec_chip->i2c_dm_data[5] & 0x7F;

		asusdec_dockram_write_data(0x0A,9);
	}

fail_to_access_ec:
	flush_workqueue(asusdec_wq);
	ec_chip->suspend_state = 1;
	ec_chip->init_success = 0;
	ec_chip->ec_in_s3 = 1;
	pr_info("asusdec: suspended\n");
	return 0;
}

static int asusdec_resume(struct device *dev)
{
	if ((gpio_get_value(asusdec_dock_in_gpio) == 0) && gpio_get_value(asusdec_apwake_gpio)) {
		asusdec_reset_dock();
	}

	ec_chip->suspend_state = 0;
	wake_lock(&ec_chip->wake_lock_init);
	ec_chip->init_success = 0;
	queue_delayed_work(asusdec_wq, &ec_chip->asusdec_dock_init_work, 0);

	pr_info("asusdec: resumed\n");
	return 0;
}

int asusdec_is_ac_over_10v_callback(void)
{
	int ret;

	if (!gpio_get_value(asusdec_dock_in_gpio)){
		if(!ec_chip && !ec_chip->indev)
			goto not_ready;

		ret = asusdec_i2c_test(ec_chip->client);
		if(ret < 0)
			goto not_ready;

		ret = asusdec_dockram_read_data(0x0A);
		if(ret < 0)
			goto not_ready;

		return ec_chip->i2c_dm_data[1] & 0x20;
	}

not_ready:
	ASUSEC_NOTICE("Dock isn't ready\n");
	return -1;
}

static SIMPLE_DEV_PM_OPS(asusdec_dev_pm_ops, asusdec_suspend, asusdec_resume);

static const struct i2c_device_id asusdec_id[] = {
	{"asusdec", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, asusdec_id);

static struct i2c_driver asusdec_driver = {
	.class	= I2C_CLASS_HWMON,
	.probe	 = asusdec_probe,
	.remove	 = __devexit_p(asusdec_remove),
	.id_table = asusdec_id,
	.driver	 = {
		.name = "asusdec",
		.owner = THIS_MODULE,
		.pm = &asusdec_dev_pm_ops,
	},
};

static int __init asusdec_init(void)
{
	int err_code = 0;

	if (asusdec_major) {
		asusdec_dev = MKDEV(asusdec_major, asusdec_minor);
		err_code = register_chrdev_region(asusdec_dev, 1, "asusdec");
	} else {
		err_code = alloc_chrdev_region(&asusdec_dev, asusdec_minor, 1,"asusdec");
		asusdec_major = MAJOR(asusdec_dev);
	}

	err_code = i2c_add_driver(&asusdec_driver);
	if(err_code){
		ASUSEC_ERR("i2c_add_driver fail\n");
		goto i2c_add_driver_fail;
	}

	asusdec_class = class_create(THIS_MODULE, "asusdec");
	if(asusdec_class <= 0){
		ASUSEC_ERR("asusdec_class create fail\n");
		err_code = -1;
		goto class_create_fail;
	}

	asusdec_device = device_create(asusdec_class, NULL, MKDEV(asusdec_major, asusdec_minor), NULL, "asusdec");
	if(asusdec_device <= 0){
		ASUSEC_ERR("asusdec_device create fail\n");
		err_code = -1;
		goto device_create_fail;
	}

	pr_info("asusdec: initiaded\n");
	return 0;

device_create_fail :
	class_destroy(asusdec_class);
class_create_fail :
	i2c_del_driver(&asusdec_driver);
i2c_add_driver_fail :
	return err_code;
}
module_init(asusdec_init);

static void __exit asusdec_exit(void)
{
	device_destroy(asusdec_class, MKDEV(asusdec_major, asusdec_minor));
	class_destroy(asusdec_class);
	i2c_del_driver(&asusdec_driver);
	unregister_chrdev_region(asusdec_dev, 1);
}
module_exit(asusdec_exit);

MODULE_DESCRIPTION("ASUS Dock EC Driver");
MODULE_LICENSE("GPL");
