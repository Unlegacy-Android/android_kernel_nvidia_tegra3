/*
 * arch/arm/mach-tegra/board-acer-t30-sensors.c
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
#include <linux/nct1008.h>
#include <mach/fb.h>
#include <mach/gpio.h>
#include <media/ov5650.h>
#include <media/ov14810.h>
#include <media/ov2710.h>
#include <media/tps61050.h>
#include <generated/mach-types.h>
#include "gpio-names.h"
#include "board.h"
#include <media/sh532u.h>
#include <media/ad5816.h>
#include <mach/edp.h>
#include <linux/therm_est.h>
#include <linux/gpio.h>

#include "gpio-names.h"
#include "board-acer-t30.h"
#include "cpu-tegra.h"
#include "devices.h"

#ifdef CONFIG_MPU_SENSORS_MPU3050
#include <linux/mpu_3050.h>
#endif

#ifdef CONFIG_STK2203_LIGHT_SENSOR
#define STK_INTR TEGRA_GPIO_PX3
#endif

#ifdef CONFIG_VIDEO_OV5640_ACER
#include <media/ov5640.h>
#endif
#ifdef CONFIG_VIDEO_OV9740
#include <media/ov9740.h>
#endif
#ifdef CONFIG_VIDEO_YUV
#include <media/yuv_sensor.h>
#endif

static struct board_info board_info;

struct camera_gpio {
	int gpio;
	const char *name;
	int init;
};

#define CAMERA_GPIO(_gpio, _name, _init)  \
	{                                 \
		.gpio = _gpio,            \
		.name = _name,            \
		.init = _init,            \
	}

static struct camera_gpio camera_gpio_table[] = {
	[0] = CAMERA_GPIO(TEGRA_GPIO_PR2,  "en_cam_1v8",  1),
	[1] = CAMERA_GPIO(TEGRA_GPIO_PQ3,  "en_cam_2v8",  0),
	[2] = CAMERA_GPIO(TEGRA_GPIO_PBB3, "5m_cam_pwdn", 1),
	[3] = CAMERA_GPIO(TEGRA_GPIO_PBB4, "5m_cam_rst",  0),
	[4] = CAMERA_GPIO(TEGRA_GPIO_PBB5, "2m_cam_pwdn", 1),
	[5] = CAMERA_GPIO(TEGRA_GPIO_PBB0, "2m_cam_rst",  0),
};

#define EN_CAM_1V8        camera_gpio_table[0].gpio  // EN_CAM_1V8#
#define EN_CAM_2V8        camera_gpio_table[1].gpio  // EN_CAM_2V8

#define OV5640_CAM_PWDN   camera_gpio_table[2].gpio  // 5M_CAM_PWDN
#define OV5640_CAM_RST    camera_gpio_table[3].gpio  // 5M_CAM_RST#

#define OV9740_CAM_PWDN   camera_gpio_table[4].gpio  // 2M_CAM_PWDN
#define OV9740_CAM_RST    camera_gpio_table[5].gpio  // 2M_CAM_RST#

#define MT9D115_CAM_PWDN  camera_gpio_table[2].gpio  // 2M_CAM_PWDN
#define MT9D115_CAM_RST   camera_gpio_table[3].gpio  // 2M_CAM_RST#

static int cardhu_camera_init(void)
{
	int i, ret;

	pr_info("%s\n", __func__);

	// initialize camera GPIOs
	for (i=0; i<ARRAY_SIZE(camera_gpio_table); i++) {
		ret = gpio_request(camera_gpio_table[i].gpio, camera_gpio_table[i].name);
		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, camera_gpio_table[i].name);
			goto fail;
		}
		gpio_direction_output(camera_gpio_table[i].gpio, camera_gpio_table[i].init);
	}

	// turn on camera power
	gpio_direction_output(EN_CAM_1V8, 0);
	msleep(3);
	gpio_direction_output(EN_CAM_2V8, 1);
	msleep(5);

	// do OV5640 hardware reset and enter hardware standby mode
	gpio_direction_output(OV5640_CAM_PWDN, 0);
	msleep(1);
	gpio_direction_output(OV5640_CAM_RST,  1);
	msleep(20);
	gpio_direction_output(OV5640_CAM_PWDN, 1);

	// do OV9740 hardware reset and enter power down mode
	gpio_direction_output(OV9740_CAM_PWDN, 0);
	msleep(1);
	gpio_direction_output(OV9740_CAM_RST,  1);
	msleep(20);
	gpio_direction_output(OV9740_CAM_PWDN, 1);

	return 0;

fail:
	while (i>=0) {
		gpio_free(camera_gpio_table[i].gpio);
		i--;
	}

	return ret;
}

#if defined(CONFIG_VIDEO_OV5640_ACER)
static int cardhu_ov5640_power_on(struct device *dev)
{
	pr_info("%s\n", __func__);

	gpio_direction_output(OV5640_CAM_PWDN, 0);
	msleep(20);

	return 0;
}

static int cardhu_ov5640_power_off(struct device *dev)
{
	pr_info("%s\n", __func__);

	gpio_direction_output(OV5640_CAM_PWDN, 1);

	return 0;
}

static struct ov5640_platform_data cardhu_ov5640_data = {
	.power_on = cardhu_ov5640_power_on,
	.power_off = cardhu_ov5640_power_off,
};
#endif

#if defined(CONFIG_VIDEO_OV9740)
static int cardhu_ov9740_power_on(void)
{
	pr_info("%s\n", __func__);

	gpio_direction_output(OV9740_CAM_PWDN, 0);
	msleep(20);

	return 0;
}

static int cardhu_ov9740_power_off(void)
{
	pr_info("%s\n", __func__);

	gpio_direction_output(OV9740_CAM_PWDN, 1);

	return 0;
}

static struct ov9740_platform_data cardhu_ov9740_data = {
	.power_on = cardhu_ov9740_power_on,
	.power_off = cardhu_ov9740_power_off,
};
#endif

#ifdef CONFIG_VIDEO_YUV
static int cardhu_mt9d115_power_on(void)
{
	pr_info("%s\n", __func__);

	// do MT9D115 hardware reset and enter hardware standby mode
	gpio_direction_output(MT9D115_CAM_RST,  0);
	msleep(1);
	gpio_direction_output(MT9D115_CAM_RST,  1);
	msleep(1);

	gpio_direction_output(MT9D115_CAM_PWDN, 0);
	// TODO: fine tune the delay time
	msleep(20);

	return 0;
}

static int cardhu_mt9d115_power_off(void)
{
	pr_info("%s\n", __func__);

	gpio_direction_output(MT9D115_CAM_PWDN, 1);
	// standby time need 2 frames
	msleep(150);

	return 0;
}

struct yuv_sensor_platform_data cardhu_mt9d115_data = {
	.power_on = cardhu_mt9d115_power_on,
	.power_off = cardhu_mt9d115_power_off,
};
#endif  // CONFIG_VIDEO_YUV

static const struct i2c_board_info cardhu_camera_i2c3_board_info[] = {
#if defined(CONFIG_VIDEO_OV5640_ACER)
	{
		I2C_BOARD_INFO("ov5640", 0x3C),
		.platform_data = &cardhu_ov5640_data,
	},
#endif
#if defined(CONFIG_VIDEO_OV9740)
	{
		I2C_BOARD_INFO("ov9740", 0x10),
		.platform_data = &cardhu_ov9740_data,
	},
#endif
#ifdef CONFIG_VIDEO_YUV
	{
		I2C_BOARD_INFO("mt9d115", 0x3C),
		.platform_data = &cardhu_mt9d115_data,
	},
#endif
};

#ifdef CONFIG_STK2203_LIGHT_SENSOR
static struct i2c_board_info cardhu_i2c0_stk2203_board_info[] = {
	{
		I2C_BOARD_INFO("stk_als", 0x10),
	},
};

static void cardhu_stk2203_init(void)
{
	int ret;

	ret = gpio_request(STK_INTR, "stk_als");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "TEGRA_GPIO_PX3");

	ret = gpio_direction_input(STK_INTR);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "TEGRA_GPIO_PX3");

	cardhu_i2c0_stk2203_board_info[0].irq = gpio_to_irq(STK_INTR);
	i2c_register_board_info(0, cardhu_i2c0_stk2203_board_info,
		ARRAY_SIZE(cardhu_i2c0_stk2203_board_info));
}
#endif

static struct throttle_table tj_throttle_table[] = {
		/* CPU_THROT_LOW cannot be used by other than CPU */
		/* NO_CAP cannot be used by CPU */
		/* CPU, CBUS, SCLK, EMC */
		{ { 1000000, NO_CAP, NO_CAP, NO_CAP } },
		{ { 760000, NO_CAP, NO_CAP, NO_CAP } },
		{ { 760000, NO_CAP, NO_CAP, NO_CAP } },
		{ { 620000, NO_CAP, NO_CAP, NO_CAP } },
		{ { 620000, NO_CAP, NO_CAP, NO_CAP } },
		{ { 620000, 437000, NO_CAP, NO_CAP } },
		{ { 620000, 352000, NO_CAP, NO_CAP } },
		{ { 475000, 352000, NO_CAP, NO_CAP } },
		{ { 475000, 352000, NO_CAP, NO_CAP } },
		{ { 475000, 352000, 250000, 375000 } },
		{ { 475000, 352000, 250000, 375000 } },
		{ { 475000, 247000, 204000, 375000 } },
		{ { 475000, 247000, 204000, 204000 } },
		{ { 475000, 247000, 204000, 204000 } },
{ { CPU_THROT_LOW, 247000, 204000, 102000 } },
};

static struct balanced_throttle tj_throttle = {
	.throt_tab_size = ARRAY_SIZE(tj_throttle_table),
	.throt_tab = tj_throttle_table,
};

static int __init cardhu_throttle_init(void)
{
	//if (machine_is_cardhu())
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

static struct i2c_board_info cardhu_i2c4_nct1008_board_info[] = {
	{
		I2C_BOARD_INFO("nct1008", 0x4C),
		.platform_data = &cardhu_nct1008_pdata,
		.irq = -1,
	}
};

static int cardhu_nct1008_init(void)
{
	int nct1008_port = -1;
	int ret = 0;

	nct1008_port = TEGRA_GPIO_PI3;

	if (nct1008_port >= 0) {
		tegra_platform_edp_init(cardhu_nct1008_pdata.trips,
								&cardhu_nct1008_pdata.num_trips,
								0); /* edp temperature margin */
		cardhu_i2c4_nct1008_board_info[0].irq =
									gpio_to_irq(nct1008_port);

		ret = gpio_request(nct1008_port, "temp_alert");
		if (ret < 0)
			return ret;

		ret = gpio_direction_input(nct1008_port);
		if (ret < 0)
			gpio_free(nct1008_port);
	}
	
	i2c_register_board_info(4, cardhu_i2c4_nct1008_board_info,
			ARRAY_SIZE(cardhu_i2c4_nct1008_board_info));

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

static struct balanced_throttle skin_throttle = {
	.throt_tab_size = 10,
	.throt_tab = {
	{ 0, 1000 },
	{ 640000, 1000 },
	{ 640000, 1000 },
	{ 640000, 1000 },
	{ 640000, 1000 },
	{ 640000, 1000 },
	{ 760000, 1000 },
	{ 760000, 1050 },
	{1000000, 1050 },
	{1000000, 1100 },
	},
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


#ifdef CONFIG_MPU_SENSORS_MPU3050
#define SENSOR_MPU_NAME "mpu3050"
static struct mpu_platform_data mpu_data = {
	.int_config  = 0x10,
	.orientation = {
		 0, -1,  0,
		-1,  0,  0,
		 0,  0, -1
	},
	/* accel */
	.accel = {
#ifdef CONFIG_INV_SENSORS_MODULE
	.get_slave_descr = NULL,
#else
	.get_slave_descr = get_accel_slave_descr,
#endif
	.adapt_num   = 0,
	.bus         = EXT_SLAVE_BUS_SECONDARY,
	.address     = 0x0F,
	.orientation = {
		 0, -1,  0,
		-1,  0,  0,
		 0,  0, -1
	},
	},
	/* compass */
	.compass = {
#ifdef CONFIG_INV_SENSORS_MODULE
	.get_slave_descr = NULL,
#else
	.get_slave_descr = get_compass_slave_descr,
#endif
	.adapt_num   = 0,
	.bus         = EXT_SLAVE_BUS_PRIMARY,
	.address     = 0x0C,
	.orientation = {
		1,  0,  0,
		0, -1,  0,
		0,  0, -1
	},
	},
};

static struct i2c_board_info __initdata mpu3050_i2c0_boardinfo[] = {
	{
		I2C_BOARD_INFO(SENSOR_MPU_NAME, 0x68),
		.platform_data = &mpu_data,
	},
};

static void cardhu_mpu_power_on(void)
{
	int ret;

	ret = gpio_request(SENSOR_3V3_2, "sensor_vdd_power_en");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "EN_SENSOR_VDD_GPIO");
	ret = gpio_direction_output(SENSOR_3V3_2, 1);
	if (ret < 0)
		pr_err("%s: gpio_direction_output failed for gpio %s\n",
		__func__, "EN_SENSOR_VDD");
	mdelay(5);
	ret = gpio_request(EN_SENSOR_VLOGIC, "sensor_vlogic_power_en");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "EN_SENSOR_VLOGIC_GPIO");
	ret = gpio_direction_output(EN_SENSOR_VLOGIC, 0);
	if (ret < 0)
		pr_err("%s: gpio_direction_output failed for gpio %s\n",
		__func__, "EN_SENSOR_VLOGIC");
	}

static void cardhu_mpuirq_init(void)
	{
	int ret;

	pr_info("*** MPU START *** cardhu_mpuirq_init...\n");

	ret = gpio_request(GYRO_INT_R, SENSOR_MPU_NAME);
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "GYRO_INT_R");
	ret = gpio_direction_input(GYRO_INT_R);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "GYRO_INT_R");

	ret = gpio_request(G_ACC_INT, "MPU_KXTF9");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "G_ACC_INT");
	ret = gpio_direction_input(G_ACC_INT);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "G_ACC_INT");

	ret = gpio_request(COMPASS_DRDY, "MPU_AKM8975");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "COMPASS_DRDY");
	ret = gpio_direction_input(COMPASS_DRDY);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "COMPASS_DRDY");

	pr_info("*** MPU END *** cardhu_mpuirq_init...\n");

	cardhu_mpu_power_on();
}
#endif

int __init cardhu_sensors_init(void)
{
	int err;

	tegra_get_board_info(&board_info);

	cardhu_camera_init();
	i2c_register_board_info(2, cardhu_camera_i2c3_board_info,
		ARRAY_SIZE(cardhu_camera_i2c3_board_info));

	err = cardhu_nct1008_init();
	if (err)
		return err;

#ifdef CONFIG_MPU_SENSORS_MPU3050
	mpu_data.accel.irq = gpio_to_irq(G_ACC_INT);
	mpu_data.compass.irq = gpio_to_irq(TEGRA_GPIO_PX7),
	mpu3050_i2c0_boardinfo[0].irq = gpio_to_irq(GYRO_INT_R);
	i2c_register_board_info(0, mpu3050_i2c0_boardinfo,
		ARRAY_SIZE(mpu3050_i2c0_boardinfo));

	cardhu_mpuirq_init();
#endif
#ifdef CONFIG_STK2203_LIGHT_SENSOR
        cardhu_stk2203_init();
#endif
	return 0;
}
