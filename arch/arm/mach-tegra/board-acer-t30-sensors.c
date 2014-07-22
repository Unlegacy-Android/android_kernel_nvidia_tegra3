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
#include <linux/mpu.h>
#include <media/sh532u.h>
#include <media/ad5816.h>
#include <mach/gpio.h>
#include <mach/edp.h>
#include <mach/thermal.h>
#include <linux/therm_est.h>

#include "gpio-names.h"
#include "board-acer-t30.h"
#include "cpu-tegra.h"

#ifdef CONFIG_STK2203_LIGHT_SENSOR
#define STK_INTR TEGRA_GPIO_PX3
#endif

#include <media/ov5640.h>
#include <media/ov9740.h>
#include <media/yuv_sensor.h>

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
		tegra_gpio_enable(camera_gpio_table[i].gpio);
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

#if defined(CONFIG_VIDEO_OV5640)
static int cardhu_ov5640_power_on(void)
{
	pr_info("%s\n", __func__);

	gpio_direction_output(OV5640_CAM_PWDN, 0);
	msleep(20);

	return 0;
}

static int cardhu_ov5640_power_off(void)
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
#if defined(CONFIG_VIDEO_OV5640)
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
static const struct i2c_board_info cardhu_i2c0_stk2203_board_info[] = {
	{
		I2C_BOARD_INFO("stk_als", 0x10),
		.irq = TEGRA_GPIO_TO_IRQ(STK_INTR),
	},
};

static void cardhu_stk2203_init(void)
{
	int ret;

	ret = gpio_request(STK_INTR, "stk_als");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "TEGRA_GPIO_PX3");
	tegra_gpio_enable(STK_INTR);

	ret = gpio_direction_input(STK_INTR);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "TEGRA_GPIO_PX3");

	i2c_register_board_info(0, cardhu_i2c0_stk2203_board_info,
		ARRAY_SIZE(cardhu_i2c0_stk2203_board_info));
}
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
	.shutdown_ext_limit = 90,
	.shutdown_local_limit = 90,
	.throttling_ext_limit = 85,
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
		/* FIXME: enable irq when throttling is supported */
		cardhu_i2c4_nct1008_board_info[0].irq = TEGRA_GPIO_TO_IRQ(nct1008_port);

		ret = gpio_request(nct1008_port, "temp_alert");
		if (ret < 0)
			return ret;

		ret = gpio_direction_input(nct1008_port);
		if (ret < 0)
			gpio_free(nct1008_port);
		else
			tegra_gpio_enable(nct1008_port);
	}

	return ret;
}

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
	.irq         = TEGRA_GPIO_TO_IRQ(G_ACC_INT),
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
	.irq         = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PX7),
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
		.irq = TEGRA_GPIO_TO_IRQ(GYRO_INT_R),
		.platform_data = &mpu_data,
	},
};

static void cardhu_mpu_power_on(void)
{
	int ret;

	tegra_gpio_enable(SENSOR_3V3_2);//3.3
	ret = gpio_request(SENSOR_3V3_2, "sensor_vdd_power_en");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "EN_SENSOR_VDD_GPIO");
	ret = gpio_direction_output(SENSOR_3V3_2, 1);
	if (ret < 0)
		pr_err("%s: gpio_direction_output failed for gpio %s\n",
		__func__, "EN_SENSOR_VDD");
	mdelay(5);
	tegra_gpio_enable(EN_SENSOR_VLOGIC);//1.8
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

	tegra_gpio_enable(GYRO_INT_R);
	ret = gpio_request(GYRO_INT_R, SENSOR_MPU_NAME);
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "GYRO_INT_R");
	ret = gpio_direction_input(GYRO_INT_R);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "GYRO_INT_R");

	tegra_gpio_enable(G_ACC_INT);
	ret = gpio_request(G_ACC_INT, "MPU_KXTF9");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s\n",
		__func__, "G_ACC_INT");
	ret = gpio_direction_input(G_ACC_INT);
	if (ret < 0)
		pr_err("%s: gpio_direction_input failed for gpio %s\n",
		__func__, "G_ACC_INT");

	tegra_gpio_enable(COMPASS_DRDY);
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

	i2c_register_board_info(4, cardhu_i2c4_nct1008_board_info,
		ARRAY_SIZE(cardhu_i2c4_nct1008_board_info));

#ifdef CONFIG_MPU_SENSORS_MPU3050
	i2c_register_board_info(0, mpu3050_i2c0_boardinfo,
		ARRAY_SIZE(mpu3050_i2c0_boardinfo));

	cardhu_mpuirq_init();
#endif
#ifdef CONFIG_STK2203_LIGHT_SENSOR
        cardhu_stk2203_init();
#endif
	return 0;
}
