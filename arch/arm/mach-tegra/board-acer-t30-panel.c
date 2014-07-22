#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/resource.h>
#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/pwm_backlight.h>
#include <asm/atomic.h>
#include <linux/nvhost.h>
#include <linux/nvmap.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/dc.h>
#include <mach/fb.h>

#include "board.h"
#include "board-acer-t30.h"
#include "devices.h"
#include "gpio-names.h"
#include "tegra3_host1x_devices.h"

#define LVDS_SHUTDOWN      TEGRA_GPIO_PC6    /* (AN25/LCD_PWR2/LVDS_SHTD)     */
#define LCD_VDD            TEGRA_GPIO_PB1    /* (N9/GMI_A18/EN_VDDLCD_T30S)   */
#define LCD_CABC           TEGRA_GPIO_PH3    /* (J1/GMI_AD11/LCD_DCR)         */
#define BL_ENABLE          TEGRA_GPIO_PH1    /* (E1/GMI_AD9/DISPOFF#)         */
#define BL_PWM             TEGRA_GPIO_PH0    /* (G3/GMI_AD8/LCD_PWM_OUT )     */
#if defined(CONFIG_TEGRA_HDMI)
#define HDMI_HPD           TEGRA_GPIO_PN7    /* (AN23/HDMI_INT/HDMI_DET_T30S) */
#define HDMI_5V            TEGRA_GPIO_PI4    /* (L5/GMI_RST_N/EN_HDMI_5V0)    */
#define HDMI_5V_ALWAYS_ON  1
#endif

#if defined(CONFIG_TEGRA_HDMI)
static struct regulator *acer_hdmi_reg = NULL;
static struct regulator *acer_hdmi_pll = NULL;
#endif

static atomic_t sd_brightness = ATOMIC_INIT(255);
static struct board_info board_info;
static struct delayed_work bl_en_gpio;

extern int acer_board_type;

static void acer_backlight_work_queue(struct work_struct *work)
{
	gpio_set_value(BL_ENABLE, 1);
}

static int acer_backlight_init(struct device *dev)
{
	/* TBR: disable gpio to  change function pin */
	tegra_gpio_disable(BL_PWM);
	INIT_DELAYED_WORK(&bl_en_gpio,acer_backlight_work_queue);
	return 0;
}

static void acer_backlight_exit(struct device *dev)
{
	gpio_set_value(BL_ENABLE, 0);

	msleep(200);
}

static int acer_backlight_notify(struct device *unused, int brightness)
{
	int cur_sd_brightness = atomic_read(&sd_brightness);
	static int ori_brightness = 0;

	if (ori_brightness != !!brightness) {
		if (!ori_brightness){
			cancel_delayed_work_sync(&bl_en_gpio);
			schedule_delayed_work(&bl_en_gpio,msecs_to_jiffies(220));
		}
	}

	ori_brightness = !!brightness;

	brightness = (brightness * cur_sd_brightness) / 255;
	if (cur_sd_brightness != 255) {
		pr_debug("[PANEL] NVSD BL - sd: %d, out: %d\n",
						cur_sd_brightness, brightness);
	}

	if (brightness > 255) {
		pr_err("[PANEL]Error: Brightness > 255!\n");
	}

	return brightness;
}

static struct platform_pwm_backlight_data acer_backlight_data = {
	.pwm_id         = 0,
	.max_brightness = 255,
	.dft_brightness = 224,
	.lth_brightness = 30,
	.pwm_period_ns  = 100000,
	.init           = acer_backlight_init,
	.exit           = acer_backlight_exit,
	.notify         = acer_backlight_notify,
};

static struct platform_device acer_backlight_device = {
	.name    = "pwm-backlight",
	.id      = -1,
	.dev     = {
		.platform_data = &acer_backlight_data,
	},
};

static int acer_panel_enable(void)
{
	gpio_set_value(LCD_VDD,1);
	udelay(400);
	gpio_set_value(LVDS_SHUTDOWN,1);
	msleep(10);
	return 0;
}

static int acer_panel_disable(void)
{
#if defined(CONFIG_MACH_PICASSO_MF)
	gpio_set_value(BL_ENABLE, 0);
	msleep(210);
#endif
	gpio_set_value(LCD_VDD, 0);
	udelay(160);
	gpio_set_value(LVDS_SHUTDOWN, 0);

	return 0;
}

#if defined(CONFIG_TEGRA_HDMI)
#if !HDMI_5V_ALWAYS_ON
static int acer_hdmi_vddio_enable(void)
{
	int err;
	err = gpio_direction_output(HDMI_5V, 1);
	if (err) {
		pr_err("[HDMI] failed to enable hdmi_5V_enable\n");
	}
	return err;
}

static int acer_hdmi_vddio_disable(void)
{
	int err;
	err = gpio_direction_output(HDMI_5V, 0);
	if (err) {
		pr_err("[HDMI] failed to disable hdmi_5V_enable\n");
	}
	return err;
}
#endif

static int acer_hdmi_enable(void)
{
	int ret;
	if (!acer_hdmi_reg) {
		acer_hdmi_reg = regulator_get(NULL, "avdd_hdmi");
		if (IS_ERR_OR_NULL(acer_hdmi_reg)) {
			pr_err("[hdmi]: couldn't get regulator avdd_hdmi\n");
			acer_hdmi_reg = NULL;
			return PTR_ERR(acer_hdmi_reg);
		}
	}
	ret = regulator_enable(acer_hdmi_reg);
	if (ret < 0) {
		pr_err("[hdmi]: couldn't enable regulator avdd_hdmi\n");
		return ret;
	}
	if (!acer_hdmi_pll) {
		acer_hdmi_pll = regulator_get(NULL, "avdd_hdmi_pll");
		if (IS_ERR_OR_NULL(acer_hdmi_pll)) {
			pr_err("[hdmi]: couldn't get regulator avdd_hdmi_pll\n");
			acer_hdmi_pll = NULL;
			regulator_put(acer_hdmi_reg);
			acer_hdmi_reg = NULL;
			return PTR_ERR(acer_hdmi_pll);
		}
	}
	ret = regulator_enable(acer_hdmi_pll);
	if (ret < 0) {
		pr_err("[hdmi]: couldn't enable regulator avdd_hdmi_pll\n");
		return ret;
	}
	return 0;
}

static int acer_hdmi_disable(void)
{
	regulator_disable(acer_hdmi_reg);
	regulator_put(acer_hdmi_reg);
	acer_hdmi_reg = NULL;

	regulator_disable(acer_hdmi_pll);
	regulator_put(acer_hdmi_pll);
	acer_hdmi_pll = NULL;

	return 0;
}
#endif

static struct resource acer_disp1_resources[] = {
	{
		.name    = "irq",
		.start   = INT_DISPLAY_GENERAL,
		.end     = INT_DISPLAY_GENERAL,
		.flags   = IORESOURCE_IRQ,
	},
	{
		.name    = "regs",
		.start   = TEGRA_DISPLAY_BASE,
		.end     = TEGRA_DISPLAY_BASE + TEGRA_DISPLAY_SIZE-1,
		.flags   = IORESOURCE_MEM,
	},
	{
		.name    = "fbmem",
		.start   = 0,    /* Filled in by acer_panel_init() */
		.end     = 0,    /* Filled in by acer_panel_init() */
		.flags   = IORESOURCE_MEM,
	},
	{
		.name    = "dsi_regs",
		.start   = TEGRA_DSI_BASE,
		.end     = TEGRA_DSI_BASE + TEGRA_DSI_SIZE - 1,
		.flags   = IORESOURCE_MEM,
	},
};

static struct tegra_dc_sd_settings acer_sd_settings = {
	.enable = 1, /* Disabled by default. */
	.use_auto_pwm = false,
	.hw_update_delay = 0,
	.bin_width = -1,
	.aggressiveness = 1,
	.phase_in_adjustments = true,
	.use_vid_luma = true,
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
	.bl_device = &acer_backlight_device,
};
/* DISPLAY PICASSO 2 */
static struct tegra_dc_mode acer_p2_panel_modes[] = {
	{
		.pclk = 152000000,
		.h_ref_to_sync = 11,
		.v_ref_to_sync = 1,
		.h_sync_width = 28,
		.v_sync_width = 4,
		.h_back_porch = 28,
		.v_back_porch = 4,
		.h_active = 1920,
		.v_active = 1200,
		.h_front_porch = 28,
		.v_front_porch = 4,
	},
};

static struct tegra_fb_data acer_p2_fb_data = {
	.win            = 0,
	.xres           = 1920,
	.yres           = 1200,
	.bits_per_pixel = 32,
	.flags          = TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_dc_out acer_p2_disp1_out = {
	.align          = TEGRA_DC_ALIGN_MSB,
	.order          = TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings    = &acer_sd_settings,
	.type           = TEGRA_DC_OUT_RGB,
	.parent_clk     = "pll_d_out0",
	.depth          = 24,
	.dither         = TEGRA_DC_ORDERED_DITHER,
	.modes          = acer_p2_panel_modes,
	.n_modes        = ARRAY_SIZE(acer_p2_panel_modes),
	.enable         = acer_panel_enable,
	.disable        = acer_panel_disable,
	.height         = 136,
	.width          = 217,
};

static struct tegra_dc_platform_data acer_p2_disp1_pdata = {
	.flags          = TEGRA_DC_FLAG_ENABLED,
	.default_out    = &acer_p2_disp1_out,
	.emc_clk_rate   = 300000000,
	.fb             = &acer_p2_fb_data,
};

static struct nvhost_device acer_p2_disp1_device = {
	.name           = "tegradc",
	.id             = 0,
	.resource       = acer_disp1_resources,
	.num_resources  = ARRAY_SIZE(acer_disp1_resources),
	.dev = {
		.platform_data = &acer_p2_disp1_pdata,
	},
};


/* DISPLAY PICASSO M */
static struct tegra_dc_mode acer_pm_panel_modes[] = {
	{
		.pclk = 76000000,
		.h_ref_to_sync = 0,
		.v_ref_to_sync = 12,
		.h_sync_width = 30,
		.v_sync_width = 5,
		.h_back_porch = 52,
		.v_back_porch = 20,
		.h_active = 1280,
		.v_active = 800,
		.h_front_porch = 64,
		.v_front_porch = 25,
	},
};

static struct tegra_fb_data acer_pm_fb_data = {
	.win            = 0,
	.xres           = 1280,
	.yres           = 800,
	.bits_per_pixel = 32,
	.flags          = TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_dc_out acer_pm_disp1_out = {
	.align          = TEGRA_DC_ALIGN_MSB,
	.order          = TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings    = &acer_sd_settings,
	.type           = TEGRA_DC_OUT_RGB,
	.parent_clk     = "pll_d_out0",
	.depth          = 18,
	.dither         = TEGRA_DC_ORDERED_DITHER,
	.modes          = acer_pm_panel_modes,
	.n_modes        = ARRAY_SIZE(acer_pm_panel_modes),
	.enable         = acer_panel_enable,
	.disable        = acer_panel_disable,
	.height         = 136,
	.width          = 217,
};

static struct tegra_dc_platform_data acer_pm_disp1_pdata = {
	.flags          = TEGRA_DC_FLAG_ENABLED,
	.default_out    = &acer_pm_disp1_out,
	.emc_clk_rate   = 300000000,
	.fb             = &acer_pm_fb_data,
};

static struct nvhost_device acer_pm_disp1_device = {
	.name           = "tegradc",
	.id             = 0,
	.resource       = acer_disp1_resources,
	.num_resources  = ARRAY_SIZE(acer_disp1_resources),
	.dev = {
		.platform_data = &acer_pm_disp1_pdata,
	},
};

/*
	HDMI
*/
#if defined(CONFIG_TEGRA_HDMI)
static struct resource acer_disp2_resources[] = {
	{
		.name    = "irq",
		.start   = INT_DISPLAY_B_GENERAL,
		.end     = INT_DISPLAY_B_GENERAL,
		.flags   = IORESOURCE_IRQ,
	},
	{
		.name    = "regs",
		.start   = TEGRA_DISPLAY2_BASE,
		.end     = TEGRA_DISPLAY2_BASE + TEGRA_DISPLAY2_SIZE - 1,
		.flags   = IORESOURCE_MEM,
	},
	{
		.name    = "fbmem",
		.flags   = IORESOURCE_MEM,
		.start   = 0,
		.end     = 0,
	},
	{
		.name    = "hdmi_regs",
		.start   = TEGRA_HDMI_BASE,
		.end     = TEGRA_HDMI_BASE + TEGRA_HDMI_SIZE - 1,
		.flags   = IORESOURCE_MEM,
	},
};

static struct tegra_fb_data acer_hdmi_fb_data = {
	.win            = 0,
	.xres           = 1920,
	.yres           = 1200,
	.bits_per_pixel = 32,
	.flags          = TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_dc_out acer_disp2_out = {
	.type           = TEGRA_DC_OUT_HDMI,
	.flags          = TEGRA_DC_OUT_HOTPLUG_HIGH,
	.parent_clk     = "pll_d2_out0",
	.dcc_bus        = 3,
	.hotplug_gpio   = HDMI_HPD,
	.max_pixclock   = KHZ2PICOS(148500),
	.align          = TEGRA_DC_ALIGN_MSB,
	.order          = TEGRA_DC_ORDER_RED_BLUE,
	.enable         = acer_hdmi_enable,
	.disable        = acer_hdmi_disable,
#if !HDMI_5V_ALWAYS_ON
	.postsuspend    = acer_hdmi_vddio_disable,
	.hotplug_init   = acer_hdmi_vddio_enable,
#endif
};

static struct tegra_dc_platform_data acer_disp2_pdata = {
	.flags          = 0,
	.default_out    = &acer_disp2_out,
	.fb             = &acer_hdmi_fb_data,
	.emc_clk_rate   = 300000000,
};

static struct nvhost_device acer_disp2_device = {
	.name           = "tegradc",
	.id             = 1,
	.resource       = acer_disp2_resources,
	.num_resources  = ARRAY_SIZE(acer_disp2_resources),
	.dev = {
		.platform_data = &acer_disp2_pdata,
	},
};
#endif

/*
	CARVEOUT
*/

static struct nvmap_platform_carveout acer_carveouts[] = {
	[0] = {
		.name       = "iram",
		.usage_mask = NVMAP_HEAP_CARVEOUT_IRAM,
		.base       = TEGRA_IRAM_BASE + TEGRA_RESET_HANDLER_SIZE,
		.size       = TEGRA_IRAM_SIZE - TEGRA_RESET_HANDLER_SIZE,
		.buddy_size = 0, /* no buddy allocation for IRAM */
	},
	[1] = {
		.name       = "generic-0",
		.usage_mask = NVMAP_HEAP_CARVEOUT_GENERIC,
		.base       = 0,    /* Filled in by acer_panel_init() */
		.size       = 0,    /* Filled in by acer_panel_init() */
		.buddy_size = SZ_32K,
	},
};

static struct nvmap_platform_data acer_nvmap_data = {
	.carveouts      = acer_carveouts,
	.nr_carveouts   = ARRAY_SIZE(acer_carveouts),
};

static struct platform_device acer_nvmap_device = {
	.name           = "tegra-nvmap",
	.id             = -1,
	.dev = {
		.platform_data = &acer_nvmap_data,
	},
};

static struct platform_device *acer_gfx_devices[] __initdata = {
	&acer_nvmap_device,
	&tegra_pwfm0_device,
	&acer_backlight_device,
};


#ifdef CONFIG_HAS_EARLYSUSPEND
struct early_suspend acer_panel_early_suspender;

static void acer_panel_early_suspend(struct early_suspend *h)
{
	cancel_delayed_work_sync(&bl_en_gpio);
	gpio_set_value(BL_ENABLE, 0);
	msleep(210);
	if (num_registered_fb > 0)
		fb_blank(registered_fb[0], FB_BLANK_POWERDOWN);
	if (num_registered_fb > 1)
		fb_blank(registered_fb[1], FB_BLANK_NORMAL);
}

static void acer_panel_late_resume(struct early_suspend *h)
{
	unsigned i;
	for (i = 0; i < num_registered_fb; i++)
		fb_blank(registered_fb[i], FB_BLANK_UNBLANK);
}
#endif

int __init acer_panel_init(void)
{
	int err;
	struct resource *res;

	tegra_get_board_info(&board_info);

	acer_carveouts[1].base = tegra_carveout_start;
	acer_carveouts[1].size = tegra_carveout_size;

	tegra_gpio_enable(LVDS_SHUTDOWN);
	tegra_gpio_enable(LCD_VDD);
	tegra_gpio_enable(LCD_CABC);
	tegra_gpio_enable(BL_ENABLE);
#if defined(CONFIG_TEGRA_HDMI)
	tegra_gpio_enable(HDMI_HPD);
#endif
	gpio_request(LVDS_SHUTDOWN, "lvds_shutdown");
	gpio_request(LCD_VDD, "lcd_vdd");
	gpio_request(LCD_CABC, "lcd_cabc");
	gpio_request(BL_ENABLE, "bl_enable");
#if defined(CONFIG_TEGRA_HDMI)
	gpio_request(HDMI_HPD, "hdmi_hpd");
#endif

	gpio_direction_output(LVDS_SHUTDOWN,1);
	gpio_direction_output(LCD_VDD, 1);
	gpio_direction_output(LCD_CABC,0);
	gpio_direction_output(BL_ENABLE,1);

#if defined(CONFIG_TEGRA_HDMI)
	tegra_gpio_enable(HDMI_5V);
	err = gpio_request(HDMI_5V, "hdmi_5V_enable");
	if (err) {
		pr_err("[HDMI] request 5V_enable failed\n");
	}
#if HDMI_5V_ALWAYS_ON
	err = gpio_direction_output(HDMI_5V, 1);
#else
	err = gpio_direction_output(HDMI_5V, 0);
#endif
	if (err) {
		pr_err("[HDMI] failed to set direction of hdmi_5V_enable\n");
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	acer_panel_early_suspender.suspend = acer_panel_early_suspend;
	acer_panel_early_suspender.resume = acer_panel_late_resume;
	acer_panel_early_suspender.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&acer_panel_early_suspender);
#endif

#ifdef CONFIG_TEGRA_GRHOST
	err = tegra3_register_host1x_devices();
	if (err)
		return err;
#endif

	err = platform_add_devices(acer_gfx_devices,
			ARRAY_SIZE(acer_gfx_devices));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	if (acer_board_type == BOARD_PICASSO_M) {
		res = nvhost_get_resource_byname(&acer_pm_disp1_device,
				IORESOURCE_MEM, "fbmem");
	}else{
		res = nvhost_get_resource_byname(&acer_p2_disp1_device,
				IORESOURCE_MEM, "fbmem");
	}
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;
#endif

	/* Copy the bootloader fb to the fb. */
	tegra_move_framebuffer(tegra_fb_start, tegra_bootloader_fb_start,
				min(tegra_fb_size, tegra_bootloader_fb_size));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	if(!err){
		if (acer_board_type == BOARD_PICASSO_M) {
			err = nvhost_device_register(&acer_pm_disp1_device);
		}else{
			err = nvhost_device_register(&acer_p2_disp1_device);
		}
	}
#if defined(CONFIG_TEGRA_HDMI)
	res = nvhost_get_resource_byname(&acer_disp2_device,
				IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb2_start;
	res->end = tegra_fb2_start + tegra_fb2_size - 1;
	if (!err)
		err = nvhost_device_register(&acer_disp2_device);
#endif
#endif

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_NVAVP)
	if (!err)
		err = nvhost_device_register(&nvavp_device);
#endif
	return err;
}
