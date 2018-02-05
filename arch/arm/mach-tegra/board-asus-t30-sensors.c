/*
 * arch/arm/mach-tegra/board-asus-t30-sensors.c
 *
 * Copyright (c) 2010-2012, NVIDIA CORPORATION, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of NVIDIA CORPORATION nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/nct1008.h>
#include <linux/mpu_inv.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>

#ifdef CONFIG_VIDEO_YUV
#include <media/yuv_sensor.h>
#endif /* CONFIG_VIDEO_YUV */

#include <mach/edp.h>
#include <mach/board-asus-t30-misc.h>

#include "board-asus-t30.h"
#include "cpu-tegra.h"

#ifdef CONFIG_VIDEO_YUV
static struct regulator *reg_cardhu_cam;	/* LDO6 */
static struct regulator *reg_cardhu_1v8_cam;	/* VDDIO_CAM 1.8V PBB4 */
static struct regulator *reg_cardhu_2v85_cam;	/* Front Camera 2.85V power */
static struct regulator *reg_cardhu_1v2_cam;	/* VDDIO_CAM 1.2V PS0 */
static struct regulator *reg_cardhu_af_pwr_en;	/* ICATCH7002A_AF_PWR_EN_GPIO PS0 */
static struct regulator *reg_cardhu_vdda_en;	/* ICATCH7002A_VDDA_EN_GPIO GPIO_PR6*/
static bool camera_busy = false;
#endif /* CONFIG_VIDEO_YUV */

static int IsTF300(void)
{
	u32 project_info = tegra3_get_project_id();

	if ((project_info == TEGRA3_PROJECT_TF300T) || 
		(project_info == TEGRA3_PROJECT_TF300TG) || 
		(project_info == TEGRA3_PROJECT_TF300TL))
		return 1;
	else
		return 0;
}

static int cardhu_camera_init(void)
{
	pr_info("cardhu_camera_init");
	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
		gpio_request(TF700T_ISP_POWER_1V2_EN_GPIO, "isp_power_1v2_en");
		gpio_request(TF700T_ISP_POWER_1V8_EN_GPIO, "isp_power_1v8_en");
		gpio_request(ISP_POWER_RESET_GPIO, "isp_power_rstx");
	}
	else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201){
		gpio_request(ISP_POWER_1V2_EN_GPIO, "isp_power_1v2_en");
		gpio_request(ISP_POWER_RESET_GPIO, "isp_power_rstx");
		gpio_request(CAM3_POWER_DWN_GPIO, "cam3_power_dwn");
		gpio_request(FRONT_YUV_SENSOR_RST_GPIO, "yuv_sensor_rst_lo");
	}
	else if(IsTF300()) {
		gpio_request(ICATCH7002A_VDDIO_EN_GPIO, "cam_vddio_ldo_en");
		gpio_request(ICATCH7002A_VDDC_EN_GPIO, "cam_vddc_ldo_en");
		gpio_request(ICATCH7002A_PWR_DN_GPIO, "cam_power_dwn");
		gpio_request(ICATCH7002A_RST_GPIO, "cam_sensor_rst_lo");
	}
	return 0;
}

#ifdef CONFIG_VIDEO_YUV
static int fjm6mo_sensor_power_on(struct device *dev)
{
	printk("fjm6mo_sensor_power_on+\n");

	if(camera_busy){
		printk("fjm6mo_sensor busy\n");
		return -EBUSY;
	}
	camera_busy = true;
	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
	{
		if (!reg_cardhu_1v2_cam) {
			reg_cardhu_1v2_cam = regulator_get(dev, "vdd_cam3");
			if (IS_ERR_OR_NULL(reg_cardhu_1v2_cam)) {
				pr_err("TF700T_m6mo_power_on PS0: vdd_cam3 failed\n");
				reg_cardhu_1v2_cam = NULL;
				return PTR_ERR(reg_cardhu_1v2_cam);
			}
			regulator_set_voltage(reg_cardhu_1v2_cam, 1200000, 1200000);
			regulator_enable(reg_cardhu_1v2_cam);
		}

		pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V2_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V2_EN_GPIO));
		gpio_direction_output(TF700T_ISP_POWER_1V2_EN_GPIO, 1);
		pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V2_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V2_EN_GPIO));

		if (!reg_cardhu_1v8_cam) {
			reg_cardhu_1v8_cam = regulator_get(dev, "vdd_1v8_cam1");
			if (IS_ERR_OR_NULL(reg_cardhu_1v8_cam)) {
				pr_err("TF700T_m6mo_power_on PBB4: vdd_1v8_cam1 failed\n");
				reg_cardhu_1v8_cam = NULL;
				return PTR_ERR(reg_cardhu_1v8_cam);
			}
			regulator_set_voltage(reg_cardhu_1v8_cam, 1800000, 1800000);
			regulator_enable(reg_cardhu_1v8_cam);
		}

		pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V8_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V8_EN_GPIO));
		gpio_direction_output(TF700T_ISP_POWER_1V8_EN_GPIO, 1);
		pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V8_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V8_EN_GPIO));

		msleep(1);
		tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_NORMAL);
	} else {
		//For i2c bus
		gpio_request(143, "gpio_pr7");
		gpio_direction_output(143, 1);
		pr_info("gpio 2.85V %d set to %d\n",143, gpio_get_value(143));

		pr_info("gpio %d set to %d\n",ISP_POWER_1V2_EN_GPIO, gpio_get_value(ISP_POWER_1V2_EN_GPIO));
		gpio_direction_output(ISP_POWER_1V2_EN_GPIO, 1);
		pr_info("gpio %d set to %d\n",ISP_POWER_1V2_EN_GPIO, gpio_get_value(ISP_POWER_1V2_EN_GPIO));

		msleep(5);

		if (!reg_cardhu_1v8_cam) {
			reg_cardhu_1v8_cam = regulator_get(dev, "vdd_1v8_cam1");
			if (IS_ERR_OR_NULL(reg_cardhu_1v8_cam)) {
				pr_err("TF201_m6mo_power_on PBB4: vdd_1v8_cam1 failed\n");
				reg_cardhu_1v8_cam = NULL;
				return PTR_ERR(reg_cardhu_1v8_cam);
			}
			regulator_set_voltage(reg_cardhu_1v8_cam, 1800000, 1800000);
			regulator_enable(reg_cardhu_1v8_cam);
		}
	}
	if (!reg_cardhu_cam) {
		reg_cardhu_cam = regulator_get(dev, "avdd_dsi_csi");
		if (IS_ERR_OR_NULL(reg_cardhu_cam)) {
			pr_err("TF201_m6mo_power_on LDO6: p_tegra_cam failed\n");
			reg_cardhu_cam = NULL;
			return PTR_ERR(reg_cardhu_cam);
		}
		regulator_set_voltage(reg_cardhu_cam, 1200000, 1200000);
		regulator_enable(reg_cardhu_cam);
	}

	return 0;
}

static int fjm6mo_sensor_power_off(struct device *dev)
{
	if(reg_cardhu_cam){
		regulator_disable(reg_cardhu_cam);
		regulator_put(reg_cardhu_cam);
		reg_cardhu_cam = NULL;
	}

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
		tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_TRISTATE);

		gpio_direction_output(TF700T_ISP_POWER_1V8_EN_GPIO, 0);
		pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V8_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V8_EN_GPIO));

		gpio_direction_output(TF700T_ISP_POWER_1V2_EN_GPIO, 0);
		pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V2_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V2_EN_GPIO));
	} else {
		if(reg_cardhu_1v8_cam){
			regulator_disable(reg_cardhu_1v8_cam);
			regulator_put(reg_cardhu_1v8_cam);
			reg_cardhu_1v8_cam = NULL;
		}
		gpio_direction_output(ISP_POWER_1V2_EN_GPIO, 0);
		pr_info("gpio %d set to %d\n",ISP_POWER_1V2_EN_GPIO, gpio_get_value(ISP_POWER_1V2_EN_GPIO));
	}

	printk("fjm6mo_sensor_power_off-\n");
	return 0;
}

struct yuv_sensor_platform_data fjm6mo_sensor_data = {
	.power_on = fjm6mo_sensor_power_on,
	.power_off = fjm6mo_sensor_power_off,
};

#if 0
//this should be moved to camera driver

int yuv_sensor_power_on_reset_pin(void)
{
	pr_info("gpio %d set to %d\n",ISP_POWER_RESET_GPIO, gpio_get_value(ISP_POWER_RESET_GPIO));
	gpio_direction_output(ISP_POWER_RESET_GPIO, 1);
	pr_info("gpio %d set to %d\n",ISP_POWER_RESET_GPIO, gpio_get_value(ISP_POWER_RESET_GPIO));
	return 0;
}

int yuv_sensor_power_off_reset_pin(void)
{
	printk("yuv_sensor_power_off+\n");
	camera_busy = false;
	gpio_direction_output(ISP_POWER_RESET_GPIO, 0);
	pr_info("gpio %d set to %d\n",ISP_POWER_RESET_GPIO, gpio_get_value(ISP_POWER_RESET_GPIO));
	return 0;
}
#endif

static int mi1040_sensor_power_on(void)
{
	printk("mi1040_sensor_power_on+\n");

	if(camera_busy){
		printk("yuv_sensor busy\n");
		return -EBUSY;
	}

	camera_busy = true;
	/* 1.8V VDDIO_CAM controlled by "EN_1V8_CAM(GPIO_PBB4)" */
	if (!reg_cardhu_1v8_cam) {
		reg_cardhu_1v8_cam = regulator_get(NULL, "vdd_1v8_cam1"); /*cam2/3?*/
		if (IS_ERR_OR_NULL(reg_cardhu_1v8_cam)) {
			reg_cardhu_1v8_cam = NULL;
			pr_err("Can't get reg_cardhu_1v8_cam\n");
			goto fail_to_get_reg;
		}
		regulator_set_voltage(reg_cardhu_1v8_cam, 1800000, 1800000);
		regulator_enable(reg_cardhu_1v8_cam);
	}

	/* 2.85V VDD_CAM2 controlled by CAM2/3_LDO_EN(GPIO_PS0)*/
	if (!reg_cardhu_2v85_cam) {
		reg_cardhu_2v85_cam = regulator_get(NULL, "vdd_cam3");
		if (IS_ERR_OR_NULL(reg_cardhu_2v85_cam)) {
			reg_cardhu_2v85_cam = NULL;
			pr_err("Can't get reg_cardhu_2v85_cam\n");
			goto fail_to_get_reg;
		}
		regulator_set_voltage(reg_cardhu_2v85_cam, 2850000, 2850000);
		regulator_enable(reg_cardhu_2v85_cam);
	}

	/* cam_power_en, powdn*/
	pr_info("gpio %d: %d",CAM3_POWER_DWN_GPIO, gpio_get_value(CAM3_POWER_DWN_GPIO));
	gpio_set_value(CAM3_POWER_DWN_GPIO, 0);
	gpio_direction_output(CAM3_POWER_DWN_GPIO, 0);
	pr_info("--> %d\n", gpio_get_value(CAM3_POWER_DWN_GPIO));

	/* yuv_sensor_rst_lo*/
	pr_info("gpio %d: %d", FRONT_YUV_SENSOR_RST_GPIO, gpio_get_value(FRONT_YUV_SENSOR_RST_GPIO));
	gpio_set_value(FRONT_YUV_SENSOR_RST_GPIO, 1);
	gpio_direction_output(FRONT_YUV_SENSOR_RST_GPIO, 1);
	pr_info("--> %d\n", gpio_get_value(FRONT_YUV_SENSOR_RST_GPIO));

	printk("mi1040_sensor_power_on-\n");
	return 0;

fail_to_get_reg:
	if (reg_cardhu_2v85_cam) {
		regulator_put(reg_cardhu_2v85_cam);
		reg_cardhu_2v85_cam = NULL;
	}
	if (reg_cardhu_1v8_cam) {
		regulator_put(reg_cardhu_1v8_cam);
		reg_cardhu_1v8_cam = NULL;
	}

	camera_busy = false;
	printk("mi1040_sensor_power_on- : -ENODEV\n");
	return -ENODEV;
}

static int mi1040_sensor_power_off(void)
{
	printk("mi1040_sensor_power_off+\n");

	gpio_set_value(FRONT_YUV_SENSOR_RST_GPIO, 0);
	gpio_direction_output(FRONT_YUV_SENSOR_RST_GPIO, 0);

	gpio_set_value(CAM3_POWER_DWN_GPIO, 1);
	gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);

	if (reg_cardhu_2v85_cam) {
		regulator_disable(reg_cardhu_2v85_cam);
		regulator_put(reg_cardhu_2v85_cam);
		reg_cardhu_2v85_cam = NULL;
	}
	if (reg_cardhu_1v8_cam) {
		regulator_disable(reg_cardhu_1v8_cam);
		regulator_put(reg_cardhu_1v8_cam);
		reg_cardhu_1v8_cam = NULL;
	}

	camera_busy = false;
	printk("mi1040_sensor_power_off-\n");
	return 0;
}

struct yuv_sensor_platform_data mi1040_sensor_data = {
	.power_on = mi1040_sensor_power_on,
	.power_off = mi1040_sensor_power_off,
};

static int iCatch7002a_sensor_power_on(struct device *dev)
{
	printk("%s+\n", __FUNCTION__);

//	if(IsTF300()) {
//		Asus_camera_enable_set_emc_rate(667000000);
//	}

	pr_info("gpio %d read as %d\n",ICATCH7002A_VDDIO_EN_GPIO, gpio_get_value(ICATCH7002A_VDDIO_EN_GPIO));
	gpio_direction_output(ICATCH7002A_VDDIO_EN_GPIO, 1);
	pr_info("gpio %d set to %d\n",ICATCH7002A_VDDIO_EN_GPIO, gpio_get_value(ICATCH7002A_VDDIO_EN_GPIO));

	msleep(1);

	if (!reg_cardhu_vdda_en) {
		reg_cardhu_vdda_en = regulator_get(dev, "vdd_2v8_cam1");
		if (IS_ERR_OR_NULL(reg_cardhu_vdda_en)) {
			pr_err("vdda_power_on gpio_pr6: vdd_2v8_cam1 failed\n");
			reg_cardhu_vdda_en = NULL;
			goto fail_to_get_reg;
		}
		regulator_enable(reg_cardhu_vdda_en);
		mdelay(5);
	}

	if (!reg_cardhu_af_pwr_en) {
		reg_cardhu_af_pwr_en = regulator_get(dev, "vdd_cam3");
		if (IS_ERR_OR_NULL(reg_cardhu_af_pwr_en)) {
			pr_err("af_power_on gpio_ps0: vdd_cam3 failed\n");
			reg_cardhu_af_pwr_en = NULL;
			goto fail_to_get_reg;
		}
		regulator_enable(reg_cardhu_af_pwr_en);
		mdelay(5);
	}

	if(IsTF300()) {
		pr_info("gpio %d read as %d\n",ICATCH7002A_VDDC_EN_GPIO, gpio_get_value(ICATCH7002A_VDDC_EN_GPIO));
		gpio_direction_output(ICATCH7002A_VDDC_EN_GPIO, 1);
		pr_info("gpio %d set to %d\n",ICATCH7002A_VDDC_EN_GPIO, gpio_get_value(ICATCH7002A_VDDC_EN_GPIO));
	}
	msleep(1);

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_NORMAL);

	/* cam_power_en, powdn*/
	pr_info("gpio %d: %d",ICATCH7002A_PWR_DN_GPIO, gpio_get_value(ICATCH7002A_PWR_DN_GPIO));
	gpio_set_value(ICATCH7002A_PWR_DN_GPIO, 1);
	gpio_direction_output(ICATCH7002A_PWR_DN_GPIO, 1);
	pr_info("--> %d\n", gpio_get_value(ICATCH7002A_PWR_DN_GPIO));

	/* yuv_sensor_rst_lo*/
	if (IsTF300()){
		pr_info("gpio %d: %d", ICATCH7002A_RST_GPIO, gpio_get_value(ICATCH7002A_RST_GPIO));
		gpio_set_value(ICATCH7002A_RST_GPIO, 1);//high
		gpio_direction_output(ICATCH7002A_RST_GPIO, 1);
		pr_info("gpio %d--> %d\n", ICATCH7002A_RST_GPIO, gpio_get_value(ICATCH7002A_RST_GPIO));
		msleep(5);
	}
	
	gpio_set_value(ICATCH7002A_RST_GPIO, 0);//low
	gpio_direction_output(ICATCH7002A_RST_GPIO, 0);
	pr_info("gpio %d--> %d\n", ICATCH7002A_RST_GPIO, gpio_get_value(ICATCH7002A_RST_GPIO));

	msleep(25);

	gpio_set_value(ICATCH7002A_RST_GPIO, 1);//high
	gpio_direction_output(ICATCH7002A_RST_GPIO, 1);
	pr_info("gpio %d--> %d\n", ICATCH7002A_RST_GPIO, gpio_get_value(ICATCH7002A_RST_GPIO));

	msleep(6);

	gpio_set_value(ICATCH7002A_PWR_DN_GPIO, 0);//low
	gpio_direction_output(ICATCH7002A_PWR_DN_GPIO, 0);
	pr_info("gpio %d--> %d\n", ICATCH7002A_PWR_DN_GPIO, gpio_get_value(ICATCH7002A_PWR_DN_GPIO));

	return 0;

fail_to_get_reg:
	if (reg_cardhu_vdda_en) {
		regulator_disable(reg_cardhu_vdda_en);
		regulator_put(reg_cardhu_vdda_en);
		reg_cardhu_vdda_en = NULL;
	}
	if (reg_cardhu_af_pwr_en) {
		regulator_disable(reg_cardhu_af_pwr_en);
		regulator_put(reg_cardhu_af_pwr_en);
		reg_cardhu_af_pwr_en = NULL;
	}
	if (reg_cardhu_1v8_cam) {
		regulator_disable(reg_cardhu_1v8_cam);
		regulator_put(reg_cardhu_1v8_cam);
		reg_cardhu_1v8_cam = NULL;
	}

	printk("%s- : -ENODEV\n", __FUNCTION__);
	return -ENODEV;
}

static int iCatch7002a_sensor_power_off(struct device *dev)
{
	printk("%s+\n", __FUNCTION__);
	gpio_set_value(ICATCH7002A_RST_GPIO, 0);
	gpio_direction_output(ICATCH7002A_RST_GPIO, 0);

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_TRISTATE);

	if(IsTF300()){
		gpio_set_value(ICATCH7002A_VDDC_EN_GPIO, 0);
		gpio_direction_output(ICATCH7002A_VDDC_EN_GPIO, 0);
	}

	if(reg_cardhu_af_pwr_en) {
		regulator_disable(reg_cardhu_af_pwr_en);
		regulator_put(reg_cardhu_af_pwr_en);
		reg_cardhu_af_pwr_en = NULL;
	}

	if(reg_cardhu_vdda_en) {
		regulator_disable(reg_cardhu_vdda_en);
		regulator_put(reg_cardhu_vdda_en);
		reg_cardhu_vdda_en = NULL;
	}

	gpio_set_value(ICATCH7002A_VDDIO_EN_GPIO, 0);
	gpio_direction_output(ICATCH7002A_VDDIO_EN_GPIO, 0);

//	if(IsTF300()){
//		Asus_camera_disable_set_emc_rate();
//	}

	printk("%s-\n", __FUNCTION__);
  return 0;
}

struct yuv_sensor_platform_data iCatch7002a_sensor_data = {
	.power_on = iCatch7002a_sensor_power_on,
	.power_off = iCatch7002a_sensor_power_off,
};
#endif  /* CONFIG_VIDEO_YUV */

static struct throttle_table tj_throttle_table[] = {
	      /* CPU_THROT_LOW cannot be used by other than CPU */
	      /* NO_CAP cannot be used by CPU */
	      /*      CPU,    CBUS,    SCLK,     EMC */
	      { { 1000000,  NO_CAP,  NO_CAP,  NO_CAP } },
	      { {  760000,  NO_CAP,  NO_CAP,  NO_CAP } },
	      { {  760000,  NO_CAP,  NO_CAP,  NO_CAP } },
	      { {  620000,  NO_CAP,  NO_CAP,  NO_CAP } },
	      { {  620000,  NO_CAP,  NO_CAP,  NO_CAP } },
	      { {  620000,  437000,  NO_CAP,  NO_CAP } },
	      { {  620000,  352000,  NO_CAP,  NO_CAP } },
	      { {  475000,  352000,  NO_CAP,  NO_CAP } },
	      { {  475000,  352000,  NO_CAP,  NO_CAP } },
	      { {  475000,  352000,  250000,  375000 } },
	      { {  475000,  352000,  250000,  375000 } },
	      { {  475000,  247000,  204000,  375000 } },
	      { {  475000,  247000,  204000,  204000 } },
	      { {  475000,  247000,  204000,  204000 } },
	{ { CPU_THROT_LOW,  247000,  204000,  102000 } },
};

static struct balanced_throttle tj_throttle = {
	.throt_tab_size = ARRAY_SIZE(tj_throttle_table),
	.throt_tab = tj_throttle_table,
};

static int __init cardhu_throttle_init(void)
{
	balanced_throttle_register(&tj_throttle, "tegra-balanced");

	return 0;
}
module_init(cardhu_throttle_init);

static struct nct1008_platform_data cardhu_nct1008_pdata = {
	.supported_hwrev = true,
	.ext_range = true,
	.conv_rate = 0x08,
	.offset = 8, /* 4 * 2C. Bug 844025 - 1C for device accuracies */
	.shutdown_ext_limit = 90,
  	.shutdown_local_limit = 90,

	.passive_delay = 2000,

	.num_trips = 1,
	.trips = {
		/* Thermal Throttling */
		[0] = {
			.cdev_type = "tegra-balanced",
			.trip_temp = 80000,
			.trip_type = THERMAL_TRIP_PASSIVE,
			.upper = THERMAL_NO_LIMIT,
			.lower = THERMAL_NO_LIMIT,
			.hysteresis = 0,
		},
	},
};

static struct i2c_board_info cardhu_i2c4_pad_bat_board_info[] = {
	{
		I2C_BOARD_INFO("pad-battery", 0xb),
	},
};

static struct i2c_board_info cardhu_i2c4_nct1008_board_info[] = {
	{
		I2C_BOARD_INFO("nct1008", 0x4C),
		.platform_data = &cardhu_nct1008_pdata,
		.irq = -1,
	}
};

#ifdef CONFIG_VIDEO_YUV
static struct i2c_board_info fjm6mo_i2c3_board_info[] = {
	{
		I2C_BOARD_INFO("fjm6mo", 0x1F),
		.platform_data = &fjm6mo_sensor_data,
	},
};

static struct i2c_board_info mi1040_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("mi1040", 0x48),
		.platform_data = &mi1040_sensor_data,
	},
};

static struct i2c_board_info iCatch7002a_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("i7002a", 0x3C),
		.platform_data = &iCatch7002a_sensor_data,
	},
};
#endif /* CONFIG_VIDEO_YUV */

static int cardhu_nct1008_init(void)
{
	int ret = 0;

	cardhu_i2c4_nct1008_board_info[0].irq =
		gpio_to_irq(TEGRA_GPIO_PCC2);

	ret = gpio_request(TEGRA_GPIO_PCC2, "temp_alert");
	if (ret < 0) {
		pr_err("%s: gpio_request failed\n", __func__);
		return ret;
	}

	ret = gpio_direction_input(TEGRA_GPIO_PCC2);
	if (ret < 0) {
		pr_err("%s: set gpio to input failed\n", __func__);
		gpio_free(TEGRA_GPIO_PCC2);
	}

	tegra_platform_edp_init(cardhu_nct1008_pdata.trips,
				&cardhu_nct1008_pdata.num_trips,
				0); /* edp temperature margin */
	return ret;
}

#ifdef CONFIG_TEGRA_SKIN_THROTTLE
static struct thermal_trip_info skin_trips[] = {
	{
		.cdev_type = "skin-balanced",
		.trip_temp = 45000,
		.trip_type = THERMAL_TRIP_PASSIVE,
		.upper = THERMAL_NO_LIMIT,
		.lower = THERMAL_NO_LIMIT,
		.hysteresis = 0,
	},
};

static struct therm_est_subdevice skin_devs[] = {
	{
		.dev_data = "nct_ext",
		.coeffs = {
			2, 1, 1, 1,
			1, 1, 1, 1,
			1, 1, 1, 0,
			1, 1, 0, 0,
			0, 0, -1, -7
		},
	},
	{
		.dev_data = "nct_int",
		.coeffs = {
			-11, -7, -5, -3,
			-3, -2, -1, 0,
			0, 0, 1, 1,
			1, 2, 2, 3,
			4, 6, 11, 18
		},
	},
};

static struct therm_est_data skin_data = {
	.num_trips = ARRAY_SIZE(skin_trips),
	.trips = skin_trips,
	.toffset = 9793,
	.polling_period = 1100,
	.passive_delay = 30000,
	.tc1 = 5,
	.tc2 = 1,
	.ndevs = ARRAY_SIZE(skin_devs),
	.devs = skin_devs,
};

static struct throttle_table skin_throttle_table[] = {
		/* CPU_THROT_LOW cannot be used by other than CPU */
		/* NO_CAP cannot be used by CPU */
		/*      CPU,    CBUS,    SCLK,     EMC */
		{ { 1000000,  NO_CAP,  NO_CAP,  NO_CAP } },
		{ {  760000,  NO_CAP,  NO_CAP,  NO_CAP } },
		{ {  760000,  NO_CAP,  NO_CAP,  NO_CAP } },
		{ {  620000,  NO_CAP,  NO_CAP,  NO_CAP } },
		{ {  620000,  NO_CAP,  NO_CAP,  NO_CAP } },
		{ {  620000,  437000,  NO_CAP,  NO_CAP } },
		{ {  620000,  352000,  NO_CAP,  NO_CAP } },
		{ {  475000,  352000,  NO_CAP,  NO_CAP } },
		{ {  475000,  352000,  NO_CAP,  NO_CAP } },
		{ {  475000,  352000,  250000,  375000 } },
		{ {  475000,  352000,  250000,  375000 } },
		{ {  475000,  247000,  204000,  375000 } },
		{ {  475000,  247000,  204000,  204000 } },
		{ {  475000,  247000,  204000,  204000 } },
	  { { CPU_THROT_LOW,  247000,  204000,  102000 } },
};

static struct balanced_throttle skin_throttle = {
	.throt_tab_size = ARRAY_SIZE(skin_throttle_table),
	.throt_tab = skin_throttle_table,
};

static int __init cardhu_skin_init(void)
{
	balanced_throttle_register(&skin_throttle, "skin-balanced");
	tegra_skin_therm_est_device.dev.platform_data = &skin_data;
	platform_device_register(&tegra_skin_therm_est_device);

	return 0;
}
late_initcall(cardhu_skin_init);
#endif

static struct mpu_platform_data mpu3050_gyro_data = {
	.int_config	= 0x10,
	.level_shifter	= 0,
	.orientation	= MPU_GYRO_ORIENTATION,	/* Located in board_[platformname].h	*/
};

static struct ext_slave_platform_data mpu3050_accel_data = {
	.address	= MPU_ACCEL_ADDR,
	.irq		= 0,
	.adapt_num	= MPU_ACCEL_BUS_NUM,
	.bus		= EXT_SLAVE_BUS_SECONDARY,
	.orientation	= MPU_ACCEL_ORIENTATION,	/* Located in board_[platformname].h	*/
};

static struct ext_slave_platform_data mpu3050_compass_data = {
	.address	= MPU_COMPASS_ADDR,
	.irq		= 0,
	.adapt_num	= MPU_COMPASS_BUS_NUM,
	.bus		= EXT_SLAVE_BUS_PRIMARY,
	.orientation	= MPU_COMPASS_ORIENTATION,	/* Located in board_[platformname].h	*/
};

static struct i2c_board_info __initdata inv_mpu_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO(MPU_GYRO_NAME, MPU_GYRO_ADDR),
		.platform_data = &mpu3050_gyro_data,
	},
	{
		I2C_BOARD_INFO(MPU_ACCEL_NAME, MPU_ACCEL_ADDR),
		.platform_data = &mpu3050_accel_data,
	},
	{
		I2C_BOARD_INFO(MPU_COMPASS_NAME, MPU_COMPASS_ADDR),
		.platform_data = &mpu3050_compass_data,
	},
};

/*Sensors orientation definition*/
struct mpu_orientation_def{
	__s8 gyro_orientation[9];
	__s8 accel_orientation[9];
	__s8 compass_orientation[9];
};

static void mpuirq_init(void)
{
	int ret = 0;
	int i = 0;
	
	pr_info("*** MPU START *** mpuirq_init...\n");

	if (tegra3_get_project_id() == TEGRA3_PROJECT_TF300T)
	{
		/* Use "TF300T" to check the project name */
		struct mpu_orientation_def TF300T = {
			TF300T_GYRO_ORIENTATION,
			TF300T_ACCEL_ORIENTATION,
			TF300T_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF300T config...\n");
		memcpy( mpu3050_gyro_data.orientation, TF300T.gyro_orientation, sizeof(mpu3050_gyro_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF300T.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF300T.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else if (tegra3_get_project_id() == TEGRA3_PROJECT_TF300TG)
	{
		/* Use "TF300TG" to check the project name */
		struct mpu_orientation_def TF300TG = {
			TF300TG_GYRO_ORIENTATION,
			TF300TG_ACCEL_ORIENTATION,
			TF300TG_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF300TG config...\n");
		memcpy( mpu3050_gyro_data.orientation, TF300TG.gyro_orientation, sizeof(mpu3050_gyro_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF300TG.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF300TG.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else if (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
	{
		/* Use "TF700T" to check the project name */
		struct mpu_orientation_def TF700T = {
			TF700T_GYRO_ORIENTATION,
			TF700T_ACCEL_ORIENTATION,
			TF700T_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF700T config...\n");
		memcpy( mpu3050_gyro_data.orientation, TF700T.gyro_orientation, sizeof(mpu3050_gyro_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF700T.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF700T.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else if (tegra3_get_project_id() == TEGRA3_PROJECT_TF300TL)
	{
		/* Use "TF300TL" to check the project name */
		struct mpu_orientation_def TF300TL = {
			TF300TL_GYRO_ORIENTATION,
			TF300TL_ACCEL_ORIENTATION,
			TF300TL_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF300TL config...\n");
		memcpy( mpu3050_gyro_data.orientation, TF300TL.gyro_orientation, sizeof(mpu3050_gyro_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF300TL.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF300TL.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else pr_info("initial mpu with TF201 config...\n");

#if	MPU_ACCEL_IRQ_GPIO
	/* ACCEL-IRQ assignment */
	ret = gpio_request(MPU_ACCEL_IRQ_GPIO, MPU_ACCEL_NAME);
	if (ret < 0) {
		pr_err("%s: gpio_request failed %d\n", __func__, ret);
		return;
	}

	ret = gpio_direction_input(MPU_ACCEL_IRQ_GPIO);
	if (ret < 0) {
		pr_err("%s: gpio_direction_input failed %d\n", __func__, ret);
		gpio_free(MPU_ACCEL_IRQ_GPIO);
		return;
	}
#endif

	/* MPU-IRQ assignment */
	ret = gpio_request(MPU_GYRO_IRQ_GPIO, MPU_GYRO_NAME);
	if (ret < 0) {
		pr_err("%s: gpio_request failed %d\n", __func__, ret);
		return;
	}

	ret = gpio_direction_input(MPU_GYRO_IRQ_GPIO);
	if (ret < 0) {
		pr_err("%s: gpio_direction_input failed %d\n", __func__, ret);
		gpio_free(MPU_GYRO_IRQ_GPIO);
		return;
	}
	pr_info("*** MPU END *** mpuirq_init...\n");

	inv_mpu_i2c2_board_info[i++].irq = gpio_to_irq(MPU_GYRO_IRQ_GPIO);
#if MPU_ACCEL_IRQ_GPIO
	inv_mpu_i2c2_board_info[i].irq = gpio_to_irq(MPU_ACCEL_IRQ_GPIO);
#endif
	i++;
#if MPU_COMPASS_IRQ_GPIO
	inv_mpu_i2c2_board_info[i++].irq = gpio_to_irq(MPU_COMPASS_IRQ_GPIO);
#endif
	i2c_register_board_info(MPU_GYRO_BUS_NUM, inv_mpu_i2c2_board_info,
		ARRAY_SIZE(inv_mpu_i2c2_board_info));
}

static struct i2c_board_info cardhu_i2c1_board_info_al3010[] = {
	{
		I2C_BOARD_INFO("al3010",0x1C),
	},
};

int __init cardhu_sensors_init(void)
{
	int err;

	err = cardhu_nct1008_init();
	if (err)
		pr_err("%s: nct1008 init failed\n", __func__);
	else
		i2c_register_board_info(4, cardhu_i2c4_nct1008_board_info,
			ARRAY_SIZE(cardhu_i2c4_nct1008_board_info));

	cardhu_camera_init();

	cardhu_i2c1_board_info_al3010[0].irq = gpio_to_irq(TEGRA_GPIO_PZ2);
	i2c_register_board_info(2, cardhu_i2c1_board_info_al3010,
		ARRAY_SIZE(cardhu_i2c1_board_info_al3010));

#ifdef CONFIG_VIDEO_YUV
/* m6mo rear camera */
	pr_info("fjm6mo i2c_register_board_info");
	i2c_register_board_info(2, fjm6mo_i2c3_board_info,
		ARRAY_SIZE(fjm6mo_i2c3_board_info));

/* mi1040 front camera */
	pr_info("mi1040 i2c_register_board_info");
	i2c_register_board_info(2, mi1040_i2c2_board_info,
		ARRAY_SIZE(mi1040_i2c2_board_info));

/* iCatch7002a rear camera */
	pr_info("iCatch7002a i2c_register_board_info");
	i2c_register_board_info(2, iCatch7002a_i2c2_board_info,
		ARRAY_SIZE(iCatch7002a_i2c2_board_info));
#endif /* CONFIG_VIDEO_YUV */

	i2c_register_board_info(4, cardhu_i2c4_pad_bat_board_info,
			ARRAY_SIZE(cardhu_i2c4_pad_bat_board_info));

	mpuirq_init();

	return 0;
}
