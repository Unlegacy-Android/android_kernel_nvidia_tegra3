/*
 * arch/arm/mach-tegra/board-asus-t30.c
 *
 * Copyright (c) 2011-2012, NVIDIA Corporation.  All rights reserved.
 * Copyright (c) 2011-2012, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c/panjit_ts.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/i2c-tegra.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/spi/spi.h>
#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT)
#include <linux/i2c/atmel_mxt_ts.h>
#endif
#if defined (CONFIG_TOUCHSCREEN_ATMEL_MT_T9)
#include <linux/i2c/atmel_maxtouch.h>
#endif
#include <linux/tegra_uart.h>
#include <linux/memblock.h>
#include <linux/spi-tegra.h>
#include <linux/rfkill-gpio.h>

#include <sound/wm8903.h>
#include <sound/max98095.h>
#include <media/tegra_dtv.h>
#include <media/tegra_camera.h>

#include <mach/clk.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/iomap.h>
#include <mach/io_dpd.h>
#include <mach/io.h>
#include <mach/i2s.h>
#include <mach/tegra_asoc_pdata.h>
#include <mach/tegra_wm8903_pdata.h>
#include <mach/usb_phy.h>
#include <mach/board-asus-t30-misc.h>
#include <mach/thermal.h>
#include <mach/pci.h>
#include <mach/tegra_fiq_debugger.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include "board.h"
#include "clock.h"
#include "board-asus-t30.h"
#include "board-touch.h"
#include "devices.h"
#include "gpio-names.h"
#include "fuse.h"
#include "pm.h"
#include "baseband-xmm-power.h"
#include "wdt-recovery.h"

#define CODEC_RT5642_RESET	TEGRA_GPIO_PP2

#ifdef CONFIG_TEGRA_THERMAL_THROTTLE
static struct throttle_table throttle_freqs_tj[] = {
	      /*    CPU,    CBUS,    SCLK,     EMC */
	      { 1000000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  760000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  760000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  620000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  620000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  620000,  437000,  NO_CAP,  NO_CAP },
	      {  620000,  352000,  NO_CAP,  NO_CAP },
	      {  475000,  352000,  NO_CAP,  NO_CAP },
	      {  475000,  352000,  NO_CAP,  NO_CAP },
	      {  475000,  352000,  250000,  375000 },
	      {  475000,  352000,  250000,  375000 },
	      {  475000,  247000,  204000,  375000 },
	      {  475000,  247000,  204000,  204000 },
	      {  475000,  247000,  204000,  204000 },
	{ CPU_THROT_LOW,  247000,  204000,  102000 },
};
#endif

#ifdef CONFIG_TEGRA_SKIN_THROTTLE
static struct throttle_table throttle_freqs_tskin[] = {
	      /*    CPU,    CBUS,    SCLK,     EMC */
	      { 1000000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  760000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  760000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  620000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  620000,  NO_CAP,  NO_CAP,  NO_CAP },
	      {  620000,  437000,  NO_CAP,  NO_CAP },
	      {  620000,  352000,  NO_CAP,  NO_CAP },
	      {  475000,  352000,  NO_CAP,  NO_CAP },
	      {  475000,  352000,  NO_CAP,  NO_CAP },
	      {  475000,  352000,  250000,  375000 },
	      {  475000,  352000,  250000,  375000 },
	      {  475000,  247000,  204000,  375000 },
	      {  475000,  247000,  204000,  204000 },
	      {  475000,  247000,  204000,  204000 },
	{ CPU_THROT_LOW,  247000,  204000,  102000 },
};
#endif

static struct balanced_throttle throttle_list[] = {
#ifdef CONFIG_TEGRA_THERMAL_THROTTLE
	{
		.id = BALANCED_THROTTLE_ID_TJ,
		.throt_tab_size = ARRAY_SIZE(throttle_freqs_tj),
		.throt_tab = throttle_freqs_tj,
	},
#endif
#ifdef CONFIG_TEGRA_SKIN_THROTTLE
	{
		.id = BALANCED_THROTTLE_ID_SKIN,
		.throt_tab_size = ARRAY_SIZE(throttle_freqs_tskin),
		.throt_tab = throttle_freqs_tskin,
	},
#endif
};

/* All units are in millicelsius */
static struct tegra_thermal_data thermal_data = {
	.shutdown_device_id = THERMAL_DEVICE_ID_NCT_EXT,
	.temp_shutdown = 90000,

#if defined(CONFIG_TEGRA_EDP_LIMITS) || defined(CONFIG_TEGRA_THERMAL_THROTTLE)
	.throttle_edp_device_id = THERMAL_DEVICE_ID_NCT_EXT,
#endif
#ifdef CONFIG_TEGRA_EDP_LIMITS
	.edp_offset = TDIODE_OFFSET,  /* edp based on tdiode */
	.hysteresis_edp = 3000,
#endif
#ifdef CONFIG_TEGRA_THERMAL_THROTTLE
	.temp_throttle = 85000,
	.tc1 = 0,
	.tc2 = 1,
	.passive_delay = 2000,
#endif
#ifdef CONFIG_TEGRA_SKIN_THROTTLE
	.skin_device_id = THERMAL_DEVICE_ID_SKIN,
	.temp_throttle_skin = 43000,
	.tc1_skin = 5,
	.tc2_skin = 1,
	.passive_delay_skin = 5000,

	.skin_temp_offset = 9793,
	.skin_period = 1100,
	.skin_devs_size = 2,
	.skin_devs = {
		{
			"nct_ext_coeff",
			THERMAL_DEVICE_ID_NCT_EXT,
			{
				2, 1, 1, 1,
				1, 1, 1, 1,
				1, 1, 1, 0,
				1, 1, 0, 0,
				0, 0, -1, -7
			}
		},
		{
			"nct_int_coeff",
			THERMAL_DEVICE_ID_NCT_INT,
			{
				-11, -7, -5, -3,
				-3, -2, -1, 0,
				0, 0, 1, 1,
				1, 2, 2, 3,
				4, 6, 11, 18
			}
		},
	},
#endif
};

#ifdef CONFIG_BT_BLUESLEEP
static struct resource cardhu_bcm4330_rfkill_resources[] = {
	{
		.name   = "bcm4330_nshutdown_gpio",
		.start  = TEGRA_GPIO_PU0,
		.end    = TEGRA_GPIO_PU0,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device cardhu_bcm4330_rfkill_device = {
	.name           = "bcm4330_rfkill",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(cardhu_bcm4330_rfkill_resources),
	.resource       = cardhu_bcm4330_rfkill_resources,
};

static struct resource cardhu_bluesleep_resources[] = {
	[0] = {
		.name = "gpio_host_wake",
			.start  = TEGRA_GPIO_PU6,
			.end    = TEGRA_GPIO_PU6,
			.flags  = IORESOURCE_IO,
	},
	[1] = {
		.name = "gpio_ext_wake",
			.start  = TEGRA_GPIO_PU1,
			.end    = TEGRA_GPIO_PU1,
			.flags  = IORESOURCE_IO,
	},
	[2] = {
		.name = "host_wake",
			.start  = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PU6),
			.end    = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PU6),
			.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static struct platform_device cardhu_bluesleep_device = {
	.name           = "bluesleep",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(cardhu_bluesleep_resources),
	.resource       = cardhu_bluesleep_resources,
};

extern void bluesleep_setup_uart_port(struct platform_device *uart_dev);
static noinline void __init cardhu_setup_bluesleep(void)
{
        platform_device_register(&cardhu_bluesleep_device);
        bluesleep_setup_uart_port(&tegra_uartc_device);
        tegra_gpio_enable(TEGRA_GPIO_PU6);
        tegra_gpio_enable(TEGRA_GPIO_PU1);
        return;
}
#elif defined CONFIG_BLUEDROID_PM
static struct resource cardhu_bluedroid_pm_resources[] = {
	[0] = {
		.name   = "shutdown_gpio",
		.start  = TEGRA_GPIO_PU0,
		.end    = TEGRA_GPIO_PU0,
		.flags  = IORESOURCE_IO,
	},
	[1] = {
		.name = "host_wake",
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
	[2] = {
		.name = "gpio_ext_wake",
		.start  = TEGRA_GPIO_PU1,
		.end    = TEGRA_GPIO_PU1,
		.flags  = IORESOURCE_IO,
	},
	[3] = {
		.name = "gpio_host_wake",
		.start  = TEGRA_GPIO_PU6,
		.end    = TEGRA_GPIO_PU6,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device cardhu_bluedroid_pm_device = {
	.name = "bluedroid_pm",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(cardhu_bluedroid_pm_resources),
	.resource       = cardhu_bluedroid_pm_resources,
};

static noinline void __init cardhu_setup_bluedroid_pm(void)
{
	cardhu_bluedroid_pm_resources[1].start =
		cardhu_bluedroid_pm_resources[1].end =
				gpio_to_irq(TEGRA_GPIO_PU6);
	platform_device_register(&cardhu_bluedroid_pm_device);
	return;
}
#endif

static __initdata struct tegra_clk_init_table cardhu_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
	{ "hda",	"pll_p",	108000000,	false},
	{ "hda2codec_2x","pll_p",	48000000,	false},
	{ "pwm",	"pll_p",	5100000,	false},
	{ "blink",	"clk_32k",	32768,		true},
	{ "i2s0",	"pll_a_out0",	0,		false},
	{ "i2s1",	"pll_a_out0",	0,		false},
	{ "i2s3",	"pll_a_out0",	0,		false},
	{ "spdif_out",	"pll_a_out0",	0,		false},
	{ "d_audio",	"clk_m",	12000000,	false},
	{ "dam0",	"clk_m",	12000000,	false},
	{ "dam1",	"clk_m",	12000000,	false},
	{ "dam2",	"clk_m",	12000000,	false},
	{ "audio1",	"i2s1_sync",	0,		false},
	{ "audio3",	"i2s3_sync",	0,		false},
	{ "vi_sensor",	"pll_m",	150000000,	false},
	{ "i2c1",	"pll_p",	3200000,	false},
	{ "i2c2",	"pll_p",	3200000,	false},
	{ "i2c3",	"pll_p",	3200000,	false},
	{ "i2c4",	"pll_p",	3200000,	false},
	{ "i2c5",	"pll_p",	3200000,	false},
	{ "vi",		"pll_p",	0,		false},
	{ NULL,		NULL,		0,		0},
};

static struct tegra_i2c_platform_data cardhu_i2c1_platform_data = {
	.adapter_nr	= 0,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PC4, 0},
	.sda_gpio		= {TEGRA_GPIO_PC5, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c2_platform_data = {
	.adapter_nr	= 1,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.is_clkon_always = true,
	.scl_gpio		= {TEGRA_GPIO_PT5, 0},
	.sda_gpio		= {TEGRA_GPIO_PT6, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c3_platform_data = {
	.adapter_nr	= 2,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB1, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB2, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c3_platform_data_for_TF300 = {
	.adapter_nr	= 2,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB1, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB2, 0},
	.arb_recovery = arb_lost_recovery,
};

/* Higher freq could be applied for TF300T/TF300TG/TF300TL Camera FW update*/

static struct tegra_i2c_platform_data cardhu_i2c4_platform_data = {
	.adapter_nr	= 3,
	.bus_count	= 1,
	.bus_clk_rate	= { 93750, 0 },
	.scl_gpio		= {TEGRA_GPIO_PV4, 0},
	.sda_gpio		= {TEGRA_GPIO_PV5, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c5_platform_data = {
	.adapter_nr	= 4,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PZ6, 0},
	.sda_gpio		= {TEGRA_GPIO_PZ7, 0},
	.arb_recovery = arb_lost_recovery,
};


#if 0
struct tegra_wired_jack_conf audio_wr_jack_conf = {
	.hp_det_n = TEGRA_GPIO_PW2,
	.en_mic_ext = TEGRA_GPIO_PX1,
	.en_mic_int = TEGRA_GPIO_PX0,
};
#endif

static struct wm8903_platform_data cardhu_wm8903_pdata = {
	.irq_active_low = 0,
	.micdet_cfg = 0,
	.micdet_delay = 100,
	.gpio_base = CARDHU_GPIO_WM8903(0),
	.gpio_cfg = {
		(WM8903_GPn_FN_DMIC_LR_CLK_OUTPUT << WM8903_GP1_FN_SHIFT),
		(WM8903_GPn_FN_DMIC_LR_CLK_OUTPUT << WM8903_GP2_FN_SHIFT) |
			WM8903_GP2_DIR,
		0,
		WM8903_GPIO_NO_CONFIG,
		WM8903_GPIO_NO_CONFIG,
	},
};
#ifdef CONFIG_DSP_FM34
static const struct i2c_board_info cardhu_dsp_board_info[] = {
	{
		I2C_BOARD_INFO("dsp_fm34", 0x60),
	},
};
#endif
static struct i2c_board_info __initdata rt5631_board_info = {
	I2C_BOARD_INFO("rt5631", 0x1a),
};
static struct i2c_board_info __initdata wm8903_board_info = {
	I2C_BOARD_INFO("wm8903", 0x1a),
	.platform_data = &cardhu_wm8903_pdata,
};
static struct i2c_board_info __initdata rt5642_board_info = {
	I2C_BOARD_INFO("rt5640", 0x1c),
};

/* Equalizer filter coefs generated from the MAXIM MAX98095
 * evkit software tool */
static struct max98095_eq_cfg max98095_eq_cfg[] = {
	{
		.name = "FLAT",
		.rate = 44100,
		.band1 = {0x2000, 0xC002, 0x4000, 0x00E9, 0x0000},
		.band2 = {0x2000, 0xC00F, 0x4000, 0x02BC, 0x0000},
		.band3 = {0x2000, 0xC0A7, 0x4000, 0x0916, 0x0000},
		.band4 = {0x2000, 0xC5C2, 0x4000, 0x1A87, 0x0000},
		.band5 = {0x2000, 0xF6B0, 0x4000, 0x3F51, 0x0000},
	},
	{
		.name = "LOWPASS1K",
		.rate = 44100,
		.band1 = {0x205D, 0xC001, 0x3FEF, 0x002E, 0x02E0},
		.band2 = {0x5B9A, 0xC093, 0x3AB2, 0x088B, 0x1981},
		.band3 = {0x0D22, 0xC170, 0x26EA, 0x0D79, 0x32CF},
		.band4 = {0x0894, 0xC612, 0x01B3, 0x1B34, 0x3FFA},
		.band5 = {0x0815, 0x3FFF, 0xCF78, 0x0000, 0x29B7},
	},
	{ /* BASS=-12dB, TREBLE=+9dB, Fc=5KHz */
		.name = "HIBOOST",
		.rate = 44100,
		.band1 = {0x0815, 0xC001, 0x3AA4, 0x0003, 0x19A2},
		.band2 = {0x0815, 0xC103, 0x092F, 0x0B55, 0x3F56},
		.band3 = {0x0E0A, 0xC306, 0x1E5C, 0x136E, 0x3856},
		.band4 = {0x2459, 0xF665, 0x0CAA, 0x3F46, 0x3EBB},
		.band5 = {0x5BBB, 0x3FFF, 0xCEB0, 0x0000, 0x28CA},
	},
	{ /* BASS=12dB, TREBLE=+12dB */
		.name = "LOUD12DB",
		.rate = 44100,
		.band1 = {0x7FC1, 0xC001, 0x3EE8, 0x0020, 0x0BC7},
		.band2 = {0x51E9, 0xC016, 0x3C7C, 0x033F, 0x14E9},
		.band3 = {0x1745, 0xC12C, 0x1680, 0x0C2F, 0x3BE9},
		.band4 = {0x4536, 0xD7E2, 0x0ED4, 0x31DD, 0x3E42},
		.band5 = {0x7FEF, 0x3FFF, 0x0BAB, 0x0000, 0x3EED},
	},
	{
		.name = "FLAT",
		.rate = 16000,
		.band1 = {0x2000, 0xC004, 0x4000, 0x0141, 0x0000},
		.band2 = {0x2000, 0xC033, 0x4000, 0x0505, 0x0000},
		.band3 = {0x2000, 0xC268, 0x4000, 0x115F, 0x0000},
		.band4 = {0x2000, 0xDA62, 0x4000, 0x33C6, 0x0000},
		.band5 = {0x2000, 0x4000, 0x4000, 0x0000, 0x0000},
	},
	{
		.name = "LOWPASS1K",
		.rate = 16000,
		.band1 = {0x2000, 0xC004, 0x4000, 0x0141, 0x0000},
		.band2 = {0x5BE8, 0xC3E0, 0x3307, 0x15ED, 0x26A0},
		.band3 = {0x0F71, 0xD15A, 0x08B3, 0x2BD0, 0x3F67},
		.band4 = {0x0815, 0x3FFF, 0xCF78, 0x0000, 0x29B7},
		.band5 = {0x0815, 0x3FFF, 0xCF78, 0x0000, 0x29B7},
	},
	{ /* BASS=-12dB, TREBLE=+9dB, Fc=2KHz */
		.name = "HIBOOST",
		.rate = 16000,
		.band1 = {0x0815, 0xC001, 0x3BD2, 0x0009, 0x16BF},
		.band2 = {0x080E, 0xC17E, 0xF653, 0x0DBD, 0x3F43},
		.band3 = {0x0F80, 0xDF45, 0xEE33, 0x36FE, 0x3D79},
		.band4 = {0x590B, 0x3FF0, 0xE882, 0x02BD, 0x3B87},
		.band5 = {0x4C87, 0xF3D0, 0x063F, 0x3ED4, 0x3FB1},
	},
	{ /* BASS=12dB, TREBLE=+12dB */
		.name = "LOUD12DB",
		.rate = 16000,
		.band1 = {0x7FC1, 0xC001, 0x3D07, 0x0058, 0x1344},
		.band2 = {0x2DA6, 0xC013, 0x3CF1, 0x02FF, 0x138B},
		.band3 = {0x18F1, 0xC08E, 0x244D, 0x0863, 0x34B5},
		.band4 = {0x2BE0, 0xF385, 0x04FD, 0x3EC5, 0x3FCE},
		.band5 = {0x7FEF, 0x4000, 0x0BAB, 0x0000, 0x3EED},
	},
};

static struct max98095_pdata cardhu_max98095_pdata = {
	/* equalizer configuration */
	.eq_cfg = max98095_eq_cfg,
	.eq_cfgcnt = ARRAY_SIZE(max98095_eq_cfg),

	/* Biquad filter response configuration */
	.bq_cfg = NULL,
	.bq_cfgcnt = 0,

	/* microphone configuration */
	.digmic_left_mode = 1,
	.digmic_right_mode = 1,
};

static struct i2c_board_info __initdata cardhu_codec_wm8903_info = {
	I2C_BOARD_INFO("wm8903", 0x1a),
	.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_CDC_IRQ),
	.platform_data = &cardhu_wm8903_pdata,
};

static struct i2c_board_info __initdata cardhu_codec_aic326x_info = {
	I2C_BOARD_INFO("aic3262-codec", 0x18),
	.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_CDC_IRQ),
};

static struct i2c_board_info __initdata cardhu_codec_max98095_info = {
	I2C_BOARD_INFO("max98095", 0x10),
	.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_CDC_IRQ),
	.platform_data = &cardhu_max98095_pdata,
};

static struct i2c_board_info __initdata cardhu_i2c_asuspec_info[] = {
	{
		I2C_BOARD_INFO("asuspec", 0x15),
	},
	{
		I2C_BOARD_INFO("asusdec", 0x19),
	},
};

static struct i2c_board_info __initdata cardhu_i2c_aw8ec_info[] = {
        {
                I2C_BOARD_INFO("asuspec", 0x15),
        },
        {
                I2C_BOARD_INFO("aw8ec", 0x19),
        },
};


static void cardhu_i2c_init(void)
{
	u32 project_info = tegra3_get_project_id();

	if (project_info == TEGRA3_PROJECT_P1801) {
		cardhu_i2c1_platform_data.bus_clk_rate[0] = 280000;
		cardhu_i2c4_platform_data.bus_clk_rate[0] = 33000;
	}

	tegra_i2c_device1.dev.platform_data = &cardhu_i2c1_platform_data;
	tegra_i2c_device2.dev.platform_data = &cardhu_i2c2_platform_data;
	if ((project_info == TEGRA3_PROJECT_TF300T) ||
		(project_info == TEGRA3_PROJECT_TF300TG) ||
		(project_info == TEGRA3_PROJECT_TF300TL) ||
		(project_info == TEGRA3_PROJECT_ME301T)  ||
		(project_info == TEGRA3_PROJECT_ME301TL)) {
		tegra_i2c_device3.dev.platform_data = &cardhu_i2c3_platform_data_for_TF300;
	} else {
		tegra_i2c_device3.dev.platform_data = &cardhu_i2c3_platform_data;
	}

	tegra_i2c_device4.dev.platform_data = &cardhu_i2c4_platform_data;
	tegra_i2c_device5.dev.platform_data = &cardhu_i2c5_platform_data;

	platform_device_register(&tegra_i2c_device5);
	platform_device_register(&tegra_i2c_device4);
	platform_device_register(&tegra_i2c_device3);
	platform_device_register(&tegra_i2c_device2);
	platform_device_register(&tegra_i2c_device1);

	//i2c_register_board_info(4, &cardhu_codec_wm8903_info, 1);
	//i2c_register_board_info(4, &cardhu_codec_max98095_info, 1);
	//i2c_register_board_info(4, &cardhu_codec_aic326x_info, 1);

	switch(project_info){
	case TEGRA3_PROJECT_TF201:
	case TEGRA3_PROJECT_TF300TG:
	case TEGRA3_PROJECT_TF700T:
	case TEGRA3_PROJECT_TF300TL:
	case TEGRA3_PROJECT_TF300T:
	case TEGRA3_PROJECT_TF500T:
		i2c_register_board_info(1, cardhu_i2c_asuspec_info, ARRAY_SIZE(cardhu_i2c_asuspec_info));
	break;
	case TEGRA3_PROJECT_P1801:
		i2c_register_board_info(1, cardhu_i2c_aw8ec_info, ARRAY_SIZE(cardhu_i2c_aw8ec_info));
	default:;
	}

	switch (project_info) {
		case TEGRA3_PROJECT_TF201:
		case TEGRA3_PROJECT_TF300TG:
		case TEGRA3_PROJECT_TF700T:
		case TEGRA3_PROJECT_TF300TL:
			i2c_register_board_info(4, &rt5631_board_info, 1);
			break;
		case TEGRA3_PROJECT_TF300T:
			i2c_register_board_info(4, &wm8903_board_info, 1);
			break;
		case TEGRA3_PROJECT_ME301T:
		case TEGRA3_PROJECT_P1801:
			i2c_register_board_info(4, &rt5642_board_info, 1);
			break;
		default:
			break;
	}

#ifdef CONFIG_DSP_FM34
	i2c_register_board_info(0, cardhu_dsp_board_info, 1);
#endif
}

static struct platform_device *cardhu_uart_devices[] __initdata = {
	&tegra_uartb_device,
	&tegra_uartc_device,
	&tegra_uartd_device,
	&tegra_uarte_device,
};
static struct uart_clk_parent uart_parent_clk[] = {
	[0] = {.name = "clk_m"},
	[1] = {.name = "pll_p"},
#ifndef CONFIG_TEGRA_PLLM_RESTRICTED
	[2] = {.name = "pll_m"},
#endif
};

static struct tegra_uart_platform_data cardhu_uart_pdata;
static struct tegra_uart_platform_data cardhu_loopback_uart_pdata;

static unsigned int debug_uart_port_irq;

static char *uart_names[] = {
	"uarta",
	"uartb",
	"uartc",
	"uartd",
	"uarte",
};

static struct platform_device *debug_uarts[] = {
	&debug_uarta_device,
	&debug_uartb_device,
	&debug_uartc_device,
	&debug_uartd_device,
	&debug_uarte_device,
};

static void __init uart_debug_init(void)
{
	int debug_port_id;
	struct platform_device *debug_uart;

	debug_port_id = get_tegra_uart_debug_port_id();
	if (debug_port_id < 0)
		debug_port_id = 0;
	else if (debug_port_id >= ARRAY_SIZE(debug_uarts)) {
		pr_info("The debug console id %d is invalid, Assuming UARTA",
			debug_port_id);
		debug_port_id = 0;
	}

	pr_info("Selecting %s as the debug port\n", uart_names[debug_port_id]);

	debug_uart_clk = clk_get_sys("serial8250.0",
		uart_names[debug_port_id]);
	debug_uart = debug_uarts[debug_port_id];
	debug_uart_port_base = ((struct plat_serial8250_port *)(
		debug_uart->dev.platform_data))->mapbase;
	debug_uart_port_irq = ((struct plat_serial8250_port *)(
		debug_uart->dev.platform_data))->irq;

	return;
}

static void __init cardhu_uart_init(void)
{
	struct clk *c;
	int i;
	struct board_info board_info;

	tegra_get_board_info(&board_info);

	for (i = 0; i < ARRAY_SIZE(uart_parent_clk); ++i) {
		c = tegra_get_clock_by_name(uart_parent_clk[i].name);
		if (IS_ERR_OR_NULL(c)) {
			pr_err("Not able to get the clock for %s\n",
						uart_parent_clk[i].name);
			continue;
		}
		uart_parent_clk[i].parent_clk = c;
		uart_parent_clk[i].fixed_clk_rate = clk_get_rate(c);
	}
	cardhu_uart_pdata.parent_clk_list = uart_parent_clk;
	cardhu_uart_pdata.parent_clk_count = ARRAY_SIZE(uart_parent_clk);
	cardhu_loopback_uart_pdata.parent_clk_list = uart_parent_clk;
	cardhu_loopback_uart_pdata.parent_clk_count =
						ARRAY_SIZE(uart_parent_clk);
	cardhu_loopback_uart_pdata.is_loopback = true;
	tegra_uartb_device.dev.platform_data = &cardhu_uart_pdata;
	tegra_uartc_device.dev.platform_data = &cardhu_uart_pdata;
	tegra_uartd_device.dev.platform_data = &cardhu_uart_pdata;
	/* UARTE is used for loopback test purpose */
	tegra_uarte_device.dev.platform_data = &cardhu_loopback_uart_pdata;

	/* Register low speed only if it is selected */
	if (!is_tegra_debug_uartport_hs()) {
		uart_debug_init();
		/* Clock enable for the debug channel */
		if (!IS_ERR_OR_NULL(debug_uart_clk)) {
			pr_info("The debug console clock name is %s\n",
						debug_uart_clk->name);
			c = tegra_get_clock_by_name("pll_p");
			if (IS_ERR_OR_NULL(c))
				pr_err("Not getting the parent clock pll_p\n");
			else
				clk_set_parent(debug_uart_clk, c);

			clk_enable(debug_uart_clk);
			clk_set_rate(debug_uart_clk, clk_get_rate(c));
		} else {
			pr_err("Not getting the clock %s for debug console\n",
					debug_uart_clk->name);
		}
	}

#ifdef CONFIG_TEGRA_IRDA
	if (((board_info.board_id == BOARD_E1186) ||
		(board_info.board_id == BOARD_E1198)) &&
			cardhu_irda_pdata.is_irda) {
		cardhu_irda_pdata.parent_clk_list = uart_parent_clk;
		cardhu_irda_pdata.parent_clk_count =
					ARRAY_SIZE(uart_parent_clk);

		tegra_uartb_device.dev.platform_data = &cardhu_irda_pdata;
	}
#endif

	tegra_serial_debug_init(debug_uart_port_base, debug_uart_port_irq,
				debug_uart_clk, -1, -1, false);

	platform_add_devices(cardhu_uart_devices,
				ARRAY_SIZE(cardhu_uart_devices));
}

static struct tegra_camera_platform_data tegra_camera_pdata = {
	.limit_3d_emc_clk = false,
};

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.dev = {
		.platform_data = &tegra_camera_pdata,
	},
	.id = -1,
};

static void tegra_camera_init(void)
{
	/* For AP37 platform, limit 3d and emc freq when camera is ON */
	if (TEGRA_REVISION_A03 == tegra_get_revision() &&
		0xA0 == tegra_sku_id())
		tegra_camera_pdata.limit_3d_emc_clk = true;
	else
		tegra_camera_pdata.limit_3d_emc_clk = false;
}

static struct platform_device *cardhu_spi_devices[] __initdata = {
	&tegra_spi_device4,
};

static struct platform_device *touch_spi_device[] __initdata = {
	&tegra_spi_device1,
};

struct spi_clk_parent spi_parent_clk[] = {
	[0] = {.name = "pll_p"},
#ifndef CONFIG_TEGRA_PLLM_RESTRICTED
	[1] = {.name = "pll_m"},
	[2] = {.name = "clk_m"},
#else
	[1] = {.name = "clk_m"},
#endif
};

static struct tegra_spi_platform_data cardhu_spi_pdata = {
	.is_dma_based		= true,
	.max_dma_buffer		= (16 * 1024),
	.is_clkon_always	= false,
	.max_rate		= 100000000,
};

static void __init cardhu_spi_init(void)
{
	int i;
	struct clk *c;
	struct board_info board_info, display_board_info;

	tegra_get_board_info(&board_info);
	tegra_get_display_board_info(&display_board_info);

	for (i = 0; i < ARRAY_SIZE(spi_parent_clk); ++i) {
		c = tegra_get_clock_by_name(spi_parent_clk[i].name);
		if (IS_ERR_OR_NULL(c)) {
			pr_err("Not able to get the clock for %s\n",
						spi_parent_clk[i].name);
			continue;
		}
		spi_parent_clk[i].parent_clk = c;
		spi_parent_clk[i].fixed_clk_rate = clk_get_rate(c);
	}
	cardhu_spi_pdata.parent_clk_list = spi_parent_clk;
	cardhu_spi_pdata.parent_clk_count = ARRAY_SIZE(spi_parent_clk);
	tegra_spi_device4.dev.platform_data = &cardhu_spi_pdata;
	platform_add_devices(cardhu_spi_devices,
				ARRAY_SIZE(cardhu_spi_devices));

	if (display_board_info.board_id == BOARD_DISPLAY_PM313) {
		platform_add_devices(touch_spi_device,
				ARRAY_SIZE(touch_spi_device));
	}
	if (board_info.board_id == BOARD_E1198) {
		tegra_spi_device2.dev.platform_data = &cardhu_spi_pdata;
		platform_device_register(&tegra_spi_device2);
		tegra_spi_slave_device1.dev.platform_data = &cardhu_spi_pdata;
		platform_device_register(&tegra_spi_slave_device1);
	}
}

static void __init cardhu_dtv_init(void)
{
	struct board_info board_info;

	tegra_get_board_info(&board_info);

	if (board_info.board_id == BOARD_E1186)
		platform_device_register(&tegra_dtv_device);
}

static struct resource tegra_rtc_resources[] = {
	[0] = {
		.start = TEGRA_RTC_BASE,
		.end = TEGRA_RTC_BASE + TEGRA_RTC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_RTC,
		.end = INT_RTC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tegra_rtc_device = {
	.name = "tegra_rtc",
	.id   = -1,
	.resource = tegra_rtc_resources,
	.num_resources = ARRAY_SIZE(tegra_rtc_resources),
};

static struct tegra_asoc_platform_data cardhu_audio_wm8903_pdata = {
	.gpio_spkr_en		= TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det		= TEGRA_GPIO_HP_DET,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= -1,
	.gpio_ext_mic_en	= -1,
	.i2s_param[HIFI_CODEC]	= {
		.audio_port_id	= 0,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
	},
	.i2s_param[BASEBAND]	= {
		.audio_port_id	= -1,
	},
	.i2s_param[BT_SCO]	= {
		.audio_port_id	= 3,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_DSP_A,
	},
};

static struct platform_device cardhu_audio_device = {
	.name	= "tegra-snd-codec",
        .id     = 0,
        .dev    = {
                .platform_data = &cardhu_audio_wm8903_pdata,
        },
};
static struct tegra_asoc_platform_data cardhu_audio_max98095_pdata = {
	.gpio_spkr_en		= -1,
	.gpio_hp_det		= TEGRA_GPIO_HP_DET,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= -1,
	.gpio_ext_mic_en	= -1,
	.i2s_param[HIFI_CODEC]	= {
		.audio_port_id	= 0,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
	},
	.i2s_param[BASEBAND]	= {
		.audio_port_id	= -1,
	},
	.i2s_param[BT_SCO]	= {
		.audio_port_id	= 3,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_DSP_A,
	},
};

static struct platform_device cardhu_audio_wm8903_device = {
	.name	= "tegra-snd-wm8903",
	.id	= 0,
	.dev	= {
		.platform_data = &cardhu_audio_wm8903_pdata,
	},
};

static struct platform_device cardhu_audio_max98095_device = {
	.name	= "tegra-snd-max98095",
	.id	= 0,
	.dev	= {
		.platform_data = &cardhu_audio_max98095_pdata,
	},
};

static struct tegra_asoc_platform_data cardhu_audio_aic326x_pdata = {
	.gpio_spkr_en		= -1,
	.gpio_hp_det		= TEGRA_GPIO_HP_DET,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= -1,
	.gpio_ext_mic_en	= -1,
	/*defaults for Verbier-Cardhu board with TI AIC326X codec*/
	.i2s_param[HIFI_CODEC]	= {
		.audio_port_id	= 0,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
		.sample_size	= 16,
	},
	.i2s_param[BT_SCO]	= {
		.sample_size	= 16,
		.audio_port_id	= 3,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_DSP_A,
	},
};

static struct platform_device cardhu_audio_aic326x_device = {
	.name	= "tegra-snd-aic326x",
	.id	= 0,
	.dev	= {
		.platform_data  = &cardhu_audio_aic326x_pdata,
	},
};

static struct platform_device *cardhu_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_rtc_device,
	&tegra_udc_device,
#if defined(CONFIG_TEGRA_IOVMM_SMMU) ||  defined(CONFIG_TEGRA_IOMMU_SMMU)
	&tegra_smmu_device,
#endif
	&tegra_wdt0_device,
	&tegra_wdt1_device,
	&tegra_wdt2_device,
#if defined(CONFIG_TEGRA_AVP)
	&tegra_avp_device,
#endif
	&tegra_camera,
#if defined(CONFIG_CRYPTO_DEV_TEGRA_SE)
	&tegra_se_device,
#endif
	&tegra_ahub_device,
	&tegra_dam_device0,
	&tegra_dam_device1,
	&tegra_dam_device2,
	&tegra_i2s_device0,
	&tegra_i2s_device1,
	&tegra_i2s_device3,
	&tegra_spdif_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&baseband_dit_device,
#ifdef CONFIG_BT_BLUESLEEP
	&cardhu_bcm4330_rfkill_device,
#endif
	&tegra_pcm_device,
	&cardhu_audio_device,
	//&cardhu_audio_max98095_device,
	//&cardhu_audio_aic326x_device,
	&tegra_hda_device,
	&tegra_cec_device,
#if defined(CONFIG_CRYPTO_DEV_TEGRA_AES)
	&tegra_aes_device,
#endif
};

#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT)
#define E1506_MXT_CONFIG_CRC 0x62F903
static const u8 e1506_config[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0x32, 0x0A, 0x00, 0x05, 0x01, 0x00,
	0x00, 0x1E, 0x0A, 0x8B, 0x00, 0x00, 0x13, 0x0B,
	0x00, 0x10, 0x32, 0x03, 0x03, 0x00, 0x03, 0x01,
	0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0xBF, 0x03, 0x1B,
	0x02, 0x00, 0x00, 0x37, 0x37, 0x00, 0x00, 0x00,
	0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xA9, 0x7F, 0x9A, 0x0E, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x03, 0x23, 0x00, 0x00, 0x00, 0x0A,
	0x0F, 0x14, 0x19, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x08, 0x10,
	0x00
};

#define MXT_CONFIG_CRC  0xD62DE8
static const u8 config[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0x32, 0x0A, 0x00, 0x14, 0x14, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x00, 0x00,
	0x1B, 0x2A, 0x00, 0x20, 0x3C, 0x04, 0x05, 0x00,
	0x02, 0x01, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0xFF,
	0x02, 0x55, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x64, 0x02, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23,
	0x00, 0x00, 0x00, 0x05, 0x0A, 0x15, 0x1E, 0x00,
	0x00, 0x04, 0xFF, 0x03, 0x3F, 0x64, 0x64, 0x01,
	0x0A, 0x14, 0x28, 0x4B, 0x00, 0x02, 0x00, 0x64,
	0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x10, 0x3C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define MXT_CONFIG_CRC_SKU2000  0xA24D9A
static const u8 config_sku2000[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0x32, 0x0A, 0x00, 0x14, 0x14, 0x19,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x00, 0x00,
	0x1B, 0x2A, 0x00, 0x20, 0x3A, 0x04, 0x05, 0x00,  //23=thr  2 di
	0x04, 0x04, 0x41, 0x0A, 0x0A, 0x0A, 0x0A, 0xFF,
	0x02, 0x55, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,  //0A=limit
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23,
	0x00, 0x00, 0x00, 0x05, 0x0A, 0x15, 0x1E, 0x00,
	0x00, 0x04, 0x00, 0x03, 0x3F, 0x64, 0x64, 0x01,
	0x0A, 0x14, 0x28, 0x4B, 0x00, 0x02, 0x00, 0x64,
	0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x10, 0x3C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static struct mxt_platform_data atmel_mxt_info = {
	.x_line         = 27,
	.y_line         = 42,
	.x_size         = 768,
	.y_size         = 1366,
	.blen           = 0x20,
	.threshold      = 0x3C,
	.voltage        = 3300000,              /* 3.3V */
	.orient         = 5,
	.config         = config,
	.config_length  = 157,
	.config_crc     = MXT_CONFIG_CRC,
	.irqflags       = IRQF_TRIGGER_FALLING,
/*	.read_chg       = &read_chg, */
	.read_chg       = NULL,
};

static struct mxt_platform_data e1506_atmel_mxt_info = {
	.x_line         = 19,
	.y_line         = 11,
	.x_size         = 960,
	.y_size         = 540,
	.blen           = 0x10,
	.threshold      = 0x32,
	.voltage        = 3300000,              /* 3.3V */
	.orient         = 3,
	.config         = e1506_config,
	.config_length  = 168,
	.config_crc     = E1506_MXT_CONFIG_CRC,
	.irqflags       = IRQF_TRIGGER_FALLING,
/*	.read_chg       = &read_chg, */
	.read_chg       = NULL,
};

static struct i2c_board_info __initdata atmel_i2c_info[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x5A),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PH4),
		.platform_data = &atmel_mxt_info,
	}
};

static struct i2c_board_info __initdata e1506_atmel_i2c_info[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4A),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PH4),
		.platform_data = &e1506_atmel_mxt_info,
	}
};
#endif
#if defined(CONFIG_TOUCHSCREEN_RM31080A)
static __initdata struct tegra_clk_init_table spi_clk_init_table[] = {
	/* name         parent          rate            enabled */
	{ "sbc1",       "pll_p",        52000000,       true},
	{ NULL,         NULL,           0,              0},
};
#endif
#define TOUCH_GPIO_IRQ_ATMEL_T9	TEGRA_GPIO_PH4
#define TOUCH_BUS_ATMEL_T9	1
#if defined (CONFIG_TOUCHSCREEN_ATMEL_MT_T9)
static u8 read_chg(void)
{
	return gpio_get_value(TOUCH_GPIO_IRQ_ATMEL_T9);
}

static u8 valid_interrupt(void)
{
	return !read_chg();
}

static struct mxt_platform_data atmel_mxt_info = {
	/* Maximum number of simultaneous touches to report. */
	.numtouch = 10,
	/* TODO: no need for any hw-specific things at init/exit? */
	.init_platform_hw = NULL,
	.exit_platform_hw = NULL,
	.max_x = 1600,
	.max_y = 1000,
	.valid_interrupt = &valid_interrupt,
	.read_chg = &read_chg,
};

static struct i2c_board_info __initdata atmel_i2c_info[] = {
	{
		I2C_BOARD_INFO("maXTouch", MXT_I2C_ADDRESS),
		.irq = TEGRA_GPIO_TO_IRQ(TOUCH_GPIO_IRQ_ATMEL_T9),
		.platform_data = &atmel_mxt_info,
	}
};
#endif
#if defined(CONFIG_TOUCHSCREEN_ELAN_TF_3K)
// Interrupt pin: TEGRA_GPIO_PH4
// Reset pin: TEGRA_GPIO_PH6
// Power pin:

#include <linux/i2c/ektf3k.h>

struct elan_ktf3k_i2c_platform_data ts_elan_ktf3k_data[] = {
        {
                .version = 0x0001,
		.abs_x_min = 0,
                .abs_x_max = ELAN_X_MAX,   //LG 9.7" Dpin 2368, Spin 2112
                .abs_y_min = 0,
                .abs_y_max = ELAN_Y_MAX,   //LG 9.7" Dpin 1728, Spin 1600
                .intr_gpio = TEGRA_GPIO_PH4,
                .rst_gpio = TEGRA_GPIO_PH6,
        },
};
static struct i2c_board_info elan_i2c_devices[] = {
        {
                I2C_BOARD_INFO(ELAN_KTF3K_NAME, 0x10),
                .platform_data = &ts_elan_ktf3k_data,
                .irq = (INT_GPIO_BASE + TEGRA_GPIO_PH4),
        },

};
#endif
#if defined(CONFIG_TOUCHSCREEN_SIS_I2C)
static struct i2c_board_info  sis9200_i2c2_boardinfo[] =
{
	{
		I2C_BOARD_INFO("sis_i2c_ts", 0x5C),
	},
};
#endif
static int __init cardhu_touch_init(void)
{
	struct board_info BoardInfo, DisplayBoardInfo;
    unsigned int project_id;
	struct elan_ktf3k_i2c_platform_data *platform;

	tegra_get_board_info(&BoardInfo);
	tegra_get_display_board_info(&DisplayBoardInfo);
	project_id = tegra3_get_project_id();
        /*
	if (DisplayBoardInfo.board_id == BOARD_DISPLAY_PM313) {
		tegra_clk_init_from_table(spi_clk_init_table);
		touch_init_raydium(TEGRA_GPIO_PH4, TEGRA_GPIO_PH6, 2);
	} else {
		gpio_request(TEGRA_GPIO_PH4, "atmel-irq");
		gpio_direction_input(TEGRA_GPIO_PH4);
		gpio_request(TEGRA_GPIO_PH6, "atmel-reset");
		gpio_direction_output(TEGRA_GPIO_PH6, 0);
		msleep(1);
		gpio_set_value(TEGRA_GPIO_PH6, 1);
		msleep(100);
		if ((BoardInfo.sku & SKU_TOUCH_MASK) == SKU_TOUCH_2000) {
			atmel_mxt_info.config = config_sku2000;
			atmel_mxt_info.config_crc = MXT_CONFIG_CRC_SKU2000;
		}
		if (DisplayBoardInfo.board_id == BOARD_DISPLAY_E1506)
			i2c_register_board_info(1, e1506_atmel_i2c_info, 1);
		else
			i2c_register_board_info(1, atmel_i2c_info, 1);
	}
        */
        if(project_id != TEGRA3_PROJECT_P1801){
            gpio_request(TEGRA_GPIO_PH4, "touch-irq");
            gpio_direction_input(TEGRA_GPIO_PH4);
        }

        gpio_request(TEGRA_GPIO_PH6, "touch-reset");
        gpio_direction_output(TEGRA_GPIO_PH6, 0);
        msleep(1);
        gpio_set_value(TEGRA_GPIO_PH6, 1);
        msleep(100);
        switch(project_id){
	    case TEGRA3_PROJECT_TF201:
#if defined (CONFIG_TOUCHSCREEN_ATMEL_MT_T9)
	        i2c_register_board_info(1, atmel_i2c_info, 1);
#endif
	        break;
	    case TEGRA3_PROJECT_P1801:
	        gpio_request(TEGRA_GPIO_PS0, "tp_power");
	        gpio_direction_output(TEGRA_GPIO_PS0, 1);
	        gpio_request(TEGRA_GPIO_PR6, "tp_vendor");
	        gpio_direction_input(TEGRA_GPIO_PR6);
	        break;
	    case TEGRA3_PROJECT_TF300T:
	    case TEGRA3_PROJECT_TF300TG:
	    case TEGRA3_PROJECT_TF300TL:
#if defined(CONFIG_TOUCHSCREEN_ELAN_TF_3K)
	        i2c_register_board_info(1, elan_i2c_devices, 1);
#endif
	        break;
        case TEGRA3_PROJECT_ME301T:
	        gpio_request(TEGRA_GPIO_PH5, "ts_power");
	        gpio_direction_output(TEGRA_GPIO_PH5,0);
	        gpio_request(TEGRA_GPIO_PI2, "tp_vendor_ID0");
	        gpio_direction_input(TEGRA_GPIO_PI2);
	        gpio_request(TEGRA_GPIO_PI4, "tp_vendor_ID1");
	        gpio_direction_input(TEGRA_GPIO_PI4);
#if defined(CONFIG_TOUCHSCREEN_SIS_I2C)
	        i2c_register_board_info(1, sis9200_i2c2_boardinfo, 1);
#endif
	        break;
	case TEGRA3_PROJECT_TF700T:
	        gpio_request(TEGRA_GPIO_PK2, "tp_wake");
	        gpio_direction_output(TEGRA_GPIO_PK2, 1);
#if defined(CONFIG_TOUCHSCREEN_ELAN_TF_3K)
	        platform = (struct elan_ktf3k_i2c_platform_data *)elan_i2c_devices[0].platform_data;
	        platform->abs_x_max = ELAN_X_MAX_202T;
	        platform->abs_y_max = ELAN_Y_MAX_202T;
	        i2c_register_board_info(TOUCH_BUS_ATMEL_T9, elan_i2c_devices, 1);
#endif
	        break;
	    }
	return 0;
}

#if defined(CONFIG_USB_SUPPORT)

static void cardu_usb_hsic_postsupend(void)
{
#ifdef CONFIG_TEGRA_BB_XMM_POWER
	baseband_xmm_set_power_status(BBXMM_PS_L2);
#endif
}

static void cardu_usb_hsic_preresume(void)
{
#ifdef CONFIG_TEGRA_BB_XMM_POWER
	baseband_xmm_set_power_status(BBXMM_PS_L2TOL0);
#endif
}

static void cardu_usb_hsic_phy_ready(void)
{
#ifdef CONFIG_TEGRA_BB_XMM_POWER
	baseband_xmm_set_power_status(BBXMM_PS_L0);
#endif
}

static void cardu_usb_hsic_phy_off(void)
{
#ifdef CONFIG_TEGRA_BB_XMM_POWER
	baseband_xmm_set_power_status(BBXMM_PS_L3);
#endif
}

static struct tegra_usb_phy_platform_ops hsic_xmm_plat_ops = {
	.post_suspend = cardu_usb_hsic_postsupend,
	.pre_resume = cardu_usb_hsic_preresume,
	.port_power = cardu_usb_hsic_phy_ready,
	.post_phy_off = cardu_usb_hsic_phy_off,
};

static struct tegra_usb_platform_data tegra_ehci2_hsic_xmm_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_HSIC,
	.op_mode	= TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.enable_gpio = EN_HSIC_GPIO,
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = false,
		.power_off_on_suspend = false,
	},
	.ops = &hsic_xmm_plat_ops,
};
#endif

static int hsic_enable_gpio = -1;
static int hsic_reset_gpio = -1;

void hsic_platform_open(void)
{
	int reset_gpio = 0, enable_gpio = 0;

	if (hsic_enable_gpio != -1)
		enable_gpio = gpio_request(hsic_enable_gpio, "uhsic_enable");
	if (hsic_reset_gpio != -1)
		reset_gpio = gpio_request(hsic_reset_gpio, "uhsic_reset");
	/* hsic enable signal deasserted, hsic reset asserted */
	if (!enable_gpio)
		gpio_direction_output(hsic_enable_gpio, 0 /* deasserted */);
	if (!reset_gpio)
		gpio_direction_output(hsic_reset_gpio, 0 /* asserted */);
	/* keep hsic reset asserted for 1 ms */
	udelay(1000);
	/* enable (power on) hsic */
	if (!enable_gpio)
		gpio_set_value_cansleep(hsic_enable_gpio, 1);
	udelay(1000);
	/* deassert reset */
	if (!reset_gpio)
		gpio_set_value_cansleep(hsic_reset_gpio, 1);

}

void hsic_platform_close(void)
{
	if (hsic_enable_gpio != -1) {
		gpio_set_value(hsic_enable_gpio, 0);
		gpio_free(hsic_enable_gpio);
	}
	if (hsic_reset_gpio != -1) {
		gpio_set_value(hsic_reset_gpio, 0);
		gpio_free(hsic_reset_gpio);
	}
}

void hsic_power_on(void)
{
	if (hsic_enable_gpio != -1) {
		gpio_set_value_cansleep(hsic_enable_gpio, 1);
		udelay(1000);
	}
}

void hsic_power_off(void)
{
	if (hsic_enable_gpio != -1) {
		gpio_set_value_cansleep(hsic_enable_gpio, 0);
		udelay(1000);
	}
}

#if defined(CONFIG_USB_SUPPORT)
static struct tegra_usb_phy_platform_ops hsic_plat_ops = {
	.open = hsic_platform_open,
	.close = hsic_platform_close,
	.pre_phy_on = hsic_power_on,
	.post_phy_off = hsic_power_off,
};

static struct tegra_usb_platform_data tegra_ehci2_hsic_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_HSIC,
	.op_mode	= TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = false,
		.power_off_on_suspend = false,
	},
	.ops = &hsic_plat_ops,
};

static struct tegra_usb_platform_data tegra_udc_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_DEVICE,
	.u_data.dev = {
		.vbus_pmu_irq = 0,
		.vbus_gpio = -1,
		.charging_supported = false,
		.remote_wakeup_supported = false,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_platform_data tegra_ehci2_utmi_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode        = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = true,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 15,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_platform_data tegra_ehci3_utmi_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
//		.vbus_reg = "vdd_vbus_typea_usb",
		.hot_plug = true,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_platform_data tegra_ehci1_utmi_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		//.vbus_reg = "vdd_vbus_micro_usb",
		.hot_plug = true,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 15,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
};
#endif

struct platform_device *tegra_cardhu_usb_hsic_host_register(void)
{
	struct platform_device *pdev;
	int val;

	pdev = platform_device_alloc(tegra_ehci2_device.name,
		tegra_ehci2_device.id);
	if (!pdev)
		return NULL;

	val = platform_device_add_resources(pdev, tegra_ehci2_device.resource,
		tegra_ehci2_device.num_resources);
	if (val)
		goto error;

	pdev->dev.dma_mask =  tegra_ehci2_device.dev.dma_mask;
	pdev->dev.coherent_dma_mask = tegra_ehci2_device.dev.coherent_dma_mask;

	val = platform_device_add_data(pdev, &tegra_ehci2_hsic_xmm_pdata,
			sizeof(struct tegra_usb_platform_data));
	if (val)
		goto error;

	val = platform_device_add(pdev);
	if (val)
		goto error;

	return pdev;

error:
	pr_err("%s: failed to add the host contoller device\n", __func__);
	platform_device_put(pdev);
	return NULL;
}

void tegra_cardhu_usb_hsic_host_unregister(struct platform_device *pdev)
{
	platform_device_unregister(pdev);
}

struct platform_device *tegra_cardhu_usb_utmip_host_register(void)
{
	struct platform_device *pdev;
	int val;

	pdev = platform_device_alloc(tegra_ehci2_device.name,
		tegra_ehci2_device.id);
	if (!pdev)
		return NULL;

	val = platform_device_add_resources(pdev, tegra_ehci2_device.resource,
		tegra_ehci2_device.num_resources);
	if (val)
		goto error;

	pdev->dev.dma_mask =  tegra_ehci2_device.dev.dma_mask;
	pdev->dev.coherent_dma_mask = tegra_ehci2_device.dev.coherent_dma_mask;

	val = platform_device_add_data(pdev, &tegra_ehci2_utmi_pdata,
			sizeof(struct tegra_usb_platform_data));
	if (val)
		goto error;

	val = platform_device_add(pdev);
	if (val)
		goto error;

	return pdev;

error:
	pr_err("%s: failed to add the host contoller device\n", __func__);
	platform_device_put(pdev);
	return NULL;
}

void tegra_cardhu_usb_utmip_host_unregister(struct platform_device *pdev)
{
	platform_device_unregister(pdev);
}

struct platform_device *tegra_usb3_utmip_host_register(void)
{
	struct platform_device *pdev;
	void *platform_data;
	int val;

	pdev = platform_device_alloc(tegra_ehci3_device.name, tegra_ehci3_device.id);
	if (!pdev)
		return NULL;

	val = platform_device_add_resources(pdev, tegra_ehci3_device.resource, tegra_ehci3_device.num_resources);
	if (val)
		goto error;

	pdev->dev.dma_mask =  tegra_ehci3_device.dev.dma_mask;
	pdev->dev.coherent_dma_mask = tegra_ehci3_device.dev.coherent_dma_mask;

	val = platform_device_add_data(pdev, &tegra_ehci3_utmi_pdata, sizeof(struct tegra_usb_platform_data));

	if (val)
		goto error;

	val = platform_device_add(pdev);
	if (val)
		goto error;

	return pdev;

error:
	pr_err("%s: failed to add the host contoller device\n", __func__);
	platform_device_put(pdev);
	return NULL;
}

void tegra_usb3_utmip_host_unregister(struct platform_device *pdev)
{
	platform_device_unregister(pdev);
}

#if defined(CONFIG_USB_SUPPORT)
static void cardhu_usb_init(void)
{
	int ret;
	struct board_info bi;
	u32 project_info = tegra3_get_project_id();

	tegra_get_board_info(&bi);

	if (project_info == TEGRA3_PROJECT_P1801) {
		ret = gpio_request(TEGRA_GPIO_PH7, "usb2_vbus_control");
		if (ret < 0)
			printk(KERN_ERR "%s: Failed to gpio_request for usb2_vbus_control.\n", __func__);
		ret = gpio_direction_output(TEGRA_GPIO_PH7, 1);
		if (ret)
			printk(KERN_ERR "%s: Failed to gpio_direction_output for usb2_vbus_control.\n", __func__);
	}

	/* OTG should be the first to be registered */
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	/* setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

	if (bi.board_id == BOARD_PM267) {
		hsic_enable_gpio = EN_HSIC_GPIO;
		hsic_reset_gpio = PM267_SMSC4640_HSIC_HUB_RESET_GPIO;
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_hsic_pdata;
		platform_device_register(&tegra_ehci2_device);
	} else if (bi.board_id == BOARD_E1256) {
		hsic_enable_gpio = EN_HSIC_GPIO;
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_hsic_pdata;
		platform_device_register(&tegra_ehci2_device);
	} else if (bi.board_id == BOARD_E1186) {
		tegra_ehci2_device.dev.platform_data =
						&tegra_ehci2_hsic_xmm_pdata;
		/* ehci2 registration happens in baseband-xmm-power  */
	}

	if (project_info == TEGRA3_PROJECT_TF300TL) {
		printk("[TF300TL] register tegra_ehci2_device\n");
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	} else if (project_info == TEGRA3_PROJECT_P1801) {
		printk("[P1801] register tegra_ehci2_device\n");
		tegra_ehci2_utmi_pdata.u_data.host.hot_plug = true;
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	}else if (project_info == TEGRA3_PROJECT_TF300TG) {
		printk("[TF300TG] register tegra_ehci2_device\n");
		tegra_ehci2_utmi_pdata.u_data.host.power_off_on_suspend = false;
		tegra_ehci2_device.dev.platform_data =  &tegra_ehci2_hsic_xmm_pdata;
		/* ehci2 registration happens in baseband-xmm-power  */
	}

	tegra_ehci3_device.dev.platform_data = &tegra_ehci3_utmi_pdata;
	platform_device_register(&tegra_ehci3_device);

}
#else
static void cardhu_usb_init(void) { }
#endif

static void cardhu_audio_init(void)
{
	struct board_info board_info;

	unsigned int project_id = tegra3_get_project_id();

	if (project_id == TEGRA3_PROJECT_ME301T) {
		/* Set codec rt5642 in normal mode */
		gpio_request(CODEC_RT5642_RESET, "codec_rt5642_reset");
		gpio_direction_output(CODEC_RT5642_RESET, 1);
	}
}

static struct baseband_power_platform_data tegra_baseband_power_data = {
	.baseband_type = BASEBAND_XMM,
	.modem = {
	.xmm = {
			.bb_rst = XMM_GPIO_BB_RST,
			.bb_on = XMM_GPIO_BB_ON,
			.bb_vbat = BB_GPIO_VBAT_ON,
			.bb_rst_ind = BB_GPIO_RESET_IND,
			.bb_vbus = BB_GPIO_VBUS_ON,
			.bb_sw_sel = BB_GPIO_SW_SEL,
			.bb_sim_cd = BB_GPIO_SIM_DET,
			.bb_sar_det = BB_GPIO_SAR_DET,
			.ipc_bb_wake = XMM_GPIO_IPC_BB_WAKE,
			.ipc_ap_wake = XMM_GPIO_IPC_AP_WAKE,
			.ipc_hsic_active = XMM_GPIO_IPC_HSIC_ACTIVE,
			.ipc_hsic_sus_req = XMM_GPIO_IPC_HSIC_SUS_REQ,
			.ipc_bb_force_crash = XMM_GPIO_IPC_BB_FORCE_CRASH,
			.hsic_device = &tegra_ehci2_device,
		},
	},
};

static struct platform_device tegra_baseband_power_device = {
	.name = "baseband_xmm_power",
	.id = -1,
	.dev = {
		.platform_data = &tegra_baseband_power_data,
	},
};

static struct platform_device tegra_baseband_power2_device = {
	.name = "baseband_xmm_power2",
	.id = -1,
	.dev = {
		.platform_data = &tegra_baseband_power_data,
	},
};


static struct tegra_pci_platform_data cardhu_pci_platform_data = {
	.port_status[0]	= 1,
	.port_status[1]	= 1,
	.port_status[2]	= 1,
	.use_dock_detect	= 0,
	.gpio		= 0,
};

static void cardhu_pci_init(void)
{
	struct board_info board_info;

	tegra_get_board_info(&board_info);
	if (board_info.board_id == BOARD_E1291) {
		cardhu_pci_platform_data.port_status[0] = 0;
		cardhu_pci_platform_data.port_status[1] = 0;
		cardhu_pci_platform_data.port_status[2] = 1;
		cardhu_pci_platform_data.use_dock_detect = 1;
		cardhu_pci_platform_data.gpio = DOCK_DETECT_GPIO;
	}
	if ((board_info.board_id == BOARD_E1186) ||
		(board_info.board_id == BOARD_E1187) ||
		(board_info.board_id == BOARD_E1291)) {
		tegra_pci_device.dev.platform_data = &cardhu_pci_platform_data;
		platform_device_register(&tegra_pci_device);
	}
}

static void cardhu_modem_init(void)
{
	struct board_info board_info;
	int w_disable_gpio, ret;

	int modem_id = tegra_get_modem_id();

	switch (tegra3_get_project_id()) {
	case TEGRA3_PROJECT_TF300TG:
		// continue init
		break;
	case TEGRA3_PROJECT_TF300TL:
		// TF300TL init GPIOs in ASUS's customized RIL driver.
		return;
	default:
		printk("do not init modem-related components on non-mobile devices\n");
		return;
	}

	tegra_get_board_info(&board_info);
	switch (board_info.board_id) {
	case BOARD_E1291:
	case BOARD_E1198:
		if (((board_info.board_id == BOARD_E1291) &&
				(board_info.fab < BOARD_FAB_A03)) ||
			((board_info.board_id == BOARD_E1198) &&
					(board_info.fab < BOARD_FAB_A02))) {
			w_disable_gpio = TEGRA_GPIO_PH5;
		} else {
			w_disable_gpio = TEGRA_GPIO_PDD5;
		}

		ret = gpio_request(w_disable_gpio, "w_disable_gpio");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %d\n",
				__func__, w_disable_gpio);
		else
			gpio_direction_input(w_disable_gpio);

		/* E1291-A04 & E1198:A02: Set PERST signal to high */
		if (((board_info.board_id == BOARD_E1291) &&
				(board_info.fab >= BOARD_FAB_A04)) ||
			((board_info.board_id == BOARD_E1198) &&
					(board_info.fab >= BOARD_FAB_A02))) {
			ret = gpio_request(TEGRA_GPIO_PH7, "modem_perst");
			if (ret < 0) {
				pr_err("%s(): Error in allocating gpio "
					"TEGRA_GPIO_PH7\n", __func__);
				break;
			}
			gpio_direction_output(TEGRA_GPIO_PH7, 1);
		}
		break;
	case BOARD_E1186:
		platform_device_register(&tegra_baseband_power_device);
		platform_device_register(&tegra_baseband_power2_device);
		break;
	case BOARD_PM269:
		printk("BOARD_PM269");
		tegra_baseband_power_data.hsic_register =
			&tegra_cardhu_usb_hsic_host_register;
		tegra_baseband_power_data.hsic_unregister =
			&tegra_cardhu_usb_hsic_host_unregister;
		tegra_baseband_power_data.utmip_register =
			&tegra_cardhu_usb_utmip_host_register;
		tegra_baseband_power_data.utmip_unregister =
			&tegra_cardhu_usb_utmip_host_unregister;
		platform_device_register(&tegra_baseband_power_device);
		platform_device_register(&tegra_baseband_power2_device);

	default:
		break;
	}

	if (modem_id == TEGRA_BB_TANGO) {
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	}

}

#ifdef CONFIG_SATA_AHCI_TEGRA
static void cardhu_sata_init(void)
{
	platform_device_register(&tegra_sata_device);
}
#else
static void cardhu_sata_init(void) { }
#endif

extern void tegra_booting_info(void );
static void ME301T_HWid_init(void)
{
        gpio_request(TEGRA_GPIO_PQ1, "ME301T_HW_ID0");//KBC_COL1
        gpio_direction_input(TEGRA_GPIO_PQ1);

        gpio_request(TEGRA_GPIO_PQ6, "ME301T_HW_ID1");//KBC_COL6
        gpio_direction_input(TEGRA_GPIO_PQ6);

        gpio_request(TEGRA_GPIO_PS5, "ME301T_HW_ID2");//KBC_ROW13
        gpio_direction_input(TEGRA_GPIO_PS5);
        printk("%s: HW_ID[2:0] = (%d, %d, %d)\n", __func__, gpio_get_value(TEGRA_GPIO_PS5), \
                gpio_get_value(TEGRA_GPIO_PQ6), gpio_get_value(TEGRA_GPIO_PQ1));
}
static void __init tegra_cardhu_init(void)
{
        u32 project_info = tegra3_get_project_id();
	/* input chip uid for initialization of kernel misc module */
	cardhu_misc_init(tegra_chip_uid());
	tegra_thermal_init(&thermal_data,
				throttle_list,
				ARRAY_SIZE(throttle_list));
	tegra_clk_init_from_table(cardhu_clk_init_table);
	tegra_soc_device_init("cardhu");
	cardhu_pinmux_init();
	cardhu_misc_reset();
	tegra_booting_info();
	cardhu_i2c_init();
	cardhu_spi_init();
	cardhu_usb_init();
#ifdef CONFIG_TEGRA_EDP_LIMITS
	cardhu_edp_init();
#endif
	cardhu_uart_init();
	tegra_camera_init();
	platform_add_devices(cardhu_devices, ARRAY_SIZE(cardhu_devices));
	tegra_ram_console_debug_init();
	tegra_io_dpd_init();
	cardhu_sdhci_init();
	cardhu_regulator_init();
	cardhu_dtv_init();
	cardhu_suspend_init();
	cardhu_touch_init();
	cardhu_audio_init();
	if (project_info == TEGRA3_PROJECT_TF300TG)
		cardhu_modem_init();
	cardhu_keys_init();
	cardhu_panel_init();
	cardhu_pmon_init();
	cardhu_sensors_init();
#ifdef CONFIG_BT_BLUESLEEP
	cardhu_setup_bluesleep();
#elif defined CONFIG_BLUEDROID_PM
	cardhu_setup_bluedroid_pm();
#endif
	cardhu_sata_init();
	//audio_wired_jack_init();
	cardhu_pins_state_init();
	cardhu_emc_init();
	tegra_release_bootloader_fb();
	cardhu_pci_init();
	if(project_info == TEGRA3_PROJECT_ME301T)
		ME301T_HWid_init();
#ifdef CONFIG_TEGRA_WDT_RECOVERY
	tegra_wdt_recovery_init();
#endif
}

static void __init tegra_cardhu_reserve(void)
{
#if defined(CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM)
	/* support 1920X1200 with 24bpp */
	tegra_reserve(0, SZ_8M + SZ_1M, SZ_8M + SZ_1M);
#else
	tegra_reserve(SZ_128M, SZ_8M, SZ_8M);
#endif
	tegra_ram_console_debug_reserve(SZ_1M);
}

static const char *cardhu_dt_board_compat[] = {
	"nvidia,cardhu",
	NULL
};

MACHINE_START(CARDHU, "cardhu")
	.boot_params    = 0x80000100,
	.map_io         = tegra_map_common_io,
	.reserve        = tegra_cardhu_reserve,
	.init_early	= tegra_init_early,
	.init_irq       = tegra_init_irq,
	.timer          = &tegra_timer,
	.init_machine   = tegra_cardhu_init,
	.dt_compat	= cardhu_dt_board_compat,
MACHINE_END
