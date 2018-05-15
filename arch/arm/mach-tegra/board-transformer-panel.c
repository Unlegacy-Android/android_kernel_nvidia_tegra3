/*
 * arch/arm/mach-tegra/board-transformer-panel.c
 *
 * Copyright (c) 2010-2012, NVIDIA Corporation.
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

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/pwm_backlight.h>
#include <linux/nvhost.h>
#include <linux/nvmap.h>
#include <mach/iomap.h>
#include <mach/dc.h>

#include <mach/board-transformer-misc.h>

#include "board.h"
#include "board-transformer.h"
#include "devices.h"
#include "tegra3_host1x_devices.h"

#define DC_CTRL_MODE	(TEGRA_DC_OUT_ONE_SHOT_MODE | \
			 TEGRA_DC_OUT_ONE_SHOT_LP_MODE)

/* E1506 display board pins */
#define e1506_lcd_te		TEGRA_GPIO_PJ1

/* E1247 reworked for pm269 pins */
#define e1247_pm269_lvds_shutdown	TEGRA_GPIO_PN6

/* common pins( backlight ) for all display boards */
#define cardhu_bl_enb				TEGRA_GPIO_PH2
#define cardhu_hdmi_hpd			TEGRA_GPIO_PN7
#define cardhu_hdmi_enb			TEGRA_GPIO_PP2

#ifdef CONFIG_TEGRA_DC
static struct regulator *cardhu_hdmi_reg = NULL;
static struct regulator *cardhu_hdmi_pll = NULL;
static struct regulator *cardhu_hdmi_vddio = NULL;
#endif

static atomic_t sd_brightness = ATOMIC_INIT(255);

static struct regulator *cardhu_lvds_reg = NULL;
static struct regulator *cardhu_lvds_vdd_bl = NULL;
static struct regulator *cardhu_lvds_vdd_panel = NULL;

extern bool isRecording;
extern int cn_vf_sku;

static tegra_dc_bl_output cardhu_bl_output_measured = {
	0, 4, 4, 4, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 49, 50, 51, 52, 53, 54,
	55, 56, 57, 58, 59, 60, 61, 62,
	63, 64, 65, 66, 67, 68, 69, 70,
	70, 72, 73, 74, 75, 76, 77, 78,
	79, 80, 81, 82, 83, 84, 85, 86,
	87, 88, 89, 90, 91, 92, 93, 94,
	95, 96, 97, 98, 99, 100, 101, 102,
	103, 104, 105, 106, 107, 108, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 124, 125, 126,
	127, 128, 129, 130, 131, 132, 133, 133,
	134, 135, 136, 137, 138, 139, 140, 141,
	142, 143, 144, 145, 146, 147, 148, 148,
	149, 150, 151, 152, 153, 154, 155, 156,
	157, 158, 159, 160, 161, 162, 163, 164,
	165, 166, 167, 168, 169, 170, 171, 172,
	173, 174, 175, 176, 177, 179, 180, 181,
	182, 184, 185, 186, 187, 188, 189, 190,
	191, 192, 193, 194, 195, 196, 197, 198,
	199, 200, 201, 202, 203, 204, 205, 206,
	207, 208, 209, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 255
};

static p_tegra_dc_bl_output bl_output;

static int cardhu_backlight_init(struct device *dev)
{
	bl_output = cardhu_bl_output_measured;

	if (WARN_ON(ARRAY_SIZE(cardhu_bl_output_measured) != 256))
		pr_err("%s: bl_output array does not have 256 elements\n", __func__);

	return 1;
};

static int cardhu_backlight_notify(struct device *unused, int brightness)
{
	int cur_sd_brightness = atomic_read(&sd_brightness);

	/* Set the backlight GPIO pin mode to 'backlight_enable' */
	gpio_set_value(cardhu_bl_enb, !!brightness);
 
	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF201
			&& isRecording) {
		gpio_set_value(cardhu_bl_enb, 1);
	}

	/* SD brightness is a percentage, 8-bit value. */
	brightness = DIV_ROUND_CLOSEST((brightness * cur_sd_brightness), 255);

	/* Apply any backlight response curve */
	if (brightness > 255) {
		pr_info("%s: error: Brightness > 255!\n", __func__);
	} else {
		brightness = bl_output[brightness];
	}

	return brightness;
}

static int cardhu_disp1_check_fb(struct device *dev, struct fb_info *info);

static struct platform_pwm_backlight_data cardhu_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 100,
	.pwm_period_ns	= 4000000,
	.init		= cardhu_backlight_init,
	.notify		= cardhu_backlight_notify,
	/* Only toggle backlight on fb blank notifications for disp1 */
	.check_fb	= cardhu_disp1_check_fb,
};

static struct platform_device cardhu_backlight_device = {
	.name	= "pwm-backlight",
	.id	= -1,
	.dev	= {
		.platform_data = &cardhu_backlight_data,
	},
};

static int cardhu_panel_enable(struct device *dev)
{
	//printk("Check cardhu_panel_enable \n");

	if (cardhu_lvds_vdd_panel == NULL) {
		cardhu_lvds_vdd_panel = regulator_get(dev, "vdd_lcd_panel");
		if (WARN_ON(IS_ERR(cardhu_lvds_vdd_panel)))
			pr_err("%s: couldn't get regulator vdd_lcd_panel: %ld\n",
				__func__, PTR_ERR(cardhu_lvds_vdd_panel));
		else
			regulator_enable(cardhu_lvds_vdd_panel);
	}
	msleep(20);

	return 0;
}

static int cardhu_panel_enable_tf700t(struct device *dev)
{
	int ret;
	//printk("Check cardhu_panel_enable_tf700t \n");

	if (gpio_get_value(TEGRA_GPIO_PI6)==0){	//Panel is Panasonic
		//printk("Check panel is panasonic \n");
		if (cardhu_lvds_vdd_bl == NULL) {
			cardhu_lvds_vdd_bl = regulator_get(dev, "vdd_backlight");
			if (WARN_ON(IS_ERR(cardhu_lvds_vdd_bl)))
				pr_err("%s: couldn't get regulator vdd_backlight: %ld\n",
						__func__, PTR_ERR(cardhu_lvds_vdd_bl));
			else
				regulator_enable(cardhu_lvds_vdd_bl);
		}

		ret = gpio_direction_output(TEGRA_GPIO_PU5, 1);
		if (ret < 0) {
			printk("Check can not pull high TEGRA_GPIO_PU5 \n");
			gpio_free(TEGRA_GPIO_PU5);
			return ret;
		}
	}
	else{								//Panel is hydis
		//printk("Check panel is hydis \n");
		gpio_set_value(TEGRA_GPIO_PH3, 0);
		ret = gpio_direction_output(TEGRA_GPIO_PU5, 0);
		if (ret < 0) {
			printk("Check can not pull low TEGRA_GPIO_PU5 \n");
			gpio_free(TEGRA_GPIO_PU5);
			return ret;
		}
	}
	mdelay(5);

	if (cardhu_lvds_reg == NULL) {
		cardhu_lvds_reg = regulator_get(dev, "vdd_lvds");
		if (WARN_ON(IS_ERR(cardhu_lvds_reg)))
			pr_err("%s: couldn't get regulator vdd_lvds: %ld\n",
					__func__, PTR_ERR(cardhu_lvds_reg));
		else
			regulator_enable(cardhu_lvds_reg);
	}

	if (cardhu_lvds_vdd_panel == NULL) {
		cardhu_lvds_vdd_panel = regulator_get(dev, "vdd_lcd_panel");
		if (WARN_ON(IS_ERR(cardhu_lvds_vdd_panel)))
			pr_err("%s: couldn't get regulator vdd_lcd_panel: %ld\n",
					__func__, PTR_ERR(cardhu_lvds_vdd_panel));
		else
			regulator_enable(cardhu_lvds_vdd_panel);
	}
	msleep(20);

	//printk("Check power on/off for bridge IC \n");
	ret = gpio_direction_output(TEGRA_GPIO_PBB3, 1);
	if (ret < 0) {
		printk("Check can not pull high TEGRA_GPIO_PBB3 \n");
		gpio_free(TEGRA_GPIO_PBB3);
		return ret;
	}

	ret = gpio_direction_output(TEGRA_GPIO_PC6, 1);
	if (ret < 0) {
		printk("Check can not pull high TF700T_1.8V(TEGRA_GPIO_PC6) \n");
		gpio_free(TEGRA_GPIO_PC6);
		return ret;
	}

	mdelay(10);

	ret = gpio_direction_output(TEGRA_GPIO_PX0, 1);
	if (ret < 0) {
		printk("Check can not pull high TF700T_I2C_Switch(TEGRA_GPIO_PX0) \n");
		gpio_free(TEGRA_GPIO_PX0);
		return ret;
	}
	mdelay(10);

	gpio_set_value(e1247_pm269_lvds_shutdown, 1);

	ret = gpio_direction_output(TEGRA_GPIO_PD2, 1);
	if (ret < 0) {
		printk("Check can not pull high TF700T_OSC(TEGRA_GPIO_PD2) \n");
		gpio_free(TEGRA_GPIO_PD2);
		return ret;
	}
	msleep(10);
	return 0;
}

static int cardhu_panel_disable(void)
{
	//printk("Check cardhu_panel_disable \n");

	if(cardhu_lvds_reg) {
		regulator_disable(cardhu_lvds_reg);
		regulator_put(cardhu_lvds_reg);
		cardhu_lvds_reg = NULL;
	}

	if(cardhu_lvds_vdd_panel) {
		regulator_disable(cardhu_lvds_vdd_panel);
		regulator_put(cardhu_lvds_vdd_panel);
		cardhu_lvds_vdd_panel= NULL;
	}

	return 0;
}

static int cardhu_panel_disable_tf700t(void)
{
	//printk("Check cardhu_panel_disable in TF700T\n");

	gpio_set_value(TEGRA_GPIO_PD2, 0);
	gpio_set_value(e1247_pm269_lvds_shutdown, 0);
	gpio_set_value(TEGRA_GPIO_PX0, 0);
	gpio_set_value(TEGRA_GPIO_PC6, 0);
	gpio_set_value(TEGRA_GPIO_PBB3, 0);

	if (gpio_get_value(TEGRA_GPIO_PI6)==0 ){		//panel is panasonic
		msleep(85);
	}
	else {  //panel is hydis
		msleep(10);
	}

	if(cardhu_lvds_vdd_panel) {
		regulator_disable(cardhu_lvds_vdd_panel);
		regulator_put(cardhu_lvds_vdd_panel);
		cardhu_lvds_vdd_panel= NULL;
	}

	gpio_set_value(TEGRA_GPIO_PU5, 0);

	if (cardhu_lvds_vdd_bl) {
		regulator_disable(cardhu_lvds_vdd_bl);
		regulator_put(cardhu_lvds_vdd_bl);
		cardhu_lvds_vdd_bl = NULL;
	}

	return 0;
}

static int cardhu_panel_postpoweron(void)
{
	//tf700t not get involved

	if (cardhu_lvds_reg == NULL) {
		cardhu_lvds_reg = regulator_get(NULL, "vdd_lvds");
		if (WARN_ON(IS_ERR(cardhu_lvds_reg)))
			pr_err("%s: couldn't get regulator vdd_lvds: %ld\n",
					__func__, PTR_ERR(cardhu_lvds_reg));
		else
			regulator_enable(cardhu_lvds_reg);
	}

	gpio_set_value(e1247_pm269_lvds_shutdown, 1);
	msleep(210);

	if (cardhu_lvds_vdd_bl == NULL) {
		cardhu_lvds_vdd_bl = regulator_get(NULL, "vdd_backlight");
		if (WARN_ON(IS_ERR(cardhu_lvds_vdd_bl)))
			pr_err("%s: couldn't get regulator vdd_backlight: %ld\n",
					__func__, PTR_ERR(cardhu_lvds_vdd_bl));
		else
			regulator_enable(cardhu_lvds_vdd_bl);
	}
	msleep(10);
	return 0;
}

static int cardhu_panel_prepoweroff(void)
{
	//tf700t not get involved

	// For TF300T EN_VDD_BL (TEGRA_GPIO_PH3) is always on, no need to control cardhu_lvds_vdd_bl
	// But for TF300TG/TL, EN_VDD_BL is BL_EN, need to control it
	// EE confirms that we can control it in original timing because
	// EN_VDD_BL/LCD_BL_PWM/LCD_BL_EN pull high/low almost the same time
	if(cardhu_lvds_vdd_bl) {
		regulator_disable(cardhu_lvds_vdd_bl);
		regulator_put(cardhu_lvds_vdd_bl);
		cardhu_lvds_vdd_bl = NULL;
	}
	msleep(200);

	gpio_set_value(e1247_pm269_lvds_shutdown, 0);
	msleep(10);
	return 0;
}

#ifdef CONFIG_TEGRA_DC
static int cardhu_hdmi_vddio_enable(struct device *dev)
{
	int ret;
	if (!cardhu_hdmi_vddio) {
		cardhu_hdmi_vddio = regulator_get(dev, "vdd_hdmi_con");
		if (IS_ERR_OR_NULL(cardhu_hdmi_vddio)) {
			ret = PTR_ERR(cardhu_hdmi_vddio);
			pr_err("hdmi: couldn't get regulator vdd_hdmi_con\n");
			cardhu_hdmi_vddio = NULL;
			return ret;
		}
	}
	ret = regulator_enable(cardhu_hdmi_vddio);
	if (ret < 0) {
		pr_err("hdmi: couldn't enable regulator vdd_hdmi_con\n");
		regulator_put(cardhu_hdmi_vddio);
		cardhu_hdmi_vddio = NULL;
		return ret;
	}
	return ret;
}

static int cardhu_hdmi_vddio_disable(void)
{
	if (cardhu_hdmi_vddio) {
		regulator_disable(cardhu_hdmi_vddio);
		regulator_put(cardhu_hdmi_vddio);
		cardhu_hdmi_vddio = NULL;
	}
	return 0;
}

static int cardhu_hdmi_enable(struct device *dev)
{
	int ret;
	if (!cardhu_hdmi_reg) {
		cardhu_hdmi_reg = regulator_get(dev, "avdd_hdmi");
		if (IS_ERR_OR_NULL(cardhu_hdmi_reg)) {
			pr_err("hdmi: couldn't get regulator avdd_hdmi\n");
			cardhu_hdmi_reg = NULL;
			return PTR_ERR(cardhu_hdmi_reg);
		}
	}
	ret = regulator_enable(cardhu_hdmi_reg);
	if (ret < 0) {
		pr_err("hdmi: couldn't enable regulator avdd_hdmi\n");
		return ret;
	}
	if (!cardhu_hdmi_pll) {
		cardhu_hdmi_pll = regulator_get(dev, "avdd_hdmi_pll");
		if (IS_ERR_OR_NULL(cardhu_hdmi_pll)) {
			pr_err("hdmi: couldn't get regulator avdd_hdmi_pll\n");
			cardhu_hdmi_pll = NULL;
			regulator_put(cardhu_hdmi_reg);
			cardhu_hdmi_reg = NULL;
			return PTR_ERR(cardhu_hdmi_pll);
		}
	}
	ret = regulator_enable(cardhu_hdmi_pll);
	if (ret < 0) {
		pr_err("hdmi: couldn't enable regulator avdd_hdmi_pll\n");
		return ret;
	}
	return 0;
}

static int cardhu_hdmi_disable(void)
{
	if (cardhu_hdmi_reg) {
		regulator_disable(cardhu_hdmi_reg);
		regulator_put(cardhu_hdmi_reg);
		cardhu_hdmi_reg = NULL;
	}
	if(cardhu_hdmi_pll) {
		regulator_disable(cardhu_hdmi_pll);
		regulator_put(cardhu_hdmi_pll);
		cardhu_hdmi_pll = NULL;
	}
	return 0;
}

static struct resource cardhu_disp1_resources[] = {
	{
		.name	= "irq",
		.start	= INT_DISPLAY_GENERAL,
		.end	= INT_DISPLAY_GENERAL,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "regs",
		.start	= TEGRA_DISPLAY_BASE,
		.end	= TEGRA_DISPLAY_BASE + TEGRA_DISPLAY_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "fbmem",
		.start	= 0,	/* Filled in by cardhu_panel_init() */
		.end	= 0,	/* Filled in by cardhu_panel_init() */
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "dsi_regs",
		.start	= TEGRA_DSI_BASE,
		.end	= TEGRA_DSI_BASE + TEGRA_DSI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource cardhu_disp2_resources[] = {
	{
		.name	= "irq",
		.start	= INT_DISPLAY_B_GENERAL,
		.end	= INT_DISPLAY_B_GENERAL,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "regs",
		.start	= TEGRA_DISPLAY2_BASE,
		.end	= TEGRA_DISPLAY2_BASE + TEGRA_DISPLAY2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "fbmem",
		.flags	= IORESOURCE_MEM,
		.start	= 0,
		.end	= 0,
	},
	{
		.name	= "hdmi_regs",
		.start	= TEGRA_HDMI_BASE,
		.end	= TEGRA_HDMI_BASE + TEGRA_HDMI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};
#endif /* CONFIG_TEGRA_DC */

static struct tegra_dc_mode panel_19X12_modes[] = {
	{
		.pclk = 154000000,
		.h_ref_to_sync = 11,
		.v_ref_to_sync = 1,
		.h_sync_width = 32,
		.v_sync_width = 6,
		.h_back_porch = 80,
		.v_back_porch = 26,
		.h_active = 1920,
		.v_active = 1200,
		.h_front_porch = 48,
		.v_front_porch = 3,
	},
};

static struct tegra_dc_mode cardhu_panel_modes[] = {
	{
		/* 1280x800@60Hz */
		.pclk = 68000000,
		.h_ref_to_sync = 4,
		.v_ref_to_sync = 2,
		.h_sync_width = 30,
		.v_sync_width = 5,
		.h_back_porch = 18,
		.v_back_porch = 12,
		.h_active = 1280,
		.v_active = 800,
		.h_front_porch = 48,
		.v_front_porch = 3,
	},
};

static struct tegra_dc_sd_settings cardhu_sd_settings = {
	.enable = 1, /* enabled by default. */
	.use_auto_pwm = false,
	.hw_update_delay = 0,
	.bin_width = -1,
	.aggressiveness = 1,
	.phase_in_adjustments = true,
	.use_vid_luma = false,
	/* Default video coefficients */
	.coeff = {5, 9, 2},
	.fc = {0, 0},
	/* Immediate backlight changes */
	.blp = {1024, 255},
	/* Gammas: R: 2.2 G: 2.2 B: 2.2 */
	/* Default BL TF */
	.bltf = {
			{
				{57, 65, 74, 83},
				{93, 103, 114, 126},
				{138, 151, 165, 179},
				{194, 209, 225, 242},
			},
			{
				{58, 66, 75, 84},
				{94, 105, 116, 127},
				{140, 153, 166, 181},
				{196, 211, 227, 244},
			},
			{
				{60, 68, 77, 87},
				{97, 107, 119, 130},
				{143, 156, 170, 184},
				{199, 215, 231, 248},
			},
			{
				{64, 73, 82, 91},
				{102, 113, 124, 137},
				{149, 163, 177, 192},
				{207, 223, 240, 255},
			},
		},
	/* Default LUT */
	.lut = {
			{
				{250, 250, 250},
				{194, 194, 194},
				{149, 149, 149},
				{113, 113, 113},
				{82, 82, 82},
				{56, 56, 56},
				{34, 34, 34},
				{15, 15, 15},
				{0, 0, 0},
			},
			{
				{246, 246, 246},
				{191, 191, 191},
				{147, 147, 147},
				{111, 111, 111},
				{80, 80, 80},
				{55, 55, 55},
				{33, 33, 33},
				{14, 14, 14},
				{0, 0, 0},
			},
			{
				{239, 239, 239},
				{185, 185, 185},
				{142, 142, 142},
				{107, 107, 107},
				{77, 77, 77},
				{52, 52, 52},
				{30, 30, 30},
				{12, 12, 12},
				{0, 0, 0},
			},
			{
				{224, 224, 224},
				{173, 173, 173},
				{133, 133, 133},
				{99, 99, 99},
				{70, 70, 70},
				{46, 46, 46},
				{25, 25, 25},
				{7, 7, 7},
				{0, 0, 0},
			},
		},
	.sd_brightness = &sd_brightness,
	.bl_device_name = "pwm-backlight",
};

#ifdef CONFIG_TEGRA_DC
static struct tegra_fb_data cardhu_hdmi_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 800,
	.bits_per_pixel	= 32,
};

static struct tegra_dc_out cardhu_disp2_out = {
	.type		= TEGRA_DC_OUT_HDMI,
	.flags		= TEGRA_DC_OUT_HOTPLUG_HIGH,
	.parent_clk	= "pll_d2_out0",

	.dcc_bus	= 3,
	.hotplug_gpio	= cardhu_hdmi_hpd,

	.max_pixclock	= KHZ2PICOS(148500),

	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,

	.enable		= cardhu_hdmi_enable,
	.disable	= cardhu_hdmi_disable,

	.postsuspend	= cardhu_hdmi_vddio_disable,
	.hotplug_init	= cardhu_hdmi_vddio_enable,
};

static struct tegra_dc_platform_data cardhu_disp2_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.default_out	= &cardhu_disp2_out,
	.fb		= &cardhu_hdmi_fb_data,
	.emc_clk_rate	= 300000000,
};
#endif

static struct tegra_dc_out cardhu_disp1_out = {
	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings	= &cardhu_sd_settings,
	.parent_clk	= "pll_p",
	.parent_clk_backup = "pll_d_out0",

	.type = TEGRA_DC_OUT_RGB,
	.depth = 18,
	.dither = TEGRA_DC_ERRDIFF_DITHER,
	
	.modes = cardhu_panel_modes,
	.n_modes = ARRAY_SIZE(cardhu_panel_modes),
	
	.height = 127,
	.width = 216,
};

#ifdef CONFIG_TEGRA_DC
static struct tegra_fb_data cardhu_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 800,
	.bits_per_pixel	= 32,
};

static struct tegra_dc_platform_data cardhu_disp1_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.default_out	= &cardhu_disp1_out,
	.emc_clk_rate	= 300000000,
	.fb = &cardhu_fb_data,
};

static struct platform_device cardhu_disp1_device = {
	.name		= "tegradc",
	.id		= 0,
	.resource	= cardhu_disp1_resources,
	.num_resources	= ARRAY_SIZE(cardhu_disp1_resources),
	.dev = {
		.platform_data = &cardhu_disp1_pdata,
	},
};

static int cardhu_disp1_check_fb(struct device *dev, struct fb_info *info)
{
	return info->device == &cardhu_disp1_device.dev;
}

static struct platform_device cardhu_disp2_device = {
	.name		= "tegradc",
	.id		= 1,
	.resource	= cardhu_disp2_resources,
	.num_resources	= ARRAY_SIZE(cardhu_disp2_resources),
	.dev = {
		.platform_data = &cardhu_disp2_pdata,
	},
};
#else
static int cardhu_disp1_check_fb(struct device *dev, struct fb_info *info)
{
	return 0;
}
#endif

#ifdef CONFIG_TEGRA_NVMAP
static struct nvmap_platform_carveout cardhu_carveouts[] = {
	[0] = NVMAP_HEAP_CARVEOUT_IRAM_INIT,
	[1] = {
		.name		= "generic-0",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_GENERIC,
		.base		= 0,	/* Filled in by cardhu_panel_init() */
		.size		= 0,	/* Filled in by cardhu_panel_init() */
		.buddy_size	= SZ_32K,
	},
};

static struct nvmap_platform_data cardhu_nvmap_data = {
	.carveouts	= cardhu_carveouts,
	.nr_carveouts	= ARRAY_SIZE(cardhu_carveouts),
};

static struct platform_device cardhu_nvmap_device = {
	.name	= "tegra-nvmap",
	.id	= -1,
	.dev	= {
		.platform_data = &cardhu_nvmap_data,
	},
};
#endif

static struct platform_device *cardhu_gfx_devices[] __initdata = {
#ifdef CONFIG_TEGRA_NVMAP
	&cardhu_nvmap_device,
#endif
	&tegra_pwfm0_device,
	&cardhu_backlight_device,
};

int __init cardhu_panel_init(void)
{
	int err, ret;
	struct resource __maybe_unused *res;
#ifdef CONFIG_TEGRA_GRHOST
	struct platform_device *phost1x;
#endif

#ifdef CONFIG_TEGRA_NVMAP
	cardhu_carveouts[1].base = tegra_carveout_start;
	cardhu_carveouts[1].size = tegra_carveout_size;
#endif

	/* Enable back light */
	ret = gpio_request(cardhu_bl_enb, "backlight_enb");
	if (!ret) {
		ret = gpio_direction_output(cardhu_bl_enb, 1);
		if (ret < 0) {
			gpio_free(cardhu_bl_enb);
			pr_err("%s: error in setting backlight_enb\n", __func__);
		}
	} else {
		pr_err("%s: error in gpio request for backlight_enb\n", __func__);
	}

	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
		cardhu_disp1_out.dither = TEGRA_DC_ORDERED_DITHER;
		cardhu_disp1_out.enable = cardhu_panel_enable_tf700t;
		cardhu_disp1_out.disable = cardhu_panel_disable_tf700t;
	} else {
		cardhu_disp1_out.enable = cardhu_panel_enable;
		cardhu_disp1_out.postpoweron = cardhu_panel_postpoweron;
		cardhu_disp1_out.prepoweroff = cardhu_panel_prepoweroff;
		cardhu_disp1_out.disable = cardhu_panel_disable;
	}

#ifdef CONFIG_TEGRA_DC
	if ( tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG && cn_vf_sku){
		cardhu_disp1_out.modes->pclk = 83900000;
		cardhu_disp1_out.modes->v_front_porch = 200;
		printk("TF300TG: Set LCD pclk as %d Hz, cn_vf_sku=%d\n", cardhu_disp1_out.modes->pclk, cn_vf_sku);
	}

	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
		printk("Check TF700T setting \n ");
		cardhu_disp1_out.modes = panel_19X12_modes;
		cardhu_disp1_out.n_modes = ARRAY_SIZE(panel_19X12_modes);
		cardhu_disp1_out.parent_clk = "pll_d_out0";
		cardhu_disp1_out.depth = 24;

		cardhu_fb_data.xres = 1920;
		cardhu_fb_data.yres = 1200;

		cardhu_disp2_out.parent_clk = "pll_d2_out0";
		cardhu_hdmi_fb_data.xres = 1920;
		cardhu_hdmi_fb_data.yres = 1200;

		gpio_request(TEGRA_GPIO_PU5, "LDO_EN");
		gpio_request(TEGRA_GPIO_PBB3, "TF700T_1.2V");
		gpio_request(TEGRA_GPIO_PC6, "TF700T_1.8V");
		gpio_request(TEGRA_GPIO_PX0, "TF700T_I2C_Switch");
		gpio_request(TEGRA_GPIO_PD2, "TF700T_OSC");
	}
#endif

	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
		gpio_request(cardhu_hdmi_enb, "hdmi_5v_en");
		gpio_direction_output(cardhu_hdmi_enb, 0);
	} else {
		gpio_request(cardhu_hdmi_enb, "hdmi_5v_en");
		gpio_direction_output(cardhu_hdmi_enb, 1);
	}

	gpio_request(cardhu_hdmi_hpd, "hdmi_hpd");
	gpio_direction_input(cardhu_hdmi_hpd);

#if !(DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	gpio_request(e1506_lcd_te, "lcd_te");
	gpio_direction_input(e1506_lcd_te);
#endif

	err = platform_add_devices(cardhu_gfx_devices,
				ARRAY_SIZE(cardhu_gfx_devices));

#ifdef CONFIG_TEGRA_GRHOST
	phost1x = tegra3_register_host1x_devices();
	if (!phost1x)
		return -EINVAL;
#endif

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	res = platform_get_resource_byname(&cardhu_disp1_device,
					 IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;
#endif

	/* Copy the bootloader fb to the fb. */
	__tegra_move_framebuffer(&cardhu_nvmap_device,
		tegra_fb_start, tegra_bootloader_fb_start,
				min(tegra_fb_size, tegra_bootloader_fb_size));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	if (!err) {
		cardhu_disp1_device.dev.parent = &phost1x->dev;
		err = platform_device_register(&cardhu_disp1_device);
	}

	res = platform_get_resource_byname(&cardhu_disp2_device,
					 IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb2_start;
	res->end = tegra_fb2_start + tegra_fb2_size - 1;

	/*
	 * If the bootloader fb2 is valid, copy it to the fb2, or else
	 * clear fb2 to avoid garbage on dispaly2.
	 */
	if (tegra_bootloader_fb2_size)
		__tegra_move_framebuffer(&cardhu_nvmap_device,
			tegra_fb2_start, tegra_bootloader_fb2_start,
			min(tegra_fb2_size, tegra_bootloader_fb2_size));
	else
		__tegra_clear_framebuffer(&cardhu_nvmap_device,
					  tegra_fb2_start, tegra_fb2_size);

	if (!err) {
		cardhu_disp2_device.dev.parent = &phost1x->dev;
		err = platform_device_register(&cardhu_disp2_device);
	}
#endif

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_NVAVP)
	if (!err) {
		nvavp_device.dev.parent = &phost1x->dev;
		err = platform_device_register(&nvavp_device);
	}
#endif
	return err;
}
