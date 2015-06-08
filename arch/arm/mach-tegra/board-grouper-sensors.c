

/*
 * arch/arm/mach-tegra/board-grouper-sensors.c
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
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/mpu.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>
#include <mach/edp.h>
#ifdef CONFIG_VIDEO_MI1040
#include <media/yuv_sensor.h>
#endif
#include "board.h"
#include "board-grouper.h"
#include "cpu-tegra.h"
#include <linux/nct1008.h>
#include <linux/slab.h>
#include <mach/board-grouper-misc.h>
#include <mach/pinmux-tegra30.h>

#ifdef CONFIG_VIDEO_MI1040
#define CAM1_LDO_EN_GPIO		TEGRA_GPIO_PR6
#define FRONT_YUV_SENSOR_RST_GPIO	TEGRA_GPIO_PO0
#define FRONT_YUV_SENSOR_RST_GPIO_BACH	TEGRA_GPIO_PBB0

static int front_yuv_sensor_rst_gpio = FRONT_YUV_SENSOR_RST_GPIO;

static struct regulator *grouper_1v8_ldo5;

static unsigned int pmic_id;
#endif

static struct throttle_table tj_throttle_table[] = {
		/* CPU_THROT_LOW cannot be used by other than CPU */
		/* NO_CAP cannot be used by CPU */
		/*    CPU,    CBUS,    SCLK,     EMC */
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
	{ { CPU_THROT_LOW, 247000, 204000, 102000 } },
};

static struct balanced_throttle tj_throttle = {
	.throt_tab_size = ARRAY_SIZE(tj_throttle_table),
	.throt_tab = tj_throttle_table,
};

static struct i2c_board_info cardhu_i2c1_board_info_al3010[] = {
	{
		I2C_BOARD_INFO("al3010",0x1C),
	},
};

static struct i2c_board_info cap1106_i2c1_board_info[] = {
    {
        I2C_BOARD_INFO("cap1106",0x28),
    },
};

#ifdef CONFIG_VIDEO_MI1040
static int grouper_camera_init(void)
{
	if (grouper_get_project_id() == GROUPER_PROJECT_BACH)
		front_yuv_sensor_rst_gpio = FRONT_YUV_SENSOR_RST_GPIO_BACH;

	pmic_id = grouper_query_pmic_id();
	pr_info("%s: pmic_id= 0x%X", __func__, pmic_id);
	return 0;
}

static int yuv_front_sensor_power_on(void)
{
	int ret;
	pr_debug("%s+\n", __func__);

	if (!grouper_1v8_ldo5) {
		if (pmic_id == GROUPER_PMIC_MAXIM) {
			grouper_1v8_ldo5 = regulator_get(NULL, "vdd_sensor_1v8");
		} else if (pmic_id == GROUPER_PMIC_TI) {
			grouper_1v8_ldo5 = regulator_get(NULL, "avdd_vdac");
		}
		if (IS_ERR_OR_NULL(grouper_1v8_ldo5)) {
			if (grouper_1v8_ldo5) {
				regulator_put(grouper_1v8_ldo5);
			}
			grouper_1v8_ldo5 = NULL;
			pr_err("%s-: Can't get grouper_1v8_ldo5.\n", __func__);
			return -ENODEV;
		}
		regulator_set_voltage(grouper_1v8_ldo5, 1800000, 1800000);
		regulator_enable(grouper_1v8_ldo5);
	}

	msleep(10);

	/* AVDD_CAM1, 2.85V, controlled by CAM1_LDO_EN */
	pr_debug("gpio %d read as %d\n",CAM1_LDO_EN_GPIO, gpio_get_value(CAM1_LDO_EN_GPIO));
	ret = gpio_request(CAM1_LDO_EN_GPIO, "cam1_ldo_en");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s, ret= %d\n",
			__func__, "CAM1_LDO_EN_GPIO", ret);
	pr_debug("gpio %d: %d", CAM1_LDO_EN_GPIO, gpio_get_value(CAM1_LDO_EN_GPIO));
	gpio_set_value(CAM1_LDO_EN_GPIO, 1);
	gpio_direction_output(CAM1_LDO_EN_GPIO, 1);
	pr_debug("--> %d\n", gpio_get_value(CAM1_LDO_EN_GPIO));

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_NORMAL);

	msleep(10);

	/* yuv_sensor_rst_lo*/
	ret = gpio_request(front_yuv_sensor_rst_gpio, "yuv_sensor_rst_lo");

	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s, ret= %d\n",
			__func__, "FRONT_YUV_SENSOR_RST_GPIO", ret);
	pr_debug("gpio %d: %d", front_yuv_sensor_rst_gpio, gpio_get_value(front_yuv_sensor_rst_gpio));
	gpio_set_value(front_yuv_sensor_rst_gpio, 1);
	gpio_direction_output(front_yuv_sensor_rst_gpio, 1);
	pr_debug("--> %d\n", gpio_get_value(front_yuv_sensor_rst_gpio));

	pr_debug("%s-\n", __func__);
	return 0;
}

static int yuv_front_sensor_power_off(void)
{
	pr_debug("%s+\n", __func__);

	if ((pmic_id == GROUPER_PMIC_MAXIM) || (pmic_id == GROUPER_PMIC_TI)) {
		gpio_set_value(front_yuv_sensor_rst_gpio, 0);
		gpio_direction_output(front_yuv_sensor_rst_gpio, 0);
		gpio_free(front_yuv_sensor_rst_gpio);

		msleep(10);

		tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_TRISTATE);

		gpio_set_value(CAM1_LDO_EN_GPIO, 0);
		gpio_direction_output(CAM1_LDO_EN_GPIO, 0);
		gpio_free(CAM1_LDO_EN_GPIO);

		msleep(10);

		if (grouper_1v8_ldo5) {
			regulator_disable(grouper_1v8_ldo5);
			regulator_put(grouper_1v8_ldo5);
			grouper_1v8_ldo5 = NULL;
		}

		pr_debug("%s-\n", __func__);
		return 0;
	} else {
		pr_err("%s: unknow pmic_id: 0x%X\n", __func__, pmic_id);
		return -ENODEV;
	}
}

struct yuv_sensor_platform_data yuv_front_sensor_data = {
	.power_on = yuv_front_sensor_power_on,
	.power_off = yuv_front_sensor_power_off,
};

static struct i2c_board_info front_sensor_i2c2_board_info[] = {  //ddebug
	{
		I2C_BOARD_INFO("mi1040", 0x48),
		.platform_data = &yuv_front_sensor_data,
	},
};
#endif

/* MPU board file definition	*/
#if (MPU_GYRO_TYPE == MPU_TYPE_MPU3050)
#define MPU_GYRO_NAME		"mpu3050"
static struct mpu_platform_data mpu_gyro_data = {
	.int_config  = 0x10,
	.level_shifter = 0,
	.orientation = MPU_GYRO_ORIENTATION,	/* Located in board_[platformname].h	*/
	.sec_slave_type = SECONDARY_SLAVE_TYPE_ACCEL,
	.sec_slave_id   = ACCEL_ID_BMA250,
	.secondary_i2c_addr = MPU_ACCEL_ADDR,
	.secondary_orientation = MPU_ACCEL_ORIENTATION,	/* Located in board_[platformname].h	*/
	.key = {221, 22, 205, 7,   217, 186, 151, 55,
		206, 254, 35, 144, 225, 102,  47, 50},
};
#endif
#if (MPU_GYRO_TYPE == MPU_TYPE_MPU6050)
#define MPU_GYRO_NAME		"mpu6050"
static struct mpu_platform_data mpu_gyro_data = {
        .int_config  = 0x10,
        .level_shifter = 0,
        .orientation = MPU_GYRO_ORIENTATION,	/* Located in board_[platformname].h	*/
        .sec_slave_type = SECONDARY_SLAVE_TYPE_NONE,
        .key = {221, 22, 205, 7,   217, 186, 151, 55,
                206, 254, 35, 144, 225, 102,  47, 50},
};
#endif
static struct mpu_platform_data mpu_compass_data = {
	.orientation = MPU_COMPASS_ORIENTATION,	/* Located in board_[platformname].h	*/
};

static struct i2c_board_info __initdata inv_mpu_i2c2_board_info[] = {
        {
                I2C_BOARD_INFO(MPU_GYRO_NAME, MPU_GYRO_ADDR),
                .platform_data = &mpu_gyro_data,
        },
        {
                I2C_BOARD_INFO(MPU_COMPASS_NAME, MPU_COMPASS_ADDR),
                .platform_data = &mpu_compass_data,
        },
};

static void mpuirq_init(void)
{
	int ret = 0;

	pr_debug("*** MPU START *** mpuirq_init...\n");

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
	pr_debug("*** MPU END *** mpuirq_init...\n");

	inv_mpu_i2c2_board_info[0].irq = gpio_to_irq(MPU_GYRO_IRQ_GPIO);
	i2c_register_board_info(MPU_GYRO_BUS_NUM, inv_mpu_i2c2_board_info,
		ARRAY_SIZE(inv_mpu_i2c2_board_info));
}

static int __init grouper_throttle_init(void)
{
	if (machine_is_grouper())
		balanced_throttle_register(&tj_throttle, "tegra-balanced");
	return 0;
 }
module_init(grouper_throttle_init);

static struct nct1008_platform_data grouper_nct1008_pdata = {
	.supported_hwrev = true,
	.ext_range = true,
	.conv_rate = 0x08,
	.offset = 8, /* 4 * 2C. Bug 844025 - 1C for device accuracies */

	.shutdown_ext_limit = 90, /* C */
	.shutdown_local_limit = 100, /* C */

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

static struct i2c_board_info grouper_i2c4_nct1008_board_info[] = {
	{
		I2C_BOARD_INFO("nct1008", 0x4C),
		.platform_data = &grouper_nct1008_pdata,
		.irq = -1,
	}
};

static int grouper_nct1008_init(void)
{
	int nct1008_port = -1;
	int ret = 0;

	nct1008_port = TEGRA_GPIO_PS3;
	if (nct1008_port >= 0) {
		/* FIXME: enable irq when throttling is supported */
		grouper_i2c4_nct1008_board_info[0].irq = gpio_to_irq(nct1008_port);

		ret = gpio_request(nct1008_port, "temp_alert");
		if (ret < 0)
			return ret;

		ret = gpio_direction_input(nct1008_port);
		if (ret < 0)
			gpio_free(nct1008_port);
	}

	tegra_platform_edp_init(grouper_nct1008_pdata.trips,
				&grouper_nct1008_pdata.num_trips,
				0); /* edp temperature margin */

	return ret;
}

int __init grouper_sensors_init(void)
{
	int err;
#ifdef CONFIG_VIDEO_MI1040
	grouper_camera_init();

	/* Front Camera mi1040 + */
    pr_debug("%s: mi1040 i2c_register_board_info", __func__);
	i2c_register_board_info(2, front_sensor_i2c2_board_info,
		ARRAY_SIZE(front_sensor_i2c2_board_info));
#endif

	err = grouper_nct1008_init();
	if (err)
		pr_err("%s: Thermal: Configure GPIO_PCC2 as an irq fail!", __func__);
	i2c_register_board_info(4, grouper_i2c4_nct1008_board_info,
		ARRAY_SIZE(grouper_i2c4_nct1008_board_info));

	mpuirq_init();

	cardhu_i2c1_board_info_al3010[0].irq = gpio_to_irq(TEGRA_GPIO_PZ2);
	i2c_register_board_info(2, cardhu_i2c1_board_info_al3010,
		ARRAY_SIZE(cardhu_i2c1_board_info_al3010));

    if (GROUPER_PROJECT_BACH == grouper_get_project_id()) {
        cap1106_i2c1_board_info[0].irq = gpio_to_irq(TEGRA_GPIO_PR3);
        i2c_register_board_info(2, cap1106_i2c1_board_info,
                ARRAY_SIZE(cap1106_i2c1_board_info));
    }

	return 0;
}
