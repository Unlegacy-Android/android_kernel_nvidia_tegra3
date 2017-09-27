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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_I2C_MUX_PCA954x
#include <linux/i2c/pca954x.h>
#endif
#include <linux/i2c/pca953x.h>
#include <linux/nct1008.h>
#include <mach/fb.h>
#include <mach/gpio.h>
#ifdef CONFIG_VIDEO_OV5650
#include <media/ov5650.h>
#include <media/ov5640.h>
#include <media/ov14810.h>
#endif
#ifdef CONFIG_VIDEO_OV2710
#include <media/ov2710.h>
#endif
#ifdef CONFIG_VIDEO_YUV
#include <media/yuv_sensor.h>
#endif /* CONFIG_VIDEO_YUV */
#include <media/tps61050.h>
#include <generated/mach-types.h>
#include "gpio-names.h"
#include "board.h"
#include <linux/mpu.h>

/* KIONIX KXT_9 Digital Tri-axis Accelerometer */
//#include <plat/mux.h>
#include <linux/kxt_9.h>

#ifdef CONFIG_VIDEO_SH532U
#include <media/sh532u.h>
#endif
#include <linux/bq27x00.h>
#include <mach/gpio.h>
#include <mach/edp.h>
#include <mach/thermal.h>
#include <linux/therm_est.h>

#include "gpio-names.h"
#include "board-asus-t30.h"
#include "cpu-tegra.h"

#include <mach/board-asus-t30-misc.h>
#include <linux/smb347-charger.h>

#if 0 //WK: Disable NV's camera code
static struct regulator *cardhu_1v8_cam1 = NULL;
static struct regulator *cardhu_1v8_cam2 = NULL;
static struct regulator *cardhu_1v8_cam3 = NULL;
static struct regulator *cardhu_vdd_2v8_cam1 = NULL;
static struct regulator *cardhu_vdd_2v8_cam2 = NULL;
static struct regulator *cardhu_vdd_cam3 = NULL;
#endif

static struct board_info board_info;
static struct regulator *reg_cardhu_cam;	/* LDO6 */
static struct regulator *reg_cardhu_1v8_cam;	/* VDDIO_CAM 1.8V PBB4 */
static struct regulator *reg_cardhu_2v85_cam;	/* Front Camera 2.85V power */
static struct regulator *reg_cardhu_1v2_cam;	/* VDDIO_CAM 1.2V PS0 */
static struct regulator *reg_cardhu_af_pwr_en;	/* ICATCH7002A_AF_PWR_EN_GPIO PS0 */
static struct regulator *reg_cardhu_vdda_en;	/* ICATCH7002A_VDDA_EN_GPIO GPIO_PR6*/
static struct regulator *reg_cardhu_vddio_cam;	/* LDO5 It's only for ME301T */
static bool camera_busy = false;

#ifdef CONFIG_I2C_MUX_PCA954x
static struct pca954x_platform_mode cardhu_pca954x_modes[] = {
	{ .adap_id = PCA954x_I2C_BUS0, .deselect_on_exit = true, },
	{ .adap_id = PCA954x_I2C_BUS1, .deselect_on_exit = true, },
	{ .adap_id = PCA954x_I2C_BUS2, .deselect_on_exit = true, },
	{ .adap_id = PCA954x_I2C_BUS3, .deselect_on_exit = true, },
};

static struct pca954x_platform_data cardhu_pca954x_data = {
	.modes    = cardhu_pca954x_modes,
	.num_modes      = ARRAY_SIZE(cardhu_pca954x_modes),
};
#endif

static int IsTF300(void)
{
    u32 project_info = tegra3_get_project_id();

    if (project_info == TEGRA3_PROJECT_TF300T)
        return 1;
    else if (project_info == TEGRA3_PROJECT_TF300TG)
        return 1;
    else if (project_info == TEGRA3_PROJECT_TF300TL)
        return 1;
    else
        return 0;
}

static int cardhu_camera_init(void)
{
#if 0 //WK: Disable NV's code.
	int ret;

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		ret = gpio_request(CAM1_POWER_DWN_GPIO, "camera_power_en");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, "CAM1_POWER_DWN_GPIO");
		ret = gpio_request(CAM3_POWER_DWN_GPIO, "cam3_power_en");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, "CAM3_POWER_DWN_GPIO");

		ret = gpio_request(CAM2_POWER_DWN_GPIO, "camera2_power_en");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, "CAM2_POWER_DWN_GPIO");

		ret = gpio_request(OV5650_RESETN_GPIO, "camera_reset");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, "OV5650_RESETN_GPIO");

		gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		mdelay(10);

		gpio_direction_output(OV5650_RESETN_GPIO, 1);
		mdelay(5);
		gpio_direction_output(OV5650_RESETN_GPIO, 0);
		mdelay(5);
		gpio_direction_output(OV5650_RESETN_GPIO, 1);
		mdelay(5);
	}

	/* To select the CSIB MUX either for cam2 or cam3 */
	ret = gpio_request(CAMERA_CSI_MUX_SEL_GPIO, "camera_csi_sel");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
			__func__, "CAMERA_CSI_MUX_SEL_GPIO");
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 0);
	gpio_export(CAMERA_CSI_MUX_SEL_GPIO, false);
#endif
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
	else if(IsTF300() ||(tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) ||
		(tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)) {
		if(IsTF300() || (tegra3_get_project_id() == TEGRA3_PROJECT_TF500T))
			gpio_request(ICATCH7002A_VDDIO_EN_GPIO, "cam_vddio_ldo_en");
		if(tegra3_get_project_id() == TEGRA3_PROJECT_TF500T) {
			gpio_request(ICATCH7002A_ISP_1V2_EN, "icatch_cam_vddio_ldo_en");
		}
		gpio_request(ICATCH7002A_VDDC_EN_GPIO, "cam_vddc_ldo_en");
		gpio_request(ICATCH7002A_PWR_DN_GPIO, "cam_power_dwn");
		gpio_request(ICATCH7002A_RST_GPIO, "cam_sensor_rst_lo");
	}
	return 0;
}

#ifdef CONFIG_VIDEO_YUV

static int yuv_sensor_power_on_TF700T(void)
{
    if (!reg_cardhu_1v2_cam) {
        reg_cardhu_1v2_cam = regulator_get(NULL, "vdd_cam3");
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
        reg_cardhu_1v8_cam = regulator_get(NULL, "vdd_1v8_cam1");
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

    return 0;
}

static int yuv_sensor_power_off_TF700T(void)
{
    tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_TRISTATE);

    gpio_direction_output(TF700T_ISP_POWER_1V8_EN_GPIO, 0);
    pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V8_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V8_EN_GPIO));

    gpio_direction_output(TF700T_ISP_POWER_1V2_EN_GPIO, 0);
    pr_info("gpio %d set to %d\n",TF700T_ISP_POWER_1V2_EN_GPIO, gpio_get_value(TF700T_ISP_POWER_1V2_EN_GPIO));

    return 0;
}

static int yuv_sensor_power_on(void)
{
    printk("yuv_sensor_power_on+\n");

    if(camera_busy){
        printk("yuv_sensor busy\n");
        return -EBUSY;
    }
    camera_busy = true;
    if (tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
    {
        yuv_sensor_power_on_TF700T();
    }
    else{
        //For i2c bus
        gpio_request(143, "gpio_pr7");
        gpio_direction_output(143, 1);
        pr_info("gpio 2.85V %d set to %d\n",143, gpio_get_value(143));

        pr_info("gpio %d set to %d\n",ISP_POWER_1V2_EN_GPIO, gpio_get_value(ISP_POWER_1V2_EN_GPIO));
        gpio_direction_output(ISP_POWER_1V2_EN_GPIO, 1);
        pr_info("gpio %d set to %d\n",ISP_POWER_1V2_EN_GPIO, gpio_get_value(ISP_POWER_1V2_EN_GPIO));

        msleep(5);

        if (!reg_cardhu_1v8_cam) {
            reg_cardhu_1v8_cam = regulator_get(NULL, "vdd_1v8_cam1");
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
        reg_cardhu_cam = regulator_get(NULL, "avdd_dsi_csi");
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
int yuv_sensor_power_on_reset_pin(void)
{
    pr_info("gpio %d set to %d\n",ISP_POWER_RESET_GPIO, gpio_get_value(ISP_POWER_RESET_GPIO));
    gpio_direction_output(ISP_POWER_RESET_GPIO, 1);
    pr_info("gpio %d set to %d\n",ISP_POWER_RESET_GPIO, gpio_get_value(ISP_POWER_RESET_GPIO));

    printk("yuv_sensor_power_on -\n");
    return 0;
}

static int yuv_sensor_power_off(void)
{
    if(reg_cardhu_cam){
        regulator_disable(reg_cardhu_cam);
        regulator_put(reg_cardhu_cam);
        reg_cardhu_cam = NULL;
    }

    if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
        yuv_sensor_power_off_TF700T();
    }
    else{
        if(reg_cardhu_1v8_cam){
            regulator_disable(reg_cardhu_1v8_cam);
            regulator_put(reg_cardhu_1v8_cam);
            reg_cardhu_1v8_cam = NULL;
        }
        gpio_direction_output(ISP_POWER_1V2_EN_GPIO, 0);
        pr_info("gpio %d set to %d\n",ISP_POWER_1V2_EN_GPIO, gpio_get_value(ISP_POWER_1V2_EN_GPIO));
    }

    printk("yuv_sensor_power_off-\n");
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

struct yuv_sensor_platform_data yuv_rear_sensor_data = {
    .power_on = yuv_sensor_power_on,
    .power_off = yuv_sensor_power_off,
};

static int yuv_front_sensor_power_on(void)
{
	printk("yuv_front_sensor_power_on+\n");

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
			pr_err("Can't get reg_cardhu_1v8_cam.\n");
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
  			pr_err("Can't get reg_cardhu_2v85_cam.\n");
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

	printk("yuv_front_sensor_power_on-\n");
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
	printk("yuv_front_sensor_power_on- : -ENODEV\n");
	return -ENODEV;
}

static int yuv_front_sensor_power_off(void)
{
	printk("yuv_front_sensor_power_off+\n");

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
	printk("yuv_front_sensor_power_off-\n");
	return 0;
}
struct yuv_sensor_platform_data yuv_front_sensor_data = {
	.power_on = yuv_front_sensor_power_on,
	.power_off = yuv_front_sensor_power_off,
};
/*==============++iCatch++================================*/
static int iCatch7002a_power_on(void)
{
    int ret;

    printk("%s+\n", __FUNCTION__);

	if(IsTF300() ||(tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) ||
		(tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)) {
		Asus_camera_enable_set_emc_rate(667000000);
	}

    if((tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) || (tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)){
        if (!reg_cardhu_vddio_cam) {
            reg_cardhu_vddio_cam = regulator_get(NULL, "avdd_vdac");
            if (IS_ERR_OR_NULL(reg_cardhu_vddio_cam)) {
                pr_err("ME301T_vddio_power_on LDO5: avdd_vdac failed\n");
                reg_cardhu_vddio_cam = NULL;
                goto fail_to_get_reg;
            }
            regulator_set_voltage(reg_cardhu_vddio_cam, 1800000, 1800000);
            regulator_enable(reg_cardhu_vddio_cam);
        }
    }
    else{
        pr_info("gpio %d read as %d\n",ICATCH7002A_VDDIO_EN_GPIO, gpio_get_value(ICATCH7002A_VDDIO_EN_GPIO));
        gpio_direction_output(ICATCH7002A_VDDIO_EN_GPIO, 1);
        pr_info("gpio %d set to %d\n",ICATCH7002A_VDDIO_EN_GPIO, gpio_get_value(ICATCH7002A_VDDIO_EN_GPIO));
    }

    msleep(1);

    if (!reg_cardhu_vdda_en) {
        reg_cardhu_vdda_en = regulator_get(NULL, "vdd_2v8_cam1");
        if (IS_ERR_OR_NULL(reg_cardhu_vdda_en)) {
            pr_err("vdda_power_on gpio_pr6: vdd_2v8_cam1 failed\n");
            reg_cardhu_vdda_en = NULL;
            goto fail_to_get_reg;
        }
        regulator_enable(reg_cardhu_vdda_en);
        mdelay(5);
    }



    if (!reg_cardhu_af_pwr_en) {
        reg_cardhu_af_pwr_en = regulator_get(NULL, "vdd_cam3");
        if (IS_ERR_OR_NULL(reg_cardhu_af_pwr_en)) {
            pr_err("af_power_on gpio_ps0: vdd_cam3 failed\n");
            reg_cardhu_af_pwr_en = NULL;
            goto fail_to_get_reg;
        }
        regulator_enable(reg_cardhu_af_pwr_en);
        mdelay(5);
    }
   	if(IsTF300() ||(tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) ||
		(tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)) {
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
	if (reg_cardhu_vddio_cam) {
		regulator_disable(reg_cardhu_vddio_cam);
		regulator_put(reg_cardhu_vddio_cam);
		reg_cardhu_vddio_cam = NULL;
	}
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
static int iCatch7002a_power_off(void)
{
	printk("%s+\n", __FUNCTION__);
	gpio_set_value(ICATCH7002A_RST_GPIO, 0);
	gpio_direction_output(ICATCH7002A_RST_GPIO, 0);

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_TRISTATE);

	if(IsTF300() ||(tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) ||
		(tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)){
        gpio_set_value(ICATCH7002A_VDDC_EN_GPIO, 0);
        gpio_direction_output(ICATCH7002A_VDDC_EN_GPIO, 0);
    }

    if (reg_cardhu_af_pwr_en) {
        regulator_disable(reg_cardhu_af_pwr_en);
        regulator_put(reg_cardhu_af_pwr_en);
        reg_cardhu_af_pwr_en = NULL;
    }

    if (reg_cardhu_vdda_en) {
        regulator_disable(reg_cardhu_vdda_en);
        regulator_put(reg_cardhu_vdda_en);
        reg_cardhu_vdda_en = NULL;
    }
    if((tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) || (tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)){
        if (reg_cardhu_vddio_cam) {
            regulator_disable(reg_cardhu_vddio_cam);
            regulator_put(reg_cardhu_vddio_cam);
            reg_cardhu_vddio_cam = NULL;
        }
    }
    else{
        gpio_set_value(ICATCH7002A_VDDIO_EN_GPIO, 0);
        gpio_direction_output(ICATCH7002A_VDDIO_EN_GPIO, 0);
    }

	if(IsTF300() ||(tegra3_get_project_id() == TEGRA3_PROJECT_ME301T) ||
		(tegra3_get_project_id() == TEGRA3_PROJECT_ME301TL)){
		Asus_camera_disable_set_emc_rate();
	}

	printk("%s-\n", __FUNCTION__);
  return 0;
}
struct yuv_sensor_platform_data iCatch7002a_data = {
	.power_on = iCatch7002a_power_on,
	.power_off = iCatch7002a_power_off,
};
/*==============--iCatch--================================*/


#endif  /* CONFIG_VIDEO_YUV */

#ifdef CONFIG_VIDEO_OV5650
static int cardhu_left_ov5650_power_on(void)
{
	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {

		if (cardhu_vdd_2v8_cam1 == NULL) {
			cardhu_vdd_2v8_cam1 = regulator_get(NULL, "vdd_2v8_cam1");
			if (WARN_ON(IS_ERR(cardhu_vdd_2v8_cam1))) {
				pr_err("%s: couldn't get regulator vdd_2v8_cam1: %ld\n",
					__func__, PTR_ERR(cardhu_vdd_2v8_cam1));
				goto reg_alloc_fail;
			}
		}
		regulator_enable(cardhu_vdd_2v8_cam1);
		mdelay(5);
	}

	/* Enable VDD_1V8_Cam1 */
	if (cardhu_1v8_cam1 == NULL) {
		cardhu_1v8_cam1 = regulator_get(NULL, "vdd_1v8_cam1");
		if (WARN_ON(IS_ERR(cardhu_1v8_cam1))) {
			pr_err("%s: couldn't get regulator vdd_1v8_cam1: %ld\n",
				__func__, PTR_ERR(cardhu_1v8_cam1));
			goto reg_alloc_fail;
		}
	}
	regulator_enable(cardhu_1v8_cam1);

	mdelay(5);
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 0);
		mdelay(20);
		gpio_direction_output(OV5650_RESETN_GPIO, 0);
		mdelay(100);
		gpio_direction_output(OV5650_RESETN_GPIO, 1);
	}

	if (board_info.board_id == BOARD_PM269) {
		gpio_direction_output(CAM1_RST_L_GPIO, 0);
		mdelay(100);
		gpio_direction_output(CAM1_RST_L_GPIO, 1);
	}

	return 0;

reg_alloc_fail:
	if (cardhu_1v8_cam1) {
		regulator_put(cardhu_1v8_cam1);
		cardhu_1v8_cam1 = NULL;
	}
	if (cardhu_vdd_2v8_cam1) {
		regulator_put(cardhu_vdd_2v8_cam1);
		cardhu_vdd_2v8_cam1 = NULL;
	}

	return -ENODEV;

}

static int cardhu_left_ov5650_power_off(void)
{
	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);
	}
	if (cardhu_1v8_cam1)
		regulator_disable(cardhu_1v8_cam1);
	if (cardhu_vdd_2v8_cam1)
		regulator_disable(cardhu_vdd_2v8_cam1);

	return 0;
}

struct ov5650_platform_data cardhu_left_ov5650_data = {
	.power_on = cardhu_left_ov5650_power_on,
	.power_off = cardhu_left_ov5650_power_off,
};

#ifdef CONFIG_VIDEO_OV14810
static int cardhu_ov14810_power_on(void)
{
	if (board_info.board_id == BOARD_E1198) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		mdelay(20);
		gpio_direction_output(OV14810_RESETN_GPIO, 0);
		mdelay(100);
		gpio_direction_output(OV14810_RESETN_GPIO, 1);
	}

	return 0;
}

static int cardhu_ov14810_power_off(void)
{
	if (board_info.board_id == BOARD_E1198) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);
	}

	return 0;
}

struct ov14810_platform_data cardhu_ov14810_data = {
	.power_on = cardhu_ov14810_power_on,
	.power_off = cardhu_ov14810_power_off,
};

struct ov14810_platform_data cardhu_ov14810uC_data = {
	.power_on = NULL,
	.power_off = NULL,
};

struct ov14810_platform_data cardhu_ov14810SlaveDev_data = {
	.power_on = NULL,
	.power_off = NULL,
};

static struct i2c_board_info cardhu_i2c_board_info_e1214[] = {
	{
		I2C_BOARD_INFO("ov14810", 0x36),
		.platform_data = &cardhu_ov14810_data,
	},
	{
		I2C_BOARD_INFO("ov14810uC", 0x67),
		.platform_data = &cardhu_ov14810uC_data,
	},
	{
		I2C_BOARD_INFO("ov14810SlaveDev", 0x69),
		.platform_data = &cardhu_ov14810SlaveDev_data,
	}
};
#endif

static int cardhu_right_ov5650_power_on(void)
{
	/* CSI-B and front sensor are muxed on cardhu */
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 0);

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {

		gpio_direction_output(CAM1_POWER_DWN_GPIO, 0);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 0);
		mdelay(10);

		if (cardhu_vdd_2v8_cam2 == NULL) {
			cardhu_vdd_2v8_cam2 = regulator_get(NULL, "vdd_2v8_cam2");
			if (WARN_ON(IS_ERR(cardhu_vdd_2v8_cam2))) {
				pr_err("%s: couldn't get regulator vdd_2v8_cam2: %ld\n",
					__func__, PTR_ERR(cardhu_vdd_2v8_cam2));
				goto reg_alloc_fail;
			}
		}
		regulator_enable(cardhu_vdd_2v8_cam2);
		mdelay(5);
	}

	/* Enable VDD_1V8_Cam2 */
	if (cardhu_1v8_cam2 == NULL) {
		cardhu_1v8_cam2 = regulator_get(NULL, "vdd_1v8_cam2");
		if (WARN_ON(IS_ERR(cardhu_1v8_cam2))) {
			pr_err("%s: couldn't get regulator vdd_1v8_cam2: %ld\n",
				__func__, PTR_ERR(cardhu_1v8_cam2));
			goto reg_alloc_fail;
		}
	}
	regulator_enable(cardhu_1v8_cam2);

	mdelay(5);

	if (board_info.board_id == BOARD_PM269) {
		gpio_direction_output(CAM2_RST_L_GPIO, 0);
		mdelay(100);
		gpio_direction_output(CAM2_RST_L_GPIO, 1);
	}

	return 0;

reg_alloc_fail:
	if (cardhu_1v8_cam2) {
		regulator_put(cardhu_1v8_cam2);
		cardhu_1v8_cam2 = NULL;
	}
	if (cardhu_vdd_2v8_cam2) {
		regulator_put(cardhu_vdd_2v8_cam2);
		cardhu_vdd_2v8_cam2 = NULL;
	}

	return -ENODEV;

}

static int cardhu_right_ov5650_power_off(void)
{
	/* CSI-B and front sensor are muxed on cardhu */
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 0);

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and do not have TCA6416 for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);
	}

	if (cardhu_1v8_cam2)
		regulator_disable(cardhu_1v8_cam2);
	if (cardhu_vdd_2v8_cam2)
		regulator_disable(cardhu_vdd_2v8_cam2);

	return 0;
}

static void cardhu_ov5650_synchronize_sensors(void)
{
	if (board_info.board_id == BOARD_E1198) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		mdelay(50);
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 0);
		mdelay(50);
	}
	else if (board_info.board_id == BOARD_E1291) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		mdelay(50);
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 0);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 0);
		mdelay(50);
	}
	else
		pr_err("%s: UnSupported BoardId\n", __func__);
}

struct ov5650_platform_data cardhu_right_ov5650_data = {
	.power_on = cardhu_right_ov5650_power_on,
	.power_off = cardhu_right_ov5650_power_off,
	.synchronize_sensors = cardhu_ov5650_synchronize_sensors,
};
#endif

#ifdef CONFIG_VIDEO_OV2710

static int cardhu_ov2710_power_on(void)
{
	/* CSI-B and front sensor are muxed on cardhu */
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 1);

	/* Enable VDD_1V8_Cam3 */
	if (cardhu_1v8_cam3 == NULL) {
		cardhu_1v8_cam3 = regulator_get(NULL, "vdd_1v8_cam3");
		if (WARN_ON(IS_ERR(cardhu_1v8_cam3))) {
			pr_err("%s: couldn't get regulator vdd_1v8_cam3: %ld\n",
				__func__, PTR_ERR(cardhu_1v8_cam3));
			goto reg_alloc_fail;
		}
	}
	regulator_enable(cardhu_1v8_cam3);

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and do not have TCA6416 for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		if (cardhu_vdd_cam3 == NULL) {
			cardhu_vdd_cam3 = regulator_get(NULL, "vdd_cam3");
			if (WARN_ON(IS_ERR(cardhu_vdd_cam3))) {
				pr_err("%s: couldn't get regulator vdd_cam3: %ld\n",
					__func__, PTR_ERR(cardhu_vdd_cam3));
				goto reg_alloc_fail;
			}
		}
		regulator_enable(cardhu_vdd_cam3);

		mdelay(5);

		gpio_direction_output(CAM1_POWER_DWN_GPIO, 0);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 0);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 0);
		mdelay(10);

	}

	mdelay(20);

	return 0;

reg_alloc_fail:
	if (cardhu_1v8_cam3) {
		regulator_put(cardhu_1v8_cam3);
		cardhu_1v8_cam3 = NULL;
	}
	if (cardhu_vdd_cam3) {
		regulator_put(cardhu_vdd_cam3);
		cardhu_vdd_cam3 = NULL;
	}

	return -ENODEV;
}

static int cardhu_ov2710_power_off(void)
{
	/* CSI-B and front sensor are muxed on cardhu */
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 1);

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);
		if (cardhu_vdd_cam3)
			regulator_disable(cardhu_vdd_cam3);
	}

	if (cardhu_1v8_cam3)
		regulator_disable(cardhu_1v8_cam3);

	return 0;
}

struct ov2710_platform_data cardhu_ov2710_data = {
	.power_on = cardhu_ov2710_power_on,
	.power_off = cardhu_ov2710_power_off,
};

static int cardhu_ov5640_power_on(void)
{
	/* CSI-B and front sensor are muxed on cardhu */
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 1);

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {

		gpio_direction_output(CAM1_POWER_DWN_GPIO, 0);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 0);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 0);
		mdelay(10);

		if (cardhu_vdd_cam3 == NULL) {
			cardhu_vdd_cam3 = regulator_get(NULL, "vdd_cam3");
			if (WARN_ON(IS_ERR(cardhu_vdd_cam3))) {
				pr_err("%s: couldn't get regulator vdd_cam3: %ld\n",
					__func__, PTR_ERR(cardhu_vdd_cam3));
				goto reg_alloc_fail;
			}
		}
		regulator_enable(cardhu_vdd_cam3);
	}

	/* Enable VDD_1V8_Cam3 */
	if (cardhu_1v8_cam3 == NULL) {
		cardhu_1v8_cam3 = regulator_get(NULL, "vdd_1v8_cam3");
		if (WARN_ON(IS_ERR(cardhu_1v8_cam3))) {
			pr_err("%s: couldn't get regulator vdd_1v8_cam3: %ld\n",
				__func__, PTR_ERR(cardhu_1v8_cam3));
			goto reg_alloc_fail;
		}
	}
	regulator_enable(cardhu_1v8_cam3);
	mdelay(5);

	return 0;

reg_alloc_fail:
	if (cardhu_1v8_cam3) {
		regulator_put(cardhu_1v8_cam3);
		cardhu_1v8_cam3 = NULL;
	}
	if (cardhu_vdd_cam3) {
		regulator_put(cardhu_vdd_cam3);
		cardhu_vdd_cam3 = NULL;
	}

	return -ENODEV;
}

static int cardhu_ov5640_power_off(void)
{
	/* CSI-B and front sensor are muxed on cardhu */
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 1);

	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		gpio_direction_output(CAM1_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM2_POWER_DWN_GPIO, 1);
		gpio_direction_output(CAM3_POWER_DWN_GPIO, 1);
	}

	if (cardhu_1v8_cam3)
		regulator_disable(cardhu_1v8_cam3);
	if (cardhu_vdd_cam3)
		regulator_disable(cardhu_vdd_cam3);

	return 0;
}

struct ov5640_platform_data cardhu_ov5640_data = {
	.power_on = cardhu_ov5640_power_on,
	.power_off = cardhu_ov5640_power_off,
};

static const struct i2c_board_info cardhu_i2c3_board_info[] = {
	{
		I2C_BOARD_INFO("pca9546", 0x70),
		.platform_data = &cardhu_pca954x_data,
	},
};


static struct nvc_gpio_pdata sh532u_gpio_pdata[] = {
	{ SH532U_GPIO_RESET, TEGRA_GPIO_PBB0, false, 0, },
};

static struct sh532u_platform_data sh532u_left_pdata = {
	.cfg		= NVC_CFG_NODEV,
	.num		= 1,
	.sync		= 2,
	.dev_name	= "focuser",
	.gpio_count	= ARRAY_SIZE(sh532u_gpio_pdata),
	.gpio		= sh532u_gpio_pdata,
};

static struct sh532u_platform_data sh532u_right_pdata = {
	.cfg		= NVC_CFG_NODEV,
	.num		= 2,
	.sync		= 1,
	.dev_name	= "focuser",
	.gpio_count	= ARRAY_SIZE(sh532u_gpio_pdata),
	.gpio		= sh532u_gpio_pdata,
};

static struct nvc_gpio_pdata pm269_sh532u_left_gpio_pdata[] = {
	{ SH532U_GPIO_RESET, CAM1_RST_L_GPIO, false, 0, },
};

static struct sh532u_platform_data pm269_sh532u_left_pdata = {
	.cfg		= 0,
	.num		= 1,
	.sync		= 2,
	.dev_name	= "focuser",
	.gpio_count	= ARRAY_SIZE(pm269_sh532u_left_gpio_pdata),
	.gpio		= pm269_sh532u_left_gpio_pdata,
};

static struct nvc_gpio_pdata pm269_sh532u_right_gpio_pdata[] = {
	{ SH532U_GPIO_RESET, CAM2_RST_L_GPIO, false, 0, },
};

static struct sh532u_platform_data pm269_sh532u_right_pdata = {
	.cfg		= 0,
	.num		= 2,
	.sync		= 1,
	.dev_name	= "focuser",
	.gpio_count	= ARRAY_SIZE(pm269_sh532u_right_gpio_pdata),
	.gpio		= pm269_sh532u_right_gpio_pdata,
};

static struct nvc_torch_pin_state cardhu_tps61050_pinstate = {
	.mask		= 0x0008, /*VGP3*/
	.values		= 0x0008,
};

static struct tps61050_platform_data cardhu_tps61050_pdata = {
	.dev_name	= "torch",
	.pinstate	= &cardhu_tps61050_pinstate,
};
#endif
#ifdef CONFIG_VIDEO_OV5650

static const struct i2c_board_info cardhu_i2c_board_info_tps61050[] = {
	{
		I2C_BOARD_INFO("tps61050", 0x33),
		.platform_data = &cardhu_tps61050_pdata,
	},
};

static struct i2c_board_info cardhu_i2c6_board_info[] = {
	{
		I2C_BOARD_INFO("ov5650L", 0x36),
		.platform_data = &cardhu_left_ov5650_data,
	},
	{
		I2C_BOARD_INFO("sh532u", 0x72),
		.platform_data = &sh532u_left_pdata,
	},
};

static struct i2c_board_info cardhu_i2c7_board_info[] = {
	{
		I2C_BOARD_INFO("ov5650R", 0x36),
		.platform_data = &cardhu_right_ov5650_data,
	},
	{
		I2C_BOARD_INFO("sh532u", 0x72),
		.platform_data = &sh532u_right_pdata,
	},
};

static struct i2c_board_info pm269_i2c6_board_info[] = {
	{
		I2C_BOARD_INFO("ov5650L", 0x36),
		.platform_data = &cardhu_left_ov5650_data,
	},
	{
		I2C_BOARD_INFO("sh532u", 0x72),
		.platform_data = &pm269_sh532u_left_pdata,
	},
};

static struct i2c_board_info pm269_i2c7_board_info[] = {
	{
		I2C_BOARD_INFO("ov5650R", 0x36),
		.platform_data = &cardhu_right_ov5650_data,
	},
	{
		I2C_BOARD_INFO("sh532u", 0x72),
		.platform_data = &pm269_sh532u_right_pdata,
	},
};
#endif

#ifdef CONFIG_VIDEO_OV2710
static struct i2c_board_info cardhu_i2c8_board_info[] = {
	{
		I2C_BOARD_INFO("ov2710", 0x36),
		.platform_data = &cardhu_ov2710_data,
	},
	{
		I2C_BOARD_INFO("ov5640", 0x3C),
		.platform_data = &cardhu_ov5640_data,
	},
};
#endif

static int nct_get_temp(void *_data, long *temp)
{
	struct nct1008_data *data = _data;
	return nct1008_thermal_get_temp(data, temp);
}

static int nct_get_temp_low(void *_data, long *temp)
{
	struct nct1008_data *data = _data;
	return nct1008_thermal_get_temp_low(data, temp);
}

static int nct_set_limits(void *_data,
			long lo_limit_milli,
			long hi_limit_milli)
{
	struct nct1008_data *data = _data;
	return nct1008_thermal_set_limits(data,
					lo_limit_milli,
					hi_limit_milli);
}

static int nct_set_alert(void *_data,
				void (*alert_func)(void *),
				void *alert_data)
{
	struct nct1008_data *data = _data;
	return nct1008_thermal_set_alert(data, alert_func, alert_data);
}

static int nct_set_shutdown_temp(void *_data, long shutdown_temp)
{
	struct nct1008_data *data = _data;
	return nct1008_thermal_set_shutdown_temp(data, shutdown_temp);
}

#ifdef CONFIG_TEGRA_SKIN_THROTTLE
static int nct_get_itemp(void *dev_data, long *temp)
{
	struct nct1008_data *data = dev_data;
	return nct1008_thermal_get_temps(data, NULL, temp);
}
#endif

static void nct1008_probe_callback(struct nct1008_data *data)
{
	struct tegra_thermal_device *ext_nct;

	ext_nct = kzalloc(sizeof(struct tegra_thermal_device),
					GFP_KERNEL);
	if (!ext_nct) {
		pr_err("unable to allocate thermal device\n");
		return;
	}

	ext_nct->name = "nct_ext";
	ext_nct->id = THERMAL_DEVICE_ID_NCT_EXT;
	ext_nct->data = data;
	ext_nct->offset = TDIODE_OFFSET;
	ext_nct->get_temp = nct_get_temp;
	ext_nct->get_temp_low = nct_get_temp_low;
	ext_nct->set_limits = nct_set_limits;
	ext_nct->set_alert = nct_set_alert;
	ext_nct->set_shutdown_temp = nct_set_shutdown_temp;

	tegra_thermal_device_register(ext_nct);

#ifdef CONFIG_TEGRA_SKIN_THROTTLE
	{
		struct tegra_thermal_device *int_nct;
		int_nct = kzalloc(sizeof(struct tegra_thermal_device),
						GFP_KERNEL);
		if (!int_nct) {
			kfree(int_nct);
			pr_err("unable to allocate thermal device\n");
			return;
		}

		int_nct->name = "nct_int";
		int_nct->id = THERMAL_DEVICE_ID_NCT_INT;
		int_nct->data = data;
		int_nct->get_temp = nct_get_itemp;

		tegra_thermal_device_register(int_nct);
	}
#endif
}

static struct nct1008_platform_data cardhu_nct1008_pdata = {
	.supported_hwrev = true,
	.ext_range = true,
	.conv_rate = 0x08,
	.offset = 8, /* 4 * 2C. Bug 844025 - 1C for device accuracies */
	.probe_callback = nct1008_probe_callback,
};

static struct i2c_board_info cardhu_i2c4_bq27510_board_info[] = {
	{
		I2C_BOARD_INFO("bq27510", 0x55),
	},
};

static struct i2c_board_info cardhu_i2c4_bq27541_board_info[] = {
	{
		I2C_BOARD_INFO("bq27541-battery", 0x55),
	},
};

static struct i2c_board_info grouper_i2c4_smb347_board_info[] = {
	{
		I2C_BOARD_INFO("smb347", 0x6a),
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

static struct i2c_board_info rear_sensor_i2c3_board_info[] = {  //ddebug
    {
        I2C_BOARD_INFO("fjm6mo", 0x1F),
        .platform_data = &yuv_rear_sensor_data,
    },
};

static struct i2c_board_info front_sensor_i2c2_board_info[] = {  //ddebug
	{
		I2C_BOARD_INFO("mi1040", 0x48),
		.platform_data = &yuv_front_sensor_data,
	},
};

static struct i2c_board_info iCatch7002a_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("i7002a", 0x3C),
		.platform_data = &iCatch7002a_data,
	},
};
#endif /* CONFIG_VIDEO_YUV */
static int cardhu_nct1008_init(void)
{
	int nct1008_port = -1;
	int ret = 0;
	u32 project_info = tegra3_get_project_id();

	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291) ||
		(board_info.board_id == BOARD_E1257) ||
		(board_info.board_id == BOARD_PM269) ||
		(board_info.board_id == BOARD_PM305) ||
		(board_info.board_id == BOARD_PM311)) {
		if(project_info == TEGRA3_PROJECT_ME301T || project_info == TEGRA3_PROJECT_ME301TL)
		{
			nct1008_port = TEGRA_GPIO_PS3;
		}
		else
		{
			nct1008_port = TEGRA_GPIO_PCC2;
		}
	} else if ((board_info.board_id == BOARD_E1186) ||
		(board_info.board_id == BOARD_E1187) ||
		(board_info.board_id == BOARD_E1256)) {
		/* FIXME: seems to be conflicting with usb3 vbus on E1186 */
		/* nct1008_port = TEGRA_GPIO_PH7; */
	}

	if (nct1008_port >= 0) {
		/* FIXME: enable irq when throttling is supported */
		cardhu_i2c4_nct1008_board_info[0].irq = TEGRA_GPIO_TO_IRQ(nct1008_port);

		ret = gpio_request(nct1008_port, "temp_alert");
		if (ret < 0)
			return ret;

		ret = gpio_direction_input(nct1008_port);
		if (ret < 0)
			gpio_free(nct1008_port);
	}

	return ret;
}

#if defined(CONFIG_GPIO_PCA953X)
static struct pca953x_platform_data cardhu_pmu_tca6416_data = {
	.gpio_base      = PMU_TCA6416_GPIO_BASE,
};

static const struct i2c_board_info cardhu_i2c4_board_info_tca6416[] = {
	{
		I2C_BOARD_INFO("tca6416", 0x20),
		.platform_data = &cardhu_pmu_tca6416_data,
	},
};

static struct pca953x_platform_data cardhu_cam_tca6416_data = {
	.gpio_base      = CAM_TCA6416_GPIO_BASE,
};

static const struct i2c_board_info cardhu_i2c2_board_info_tca6416[] = {
	{
		I2C_BOARD_INFO("tca6416", 0x20),
		.platform_data = &cardhu_cam_tca6416_data,
	},
};

static int __init pmu_tca6416_init(void)
{
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291))
			return 0;

	pr_info("Registering pmu pca6416\n");
	i2c_register_board_info(4, cardhu_i2c4_board_info_tca6416,
		ARRAY_SIZE(cardhu_i2c4_board_info_tca6416));
	return 0;
}

static int __init cam_tca6416_init(void)
{
	/* Boards E1198 and E1291 are of Cardhu personality
	 * and donot have TCA6416 exp for camera */
	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291))
		return 0;

	pr_info("Registering cam pca6416\n");
	i2c_register_board_info(2, cardhu_i2c2_board_info_tca6416,
		ARRAY_SIZE(cardhu_i2c2_board_info_tca6416));
	return 0;
}
#else
static int __init pmu_tca6416_init(void)
{
	return 0;
}

static int __init cam_tca6416_init(void)
{
	return 0;
}
#endif

static struct mpu_platform_data mpu3050_data = {
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
		I2C_BOARD_INFO(MPU3050_GYRO_NAME, MPU3050_GYRO_ADDR),
		.irq = TEGRA_GPIO_TO_IRQ(MPU_GYRO_IRQ_GPIO),
		.platform_data = &mpu3050_data,
	},
	{
		I2C_BOARD_INFO(MPU_ACCEL_NAME, MPU_ACCEL_ADDR),
#if	MPU_ACCEL_IRQ_GPIO
		.irq = TEGRA_GPIO_TO_IRQ(MPU_ACCEL_IRQ_GPIO),
#endif
		.platform_data = &mpu3050_accel_data,
	},
	{
		I2C_BOARD_INFO(MPU_COMPASS_NAME, MPU_COMPASS_ADDR),
#if	MPU_COMPASS_IRQ_GPIO
		.irq = TEGRA_GPIO_TO_IRQ(MPU_COMPASS_IRQ_GPIO),
#endif
		.platform_data = &mpu3050_compass_data,
	},
};


//CONFIG_MPU_SENSORS_MPU6050B1
static struct mpu_platform_data mpu6050_data = {
	.int_config	= 0x10,
	.level_shifter	= 0,
	.orientation	= TF300TG_GYRO_ORIENTATION,	// Located in board_[platformname].h
};

static struct ext_slave_platform_data mpu6050_compass_data = {
	.address	= MPU_COMPASS_ADDR,
	.irq		= 0,
	.adapt_num	= MPU_COMPASS_BUS_NUM,
	.bus		= EXT_SLAVE_BUS_PRIMARY,
	.orientation	= TF300TG_COMPASS_ORIENTATION,	// Located in board_[platformname].h
};

static struct i2c_board_info __initdata inv_mpu6050_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO(MPU6050_GYRO_NAME, MPU6050_TF500T_GYRO_ADDR),
		.irq = TEGRA_GPIO_TO_IRQ(MPU_GYRO_IRQ_GPIO),
		.platform_data = &mpu6050_data,
	},
	{
		I2C_BOARD_INFO(MPU_COMPASS_NAME, MPU_COMPASS_ADDR),
#if	0
		.irq = TEGRA_GPIO_TO_IRQ(MPU_COMPASS_IRQ_GPIO),
#endif
		.platform_data = &mpu6050_compass_data,
	},
};

//CONFIG_SENSORS_KXTJ9
static struct KXT_9_platform_data kxt_9_data = {
	.min_interval	= 1,
	.poll_interval	= 1000,

	.g_range	= KXT_9_G_2G,
	.shift_adj	= SHIFT_ADJ_2G,

	.axis_map_x	= 0,
	.axis_map_y	= 1,
	.axis_map_z	= 2,

	.negate_x	= 0,
	.negate_y	= 0,
	.negate_z	= 0,

	.data_odr_init		= ODR12_5F,
	.ctrl_reg1_init		= KXT_9_G_2G | RES_12BIT | DRDYE | TDTE | WUFE | TPE,
	.int_ctrl_init		= KXT_9_IEN | KXT_9_IEA | KXT_9_IEL,
	.tilt_timer_init	= 0x03,
	.engine_odr_init	= OTP12_5 | OWUF50 | OTDT400,
	.wuf_timer_init		= 0x16,
	.wuf_thresh_init	= 0x28,
	.tdt_timer_init		= 0x78,
	.tdt_h_thresh_init	= 0xFF,
	.tdt_l_thresh_init	= 0x14,
	.tdt_tap_timer_init	= 0x53,
	.tdt_total_timer_init	= 0x24,
	.tdt_latency_timer_init	= 0x10,
	.tdt_window_timer_init	= 0xA0,

	.gpio = TEGRA_GPIO_PO5,
};

static const struct i2c_board_info  kxt_9_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO(KIONIX_ACCEL_NAME, KIONIX_ACCEL_ADDR),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PO5),
		.platform_data = &kxt_9_data,
	},
};

/*Sensors orientation definition*/
struct mpu_orientation_def{
	__s8 gyro_orientation[9];
	__s8 accel_orientation[9];
	__s8 compass_orientation[9];
};

static void mpuirq6050_init(void)
{
	u32 project_info;
	pr_info("*** MPU6050 START *** cardhu_mpuirq_init...\n");

	project_info = tegra3_get_project_id();

	if (project_info == TEGRA3_PROJECT_TF500T)
	{
		/* Use "TF500T" to check the project name */
		struct mpu_orientation_def TF500T = {
			TF500T_GYRO_ORIENTATION,
			TF500T_GYRO_ORIENTATION,
			TF500T_COMPASS_ORIENTATION,
			};
		pr_info("initial mpu with TF500T config...\n");
		memcpy( mpu6050_data.orientation, TF500T.gyro_orientation, sizeof(mpu6050_data.orientation));
		memcpy( mpu6050_compass_data.orientation, TF500T.compass_orientation, sizeof(mpu6050_compass_data.orientation));
	}
	else
	{
		/* Use "ME301T and ME301TL" to check the project name */
		struct mpu_orientation_def ME301T = {
			ME301T_GYRO_ORIENTATION,
			ME301T_GYRO_ORIENTATION,
			ME301T_COMPASS_ORIENTATION,
			};
		pr_info("initial mpu with ME301T config...\n");
		memcpy( mpu6050_data.orientation, ME301T.gyro_orientation, sizeof(mpu6050_data.orientation));
		memcpy( mpu6050_compass_data.orientation, ME301T.compass_orientation, sizeof(mpu6050_compass_data.orientation));
	}

	pr_info("*** MPU6050 END *** mpuirq_init...\n");

	i2c_register_board_info(MPU_GYRO_BUS_NUM, inv_mpu6050_i2c2_board_info,
		ARRAY_SIZE(inv_mpu6050_i2c2_board_info));
}

static void mpuirq_init(void)
{
	u32 project_info;
	int ret = 0;
	pr_info("*** MPU START *** cardhu_mpuirq_init...\n");

	project_info = tegra3_get_project_id();

	if (project_info == TEGRA3_PROJECT_TF300T)
	{
		/* Use "TF300T" to check the project name */
		struct mpu_orientation_def TF300T = {
			TF300T_GYRO_ORIENTATION,
			TF300T_ACCEL_ORIENTATION,
			TF300T_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF300T config...\n");
		memcpy( mpu3050_data.orientation, TF300T.gyro_orientation, sizeof(mpu3050_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF300T.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF300T.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else if (project_info == TEGRA3_PROJECT_TF300TG)
	{
		/* Use "TF300TG" to check the project name */
		struct mpu_orientation_def TF300TG = {
			TF300TG_GYRO_ORIENTATION,
			TF300TG_ACCEL_ORIENTATION,
			TF300TG_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF300TG config...\n");
		memcpy( mpu3050_data.orientation, TF300TG.gyro_orientation, sizeof(mpu3050_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF300TG.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF300TG.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else if (project_info == TEGRA3_PROJECT_TF700T)
	{
		/* Use "TF700T" to check the project name */
		struct mpu_orientation_def TF700T = {
			TF700T_GYRO_ORIENTATION,
			TF700T_ACCEL_ORIENTATION,
			TF700T_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF700T config...\n");
		memcpy( mpu3050_data.orientation, TF700T.gyro_orientation, sizeof(mpu3050_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF700T.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF700T.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}
	else if (project_info == TEGRA3_PROJECT_TF300TL)
	{
		/* Use "TF300TL" to check the project name */
		struct mpu_orientation_def TF300TL = {
			TF300TL_GYRO_ORIENTATION,
			TF300TL_ACCEL_ORIENTATION,
			TF300TL_COMPASS_ORIENTATION,
			};

		pr_info("initial mpu with TF300TL config...\n");
		memcpy( mpu3050_data.orientation, TF300TL.gyro_orientation, sizeof(mpu3050_data.orientation));
		memcpy( mpu3050_accel_data.orientation, TF300TL.accel_orientation, sizeof(mpu3050_accel_data.orientation));
		memcpy( mpu3050_compass_data.orientation, TF300TL.compass_orientation, sizeof(mpu3050_compass_data.orientation));
	}

#if	MPU_ACCEL_IRQ_GPIO
	/* ACCEL-IRQ assignment */
	// tegra_gpio_enable(MPU_ACCEL_IRQ_GPIO);
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
    /*	tegra_gpio_enable(MPU_GYRO_IRQ_GPIO);
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
	}*/
	pr_info("*** MPU END *** mpuirq_init...\n");

	i2c_register_board_info(MPU_GYRO_BUS_NUM, inv_mpu_i2c2_board_info,
		ARRAY_SIZE(inv_mpu_i2c2_board_info));
}

static const struct i2c_board_info cardhu_i2c1_board_info_al3010[] = {
    {
        I2C_BOARD_INFO("al3010",0x1C),
        .irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PZ2),
    },
};

static void kxtj9_init(void)
{
	pr_info("*** kxtj9 START *** \n");
	int ret = 0;

	/* ACCEL-IRQ assignment */
	// tegra_gpio_enable(KIONIX_ACCEL_IRQ_GPIO);
	ret = gpio_request(KIONIX_ACCEL_IRQ_GPIO, KIONIX_ACCEL_NAME);
	if (ret < 0) {
		pr_err("%s: gpio_request failed %d\n", __func__, ret);
		return;
	}

	ret = gpio_direction_input(KIONIX_ACCEL_IRQ_GPIO);
	if (ret < 0) {
		pr_err("%s: gpio_direction_input failed %d\n", __func__, ret);
		gpio_free(KIONIX_ACCEL_IRQ_GPIO);
		return;
	}
	i2c_register_board_info(KIONIX_ACCEL_BUS_NUM, kxt_9_i2c2_board_info,
		ARRAY_SIZE(kxt_9_i2c2_board_info));
	pr_info("*** kxtj9 END *** \n");
}

int __init cardhu_sensors_init(void)
{
	int err;
	int ret = 0;
	u32 project_info = tegra3_get_project_id();

	tegra_get_board_info(&board_info);

	cardhu_camera_init();
	cam_tca6416_init();

    if(tegra3_get_project_id() != TEGRA3_PROJECT_P1801){
        i2c_register_board_info(2, cardhu_i2c1_board_info_al3010,
            ARRAY_SIZE(cardhu_i2c1_board_info_al3010));
	}

#ifdef CONFIG_I2C_MUX_PCA954x
	i2c_register_board_info(2, cardhu_i2c3_board_info,
		ARRAY_SIZE(cardhu_i2c3_board_info));

	i2c_register_board_info(2, cardhu_i2c_board_info_tps61050,
		ARRAY_SIZE(cardhu_i2c_board_info_tps61050));

//#ifdef CONFIG_VIDEO_OV14810
	/* This is disabled by default; To enable this change Kconfig;
	 * there should be some way to detect dynamically which board
	 * is connected (E1211/E1214), till that time sensor selection
	 * logic is static;
	 * e1214 corresponds to ov14810 sensor */
	i2c_register_board_info(2, cardhu_i2c_board_info_e1214,
		ARRAY_SIZE(cardhu_i2c_board_info_e1214));
//#else
	/* Left  camera is on PCA954x's I2C BUS0, Right camera is on BUS1 &
	 * Front camera is on BUS2 */
	if (board_info.board_id != BOARD_PM269) {
		i2c_register_board_info(PCA954x_I2C_BUS0,
					cardhu_i2c6_board_info,
					ARRAY_SIZE(cardhu_i2c6_board_info));

		i2c_register_board_info(PCA954x_I2C_BUS1,
					cardhu_i2c7_board_info,
					ARRAY_SIZE(cardhu_i2c7_board_info));
	} else {
		i2c_register_board_info(PCA954x_I2C_BUS0,
					pm269_i2c6_board_info,
					ARRAY_SIZE(pm269_i2c6_board_info));

		i2c_register_board_info(PCA954x_I2C_BUS1,
					pm269_i2c7_board_info,
					ARRAY_SIZE(pm269_i2c7_board_info));
	}
	i2c_register_board_info(PCA954x_I2C_BUS2, cardhu_i2c8_board_info,
		ARRAY_SIZE(cardhu_i2c8_board_info));
#endif
#ifdef CONFIG_VIDEO_YUV

//+ m6mo rear camera
    pr_info("fjm6mo i2c_register_board_info");
    i2c_register_board_info(2, rear_sensor_i2c3_board_info,
        ARRAY_SIZE(rear_sensor_i2c3_board_info));

/* Front Camera mi1040 + */
    pr_info("mi1040 i2c_register_board_info");
	i2c_register_board_info(2, front_sensor_i2c2_board_info,
		ARRAY_SIZE(front_sensor_i2c2_board_info));
/* Front Camera mi1040 - */

/* iCatch7002a + */
	pr_info("iCatch7002a i2c_register_board_info");
	i2c_register_board_info(2, iCatch7002a_i2c2_board_info,
		ARRAY_SIZE(iCatch7002a_i2c2_board_info));
/* iCatch7002a - */
#endif /* CONFIG_VIDEO_YUV */
	pmu_tca6416_init();

	if (board_info.board_id == BOARD_E1291)
		i2c_register_board_info(4, cardhu_i2c4_bq27510_board_info,
			ARRAY_SIZE(cardhu_i2c4_bq27510_board_info));

	if(TEGRA3_PROJECT_ME301T == project_info || TEGRA3_PROJECT_ME301TL == project_info || TEGRA3_PROJECT_ME570T == project_info)
	{
		ret = i2c_register_board_info(4, grouper_i2c4_smb347_board_info,
			ARRAY_SIZE(grouper_i2c4_smb347_board_info));
		printk("smb347 i2c_register_board_info, ret = %d\n", ret);

		ret = i2c_register_board_info(4, cardhu_i2c4_bq27541_board_info,
			ARRAY_SIZE(cardhu_i2c4_bq27541_board_info));
		printk("bq27510 i2c_register_board_info, ret = %d\n", ret);
	}
	else
	{
		i2c_register_board_info(4, cardhu_i2c4_pad_bat_board_info,
			ARRAY_SIZE(cardhu_i2c4_pad_bat_board_info));
	}

	err = cardhu_nct1008_init();
	if (err)
		return err;

	i2c_register_board_info(4, cardhu_i2c4_nct1008_board_info,
		ARRAY_SIZE(cardhu_i2c4_nct1008_board_info));

	if ((project_info == TEGRA3_PROJECT_TF500T) ||
		(project_info == TEGRA3_PROJECT_ME301T) ||
		(project_info == TEGRA3_PROJECT_ME301TL))
	{
		mpuirq6050_init();
	}
	else if (project_info == TEGRA3_PROJECT_P1801)
	{
		kxtj9_init();
	}
	else
	{
		mpuirq_init();
	}

	return 0;
}

#ifdef CONFIG_VIDEO_OV5650
struct ov5650_gpios {
	const char *name;
	int gpio;
	int enabled;
};

#define OV5650_GPIO(_name, _gpio, _enabled)		\
	{						\
		.name = _name,				\
		.gpio = _gpio,				\
		.enabled = _enabled,			\
	}

static struct ov5650_gpios ov5650_gpio_keys[] = {
	[0] = OV5650_GPIO("cam1_pwdn", CAM1_PWR_DN_GPIO, 0),
	[1] = OV5650_GPIO("cam1_rst_lo", CAM1_RST_L_GPIO, 1),
	[2] = OV5650_GPIO("cam1_af_pwdn_lo", CAM1_AF_PWR_DN_L_GPIO, 0),
	[3] = OV5650_GPIO("cam1_ldo_shdn_lo", CAM1_LDO_SHUTDN_L_GPIO, 1),
	[4] = OV5650_GPIO("cam2_pwdn", CAM2_PWR_DN_GPIO, 0),
	[5] = OV5650_GPIO("cam2_rst_lo", CAM2_RST_L_GPIO, 1),
	[6] = OV5650_GPIO("cam2_af_pwdn_lo", CAM2_AF_PWR_DN_L_GPIO, 0),
	[7] = OV5650_GPIO("cam2_ldo_shdn_lo", CAM2_LDO_SHUTDN_L_GPIO, 1),
	[8] = OV5650_GPIO("cam3_pwdn", CAM_FRONT_PWR_DN_GPIO, 0),
	[9] = OV5650_GPIO("cam3_rst_lo", CAM_FRONT_RST_L_GPIO, 1),
	[10] = OV5650_GPIO("cam3_af_pwdn_lo", CAM_FRONT_AF_PWR_DN_L_GPIO, 0),
	[11] = OV5650_GPIO("cam3_ldo_shdn_lo", CAM_FRONT_LDO_SHUTDN_L_GPIO, 1),
	[12] = OV5650_GPIO("cam_led_exp", CAM_FRONT_LED_EXP, 1),
	[13] = OV5650_GPIO("cam_led_rear_exp", CAM_SNN_LED_REAR_EXP, 1),
	[14] = OV5650_GPIO("cam_i2c_mux_rst", CAM_I2C_MUX_RST_EXP, 1),
};

int __init cardhu_ov5650_late_init(void)
{
	int ret;
	int i;

	if (!machine_is_cardhu())
		return 0;

	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291))
		return 0;

	printk("%s: \n", __func__);
	for (i = 0; i < ARRAY_SIZE(ov5650_gpio_keys); i++) {
		ret = gpio_request(ov5650_gpio_keys[i].gpio,
			ov5650_gpio_keys[i].name);
		if (ret < 0) {
			printk("%s: gpio_request failed for gpio #%d\n",
				__func__, i);
			goto fail;
		}
		printk("%s: enable - %d\n", __func__, i);
		gpio_direction_output(ov5650_gpio_keys[i].gpio,
			ov5650_gpio_keys[i].enabled);
		gpio_export(ov5650_gpio_keys[i].gpio, false);
	}

	return 0;

fail:
	while (i--)
		gpio_free(ov5650_gpio_keys[i].gpio);
	return ret;
}

late_initcall(cardhu_ov5650_late_init);
#endif
