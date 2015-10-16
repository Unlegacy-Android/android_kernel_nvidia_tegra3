/*
 * arch/arm/mach-tegra/board-grouper-panel.c
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

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/pwm_backlight.h>
#include <linux/nvhost.h>
#include <linux/nvmap.h>
#include <mach/iomap.h>
#include <mach/dc.h>

#include "board.h"
#include "board-grouper.h"
#include "devices.h"
#include "tegra3_host1x_devices.h"
#include <mach/board-grouper-misc.h>

/* grouper default display board pins */
#define grouper_lvds_shutdown	TEGRA_GPIO_PN6

static atomic_t sd_brightness = ATOMIC_INIT(255);

static struct regulator *grouper_lvds_reg;
static struct regulator *grouper_lvds_vdd_panel;

static tegra_dc_bl_output grouper_bl_output_measured = {
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

static int grouper_backlight_init(struct device *dev)
{
	bl_output = grouper_bl_output_measured;

	if (WARN_ON(ARRAY_SIZE(grouper_bl_output_measured) != 256))
		pr_err("%s: bl_output array does not have 256 elements\n", __func__);

	return 1;
}

static int grouper_backlight_notify(struct device *unused, int brightness)
{
	int cur_sd_brightness = atomic_read(&sd_brightness);

	/* SD brightness is a percentage, 8-bit value. */
	brightness = DIV_ROUND_CLOSEST((brightness * cur_sd_brightness), 255);

	/* Apply any backlight response curve */
	if (brightness > 255)
		pr_info("%s: error: Brightness > 255!\n", __func__);
	else
		brightness = bl_output[brightness];

	return brightness;
}

static int grouper_disp1_check_fb(struct device *dev, struct fb_info *info);

static struct platform_pwm_backlight_data grouper_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 40,
	.pwm_period_ns	= 50000,
	.init		= grouper_backlight_init,
	.notify		= grouper_backlight_notify,
	/* Only toggle backlight on fb blank notifications for disp1 */
	.check_fb	= grouper_disp1_check_fb,
};

static struct platform_device grouper_backlight_device = {
	.name	= "pwm-backlight",
	.id	= -1,
	.dev	= {
		.platform_data = &grouper_backlight_data,
	},
};

static int grouper_panel_prepoweroff(void)
{
	gpio_set_value(grouper_lvds_shutdown, 0);

	return 0;
}

static int grouper_panel_postpoweron(void)
{
	if (grouper_lvds_reg == NULL) {
		grouper_lvds_reg = regulator_get(NULL, "vdd_lvds");
		if (WARN_ON(IS_ERR(grouper_lvds_reg)))
			pr_err("%s: couldn't get regulator vdd_lvds: %ld\n",
				__func__, PTR_ERR(grouper_lvds_reg));
		else
			regulator_enable(grouper_lvds_reg);
	}

	mdelay(200);

	gpio_set_value(grouper_lvds_shutdown, 1);

	mdelay(50);

	return 0;
}

static int grouper_panel_enable(struct device *dev)
{
	if (grouper_lvds_vdd_panel == NULL) {
		grouper_lvds_vdd_panel = regulator_get(dev, "vdd_lcd_panel");
		if (WARN_ON(IS_ERR(grouper_lvds_vdd_panel)))
			pr_err("%s: couldn't get regulator vdd_lcd_panel: %ld\n",
				__func__, PTR_ERR(grouper_lvds_vdd_panel));
		else
			regulator_enable(grouper_lvds_vdd_panel);
	}

	if (grouper_get_project_id() == GROUPER_PROJECT_BACH) {
		gpio_direction_output(TEGRA_GPIO_PV6, 1);
	}

	return 0;
}

static int grouper_panel_disable(void)
{
	mdelay(5);

	if (grouper_lvds_reg) {
		regulator_disable(grouper_lvds_reg);
		regulator_put(grouper_lvds_reg);
		grouper_lvds_reg = NULL;
	}

	if (grouper_get_project_id() == GROUPER_PROJECT_BACH) {
		gpio_direction_output(TEGRA_GPIO_PV6, 0);
	}

	if (grouper_lvds_vdd_panel) {
		regulator_disable(grouper_lvds_vdd_panel);
		regulator_put(grouper_lvds_vdd_panel);
		grouper_lvds_vdd_panel = NULL;
	}

	return 0;
}

#ifdef CONFIG_TEGRA_DC
static struct resource grouper_disp1_resources[] = {
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
		.start	= 0,	/* Filled in by grouper_panel_init() */
		.end	= 0,	/* Filled in by grouper_panel_init() */
		.flags	= IORESOURCE_MEM,
	},
};
#endif

static struct tegra_dc_mode grouper_panel_modes[] = {
	{
		/* 1280x800@60Hz */
		.pclk = 68000000,
		.h_ref_to_sync = 1,
		.v_ref_to_sync = 1,
		.h_sync_width = 24,
		.v_sync_width = 1,
		.h_back_porch = 32,
		.v_back_porch = 2,
		.h_active = 800,
		.v_active = 1280,
		.h_front_porch = 24,
		.v_front_porch = 5,
	},
};

static struct tegra_dc_sd_settings grouper_sd_settings = {
	.enable = 0, /* disabled by default. */
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
static struct tegra_fb_data grouper_fb_data = {
	.win		= 0,
	.xres		= 800,
	.yres		= 1280,
	.bits_per_pixel	= 32,
};
#endif

static struct tegra_dc_out grouper_disp1_out = {
	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings	= &grouper_sd_settings,
	.parent_clk	= "pll_p",
	.parent_clk_backup = "pll_d2_out0",

	.type		= TEGRA_DC_OUT_RGB,
	.depth		= 18,
	.dither		= TEGRA_DC_ERRDIFF_DITHER,

	.modes		= grouper_panel_modes,
	.n_modes	= ARRAY_SIZE(grouper_panel_modes),

	.enable		= grouper_panel_enable,
	.postpoweron	= grouper_panel_postpoweron,
	.prepoweroff	= grouper_panel_prepoweroff,
	.disable	= grouper_panel_disable,

	.height		= 162,
	.width		= 104,
};

#ifdef CONFIG_TEGRA_DC
static struct tegra_dc_platform_data grouper_disp1_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.default_out	= &grouper_disp1_out,
	.emc_clk_rate	= 300000000,
	.fb		= &grouper_fb_data,
};

static struct platform_device grouper_disp1_device = {
	.name		= "tegradc",
	.id		= 0,
	.resource	= grouper_disp1_resources,
	.num_resources	= ARRAY_SIZE(grouper_disp1_resources),
	.dev = {
		.platform_data = &grouper_disp1_pdata,
	},
};

static int grouper_disp1_check_fb(struct device *dev, struct fb_info *info)
{
	return info->device == &grouper_disp1_device.dev;
}
#else
static int grouper_disp1_check_fb(struct device *dev, struct fb_info *info)
{
	return 0;
}
#endif

#ifdef CONFIG_TEGRA_NVMAP
static struct nvmap_platform_carveout grouper_carveouts[] = {
	[0] = NVMAP_HEAP_CARVEOUT_IRAM_INIT,
	[1] = {
		.name		= "generic-0",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_GENERIC,
		.base		= 0,	/* Filled in by grouper_panel_init() */
		.size		= 0,	/* Filled in by grouper_panel_init() */
		.buddy_size	= SZ_32K,
	},
};

static struct nvmap_platform_data grouper_nvmap_data = {
	.carveouts	= grouper_carveouts,
	.nr_carveouts	= ARRAY_SIZE(grouper_carveouts),
};

static struct platform_device grouper_nvmap_device = {
	.name	= "tegra-nvmap",
	.id	= -1,
	.dev	= {
		.platform_data = &grouper_nvmap_data,
	},
};
#endif

static struct platform_device *grouper_gfx_devices[] __initdata = {
#ifdef CONFIG_TEGRA_NVMAP
	&grouper_nvmap_device,
#endif
	&tegra_pwfm0_device,
	&grouper_backlight_device,
};

int __init grouper_panel_init(void)
{
	int err;
	struct resource __maybe_unused *res;
#ifdef CONFIG_TEGRA_GRHOST
	struct platform_device *phost1x;
#endif


#ifdef CONFIG_TEGRA_NVMAP
	grouper_carveouts[1].base = tegra_carveout_start;
	grouper_carveouts[1].size = tegra_carveout_size;
#endif

	if (grouper_get_project_id() == GROUPER_PROJECT_BACH) {
		grouper_disp1_out.parent_clk = "pll_d_out0";
		grouper_disp1_out.modes->pclk = 81750000;
		grouper_disp1_out.modes->h_sync_width= 64;
		grouper_disp1_out.modes->h_back_porch= 128;
		grouper_disp1_out.modes->h_front_porch = 64;
		pr_info("Bach: Set LCD pclk as %d Hz\n", grouper_disp1_out.modes->pclk);
		gpio_request(TEGRA_GPIO_PV6, "gpio_v6");
	}

	err = platform_add_devices(grouper_gfx_devices,
				ARRAY_SIZE(grouper_gfx_devices));

#ifdef CONFIG_TEGRA_GRHOST
	phost1x = tegra3_register_host1x_devices();
	if (!phost1x)
		return -EINVAL;
#endif

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	res = platform_get_resource_byname(&grouper_disp1_device,
					IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;
#endif

	/* Copy the bootloader fb to the fb. */
	__tegra_move_framebuffer(&grouper_nvmap_device,
		tegra_fb_start, tegra_bootloader_fb_start,
				min(tegra_fb_size, tegra_bootloader_fb_size));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	if (!err) {
		grouper_disp1_device.dev.parent = &phost1x->dev;
		err = platform_device_register(&grouper_disp1_device);
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
