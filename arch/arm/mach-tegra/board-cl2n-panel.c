/*
 * arch/arm/mach-tegra/board-kai-panel.c
 *
 * Copyright (c) 2012-2013, NVIDIA Corporation.
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

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/resource.h>
#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <linux/pwm_backlight.h>
#include <asm/atomic.h>
#include <linux/nvhost.h>
#include <linux/nvmap.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/dc.h>
#include <mach/fb.h>
#include <mach/gpio-tegra.h>

#include <mach/board-cl2n-misc.h>

#include <linux/clk.h>

#include "board.h"
#include "board-cl2n.h"
#include "devices.h"
#include "gpio-names.h"
#include "tegra3_host1x_devices.h"

/* kai default display board pins */
#define kai_lvds_avdd_en		TEGRA_GPIO_PH6 /* 13.LCD_AVDD_EN */
#define kai_lvds_stdby			TEGRA_GPIO_PH2 /* 10.STBY */
#define kai_lvds_rst			TEGRA_GPIO_PG7 /* 11.RESET* */
#define kai_lvds_shutdown		TEGRA_GPIO_PN6 /* 0.Enable (LVDS_SHTDN*) (LO:OFF, HI:ON) */
#define kai_lvds_rs			TEGRA_GPIO_PV6 /* 12.LVDS swing mode, 0=200mV, 1=350mV */
#define kai_lvds_lr			TEGRA_GPIO_PG1 /* 9.LCD_LR */

#define kai_vdd_lvds_enb		TEGRA_GPIO_PW1 /* 3.EN_VDD_PNL */
#define kai_lvds_mode0			TEGRA_GPIO_PG2 /* 5.LCD_MODE0 */
#define kai_lvds_mode1			TEGRA_GPIO_PG3 /* 6.LCD_MODE1 */
#define kai_lvds_bpp			TEGRA_GPIO_PG6 /* 7.BPP (Low:24bpp, High:18bpp) */
#define kai_lvds_ud			TEGRA_GPIO_PG0 /* 8.LCD_UD */
#define kai_lvds_stdby_evt		TEGRA_GPIO_PG5 /* 10.STBY */

/* kai A00 display board pins */
#define kai_lvds_rs_a00		TEGRA_GPIO_PH1

/* common pins( backlight ) for all display boards */
#define kai_bl_enb			TEGRA_GPIO_PH3 /* 1.EN_VDD_BL1 */
#define kai_bl_pwm			TEGRA_GPIO_PH0 /* 4.LCD1_BL_PWM */
#define kai_hdmi_hpd			TEGRA_GPIO_PN7

#ifdef CONFIG_TEGRA_DC
static struct regulator *kai_hdmi_reg;
static struct regulator *kai_hdmi_pll;
static struct regulator *kai_hdmi_vddio;
#endif

static atomic_t sd_brightness = ATOMIC_INIT(255);

static struct regulator *kai_lvds_reg;
static struct regulator *kai_lvds_vdd_panel;

/*20120515, JimmySu add board ID*/
static int panel_board_id =0x7;

static tegra_dc_bl_output kai_dvt_bl_output_measured = {
	0, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 20, 21,
	22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 32, 34, 34, 36, 36,
	38, 39, 40, 40, 41, 42, 42, 43,
	44, 44, 45, 46, 46, 47, 48, 48,
	49, 50, 50, 51, 52, 53, 54, 54,
	55, 56, 57, 58, 58, 59, 60, 61,
	62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, 72, 73, 74, 75, 76,
	76, 77, 78, 79, 80, 81, 82, 83,
	85, 86, 87, 89, 90, 91, 92, 92,
	93, 94, 95, 96, 96, 97, 98, 99,
	100, 100, 101, 102, 103, 104, 104, 105,
	106, 107, 108, 108, 109, 110, 112, 114,
	116, 118, 120, 121, 122, 123, 124, 125,
	126, 127, 128, 129, 130, 131, 132, 133,
	134, 135, 136, 137, 138, 139, 140, 141,
	142, 143, 144, 145, 146, 147, 148, 149,
	150, 151, 151, 152, 153, 153, 154, 155,
	155, 156, 157, 157, 158, 159, 159, 160,
	162, 164, 166, 168, 170, 172, 174, 176,
	178, 180, 181, 181, 182, 183, 183, 184,
	185, 185, 186, 187, 187, 188, 189, 189,
	190, 191, 192, 193, 194, 195, 196, 197,
	198, 199, 200, 201, 201, 202, 203, 203,
	204, 205, 205, 206, 207, 207, 208, 209,
	209, 210, 211, 211, 212, 212, 213, 213,
	214, 215, 215, 216, 216, 217, 217, 218,
	219, 219, 220, 222, 226, 230, 232, 234,
	236, 238, 240, 244, 248, 251, 253, 255
};

static tegra_dc_bl_output kai_mppvt_bl_output_measured = {
	0, 6, 6, 6, 6, 6, 6, 7,
	7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 20, 21,
	22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 32, 34, 34, 36, 36,
	38, 39, 40, 40, 41, 42, 42, 43,
	44, 44, 45, 46, 46, 47, 48, 48,
	49, 50, 50, 51, 52, 53, 54, 54,
	55, 56, 57, 58, 58, 59, 60, 61,
	62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, 72, 73, 74, 75, 76,
	76, 77, 78, 79, 80, 81, 82, 83,
	85, 86, 87, 89, 90, 91, 92, 92,
	93, 94, 95, 96, 96, 97, 98, 99,
	100, 100, 101, 102, 103, 104, 104, 105,
	106, 107, 108, 108, 109, 110, 112, 114,
	116, 118, 120, 121, 122, 123, 124, 125,
	126, 127, 128, 129, 130, 131, 132, 133,
	134, 135, 136, 137, 138, 139, 140, 141,
	142, 143, 144, 145, 146, 147, 148, 149,
	150, 151, 151, 152, 153, 153, 154, 155,
	155, 156, 157, 157, 158, 159, 159, 160,
	162, 164, 166, 168, 170, 172, 174, 176,
	178, 180, 181, 181, 182, 183, 183, 184,
	185, 185, 186, 187, 187, 188, 189, 189,
	190, 191, 192, 193, 194, 195, 196, 197,
	198, 199, 200, 201, 201, 202, 203, 203,
	204, 205, 205, 206, 207, 207, 208, 209,
	209, 210, 211, 211, 212, 212, 213, 213,
	214, 215, 215, 216, 216, 217, 217, 218,
	219, 219, 220, 222, 226, 230, 232, 234,
	236, 238, 240, 244, 248, 251, 253, 255
};

static p_tegra_dc_bl_output bl_output;

static int kai_backlight_init(struct device *dev)
{
	if (panel_board_id == CL2N_BOARD_VER_B00){
		//printk("Detected DVT board\n");
		bl_output = kai_dvt_bl_output_measured;
		if (WARN_ON(ARRAY_SIZE(kai_dvt_bl_output_measured) != 256))
			pr_err("bl_output array does not have 256 elements\n");
	} else {
		//printk("Detected MP/PVT board (or something else entirely)\n");
		bl_output = kai_mppvt_bl_output_measured;
		if (WARN_ON(ARRAY_SIZE(kai_mppvt_bl_output_measured) != 256))
			pr_err("bl_output array does not have 256 elements\n");
	}

	return 1;
};

static int kai_backlight_notify(struct device *unused, int brightness)
{
	int cur_sd_brightness = atomic_read(&sd_brightness);

	/* SD brightness is a percentage, 8-bit value. */
	brightness = (brightness * cur_sd_brightness) / 255;

	/* Apply any backlight response curve */
	if (brightness > 255)
		pr_info("Error: Brightness > 255!\n");
	else
		brightness = bl_output[brightness];

	return brightness;
}

static int kai_disp1_check_fb(struct device *dev, struct fb_info *info);

static struct platform_pwm_backlight_data kai_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 100/*224*/,
	.pwm_period_ns	= 100000, /* is 1000000 on cardhu */
	.init		= kai_backlight_init,
	.notify		= kai_backlight_notify,
	/* Only toggle backlight on fb blank notifications for disp1 */
	.check_fb	= kai_disp1_check_fb,
};

static struct platform_device kai_backlight_device = {
	.name	= "pwm-backlight",
	.id	= -1,
	.dev	= {
		.platform_data = &kai_backlight_data,
	},
};

static int kai_panel_postpoweron(void)
{
	if (kai_lvds_reg == NULL) {
		kai_lvds_reg = regulator_get(NULL, "vdd_lvds");
		if (WARN_ON(IS_ERR(kai_lvds_reg)))
			pr_err("%s: couldn't get regulator vdd_lvds: %ld\n",
			       __func__, PTR_ERR(kai_lvds_reg));
		else
			regulator_enable(kai_lvds_reg);
	}

	mdelay(5);

	gpio_set_value(kai_lvds_avdd_en, 1);
	mdelay(5);

	if (panel_board_id == CL2N_BOARD_VER_A00){
		gpio_set_value(kai_lvds_stdby_evt, 1);
	}else{
	gpio_set_value(kai_lvds_stdby, 1);
	}

	gpio_set_value(kai_lvds_rst, 1);
	gpio_set_value(kai_lvds_shutdown, 1);
	gpio_set_value(kai_lvds_lr, 1);

	mdelay(10);

	return 0;
}

static int kai_panel_enable(struct device *dev)
{
	if (kai_lvds_vdd_panel == NULL) {
		kai_lvds_vdd_panel = regulator_get(dev, "vdd_lcd_panel");
		if (WARN_ON(IS_ERR(kai_lvds_vdd_panel)))
			pr_err("%s: couldn't get regulator vdd_lcd_panel: %ld\n",
			       __func__, PTR_ERR(kai_lvds_vdd_panel));
		else
			regulator_enable(kai_lvds_vdd_panel);
	}

	return 0;
}

static int kai_panel_disable(void)
{
	regulator_disable(kai_lvds_vdd_panel);
	regulator_put(kai_lvds_vdd_panel);
	kai_lvds_vdd_panel = NULL;

	return 0;
}

static int kai_panel_prepoweroff(void)
{
	gpio_set_value(kai_lvds_lr, 0);
	gpio_set_value(kai_lvds_shutdown, 0);
	gpio_set_value(kai_lvds_rst, 0);

	if (panel_board_id == CL2N_BOARD_VER_A00){
	gpio_set_value(kai_lvds_stdby_evt, 0);
	}else{
	gpio_set_value(kai_lvds_stdby, 0);
	}
	mdelay(5);

	gpio_set_value(kai_lvds_avdd_en, 0);
	mdelay(5);

	regulator_disable(kai_lvds_reg);
	regulator_put(kai_lvds_reg);
	kai_lvds_reg = NULL;

	return 0;
}

#ifdef CONFIG_TEGRA_DC
static int kai_hdmi_vddio_enable(struct device *dev)
{
	int ret;
	if (!kai_hdmi_vddio) {
		kai_hdmi_vddio = regulator_get(dev, "vdd_hdmi_con");
		if (IS_ERR_OR_NULL(kai_hdmi_vddio)) {
			ret = PTR_ERR(kai_hdmi_vddio);
			pr_err("hdmi: couldn't get regulator vdd_hdmi_con\n");
			kai_hdmi_vddio = NULL;
			return ret;
		}
	}
	ret = regulator_enable(kai_hdmi_vddio);
	if (ret < 0) {
		pr_err("hdmi: couldn't enable regulator vdd_hdmi_con\n");
		regulator_put(kai_hdmi_vddio);
		kai_hdmi_vddio = NULL;
		return ret;
	}
	return ret;
}

static int kai_hdmi_vddio_disable(void)
{
	if (kai_hdmi_vddio) {
		regulator_disable(kai_hdmi_vddio);
		regulator_put(kai_hdmi_vddio);
		kai_hdmi_vddio = NULL;
	}
	return 0;
}

static int kai_hdmi_enable(struct device *dev)
{
	int ret;
	if (!kai_hdmi_reg) {
		kai_hdmi_reg = regulator_get(dev, "avdd_hdmi");
		if (IS_ERR_OR_NULL(kai_hdmi_reg)) {
			pr_err("hdmi: couldn't get regulator avdd_hdmi\n");
			kai_hdmi_reg = NULL;
			return PTR_ERR(kai_hdmi_reg);
		}
	}
	ret = regulator_enable(kai_hdmi_reg);
	if (ret < 0) {
		pr_err("hdmi: couldn't enable regulator avdd_hdmi\n");
		return ret;
	}
	if (!kai_hdmi_pll) {
		kai_hdmi_pll = regulator_get(dev, "avdd_hdmi_pll");
		if (IS_ERR_OR_NULL(kai_hdmi_pll)) {
			pr_err("hdmi: couldn't get regulator avdd_hdmi_pll\n");
			kai_hdmi_pll = NULL;
			regulator_put(kai_hdmi_reg);
			kai_hdmi_reg = NULL;
			return PTR_ERR(kai_hdmi_pll);
		}
	}
	ret = regulator_enable(kai_hdmi_pll);
	if (ret < 0) {
		pr_err("hdmi: couldn't enable regulator avdd_hdmi_pll\n");
		return ret;
	}
	return 0;
}

static int kai_hdmi_disable(void)
{
	regulator_disable(kai_hdmi_reg);
	regulator_put(kai_hdmi_reg);
	kai_hdmi_reg = NULL;

	regulator_disable(kai_hdmi_pll);
	regulator_put(kai_hdmi_pll);
	kai_hdmi_pll = NULL;
	return 0;
}

static struct resource kai_disp1_resources[] = {
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
		.start	= 0,	/* Filled in by kai_panel_init() */
		.end	= 0,	/* Filled in by kai_panel_init() */
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource kai_disp2_resources[] = {
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
#endif

/*
 * how to determine pclk
 * h_total =
 * Horiz_BackPorch + Horiz_SyncWidth + Horiz_DispActive + Horiz_FrontPorch;
 *
 * v_total =
 * Vert_BackPorch + Vert_SyncWidth + Vert_DispActive + Vert_FrontPorch;
 * panel_freq = ( h_total * v_total * refresh_freq );
 */

static struct tegra_dc_mode kai_panel_modes[] = {
	{
		/* 1280x800@60Hz */
		//.pclk = 66600000,
		.pclk = 72500000, /* h_total = 10 + 10 + 1280 + 140 = 1440; v_total = 15 + 8 + 800 + 15 = 838 ; 1440 * 838 * 60 =  72403200*/
		.h_ref_to_sync = 11,
		.v_ref_to_sync = 1,
		.h_sync_width = 10,
		.v_sync_width = 8,
		.h_back_porch = 10,
		.v_back_porch = 15,
		.h_active = 1280,
		.v_active = 800,
		.h_front_porch = 140,
		.v_front_porch = 15,
	},
};

static struct tegra_dc_sd_settings kai_sd_settings = {
	.enable = 1, /* enabled by default. */
	.use_auto_pwm = false,
	.hw_update_delay = 0,
	.bin_width = -1,
	.aggressiveness = 1,
	.panel_min_brightness = 0,
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
static struct tegra_fb_data kai_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 800,
#ifdef CONFIG_TEGRA_DC_USE_HW_BPP
	.bits_per_pixel = -1,
#else
	.bits_per_pixel	= 32,
#endif
	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_fb_data kai_hdmi_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 800,
#ifdef CONFIG_TEGRA_DC_USE_HW_BPP
	.bits_per_pixel = -1,
#else
	.bits_per_pixel	= 32,
#endif
	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_dc_out kai_disp2_out = {
	.type		= TEGRA_DC_OUT_HDMI,
	.flags		= TEGRA_DC_OUT_HOTPLUG_HIGH,

	.dcc_bus	= 3,
	.hotplug_gpio	= kai_hdmi_hpd,

	.max_pixclock	= KHZ2PICOS(148500),

	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,

	.enable		= kai_hdmi_enable,
	.disable	= kai_hdmi_disable,

	.postsuspend	= kai_hdmi_vddio_disable,
	.hotplug_init	= kai_hdmi_vddio_enable,
};

static struct tegra_dc_platform_data kai_disp2_pdata = {
	.flags		= 0,
	.default_out	= &kai_disp2_out,
	.fb		= &kai_hdmi_fb_data,
	.emc_clk_rate	= 300000000,
};
#endif

static struct tegra_dc_out kai_disp1_out = {
	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings	= &kai_sd_settings,
	.parent_clk	= "pll_p",
	.parent_clk_backup = "pll_d2_out0",

	.type		= TEGRA_DC_OUT_RGB,
	.depth		= 18,
	.dither		= TEGRA_DC_ORDERED_DITHER,

	.modes		= kai_panel_modes,
	.n_modes	= ARRAY_SIZE(kai_panel_modes),

	.enable		= kai_panel_enable,
	.postpoweron	= kai_panel_postpoweron,
	.prepoweroff	= kai_panel_prepoweroff,
	.disable	= kai_panel_disable,
};

#ifdef CONFIG_TEGRA_DC
static struct tegra_dc_platform_data kai_disp1_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.default_out	= &kai_disp1_out,
	.emc_clk_rate	= 300000000,
	.fb		= &kai_fb_data,
};

static struct platform_device kai_disp1_device = {
	.name		= "tegradc",
	.id		= 0,
	.resource	= kai_disp1_resources,
	.num_resources	= ARRAY_SIZE(kai_disp1_resources),
	.dev = {
		.platform_data = &kai_disp1_pdata,
	},
};

static int kai_disp1_check_fb(struct device *dev, struct fb_info *info)
{
	return info->device == &kai_disp1_device.dev;
}

static struct platform_device kai_disp2_device = {
	.name		= "tegradc",
	.id		= 1,
	.resource	= kai_disp2_resources,
	.num_resources	= ARRAY_SIZE(kai_disp2_resources),
	.dev = {
		.platform_data = &kai_disp2_pdata,
	},
};
#else
static int kai_disp1_check_fb(struct device *dev, struct fb_info *info)
{
	return 0;
}
#endif

#if defined(CONFIG_TEGRA_NVMAP)
static struct nvmap_platform_carveout kai_carveouts[] = {
	[0] = NVMAP_HEAP_CARVEOUT_IRAM_INIT,
	[1] = {
		.name		= "generic-0",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_GENERIC,
		.base		= 0,	/* Filled in by kai_panel_init() */
		.size		= 0,	/* Filled in by kai_panel_init() */
		.buddy_size	= SZ_32K,
	},
};

static struct nvmap_platform_data kai_nvmap_data = {
	.carveouts	= kai_carveouts,
	.nr_carveouts	= ARRAY_SIZE(kai_carveouts),
};

static struct platform_device kai_nvmap_device = {
	.name	= "tegra-nvmap",
	.id	= -1,
	.dev	= {
		.platform_data = &kai_nvmap_data,
	},
};
#endif

static struct platform_device *kai_gfx_devices[] __initdata = {
#if defined(CONFIG_TEGRA_NVMAP)
	&kai_nvmap_device,
#endif
	&tegra_pwfm0_device,
	&kai_backlight_device,
};

int __init kai_panel_init(void)
{
	int err;
	struct resource __maybe_unused *res;
	struct board_info board_info;
	struct platform_device *phost1x;

	tegra_get_board_info(&board_info);

        panel_board_id = cl2n_get_board_strap();

	if (panel_board_id == CL2N_BOARD_VER_B00){
		kai_sd_settings.panel_min_brightness = 12;
	}else{
		kai_sd_settings.panel_min_brightness = 6;
	}

#if defined(CONFIG_TEGRA_NVMAP)
	kai_carveouts[1].base = tegra_carveout_start;
	kai_carveouts[1].size = tegra_carveout_size;
#endif
	err = gpio_request(kai_lvds_avdd_en, "lvds_avdd_en");
	if (err < 0) {
		pr_err("%s: gpio_request failed %d\n",
			__func__, err);
		return err;
	}
	err = gpio_direction_output(kai_lvds_avdd_en, 1);
	if (err < 0) {
		pr_err("%s: gpio_direction_output failed %d\n",
			__func__, err);
		gpio_free(kai_lvds_avdd_en);
		return err;
	}
	if (panel_board_id == CL2N_BOARD_VER_A00){
		err = gpio_request(kai_lvds_stdby_evt, "lvds_stdby_evt");
		if (err < 0) {
			pr_err("%s: gpio_request failed %d\n",
				__func__, err);
			return err;
		}
		err = gpio_direction_output(kai_lvds_stdby_evt, 1);
		if (err < 0) {
			pr_err("%s: gpio_direction_output failed %d\n",
				__func__, err);
			gpio_free(kai_lvds_stdby_evt);
			return err;
		}
	} else {
		err = gpio_request(kai_lvds_stdby, "lvds_stdby");
		if (err < 0) {
			pr_err("%s: gpio_request failed %d\n",
				__func__, err);
			return err;
		}
		err = gpio_direction_output(kai_lvds_stdby, 1);
		if (err < 0) {
			pr_err("%s: gpio_direction_output failed %d\n",
				__func__, err);
			gpio_free(kai_lvds_stdby);
			return err;
		}
	}

	err = gpio_request(kai_lvds_rst, "lvds_rst");
	if (err < 0) {
		pr_err("%s: gpio_request failed %d\n",
			__func__, err);
		return err;
	}
	err = gpio_direction_output(kai_lvds_rst, 1);
	if (err < 0) {
		pr_err("%s: gpio_direction_output failed %d\n",
			__func__, err);
		gpio_free(kai_lvds_rst);
		return err;
	}

	err = gpio_request(kai_lvds_rs, "lvds_rs");
	if (err < 0) {
		pr_err("%s: gpio_request failed %d\n",
			__func__, err);
		return err;
	}
	err = gpio_direction_output(kai_lvds_rs, 1);
	if (err < 0) {
		pr_err("%s: gpio_direction_output failed %d\n",
			__func__, err);
		gpio_free(kai_lvds_rs);
		return err;
	}

	err = gpio_request(kai_lvds_lr, "lvds_lr");
	if (err < 0) {
		pr_err("%s: gpio_request failed %d\n",
			__func__, err);
		return err;
	}
	err = gpio_direction_output(kai_lvds_lr, 1);
	if (err < 0) {
		pr_err("%s: gpio_direction_output failed %d\n",
			__func__, err);
		gpio_free(kai_lvds_lr);
		return err;
	}

	err = gpio_request(kai_lvds_shutdown, "lvds_shutdown");
	if (err < 0) {
		pr_err("%s: gpio_request failed %d\n",
			__func__, err);
		return err;
	}
	err = gpio_direction_output(kai_lvds_shutdown, 1);
	if (err < 0) {
		pr_err("%s: gpio_direction_output failed %d\n",
			__func__, err);
		gpio_free(kai_lvds_shutdown);
		return err;
	}

	err = gpio_request(kai_hdmi_hpd, "hdmi_hpd");
	if (err < 0) {
		pr_err("%s: gpio_request failed %d\n",
			__func__, err);
		return err;
	}
	err = gpio_direction_input(kai_hdmi_hpd);
	if (err < 0) {
		pr_err("%s: gpio_direction_input failed %d\n",
			__func__, err);
		gpio_free(kai_hdmi_hpd);
		return err;
	}

	err = platform_add_devices(kai_gfx_devices,
				ARRAY_SIZE(kai_gfx_devices));

#ifdef CONFIG_TEGRA_GRHOST
	phost1x = tegra3_register_host1x_devices();
	if (!phost1x)
		return -EINVAL;
#endif

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	res = platform_get_resource_byname(&kai_disp1_device,
					 IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;
#endif

	/* Copy the bootloader fb to the fb. */
	__tegra_move_framebuffer(&kai_nvmap_device,
		tegra_fb_start, tegra_bootloader_fb_start,
				min(tegra_fb_size, tegra_bootloader_fb_size));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	if (!err) {
		kai_disp1_device.dev.parent = &phost1x->dev;
		err = platform_device_register(&kai_disp1_device);
	}

	res = platform_get_resource_byname(&kai_disp2_device,
					 IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb2_start;
	res->end = tegra_fb2_start + tegra_fb2_size - 1;
	if (!err) {
		kai_disp2_device.dev.parent = &phost1x->dev;
		err = platform_device_register(&kai_disp2_device);
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
