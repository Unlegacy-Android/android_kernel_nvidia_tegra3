/*
 * arch/arm/mach-tegra/board-acer-t30-sdhci.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2011-2012 NVIDIA Corporation.
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

#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>
#include <mach/pinmux.h>
#include <mach/pinmux-tegra30.h>

#include "gpio-names.h"
#include "board.h"
#include "board-acer-t30.h"

#define CARDHU_WLAN_VDD        TEGRA_GPIO_PK7
#define CARDHU_WLAN_RST        TEGRA_GPIO_PP2
#define CARDHU_WLAN_WOW        TEGRA_GPIO_PS2
#define CARDHU_BT_RST TEGRA_GPIO_PU0
#define CARDHU_BT_SHUTDOWN     TEGRA_GPIO_PU6
#define CARDHU_SD_CD TEGRA_GPIO_PS4
#define CARDHU_SD_WP -1
#define PM269_SD_WP -1

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
static int cardhu_wifi_status_register(void (*callback)(int , void *), void *);

static int cardhu_wifi_reset(int on);
static int cardhu_wifi_power(int on);
static int cardhu_wifi_set_carddetect(int val);

static struct wifi_platform_data cardhu_wifi_control = {
	.set_power	= cardhu_wifi_power,
	.set_reset	= cardhu_wifi_reset,
	.set_carddetect	= cardhu_wifi_set_carddetect,
};

static struct resource wifi_resource[] = {
	[0] = {
		.name	= "bcmdhd_wlan_irq",
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE,
	},
};

static struct platform_device cardhu_wifi_device = {
	.name		= "bcmdhd_wlan",
	.id		= 1,
	.num_resources	= 1,
	.resource	= wifi_resource,
	.dev		= {
		.platform_data = &cardhu_wifi_control,
	},
};

static struct platform_device marvell_wifi_device = {
	.name		= "mrvl8797_wlan",
	.id		= 1,
	.num_resources	= 0,
	.dev		= {
		.platform_data = &cardhu_wifi_control,
	},
};

static struct resource sdhci_resource0[] = {
	[0] = {
		.start  = INT_SDMMC1,
		.end    = INT_SDMMC1,
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC1_BASE,
		.end	= TEGRA_SDMMC1_BASE + TEGRA_SDMMC1_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource sdhci_resource2[] = {
	[0] = {
		.start  = INT_SDMMC3,
		.end    = INT_SDMMC3,
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC3_BASE,
		.end	= TEGRA_SDMMC3_BASE + TEGRA_SDMMC3_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource sdhci_resource3[] = {
	[0] = {
		.start  = INT_SDMMC4,
		.end    = INT_SDMMC4,
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC4_BASE,
		.end	= TEGRA_SDMMC4_BASE + TEGRA_SDMMC4_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

#ifdef CONFIG_MMC_EMBEDDED_SDIO
static struct embedded_sdio_data embedded_sdio_data2 = {
	.cccr   = {
		.sdio_vsn       = 2,
		.multi_block    = 1,
		.low_speed      = 0,
		.wide_bus       = 0,
		.high_power     = 1,
		.high_speed     = 1,
	},
	.cis  = {
		.vendor         = 0x02d0,
		.device         = 0x4329,
	},
};
#endif

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data2 = {
	.mmc_data = {
		.register_status_notify	= cardhu_wifi_status_register,
#ifdef CONFIG_MMC_EMBEDDED_SDIO
		.embedded_sdio = &embedded_sdio_data2,
#endif
		.built_in = 1,
		.ocr_mask = MMC_OCR_1V8_MASK,
	},
#ifndef CONFIG_MMC_EMBEDDED_SDIO
	.pm_flags = MMC_PM_KEEP_POWER,
#endif
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 41000000,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data0 = {
	.cd_gpio = CARDHU_SD_CD,
	.wp_gpio = CARDHU_SD_WP,
	.power_gpio = -1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 41000000,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data3 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.is_8bit = 1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 41000000,
	.mmc_data = {
		.built_in = 1,
	},
};

static struct platform_device tegra_sdhci_device0 = {
	.name		= "sdhci-tegra",
	.id		= 0,
	.resource	= sdhci_resource0,
	.num_resources	= ARRAY_SIZE(sdhci_resource0),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data0,
	},
};

static struct platform_device tegra_sdhci_device2 = {
	.name		= "sdhci-tegra",
	.id		= 2,
	.resource	= sdhci_resource2,
	.num_resources	= ARRAY_SIZE(sdhci_resource2),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data2,
	},
};

static struct platform_device tegra_sdhci_device3 = {
	.name		= "sdhci-tegra",
	.id		= 3,
	.resource	= sdhci_resource3,
	.num_resources	= ARRAY_SIZE(sdhci_resource3),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data3,
	},
};

static int cardhu_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int cardhu_wifi_set_carddetect(int val)
{
	pr_debug("%s: %d\n", __func__, val);
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

static void set_pin_pupd_input(int pin , int pupd , int input)
{
	int err;

	err = tegra_pinmux_set_pullupdown(pin , pupd);
	if (err < 0)
		printk(KERN_ERR "%s: can't set pin %d pullupdown to %d\n", __func__, pin , pupd);

	err = tegra_pinmux_set_io(pin , input);
	if (err < 0)
		printk(KERN_ERR "%s: can't set pin %d e_input to %d\n", __func__, pin , input);
}

static int wifi_sdio_gpio[] = {
	TEGRA_GPIO_PA6,
	TEGRA_GPIO_PA7,
	TEGRA_GPIO_PB7,
	TEGRA_GPIO_PB6,
	TEGRA_GPIO_PB5,
	TEGRA_GPIO_PB4,
};

static int enable_wifi_sdio_func(void)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(wifi_sdio_gpio); i++) {
		gpio_free(wifi_sdio_gpio[i]);
	}

	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_CLK , TEGRA_PUPD_NORMAL , TEGRA_PIN_INPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_CMD , TEGRA_PUPD_PULL_UP, TEGRA_PIN_INPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT3 , TEGRA_PUPD_PULL_UP , TEGRA_PIN_INPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT2 , TEGRA_PUPD_PULL_UP , TEGRA_PIN_INPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT1 , TEGRA_PUPD_PULL_UP , TEGRA_PIN_INPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT0 , TEGRA_PUPD_PULL_UP , TEGRA_PIN_INPUT);

	return 0;
}

static int disable_wifi_sdio_func(void)
{
	unsigned int rc = 0;
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(wifi_sdio_gpio); i++) {
		rc = gpio_request(wifi_sdio_gpio[i], NULL);
		if (rc) {
			printk(KERN_INFO "%s, request gpio %d failed !!!\n", __func__, wifi_sdio_gpio[i]);
			return rc;
		}

		rc = gpio_direction_output(wifi_sdio_gpio[i], 0);
		if (rc) {
			printk(KERN_INFO "%s, direction gpio %d failed !!!\n", __func__, wifi_sdio_gpio[i]);
			return rc;
		}
	}

	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_CLK , TEGRA_PUPD_NORMAL , TEGRA_PIN_OUTPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_CMD , TEGRA_PUPD_NORMAL , TEGRA_PIN_OUTPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT3 , TEGRA_PUPD_NORMAL , TEGRA_PIN_OUTPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT2 , TEGRA_PUPD_NORMAL , TEGRA_PIN_OUTPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT1 , TEGRA_PUPD_NORMAL , TEGRA_PIN_OUTPUT);
	set_pin_pupd_input(TEGRA_PINGROUP_SDMMC3_DAT0 , TEGRA_PUPD_NORMAL , TEGRA_PIN_OUTPUT);
	return 0;
}

static int cardhu_wifi_power(int on)
{
	struct clk *bcm4329_32k_clk;

	pr_debug("%s: %d\n", __func__, on);

	bcm4329_32k_clk = clk_get(NULL, "bcm4329_32k_clk");
	if (IS_ERR(bcm4329_32k_clk))
		pr_err("%s: failed to get bcm4329_32k_clk\n", __func__);

	if (on)
	    gpio_direction_input(CARDHU_WLAN_WOW);
	else
	    gpio_direction_output(CARDHU_WLAN_WOW, 0);

	/* Set VDD high at first before turning on*/
	if (on) {
		if (!IS_ERR(bcm4329_32k_clk))
			clk_enable(bcm4329_32k_clk);
		gpio_direction_output(CARDHU_BT_RST, 1);
		enable_wifi_sdio_func();
		if (!gpio_get_value(CARDHU_WLAN_VDD)) {
			gpio_set_value(CARDHU_WLAN_VDD, 1);
		}
	}
	mdelay(100);
	gpio_set_value(CARDHU_WLAN_RST, on);
	mdelay(200);

	/*
	 * When BT and Wi-Fi turn off at the same time, the last one must do the VDD off action.
	 * So BT/WI-FI must check the other's status in order to set VDD low at last.
	 */
	if (!on) {
		if (!IS_ERR(bcm4329_32k_clk))
			clk_disable(bcm4329_32k_clk);
		if (!gpio_get_value(CARDHU_BT_SHUTDOWN)) {
			gpio_direction_output(CARDHU_BT_RST, 0);
			gpio_set_value(CARDHU_WLAN_VDD, 0);
		}
		disable_wifi_sdio_func();
	}

	return 0;
}

static int cardhu_wifi_reset(int on)
{
	pr_debug("%s: do nothing\n", __func__);
	return 0;
}

static int __init cardhu_wifi_init(void)
{
	int rc;
	int commchip_id = tegra_get_commchip_id();

	rc = gpio_request(CARDHU_WLAN_VDD, "wlan_vdd");
	if (rc)
		pr_err("WLAN_VDD gpio request failed:%d\n", rc);
	rc = gpio_request(CARDHU_WLAN_RST, "wlan_rst");
	if (rc)
		pr_err("WLAN_RST gpio request failed:%d\n", rc);
	rc = gpio_request(CARDHU_WLAN_WOW, "bcmsdh_sdmmc");
	if (rc)
		pr_err("WLAN_WOW gpio request failed:%d\n", rc);

	rc = gpio_direction_output(CARDHU_WLAN_VDD, 0);
	if (rc)
		pr_err("WLAN_VDD gpio direction configuration failed:%d\n", rc);
	rc = gpio_direction_output(CARDHU_WLAN_RST, 0);
	if (rc)
		pr_err("WLAN_RST gpio direction configuration failed:%d\n", rc);
	rc = gpio_direction_input(CARDHU_WLAN_WOW);
	if (rc)
		pr_err("WLAN_WOW gpio direction configuration failed:%d\n", rc);

	if (commchip_id == COMMCHIP_MARVELL_SD8797)
		platform_device_register(&marvell_wifi_device);
	else {
		cardhu_wifi_device.resource[0].start = gpio_to_irq(CARDHU_WLAN_WOW);
		cardhu_wifi_device.resource[0].end = gpio_to_irq(CARDHU_WLAN_WOW);
		clk_add_alias("bcm4329_32k_clk", NULL, "blink", NULL);
		platform_device_register(&cardhu_wifi_device);
	}

	disable_wifi_sdio_func();
	return 0;
}

#ifdef CONFIG_TEGRA_PREPOWER_WIFI
static int __init cardhu_wifi_prepower(void)
{
	if (!machine_is_cardhu())
		return 0;

	cardhu_wifi_power(1);

	return 0;
}

subsys_initcall_sync(cardhu_wifi_prepower);
#endif

int __init cardhu_sdhci_init(void)
{
	struct board_info board_info;
	tegra_get_board_info(&board_info);
	if ((board_info.board_id == BOARD_PM269) ||
		(board_info.board_id == BOARD_E1257) ||
		(board_info.board_id == BOARD_PM305) ||
		(board_info.board_id == BOARD_PM311)) {
			tegra_sdhci_platform_data0.wp_gpio = PM269_SD_WP;
			tegra_sdhci_platform_data2.max_clk_limit = 12000000;
	}

	platform_device_register(&tegra_sdhci_device3);
	platform_device_register(&tegra_sdhci_device2);
	platform_device_register(&tegra_sdhci_device0);

	cardhu_wifi_init();
	return 0;
}
