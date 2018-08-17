/*
 * arch/arm/mach-tegra/board-transformer-sdhci.c
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

#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>

#include "gpio-names.h"
#include "board-transformer.h"

#define CARDHU_WLAN_PWR		TEGRA_GPIO_PD4
#define CARDHU_WLAN_RST		TEGRA_GPIO_PD3
#define CARDHU_WLAN_WOW		TEGRA_GPIO_PO4
#define CARDHU_SD_CD		TEGRA_GPIO_PI5

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

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

static int cardhu_wifi_power(int on)
{
	pr_debug("%s: %d\n", __func__, on);

	gpio_set_value(CARDHU_WLAN_PWR, on);
	mdelay(100);
	gpio_set_value(CARDHU_WLAN_RST, on);
	mdelay(200);

	return 0;
}

static int cardhu_wifi_reset(int on)
{
	pr_debug("%s: do nothing\n", __func__);
	return 0;
}

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

static struct platform_device broadcom_wifi_device = {
	.name		= "bcmdhd_wlan",
	.id		= 1,
	.num_resources	= 1,
	.resource	= wifi_resource,
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
		.start  = TEGRA_SDMMC4_BASE,
		.end    = TEGRA_SDMMC4_BASE + TEGRA_SDMMC4_SIZE-1,
		.flags  = IORESOURCE_MEM,
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
		.register_status_notify = cardhu_wifi_status_register,
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
	.max_clk_limit = 12000000,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data0 = {
	.cd_gpio = CARDHU_SD_CD,
	.wp_gpio = -1,
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
	}
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

static int __init cardhu_wifi_init(void)
{
	int rc;

	rc = gpio_request(CARDHU_WLAN_PWR, "wlan_power");
	if (rc)
		pr_err("WLAN_PWR gpio request failed: %d\n", rc);

	rc = gpio_request(CARDHU_WLAN_RST, "wlan_rst");
	if (rc)
		pr_err("WLAN_RST gpio request failed: %d\n", rc);

	rc = gpio_request(CARDHU_WLAN_WOW, "bcmsdh_sdmmc");
	if (rc)
		pr_err("WLAN_WOW gpio request failed: %d\n", rc);

	rc = gpio_direction_output(CARDHU_WLAN_PWR, 0);
	if (rc)
		pr_err("WLAN_PWR gpio direction configuration failed: %d\n", rc);

	rc = gpio_direction_output(CARDHU_WLAN_RST, 0);
	if (rc)
		pr_err("WLAN_RST gpio direction configuration failed: %d\n", rc);

	rc = gpio_direction_input(CARDHU_WLAN_WOW);
	if (rc)
		pr_err("WLAN_WOW gpio direction configuration failed: %d\n", rc);

	wifi_resource[0].start = wifi_resource[0].end =
		gpio_to_irq(TEGRA_GPIO_PO4);
	platform_device_register(&broadcom_wifi_device);

	return 0;
}

#ifdef CONFIG_TEGRA_PREPOWER_WIFI
static int __init cardhu_wifi_prepower(void)
{
	cardhu_wifi_power(1);
	return 0;
}

subsys_initcall_sync(cardhu_wifi_prepower);
#endif

int __init cardhu_sdhci_init(void)
{
	platform_device_register(&tegra_sdhci_device3);
	platform_device_register(&tegra_sdhci_device2);
	platform_device_register(&tegra_sdhci_device0);

	cardhu_wifi_init();
	return 0;
}
