/*
 * ov9740.c - OV9740 sensor driver
 *
 * Copyright (c) 2011, NVIDIA, All Rights Reserved.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

/**
 * SetMode Sequence for 1280x720. Phase 0. Sensor Dependent.
 * This sequence should put sensor in streaming mode for 1280x960
 * This is usually given by the FAE or the sensor vendor.
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <video/tegra_camera.h>
#include <media/ov9740.h>
#include "ov9740_setting.h"

struct ov9740_info {
	int mode;
	struct i2c_client *i2c_client;
	struct ov9740_platform_data *pdata;
};

static struct ov9740_info *info;
static bool inited = false;

#define OV9740_MAX_RETRIES  3

static int ov9740_read_reg(struct i2c_client *client, u16 addr, u8 *val)
{
	int err, retry = 0;
	struct i2c_msg msg[2];
	unsigned char data[3];

	if (!client->adapter)
		return -ENODEV;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	/* high byte goes out first */
	data[0] = (u8) (addr >> 8);
	data[1] = (u8) (addr & 0xff);

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + 2;

	do {
		err = i2c_transfer(client->adapter, msg, 2);
		if (err == 2) {
			*val = data[2];
			return 0;
		}

		retry++;
		pr_err("%s: i2c transfer failed, retrying addr=0x%04X\n", __func__, addr);
		msleep(10);
	} while (retry <= OV9740_MAX_RETRIES);

	return err;
}

static int ov9740_write_reg(struct i2c_client *client, u16 addr, u8 val)
{
	int err, retry = 0;
	struct i2c_msg msg;
	unsigned char data[3];

	if (!client->adapter)
		return -ENODEV;

	data[0] = (u8) (addr >> 8);
	data[1] = (u8) (addr & 0xff);
	data[2] = (u8) val;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;

		retry++;
		pr_err("%s: i2c transfer failed, retrying addr=0x%04X, val=0x%02X\n",
			__func__, addr, val);
		msleep(10);
	} while (retry <= OV9740_MAX_RETRIES);

	return err;
}

static int ov9740_write_table(struct ov9740_info *info, const struct ov9740_reg table[])
{
	int err;
	const struct ov9740_reg *next;

	pr_info("%s ++\n", __func__);

	for (next=table; next->op!=OV9740_TABLE_END; next++) {
		switch(next->op) {
			case OV9740_WRITE_REG:
				err = ov9740_write_reg(info->i2c_client, next->addr, next->val);
				if (err)
					return err;
				break;

			case OV9740_WRITE_REG_MASK:
				// when encountering WRITE_REG_MASK, a write operation with mask is introduced
				// the written value is stored in this entry
				// and the data mask is stored in the next entry with op type DATA_MASK
				// the original value is read out and only the bits under the mask will be modified
				{
					u8 temp, data, mask;

					data = next->val;
					mask = (next+1)->val;
					err = ov9740_read_reg(info->i2c_client, next->addr, &temp);
					if (err)
						return err;
					// clear and modify the bits under the mask
					temp = (temp & (~mask)) | (data & mask);
					err = ov9740_write_reg(info->i2c_client, next->addr, temp);
					if (err)
						return err;
				}
				break;

			case OV9740_DATA_MASK:
				break;

			case OV9740_WAIT_MS:
				msleep(next->val);
				break;

			default:
				pr_err("%s: invalid op %d\n", __func__, next->op);
				return -EINVAL;
		}
	}

	pr_info("%s --\n", __func__);
	return 0;
}

static int ov9740_set_mode(struct ov9740_info *info, struct ov9740_mode *mode)
{
	int sensor_mode;
	int err;

	pr_info("%s ++: xres=%d, yres=%d\n", __func__, mode->xres, mode->yres);

	if (mode->xres==1280 && mode->yres==720)
		sensor_mode = OV9740_MODE_1280x720;
	else {
		pr_err("%s: invalid resolution supplied to set mode xres=%d, yres=%d\n",
		       __func__, mode->xres, mode->yres);
		return -EINVAL;
	}

	if (info->mode == sensor_mode) {
		pr_info("%s --\n", __func__);
		return 0;
	}

	err = ov9740_write_table(info, mode_table[sensor_mode]);
	if (err)
		return err;

	info->mode = sensor_mode;

	pr_info("%s --\n", __func__);
	return 0;
}

static int ov9740_get_status(struct ov9740_info *info)
{
	pr_info("%s\n", __func__);

	return 0;
}

static int ov9740_set_white_balance(struct ov9740_info *info, int white_balance)
{
	int err = 0;

	pr_info("%s: white_balance = %d\n", __func__, white_balance);

	switch (white_balance) {
		case OV9740_WhiteBalance_Auto:
			err = ov9740_write_table(info, WhiteBalance_Auto);
			break;

		case OV9740_WhiteBalance_Incandescent:
			err = ov9740_write_table(info, WhiteBalance_Incandescent);
			break;

		case OV9740_WhiteBalance_Fluorescent:
			err = ov9740_write_table(info, WhiteBalance_Fluorescent);
			break;

		case OV9740_WhiteBalance_Daylight:
			err = ov9740_write_table(info, WhiteBalance_Daylight);
			break;

		case OV9740_WhiteBalance_CloudyDaylight:
			err = ov9740_write_table(info, WhiteBalance_CloudyDaylight);
			break;

		default:
			break;
	}

	return err;
}

static int ov9740_set_color_effect(struct ov9740_info *info, int color_effect)
{
	int err;

	pr_info("%s: color_effect = %d\n", __func__, color_effect);

	// 0x4201: control passed frame number
	// Step 1 : Stop the streaming after one more frame
	err = ov9740_write_reg(info->i2c_client, 0x4201, 0x01);

	// Step 2 : Skip 2 frames at minimum frame rate 15fps(waiting 140ms)
	msleep(140);

	switch (color_effect) {
		case OV9740_ColorEffect_Aqua:
			err = ov9740_write_table(info, ColorEffect_Aqua);
			break;

		case OV9740_ColorEffect_Mono:
			err = ov9740_write_table(info, ColorEffect_Mono);
			break;

		case OV9740_ColorEffect_Negative:
			err = ov9740_write_table(info, ColorEffect_Negative);
			break;

		case OV9740_ColorEffect_None:
			err = ov9740_write_table(info, ColorEffect_None);
			break;

		case OV9740_ColorEffect_Sepia:
			err = ov9740_write_table(info, ColorEffect_Sepia);
			break;

		default:
			break;
        }

	// Step 3 : start streaming
	err = ov9740_write_reg(info->i2c_client, 0x4201, 0x00);

	return err;
}

static int ov9740_set_exposure(struct ov9740_info *info, int exposure)
{
	int err = 0;

	pr_info("%s: exposure = %d\n", __func__, exposure);

	switch (exposure) {
		case OV9740_Exposure_Minus_Two:
			err = ov9740_write_table(info, Exposure_Minus_Two);
			break;

		case OV9740_Exposure_Minus_One:
			err = ov9740_write_table(info, Exposure_Minus_One);
			break;

		case OV9740_Exposure_Zero:
			err = ov9740_write_table(info, Exposure_Zero);
			break;

		case OV9740_Exposure_Plus_One:
			err = ov9740_write_table(info, Exposure_Plus_One);
			break;

		case OV9740_Exposure_Plus_Two:
			err = ov9740_write_table(info, Exposure_Plus_Two);
			break;

		default:
			break;
	}

	return err;
}

static int ov9740_get_exposure_time(struct ov9740_info *info, struct ov9740_rational *exposure_time)
{
	u8 reg_0202, reg_0203;

	ov9740_read_reg(info->i2c_client, 0x0202, &reg_0202);
	ov9740_read_reg(info->i2c_client, 0x0203, &reg_0203);

	// exposure_time = sec_per_line * exposure_lines
	// sec_per_line = 1 sec / (30 frames * 775 VTS per frame)
	// exposure_lines = 0x0202 bit[7:0], 0x0203 bit[7:0]
	exposure_time->numerator = (u32)reg_0202<<8 | (u32)reg_0203;
	exposure_time->denominator = 23250;
	pr_info("%s: exposure_lines = %lu\n", __func__, exposure_time->numerator);

	return 0;
}

static long ov9740_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct ov9740_info *info = file->private_data;

	switch (cmd) {
		case OV9740_IOCTL_SET_MODE:
		{
			struct ov9740_mode mode;
			if (copy_from_user(&mode,
					(const void __user *)arg,
					sizeof(struct ov9740_mode)))
				return -EFAULT;

			return ov9740_set_mode(info, &mode);
		}

		case OV9740_IOCTL_GET_STATUS:
			return ov9740_get_status(info);

		case OV9740_IOCTL_SET_WHITE_BALANCE:
			return ov9740_set_white_balance(info, arg);

		case OV9740_IOCTL_SET_COLOR_EFFECT:
			return ov9740_set_color_effect(info, arg);

		case OV9740_IOCTL_SET_EXPOSURE:
			return ov9740_set_exposure(info, arg);

		case OV9740_IOCTL_GET_EXPOSURE_TIME:
		{
			struct ov9740_rational exposure_time;
			ov9740_get_exposure_time(info, &exposure_time);
			if (copy_to_user((void __user *)arg,
					&exposure_time,
					sizeof(struct ov9740_rational)))
				return -EFAULT;

			return 0;
		}

		default:
			pr_err("%s: invalid cmd %u\n", __func__, cmd);
			return -EINVAL;
	}

	return 0;
}

static int ov9740_initialize(struct ov9740_info *info)
{
	int err;
	u8 high_byte = 0, low_byte = 0;
	u16 chip_id = 0;
	struct tegra_camera_clk_info clk_info;

	extern void extern_tegra_camera_enable_clk(void);
	extern void extern_tegra_camera_disable_clk(void);
	extern void extern_tegra_camera_clk_set_rate(struct tegra_camera_clk_info *);

	pr_info("%s ++\n", __func__);

	// set MCLK to 24MHz
	clk_info.id = TEGRA_CAMERA_MODULE_VI;
	clk_info.clk_id = TEGRA_CAMERA_VI_SENSOR_CLK;
	clk_info.rate = 24000000;
	extern_tegra_camera_clk_set_rate(&clk_info);

	// turn on MCLK and pull down PWDN pin
	extern_tegra_camera_enable_clk();
	if (info->pdata && info->pdata->power_on)
		info->pdata->power_on();

	// read chip_id
	ov9740_read_reg(info->i2c_client, 0x0000, &high_byte);
	ov9740_read_reg(info->i2c_client, 0x0001, &low_byte);

	chip_id = (u16)high_byte << 8 | (u16)low_byte;
	if (chip_id == 0x9740)
		pr_info("%s: chip_id = 0x%04X", __func__, chip_id);
	else {
		pr_err("%s: wrong chip_id = 0x%04X\n", __func__, chip_id);
		return -1;
	}

	// write initial setting
	err = ov9740_write_table(info, mode_table[OV9740_MODE_1280x720]);
	if (err)
		return err;
	info->mode = OV9740_MODE_1280x720;

	// pull high PWDN pin and turn off MCLK
	if (info->pdata && info->pdata->power_off)
		info->pdata->power_off();
	extern_tegra_camera_disable_clk();

	pr_info("%s --\n", __func__);
	return 0;
}

static int ov9740_open(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);

	if (!inited) {
		if (ov9740_initialize(info) != 0)
			return -ENODEV;
		inited = true;
		msleep(100);
	}

	file->private_data = info;
	if (info->pdata && info->pdata->power_on)
		info->pdata->power_on();

	return 0;
}

static int ov9740_release(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);

	if (info->pdata && info->pdata->power_off)
		info->pdata->power_off();
	file->private_data = NULL;

	return 0;
}

static const struct file_operations ov9740_fileops = {
	.owner          = THIS_MODULE,
	.open           = ov9740_open,
	.unlocked_ioctl = ov9740_ioctl,
	.release        = ov9740_release,
};

static struct miscdevice ov9740_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ov9740",
	.fops = &ov9740_fileops,
};

static int ov9740_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;

	pr_info("%s ++\n", __func__);

	info = kzalloc(sizeof(struct ov9740_info), GFP_KERNEL);
	if (!info) {
		pr_err("%s: unable to allocate memory\n", __func__);
		return -ENOMEM;
	}

	info->pdata = client->dev.platform_data;
	info->i2c_client = client;

	err = misc_register(&ov9740_device);
	if (err) {
		pr_err("%s: unable to register misc device\n", __func__);
		kfree(info);
		return err;
	}

	i2c_set_clientdata(client, info);

	pr_info("%s --\n", __func__);
	return 0;
}

static int ov9740_remove(struct i2c_client *client)
{
	struct ov9740_info *info;

	pr_info("%s\n", __func__);

	info = i2c_get_clientdata(client);
	misc_deregister(&ov9740_device);
	kfree(info);

	return 0;
}

static const struct i2c_device_id ov9740_id[] = {
	{ "ov9740", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, ov9740_id);

static struct i2c_driver ov9740_i2c_driver = {
	.driver   = {
		.name  = "ov9740",
		.owner = THIS_MODULE,
	},
	.probe    = ov9740_probe,
	.remove   = ov9740_remove,
	.id_table = ov9740_id,
};

module_i2c_driver(ov9740_i2c_driver);
