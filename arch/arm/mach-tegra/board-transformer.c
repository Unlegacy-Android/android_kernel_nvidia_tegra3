/*
 * arch/arm/mach-tegra/board-transformer.c
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
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/i2c-tegra.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/spi/spi.h>
#include <linux/tegra_uart.h>
#include <linux/memblock.h>
#include <linux/spi-tegra.h>
#include <linux/rfkill-gpio.h>

#include <sound/wm8903.h>
#include <media/tegra_dtv.h>
#include <asm/hardware/gic.h>

#include <mach/edp.h>
#include <mach/clk.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/io_dpd.h>
#include <mach/io.h>
#include <mach/i2s.h>
#include <mach/tegra_asoc_pdata.h>
#include <mach/tegra_wm8903_pdata.h>
#include <mach/usb_phy.h>
#include <mach/board-transformer-misc.h>
#include <mach/tegra_fiq_debugger.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include "board.h"
#include "board-common.h"
#include "clock.h"
#include "common.h"
#include "board-transformer.h"
#include "devices.h"
#include "gpio-names.h"
#include "fuse.h"
#include "pm.h"
#include "baseband-xmm-power.h"
#include "wdt-recovery.h"

static struct resource cardhu_bcm4329_rfkill_resources[] = {
  	{
		.name   = "bcm4329_nshutdown_gpio",
  		.start  = TEGRA_GPIO_PU0,
  		.end    = TEGRA_GPIO_PU0,
  		.flags  = IORESOURCE_IO,
  	},
  };

static struct platform_device cardhu_bcm4329_rfkill_device = {
	.name           = "bcm4329_rfkill",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(cardhu_bcm4329_rfkill_resources),
	.resource       = cardhu_bcm4329_rfkill_resources,
};

static struct resource cardhu_bluesleep_resources[] = {
	[0] = {
		.name	= "gpio_host_wake",
		.start	= TEGRA_GPIO_PU6,
		.end	= TEGRA_GPIO_PU6,
		.flags	= IORESOURCE_IO,
	},
	[1] = {
		.name	= "gpio_ext_wake",
		.start	= TEGRA_GPIO_PU1,
		.end	= TEGRA_GPIO_PU1,
		.flags	= IORESOURCE_IO,
	},
	[2] = {
		.name	= "host_wake",
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static struct platform_device cardhu_bluesleep_device = {
	.name			= "bluesleep",
	.id			= -1,
	.num_resources		= ARRAY_SIZE(cardhu_bluesleep_resources),
	.resource		= cardhu_bluesleep_resources,
};

#ifdef CONFIG_BT_BLUESLEEP
extern void bluesleep_setup_uart_port(struct platform_device *uart_dev);
#endif

static noinline void __init cardhu_setup_bluesleep(void)
{
	cardhu_bluesleep_resources[2].start = cardhu_bluesleep_resources[2].end =
		gpio_to_irq(TEGRA_GPIO_PU6);
	platform_device_register(&cardhu_bluesleep_device);
#ifdef CONFIG_BT_BLUESLEEP
	bluesleep_setup_uart_port(&tegra_uartc_device);
#endif
}

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
	.adapter_nr		= 0,
	.bus_count		= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PC4, 0},
	.sda_gpio		= {TEGRA_GPIO_PC5, 0},
	.arb_recovery		= arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c2_platform_data = {
	.adapter_nr		= 1,
	.bus_count		= 1,
	.bus_clk_rate	= { 400000, 0 },
	.is_clkon_always = true,
	.scl_gpio		= {TEGRA_GPIO_PT5, 0},
	.sda_gpio		= {TEGRA_GPIO_PT6, 0},
	.arb_recovery		= arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c3_platform_data = {
	.adapter_nr		= 2,
	.bus_count		= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB1, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB2, 0},
	.arb_recovery		= arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c3_platform_data_for_TF300 = {
	.adapter_nr		= 2,
	.bus_count		= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB1, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB2, 0},
	.arb_recovery		= arb_lost_recovery,
};

/* Higher freq could be applied for TF300T/TF300TG/TF300TL Camera FW update */
static struct tegra_i2c_platform_data cardhu_i2c4_platform_data = {
	.adapter_nr		= 3,
	.bus_count		= 1,
	.bus_clk_rate	= { 93750, 0 },
	.scl_gpio		= {TEGRA_GPIO_PV4, 0},
	.sda_gpio		= {TEGRA_GPIO_PV5, 0},
	.arb_recovery		= arb_lost_recovery,
};

static struct tegra_i2c_platform_data cardhu_i2c5_platform_data = {
	.adapter_nr		= 4,
	.bus_count		= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PZ6, 0},
	.sda_gpio		= {TEGRA_GPIO_PZ7, 0},
	.arb_recovery		= arb_lost_recovery,
};

static struct wm8903_platform_data cardhu_wm8903_pdata = {
	.irq_active_low = 0,
	.micdet_cfg = 0,
	.micdet_delay = 100,
	.gpio_base = CARDHU_GPIO_WM8903(0),
	.gpio_cfg = {
		(WM8903_GPn_FN_DMIC_LR_CLK_OUTPUT << WM8903_GP1_FN_SHIFT),
		(WM8903_GPn_FN_DMIC_LR_CLK_OUTPUT << WM8903_GP2_FN_SHIFT) |
		WM8903_GP2_DIR,
		WM8903_GPIO_CONFIG_ZERO,
		0,
		0,
	},
};

#ifdef CONFIG_DSP_FM34
static struct i2c_board_info __initdata dsp_board_info = {
	I2C_BOARD_INFO("dsp_fm34", 0x60),
};
#endif

static struct i2c_board_info __initdata rt5631_board_info = {
	I2C_BOARD_INFO("rt5631", 0x1a),
};

static struct i2c_board_info __initdata wm8903_board_info = {
	I2C_BOARD_INFO("wm8903", 0x1a),
	.platform_data = &cardhu_wm8903_pdata,
};

static struct i2c_board_info __initdata cardhu_i2c_asuspec_info[] = {
	{
		I2C_BOARD_INFO("asuspec", 0x15),
	},
	{
		I2C_BOARD_INFO("asusdec", 0x19),
	},
};

static void cardhu_i2c_init(void)
{
	u32 project_info = tegra3_get_project_id();

	tegra_i2c_device1.dev.platform_data = &cardhu_i2c1_platform_data;
	tegra_i2c_device2.dev.platform_data = &cardhu_i2c2_platform_data;
	if ((project_info == TEGRA3_PROJECT_TF300T) ||
		(project_info == TEGRA3_PROJECT_TF300TG) ||
		(project_info == TEGRA3_PROJECT_TF300TL)) {
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

	i2c_register_board_info(1, cardhu_i2c_asuspec_info, ARRAY_SIZE(cardhu_i2c_asuspec_info));

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
		default:
			break;
	}

#ifdef CONFIG_DSP_FM34
	i2c_register_board_info(0, &dsp_board_info, 1);
#endif
}

static struct platform_device *cardhu_uart_devices[] __initdata = {
	&tegra_uarta_device,
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

	debug_port_id = uart_console_debug_init(0);
	if (debug_port_id < 0)
		return;

	if (debug_port_id >= ARRAY_SIZE(debug_uarts)) {
		pr_info("The debug console id %d is invalid, Assuming UARTA",
			debug_port_id);
		debug_port_id = 0;
	}

	cardhu_uart_devices[debug_port_id] = uart_console_debug_device;
}

static void __init cardhu_uart_init(void)
{
	struct clk *c;
	int i;

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
	tegra_uarta_device.dev.platform_data = &cardhu_uart_pdata;
	tegra_uartb_device.dev.platform_data = &cardhu_uart_pdata;
	tegra_uartc_device.dev.platform_data = &cardhu_uart_pdata;
	tegra_uartd_device.dev.platform_data = &cardhu_uart_pdata;
	/* UARTE is used for loopback test purpose */
	tegra_uarte_device.dev.platform_data = &cardhu_loopback_uart_pdata;

	/* Register low speed only if it is selected */
	if (!is_tegra_debug_uartport_hs())
		uart_debug_init();

	tegra_serial_debug_init(debug_uart_port_base, debug_uart_port_irq,
				debug_uart_clk, -1, -1);

	platform_add_devices(cardhu_uart_devices,
			ARRAY_SIZE(cardhu_uart_devices));
}

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.id = -1,
};

static struct platform_device *cardhu_spi_devices[] __initdata = {
	&tegra_spi_device4,
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
	.max_rate			= 100000000,
};

static void __init cardhu_spi_init(void)
{
	int i;
	struct clk *c;

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

static struct tegra_asoc_platform_data cardhu_audio_pdata = {
	.gpio_spkr_en		= TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det		= TEGRA_GPIO_HP_DET,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= -1,
	.gpio_ext_mic_en	= -1,
	.i2s_param[HIFI_CODEC] = {
		.audio_port_id	= 0,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
	},
	.i2s_param[BASEBAND] = {
		.audio_port_id	= -1,
	},
	.i2s_param[BT_SCO] = {
		.audio_port_id	= 3,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_DSP_A,
	},
};

static struct platform_device cardhu_audio_device = {
	.name	= "tegra-snd-codec",
	.id		= 0,
	.dev	= {
		.platform_data = &cardhu_audio_pdata,
	},
};

static struct platform_device *cardhu_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_rtc_device,
	&tegra_udc_device,
	&tegra_wdt0_device,
#ifdef CONFIG_TEGRA_AVP
	&tegra_avp_device,
#endif
	&tegra_camera,
#ifdef CONFIG_CRYPTO_DEV_TEGRA_SE
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
	&cardhu_bcm4329_rfkill_device,
	&tegra_pcm_device,
	&cardhu_audio_device,
	&tegra_hda_device,
	&tegra_cec_device,
#ifdef CONFIG_CRYPTO_DEV_TEGRA_AES
	&tegra_aes_device,
#endif
};

#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT
// Interrupt pin: TEGRA_GPIO_PH4
// Reset pin: TEGRA_GPIO_PH6

#include <linux/i2c/atmel_mxt_ts.h>

static u8 read_chg(void)
{
	return gpio_get_value(TEGRA_GPIO_PH4);
}

static struct mxt_platform_data atmel_mxt_info = {
	.irqflags		= IRQF_ONESHOT | IRQF_TRIGGER_LOW,
	.read_chg		= &read_chg,
};

static struct i2c_board_info atmel_i2c_info[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", MXT1386_I2C_ADDR2),
		.flags = I2C_CLIENT_WAKE,
		.platform_data = &atmel_mxt_info,
	}
};
#endif

#ifdef CONFIG_TOUCHSCREEN_ELAN_TF_3K
// Interrupt pin: TEGRA_GPIO_PH4
// Reset pin: TEGRA_GPIO_PH6

#include <linux/i2c/ektf3k.h>

#define ELAN_X_MAX			2240
#define ELAN_Y_MAX			1408
#define ELAN_X_MAX_202T		2944
#define ELAN_Y_MAX_202T		1856

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
	},
};
#endif

static int __init cardhu_touch_init(void)
{
#ifdef CONFIG_TOUCHSCREEN_ELAN_TF_3K
	struct elan_ktf3k_i2c_platform_data *platform;
#endif

	gpio_request(TEGRA_GPIO_PH4, "touch-irq");
	gpio_direction_input(TEGRA_GPIO_PH4);

	gpio_request(TEGRA_GPIO_PH6, "touch-reset");
	gpio_direction_output(TEGRA_GPIO_PH6, 0);

	msleep(1);
	gpio_set_value(TEGRA_GPIO_PH6, 1);
	msleep(100);

	switch(tegra3_get_project_id()){
		case TEGRA3_PROJECT_TF201:
#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT
			atmel_i2c_info[0].irq = gpio_to_irq(TEGRA_GPIO_PH4);
			i2c_register_board_info(1, atmel_i2c_info, 1);
#endif
			break;
		case TEGRA3_PROJECT_TF300T:
		case TEGRA3_PROJECT_TF300TG:
		case TEGRA3_PROJECT_TF300TL:
#ifdef CONFIG_TOUCHSCREEN_ELAN_TF_3K
			elan_i2c_devices[0].irq = gpio_to_irq(TEGRA_GPIO_PH4);
			i2c_register_board_info(1, elan_i2c_devices, 1);
#endif
			break;
		case TEGRA3_PROJECT_TF700T:
			gpio_request(TEGRA_GPIO_PK2, "tp_wake");
			gpio_direction_output(TEGRA_GPIO_PK2, 1);
#ifdef CONFIG_TOUCHSCREEN_ELAN_TF_3K
			platform = (struct elan_ktf3k_i2c_platform_data *)elan_i2c_devices[0].platform_data;
			platform->abs_x_max = ELAN_X_MAX_202T;
			platform->abs_y_max = ELAN_Y_MAX_202T;
			elan_i2c_devices[0].irq = gpio_to_irq(TEGRA_GPIO_PH4);
			i2c_register_board_info(1, elan_i2c_devices, 1);
#endif
			break;
	}
	return 0;
}

#ifdef CONFIG_USB_SUPPORT
static void cardu_usb_hsic_postsupend(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L2);
}

static void cardu_usb_hsic_preresume(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L2TOL0);
}

static void cardu_usb_hsic_phy_ready(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L0);
}

static void cardu_usb_hsic_phy_off(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L3);
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
//		.enable_gpio = EN_HSIC_GPIO,
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = false,
		.power_off_on_suspend = false,
	},
	.ops = &hsic_xmm_plat_ops,
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

static struct tegra_usb_platform_data tegra_ehci1_utmi_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
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

static struct tegra_usb_platform_data tegra_ehci2_utmi_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
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

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
};

static void __init cardhu_usb_init(void)
{
	/* OTG should be the first to be registered */
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	/* setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF300TL) {
		pr_info("[TF300TL] register tegra_ehci2_device\n");
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	} else if (tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG) {
		pr_info("[TF300TG] register tegra_ehci2_device\n");
		tegra_ehci2_utmi_pdata.u_data.host.power_off_on_suspend = false;
		tegra_ehci2_device.dev.platform_data =  &tegra_ehci2_hsic_xmm_pdata;
		/* ehci2 registration happens in baseband-xmm-power  */
	}

	tegra_ehci3_device.dev.platform_data = &tegra_ehci3_utmi_pdata;
	platform_device_register(&tegra_ehci3_device);
}

#if 0
static struct platform_device *tegra_cardhu_usb_hsic_host_register(void)
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

static void tegra_cardhu_usb_hsic_host_unregister(struct platform_device *pdev)
{
	platform_device_unregister(pdev);

	if (pdev && &pdev->dev) {
		platform_device_unregister(pdev);
		*platdev = NULL;
	} else
		pr_err("%s: no platform device\n", __func__);
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

static struct baseband_power_platform_data tegra_baseband_power_data = {
	.baseband_type = BASEBAND_XMM,
	.modem = {
	.xmm = {
		.bb_rst = XMM_GPIO_BB_RST,
		.bb_on = XMM_GPIO_BB_ON,
		.bb_vbat = XMM_GPIO_BB_VBAT,
		.bb_vbus = XMM_GPIO_BB_VBUS,
		.bb_sw_sel = XMM_GPIO_BB_SW_SEL,
		.sim_card_det = XMM_GPIO_SIM_CARD_DET,
		.ipc_bb_rst_ind = XMM_GPIO_IPC_BB_RST_IND,
		.ipc_bb_wake = XMM_GPIO_IPC_BB_WAKE,
		.ipc_ap_wake = XMM_GPIO_IPC_AP_WAKE,
		.ipc_hsic_active = XMM_GPIO_IPC_HSIC_ACTIVE,
		.ipc_hsic_sus_req = XMM_GPIO_IPC_HSIC_SUS_REQ,
		.ipc_bb_force_crash = XMM_GPIO_IPC_BB_FORCE_CRASH,

		.bb_sar_det = BB_GPIO_SAR_DET,
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

static void __init cardhu_modem_init(void)
{
	int modem_id = tegra_get_modem_id();

	switch (tegra3_get_project_id()) {
	case TEGRA3_PROJECT_TF300TG:
		// continue init
		break;
	case TEGRA3_PROJECT_TF300TL:
		// TF300TL init GPIOs in ASUS's customized RIL driver.
		return;
	default:
		pr_info("%s: Device doesn't include modile data module\n", __func__);
		return;
	}

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

	if (modem_id == TEGRA_BB_TANGO) {
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	}
}
#endif

#else
static void __init cardhu_usb_init(void)  { }
//static void __init cardhu_modem_init(void)  { }
#endif /* CONFIG_USB_SUPPORT */

static void __init cardhu_booting_info(void)
{
	static void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	unsigned int reg;
	#define PMC_RST_STATUS_WDT (1)
	#define PMC_RST_STATUS_SW  (3)

	reg = readl(pmc + 0x1b4);
	pr_info("tegra_booting_info reg = %x\n", reg);

	if (reg == PMC_RST_STATUS_SW){
		pr_info("tegra_booting_info-SW reboot\n");
	} else if (reg == PMC_RST_STATUS_WDT){
		pr_info("tegra_booting_info-watchdog reboot\n");
	} else {
		pr_info("tegra_booting_info-normal\n");
	}
}

static void __init tegra_cardhu_init(void)
{
	/* input chip uid for initialization of kernel misc module */
	tegra_clk_init_from_table(cardhu_clk_init_table);
	tegra_enable_pinmux();
	tegra_smmu_init();
	tegra_soc_device_init("cardhu");
	cardhu_pinmux_init_early();
	cardhu_booting_info();
	cardhu_misc_init(tegra_chip_uid());
	cardhu_pinmux_init();
	cardhu_gpio_init();
	cardhu_misc_reset();
	cardhu_i2c_init();
	cardhu_spi_init();
	cardhu_usb_init();
#ifdef CONFIG_TEGRA_EDP_LIMITS
	cardhu_edp_init();
#endif
	cardhu_uart_init();
	platform_add_devices(cardhu_devices, ARRAY_SIZE(cardhu_devices));
	tegra_ram_console_debug_init();
	tegra_io_dpd_init();
	cardhu_sdhci_init();
	cardhu_regulator_init();
	cardhu_suspend_init();
	cardhu_touch_init();
//	cardhu_modem_init();
	cardhu_keys_init();
	cardhu_panel_init();
	cardhu_sensors_init();
	cardhu_setup_bluesleep();
	cardhu_pins_state_init();
	cardhu_emc_init();
//	tegra_release_bootloader_fb();
#ifdef CONFIG_TEGRA_WDT_RECOVERY
	tegra_wdt_recovery_init();
#endif
	tegra_register_fuse();
}

static void __init tegra_cardhu_reserve(void)
{
#ifdef CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM
	/* support 1920X1200 with 24bpp */
	tegra_reserve(0, SZ_8M + SZ_1M, SZ_8M + SZ_1M);
#else
	tegra_reserve(SZ_128M, SZ_8M, SZ_8M);
#endif
}

static const char *cardhu_dt_board_compat[] = {
	"nvidia,cardhu",
	NULL
};

MACHINE_START(TRANSFORMER, "transformer")
	.atag_offset		= 0x100,
	.soc			= &tegra_soc_desc,
	.map_io			= tegra_map_common_io,
	.reserve		= tegra_cardhu_reserve,
	.init_early		= tegra30_init_early,
	.init_irq		= tegra_init_irq,
	.handle_irq		= gic_handle_irq,
	.timer			= &tegra_timer,
	.init_machine		= tegra_cardhu_init,
	.dt_compat		= cardhu_dt_board_compat,
	.restart		= tegra_assert_system_reset,
MACHINE_END
