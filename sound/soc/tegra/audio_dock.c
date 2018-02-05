/*
 * Audio dock.
 *
 * Copyright (C) 2012 ASUSTek Corporation.
 *
 * Authors:
 *  Sam Chen <sam_chen@asus.com>
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

#include <asm/mach-types.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <sound/core.h>
#include <sound/soc.h>
#include "../codecs/wm8903.h"
#include "../drivers/input/asusec/asusdec.h"
#include <mach/board-asus-t30-misc.h>
#include <asm/gpio.h>
#include "../gpio-names.h"

extern struct snd_soc_codec *wm8903_codec;
extern struct snd_soc_codec *rt5631_audio_codec;
static struct snd_soc_codec *audio_codec;

extern bool lineout_alive;
extern void set_lineout_state(bool);

int audio_stand_route(bool status)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if(audio_codec == NULL){
		printk("%s: audio_codec is NULL\n", __func__);
		return 0;
	}

	dapm = &audio_codec->dapm;

	if(snd_soc_dapm_get_pin_status(dapm, "Int Spk") ||
			 snd_soc_dapm_get_pin_status(dapm, "AUX")){
		if(status){
        	        printk("%s: audio stand lineout on\n", __func__);
	                snd_soc_dapm_enable_pin(dapm, "AUX");
	                snd_soc_dapm_disable_pin(dapm, "Int Spk");
        	        snd_soc_dapm_sync(dapm);
		}else{
			printk("%s: audio stand lineout off\n", __func__);
			snd_soc_dapm_disable_pin(dapm, "AUX");
			snd_soc_dapm_enable_pin(dapm, "Int Spk");
			snd_soc_dapm_sync(dapm);
		}
	}
	return 0;
}
EXPORT_SYMBOL(audio_stand_route);

int audio_dock_in_out(u8 status)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if(audio_codec == NULL){
		printk("%s: audio_codec is NULL\n", __func__);
		return 0;
	}

	dapm = &audio_codec->dapm;

	if(snd_soc_dapm_get_pin_status(dapm, "Int Spk") ||
			snd_soc_dapm_get_pin_status(dapm, "AUX")){
		if(status == AUDIO_DOCK ){
            printk("%s: audio_dock_in\n", __func__);
	        snd_soc_dapm_enable_pin(dapm, "AUX");
        	snd_soc_dapm_disable_pin(dapm, "Int Spk");
            snd_soc_dapm_sync(dapm);
		}else if(status == AUDIO_STAND ){
			if(gpio_get_value(TEGRA_GPIO_PX3) == 0){
				lineout_alive = true;
				audio_stand_route(true);
				set_lineout_state(true);
			}else{
				lineout_alive = false;
				audio_stand_route(false);
				set_lineout_state(false);
			}
		}else{
            printk("%s: audio_stand_dock_out\n", __func__);
	        snd_soc_dapm_disable_pin(dapm, "AUX");
	        snd_soc_dapm_enable_pin(dapm, "Int Spk");
        	snd_soc_dapm_sync(dapm);
		}
	}else if (snd_soc_dapm_get_pin_status(dapm, "Headphone Jack")){
		printk("%s: headphone is inserted\n", __func__);
		/* if headphone is inserted, we set lineout state
		 * to decide route to speaker or lineout after
		 * headphone is removed.
		 */
		if(gpio_get_value(TEGRA_GPIO_PX3) == 0){
			lineout_alive = true;
			set_lineout_state(true);
		}else{
			lineout_alive = false;
			set_lineout_state(false);
		}
	}
	return 0;
}
EXPORT_SYMBOL(audio_dock_in_out);

void audio_dock_init(void)
{
        switch(tegra3_get_project_id()){
		case TEGRA3_PROJECT_TF201:
		case TEGRA3_PROJECT_TF300TG:
		case TEGRA3_PROJECT_TF300TL:
		case TEGRA3_PROJECT_TF700T:
			audio_codec = rt5631_audio_codec;
			break;
		case TEGRA3_PROJECT_TF300T:
			audio_codec = wm8903_codec;
			break;
		default:
			break;
	}
}
EXPORT_SYMBOL(audio_dock_init);

MODULE_DESCRIPTION("Audio Dock utility code");
MODULE_LICENSE("GPL");

