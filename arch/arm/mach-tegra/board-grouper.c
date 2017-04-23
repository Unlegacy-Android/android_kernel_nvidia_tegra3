/*
 * arch/arm/mach-tegra/board-grouper.c
 *
 * Copyright (c) 2012, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include <linux/nfc/pn544.h>
#include <linux/skbuff.h>
#include <linux/regulator/consumer.h>
#include <linux/power/smb347-charger.h>
#include <linux/power/gpio-charger.h>
#include <linux/rfkill-gpio.h>

#include <asm/hardware/gic.h>

#include <mach/clk.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/pinmux-tegra30.h>
#include <mach/iomap.h>
#include <mach/io.h>
#include <mach/i2s.h>
#include <mach/tegra_asoc_pdata.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/usb_phy.h>
#include <mach/board-grouper-misc.h>
#include <mach/tegra_fiq_debugger.h>

#include "board.h"
#include "board-common.h"
#include "clock.h"
#include "board-grouper.h"
#include "baseband-xmm-power.h"
#include "devices.h"
#include "gpio-names.h"
#include "fuse.h"
#include "pm.h"
#include "wdt-recovery.h"
#include "common.h"

static struct rfkill_gpio_platform_data grouper_bt_rfkill_pdata[] = {
	{
		.name		= "bt_rfkill",
		.shutdown_gpio	= TEGRA_GPIO_PU0,
		.reset_gpio	= TEGRA_GPIO_INVALID,
		.type		= RFKILL_TYPE_BLUETOOTH,
	},
};

static struct platform_device grouper_bt_rfkill_device = {
	.name 	= "rfkill_gpio",
	.id		= -1,
	.dev = {
		.platform_data = &grouper_bt_rfkill_pdata,
	},
};

static struct resource grouper_bluesleep_resources[] = {
	[0] = {
		.name = "gpio_host_wake",
			.start	= TEGRA_GPIO_PU6,
			.end	= TEGRA_GPIO_PU6,
			.flags	= IORESOURCE_IO,
	},
	[1] = {
		.name = "gpio_ext_wake",
			.start	= TEGRA_GPIO_PU1,
			.end	= TEGRA_GPIO_PU1,
			.flags	= IORESOURCE_IO,
	},
	[2] = {
		.name = "host_wake",
			.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static struct platform_device grouper_bluesleep_device = {
	.name		= "bluesleep",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(grouper_bluesleep_resources),
	.resource	= grouper_bluesleep_resources,
};

#ifdef CONFIG_BT_BLUESLEEP
extern void bluesleep_setup_uart_port(struct platform_device *uart_dev);
#endif

static noinline void __init grouper_setup_bluesleep(void)
{
	grouper_bluesleep_resources[2].start = grouper_bluesleep_resources[2].end =
		gpio_to_irq(TEGRA_GPIO_PU6);

	platform_device_register(&grouper_bluesleep_device);

#ifdef CONFIG_BT_BLUESLEEP
	bluesleep_setup_uart_port(&tegra_uartc_device);
#endif
}

static __initdata struct tegra_clk_init_table grouper_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
	{ "hda",	"pll_p",	108000000,	false},
	{ "hda2codec_2x", "pll_p",	48000000,	false},
	{ "pwm",	"pll_p",	5100000,	false},
	{ "blink",	"clk_32k",	32768,		true},
	{ "i2s1",	"pll_a_out0",	0,		false},
	{ "i2s3",	"pll_a_out0",	0,		false},
	{ "i2s4",	"pll_a_out0",	0,		false},
	{ "spdif_out",	"pll_a_out0",	0,		false},
	{ "d_audio",	"clk_m",	12000000,	false},
	{ "dam0",	"clk_m",	12000000,	false},
	{ "dam1",	"clk_m",	12000000,	false},
	{ "dam2",	"clk_m",	12000000,	false},
	{ "audio1",	"i2s1_sync",	0,		false},
	{ "audio3",	"i2s3_sync",	0,		false},
	{ "vi_sensor",	"pll_p",	150000000,	false},
	{ "i2c1",	"pll_p",	3200000,	false},
	{ "i2c2",	"pll_p",	3200000,	false},
	{ "i2c3",	"pll_p",	3200000,	false},
	{ "i2c4",	"pll_p",	3200000,	false},
	{ "i2c5",	"pll_p",	3200000,	false},
	{ NULL,		NULL,		0,		0},
};

static struct pn544_i2c_platform_data nfc_pdata = {
	.irq_gpio = TEGRA_GPIO_PX0,
	.ven_gpio = TEGRA_GPIO_PS7,
	.firm_gpio = TEGRA_GPIO_PR3,
};

static struct i2c_board_info __initdata grouper_nfc_board_info[] = {
	{
		I2C_BOARD_INFO("pn544", 0x28),
		.platform_data = &nfc_pdata,
	},
};

static struct tegra_i2c_platform_data grouper_i2c1_platform_data = {
	.adapter_nr	= 0,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PC4, 0},
	.sda_gpio		= {TEGRA_GPIO_PC5, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data grouper_i2c2_platform_data = {
	.adapter_nr	= 1,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.is_clkon_always = true,
	.scl_gpio		= {TEGRA_GPIO_PT5, 0},
	.sda_gpio		= {TEGRA_GPIO_PT6, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data grouper_i2c3_platform_data = {
	.adapter_nr	= 2,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB1, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB2, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data grouper_i2c4_platform_data = {
	.adapter_nr	= 3,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PV4, 0},
	.sda_gpio		= {TEGRA_GPIO_PV5, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data grouper_i2c5_platform_data = {
	.adapter_nr	= 4,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PZ6, 0},
	.sda_gpio		= {TEGRA_GPIO_PZ7, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct i2c_board_info grouper_i2c4_bq27541_board_info[] = {
	{
		I2C_BOARD_INFO("bq27541", 0x55),
	}
};

static char *grouper_battery[] = {
	"battery",
};

static struct gpio_charger_platform_data dock_charger_pdata = {
	.name				= "grouper-dock-charger",
	.type				= POWER_SUPPLY_TYPE_USB_ACA,
	.gpio				= TEGRA_GPIO_PU4,
	.gpio_active_low	= 1,
	.supplied_to		= grouper_battery,
	.num_supplicants	= ARRAY_SIZE(grouper_battery),
};

static struct platform_device dock_device = {
	.name	= "grouper-dock",
	.id		= -1,
};

static struct smb347_charger_platform_data smb347_charger_pdata = {
	.battery_info = {
		.name		= "ME370",
		.technology	= POWER_SUPPLY_TECHNOLOGY_LION,
		.voltage_max_design	= 4200000,
		.voltage_min_design	= 3500000,
	},
	.max_charge_current	= 1800000,
	.max_charge_voltage	= 4200000,
	.usb_hc_current_limit	= 900000,
	.soft_cold_temp_limit	= SMB347_TEMP_USE_DEFAULT,
	.soft_hot_temp_limit	= 40,
	.hard_cold_temp_limit	= SMB347_TEMP_USE_DEFAULT,
	.hard_hot_temp_limit	= 45,
	.suspend_on_hard_temp_limit = true,
	.use_mains		= true,
	.use_usb		= true,
	.use_usb_otg		= false,
	.irq_gpio		= TEGRA_GPIO_PV1,
	.enable_control		= SMB347_CHG_ENABLE_SW,
	.usb_mode_pin_ctrl	= true,
	.supplied_to		= grouper_battery,
	.num_supplicants	= ARRAY_SIZE(grouper_battery),
};

static struct i2c_board_info grouper_i2c4_smb347_board_info[] = {
	{
		I2C_BOARD_INFO("smb347", 0x6a),
		.platform_data = &smb347_charger_pdata,
	},
};

static struct i2c_board_info __initdata rt5640_board_info = {
	I2C_BOARD_INFO("rt5640", 0x1c),
};

static void grouper_i2c_init(void)
{
	tegra_i2c_device1.dev.platform_data = &grouper_i2c1_platform_data;
	tegra_i2c_device2.dev.platform_data = &grouper_i2c2_platform_data;
	tegra_i2c_device3.dev.platform_data = &grouper_i2c3_platform_data;
	tegra_i2c_device4.dev.platform_data = &grouper_i2c4_platform_data;
	tegra_i2c_device5.dev.platform_data = &grouper_i2c5_platform_data;

	platform_device_register(&tegra_i2c_device5);
	platform_device_register(&tegra_i2c_device4);
	platform_device_register(&tegra_i2c_device3);
	platform_device_register(&tegra_i2c_device2);
	platform_device_register(&tegra_i2c_device1);

	i2c_register_board_info(4, grouper_i2c4_bq27541_board_info,
		ARRAY_SIZE(grouper_i2c4_bq27541_board_info));

	i2c_register_board_info(4, grouper_i2c4_smb347_board_info,
		ARRAY_SIZE(grouper_i2c4_smb347_board_info));

	rt5640_board_info.irq = gpio_to_irq(TEGRA_GPIO_CDC_IRQ);
	i2c_register_board_info(4, &rt5640_board_info, 1);

	if (grouper_get_project_id() == GROUPER_PROJECT_NAKASI_3G) {
		nfc_pdata.irq_gpio = TEGRA_GPIO_PS7;
		nfc_pdata.ven_gpio = TEGRA_GPIO_PP0;
		nfc_pdata.firm_gpio = TEGRA_GPIO_PP3;
		grouper_nfc_board_info[0].addr = (0x2A);
		grouper_nfc_board_info[0].irq = gpio_to_irq(TEGRA_GPIO_PS7);
	} else
		grouper_nfc_board_info[0].irq = gpio_to_irq(TEGRA_GPIO_PX0);
	i2c_register_board_info(2, grouper_nfc_board_info, 1);
}

static struct platform_device *grouper_uart_devices[] __initdata = {
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

static struct tegra_uart_platform_data grouper_uart_pdata;
static struct tegra_uart_platform_data grouper_loopback_uart_pdata;

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

	debug_port_id = uart_console_debug_init(3);
	if (debug_port_id < 0)
		return;
	
	if (debug_port_id >= ARRAY_SIZE(debug_uarts)) {
		pr_info("The debug console id %d is invalid, Assuming UARTA",
			debug_port_id);
		debug_port_id = 0;
	}
	
	grouper_uart_devices[debug_port_id] = uart_console_debug_device;
}

static void __init grouper_uart_init(void)
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
	grouper_uart_pdata.parent_clk_list = uart_parent_clk;
	grouper_uart_pdata.parent_clk_count = ARRAY_SIZE(uart_parent_clk);
	grouper_loopback_uart_pdata.parent_clk_list = uart_parent_clk;
	grouper_loopback_uart_pdata.parent_clk_count =
						ARRAY_SIZE(uart_parent_clk);
	grouper_loopback_uart_pdata.is_loopback = true;
	tegra_uartb_device.dev.platform_data = &grouper_uart_pdata;
	tegra_uartc_device.dev.platform_data = &grouper_uart_pdata;
	tegra_uartd_device.dev.platform_data = &grouper_uart_pdata;
	/* UARTE is used for loopback test purpose */
	tegra_uarte_device.dev.platform_data = &grouper_loopback_uart_pdata;

	/* Register low speed only if it is selected */
	if (!is_tegra_debug_uartport_hs())
		uart_debug_init();

	tegra_serial_debug_init(debug_uart_port_base, debug_uart_port_irq,
				debug_uart_clk, -1, -1);

	platform_add_devices(grouper_uart_devices,
				ARRAY_SIZE(grouper_uart_devices));
}

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.id = -1,
};

static struct platform_device *grouper_spi_devices[] __initdata = {
	&tegra_spi_device4,
	&tegra_spi_device1,
};

static struct spi_clk_parent spi_parent_clk[] = {
	[0] = {.name = "pll_p"},
#ifndef CONFIG_TEGRA_PLLM_RESTRICTED
	[1] = {.name = "pll_m"},
	[2] = {.name = "clk_m"},
#else
	[1] = {.name = "clk_m"},
#endif
};

static struct tegra_spi_platform_data grouper_spi_pdata = {
	.is_dma_based		= true,
	.max_dma_buffer		= (16 * 1024),
	.is_clkon_always	= false,
	.max_rate		= 100000000,
};

static void __init grouper_spi_init(void)
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

	grouper_spi_pdata.parent_clk_list = spi_parent_clk;
	grouper_spi_pdata.parent_clk_count = ARRAY_SIZE(spi_parent_clk);
	tegra_spi_device4.dev.platform_data = &grouper_spi_pdata;
	platform_add_devices(grouper_spi_devices,
				ARRAY_SIZE(grouper_spi_devices));
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
	.name	= "tegra_rtc",
	.id		= -1,
	.resource = tegra_rtc_resources,
	.num_resources = ARRAY_SIZE(tegra_rtc_resources),
};

static struct tegra_asoc_platform_data grouper_audio_pdata = {
	.gpio_spkr_en		= TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det		= TEGRA_GPIO_HP_DET,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= TEGRA_GPIO_INT_MIC_EN,
	.gpio_ext_mic_en	= TEGRA_GPIO_EXT_MIC_EN,
	.i2s_param[HIFI_CODEC]	= {
		.audio_port_id	= 0,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
	},
	.i2s_param[BT_SCO]	= {
		.audio_port_id	= 3,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_DSP_A,
	},
};

static struct platform_device grouper_audio_device = {
	.name	= "tegra-snd-rt5640",
	.id	= 0,
	.dev	= {
		.platform_data = &grouper_audio_pdata,
	},
};

static struct platform_device *grouper_devices[] __initdata = {
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
	&tegra_i2s_device1,
	&tegra_i2s_device3,
	&tegra_i2s_device4,
	&tegra_spdif_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&grouper_bt_rfkill_device,
	&tegra_pcm_device,
	&grouper_audio_device,
	&tegra_hda_device,
#ifdef CONFIG_CRYPTO_DEV_TEGRA_AES
	&tegra_aes_device,
#endif
};

#ifdef CONFIG_TOUCHSCREEN_ELAN_TF_3K
// Interrupt pin: TEGRA_GPIO_PH4
// Reset pin: TEGRA_GPIO_PH6

#include <linux/i2c/ektf3k.h>

static struct elan_ktf3k_i2c_platform_data ts_elan_ktf3k_data[] = {
	{
		.version = 0x0001,
		.abs_x_min = 0,
		.abs_x_max = ELAN_X_MAX_370T - 1,
		.abs_y_min = 0,
		.abs_y_max = ELAN_Y_MAX_370T - 1,
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

static int __init grouper_touch_init(void)
{
	gpio_request(TEGRA_GPIO_PH3, "elan-pwn");
	gpio_direction_output(TEGRA_GPIO_PH3, 1);

	gpio_request(TEGRA_GPIO_PH4, "elan-irq");
	gpio_direction_input(TEGRA_GPIO_PH4);

	gpio_request(TEGRA_GPIO_PH6, "elan-reset");
	gpio_direction_output(TEGRA_GPIO_PH6, 0);
	msleep(1);
	gpio_set_value(TEGRA_GPIO_PH6, 1);
	msleep(100);

#ifdef CONFIG_TOUCHSCREEN_ELAN_TF_3K
	elan_i2c_devices[0].irq = gpio_to_irq(TEGRA_GPIO_PH4);
	pr_info("%s: registering ELAN touchscreen driver\n", __func__);
	i2c_register_board_info(1, elan_i2c_devices, 1);
#endif
	return 0;
}

#if defined(CONFIG_USB_SUPPORT)

static int hsic_enable_gpio = EN_HSIC_GPIO;

void grouper_usb_hsic_postsupend(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L2);
}

void grouper_usb_hsic_preresume(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L2TOL0);
}

static void grouper_usb_hsic_post_resume(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L0);
}

void grouper_usb_hsic_phy_ready(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L0);
}

void grouper_usb_hsic_phy_off(void)
{
	pr_debug("%s\n", __func__);
	baseband_xmm_set_power_status(BBXMM_PS_L2);
	if (hsic_enable_gpio != -1) {
		gpio_set_value_cansleep(hsic_enable_gpio, 0);
		udelay(1000);
		tegra_baseband_rail_off();
	}
}

void grouper_usb_hsic_phy_on(void)
{
	pr_debug("%s\n", __func__);
	if (hsic_enable_gpio != -1) {
		tegra_baseband_rail_on();
		gpio_set_value_cansleep(hsic_enable_gpio, 1);
		udelay(1000);
	}
}

void grouper_hsic_platform_open(void)
{
	int enable_gpio = -1;

	pr_debug("%s\n", __func__);
	if (hsic_enable_gpio != -1)
		enable_gpio = gpio_request(hsic_enable_gpio, "uhsic_enable");
	if (!enable_gpio)
		gpio_direction_output(hsic_enable_gpio, 0 /* deasserted */);
	/* keep hsic reset asserted for 1 ms */
	udelay(1000);
	/* enable (power on) hsic */
	if (!enable_gpio)
		gpio_set_value_cansleep(hsic_enable_gpio, 1);
	udelay(1000);
}

static void grouper_otg_power(int enable)
{
	struct power_supply *smb_usb = power_supply_get_by_name("smb347-usb");
	union power_supply_propval usb_otg = { enable };

	pr_debug("%s: %d\n", __func__, enable);

	if (smb_usb && smb_usb->set_property)
		smb_usb->set_property(
			smb_usb,
			POWER_SUPPLY_PROP_USB_OTG,
			&usb_otg);
	else
		pr_err("%s: couldn't get power supply\n", __func__);
}

void grouper_usb_ehci1_phy_on(void)
{
	grouper_otg_power(1);
}

void grouper_usb_echi1_phy_off(void)
{
	grouper_otg_power(0);
}

static struct tegra_usb_phy_platform_ops uhsic_pdata_ops = {
	.open = &grouper_hsic_platform_open,
	.pre_phy_on = &grouper_usb_hsic_phy_on,
	.post_suspend = &grouper_usb_hsic_postsupend,
	.pre_resume = &grouper_usb_hsic_preresume,
	.post_resume = &grouper_usb_hsic_post_resume,
	.port_power = &grouper_usb_hsic_phy_ready,
	.post_phy_off = &grouper_usb_hsic_phy_off,
};

static struct tegra_usb_phy_platform_ops ehci1_pdata_ops = {
	.pre_phy_on = &grouper_usb_ehci1_phy_on,
	.post_phy_off = &grouper_usb_echi1_phy_off,
};

static struct tegra_usb_platform_data tegra_ehci_uhsic_pdata = {
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
	.ops = &uhsic_pdata_ops,
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
	.ops = &ehci1_pdata_ops,
};

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
};

static struct platform_device *
tegra_usb_hsic_host_register(struct platform_device *ehci_dev)
{
	struct platform_device *pdev;
	int val;

	pdev = platform_device_alloc(ehci_dev->name, ehci_dev->id);
	if (!pdev)
		return NULL;

	val = platform_device_add_resources(pdev, ehci_dev->resource,
						ehci_dev->num_resources);
	if (val)
		goto error;

	pdev->dev.dma_mask = ehci_dev->dev.dma_mask;
	pdev->dev.coherent_dma_mask = ehci_dev->dev.coherent_dma_mask;

	val = platform_device_add_data(pdev, &tegra_ehci_uhsic_pdata,
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

void tegra_usb_hsic_host_unregister(struct platform_device **platdev)
{
	struct platform_device *pdev = *platdev;

	if (pdev && &pdev->dev) {
		platform_device_unregister(pdev);
		*platdev = NULL;
	} else
		pr_err("%s: no platform device\n", __func__);
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

static void grouper_usb_init(void)
{
	pr_info("%s\n", __func__);
	/* OTG should be the first to be registered */
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	/* Setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

	if (grouper_get_project_id() == GROUPER_PROJECT_NAKASI_3G) {
		pr_info("%s: NAKASI_3G - projectId=%d - pcbaRev=%d \n", __func__,
				grouper_get_project_id(),
				grouper_query_pcba_revision());
		switch (grouper_query_pcba_revision()) {
			case TILAPIA_PCBA_SR1:
			case TILAPIA_PCBA_SR2:
			case TILAPIA_PCBA_SR3:
				hsic_enable_gpio = EN_HSIC_GPIO;
				pr_info("%s: NAKASI_3G - hsic_enable_gpio = EN_HSIC_GPIO\n", __func__ );
				break;
			default:
				pr_info("%s: NAKASI_3G - hsic_enable_gpio = TEGRA_GPIO_PU4\n", __func__ );
				hsic_enable_gpio = TEGRA_GPIO_PU4;
		}
		tegra_ehci2_device.dev.platform_data = &tegra_ehci_uhsic_pdata;
		/* ehci2 registration happens in baseband-xmm-power  */
	}
}

static void grouper_modem_init(void)
{
	if (grouper_get_project_id() == GROUPER_PROJECT_NAKASI_3G) {
		pr_info("%s\n", __func__);
		tegra_baseband_power_data.ehci_device =
					&tegra_ehci2_device,
		tegra_baseband_power_data.hsic_register =
					&tegra_usb_hsic_host_register;
		tegra_baseband_power_data.hsic_unregister =
					&tegra_usb_hsic_host_unregister;
		platform_device_register(&tegra_baseband_power_device);
	}
}
#else
static void grouper_usb_init(void) { }
static void grouper_modem_init(void) { }
#endif

static void grouper_dock_charger_init(void)
{
	pr_info("%s\n", __func__);
	if (grouper_get_project_id() == GROUPER_PROJECT_NAKASI_3G)
		dock_charger_pdata.gpio = TEGRA_GPIO_PO5;
	else
		dock_charger_pdata.gpio = TEGRA_GPIO_PU4;
	dock_device.dev.platform_data = &dock_charger_pdata;
	platform_device_register(&dock_device);
}

static void grouper_audio_init(void)
{
	pr_info("%s\n", __func__);
	grouper_audio_pdata.codec_name = "rt5640.4-001c";
	grouper_audio_pdata.codec_dai_name = "rt5640-aif1";
}

void grouper_booting_info(void)
{
	static void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	unsigned int reg;
	#define PMC_RST_STATUS_WDT (1)
	#define PMC_RST_STATUS_SW (3)

	reg = readl(pmc + 0x1b4);
	pr_info("%s: reg=%x\n", __func__, reg);

	if (reg == PMC_RST_STATUS_SW) {
		pr_info("%s: SW reboot\n", __func__);
	} else if (reg == PMC_RST_STATUS_WDT) {
		pr_info("%s: watchdog reboot\n", __func__);
	} else {
		pr_info("%s: normal\n", __func__);
	}
}

static void __init tegra_grouper_init(void)
{
	tegra_clk_init_from_table(grouper_clk_init_table);
	tegra_enable_pinmux();
	tegra_smmu_init();
	tegra_soc_device_init("grouper");
	grouper_pinmux_init_early();
	grouper_misc_init();
	grouper_pinmux_init();
	grouper_misc_reset();
	grouper_booting_info();
	grouper_i2c_init();
	grouper_spi_init();
	grouper_usb_init();
#ifdef CONFIG_TEGRA_EDP_LIMITS
	grouper_edp_init();
#endif
	grouper_uart_init();
	grouper_audio_init();
	platform_add_devices(grouper_devices, ARRAY_SIZE(grouper_devices));
	grouper_dock_charger_init();
	tegra_ram_console_debug_init();
	grouper_sdhci_init();
	grouper_regulator_init();
	grouper_suspend_init();
	grouper_touch_init();
	grouper_modem_init();
	grouper_keys_init();
	grouper_panel_init();
	grouper_sensors_init();
	grouper_setup_bluesleep();
	grouper_pins_state_init();
	grouper_emc_init();
	//tegra_release_bootloader_fb();
#ifdef CONFIG_TEGRA_WDT_RECOVERY
	tegra_wdt_recovery_init();
#endif
	tegra_register_fuse();
}

static void __init tegra_grouper_reserve(void)
{
#ifdef CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM
	/* 800*1280*4*2 = 8192000 bytes */
	tegra_reserve(0, SZ_8M, 0);
#else
	tegra_reserve(SZ_128M, SZ_8M, 0);
#endif
}

MACHINE_START(GROUPER, "grouper")
	.atag_offset	= 0x100,
	.soc 		= &tegra_soc_desc,
	.map_io		= tegra_map_common_io,
	.reserve	= tegra_grouper_reserve,
	.init_early	= tegra30_init_early,
	.init_irq	= tegra_init_irq,
	.handle_irq = gic_handle_irq,
	.timer		= &tegra_timer,
	.init_machine	= tegra_grouper_init,
	.restart	= tegra_assert_system_reset,
MACHINE_END
