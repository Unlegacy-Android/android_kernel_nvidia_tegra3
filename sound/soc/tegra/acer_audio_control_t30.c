/*
 * acer_audio_control.c - for WM8903 codec and fm2018 voice processor.
 *
 * Copyright (C) 2011 Acer, Inc.
 * Author: Andyl Liu <Andyl_Liu@acer.com.tw>
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc-dapm.h>
#include <sound/soc-dai.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>

#include "acer_audio_control_t30.h"
#include "../codecs/wm8903.h"

#define AUDIO_CONTROL_DRIVER_NAME "acer-audio-control"

/* Enable log or not */
#if 1
#define ACER_DBG(fmt, arg...) printk(KERN_INFO "[AudioControl](%d): " fmt "\n", __LINE__, ## arg)
#else
#define ACER_DBG(fmt, arg...) do {} while (0)
#endif

/* Fops function */
static int acer_audio_control_open(struct inode *inode, struct file *file);
static int acer_audio_control_close(struct inode *inode, struct file *file);
static long acer_audio_control_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* Module function */
static int acer_audio_control_probe(struct platform_device *pdev);
static int acer_audio_control_remove(struct platform_device *pdev);

/* extern */
extern int get_headset_state(void);
#ifdef CONFIG_ACER_ES305
extern int a1026_i2c_read(char *rxData, int length);
extern int a1026_i2c_write(char *txData, int length);
extern ssize_t chk_wakeup_a1026(void);
#endif

extern struct acer_audio_data audio_data;

enum headset_state {
	BIT_NO_HEADSET = 0,
	BIT_HEADSET = (1 << 0),
	BIT_HEADSET_NO_MIC = (1 << 1),
};

int snd_soc_dapm_get_iconia_param(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	const char *pin = (const char *)kcontrol->private_value;

	mutex_lock(&codec->mutex);

	/* TODO: get iconia param. */
	audio_data.pin = pin;

	mutex_unlock(&codec->mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_dapm_get_iconia_param);

int snd_soc_dapm_put_iconia_param(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	const char *pin = (const char *)kcontrol->private_value;
	int is_mode_new = ucontrol->value.integer.value[0];

	mutex_lock(&codec->mutex);

	audio_data.pin = pin;

	switch_audio_table(is_mode_new);

	mutex_unlock(&codec->mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_dapm_put_iconia_param);

int snd_soc_dapm_info_iconia_param(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_dapm_info_iconia_param);

int switch_audio_table(int control_mode)
{
	ACER_DBG("audio source = %d", control_mode);

	audio_data.mode.control = control_mode;

	switch (audio_data.mode.control) {
		case VOICE_COMMUNICATION: /* For VOIP */
			/* TODO: input talbe for es305*/
			break;

		case VOICE_RECOGNITION: /* For CTS */
			break;

		case CAMCORDER:
			break;

		case MIC: /* For RECORD */
		case DEFAULT:
		default:
			break;
	}

	return 1;
}

/* fops for auxpga */
static const struct file_operations acer_audio_control_fops = {
	.owner			= THIS_MODULE,
	.open			= acer_audio_control_open,
	.release		= acer_audio_control_close,
	.unlocked_ioctl	= acer_audio_control_ioctl,
};

/* miscdevice driver register */
static struct miscdevice acer_audio_control_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= AUDIO_CONTROL_DRIVER_NAME,
	.fops	= &acer_audio_control_fops,
};

static int acer_audio_control_open(struct inode *inode, struct file *file)
{
	pr_info("[ACER_AUDIO_CONTROL] has been opened\n");
	return 0;
}

static int acer_audio_control_close(struct inode *inode, struct file *file)
{
	pr_info("[ACER_AUDIO_CONTROL] has been closed\n");
	return 0;
}

static long acer_audio_control_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err = 0;

	pr_info("[ACER_AUDIO_CONTROL] acer_audio_control ioctl \n");
	if (_IOC_TYPE(cmd) != ACER_AUDIO_CONTROL_IOCTL_MAGIC) {
		pr_err("[ACER_AUDIO_CONTROL] IOCTL: cmd magic type error\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOC_MAXNR) {
		pr_err("[ACER_AUDIO_CONTROL] IOCTL: cmd number error\n");
		return -ENOTTY;
	}
	if (_IOC_DIR(cmd) & _IOC_NONE) {
		err = !access_ok(VERIFY_WRITE,(void __user*)arg, _IOC_SIZE(cmd));
	}
	if (err) {
		pr_err("[ACER_AUDIO_CONTROL] IOCTL: cmd access right error\n");
		return -EFAULT;
	}

	switch(cmd) {
	case WM8903_SET_REG: {
		unsigned int reg;
		unsigned int value;
		struct wm8903_codec_info codec_info;

		pr_info("WM8903_SET_REG...\n");

		if (copy_from_user(&codec_info, (void __user *)arg, sizeof(struct wm8903_codec_info)))
			return -1;

		reg = 0xffff & codec_info.reg;
		value = 0xffff & codec_info.val;

		if (0 != snd_soc_write(audio_data.codec, reg, value))
			return -1;

		return 0;
	}

	case WM8903_GET_REG: {
		unsigned int reg;
		unsigned int value;
		struct wm8903_codec_info codec_info;

		if (copy_from_user(&codec_info, (void __user *)arg, sizeof(struct wm8903_codec_info)))
			return -1;

		reg = 0xffff & codec_info.reg;
		value = snd_soc_read(audio_data.codec, reg);

		codec_info.reg = reg;
		codec_info.val = value;

		pr_info("WM8903_GET_REG : reg=%x, val=%x...\n", codec_info.reg, codec_info.val);

		if (copy_to_user((void __user *) arg, &codec_info, sizeof(struct wm8903_codec_info)))
			return -1;

		return 0;
	}

#ifdef CONFIG_ACER_ES305
	case ES305_SET_CMD: {
		unsigned char cmd_str[4];
		struct es305_dsp_info dsp_info;

		chk_wakeup_a1026();

		if (copy_from_user(&dsp_info, (void __user *)arg, sizeof(struct es305_dsp_info)))
			return -1;

		cmd_str[0] = (dsp_info.cmd >> 8) & 0xFF;
		cmd_str[1] = dsp_info.cmd & 0xFF;
		cmd_str[2] = (dsp_info.val >> 8) & 0xFF;
		cmd_str[3] = dsp_info.val & 0xFF;

		pr_info("ES305_SET_CMD : %02x%02x%02x%02x...\n", cmd_str[0], cmd_str[1], cmd_str[2], cmd_str[3]);

		if (0 != a1026_i2c_write(cmd_str, 4))
			return -1;

		return 0;
	}

	case ES305_GET_CMD: {
		unsigned char cmd_str[4];
		struct es305_dsp_info dsp_info;

		chk_wakeup_a1026();

		if (copy_from_user(&dsp_info, (void __user *)arg, sizeof(struct es305_dsp_info)))
			return -1;

		cmd_str[0] = (dsp_info.cmd >> 8) & 0xFF;
		cmd_str[1] = dsp_info.cmd & 0xFF;
		cmd_str[2] = (dsp_info.val >> 8) & 0xFF;
		cmd_str[3] = dsp_info.val & 0xFF;

		if (0 != a1026_i2c_read(cmd_str, 4))
			return -1;

		pr_info("ES305_GET_CMD : %02x%02x%02x%02x...\n", cmd_str[0], cmd_str[1], cmd_str[2], cmd_str[3]);

		dsp_info.cmd = ((cmd_str[0] << 8) & 0xFF) + cmd_str[1];
		dsp_info.val = ((cmd_str[2] << 8) & 0xFF) + cmd_str[3];

		if (copy_to_user((void __user *) arg, &dsp_info, sizeof(struct es305_dsp_info)))
			return -1;

		return 0;
	}
#endif

	default:
		pr_err("[WCD9310] IOCTL: Command not found!\n");
		return -1;
	}
}

/* platform driver register */
static struct platform_driver acer_audio_control_driver = {
	.probe  = acer_audio_control_probe,
	.remove = acer_audio_control_remove,
	.driver = {
		.name = AUDIO_CONTROL_DRIVER_NAME
	},
};

/* platform device register */
static struct platform_device acer_audio_control_device = {
	.name = AUDIO_CONTROL_DRIVER_NAME,
};

static int acer_audio_control_probe(struct platform_device *pdev)
{
	if (misc_register(&acer_audio_control_dev)) {
		pr_err("acer_audio_control_probe: acer_audio_control_dev register failed\n");
		return -ENOMEM;
	}
	pr_info("[AudioControl] probe done.\n");
	return 0;
}

static int acer_audio_control_remove(struct platform_device *pdev)
{
	ACER_DBG("%s", __func__);
	return 0;
}

static int __init acer_audio_control_init(void)
{
	int ret;
	ACER_DBG("%s", __func__);
	ret = platform_driver_register(&acer_audio_control_driver);
	if (ret) {
		pr_err("[acer_audio_control_driver] failed to register!!\n");
		return ret;
	}
	return platform_device_register(&acer_audio_control_device);
}

static void __exit acer_audio_control_exit(void)
{
	platform_device_unregister(&acer_audio_control_device);
	platform_driver_unregister(&acer_audio_control_driver);
}

module_init(acer_audio_control_init);
module_exit(acer_audio_control_exit);

MODULE_DESCRIPTION("ACER AUDIO CONTROL DRIVER");
MODULE_LICENSE("GPL");
