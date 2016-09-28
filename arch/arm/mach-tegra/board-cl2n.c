/*
 * arch/arm/mach-tegra/board-kai.c
 *
 * Copyright (c) 2012-2013, NVIDIA CORPORATION. All rights reserved.
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
#include <linux/skbuff.h>
#include <linux/rfkill-gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/smb349-charger.h>
#include <linux/leds.h>
#include <linux/of_platform.h>
#include <linux/i2c/ft5x06_ts.h>

#include <asm/hardware/gic.h>

#include <mach/clk.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/pinmux-tegra30.h>
#include <mach/iomap.h>
#include <mach/io.h>
#include <mach/io_dpd.h>
#include <mach/i2s.h>
#include <mach/tegra_asoc_pdata.h>
#include <sound/tlv320aic326x.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/usb_phy.h>
#include <mach/gpio-tegra.h>
#include <mach/tegra_fiq_debugger.h>
#include <mach/tegra_wakeup_monitor.h>

#include "board.h"
#include "board-common.h"
#include "clock.h"
#include "board-cl2n.h"
#include "devices.h"
#include "gpio-names.h"
#include "fuse.h"
#include "pm.h"
#include "wdt-recovery.h"
#include "common.h"

static struct rfkill_gpio_platform_data cardhu_bt_rfkill_pdata[] = {
	{
		.name           = "bt_rfkill",
		.shutdown_gpio  = TEGRA_GPIO_PU0,
		.reset_gpio     = TEGRA_GPIO_INVALID,
		.type           = RFKILL_TYPE_BLUETOOTH,
	},
};

static struct platform_device cardhu_bt_rfkill_device = {
	.name = "rfkill_gpio",
	.id             = -1,
	.dev = {
		.platform_data = &cardhu_bt_rfkill_pdata,
	},
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
			.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static struct platform_device cardhu_bluesleep_device = {
	.name		= "bluesleep",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(cardhu_bluesleep_resources),
	.resource	= cardhu_bluesleep_resources,
};

extern void bluesleep_setup_uart_port(struct platform_device *uart_dev);

static noinline void __init cardhu_setup_bluesleep(void)
{
	cardhu_bluesleep_resources[2].start =
		cardhu_bluesleep_resources[2].end =
			gpio_to_irq(TEGRA_GPIO_PU6);
	platform_device_register(&cardhu_bluesleep_device);
	bluesleep_setup_uart_port(&tegra_uartc_device);
}

static __initdata struct tegra_clk_init_table kai_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
	{ "hda",	"pll_p",	108000000,	false},
	{ "hda2codec_2x", "pll_p",	48000000,	false},
	{ "pwm",	"pll_p",	3187500,	false},
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

static struct tegra_i2c_platform_data kai_i2c1_platform_data = {
	.adapter_nr	= 0,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PC4, 0},
	.sda_gpio		= {TEGRA_GPIO_PC5, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data kai_i2c2_platform_data = {
	.adapter_nr	= 1,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.is_clkon_always = true,
	.scl_gpio		= {TEGRA_GPIO_PT5, 0},
	.sda_gpio		= {TEGRA_GPIO_PT6, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data kai_i2c3_platform_data = {
	.adapter_nr	= 2,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB1, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB2, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data kai_i2c4_platform_data = {
	.adapter_nr	= 3,
	.bus_count	= 1,
	.bus_clk_rate	= { 10000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PV4, 0},
	.sda_gpio		= {TEGRA_GPIO_PV5, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data kai_i2c5_platform_data = {
	.adapter_nr	= 4,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PZ6, 0},
	.sda_gpio		= {TEGRA_GPIO_PZ7, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct i2c_board_info kai_i2c4_bq27541_board_info[] = {
	{
	I2C_BOARD_INFO("bq27541", 0x55),
	},
};

static struct regulator_consumer_supply smb349_vbus_supply[] = {
	REGULATOR_SUPPLY("usb_bat_chg", "tegra-udc.0"),
};

static struct regulator_consumer_supply smb349_otg_vbus_supply[] = {
	REGULATOR_SUPPLY("usb_vbus", "tegra-ehci.0"),
};

static struct smb349_charger_platform_data smb349_charger_pdata = {
	.max_charge_current_mA = 2000,
	.charging_term_current_mA = 200,
	.consumer_supplies = smb349_vbus_supply,
	.num_consumer_supplies = ARRAY_SIZE(smb349_vbus_supply),
	.otg_consumer_supplies = smb349_otg_vbus_supply,
	.num_otg_consumer_supplies = ARRAY_SIZE(smb349_otg_vbus_supply),
	.configuration_data = {
		0x6A,/*0x00*/ /*input current*/			//0x00
		0x40,/*taper current*/				//0X01
		0xFF,						//0x02
		0xFF,						//0x03
		0x38,/*recharge current=100mA*/			//0x04
		0x06,/*500mA for other charger*/		//0x05
		0xFF,						//0x06
		0x40,/*min system voltage and termal enable*/	//0x07
		0xFF,						//0x08, doesn't exist
		0xFF,/*OTG active low: 0x20*/			//0x09
		0xFF,						//0x0a
		0x4E,/*0x8E*/ /*temperature monitor:0~50*/	//0x0b
		0x80,						//0x0c
		0x98,/*<-- interrupt mask*/			//0x0d
		0xFF,						//0x0e
		0xFF,						//0x0f
		0x0F,/*low battery threshold:3.58*/		//0x10
	},
};

static struct i2c_board_info kai_i2c4_smb349_board_info[] = {
	{
		I2C_BOARD_INFO("smb349", 0x1B),
		.platform_data = &smb349_charger_pdata,
	},
};

static struct aic326x_pdata kai_aic326x_pdata = {
	.debounce_time_ms = 512,
	.cspin = TEGRA_CODEC_SPI_CS,
};

static struct spi_board_info __initdata aic326x_spi_board_info[] = {
	{
		.modalias = "aic3262-codec",
		.bus_num = 0,
		.chip_select = 1,
		.mode = SPI_MODE_1,
		.max_speed_hz = 4000000,
		.platform_data = &kai_aic326x_pdata,
	},
 };

static void kai_i2c_init(void)
{
	struct board_info board_info;

	tegra_get_board_info(&board_info);

	tegra_i2c_device1.dev.platform_data = &kai_i2c1_platform_data;
	tegra_i2c_device2.dev.platform_data = &kai_i2c2_platform_data;
	tegra_i2c_device3.dev.platform_data = &kai_i2c3_platform_data;
	tegra_i2c_device4.dev.platform_data = &kai_i2c4_platform_data;
	tegra_i2c_device5.dev.platform_data = &kai_i2c5_platform_data;

	platform_device_register(&tegra_i2c_device5);
	platform_device_register(&tegra_i2c_device4);
	platform_device_register(&tegra_i2c_device3);
	platform_device_register(&tegra_i2c_device2);
	platform_device_register(&tegra_i2c_device1);

	i2c_register_board_info(4, kai_i2c4_smb349_board_info,
		ARRAY_SIZE(kai_i2c4_smb349_board_info));

	i2c_register_board_info(4, kai_i2c4_bq27541_board_info,
		ARRAY_SIZE(kai_i2c4_bq27541_board_info));

}

static struct platform_device *kai_uart_devices[] __initdata = {
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

static struct tegra_uart_platform_data kai_uart_pdata;
static struct tegra_uart_platform_data kai_loopback_uart_pdata;

static void __init uart_debug_init(void)
{
	int debug_port_id;

	debug_port_id = uart_console_debug_init(3);
	if (debug_port_id < 0)
		return;
	kai_uart_devices[debug_port_id] = uart_console_debug_device;

	return;
}

static void __init kai_uart_init(void)
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
	kai_uart_pdata.parent_clk_list = uart_parent_clk;
	kai_uart_pdata.parent_clk_count = ARRAY_SIZE(uart_parent_clk);
	kai_loopback_uart_pdata.parent_clk_list = uart_parent_clk;
	kai_loopback_uart_pdata.parent_clk_count =
						ARRAY_SIZE(uart_parent_clk);
	kai_loopback_uart_pdata.is_loopback = true;
	tegra_uarta_device.dev.platform_data = &kai_uart_pdata;
	tegra_uartb_device.dev.platform_data = &kai_uart_pdata;
	tegra_uartc_device.dev.platform_data = &kai_uart_pdata;
	tegra_uartd_device.dev.platform_data = &kai_uart_pdata;
	/* UARTE is used for loopback test purpose */
	tegra_uarte_device.dev.platform_data = &kai_loopback_uart_pdata;

	/* Register low speed only if it is selected */
	if (!is_tegra_debug_uartport_hs())
		uart_debug_init();

	platform_add_devices(kai_uart_devices,
				ARRAY_SIZE(kai_uart_devices));
}

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.id = -1,
};

static struct platform_device *kai_spi_devices[] __initdata = {
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

static struct tegra_spi_platform_data kai_spi1_pdata = {
		.is_dma_based           = true,
		.max_dma_buffer         = (16 * 1024),
		.is_clkon_always        = true,
		.max_rate               = 100000000,
};

static void __init kai_spi_init(void)
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

	kai_spi1_pdata.parent_clk_list = spi_parent_clk;
	kai_spi1_pdata.parent_clk_count = ARRAY_SIZE(spi_parent_clk);
	tegra_spi_device1.dev.platform_data = &kai_spi1_pdata;
	platform_add_devices(kai_spi_devices,
				ARRAY_SIZE(kai_spi_devices));

	/*register TI AIC326x codec on SPI bus*/
	spi_register_board_info(aic326x_spi_board_info, ARRAY_SIZE(aic326x_spi_board_info));
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

static struct tegra_wakeup_monitor_platform_data
					kai_tegra_wakeup_monitor_pdata = {
	.wifi_wakeup_source	= 1,  /* kai's wifi wakeup source */
};

static struct platform_device kai_tegra_wakeup_monitor_device = {
	.name = "tegra_wakeup_monitor",
	.id   = -1,
	.dev  = {
		.platform_data = &kai_tegra_wakeup_monitor_pdata,
	},
};

static struct tegra_asoc_platform_data kai_audio_device_aic326x_platform_data ={
	.gpio_spkr_en = -1,
	.gpio_hp_det = TEGRA_GPIO_HP_DET,
	.gpio_hp_mute = -1,
	.gpio_int_mic_en = -1,
	.gpio_ext_mic_en = -1,
	/*defaults for kai with TI AIC326X codec at linux-3.1*/
	.i2s_param[HIFI_CODEC]  = {
		.audio_port_id  = 1,
		.is_i2s_master  = 1,
		.i2s_mode       = TEGRA_DAIFMT_I2S,
		.sample_size    = 16,
	},
	.i2s_param[BT_SCO]      = {
		.sample_size    = 16,
		.audio_port_id  = 3,
		.is_i2s_master  = 1,
		.i2s_mode       = TEGRA_DAIFMT_DSP_A,
	},
};


static struct platform_device kai_audio_device_aic326x = {
	.name   = "tegra-snd-aic326x",
	.id	= 0,
	.dev	= {
		.platform_data  = &kai_audio_device_aic326x_platform_data,
	},
};

static struct gpio_led kai_led_info[] = {
	{
		.name			= "statled",
		.default_trigger	= "default-on",
		.gpio			= TEGRA_GPIO_STAT_LED,
		.active_low		= 1,
		.retain_state_suspended	= 0,
		.default_state		= LEDS_GPIO_DEFSTATE_OFF,
	},
};

static struct gpio_led_platform_data kai_leds_pdata = {
	.leds		= kai_led_info,
	.num_leds	= ARRAY_SIZE(kai_led_info),
};

static struct platform_device kai_leds_gpio_device = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = &kai_leds_pdata,
	},
};

static struct platform_device *kai_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_rtc_device,
	&tegra_udc_device,
	&tegra_wdt0_device,
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
	&tegra_i2s_device1,
	&tegra_i2s_device3,
	&tegra_i2s_device4,
	&tegra_spdif_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&tegra_pcm_device,
#if defined(CONFIG_TEGRA_WAKEUP_MONITOR)
	&kai_tegra_wakeup_monitor_device,
#endif
	&kai_audio_device_aic326x,
	&kai_leds_gpio_device,
	&tegra_hda_device,
#if defined(CONFIG_CRYPTO_DEV_TEGRA_AES)
	&tegra_aes_device,
#endif
	&cardhu_bt_rfkill_device,
};

static struct ft5x0x_platform_data ft5x06_ts_info = {
	.x_max = 1280,
	.y_max = 800,
	.irqflags = IRQF_TRIGGER_FALLING,
	.reset = TEGRA_GPIO_PH5,
};

static struct i2c_board_info __initdata ft5x06_i2c_info[] = {
	{
		I2C_BOARD_INFO(FT5X0X_NAME, 0x38),
		.platform_data = &ft5x06_ts_info,
	}
};

static int __init touch_init_ft5x06_kai(void)
{
	pr_debug("%s\n", __func__);

	ft5x06_i2c_info[0].irq=gpio_to_irq(TEGRA_GPIO_PH4);

	gpio_request(TEGRA_GPIO_PH4, "ft5x0x-irq");
	gpio_direction_input(TEGRA_GPIO_PH4);
	i2c_register_board_info(1, ft5x06_i2c_info, 1);

	return 0;
}

static int __init kai_touch_init(void)
{
	return touch_init_ft5x06_kai();
}

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
	.op_mode	= TEGRA_USB_OPMODE_HOST,
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

#if CONFIG_USB_SUPPORT
static void kai_usb_init(void)
{
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	/* Setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

	tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
	platform_device_register(&tegra_ehci2_device);
}

static void kai_modem_init(void)
{
	int ret;
	int modem_id = tegra_get_modem_id();

	if (modem_id == TEGRA_BB_TANGO) {
		tegra_ehci2_device.dev.platform_data = &tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	}


	ret = gpio_request(TEGRA_GPIO_W_DISABLE, "w_disable_gpio");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %d\n",
			__func__, TEGRA_GPIO_W_DISABLE);
	else {
		ret = gpio_direction_output(TEGRA_GPIO_W_DISABLE, 1);
		if (ret < 0) {
			pr_err("%s: gpio_direction_output failed %d\n",
				__func__, ret);
			gpio_free(TEGRA_GPIO_W_DISABLE);
		}
	}

	ret = gpio_request(TEGRA_GPIO_MODEM_RSVD1, "Port_V_PIN_0");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %d\n",
			__func__, TEGRA_GPIO_MODEM_RSVD1);
	else {
		ret = gpio_direction_input(TEGRA_GPIO_MODEM_RSVD1);
		if (ret < 0) {
			pr_err("%s: gpio_direction_output failed %d\n",
				__func__, ret);
			gpio_free(TEGRA_GPIO_MODEM_RSVD1);
		}
	}


	ret = gpio_request(TEGRA_GPIO_MODEM_RSVD2, "Port_H_PIN_7");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %d\n",
			__func__, TEGRA_GPIO_MODEM_RSVD2);
	else {
		ret = gpio_direction_output(TEGRA_GPIO_MODEM_RSVD2, 1);
		if (ret < 0) {
			pr_err("%s: gpio_direction_output failed %d\n",
				__func__, ret);
			gpio_free(TEGRA_GPIO_MODEM_RSVD2);
		}
	}

}

#else
static void kai_usb_init(void) { }
static void kai_modem_init(void) { }
#endif

static void __init tegra_kai_init(void)
{
	tegra_clk_init_from_table(kai_clk_init_table);
	tegra_enable_pinmux();
	tegra_smmu_init();
	tegra_soc_device_init("kai");
	kai_pinmux_init();
	kai_i2c_init();
	kai_spi_init();
	kai_usb_init();
#ifdef CONFIG_TEGRA_EDP_LIMITS
	kai_edp_init();
#endif
	kai_uart_init();
	platform_add_devices(kai_devices, ARRAY_SIZE(kai_devices));
	tegra_ram_console_debug_init();
	tegra_io_dpd_init();
	kai_sdhci_init();
	kai_regulator_init();
	kai_suspend_init();
	kai_touch_init();
	kai_keys_init();
	kai_panel_init();
	cardhu_setup_bluesleep();
	kai_sensors_init();
	kai_pins_state_init();
	kai_emc_init();
	//tegra_release_bootloader_fb();
	kai_modem_init();
#ifdef CONFIG_TEGRA_WDT_RECOVERY
	tegra_wdt_recovery_init();
#endif
	tegra_serial_debug_init(TEGRA_UARTD_BASE, INT_WDT_CPU, NULL, -1, -1);
	tegra_register_fuse();
	tegra_vibrator_init();

}

static void __init tegra_kai_dt_init(void)
{
	tegra_kai_init();

#ifdef CONFIG_USE_OF
	of_platform_populate(NULL,
		of_default_bus_match_table, NULL, NULL);
#endif
}

static void __init tegra_kai_reserve(void)
{
#if defined(CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM)
	/* support 1920X1200 with 24bpp */
	tegra_reserve(0, SZ_8M + SZ_1M, SZ_8M + SZ_1M);
#else
	tegra_reserve(SZ_128M, SZ_8M, SZ_8M);
#endif
}

static const char * const kai_dt_board_compat[] = {
	"nvidia,kai",
	NULL
};

MACHINE_START(KAI, "kai")
	.atag_offset	= 0x100,
	.soc		= &tegra_soc_desc,
	.map_io		= tegra_map_common_io,
	.reserve	= tegra_kai_reserve,
	.init_early	= tegra30_init_early,
	.init_irq	= tegra_init_irq,
	.handle_irq	= gic_handle_irq,
	.timer		= &tegra_timer,
	.init_machine	= tegra_kai_dt_init,
	.restart	= tegra_assert_system_reset,
	.dt_compat	= kai_dt_board_compat,
MACHINE_END
