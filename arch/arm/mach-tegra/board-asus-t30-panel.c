/*
 * arch/arm/mach-tegra/board-asus-t30-panel.c
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
#include <linux/ion.h>
#include <linux/tegra_ion.h>
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
#include <mach/smmu.h>
#include <mach/board-asus-t30-misc.h>

#include "board.h"
#include "board-asus-t30.h"
#include "devices.h"
#include "gpio-names.h"
#include "tegra3_host1x_devices.h"

#define DC_CTRL_MODE	(TEGRA_DC_OUT_ONE_SHOT_MODE | \
			 TEGRA_DC_OUT_ONE_SHOT_LP_MODE)

#define AVDD_LCD PMU_TCA6416_GPIO_PORT17
#define DSI_PANEL_RESET 1

/* Select LVDS panel resolution. 13X7 is default */
#define PM313_LVDS_PANEL_19X12			1
#define PM313_LVDS_PANEL_BPP			0 /* 0:24bpp, 1:18bpp */

/* PM313 display board specific pins */
#define pm313_R_FDE			TEGRA_GPIO_PW0
#define pm313_R_FB			TEGRA_GPIO_PN4
#define pm313_MODE0			TEGRA_GPIO_PZ4
#define pm313_MODE1			TEGRA_GPIO_PW1
#define pm313_BPP			TEGRA_GPIO_PN6 /* 0:24bpp, 1:18bpp */
#define pm313_lvds_shutdown		TEGRA_GPIO_PL2

/* E1506 display board pins */
#define e1506_lcd_te		TEGRA_GPIO_PJ1
#define e1506_dsi_vddio		TEGRA_GPIO_PH1
#define e1506_dsia_bl_pwm	TEGRA_GPIO_PH0
#define e1506_panel_enb		TEGRA_GPIO_PW1
#define e1506_bl_enb		TEGRA_GPIO_PH2

/* E1247 reworked for pm269 pins */
#define e1247_pm269_lvds_shutdown	TEGRA_GPIO_PN6

/* E1247 cardhu default display board pins */
#define cardhu_lvds_shutdown		TEGRA_GPIO_PL2

/* common pins( backlight ) for all display boards */
#define cardhu_bl_enb			TEGRA_GPIO_PH2
#define cardhu_bl_pwm			TEGRA_GPIO_PH0
#define cardhu_hdmi_hpd			TEGRA_GPIO_PN7
#define cardhu_hdmi_enb         	TEGRA_GPIO_PP2
#define cardhu_hdmi_enb_ME570T_ME301T	TPS6591X_GPIO_8

/* common dsi panel pins */
#define cardhu_dsia_bl_enb		TEGRA_GPIO_PW1
#define cardhu_dsib_bl_enb		TEGRA_GPIO_PW0
#define cardhu_dsi_pnl_reset		TEGRA_GPIO_PD2
#define cardhu_dsi_219_pnl_reset	TEGRA_GPIO_PW0

#define ME301T_panel_type_ID1 TEGRA_GPIO_PH7 //GMI_AD15
#define ME301T_panel_type_ID2 TEGRA_GPIO_PK7 //GMI_A19

#define EN_VDD_BL TEGRA_GPIO_PH3

#ifdef CONFIG_TEGRA_DC
static struct regulator *cardhu_hdmi_reg = NULL;
static struct regulator *cardhu_hdmi_pll = NULL;
static struct regulator *cardhu_hdmi_vddio = NULL;
#endif

static atomic_t sd_brightness = ATOMIC_INIT(255);

static struct regulator *cardhu_dsi_reg = NULL;
static struct regulator *cardhu_lvds_reg = NULL;
static struct regulator *cardhu_lvds_vdd_bl = NULL;
static struct regulator *cardhu_lvds_vdd_panel = NULL;

static struct board_info board_info;
static struct board_info display_board_info;
extern bool isRecording;
extern int scalar_update_status;

static struct i2c_board_info scalar_i2c1_board_info[] = {
    {
        I2C_BOARD_INFO("scalar", 0x49),
    },
};

static tegra_dc_bl_output cardhu_bl_output_measured = {
	0, 1, 2, 3, 4, 5, 6, 7,
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

static p_tegra_dc_bl_output bl_output = cardhu_bl_output_measured;

static bool is_panel_218;
static bool is_panel_219;
static bool is_panel_1506;
extern int cn_vf_sku;

static bool is_dsi_panel(void)
{
	return is_panel_218 || is_panel_219 || is_panel_1506;
}

static int cardhu_backlight_init(struct device *dev)
{
	int ret = 0;

	if (WARN_ON(ARRAY_SIZE(cardhu_bl_output_measured) != 256))
		pr_err("bl_output array does not have 256 elements\n");

	if (!is_dsi_panel()) {
		ret = gpio_request(cardhu_bl_enb, "backlight_enb");
		if (ret < 0)
			return ret;

		ret = gpio_direction_output(cardhu_bl_enb, 1);
		if (ret < 0)
			gpio_free(cardhu_bl_enb);
	} else if (is_panel_218) {
		/* Enable back light for DSIa panel */
		ret = gpio_request(cardhu_dsia_bl_enb, "dsia_bl_enable");
		if (ret < 0)
			return ret;

		ret = gpio_direction_output(cardhu_dsia_bl_enb, 1);
		if (ret < 0)
			gpio_free(cardhu_dsia_bl_enb);

		/* Enable back light for DSIb panel */
		ret = gpio_request(cardhu_dsib_bl_enb, "dsib_bl_enable");
		if (ret < 0)
			return ret;

		ret = gpio_direction_output(cardhu_dsib_bl_enb, 1);
		if (ret < 0)
			gpio_free(cardhu_dsib_bl_enb);
	} else if (is_panel_219) {
		/* Enable back light for DSIa panel */
		ret = gpio_request(cardhu_dsia_bl_enb, "dsia_bl_enable");
		if (ret < 0)
			return ret;

		ret = gpio_direction_output(cardhu_dsia_bl_enb, 1);
		if (ret < 0)
			gpio_free(cardhu_dsia_bl_enb);
	}

	return ret;
};

static void cardhu_backlight_exit(struct device *dev)
{
	if (!is_dsi_panel()) {
		/* int ret; */
		/*ret = gpio_request(cardhu_bl_enb, "backlight_enb");*/
		gpio_set_value(cardhu_bl_enb, 0);
		gpio_free(cardhu_bl_enb);
	} else if (is_panel_218) {
		/* Disable back light for DSIa panel */
		gpio_set_value(cardhu_dsia_bl_enb, 0);
		gpio_free(cardhu_dsia_bl_enb);

		/* Disable back light for DSIb panel */
		gpio_set_value(cardhu_dsib_bl_enb, 0);
		gpio_free(cardhu_dsib_bl_enb);

		gpio_set_value(cardhu_lvds_shutdown, 1);
		mdelay(20);
	} else if (is_panel_219) {
		/* Disable back light for DSIa panel */
		gpio_set_value(cardhu_dsia_bl_enb, 0);
		gpio_free(cardhu_dsia_bl_enb);

		gpio_set_value(cardhu_lvds_shutdown, 1);
		mdelay(20);
	}
}

static int cardhu_backlight_notify(struct device *unused, int brightness)
{
	int cur_sd_brightness = atomic_read(&sd_brightness);

	if (!is_dsi_panel()) {
		/* Set the backlight GPIO pin mode to 'backlight_enable' */
		gpio_set_value(cardhu_bl_enb, !!brightness);
	} else if (is_panel_218) {
		/* DSIa */
		gpio_set_value(cardhu_dsia_bl_enb, !!brightness);

		/* DSIb */
		gpio_set_value(cardhu_dsib_bl_enb, !!brightness);
	} else if (is_panel_219) {
		/* DSIa */
		gpio_set_value(cardhu_dsia_bl_enb, !!brightness);
	} else if (is_panel_1506) {
		/* DSIa */
		gpio_set_value(e1506_bl_enb, !!brightness);
	}

        if (tegra3_get_project_id() == TEGRA3_PROJECT_TF201
                        && isRecording) {
                gpio_set_value(cardhu_bl_enb, 1);
        }

	/* SD brightness is a percentage, 8-bit value. */
	brightness = (brightness * cur_sd_brightness) / 255;

	/* Apply any backlight response curve */
	if (brightness > 255) {
		pr_info("Error: Brightness > 255!\n");
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
	.exit		= cardhu_backlight_exit,
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

static int cardhu_panel_enable(void)
{
	//printk("Check cardhu_panel_enable \n");

	if (cardhu_lvds_vdd_panel == NULL) {
		cardhu_lvds_vdd_panel = regulator_get(NULL, "vdd_lcd_panel");
		if (WARN_ON(IS_ERR(cardhu_lvds_vdd_panel)))
			pr_err("%s: couldn't get regulator vdd_lcd_panel: %ld\n",
				__func__, PTR_ERR(cardhu_lvds_vdd_panel));
		else
			regulator_enable(cardhu_lvds_vdd_panel);
	}
	msleep(20);

	return 0;
}

static int cardhu_panel_enable_tf700t(void)
{
	int ret;
	//printk("Check cardhu_panel_enable_tf700t \n");

	if (gpio_get_value(TEGRA_GPIO_PI6)==0){	//Panel is Panasonic
		//printk("Check panel is panasonic \n");
		if (cardhu_lvds_vdd_bl == NULL) {
			cardhu_lvds_vdd_bl = regulator_get(NULL, "vdd_backlight");
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
		cardhu_lvds_reg = regulator_get(NULL, "vdd_lvds");
		if (WARN_ON(IS_ERR(cardhu_lvds_reg)))
			pr_err("%s: couldn't get regulator vdd_lvds: %ld\n",
					__func__, PTR_ERR(cardhu_lvds_reg));
		else
			regulator_enable(cardhu_lvds_reg);
	}

	if (cardhu_lvds_vdd_panel == NULL) {
		cardhu_lvds_vdd_panel = regulator_get(NULL, "vdd_lcd_panel");
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

	if (display_board_info.board_id == BOARD_DISPLAY_PM313) {
		/* lvds configuration */
		gpio_set_value(pm313_R_FDE, 1);
		gpio_set_value(pm313_R_FB, 1);
		gpio_set_value(pm313_MODE0, 1);
		gpio_set_value(pm313_MODE1, 0);
		gpio_set_value(pm313_BPP, PM313_LVDS_PANEL_BPP);

		/* FIXME : it may require more or less delay for latching
		   values correctly before enabling RGB2LVDS */
		mdelay(100);
		gpio_set_value(pm313_lvds_shutdown, 1);
	} else {
		gpio_set_value(e1247_pm269_lvds_shutdown, 1);
	}

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

	if (gpio_get_value(TEGRA_GPIO_PI6)==0 ){        //panel is panasonic
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
	/*
	   regulator_disable(cardhu_lvds_reg);
	   regulator_put(cardhu_lvds_reg);
	   cardhu_lvds_reg = NULL;
	 */
	return 0;
}

static int cardhu_panel_postpoweron(void)
{
	//tf700t not get involved
	//printk("Check cardhu_panel_postpoweron \n");

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
	//printk("Check cardhu_panel_prepoweroff \n");

	// For TF300T EN_VDD_BL is always on, no need to control cardhu_lvds_vdd_bl
	// But for TF300TG/TL, EN_VDD_BL is BL_EN, need to control it
	// EE confirms that we can control it in original timing because
	// EN_VDD_BL/LCD_BL_PWM/LCD_BL_EN pull high/low almost the same time
	if(cardhu_lvds_vdd_bl) {
		regulator_disable(cardhu_lvds_vdd_bl);
		regulator_put(cardhu_lvds_vdd_bl);
		cardhu_lvds_vdd_bl = NULL;
	}    
	msleep(200);

	if (display_board_info.board_id == BOARD_DISPLAY_PM313) {
		gpio_set_value(pm313_lvds_shutdown, 0);
	} else {
		gpio_set_value(e1247_pm269_lvds_shutdown, 0);
	}    
	msleep(10);
        return 0;
}

#ifdef CONFIG_TEGRA_DC
static int cardhu_hdmi_vddio_enable(void)
{
	int ret;
	if (!cardhu_hdmi_vddio) {
		cardhu_hdmi_vddio = regulator_get(NULL, "vdd_hdmi_con");
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

static int cardhu_hdmi_enable(void)
{
	int ret;
	if (!cardhu_hdmi_reg) {
		cardhu_hdmi_reg = regulator_get(NULL, "avdd_hdmi");
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
		cardhu_hdmi_pll = regulator_get(NULL, "avdd_hdmi_pll");
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
        if ( tegra3_get_project_id() == TEGRA3_PROJECT_P1801 ){
                gpio_set_value(EN_VDD_BL, 1);
        }
	return 0;
}

static int cardhu_hdmi_disable(void)
{
	//printk("%s: hdmi power off\n", __func__);
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

        //do not power off scalar when scalar firmware is updating
        //or is recording
        if ( tegra3_get_project_id() == TEGRA3_PROJECT_P1801
                        && scalar_update_status!=2
                        && scalar_update_status!=-1
                        && !isRecording){
                msleep(300);
                gpio_set_value(EN_VDD_BL, 0);
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
#ifdef CONFIG_TEGRA_DSI_INSTANCE_1
	{
		.name	= "dsi_regs",
		.start	= TEGRA_DSIB_BASE,
		.end	= TEGRA_DSIB_BASE + TEGRA_DSIB_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
#else
	{
		.name	= "dsi_regs",
		.start	= TEGRA_DSI_BASE,
		.end	= TEGRA_DSI_BASE + TEGRA_DSI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
#endif
};

static struct resource cardhu_disp1_resources_P1801[] = {
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
		.name   = "hdmi_regs",
		.start  = TEGRA_HDMI_BASE,
		.end    = TEGRA_HDMI_BASE + TEGRA_HDMI_SIZE - 1,
		.flags  = IORESOURCE_MEM,
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
#endif

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

static struct tegra_dc_mode cardhu_panel_modes_P1801[] = {
	{
		.pclk = 148500000,
		.h_ref_to_sync = 1,
		.v_ref_to_sync = 1,
		.h_sync_width = 44,
		.v_sync_width = 5,
		.h_back_porch = 148,
		.v_back_porch = 36,
		.h_active = 1920,
		.v_active = 1080,
		.h_front_porch = 88,
		.v_front_porch = 4,
	},
};

static struct tegra_dc_mode cardhu_panel_modes_800_1280[] = {
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

static struct tegra_dc_mode cardhu_panel_modes[] = {
	{
		/* 1366x768@60Hz */
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
	{
		/* 1366x768@50Hz */
		.pclk = 74180000,
		.h_ref_to_sync = 1,
		.v_ref_to_sync = 1,
		.h_sync_width = 30,
		.v_sync_width = 5,
		.h_back_porch = 56,
		.v_back_porch = 80,
		.h_active = 1366,
		.v_active = 768,
		.h_front_porch = 64,
		.v_front_porch = 125,
	},
	{
		/* 1366x768@48 */
		.pclk = 74180000,
		.h_ref_to_sync = 1,
		.v_ref_to_sync = 1,
		.h_sync_width = 30,
		.v_sync_width = 5,
		.h_back_porch = 52,
		.v_back_porch = 98,
		.h_active = 1366,
		.v_active = 768,
		.h_front_porch = 64,
		.v_front_porch = 152,
	},
};

static struct tegra_dc_mode cardhu_panel_modes_55hz[] = {
	{
		/* 1366x768p 55Hz */
		.pclk = 68000000,
		.h_ref_to_sync = 0,
		.v_ref_to_sync = 12,
		.h_sync_width = 30,
		.v_sync_width = 5,
		.h_back_porch = 52,
		.v_back_porch = 20,
		.h_active = 1366,
		.v_active = 768,
		.h_front_porch = 64,
		.v_front_porch = 25,
	},
};

static struct tegra_dc_sd_settings cardhu_sd_settings = {
	.enable = 1, /* enabled by default. */
	.use_auto_pwm = false,
	.hw_update_delay = 0,
	.bin_width = -1,
	.aggressiveness = 1,
	.phase_in_adjustments = true,
#ifdef CONFIG_TEGRA_SD_GEN2
	.k_limit_enable = true,
	.k_limit = 180,
	.sd_window_enable = false,
	.soft_clipping_enable = true,
	/* Low soft clipping threshold to compensate for aggressive k_limit */
	.soft_clipping_threshold = 128,
	.smooth_k_enable = true,
	.smooth_k_incr = 4,
#endif

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
	.bl_device = &cardhu_backlight_device,
};

#ifdef CONFIG_TEGRA_DC
static struct tegra_fb_data cardhu_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 800,
#ifdef CONFIG_TEGRA_DC_USE_HW_BPP
	.bits_per_pixel = -1,
#else
	.bits_per_pixel	= 32,
#endif
//	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_fb_data cardhu_hdmi_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 800,
#ifdef CONFIG_TEGRA_DC_USE_HW_BPP
	.bits_per_pixel = -1,
#else
	.bits_per_pixel	= 32,
#endif
//	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_fb_data cardhu_fb_data_800_1280 = {
	.win		= 0,
	.xres		= 800,
	.yres		= 1280,
	.bits_per_pixel	= 32,
//	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_fb_data cardhu_hdmi_fb_data_800_1280 = {
	.win		= 0,
	.xres		= 800,
	.yres		= 1280,
	.bits_per_pixel	= 32,
//	.flags		= TEGRA_FB_FLIP_ON_PROBE,
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

static int cardhu_dsi_panel_enable(void)
{
	int ret;

	if (cardhu_dsi_reg == NULL) {
		cardhu_dsi_reg = regulator_get(NULL, "avdd_dsi_csi");
		if (IS_ERR_OR_NULL(cardhu_dsi_reg)) {
			pr_err("dsi: Could not get regulator avdd_dsi_csi\n");
			cardhu_dsi_reg = NULL;
			return PTR_ERR(cardhu_dsi_reg);
		}
	}

	regulator_enable(cardhu_dsi_reg);

	if (!is_panel_1506) {
		ret = gpio_request(AVDD_LCD, "avdd_lcd");
		if (ret < 0)
			gpio_free(AVDD_LCD);
		ret = gpio_direction_output(AVDD_LCD, 1);
		if (ret < 0)
			gpio_free(AVDD_LCD);
	}

	if (is_panel_219) {
		ret = gpio_request(cardhu_bl_pwm, "bl_pwm");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(cardhu_bl_pwm, 0);
		if (ret < 0) {
			gpio_free(cardhu_bl_pwm);
			return ret;
		}

		ret = gpio_request(cardhu_bl_enb, "bl_enb");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(cardhu_bl_enb, 0);
		if (ret < 0) {
			gpio_free(cardhu_bl_enb);
			return ret;
		}

		gpio_set_value(cardhu_lvds_shutdown, 1);
		mdelay(20);
		gpio_set_value(cardhu_bl_pwm, 1);
		mdelay(10);
		gpio_set_value(cardhu_bl_enb, 1);
		mdelay(15);
	} else if (is_panel_1506) {
		ret = gpio_request(e1506_dsi_vddio, "e1506_dsi_vddio");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(e1506_dsi_vddio, 0);
		if (ret < 0) {
			gpio_free(e1506_dsi_vddio);
			return ret;
		}

		ret = gpio_request(e1506_panel_enb, "e1506_panel_enb");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(e1506_panel_enb, 0);
		if (ret < 0) {
			gpio_free(e1506_panel_enb);
			return ret;
		}

		ret = gpio_request(e1506_bl_enb, "e1506_bl_enb");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(e1506_bl_enb, 0);
		if (ret < 0) {
			gpio_free(e1506_bl_enb);
			return ret;
		}

		gpio_set_value(e1506_dsi_vddio, 1);
		mdelay(1);
		gpio_set_value(e1506_panel_enb, 1);
		mdelay(10);
		gpio_set_value(e1506_bl_enb, 1);
		mdelay(15);
	}

#if DSI_PANEL_RESET
	if (is_panel_218) {
		ret = gpio_request(cardhu_dsi_pnl_reset, "dsi_panel_reset");
		if (ret < 0)
			return ret;

		ret = gpio_direction_output(cardhu_dsi_pnl_reset, 0);
		if (ret < 0) {
			gpio_free(cardhu_dsi_pnl_reset);
			return ret;
		}

		gpio_set_value(cardhu_dsi_pnl_reset, 1);
		gpio_set_value(cardhu_dsi_pnl_reset, 0);
		mdelay(2);
		gpio_set_value(cardhu_dsi_pnl_reset, 1);
		mdelay(2);
	} else if (is_panel_219) {
		ret = gpio_request(cardhu_dsi_219_pnl_reset, "dsi_panel_reset");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(cardhu_dsi_219_pnl_reset, 0);
		if (ret < 0) {
			gpio_free(cardhu_dsi_219_pnl_reset);
			return ret;
		}

		gpio_set_value(cardhu_dsi_219_pnl_reset, 0);
		gpio_set_value(cardhu_dsi_219_pnl_reset, 1);
		mdelay(10);
		gpio_set_value(cardhu_dsi_219_pnl_reset, 0);
		mdelay(10);
		gpio_set_value(cardhu_dsi_219_pnl_reset, 1);
		mdelay(15);
	} else if (is_panel_1506) {
		ret = gpio_request(cardhu_dsi_pnl_reset, "dsi_panel_reset");
		if (ret < 0)
			return ret;
		ret = gpio_direction_output(cardhu_dsi_pnl_reset, 0);
		if (ret < 0) {
			gpio_free(cardhu_dsi_pnl_reset);
			return ret;
		}

		gpio_set_value(cardhu_dsi_pnl_reset, 1);
		mdelay(1);
		gpio_set_value(cardhu_dsi_pnl_reset, 0);
		mdelay(1);
		gpio_set_value(cardhu_dsi_pnl_reset, 1);
		mdelay(20);
	}
#endif

	return 0;
}

static int cardhu_dsi_panel_disable(void)
{
	int err;

	err = 0;

	if (is_panel_219) {
		gpio_free(cardhu_dsi_219_pnl_reset);
		gpio_free(cardhu_bl_enb);
		gpio_free(cardhu_bl_pwm);
		gpio_free(cardhu_lvds_shutdown);
	} else if (is_panel_218) {
		gpio_free(cardhu_dsi_pnl_reset);
	} else if (is_panel_1506) {
		gpio_free(e1506_bl_enb);
		gpio_free(cardhu_dsi_pnl_reset);
		gpio_free(e1506_panel_enb);
		gpio_free(e1506_dsi_vddio);
	}
	return err;
}

static int cardhu_dsi_panel_postsuspend(void)
{
	int err;

	err = 0;
	printk(KERN_INFO "DSI panel postsuspend\n");

	if (cardhu_dsi_reg) {
		err = regulator_disable(cardhu_dsi_reg);
		if (err < 0)
			printk(KERN_ERR
			"DSI regulator avdd_dsi_csi disable failed\n");
		regulator_put(cardhu_dsi_reg);
		cardhu_dsi_reg = NULL;
	}

	if (is_panel_218)
		gpio_free(AVDD_LCD);

	return err;
}

static struct tegra_dsi_cmd dsi_init_cmd[] = {
	DSI_CMD_SHORT(0x05, 0x11, 0x00),
	DSI_DLY_MS(150),
#if (DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	DSI_CMD_SHORT(0x15, 0x35, 0x00),
#endif
	DSI_CMD_SHORT(0x05, 0x29, 0x00),
	DSI_DLY_MS(20),
};

u8 password_array[] = {0xb9, 0xff, 0x83, 0x92};

static struct tegra_dsi_cmd dsi_init_cmd_1506[] = {
	DSI_CMD_SHORT(0x05, 0x11, 0x00),
	DSI_DLY_MS(150),
	DSI_CMD_LONG(0x39, password_array),
	DSI_DLY_MS(10),
	DSI_CMD_SHORT(0x15, 0xd4, 0x0c),
	DSI_DLY_MS(10),
	DSI_CMD_SHORT(0x15, 0xba, 0x11),
	DSI_DLY_MS(10),
#if (DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	DSI_CMD_SHORT(0x15, 0x35, 0x00),
#endif
	DSI_CMD_SHORT(0x05, 0x29, 0x00),
	DSI_DLY_MS(20),
};

static struct tegra_dsi_cmd dsi_early_suspend_cmd[] = {
	DSI_CMD_SHORT(0x05, 0x28, 0x00),
	DSI_DLY_MS(20),
#if (DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	DSI_CMD_SHORT(0x05, 0x34, 0x00),
#endif
};

static struct tegra_dsi_cmd dsi_late_resume_cmd[] = {
#if (DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	DSI_CMD_SHORT(0x15, 0x35, 0x00),
#endif
	DSI_CMD_SHORT(0x05, 0x29, 0x00),
	DSI_DLY_MS(20),
};

static struct tegra_dsi_cmd dsi_suspend_cmd[] = {
	DSI_CMD_SHORT(0x05, 0x28, 0x00),
	DSI_DLY_MS(120),
#if (DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	DSI_CMD_SHORT(0x05, 0x34, 0x00),
#endif
	DSI_CMD_SHORT(0x05, 0x10, 0x00),
	DSI_DLY_MS(5),
};

static struct tegra_dsi_cmd dsi_suspend_cmd_1506[] = {
	DSI_CMD_SHORT(0x05, 0x28, 0x00),
	DSI_CMD_SHORT(0x05, 0x10, 0x00),
	DSI_DLY_MS(120),
};

struct tegra_dsi_out cardhu_dsi = {
	.n_data_lanes = 2,
	.pixel_format = TEGRA_DSI_PIXEL_FORMAT_24BIT_P,

#if (DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	/*
	 * The one-shot frame time must be shorter than the time between TE.
	 * Increasing refresh_rate will result in a decrease in the frame time
	 * for one-shot. rated_refresh_rate is only an approximation of the
	 * TE rate, and is only used to report refresh rate to upper layers.
	 */
	.refresh_rate = 66,
	.rated_refresh_rate = 60,
#else
	.refresh_rate = 60,
#endif
	.virtual_channel = TEGRA_DSI_VIRTUAL_CHANNEL_0,

	.panel_has_frame_buffer = true,
#ifdef CONFIG_TEGRA_DSI_INSTANCE_1
	.dsi_instance = 1,
#else
	.dsi_instance = 0,
#endif
	.panel_reset = DSI_PANEL_RESET,
	.power_saving_suspend = true,

	.n_early_suspend_cmd = ARRAY_SIZE(dsi_early_suspend_cmd),
	.dsi_early_suspend_cmd = dsi_early_suspend_cmd,

	.n_late_resume_cmd = ARRAY_SIZE(dsi_late_resume_cmd),
	.dsi_late_resume_cmd = dsi_late_resume_cmd,

	.n_suspend_cmd = ARRAY_SIZE(dsi_suspend_cmd),
	.dsi_suspend_cmd = dsi_suspend_cmd,

	.video_data_type = TEGRA_DSI_VIDEO_TYPE_COMMAND_MODE,
	.lp_cmd_mode_freq_khz = 430000,
};

static struct tegra_dc_mode cardhu_dsi_modes_219[] = {
	{
		.pclk = 10000000,
		.h_ref_to_sync = 4,
		.v_ref_to_sync = 1,
		.h_sync_width = 16,
		.v_sync_width = 1,
		.h_back_porch = 32,
		.v_back_porch = 1,
		.h_active = 540,
		.v_active = 960,
		.h_front_porch = 32,
		.v_front_porch = 2,
	},
};

static struct tegra_dc_mode cardhu_dsi_modes_218[] = {
	{
		.pclk = 323000000,
		.h_ref_to_sync = 11,
		.v_ref_to_sync = 1,
		.h_sync_width = 16,
		.v_sync_width = 4,
		.h_back_porch = 16,
		.v_back_porch = 4,
		.h_active = 864,
		.v_active = 480,
		.h_front_porch = 16,
		.v_front_porch = 4,
	},
};

static struct tegra_dc_mode cardhu_dsi_modes_1506[] = {
	{
		.pclk = 61417000,
		.h_ref_to_sync = 2,
		.v_ref_to_sync = 2,
		.h_sync_width = 4,
		.v_sync_width = 4,
		.h_back_porch = 100,
		.v_back_porch = 14,
		.h_active = 720,
		.v_active = 1280,
		.h_front_porch = 4,
		.v_front_porch = 4,
	},
};


static struct tegra_fb_data cardhu_dsi_fb_data = {
	.win		= 0,
	.bits_per_pixel	= 32,
	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_dc_out cardhu_disp1_out = {
	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings	= &cardhu_sd_settings,
	.parent_clk	= "pll_p",
};

static struct tegra_dc_out cardhu_disp1_out_P1801 = {
	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,
	.sd_settings	= &cardhu_sd_settings,
	.parent_clk	= "pll_d_out0",

	.depth		= 24,

	.modes		= cardhu_panel_modes_P1801,
	.n_modes	= ARRAY_SIZE(cardhu_panel_modes_P1801),

	.type		= TEGRA_DC_OUT_HDMI,
	.flags		= TEGRA_DC_OUT_HOTPLUG_HIGH,
	.dcc_bus	= 3,
	.hotplug_gpio	= cardhu_hdmi_hpd,
	.max_pixclock	= KHZ2PICOS(148500),

	.enable		= cardhu_hdmi_enable,
	.disable	= cardhu_hdmi_disable,

	.postsuspend	= cardhu_hdmi_vddio_disable,
	.hotplug_init	= cardhu_hdmi_vddio_enable,
};

#ifdef CONFIG_TEGRA_DC
static struct tegra_dc_platform_data cardhu_disp1_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.default_out	= &cardhu_disp1_out,
	.emc_clk_rate	= 300000000,
};

static struct nvhost_device cardhu_disp1_device = {
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

static struct nvhost_device cardhu_disp2_device = {
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

#if defined(CONFIG_TEGRA_NVMAP)
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

#if defined(CONFIG_ION_TEGRA)

static struct platform_device tegra_iommu_device = {
	.name = "tegra_iommu_device",
	.id = -1,
	.dev = {
		.platform_data = (void *)((1 << HWGRP_COUNT) - 1),
	},
};

static struct ion_platform_data tegra_ion_data = {
	.nr = 4,
	.heaps = {
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = TEGRA_ION_HEAP_CARVEOUT,
			.name = "carveout",
			.base = 0,
			.size = 0,
		},
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = TEGRA_ION_HEAP_IRAM,
			.name = "iram",
			.base = TEGRA_IRAM_BASE + TEGRA_RESET_HANDLER_SIZE,
			.size = TEGRA_IRAM_SIZE - TEGRA_RESET_HANDLER_SIZE,
		},
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = TEGRA_ION_HEAP_VPR,
			.name = "vpr",
			.base = 0,
			.size = 0,
		},
		{
			.type = ION_HEAP_TYPE_IOMMU,
			.id = TEGRA_ION_HEAP_IOMMU,
			.name = "iommu",
			.base = TEGRA_SMMU_BASE,
			.size = TEGRA_SMMU_SIZE,
			.priv = &tegra_iommu_device.dev,
		},
	},
};

static struct platform_device tegra_ion_device = {
	.name = "ion-tegra",
	.id = -1,
	.dev = {
		.platform_data = &tegra_ion_data,
	},
};
#endif

static struct platform_device *cardhu_gfx_devices[] __initdata = {
#if defined(CONFIG_TEGRA_NVMAP)
	&cardhu_nvmap_device,
#endif
#if defined(CONFIG_ION_TEGRA)
	&tegra_ion_device,
#endif
	&tegra_pwfm0_device,
	&cardhu_backlight_device,
};


#ifdef CONFIG_HAS_EARLYSUSPEND
/* put early_suspend/late_resume handlers here for the display in order
 * to keep the code out of the display driver, keeping it closer to upstream
 */
struct early_suspend cardhu_panel_early_suspender;

static void cardhu_panel_early_suspend(struct early_suspend *h)
{
	/* power down LCD, add use a black screen for HDMI */
	if (num_registered_fb > 0)
		fb_blank(registered_fb[0], FB_BLANK_POWERDOWN);
	if (num_registered_fb > 1)
		fb_blank(registered_fb[1], FB_BLANK_NORMAL);

#ifdef CONFIG_TEGRA_CONVSERVATIVE_GOV_ON_EARLYSUPSEND
	cpufreq_store_default_gov();
	cpufreq_change_gov(cpufreq_conservative_gov);
#endif
}

static void cardhu_panel_late_resume(struct early_suspend *h)
{
	unsigned i;
#ifdef CONFIG_TEGRA_CONVSERVATIVE_GOV_ON_EARLYSUPSEND
	cpufreq_restore_default_gov();
#endif
	for (i = 0; i < num_registered_fb; i++)
		fb_blank(registered_fb[i], FB_BLANK_UNBLANK);
}
#endif

static void cardhu_panel_preinit(void)
{
	if (display_board_info.board_id == BOARD_DISPLAY_E1213)
		is_panel_218 = true;
	else if (display_board_info.board_id == BOARD_DISPLAY_E1253)
		is_panel_219 = true;
	else if (display_board_info.board_id == BOARD_DISPLAY_E1506)
		is_panel_1506 = true;

	if (!is_dsi_panel()) {
		cardhu_disp1_out.parent_clk_backup = "pll_d_out0";
		cardhu_disp1_out.type = TEGRA_DC_OUT_RGB;
		cardhu_disp1_out.depth = 18;
		cardhu_disp1_out.dither = TEGRA_DC_ORDERED_DITHER;
		if (tegra3_get_project_id() == TEGRA3_PROJECT_ME570T){
			cardhu_disp1_out.modes = cardhu_panel_modes_800_1280;
			cardhu_disp1_out.n_modes = ARRAY_SIZE(cardhu_panel_modes_800_1280);
			cardhu_disp1_pdata.fb = &cardhu_fb_data_800_1280;
		}
		else{
			cardhu_disp1_out.modes = cardhu_panel_modes;
			cardhu_disp1_out.n_modes = ARRAY_SIZE(cardhu_panel_modes);
			cardhu_disp1_pdata.fb = &cardhu_fb_data;
		}

		if (tegra3_get_project_id()!=0x4 ){
			cardhu_disp1_out.enable = cardhu_panel_enable;
			cardhu_disp1_out.postpoweron = cardhu_panel_postpoweron;
		} else {
		        cardhu_disp1_out.enable = cardhu_panel_enable_tf700t;
		}

		if (tegra3_get_project_id()!=0x4 ){
			cardhu_disp1_out.disable = cardhu_panel_disable;
			cardhu_disp1_out.prepoweroff = cardhu_panel_prepoweroff;
		} else {
			cardhu_disp1_out.disable = cardhu_panel_disable_tf700t;
		}
		/* Set height and width in mm. */
		cardhu_disp1_out.height = 127;
		cardhu_disp1_out.width = 216;
	} else {
		cardhu_disp1_out.flags = DC_CTRL_MODE;
		cardhu_disp1_out.type = TEGRA_DC_OUT_DSI;
		cardhu_disp1_out.dsi = &cardhu_dsi;
		cardhu_disp1_out.enable = cardhu_dsi_panel_enable;
		cardhu_disp1_out.disable = cardhu_dsi_panel_disable;
		cardhu_disp1_out.postsuspend = cardhu_dsi_panel_postsuspend;

		cardhu_dsi.n_init_cmd = ARRAY_SIZE(dsi_init_cmd);
		cardhu_dsi.dsi_init_cmd = dsi_init_cmd;
		cardhu_dsi.n_suspend_cmd = ARRAY_SIZE(dsi_suspend_cmd);
		cardhu_dsi.dsi_suspend_cmd = dsi_suspend_cmd;

		if (is_panel_218) {
			cardhu_disp1_out.modes	= cardhu_dsi_modes_218;
			cardhu_disp1_out.n_modes =
				ARRAY_SIZE(cardhu_dsi_modes_218);
			cardhu_dsi_fb_data.xres = 864;
			cardhu_dsi_fb_data.yres = 480;
			/* Set height and width in mm. */
			cardhu_disp1_out.height = 47;
			cardhu_disp1_out.width = 84;
		} else if (is_panel_219) {
			cardhu_disp1_out.modes	= cardhu_dsi_modes_219;
			cardhu_disp1_out.n_modes =
				ARRAY_SIZE(cardhu_dsi_modes_219);
			cardhu_dsi_fb_data.xres = 540;
			cardhu_dsi_fb_data.yres = 960;
			/* Set height and width in mm. */
			cardhu_disp1_out.height = 95;
			cardhu_disp1_out.width = 53;
		} else if (is_panel_1506) {
			cardhu_disp1_out.modes	= cardhu_dsi_modes_1506;
			cardhu_disp1_out.n_modes =
				ARRAY_SIZE(cardhu_dsi_modes_1506);
			cardhu_dsi.n_init_cmd = ARRAY_SIZE(dsi_init_cmd_1506);
			cardhu_dsi.dsi_init_cmd = dsi_init_cmd_1506;
			cardhu_dsi.n_suspend_cmd =
				ARRAY_SIZE(dsi_suspend_cmd_1506);
			cardhu_dsi.dsi_suspend_cmd = dsi_suspend_cmd_1506;
			cardhu_dsi.panel_send_dc_frames = true;
			cardhu_dsi.suspend_aggr = DSI_HOST_SUSPEND_LV0;
			cardhu_dsi_fb_data.xres = 720;
			cardhu_dsi_fb_data.yres = 1280;
			/* Set height and width in mm. */
			cardhu_disp1_out.height = 95;
			cardhu_disp1_out.width = 53;
		}

		cardhu_disp1_pdata.fb = &cardhu_dsi_fb_data;
	}
}

int __init cardhu_panel_init(void)
{
	int err;
	struct resource __maybe_unused *res;

	tegra_get_board_info(&board_info);
	tegra_get_display_board_info(&display_board_info);

#if defined(CONFIG_TEGRA_NVMAP)
	cardhu_carveouts[1].base = tegra_carveout_start;
	cardhu_carveouts[1].size = tegra_carveout_size;
#endif

#if defined(CONFIG_ION_TEGRA)
	tegra_ion_data.heaps[0].base = tegra_carveout_start;
	tegra_ion_data.heaps[0].size = tegra_carveout_size;
#endif

	cardhu_panel_preinit();
	if (is_dsi_panel())
		goto skip_lvds;
#if defined(CONFIG_TEGRA_DC)
	if (WARN_ON(board_info.board_id == BOARD_E1291 &&
		((board_info.sku & SKU_TOUCHSCREEN_MECH_FIX) == 0))) {
		/* use 55Hz panel timings to reduce noise on sensitive touch */
		printk("Using cardhu_panel_modes_55hz\n");
		cardhu_disp1_out.parent_clk = "pll_p";
		cardhu_disp1_out.modes = cardhu_panel_modes_55hz;
		cardhu_disp1_out.n_modes = ARRAY_SIZE(cardhu_panel_modes_55hz);
	}

	if (display_board_info.board_id == BOARD_DISPLAY_PM313) {
		/* initialize the values */
#if defined(PM313_LVDS_PANEL_19X12)
		cardhu_disp1_out.modes = panel_19X12_modes;
		cardhu_disp1_out.n_modes = ARRAY_SIZE(panel_19X12_modes);
		cardhu_disp1_out.parent_clk = "pll_d_out0";
#if (PM313_LVDS_PANEL_BPP == 1)
		cardhu_disp1_out.depth = 18;
#else
		cardhu_disp1_out.depth = 24;
#endif
		/* Set height and width in mm. */
		cardhu_disp1_out.height = 127;
		cardhu_disp1_out.width = 203;
		cardhu_fb_data.xres = 1920;
		cardhu_fb_data.yres = 1200;

		cardhu_disp2_out.parent_clk = "pll_d2_out0";
		cardhu_hdmi_fb_data.xres = 1920;
		cardhu_hdmi_fb_data.yres = 1200;
#endif

		/* lvds configuration */
		err = gpio_request(pm313_R_FDE, "R_FDE");
		err |= gpio_direction_output(pm313_R_FDE, 1);

		err |= gpio_request(pm313_R_FB, "R_FB");
		err |= gpio_direction_output(pm313_R_FB, 1);

		err |= gpio_request(pm313_MODE0, "MODE0");
		err |= gpio_direction_output(pm313_MODE0, 1);

		err |= gpio_request(pm313_MODE1, "MODE1");
		err |= gpio_direction_output(pm313_MODE1, 0);

		err |= gpio_request(pm313_BPP, "BPP");
		err |= gpio_direction_output(pm313_BPP, PM313_LVDS_PANEL_BPP);

		err = gpio_request(pm313_lvds_shutdown, "lvds_shutdown");
		/* free ride provided by bootloader */
		err |= gpio_direction_output(pm313_lvds_shutdown, 1);

		if (err)
			printk(KERN_ERR "ERROR(s) in LVDS configuration\n");
	}

	if ( tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG && cn_vf_sku){
		cardhu_disp1_out.modes->pclk = 83900000;
		cardhu_disp1_out.modes->v_front_porch = 200;
		printk("TF300TG: Set LCD pclk as %d Hz, cn_vf_sku=%d\n", cardhu_disp1_out.modes->pclk, cn_vf_sku);
	}

	if ( tegra3_get_project_id() == TEGRA3_PROJECT_P1801 ){
		printk("P1801 display setting, set HDMI as main display\n ");
		cardhu_fb_data.xres = 1920;
		cardhu_fb_data.yres = 1080;

		cardhu_disp1_out_P1801.sd_settings->panel_min_brightness = 77;
		cardhu_disp1_out_P1801.sd_settings->enable = 0;

		cardhu_disp1_pdata.default_out = &cardhu_disp1_out_P1801;
		cardhu_disp1_device.resource	= cardhu_disp1_resources_P1801;
		cardhu_disp1_device.num_resources = ARRAY_SIZE(cardhu_disp1_resources_P1801);

		pr_info("scalar i2c_register_board_info");
		i2c_register_board_info(0, scalar_i2c1_board_info, ARRAY_SIZE(scalar_i2c1_board_info));

                gpio_request(EN_VDD_BL, "EN_VDD_BL");
                gpio_direction_output(EN_VDD_BL, 1);
	}

	if ( tegra3_get_project_id() == TEGRA3_PROJECT_ME301T){
		cardhu_disp1_pdata.min_emc_clk_rate = 102000000;

		cardhu_disp1_out.sd_settings->panel_min_brightness = 28;

		//take effect for ER2/PR/MP
		gpio_request(ME301T_panel_type_ID1, "ME301T_panel_type_ID1");
		gpio_direction_input(ME301T_panel_type_ID1);

		gpio_request(ME301T_panel_type_ID2, "ME301T_panel_type_ID2");
		gpio_direction_input(ME301T_panel_type_ID2);

		printk("%s: panel_type_ID = (%d, %d)\n", __func__, gpio_get_value(ME301T_panel_type_ID1), gpio_get_value(ME301T_panel_type_ID2));
	}

	if (tegra3_get_project_id()==0x4 ){
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
	if (tegra3_get_project_id()==0x4 ){
		gpio_request(cardhu_hdmi_enb, "hdmi_5v_en");
		gpio_direction_output(cardhu_hdmi_enb, 0);
	}
	else if (tegra3_get_project_id() == TEGRA3_PROJECT_ME570T ||
		tegra3_get_project_id() == TEGRA3_PROJECT_ME301T){
		gpio_request(cardhu_hdmi_enb_ME570T_ME301T, "hdmi_5v_en");
		gpio_direction_output(cardhu_hdmi_enb_ME570T_ME301T, 1);
	}
	else {
		gpio_request(cardhu_hdmi_enb, "hdmi_5v_en");
		gpio_direction_output(cardhu_hdmi_enb, 1);
	}

skip_lvds:
	gpio_request(cardhu_hdmi_hpd, "hdmi_hpd");
	gpio_direction_input(cardhu_hdmi_hpd);

#if !(DC_CTRL_MODE & TEGRA_DC_OUT_ONE_SHOT_MODE)
	gpio_request(e1506_lcd_te, "lcd_te");
	gpio_direction_input(e1506_lcd_te);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	cardhu_panel_early_suspender.suspend = cardhu_panel_early_suspend;
	cardhu_panel_early_suspender.resume = cardhu_panel_late_resume;
	cardhu_panel_early_suspender.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&cardhu_panel_early_suspender);
#endif

#ifdef CONFIG_TEGRA_GRHOST
	err = tegra3_register_host1x_devices();
	if (err)
		return err;
#endif

	err = platform_add_devices(cardhu_gfx_devices,
				ARRAY_SIZE(cardhu_gfx_devices));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	res = nvhost_get_resource_byname(&cardhu_disp1_device,
					 IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;
#endif

	/* Copy the bootloader fb to the fb. */
	tegra_move_framebuffer(tegra_fb_start, tegra_bootloader_fb_start,
				min(tegra_fb_size, tegra_bootloader_fb_size));

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_DC)
	if (!err)
		err = nvhost_device_register(&cardhu_disp1_device);

	if ( tegra3_get_project_id() != TEGRA3_PROJECT_P1801 ){

		res = nvhost_get_resource_byname(&cardhu_disp2_device,
						 IORESOURCE_MEM, "fbmem");
		res->start = tegra_fb2_start;
		res->end = tegra_fb2_start + tegra_fb2_size - 1;

		/* Copy the bootloader fb to the fb2. */
		tegra_move_framebuffer(tegra_fb2_start, tegra_bootloader_fb_start,
					min(tegra_fb2_size, tegra_bootloader_fb_size));

		if (!err)
			err = nvhost_device_register(&cardhu_disp2_device);
	}

#endif

#if defined(CONFIG_TEGRA_GRHOST) && defined(CONFIG_TEGRA_NVAVP)
	if (!err)
		err = nvhost_device_register(&nvavp_device);
#endif
	return err;
}
