/*
 * rt5631.c  --  RT5631 ALSA Soc Audio driver
 *
 * Copyright 2011 Realtek Microelectronics
 *
 * Author: flove <flove@realtek.com>
 *
 * Based on WM8753.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include <linux/gpio.h>
#include <../gpio-names.h>
#include <mach/board-transformer-misc.h>
#include <../board-transformer.h>

#include "rt5631.h"

#if defined(CONFIG_SND_HWDEP) || defined(CONFIG_SND_HWDEP_MODULE)
#include "rt56xx_ioctl.h"
#endif

#define ENABLE_EQ (1)
#define ENABLE_ALC (1)

#define TF700T_PCB_ER1 (0x3)

struct rt5631_priv {
	int codec_version;
	int master;
	int sysclk;
	int rx_rate;
	int bclk_rate;
	int dmic_used_flag;
	int eq_mode;
	int pll_used_flag;
};

static int pw_ladc = 0;
static struct snd_soc_codec *rt5631_codec;
#if ENABLE_ALC
static bool spk_out_flag = false;
static bool ADC_flag = false;
static bool DMIC_flag = true;   //heaset = false;
#endif
struct snd_soc_codec *rt5631_audio_codec = NULL;
EXPORT_SYMBOL(rt5631_audio_codec) ;

static const u16 rt5631_reg[RT5631_VENDOR_ID2 + 1] = {
	[RT5631_SPK_OUT_VOL] = 0x8888,
	[RT5631_HP_OUT_VOL] = 0x8080,
	[RT5631_MONO_AXO_1_2_VOL] = 0xa080,
	[RT5631_AUX_IN_VOL] = 0x0808,
	[RT5631_ADC_REC_MIXER] = 0xf0f0,
	[RT5631_VDAC_DIG_VOL] = 0x0010,
	[RT5631_OUTMIXER_L_CTRL] = 0xffc0,
	[RT5631_OUTMIXER_R_CTRL] = 0xffc0,
	[RT5631_AXO1MIXER_CTRL] = 0x88c0,
	[RT5631_AXO2MIXER_CTRL] = 0x88c0,
	[RT5631_DIG_MIC_CTRL] = 0x3000,
	[RT5631_MONO_INPUT_VOL] = 0x8808,
	[RT5631_SPK_MIXER_CTRL] = 0xf8f8,
	[RT5631_SPK_MONO_OUT_CTRL] = 0xfc00,
	[RT5631_SPK_MONO_HP_OUT_CTRL] = 0x4440,
	[RT5631_SDP_CTRL] = 0x8000,
	[RT5631_MONO_SDP_CTRL] = 0x8000,
	[RT5631_STEREO_AD_DA_CLK_CTRL] = 0x2010,
	[RT5631_GEN_PUR_CTRL_REG] = 0x0e00,
	[RT5631_INT_ST_IRQ_CTRL_2] = 0x071a,
	[RT5631_MISC_CTRL] = 0x2040,
	[RT5631_DEPOP_FUN_CTRL_2] = 0x8000,
	[RT5631_SOFT_VOL_CTRL] = 0x07e0,
	[RT5631_ALC_CTRL_1] = 0x0206,
	[RT5631_ALC_CTRL_3] = 0x2000,
	[RT5631_PSEUDO_SPATL_CTRL] = 0x0553,
};

/**
 * rt5631_write_index - write index register of 2nd layer
 */
static void rt5631_write_index(struct snd_soc_codec *codec,
		unsigned int reg, unsigned int value)
{
	snd_soc_write(codec, RT5631_INDEX_ADD, reg);
	snd_soc_write(codec, RT5631_INDEX_DATA, value);
}

/**
 * rt5631_read_index - read index register of 2nd layer
 */
static unsigned int rt5631_read_index(struct snd_soc_codec *codec,
				unsigned int reg)
{
	unsigned int value;

	snd_soc_write(codec, RT5631_INDEX_ADD, reg);
	value = snd_soc_read(codec, RT5631_INDEX_DATA);

	return value;
}

static void rt5631_write_index_mask(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value, unsigned int mask)
{
	unsigned int reg_val;

	if (!mask)
		return;

	if (mask != 0xffff) {
		reg_val = rt5631_read_index(codec, reg);
		reg_val &= ~mask;
		reg_val |= (value & mask);
		rt5631_write_index(codec, reg, reg_val);
	} else {
		rt5631_write_index(codec, reg, value);
	}

	return;
}

static int rt5631_reset(struct snd_soc_codec *codec)
{
	return snd_soc_write(codec, RT5631_RESET, 0);
}

static int rt5631_volatile_register(struct snd_soc_codec *codec,
				    unsigned int reg)
{
	switch (reg) {
	case RT5631_RESET:
	case RT5631_INT_ST_IRQ_CTRL_2:
	case RT5631_INDEX_ADD:
	case RT5631_INDEX_DATA:
	case RT5631_EQ_CTRL:
		return 1;
	default:
		return 0;
	}
}

static int rt5631_readable_register(struct snd_soc_codec *codec,
				    unsigned int reg)
{
	switch (reg) {
	case RT5631_RESET:
	case RT5631_SPK_OUT_VOL:
	case RT5631_HP_OUT_VOL:
	case RT5631_MONO_AXO_1_2_VOL:
	case RT5631_AUX_IN_VOL:
	case RT5631_STEREO_DAC_VOL_1:
	case RT5631_MIC_CTRL_1:
	case RT5631_STEREO_DAC_VOL_2:
	case RT5631_ADC_CTRL_1:
	case RT5631_ADC_REC_MIXER:
	case RT5631_ADC_CTRL_2:
	case RT5631_VDAC_DIG_VOL:
	case RT5631_OUTMIXER_L_CTRL:
	case RT5631_OUTMIXER_R_CTRL:
	case RT5631_AXO1MIXER_CTRL:
	case RT5631_AXO2MIXER_CTRL:
	case RT5631_MIC_CTRL_2:
	case RT5631_DIG_MIC_CTRL:
	case RT5631_MONO_INPUT_VOL:
	case RT5631_SPK_MIXER_CTRL:
	case RT5631_SPK_MONO_OUT_CTRL:
	case RT5631_SPK_MONO_HP_OUT_CTRL:
	case RT5631_SDP_CTRL:
	case RT5631_MONO_SDP_CTRL:
	case RT5631_STEREO_AD_DA_CLK_CTRL:
	case RT5631_PWR_MANAG_ADD1:
	case RT5631_PWR_MANAG_ADD2:
	case RT5631_PWR_MANAG_ADD3:
	case RT5631_PWR_MANAG_ADD4:
	case RT5631_GEN_PUR_CTRL_REG:
	case RT5631_GLOBAL_CLK_CTRL:
	case RT5631_PLL_CTRL:
	case RT5631_INT_ST_IRQ_CTRL_1:
	case RT5631_INT_ST_IRQ_CTRL_2:
	case RT5631_GPIO_CTRL:
	case RT5631_MISC_CTRL:
	case RT5631_DEPOP_FUN_CTRL_1:
	case RT5631_DEPOP_FUN_CTRL_2:
	case RT5631_JACK_DET_CTRL:
	case RT5631_SOFT_VOL_CTRL:
	case RT5631_ALC_CTRL_1:
	case RT5631_ALC_CTRL_2:
	case RT5631_ALC_CTRL_3:
	case RT5631_PSEUDO_SPATL_CTRL:
	case RT5631_INDEX_ADD:
	case RT5631_INDEX_DATA:
	case RT5631_EQ_CTRL:
	case RT5631_VENDOR_ID:
	case RT5631_VENDOR_ID1:
	case RT5631_VENDOR_ID2:
		return 1;
	default:
		return 0;
	}
}

struct rt5631_init_reg {
	u8 reg;
	u16 val;
};

/*
 * speaker channel volume select SPKMIXER, 0DB by default
 * Headphone channel volume select OUTMIXER,0DB by default
 * AXO1/AXO2 channel volume select OUTMIXER,0DB by default
 * Record Mixer source from Mic1/Mic2 by default
 * Mic1/Mic2 boost 44dB by default
 * DAC_L-->OutMixer_L by default
 * DAC_R-->OutMixer_R by default
 * DAC-->SpeakerMixer
 * Speaker volume-->SPOMixer(L-->L,R-->R)
 * Speaker AMP ratio gain is 1.99X (5.99dB)
 * HP from OutMixer,speaker out from SpeakerOut Mixer
 * enable HP zero cross
 * change Mic1 & mic2 to differential mode
 */
static struct rt5631_init_reg init_list[] = {
	{RT5631_ADC_CTRL_1		, 0x8080},
	{RT5631_SPK_OUT_VOL		, 0xc7c7},
	{RT5631_HP_OUT_VOL		, 0xc5c5},
	{RT5631_MONO_AXO_1_2_VOL	, 0xe040},
	{RT5631_ADC_REC_MIXER		, 0xb0f0},
	{RT5631_MIC_CTRL_2		, 0x6600},
	{RT5631_OUTMIXER_L_CTRL		, 0xdfC0},
	{RT5631_OUTMIXER_R_CTRL		, 0xdfC0},
	{RT5631_SPK_MIXER_CTRL		, 0xd8d8},
	{RT5631_SPK_MONO_OUT_CTRL	, 0x6c00},
	{RT5631_GEN_PUR_CTRL_REG	, 0x7e00}, //Speaker AMP ratio gain is 1.99X (5.99dB)
	{RT5631_SPK_MONO_HP_OUT_CTRL	, 0x0000},
	{RT5631_MIC_CTRL_1		, 0x8000}, //change Mic1 to differential mode,mic2 to single end mode
	{RT5631_INT_ST_IRQ_CTRL_2	, 0x0f18},
	{RT5631_ALC_CTRL_1		, 0x0B00}, //ALC Attack time  = 170.667ms, Recovery time = 83.333us
	{RT5631_ALC_CTRL_3		, 0x2410}, //Enable for DAC path, Limit level = -6dBFS
	{RT5631_AXO2MIXER_CTRL		, 0x8860},
};
#define RT5631_INIT_REG_LEN ARRAY_SIZE(init_list)

/*
 * EQ parameter
 */
enum {
	NORMAL,
	CLUB,
	DANCE,
	LIVE,
	POP,
	ROCK,
	OPPO,
	TREBLE,
	BASS,
	TF201,
	TF300TG,
	TF700T,
	TF300TL,
};

typedef struct _HW_EQ_PRESET {
	u16 HwEqType;
	u16 EqValue[22];
	u16 HwEQCtrl;
	u16 EqInVol;
	u16 EqOutVol;
}HW_EQ_PRESET;

static HW_EQ_PRESET HwEq_Preset[] = {
/* EQ param reg :  0x0,    0x1,    0x2,    0x3,    0x4,    0x5,    0x6,    0x7,    0x8,    0x9,    0xA,    0xB,    0xC,    0xD,    0xE,    0xF      EQ control reg: 0x6e  */
	{NORMAL		, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 0x0000, 0x8000, 0x0003},
	{CLUB		, {0x1C10, 0x0000, 0xC1CC, 0x1E5D, 0x0699, 0xCD48, 0x188D, 0x0699, 0xC3B6, 0x1CD0, 0x0699, 0x0436, 0x0000, 0x0000, 0x0000, 0x0000}, 0x000E, 0x8000, 0x0003},
	{DANCE		, {0x1F2C, 0x095B, 0xC071, 0x1F95, 0x0616, 0xC96E, 0x1B11, 0xFC91, 0xDCF2, 0x1194, 0xFAF2, 0x0436, 0x0000, 0x0000, 0x0000, 0x0000}, 0x000F, 0x8000, 0x0003},
	{LIVE		, {0x1EB5, 0xFCB6, 0xC24A, 0x1DF8, 0x0E7C, 0xC883, 0x1C10, 0x0699, 0xDA41, 0x1561, 0x0295, 0x0436, 0x0000, 0x0000, 0x0000, 0x0000}, 0x000F, 0x8000, 0x0003},
	{POP		, {0x1EB5, 0xFCB6, 0xC1D4, 0x1E5D, 0x0E23, 0xD92E, 0x16E6, 0xFCB6, 0x0000, 0x0969, 0xF988, 0x0436, 0x0000, 0x0000, 0x0000, 0x0000}, 0x000F, 0x8000, 0x0003},
	{ROCK		, {0x1EB5, 0xFCB6, 0xC071, 0x1F95, 0x0424, 0xC30A, 0x1D27, 0xF900, 0x0C5D, 0x0FC7, 0x0E23, 0x0436, 0x0000, 0x0000, 0x0000, 0x0000}, 0x000F, 0x8000, 0x0003},
	{OPPO		, {0x0000, 0x0000, 0xCA4A, 0x17F8, 0x0FEC, 0xCA4A, 0x17F8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 0x000F, 0x8000, 0x0003},
	{TREBLE		, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x188D, 0x1699, 0x0000, 0x0000, 0x0000}, 0x0010, 0x8000, 0x0003},
	{BASS		, {0x1A43, 0x0C00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 0x0001, 0x8000, 0x0003},
	{TF201		, {0x0264, 0xFE43, 0xC111, 0x1EF8, 0x1D18, 0xC1EC, 0x1E61, 0xFA19, 0xC6B1, 0x1B54, 0x0FEC, 0x0D41, 0x095B, 0xC0B6, 0x1F4C, 0x1FA5}, 0x403F, 0x8003, 0x0004},
	{TF300TG	, {0x1CD0, 0x1D18, 0xC21C, 0x1E30, 0xF900, 0xC2C8, 0x1EC4, 0x095B, 0xCA22, 0x1C10, 0x1830, 0xF76D, 0x0FEC, 0xC130, 0x1ED6, 0x1F69}, 0x403F, 0x8004, 0x0005},
	{TF700T 	, {0x0264, 0xFE43, 0xC0E5, 0x1F2C, 0x0C73, 0xC19B, 0x1EB2, 0xFA19, 0xC5FC, 0x1C10, 0x095B, 0x1561, 0x0699, 0xC18B, 0x1E7F, 0x1F3D}, 0x402A, 0x8003, 0x0005},
	{TF300TL	, {0x1CD0, 0x1D18, 0xC21C, 0x1E30, 0xF900, 0xC2C8, 0x1EC4, 0x095B, 0xCA22, 0x1C10, 0x1830, 0xF76D, 0x0FEC, 0xC130, 0x1ED6, 0x1F69}, 0x403F, 0x8004, 0x0005},
};

static int rt5631_reg_init(struct snd_soc_codec *codec)
{
	int i;

	for (i = 0; i < RT5631_INIT_REG_LEN; i++)
		snd_soc_write(codec, init_list[i].reg, init_list[i].val);

	return 0;
}

static const DECLARE_TLV_DB_SCALE(out_vol_tlv, -4650, 150, 0);
static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -95625, 375, 0);
static const DECLARE_TLV_DB_SCALE(in_vol_tlv, -3450, 150, 0);
/* {0, +20, +24, +30, +35, +40, +44, +50, +52}dB */
static unsigned int mic_bst_tlv[] = {
	TLV_DB_RANGE_HEAD(7),
	0, 0, TLV_DB_SCALE_ITEM(0, 0, 0),
	1, 1, TLV_DB_SCALE_ITEM(2000, 0, 0),
	2, 2, TLV_DB_SCALE_ITEM(2400, 0, 0),
	3, 5, TLV_DB_SCALE_ITEM(3000, 500, 0),
	6, 6, TLV_DB_SCALE_ITEM(4400, 0, 0),
	7, 7, TLV_DB_SCALE_ITEM(5000, 0, 0),
	8, 8, TLV_DB_SCALE_ITEM(5200, 0, 0),
};

static int rt5631_dmic_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	ucontrol->value.integer.value[0] = rt5631->dmic_used_flag;

	return 0;
}

static int rt5631_dmic_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	rt5631->dmic_used_flag = ucontrol->value.integer.value[0];
	return 0;
}

static int rt5631_eq_sel_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	ucontrol->value.integer.value[0] = rt5631->eq_mode;

	return 0;
}

static void rt5631_update_eqmode(struct snd_soc_codec *codec, int mode)
{
	u16 HwEqIndex;

	if (NORMAL == mode) {
		/* In Normal mode, the EQ parameter is cleared,
		 * and hardware LP, BP1, BP2, BP3, HP1, HP2
		 * block control and EQ block are disabled.
		 */
		for (HwEqIndex = RT5631_EQ_BW_LOP; HwEqIndex <= RT5631_EQ_HPF_GAIN; HwEqIndex++)
			rt5631_write_index(codec, HwEqIndex,
				HwEq_Preset[mode].EqValue[HwEqIndex]);
		snd_soc_update_bits(codec, RT5631_EQ_CTRL, 0x003f, 0x0000);
		rt5631_write_index_mask(codec, RT5631_EQ_PRE_VOL_CTRL, 
				0x0000, 0x8000);
	} else {
		/* Fill and update EQ parameter,
		 * and EQ block are enabled.
		 */
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1, 
			RT5631_PWR_ADC_L_CLK, RT5631_PWR_ADC_L_CLK);
		rt5631_write_index_mask(codec, RT5631_EQ_PRE_VOL_CTRL,
					0x8000, 0x8000);
		snd_soc_write(codec, RT5631_EQ_CTRL, 0x8000);
		for (HwEqIndex = RT5631_EQ_BW_LOP; HwEqIndex <= RT5631_EQ_HPF_GAIN; HwEqIndex++)
			rt5631_write_index(codec, HwEqIndex,
				HwEq_Preset[mode].EqValue[HwEqIndex]);
		rt5631_write_index(codec, RT5631_EQ_PRE_VOL_CTRL, HwEq_Preset[mode].EqInVol); //set EQ input volume
		rt5631_write_index(codec, RT5631_EQ_POST_VOL_CTRL, HwEq_Preset[mode].EqOutVol); //set EQ output volume
		snd_soc_write(codec, RT5631_EQ_CTRL, HwEq_Preset[mode].HwEQCtrl | 0xc000);
		if((HwEq_Preset[mode].HwEQCtrl & 0x8000))
			snd_soc_update_bits(codec, RT5631_EQ_CTRL, 0xc000, 0x8000);
		else
			snd_soc_update_bits(codec, RT5631_EQ_CTRL, 0xc000, 0x0000);
		if(!pw_ladc)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1, RT5631_PWR_ADC_L_CLK, 0);
	}

	return;
}

static int rt5631_eq_sel_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	if (rt5631->eq_mode == ucontrol->value.integer.value[0])
		return 0;

	rt5631_update_eqmode(codec, ucontrol->value.enumerated.item[0]);
	rt5631->eq_mode = ucontrol->value.integer.value[0];

	return 0;
}

static int rt5631_get_gain(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

static int rt5631_set_gain(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	int ret = 0;

	mutex_lock(&codec->mutex);

	if(ucontrol->value.enumerated.item[0]){
		#if ENABLE_ALC
		printk("%s(): set ALC AMIC parameter\n", __func__);
		DMIC_flag = false;
		if(!spk_out_flag){
			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x000a);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe090);
			} else {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0004);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe084);
			}
		}
		#endif
		/* set heaset mic gain */
		printk("%s():set headset gain\n", __func__);
		if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
			snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x001f, 0x0005);
		else
			snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x001f, 0x0000);
	} else {
		#if ENABLE_ALC
		printk("%s(): set ALC DMIC parameter\n", __func__);
		DMIC_flag = true;
		if(!spk_out_flag){
			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x000e);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe099);
			} else {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0006);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe09a);
			}
		}
		#endif
		/* set dmic gain */
		printk("%s(): use codec for capture gain\n", __func__);
		if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
			snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x00ff, 0x0013);
		else
			snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x001f, 0x000f);    //boost 22.5dB
	}
	mutex_unlock(&codec->mutex);

	return ret;
}

/* EQ Mode */
static const char *rt5631_eq_src_sel[] = {"NORMAL", "CLUB", "DANCE", "LIVE", "POP",
				"ROCK", "OPPO", "TREBLE", "BASS"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_eq_src_enum, 4, 9, rt5631_eq_src_sel);

/* MIC Input Type */
static const char *rt5631_input_mode[] = {
	"Single ended", "Differential"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_mic1_mode_enum, RT5631_MIC_CTRL_1,
	RT5631_MIC1_DIFF_INPUT_SHIFT, rt5631_input_mode);

static const SOC_ENUM_SINGLE_DECL(
	rt5631_mic2_mode_enum, RT5631_MIC_CTRL_1,
	RT5631_MIC2_DIFF_INPUT_SHIFT, rt5631_input_mode);

/* MONO Input Type */
static const SOC_ENUM_SINGLE_DECL(
	rt5631_monoin_mode_enum, RT5631_MONO_INPUT_VOL,
	RT5631_MONO_DIFF_INPUT_SHIFT, rt5631_input_mode);

static const struct snd_kcontrol_new rt5631_snd_controls[] = {
	/* MIC */
	SOC_ENUM("MIC1 Mode Control",  rt5631_mic1_mode_enum),
	SOC_SINGLE_TLV("MIC1 Boost", RT5631_MIC_CTRL_2,
		RT5631_MIC1_BOOST_SHIFT, 8, 0, mic_bst_tlv),
	SOC_ENUM("MIC2 Mode Control", rt5631_mic2_mode_enum),
	SOC_SINGLE_TLV("MIC2 Boost", RT5631_MIC_CTRL_2,
		RT5631_MIC2_BOOST_SHIFT, 8, 0, mic_bst_tlv),

	/* MONO IN */
	SOC_ENUM("MONOIN Mode Control", rt5631_monoin_mode_enum),
	SOC_DOUBLE_TLV("MONOIN_RX Capture Volume", RT5631_MONO_INPUT_VOL,
			RT5631_L_VOL_SHIFT, RT5631_R_VOL_SHIFT,
			RT5631_VOL_MASK, 1, in_vol_tlv),

	/* AXI */
	SOC_DOUBLE_TLV("AXI Capture Volume", RT5631_AUX_IN_VOL,
			RT5631_L_VOL_SHIFT, RT5631_R_VOL_SHIFT,
			RT5631_VOL_MASK, 1, in_vol_tlv),
	/* DAC */
	SOC_DOUBLE_TLV("PCM Playback Volume", RT5631_STEREO_DAC_VOL_2,
			RT5631_L_VOL_SHIFT, RT5631_R_VOL_SHIFT,
			RT5631_DAC_VOL_MASK, 1, dac_vol_tlv),
	SOC_DOUBLE("PCM Playback Switch", RT5631_STEREO_DAC_VOL_1,
			RT5631_L_MUTE_SHIFT, RT5631_R_MUTE_SHIFT, 1, 1),
	/* AXO */
	SOC_SINGLE("AXO1 Playback Switch", RT5631_MONO_AXO_1_2_VOL,
				RT5631_L_MUTE_SHIFT, 1, 1),
	SOC_SINGLE("AXO2 Playback Switch", RT5631_MONO_AXO_1_2_VOL,
				RT5631_R_VOL_SHIFT, 1, 1),
	/* OUTVOL */
	SOC_DOUBLE("OUTVOL Channel Switch", RT5631_SPK_OUT_VOL,
		RT5631_L_EN_SHIFT, RT5631_R_EN_SHIFT, 1, 0),

	/* SPK */
	SOC_DOUBLE("Speaker Playback Switch", RT5631_SPK_OUT_VOL,
		RT5631_L_MUTE_SHIFT, RT5631_R_MUTE_SHIFT, 1, 1),
	SOC_DOUBLE_TLV("Speaker Playback Volume", RT5631_SPK_OUT_VOL,
		RT5631_L_VOL_SHIFT, RT5631_R_VOL_SHIFT, 39, 1, out_vol_tlv),

	/* MONO OUT */
	SOC_SINGLE("MONO Playback Switch", RT5631_MONO_AXO_1_2_VOL,
				RT5631_MUTE_MONO_SHIFT, 1, 1),
	/* HP */
	SOC_DOUBLE("HP Playback Switch", RT5631_HP_OUT_VOL,
		RT5631_L_MUTE_SHIFT, RT5631_R_MUTE_SHIFT, 1, 1),
	SOC_DOUBLE_TLV("HP Playback Volume", RT5631_HP_OUT_VOL,
		RT5631_L_VOL_SHIFT, RT5631_R_VOL_SHIFT,
		RT5631_VOL_MASK, 1, out_vol_tlv),

	/* DMIC */
	SOC_SINGLE_EXT("DMIC Switch", 0, 0, 1, 0,
		rt5631_dmic_get, rt5631_dmic_put),
	SOC_DOUBLE("DMIC Capture Switch", RT5631_DIG_MIC_CTRL,
		RT5631_DMIC_L_CH_MUTE_SHIFT,
		RT5631_DMIC_R_CH_MUTE_SHIFT, 1, 1),

	/* EQ Mode */
	SOC_ENUM_EXT("EQ Mode", rt5631_eq_src_enum,
		rt5631_eq_sel_get, rt5631_eq_sel_put),

	/* Set recording gain */
	SOC_SINGLE_BOOL_EXT("Recording Gain", 0,
		rt5631_get_gain, rt5631_set_gain),
};

static int check_sysclk1_source(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_GLOBAL_CLK_CTRL);
	return reg & RT5631_SYSCLK_SOUR_SEL_PLL;
}

static int check_dmic_used(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(source->codec);
	return rt5631->dmic_used_flag;
}

static int check_dacl_to_outmixl(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_OUTMIXER_L_CTRL);
	return !(reg & RT5631_M_DAC_L_TO_OUTMIXER_L);
}

static int check_dacr_to_outmixr(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_OUTMIXER_R_CTRL);
	return !(reg & RT5631_M_DAC_R_TO_OUTMIXER_R);
}

static int check_dacl_to_spkmixl(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_SPK_MIXER_CTRL);
	return !(reg & RT5631_M_DAC_L_TO_SPKMIXER_L);
}

static int check_dacr_to_spkmixr(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_SPK_MIXER_CTRL);
	return !(reg & RT5631_M_DAC_R_TO_SPKMIXER_R);
}

static int check_adcl_select(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_ADC_REC_MIXER);
	return !(reg & RT5631_M_MIC1_TO_RECMIXER_L);
}

static int check_adcr_select(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	unsigned int reg;

	reg = snd_soc_read(source->codec, RT5631_ADC_REC_MIXER);
	return !(reg & RT5631_M_MIC2_TO_RECMIXER_R);
}

/**
 * onebit_depop_power_stage - auto depop in power stage.
 * @enable: power on/off
 *
 * When power on/off headphone, the depop sequence is done by hardware.
 */
static void onebit_depop_power_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* enable one-bit depop function */
	snd_soc_update_bits(codec, RT5631_DEPOP_FUN_CTRL_2,
				RT5631_EN_ONE_BIT_DEPOP, 0);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, RT5631_SOFT_VOL_CTRL);
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		/* config one-bit depop parameter */
		rt5631_write_index(codec, RT5631_TEST_MODE_CTRL, 0x84c0);
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x309f);
		rt5631_write_index(codec, RT5631_CP_INTL_REG2, 0x6530);
		/* power on capless block */
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_2,
				RT5631_EN_CAP_FREE_DEPOP);
	} else {
		/* power off capless block */
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_2, 0);
		msleep(100);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);
}

/**
 * onebit_depop_mute_stage - auto depop in mute stage.
 * @enable: mute/unmute
 *
 * When mute/unmute headphone, the depop sequence is done by hardware.
 */
static void onebit_depop_mute_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* enable one-bit depop function */
	snd_soc_update_bits(codec, RT5631_DEPOP_FUN_CTRL_2,
				RT5631_EN_ONE_BIT_DEPOP, 0);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, RT5631_SOFT_VOL_CTRL);
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		schedule_timeout_uninterruptible(msecs_to_jiffies(10));
		/* config one-bit depop parameter */
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x307f);
		snd_soc_update_bits(codec, RT5631_HP_OUT_VOL,
				RT5631_L_MUTE | RT5631_R_MUTE, 0);
		msleep(300);
	} else {
		snd_soc_update_bits(codec, RT5631_HP_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE,
			RT5631_L_MUTE | RT5631_R_MUTE);
		msleep(100);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);
}

/**
 * onebit_depop_power_stage - step by step depop sequence in power stage.
 * @enable: power on/off
 *
 * When power on/off headphone, the depop sequence is done in step by step.
 */
static void depop_seq_power_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* depop control by register */
	snd_soc_update_bits(codec, RT5631_DEPOP_FUN_CTRL_2,
		RT5631_EN_ONE_BIT_DEPOP, RT5631_EN_ONE_BIT_DEPOP);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, RT5631_SOFT_VOL_CTRL);
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		/* config depop sequence parameter */
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x303e);

		/* power on headphone and charge pump */
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP |
			RT5631_PWR_HP_R_AMP,
			RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP |
			RT5631_PWR_HP_R_AMP);

		/* power on soft generator and depop mode2 */
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_DEPOP2_FOR_HP);
		msleep(100);

		/* stop depop mode */
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_HP_DEPOP_DIS, RT5631_PWR_HP_DEPOP_DIS);
	} else {
		/* config depop sequence parameter */
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x303F);
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_MUTE_UNMUTE_DEPOP |
			RT5631_PD_HPAMP_L_ST_UP | RT5631_PD_HPAMP_R_ST_UP);
		msleep(75);
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_PD_HPAMP_L_ST_UP |
			RT5631_PD_HPAMP_R_ST_UP);

		/* start depop mode */
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
				RT5631_PWR_HP_DEPOP_DIS, 0);

		/* config depop sequence parameter */
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_DEPOP2_FOR_HP |
			RT5631_PD_HPAMP_L_ST_UP | RT5631_PD_HPAMP_R_ST_UP);
		msleep(80);
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN);

		/* power down headphone and charge pump */
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP |
			RT5631_PWR_HP_R_AMP, 0);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);
}

/**
 * depop_seq_mute_stage - step by step depop sequence in mute stage.
 * @enable: mute/unmute
 *
 * When mute/unmute headphone, the depop sequence is done in step by step.
 */
static void depop_seq_mute_stage(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	/* depop control by register */
	snd_soc_update_bits(codec, RT5631_DEPOP_FUN_CTRL_2,
		RT5631_EN_ONE_BIT_DEPOP, RT5631_EN_ONE_BIT_DEPOP);

	/* keep soft volume and zero crossing setting */
	soft_vol = snd_soc_read(codec, RT5631_SOFT_VOL_CTRL);
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = snd_soc_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		schedule_timeout_uninterruptible(msecs_to_jiffies(10));

		/* config depop sequence parameter */
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x302f);
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_MUTE_UNMUTE_DEPOP |
			RT5631_EN_HP_R_M_UN_MUTE_DEPOP |
			RT5631_EN_HP_L_M_UN_MUTE_DEPOP);

		snd_soc_update_bits(codec, RT5631_HP_OUT_VOL,
				RT5631_L_MUTE | RT5631_R_MUTE, 0);
		msleep(160);
	} else {
		/* config depop sequence parameter */
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x302f);
		snd_soc_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_MUTE_UNMUTE_DEPOP |
			RT5631_EN_HP_R_M_UN_MUTE_DEPOP |
			RT5631_EN_HP_L_M_UN_MUTE_DEPOP);

		snd_soc_update_bits(codec, RT5631_HP_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE,
			RT5631_L_MUTE | RT5631_R_MUTE);
		msleep(150);
	}

	/* recover soft volume and zero crossing setting */
	snd_soc_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);
}

static int hp_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		if (rt5631->codec_version) {
			onebit_depop_mute_stage(codec, 0);
			onebit_depop_power_stage(codec, 0);
		} else {
			depop_seq_mute_stage(codec, 0);
			depop_seq_power_stage(codec, 0);
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		if (rt5631->codec_version) {
			onebit_depop_power_stage(codec, 1);
			onebit_depop_mute_stage(codec, 1);
		} else {
			depop_seq_power_stage(codec, 1);
			depop_seq_mute_stage(codec, 1);
		}
		break;

	default:
		break;
	}

	return 0;
}

static int set_dmic_params(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	switch (rt5631->rx_rate) {
	case 44100:
	case 48000:
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,
			RT5631_DMIC_CLK_CTRL_MASK,
			RT5631_DMIC_CLK_CTRL_TO_32FS);
		break;

	case 32000:
	case 22050:
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,
			RT5631_DMIC_CLK_CTRL_MASK,
			RT5631_DMIC_CLK_CTRL_TO_64FS);
		break;

	case 16000:
	case 11025:
	case 8000:
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,
			RT5631_DMIC_CLK_CTRL_MASK,
			RT5631_DMIC_CLK_CTRL_TO_128FS);
		break;

	default:
		return -EINVAL;
	}

	snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,
		RT5631_DMIC_L_CH_MUTE | RT5631_DMIC_R_CH_MUTE,
		RT5631_DMIC_L_CH_UNMUTE | RT5631_DMIC_R_CH_UNMUTE);

	return 0;
}

static const struct snd_kcontrol_new rt5631_recmixl_mixer_controls[] = {
	SOC_DAPM_SINGLE("OUTMIXL Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_OUTMIXL_RECMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_BST1 Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_MIC1_RECMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXILVOL Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_AXIL_RECMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MONOIN_RX Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_MONO_IN_RECMIXL_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_recmixr_mixer_controls[] = {
	SOC_DAPM_SINGLE("MONOIN_RX Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_MONO_IN_RECMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXIRVOL Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_AXIR_RECMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_MIC2_RECMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTMIXR Capture Switch", RT5631_ADC_REC_MIXER,
			RT5631_M_OUTMIXR_RECMIXR_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spkmixl_mixer_controls[] = {
	SOC_DAPM_SINGLE("RECMIXL Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_RECMIXL_SPKMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_P Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_MIC1P_SPKMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACL Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_DACL_SPKMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTMIXL Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_OUTMIXL_SPKMIXL_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spkmixr_mixer_controls[] = {
	SOC_DAPM_SINGLE("OUTMIXR Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_OUTMIXR_SPKMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACR Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_DACR_SPKMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_P Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_MIC2P_SPKMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXR Playback Switch", RT5631_SPK_MIXER_CTRL,
			RT5631_M_RECMIXR_SPKMIXR_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_outmixl_mixer_controls[] = {
	SOC_DAPM_SINGLE("RECMIXL Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_RECMIXL_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXR Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_RECMIXR_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACL Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_DACL_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_MIC1_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_MIC2_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MONOIN_RXP Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_MONO_INP_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXILVOL Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_AXIL_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXIRVOL Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_AXIR_OUTMIXL_BIT, 1, 1),
	SOC_DAPM_SINGLE("VDAC Playback Switch", RT5631_OUTMIXER_L_CTRL,
				RT5631_M_VDAC_OUTMIXL_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_outmixr_mixer_controls[] = {
	SOC_DAPM_SINGLE("VDAC Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_VDAC_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXIRVOL Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_AXIR_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("AXILVOL Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_AXIL_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MONOIN_RXN Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_MONO_INN_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_MIC2_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_MIC1_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACR Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_DACR_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXR Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_RECMIXR_OUTMIXR_BIT, 1, 1),
	SOC_DAPM_SINGLE("RECMIXL Playback Switch", RT5631_OUTMIXER_R_CTRL,
				RT5631_M_RECMIXL_OUTMIXR_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_AXO1MIX_mixer_controls[] = {
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_AXO1MIXER_CTRL,
				RT5631_M_MIC1_AXO1MIX_BIT , 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_AXO1MIXER_CTRL,
				RT5631_M_MIC2_AXO1MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLL Playback Switch", RT5631_AXO1MIXER_CTRL,
				RT5631_M_OUTMIXL_AXO1MIX_BIT , 1 , 1),
	SOC_DAPM_SINGLE("OUTVOLR Playback Switch", RT5631_AXO1MIXER_CTRL,
				RT5631_M_OUTMIXR_AXO1MIX_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_AXO2MIX_mixer_controls[] = {
	SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_AXO2MIXER_CTRL,
				RT5631_M_MIC1_AXO2MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_AXO2MIXER_CTRL,
				RT5631_M_MIC2_AXO2MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLL Playback Switch", RT5631_AXO2MIXER_CTRL,
				RT5631_M_OUTMIXL_AXO2MIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLR Playback Switch", RT5631_AXO2MIXER_CTRL,
				RT5631_M_OUTMIXR_AXO2MIX_BIT, 1 , 1),
};

static const struct snd_kcontrol_new rt5631_spolmix_mixer_controls[] = {
	SOC_DAPM_SINGLE("SPKVOLL Playback Switch", RT5631_SPK_MONO_OUT_CTRL,
				RT5631_M_SPKVOLL_SPOLMIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOLR Playback Switch", RT5631_SPK_MONO_OUT_CTRL,
				RT5631_M_SPKVOLR_SPOLMIX_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spormix_mixer_controls[] = {
	SOC_DAPM_SINGLE("SPKVOLL Playback Switch", RT5631_SPK_MONO_OUT_CTRL,
				RT5631_M_SPKVOLL_SPORMIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOLR Playback Switch", RT5631_SPK_MONO_OUT_CTRL,
				RT5631_M_SPKVOLR_SPORMIX_BIT, 1, 1),
};

static const struct snd_kcontrol_new rt5631_monomix_mixer_controls[] = {
	SOC_DAPM_SINGLE("OUTVOLL Playback Switch", RT5631_SPK_MONO_OUT_CTRL,
				RT5631_M_OUTVOLL_MONOMIX_BIT, 1, 1),
	SOC_DAPM_SINGLE("OUTVOLR Playback Switch", RT5631_SPK_MONO_OUT_CTRL,
				RT5631_M_OUTVOLR_MONOMIX_BIT, 1, 1),
};

static int spk_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	static int spkl_out_enable, spkr_out_enable;
	unsigned int rt531_dac_pwr = 0;
	unsigned int tf700t_pcb_id = 0;
	unsigned int reg_val;

	tf700t_pcb_id = tegra3_query_pcba_revision_pcbid();
	rt531_dac_pwr = (snd_soc_read(codec, RT5631_PWR_MANAG_ADD1) & 0x0300) >> 8;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		#if ENABLE_ALC
			printk("spk_event --ALC_SND_SOC_DAPM_POST_PMU\n");
			spk_out_flag = true;
			/* Enable ALC */
			switch (tegra3_get_project_id()) {
			case TEGRA3_PROJECT_TF201:
				snd_soc_write(codec,
					RT5631_GEN_PUR_CTRL_REG, 0x6e00);
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0B00);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0000);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0x6410);
				break;
			case TEGRA3_PROJECT_TF300TG:
			case TEGRA3_PROJECT_TF300TL:
				snd_soc_write(codec,
					RT5631_GEN_PUR_CTRL_REG, 0x6e00);
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0B00);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0000);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0x6510);
				break;
			case TEGRA3_PROJECT_TF700T:
				snd_soc_write(codec,
					RT5631_GEN_PUR_CTRL_REG, 0x7e00);
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0307);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0000);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0x6510);
				break;
			}
		#endif
		#if ENABLE_EQ
			if(rt531_dac_pwr == 0x3) {
				  snd_soc_update_bits(codec,RT5631_PWR_MANAG_ADD1, 0x8000, 0x8000); //enable IIS interface power
				  if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201) {
						rt5631_update_eqmode(codec, TF201);
				  } else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG) {
						rt5631_update_eqmode(codec, TF300TG);
				  } else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
						rt5631_update_eqmode(codec, TF700T);
				  } else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF300TL) {
						rt5631_update_eqmode(codec, TF300TL);
				  } else {
						rt5631_update_eqmode(codec, TF201);
				  }//enable EQ after power on DAC power
			}
		#endif
		if (!spkl_out_enable && !strcmp(w->name, "SPKL Amp")) {

			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201) {
				snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_L_VOL, 0x0700);
			} else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG) {
				snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_L_VOL, 0x0700);
			} else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
				snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_L_VOL, 0x0600);
			}
			if((tf700t_pcb_id == TF700T_PCB_ER1) &&
			   (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)){
			   snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_L_VOL, 0x0d00);
			   printk("%s: %s\n", __func__, "TF700T ER1 spk L ch vol = -7.5dB");
			}

			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_L_VOL, RT5631_PWR_SPK_L_VOL);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
					RT5631_PWR_CLASS_D, RT5631_PWR_CLASS_D);
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_L_MUTE, 0);
			spkl_out_enable = 1;
		}
		if (!spkr_out_enable && !strcmp(w->name, "SPKR Amp")) {

			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201) {
				snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_R_VOL, 0x0007);
			} else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG) {
				snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_R_VOL, 0x0007);
			} else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
				snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_R_VOL, 0x0006);
			}
			if((tf700t_pcb_id == TF700T_PCB_ER1) &&
				(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)) {
                snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL, RT5631_R_VOL, 0x000d);
                printk("%s: %s\n", __func__, "TF700T ER1 spk R ch vol = -7.5dB");
			}

			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_R_VOL, RT5631_PWR_SPK_R_VOL);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
					RT5631_PWR_CLASS_D, RT5631_PWR_CLASS_D);
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_R_MUTE, 0);
			spkr_out_enable = 1;
		}
		break;

	case SND_SOC_DAPM_POST_PMD:
		if (spkl_out_enable && !strcmp(w->name, "SPKL Amp")) {
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_L_MUTE, RT5631_L_MUTE);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_L_VOL, 0);
			spkl_out_enable = 0;
		}
		if (spkr_out_enable && !strcmp(w->name, "SPKR Amp")) {
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_R_MUTE, RT5631_R_MUTE);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_R_VOL, 0);
			spkr_out_enable = 0;
		}
		if (0 == spkl_out_enable && 0 == spkr_out_enable)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
					RT5631_PWR_CLASS_D, 0);
		break;

	case SND_SOC_DAPM_PRE_PMD:
		#if ENABLE_ALC
		printk("spk_event --ALC_SND_SOC_DAPM_PRE_PMD\n");
		spk_out_flag = false;
		if(!spk_out_flag && !ADC_flag) {
			//Disable ALC
			snd_soc_update_bits(codec, RT5631_ALC_CTRL_3, 0xf000, 0x2000);
		} else if(!spk_out_flag && DMIC_flag ) {
			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x000e);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe099);
			} else {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0006);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe09a);
			}
		} else if(!spk_out_flag && !DMIC_flag ) {
			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x000a);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe090);
			} else {
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0004);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe084);
			}
		}
		#endif
		#if ENABLE_EQ
			printk("spk_event --EQ_SND_SOC_DAPM_PRE_PMD\n");
		if(rt531_dac_pwr == 0x3) {
			rt5631_update_eqmode(codec,NORMAL);    //disable EQ before powerdown speaker power
		}
		#endif
		break;

	default:
		return 0;
	}

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T ||
	   tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG ||
	   tegra3_get_project_id() == TEGRA3_PROJECT_TF300TL) {
		rt5631_write_index(codec, 0x48, 0xF73C);
		reg_val = rt5631_read_index(codec, 0x48);
		printk("%s -codec index 0x48=0x%04X\n", __FUNCTION__, reg_val);
	}

	return 0;
}

/**
 * config_common_power - control all common power of codec system
 * @pmu: power up or not
 */
static int config_common_power(struct snd_soc_codec *codec, bool pmu)
{
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	unsigned int mux_val;
	static int ref_count = 0;

	if (pmu) {
		ref_count++;
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_REF,
			RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_REF);
		mux_val = snd_soc_read(codec, RT5631_SPK_MONO_HP_OUT_CTRL);
		if (!(mux_val & RT5631_HP_L_MUX_SEL_DAC_L))
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
				RT5631_PWR_DAC_L_TO_MIXER, RT5631_PWR_DAC_L_TO_MIXER);
		if (!(mux_val & RT5631_HP_R_MUX_SEL_DAC_R))
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
				RT5631_PWR_DAC_R_TO_MIXER, RT5631_PWR_DAC_R_TO_MIXER);
		if (rt5631->pll_used_flag)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD2,
						RT5631_PWR_PLL1, RT5631_PWR_PLL1);
	} else {
		ref_count--;
		if(ref_count == 0){
			printk("%s: Real powr down, ref_count = 0\n", __func__);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
				RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_REF |
				RT5631_PWR_DAC_L_TO_MIXER | RT5631_PWR_DAC_R_TO_MIXER, 0);
		}
		if (rt5631->pll_used_flag)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD2,
						RT5631_PWR_PLL1, 0);
	}

	return 0;
}

static int adc_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	static bool pmu;

	switch (event) {
	case SND_SOC_DAPM_POST_PMD:
		pw_ladc = 0;
		if (pmu) {
			config_common_power(codec, false);
			pmu = false;
		}
		break;

	case SND_SOC_DAPM_PRE_PMU:
		if (!pmu) {
			config_common_power(codec, true);
			pmu = true;
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		pw_ladc = 1;

		#if ENABLE_ALC
		printk("adc_event --ALC_SND_SOC_DAPM_POST_PMU\n");
		ADC_flag = true;
		if(!spk_out_flag && DMIC_flag ){
			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x000e);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe099);
			}else{
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0006);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe09a);
			}
		}else if(!spk_out_flag && !DMIC_flag ){
			if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x000a);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe090);
				snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x001f, 0x0005);
			}else{
				snd_soc_write(codec, RT5631_ALC_CTRL_1, 0x0207);
				snd_soc_write(codec, RT5631_ALC_CTRL_2, 0x0004);
				snd_soc_write(codec, RT5631_ALC_CTRL_3, 0xe084);
			}
		}
		msleep(1);
		snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x8080, 0x0000);

		#endif
		break;

	case SND_SOC_DAPM_PRE_PMD:
		#if ENABLE_ALC
		printk("adc_event --ALC_SND_SOC_DAPM_PRE_PMD\n");
		ADC_flag = false;
		if(!spk_out_flag ){
			//Disable ALC
			snd_soc_update_bits(codec, RT5631_ALC_CTRL_3, 0xf000, 0x2000);
			}
		#endif
		snd_soc_update_bits(codec, RT5631_ADC_CTRL_1, 0x8080, 0x8080);

		break;

	default:
		break;
	}

	return 0;
}

static int dac_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	static bool pmu;

	switch (event) {
	case SND_SOC_DAPM_POST_PMD:

		if (pmu) {
			config_common_power(codec, false);
			pmu = false;
		}
		break;

	case SND_SOC_DAPM_PRE_PMU:
		if (!pmu) {
			config_common_power(codec, true);
			pmu = true;
		}
		break;

	case SND_SOC_DAPM_PRE_PMD:
		#if ENABLE_ALC
		printk("dac_event --ALC_SND_SOC_DAPM_PRE_PMD\n");
		if(!spk_out_flag && !ADC_flag ){
			//Disable ALC
			snd_soc_update_bits(codec, RT5631_ALC_CTRL_3, 0xf000, 0x2000);
		}
		#endif
	break;

	default:
		break;
	}

	return 0;
}


/* SPO Left Channel Input */
static const char *rt5631_spol_src_sel[] = {
	"SPOLMIX", "MONOIN_RX", "VDAC", "DACL"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_spol_src_enum, RT5631_SPK_MONO_HP_OUT_CTRL,
	RT5631_SPK_L_MUX_SEL_SHIFT, rt5631_spol_src_sel);

static const struct snd_kcontrol_new rt5631_spol_mux_control =
	SOC_DAPM_ENUM("SPOL SRC", rt5631_spol_src_enum);

/* SPO Right Channel Input */
static const char *rt5631_spor_src_sel[] = {
	"SPORMIX", "MONOIN_RX", "VDAC", "DACR"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_spor_src_enum, RT5631_SPK_MONO_HP_OUT_CTRL,
	RT5631_SPK_R_MUX_SEL_SHIFT, rt5631_spor_src_sel);

static const struct snd_kcontrol_new rt5631_spor_mux_control =
	SOC_DAPM_ENUM("SPOR SRC", rt5631_spor_src_enum);

/* MONO Input */
static const char *rt5631_mono_src_sel[] = {"MONOMIX", "MONOIN_RX", "VDAC"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_mono_src_enum, RT5631_SPK_MONO_HP_OUT_CTRL,
	RT5631_MONO_MUX_SEL_SHIFT, rt5631_mono_src_sel);

static const struct snd_kcontrol_new rt5631_mono_mux_control =
	SOC_DAPM_ENUM("MONO SRC", rt5631_mono_src_enum);

/* Left HPO Input */
static const char *rt5631_hpl_src_sel[] = {"Left HPVOL", "Left DAC"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_hpl_src_enum, RT5631_SPK_MONO_HP_OUT_CTRL,
	RT5631_HP_L_MUX_SEL_SHIFT, rt5631_hpl_src_sel);

static const struct snd_kcontrol_new rt5631_hpl_mux_control =
	SOC_DAPM_ENUM("HPL SRC", rt5631_hpl_src_enum);

/* Right HPO Input */
static const char *rt5631_hpr_src_sel[] = {"Right HPVOL", "Right DAC"};

static const SOC_ENUM_SINGLE_DECL(
	rt5631_hpr_src_enum, RT5631_SPK_MONO_HP_OUT_CTRL,
	RT5631_HP_R_MUX_SEL_SHIFT, rt5631_hpr_src_sel);

static const struct snd_kcontrol_new rt5631_hpr_mux_control =
	SOC_DAPM_ENUM("HPR SRC", rt5631_hpr_src_enum);

static const struct snd_soc_dapm_widget rt5631_dapm_widgets[] = {
	/* PLL1 */
	SND_SOC_DAPM_SUPPLY("PLL1", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_PLL1_BIT, 0, NULL, 0),

	/* Input Side */
	/* Input Lines */
	SND_SOC_DAPM_INPUT("MIC1"),
	SND_SOC_DAPM_INPUT("MIC2"),
	SND_SOC_DAPM_INPUT("AXIL"),
	SND_SOC_DAPM_INPUT("AXIR"),
	SND_SOC_DAPM_INPUT("MONOIN_RXN"),
	SND_SOC_DAPM_INPUT("MONOIN_RXP"),
	SND_SOC_DAPM_INPUT("DMIC"),

	/* MICBIAS */
	SND_SOC_DAPM_MICBIAS("MIC Bias1", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MICBIAS1_VOL_BIT, 0),
	SND_SOC_DAPM_MICBIAS("MIC Bias2", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MICBIAS2_VOL_BIT, 0),

	/* Boost */
	SND_SOC_DAPM_PGA("MIC1 Boost", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MIC1_BOOT_GAIN_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("MIC2 Boost", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MIC2_BOOT_GAIN_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("MONOIN_RXP Boost", RT5631_PWR_MANAG_ADD4,
			RT5631_PWR_MONO_IN_P_VOL_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("MONOIN_RXN Boost", RT5631_PWR_MANAG_ADD4,
			RT5631_PWR_MONO_IN_N_VOL_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("AXIL Boost", RT5631_PWR_MANAG_ADD4,
			RT5631_PWR_AXIL_IN_VOL_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("AXIR Boost", RT5631_PWR_MANAG_ADD4,
			RT5631_PWR_AXIR_IN_VOL_BIT, 0, NULL, 0),

	/* MONO In */
	SND_SOC_DAPM_MIXER("MONO_IN", SND_SOC_NOPM, 0, 0, NULL, 0),

	/* REC Mixer */
	SND_SOC_DAPM_MIXER("RECMIXL Mixer", RT5631_PWR_MANAG_ADD2,
		RT5631_PWR_RECMIXER_L_BIT, 0,
		&rt5631_recmixl_mixer_controls[0],
		ARRAY_SIZE(rt5631_recmixl_mixer_controls)),
	SND_SOC_DAPM_MIXER("RECMIXR Mixer", RT5631_PWR_MANAG_ADD2,
		RT5631_PWR_RECMIXER_R_BIT, 0,
		&rt5631_recmixr_mixer_controls[0],
		ARRAY_SIZE(rt5631_recmixr_mixer_controls)),
	/* Because of record duplication for L/R channel,
	 * L/R ADCs need power up at the same time */
	SND_SOC_DAPM_MIXER("ADC Mixer", SND_SOC_NOPM, 0, 0, NULL, 0),

	/* DMIC */
	SND_SOC_DAPM_SUPPLY("DMIC Supply", RT5631_DIG_MIC_CTRL,
		RT5631_DMIC_ENA_SHIFT, 0,
		set_dmic_params, SND_SOC_DAPM_PRE_PMU),
	/* ADC Data Srouce */
	SND_SOC_DAPM_SUPPLY("Left ADC Select", RT5631_INT_ST_IRQ_CTRL_2,
			RT5631_ADC_DATA_SEL_MIC1_SHIFT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Right ADC Select", RT5631_INT_ST_IRQ_CTRL_2,
			RT5631_ADC_DATA_SEL_MIC2_SHIFT, 0, NULL, 0),

	/* ADCs */
	SND_SOC_DAPM_ADC_E("Left ADC", "HIFI Capture",
		RT5631_PWR_MANAG_ADD1, RT5631_PWR_ADC_L_CLK_BIT, 0,
		adc_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU |
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
	SND_SOC_DAPM_ADC_E("Right ADC", "HIFI Capture",
		RT5631_PWR_MANAG_ADD1, RT5631_PWR_ADC_R_CLK_BIT, 0,
		adc_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU |
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

	/* DAC and ADC supply power */
	SND_SOC_DAPM_SUPPLY("I2S", RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_MAIN_I2S_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("DAC REF", RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_DAC_REF_BIT, 0, NULL, 0),

	/* Output Side */
	/* DACs */
	SND_SOC_DAPM_DAC_E("Left DAC", "HIFI Playback",
		RT5631_PWR_MANAG_ADD1, RT5631_PWR_DAC_L_CLK_BIT, 0,
		dac_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU |
		SND_SOC_DAPM_PRE_PMD),
	SND_SOC_DAPM_DAC_E("Right DAC", "HIFI Playback",
		RT5631_PWR_MANAG_ADD1, RT5631_PWR_DAC_R_CLK_BIT, 0,
		dac_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU |
		SND_SOC_DAPM_PRE_PMD),
	SND_SOC_DAPM_DAC("Voice DAC", "Voice DAC Mono Playback",
				SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_PGA("Voice DAC Boost", SND_SOC_NOPM, 0, 0, NULL, 0),
	/* DAC supply power */
	SND_SOC_DAPM_SUPPLY("Left DAC To Mixer", RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_DAC_L_TO_MIXER_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Right DAC To Mixer", RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_DAC_R_TO_MIXER_BIT, 0, NULL, 0),

	/* Left SPK Mixer */
	SND_SOC_DAPM_MIXER("SPKMIXL Mixer", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_SPKMIXER_L_BIT, 0,
			&rt5631_spkmixl_mixer_controls[0],
			ARRAY_SIZE(rt5631_spkmixl_mixer_controls)),
	/* Left Out Mixer */
	SND_SOC_DAPM_MIXER("OUTMIXL Mixer", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_OUTMIXER_L_BIT, 0,
			&rt5631_outmixl_mixer_controls[0],
			ARRAY_SIZE(rt5631_outmixl_mixer_controls)),
	/* Right Out Mixer */
	SND_SOC_DAPM_MIXER("OUTMIXR Mixer", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_OUTMIXER_R_BIT, 0,
			&rt5631_outmixr_mixer_controls[0],
			ARRAY_SIZE(rt5631_outmixr_mixer_controls)),
	/* Right SPK Mixer */
	SND_SOC_DAPM_MIXER("SPKMIXR Mixer", RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_SPKMIXER_R_BIT, 0,
			&rt5631_spkmixr_mixer_controls[0],
			ARRAY_SIZE(rt5631_spkmixr_mixer_controls)),

	/* Volume Mux */
	SND_SOC_DAPM_PGA("Left SPKVOL Mux", RT5631_PWR_MANAG_ADD4,
		RT5631_PWR_SPK_L_VOL_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA_E("Left HPVOL Mux", RT5631_PWR_MANAG_ADD4,
		RT5631_PWR_HP_L_OUT_VOL_BIT, 0, NULL, 0,
		hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
	SND_SOC_DAPM_PGA("Left OUTVOL Mux", RT5631_PWR_MANAG_ADD4,
		RT5631_PWR_LOUT_VOL_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Right OUTVOL Mux", RT5631_PWR_MANAG_ADD4,
		RT5631_PWR_ROUT_VOL_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA_E("Right HPVOL Mux", RT5631_PWR_MANAG_ADD4,
		RT5631_PWR_HP_R_OUT_VOL_BIT, 0, NULL, 0,
		hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
	SND_SOC_DAPM_PGA("Right SPKVOL Mux", RT5631_PWR_MANAG_ADD4,
		RT5631_PWR_SPK_R_VOL_BIT, 0, NULL, 0),

	/* DAC To HP */
	SND_SOC_DAPM_PGA_S("Left DAC_HP", 0, SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA_S("Right DAC_HP", 0, SND_SOC_NOPM, 0, 0, NULL, 0),

	/* HP Depop */
	SND_SOC_DAPM_PGA_S("HP Depop", 1, SND_SOC_NOPM, 0, 0,
		hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

	/* AXO1 Mixer */
	SND_SOC_DAPM_MIXER("AXO1MIX Mixer", RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_AXO1MIXER_BIT, 0,
			&rt5631_AXO1MIX_mixer_controls[0],
			ARRAY_SIZE(rt5631_AXO1MIX_mixer_controls)),
	/* SPOL Mixer */
	SND_SOC_DAPM_MIXER("SPOLMIX Mixer", SND_SOC_NOPM, 0, 0,
			&rt5631_spolmix_mixer_controls[0],
			ARRAY_SIZE(rt5631_spolmix_mixer_controls)),
	/* MONO Mixer */
	SND_SOC_DAPM_MIXER("MONOMIX Mixer", RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_MONOMIXER_BIT, 0,
			&rt5631_monomix_mixer_controls[0],
			ARRAY_SIZE(rt5631_monomix_mixer_controls)),
	/* SPOR Mixer */
	SND_SOC_DAPM_MIXER("SPORMIX Mixer", SND_SOC_NOPM, 0, 0,
			&rt5631_spormix_mixer_controls[0],
			ARRAY_SIZE(rt5631_spormix_mixer_controls)),
	/* AXO2 Mixer */
	SND_SOC_DAPM_MIXER("AXO2MIX Mixer", RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_AXO2MIXER_BIT, 0,
			&rt5631_AXO2MIX_mixer_controls[0],
			ARRAY_SIZE(rt5631_AXO2MIX_mixer_controls)),

	/* Mux */
	SND_SOC_DAPM_MUX("SPOL Mux", SND_SOC_NOPM, 0, 0,
			&rt5631_spol_mux_control),
	SND_SOC_DAPM_MUX("SPOR Mux", SND_SOC_NOPM, 0, 0,
			&rt5631_spor_mux_control),
	SND_SOC_DAPM_MUX("MONO Mux", SND_SOC_NOPM, 0, 0,
			&rt5631_mono_mux_control),
	SND_SOC_DAPM_MUX("HPL Mux", SND_SOC_NOPM, 0, 0,
			&rt5631_hpl_mux_control),
	SND_SOC_DAPM_MUX("HPR Mux", SND_SOC_NOPM, 0, 0,
			&rt5631_hpr_mux_control),

	/* AMP */
	SND_SOC_DAPM_PGA_E("SPKL Amp", SND_SOC_NOPM, 0, 0, NULL, 0,
		spk_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
	SND_SOC_DAPM_PGA_E("SPKR Amp", SND_SOC_NOPM, 1, 0, NULL, 0,
		spk_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),

	/* AMP supply */
	SND_SOC_DAPM_SUPPLY("MONO Depop", RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_MONO_DEPOP_DIS_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Class D", RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_CLASS_D_BIT, 0, NULL, 0),

	/* Output Lines */
	SND_SOC_DAPM_OUTPUT("AUXO1"),
	SND_SOC_DAPM_OUTPUT("AUXO2"),
	SND_SOC_DAPM_OUTPUT("SPOL"),
	SND_SOC_DAPM_OUTPUT("SPOR"),
	SND_SOC_DAPM_OUTPUT("HPOL"),
	SND_SOC_DAPM_OUTPUT("HPOR"),
	SND_SOC_DAPM_OUTPUT("MONO"),
};

static const struct snd_soc_dapm_route rt5631_dapm_routes[] = {
	{"MIC1 Boost", NULL, "MIC1"},
	{"MIC2 Boost", NULL, "MIC2"},
	{"MONOIN_RXP Boost", NULL, "MONOIN_RXP"},
	{"MONOIN_RXN Boost", NULL, "MONOIN_RXN"},
	{"AXIL Boost", NULL, "AXIL"},
	{"AXIR Boost", NULL, "AXIR"},

	{"MONO_IN", NULL, "MONOIN_RXP Boost"},
	{"MONO_IN", NULL, "MONOIN_RXN Boost"},

	{"RECMIXL Mixer", "OUTMIXL Capture Switch", "OUTMIXL Mixer"},
	{"RECMIXL Mixer", "MIC1_BST1 Capture Switch", "MIC1 Boost"},
	{"RECMIXL Mixer", "AXILVOL Capture Switch", "AXIL Boost"},
	{"RECMIXL Mixer", "MONOIN_RX Capture Switch", "MONO_IN"},

	{"RECMIXR Mixer", "OUTMIXR Capture Switch", "OUTMIXR Mixer"},
	{"RECMIXR Mixer", "MIC2_BST2 Capture Switch", "MIC2 Boost"},
	{"RECMIXR Mixer", "AXIRVOL Capture Switch", "AXIR Boost"},
	{"RECMIXR Mixer", "MONOIN_RX Capture Switch", "MONO_IN"},

	{"ADC Mixer", NULL, "RECMIXL Mixer"},
	{"ADC Mixer", NULL, "RECMIXR Mixer"},

	{"Left ADC", NULL, "ADC Mixer"},
	{"Left ADC", NULL, "Left ADC Select", check_adcl_select},
	{"Left ADC", NULL, "PLL1", check_sysclk1_source},
	{"Left ADC", NULL, "I2S"},
	{"Left ADC", NULL, "DAC REF"},

	{"Right ADC", NULL, "ADC Mixer"},
	{"Right ADC", NULL, "Right ADC Select", check_adcr_select},
	{"Right ADC", NULL, "PLL1", check_sysclk1_source},
	{"Right ADC", NULL, "I2S"},
	{"Right ADC", NULL, "DAC REF"},

	{"DMIC", NULL, "DMIC Supply", check_dmic_used},
	{"Left ADC", NULL, "DMIC"},
	{"Right ADC", NULL, "DMIC"},

	{"Left DAC", NULL, "PLL1", check_sysclk1_source},
	{"Left DAC", NULL, "I2S"},
	{"Left DAC", NULL, "DAC REF"},
	{"Right DAC", NULL, "PLL1", check_sysclk1_source},
	{"Right DAC", NULL, "I2S"},
	{"Right DAC", NULL, "DAC REF"},

	{"Voice DAC Boost", NULL, "Voice DAC"},

	{"SPKMIXL Mixer", NULL, "Left DAC To Mixer", check_dacl_to_spkmixl},
	{"SPKMIXL Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
	{"SPKMIXL Mixer", "MIC1_P Playback Switch", "MIC1"},
	{"SPKMIXL Mixer", "DACL Playback Switch", "Left DAC"},
	{"SPKMIXL Mixer", "OUTMIXL Playback Switch", "OUTMIXL Mixer"},

	{"SPKMIXR Mixer", NULL, "Right DAC To Mixer", check_dacr_to_spkmixr},
	{"SPKMIXR Mixer", "OUTMIXR Playback Switch", "OUTMIXR Mixer"},
	{"SPKMIXR Mixer", "DACR Playback Switch", "Right DAC"},
	{"SPKMIXR Mixer", "MIC2_P Playback Switch", "MIC2"},
	{"SPKMIXR Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},

	{"OUTMIXL Mixer", NULL, "Left DAC To Mixer", check_dacl_to_outmixl},
	{"OUTMIXL Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
	{"OUTMIXL Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},
	{"OUTMIXL Mixer", "DACL Playback Switch", "Left DAC"},
	{"OUTMIXL Mixer", "MIC1_BST1 Playback Switch", "MIC1 Boost"},
	{"OUTMIXL Mixer", "MIC2_BST2 Playback Switch", "MIC2 Boost"},
	{"OUTMIXL Mixer", "MONOIN_RXP Playback Switch", "MONOIN_RXP Boost"},
	{"OUTMIXL Mixer", "AXILVOL Playback Switch", "AXIL Boost"},
	{"OUTMIXL Mixer", "AXIRVOL Playback Switch", "AXIR Boost"},
	{"OUTMIXL Mixer", "VDAC Playback Switch", "Voice DAC Boost"},

	{"OUTMIXR Mixer", NULL, "Right DAC To Mixer", check_dacr_to_outmixr},
	{"OUTMIXR Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
	{"OUTMIXR Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},
	{"OUTMIXR Mixer", "DACR Playback Switch", "Right DAC"},
	{"OUTMIXR Mixer", "MIC1_BST1 Playback Switch", "MIC1 Boost"},
	{"OUTMIXR Mixer", "MIC2_BST2 Playback Switch", "MIC2 Boost"},
	{"OUTMIXR Mixer", "MONOIN_RXN Playback Switch", "MONOIN_RXN Boost"},
	{"OUTMIXR Mixer", "AXILVOL Playback Switch", "AXIL Boost"},
	{"OUTMIXR Mixer", "AXIRVOL Playback Switch", "AXIR Boost"},
	{"OUTMIXR Mixer", "VDAC Playback Switch", "Voice DAC Boost"},

	{"Left SPKVOL Mux",  NULL, "SPKMIXL Mixer"},
	{"Left HPVOL Mux",  NULL, "OUTMIXL Mixer"},
	{"Left OUTVOL Mux",  NULL, "OUTMIXL Mixer"},
	{"Right OUTVOL Mux",  NULL, "OUTMIXR Mixer"},
	{"Right HPVOL Mux",  NULL, "OUTMIXR Mixer"},
	{"Right SPKVOL Mux",  NULL, "SPKMIXR Mixer"},

	{"AXO1MIX Mixer", "MIC1_BST1 Playback Switch", "MIC1 Boost"},
	{"AXO1MIX Mixer", "OUTVOLL Playback Switch", "Left OUTVOL Mux"},
	{"AXO1MIX Mixer", "OUTVOLR Playback Switch", "Right OUTVOL Mux"},
	{"AXO1MIX Mixer", "MIC2_BST2 Playback Switch", "MIC2 Boost"},

	{"AXO2MIX Mixer", "MIC1_BST1 Playback Switch", "MIC1 Boost"},
	{"AXO2MIX Mixer", "OUTVOLL Playback Switch", "Left OUTVOL Mux"},
	{"AXO2MIX Mixer", "OUTVOLR Playback Switch", "Right OUTVOL Mux"},
	{"AXO2MIX Mixer", "MIC2_BST2 Playback Switch", "MIC2 Boost"},

	{"SPOLMIX Mixer", "SPKVOLL Playback Switch", "Left SPKVOL Mux"},
	{"SPOLMIX Mixer", "SPKVOLR Playback Switch", "Right SPKVOL Mux"},

	{"SPORMIX Mixer", "SPKVOLL Playback Switch", "Left SPKVOL Mux"},
	{"SPORMIX Mixer", "SPKVOLR Playback Switch", "Right SPKVOL Mux"},

	{"MONOMIX Mixer", "OUTVOLL Playback Switch", "Left OUTVOL Mux"},
	{"MONOMIX Mixer", "OUTVOLR Playback Switch", "Right OUTVOL Mux"},

	{"SPOL Mux", "SPOLMIX", "SPOLMIX Mixer"},
	{"SPOL Mux", "MONOIN_RX", "MONO_IN"},
	{"SPOL Mux", "VDAC", "Voice DAC Boost"},
	{"SPOL Mux", "DACL", "Left DAC"},

	{"SPOR Mux", "SPORMIX", "SPORMIX Mixer"},
	{"SPOR Mux", "MONOIN_RX", "MONO_IN"},
	{"SPOR Mux", "VDAC", "Voice DAC Boost"},
	{"SPOR Mux", "DACR", "Right DAC"},

	{"MONO Mux", "MONOMIX", "MONOMIX Mixer"},
	{"MONO Mux", "MONOIN_RX", "MONO_IN"},
	{"MONO Mux", "VDAC", "Voice DAC Boost"},

	{"Right DAC_HP", NULL, "Right DAC"},
	{"Left DAC_HP", NULL, "Left DAC"},

	{"HPL Mux", "Left HPVOL", "Left HPVOL Mux"},
	{"HPL Mux", "Left DAC", "Left DAC_HP"},
	{"HPR Mux", "Right HPVOL", "Right HPVOL Mux"},
	{"HPR Mux", "Right DAC", "Right DAC_HP"},

	{"HP Depop", NULL, "HPL Mux"},
	{"HP Depop", NULL, "HPR Mux"},

	{"AUXO1", NULL, "AXO1MIX Mixer"},
	{"AUXO2", NULL, "AXO2MIX Mixer"},

	{"SPKL Amp", NULL, "SPOL Mux"},
	{"SPKR Amp", NULL, "SPOR Mux"},
	{"SPOL", NULL, "SPKL Amp"},
	{"SPOR", NULL, "SPKR Amp"},

	{"SPOL", NULL, "Class D"},
	{"SPOL", NULL, "SPOL Mux"},
	{"SPOR", NULL, "Class D"},
	{"SPOR", NULL, "SPOR Mux"},

	{"HPOL", NULL, "HP Depop"},
	{"HPOR", NULL, "HP Depop"},

	{"MONO", NULL, "MONO Depop"},
	{"MONO", NULL, "MONO Mux"},
};

struct coeff_clk_div {
	u32 mclk;
	u32 bclk;
	u32 rate;
	u16 reg_val;
};

/* PLL divisors */
struct pll_div {
	u32 pll_in;
	u32 pll_out;
	u16 reg_val;
};

static const struct pll_div codec_master_pll_div[] = {
	{2048000,  8192000,  0x0ea0},
	{3686400,  8192000,  0x4e27},
	{12000000,  8192000,  0x456b},
	{13000000,  8192000,  0x495f},
	{13100000,  8192000,  0x0320},
	{2048000,  11289600,  0xf637},
	{3686400,  11289600,  0x2f22},
	{12000000,  11289600,  0x3e2f},
	{13000000,  11289600,  0x4d5b},
	{13100000,  11289600,  0x363b},
	{2048000,  16384000,  0x1ea0},
	{3686400,  16384000,  0x9e27},
	{12000000,  16384000,  0x452b},
	{13000000,  16384000,  0x542f},
	{13100000,  16384000,  0x03a0},
	{2048000,  16934400,  0xe625},
	{3686400,  16934400,  0x9126},
	{12000000,  16934400,  0x4d2c},
	{13000000,  16934400,  0x742f},
	{13100000,  16934400,  0x3c27},
	{2048000,  22579200,  0x2aa0},
	{3686400,  22579200,  0x2f20},
	{12000000,  22579200,  0x7e2f},
	{13000000,  22579200,  0x742f},
	{13100000,  22579200,  0x3c27},
	{2048000,  24576000,  0x2ea0},
	{3686400,  24576000,  0xee27},
	{12000000,  24576000,  0x2915},
	{13000000,  24576000,  0x772e},
	{13100000,  24576000,  0x0d20},
	{26000000,  24576000,  0x2027},
	{26000000,  22579200,  0x392f},
	{24576000,  22579200,  0x0921},
	{24576000,  24576000,  0x02a0},
};

static const struct pll_div codec_slave_pll_div[] = {
	{256000,  2048000,  0x46f0},
	{256000,  4096000,  0x3ea0},
	{352800,  5644800,  0x3ea0},
	{512000,  8192000,  0x3ea0},
	{1024000,  8192000,  0x46f0},
	{705600,  11289600,  0x3ea0},
	{1024000,  16384000,  0x3ea0},
	{1411200,  22579200,  0x3ea0},
	{1536000,  24576000,  0x3ea0},
	{2048000,  16384000,  0x1ea0},
	{2822400,  22579200,  0x1ea0},
	{2822400,  45158400,  0x5ec0},
	{5644800,  45158400,  0x46f0},
	{3072000,  24576000,  0x1ea0},
	{3072000,  49152000,  0x5ec0},
	{6144000,  49152000,  0x46f0},
	{705600,  11289600,  0x3ea0},
	{705600,  8467200,  0x3ab0},
	{24576000,  24576000,  0x02a0},
	{1411200,  11289600,  0x1690},
	{2822400,  11289600,  0x0a90},
	{1536000,  12288000,  0x1690},
	{3072000,  12288000,  0x0a90},
};

static struct coeff_clk_div coeff_div[] = {
	/* sysclk is 256fs */
	{2048000,  8000 * 32,  8000, 0x1000},
	{2048000,  8000 * 64,  8000, 0x0000},
	{2822400,  11025 * 32,  11025,  0x1000},
	{2822400,  11025 * 64,  11025,  0x0000},
	{4096000,  16000 * 32,  16000,  0x1000},
	{4096000,  16000 * 64,  16000,  0x0000},
	{5644800,  22050 * 32,  22050,  0x1000},
	{5644800,  22050 * 64,  22050,  0x0000},
	{8192000,  32000 * 32,  32000,  0x1000},
	{8192000,  32000 * 64,  32000,  0x0000},
	{11289600,  44100 * 32,  44100,  0x1000},
	{11289600,  44100 * 64,  44100,  0x0000},
	{12288000,  48000 * 32,  48000,  0x1000},
	{12288000,  48000 * 64,  48000,  0x0000},
	{22579200,  88200 * 32,  88200,  0x1000},
	{22579200,  88200 * 64,  88200,  0x0000},
	{24576000,  96000 * 32,  96000,  0x1000},
	{24576000,  96000 * 64,  96000,  0x0000},
	/* sysclk is 384fs */
	{16934400, 44100 * 32, 44100, 0x1080},
	{16934400, 44100 * 64, 44100, 0x0080},
	{18432000, 48000 * 32, 48000, 0x1080},
	{18432000, 48000 * 64, 48000, 0x0080},
	/* sysclk is 512fs */
	{4096000,  8000 * 32,  8000, 0x3000},
	{4096000,  8000 * 64,  8000, 0x2000},
	{5644800,  11025 * 32,  11025, 0x3000},
	{5644800,  11025 * 64,  11025, 0x2000},
	{8192000,  16000 * 32,  16000, 0x3000},
	{8192000,  16000 * 64,  16000, 0x2000},
	{11289600,  22050 * 32,  22050, 0x3000},
	{11289600,  22050 * 64,  22050, 0x2000},
	{16384000,  32000 * 32,  32000, 0x3000},
	{16384000,  32000 * 64,  32000, 0x2000},
	{22579200,  44100 * 32,  44100, 0x3000},
	{22579200,  44100 * 64,  44100, 0x2000},
	{24576000,  48000 * 32,  48000, 0x3000},
	{24576000,  48000 * 64,  48000, 0x2000},
	{45158400,  88200 * 32,  88200, 0x3000},
	{45158400,  88200 * 64,  88200, 0x2000},
	{49152000,  96000 * 32,  96000, 0x3000},
	{49152000,  96000 * 64,  96000, 0x2000},
	/* sysclk is 24.576Mhz or 22.5792Mhz */
	{24576000,  8000 * 32,  8000,  0x7080},
	{24576000,  8000 * 64,  8000,  0x6080},
	{24576000,  16000 * 32,  16000,  0x5080},
	{24576000,  16000 * 64,  16000,  0x4080},
	{24576000,  24000 * 32,  24000,  0x5000},
	{24576000,  24000 * 64,  24000,  0x4000},
	{24576000,  32000 * 32,  32000,  0x3080},
	{24576000,  32000 * 64,  32000,  0x2080},
	{22579200,  11025 * 32,  11025,  0x7000},
	{22579200,  11025 * 64,  11025,  0x6000},
	{22579200,  22050 * 32,  22050,  0x5000},
	{22579200,  22050 * 64,  22050,  0x4000},
};

static int get_coeff(int mclk, int rate, int timesofbclk)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
		if (coeff_div[i].mclk == mclk && coeff_div[i].rate == rate &&
			(coeff_div[i].bclk / coeff_div[i].rate) == timesofbclk)
			return i;
	}
	return -EINVAL;
}

static int rt5631_hifi_pcm_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	int timesofbclk = 64, coeff;
	unsigned int iface = 0;

	dev_dbg(codec->dev, "enter %s\n", __func__);

	rt5631->bclk_rate = snd_soc_params_to_bclk(params);
	if (rt5631->bclk_rate < 0) {
		dev_err(codec->dev, "Fail to get BCLK rate\n");
		return rt5631->bclk_rate;
	}
	rt5631->rx_rate = params_rate(params);

	if (rt5631->master)
		coeff = get_coeff(rt5631->sysclk, rt5631->rx_rate,
			rt5631->bclk_rate / rt5631->rx_rate);
	else
		coeff = get_coeff(rt5631->sysclk, rt5631->rx_rate,
					timesofbclk);
	if (coeff < 0) {
		dev_err(codec->dev, "Fail to get coeff\n");
		return -EINVAL;
	}

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		iface |= RT5631_SDP_I2S_DL_20;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iface |= RT5631_SDP_I2S_DL_24;
		break;
	case SNDRV_PCM_FORMAT_S8:
		iface |= RT5631_SDP_I2S_DL_8;
		break;
	default:
		return -EINVAL;
	}

	snd_soc_update_bits(codec, RT5631_SDP_CTRL,
		RT5631_SDP_I2S_DL_MASK, iface);
	snd_soc_write(codec, RT5631_STEREO_AD_DA_CLK_CTRL,
					coeff_div[coeff].reg_val);

	return 0;
}

static int rt5631_hifi_codec_set_dai_fmt(struct snd_soc_dai *codec_dai,
						unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	unsigned int iface = 0;

	dev_dbg(codec->dev, "enter %s\n", __func__);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		rt5631->master = 1;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		iface |= RT5631_SDP_MODE_SEL_SLAVE;
		rt5631->master = 0;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= RT5631_SDP_I2S_DF_LEFT;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= RT5631_SDP_I2S_DF_PCM_A;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface  |= RT5631_SDP_I2S_DF_PCM_B;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= RT5631_SDP_I2S_BCLK_POL_CTRL;
		break;
	default:
		return -EINVAL;
	}

	snd_soc_write(codec, RT5631_SDP_CTRL, iface);

	return 0;
}

static int rt5631_hifi_codec_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	dev_dbg(codec->dev, "enter %s, syclk=%d\n", __func__, freq);

	if ((freq >= (256 * 8000)) && (freq <= (512 * 96000))) {
		rt5631->sysclk = freq;
		return 0;
	}

	return -EINVAL;
}

static int rt5631_codec_set_dai_pll(struct snd_soc_dai *codec_dai, int pll_id,
		int source, unsigned int freq_in, unsigned int freq_out)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	int i, ret = -EINVAL;

	dev_dbg(codec->dev, "enter %s\n", __func__);

	if (!freq_in || !freq_out) {
		dev_dbg(codec->dev, "PLL disabled\n");

		snd_soc_update_bits(codec, RT5631_GLOBAL_CLK_CTRL,
			RT5631_SYSCLK_SOUR_SEL_MASK,
			RT5631_SYSCLK_SOUR_SEL_MCLK);

		return 0;
	}

	if (rt5631->master) {
		for (i = 0; i < ARRAY_SIZE(codec_master_pll_div); i++)
			if (freq_in == codec_master_pll_div[i].pll_in &&
			freq_out == codec_master_pll_div[i].pll_out) {
				dev_info(codec->dev,
					"change PLL in master mode\n");
				snd_soc_write(codec, RT5631_PLL_CTRL,
					codec_master_pll_div[i].reg_val);
				schedule_timeout_uninterruptible(
					msecs_to_jiffies(20));
				snd_soc_update_bits(codec,
					RT5631_GLOBAL_CLK_CTRL,
					RT5631_SYSCLK_SOUR_SEL_MASK |
					RT5631_PLLCLK_SOUR_SEL_MASK,
					RT5631_SYSCLK_SOUR_SEL_PLL |
					RT5631_PLLCLK_SOUR_SEL_MCLK);
				ret = 0;
				break;
			}
	} else {
		for (i = 0; i < ARRAY_SIZE(codec_slave_pll_div); i++)
			if (freq_in == codec_slave_pll_div[i].pll_in &&
			freq_out == codec_slave_pll_div[i].pll_out) {
				dev_info(codec->dev,
					"change PLL in slave mode\n");
				snd_soc_write(codec, RT5631_PLL_CTRL,
					codec_slave_pll_div[i].reg_val);
				schedule_timeout_uninterruptible(
					msecs_to_jiffies(20));
				snd_soc_update_bits(codec,
					RT5631_GLOBAL_CLK_CTRL,
					RT5631_SYSCLK_SOUR_SEL_MASK |
					RT5631_PLLCLK_SOUR_SEL_MASK,
					RT5631_SYSCLK_SOUR_SEL_PLL |
					RT5631_PLLCLK_SOUR_SEL_BCLK);
				ret = 0;
				break;
			}
	}

	return ret;
}

static int rt5631_set_bias_level(struct snd_soc_codec *codec,
			enum snd_soc_bias_level level)
{
	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MICBIAS1_VOL | RT5631_PWR_MICBIAS2_VOL,
			RT5631_PWR_MICBIAS1_VOL | RT5631_PWR_MICBIAS2_VOL);
		break;

	case SND_SOC_BIAS_STANDBY:
		if (codec->dapm.bias_level == SND_SOC_BIAS_OFF) {
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
				RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS,
				RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
			msleep(80);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
				RT5631_PWR_FAST_VREF_CTRL,
				RT5631_PWR_FAST_VREF_CTRL);
			codec->cache_only = false;
			snd_soc_cache_sync(codec);
		}
		break;

	case SND_SOC_BIAS_OFF:
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD1, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD2, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD3, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD4, 0x0000);
		break;

	default:
		break;
	}
	codec->dapm.bias_level = level;

	return 0;
}

static int rt5631_probe(struct snd_soc_codec *codec)
{
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;
	int ret;

	ret = snd_soc_codec_set_cache_io(codec, 8, 16, SND_SOC_I2C);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);
		return ret;
	}

	val = rt5631_read_index(codec, RT5631_ADDA_MIXER_INTL_REG3);
	if (val & 0x0002)
		rt5631->codec_version = 1;
	else
		rt5631->codec_version = 0;

	rt5631_reset(codec);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
		RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS,
		RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
	msleep(80);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
		RT5631_PWR_FAST_VREF_CTRL, RT5631_PWR_FAST_VREF_CTRL);
	/* enable HP zero cross */
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, 0x0f18);

	rt5631_reg_init(codec);

	/* power off ClassD auto Recovery */
	if (rt5631->codec_version)
		snd_soc_update_bits(codec, RT5631_INT_ST_IRQ_CTRL_2,
					0x2000, 0x2000);
	else
		snd_soc_update_bits(codec, RT5631_INT_ST_IRQ_CTRL_2,
					0x2000, 0);
	/* DMIC */
	if (rt5631->dmic_used_flag) {
		snd_soc_update_bits(codec, RT5631_GPIO_CTRL,
			RT5631_GPIO_PIN_FUN_SEL_MASK |
			RT5631_GPIO_DMIC_FUN_SEL_MASK,
			RT5631_GPIO_PIN_FUN_SEL_GPIO_DIMC |
			RT5631_GPIO_DMIC_FUN_SEL_DIMC);
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,
			RT5631_DMIC_L_CH_LATCH_MASK |
			RT5631_DMIC_R_CH_LATCH_MASK,
			RT5631_DMIC_L_CH_LATCH_FALLING |
			RT5631_DMIC_R_CH_LATCH_RISING);
	}

	codec->dapm.bias_level = SND_SOC_BIAS_STANDBY;
	rt5631_codec = codec;
	rt5631_audio_codec = codec;

#if defined(CONFIG_SND_HWDEP) || defined(CONFIG_SND_HWDEP_MODULE)
    realtek_ce_init_hwdep(rt5631_codec);
#endif

	return 0;
}

static int rt5631_remove(struct snd_soc_codec *codec)
{
	rt5631_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

#ifdef CONFIG_PM
static int rt5631_suspend(struct snd_soc_codec *codec)
{
	rt5631_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int rt5631_resume(struct snd_soc_codec *codec)
{
	rt5631_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	return 0;
}
#else
#define rt5631_suspend NULL
#define rt5631_resume NULL
#endif

#define RT5631_STEREO_RATES SNDRV_PCM_RATE_8000_96000
#define RT5631_FORMAT	(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S8)

static const struct snd_soc_dai_ops rt5631_ops = {
	.hw_params = rt5631_hifi_pcm_params,
	.set_fmt = rt5631_hifi_codec_set_dai_fmt,
	.set_sysclk = rt5631_hifi_codec_set_dai_sysclk,
	.set_pll = rt5631_codec_set_dai_pll,
};

static struct snd_soc_dai_driver rt5631_dai[] = {
	{
		.name = "rt5631-hifi",
		.id = 1,
		.playback = {
			.stream_name = "HIFI Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5631_STEREO_RATES,
			.formats = RT5631_FORMAT,
		},
		.capture = {
			.stream_name = "HIFI Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5631_STEREO_RATES,
			.formats = RT5631_FORMAT,
		},
		.ops = &rt5631_ops,
	},
};

static struct snd_soc_codec_driver soc_codec_dev_rt5631 = {
	.probe = rt5631_probe,
	.remove = rt5631_remove,
	.suspend = rt5631_suspend,
	.resume = rt5631_resume,
	.set_bias_level = rt5631_set_bias_level,
	.reg_cache_size = RT5631_VENDOR_ID2 + 1,
	.reg_word_size = sizeof(u16),
	.reg_cache_default = rt5631_reg,
	.volatile_register = rt5631_volatile_register,
	.readable_register = rt5631_readable_register,
	.reg_cache_step = 1,
	.controls = rt5631_snd_controls,
	.num_controls = ARRAY_SIZE(rt5631_snd_controls),
	.dapm_widgets = rt5631_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(rt5631_dapm_widgets),
	.dapm_routes = rt5631_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(rt5631_dapm_routes),
};

static const struct i2c_device_id rt5631_i2c_id[] = {
	{ "rt5631", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rt5631_i2c_id);

static int rt5631_i2c_probe(struct i2c_client *i2c,
		    const struct i2c_device_id *id)
{
	struct rt5631_priv *rt5631;
	int ret;

	rt5631 = devm_kzalloc(&i2c->dev, sizeof(struct rt5631_priv),
			      GFP_KERNEL);
	if (NULL == rt5631)
		return -ENOMEM;

	i2c_set_clientdata(i2c, rt5631);

	ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_rt5631,
			rt5631_dai, ARRAY_SIZE(rt5631_dai));
	return ret;
}

static __devexit int rt5631_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	return 0;
}

static struct i2c_driver rt5631_i2c_driver = {
	.driver = {
		.name = "rt5631",
		.owner = THIS_MODULE,
	},
	.probe = rt5631_i2c_probe,
	.remove   = __devexit_p(rt5631_i2c_remove),
	.id_table = rt5631_i2c_id,
};

static int codec_3v3_power_switch_init(void)
{
	int ret = 0;

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T) {
		ret = gpio_request(TEGRA_GPIO_PP0, "rt5631_3v3_power_control");
		if(!ret) {
			ret = gpio_direction_output(TEGRA_GPIO_PP0, 1);
			if(ret < 0) {
				gpio_free(TEGRA_GPIO_PP0);
				pr_err("%s: error in setting rt5631_3v3_power_control as output\n", __func__);
			}
		} else {
			pr_err("%s: error in gpio request for rt5631_3v3_power_control\n", __func__);
			return ret;
		}
	}
	return ret;
}

static int __init rt5631_modinit(void)
{
	codec_3v3_power_switch_init();
	return i2c_add_driver(&rt5631_i2c_driver);
}
module_init(rt5631_modinit);

static void __exit rt5631_modexit(void)
{
	i2c_del_driver(&rt5631_i2c_driver);
}
module_exit(rt5631_modexit);

MODULE_DESCRIPTION("ASoC RT5631 driver");
MODULE_AUTHOR("flove <flove@realtek.com>");
MODULE_LICENSE("GPL");
