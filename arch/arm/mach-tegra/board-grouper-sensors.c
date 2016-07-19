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
#include <linux/input/cap1106.h>
#include <linux/input/lid.h>
#include <linux/i2c.h>
#include <linux/nct1008.h>
#include <linux/mpu.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <mach/edp.h>
#include <asm/mach-types.h>

#ifdef CONFIG_VIDEO_MI1040
#include <media/yuv_sensor.h>
#endif
#include "board-grouper.h"
#include "cpu-tegra.h"

#include <mach/board-grouper-misc.h>
#include <mach/pinmux-tegra30.h>

static int mi1040_rst_gpio = CAM2_RST_GPIO;

static struct regulator *grouper_1v8_ldo5;

static unsigned int pmic_id;

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
	  { { CPU_THROT_LOW,  247000,  204000,  102000 } },
};

static struct balanced_throttle tj_throttle = {
	.throt_tab_size = ARRAY_SIZE(tj_throttle_table),
	.throt_tab = tj_throttle_table,
};

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
	.conv_rate = 0x08, /* 0x08 corresponds to 63Hz conversion rate */
	.offset = 8, /* 4 * 2C. 1C for device accuracies */

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
	int ret = 0;

	/* FIXME: enable irq when throttling is supported */
	grouper_i2c4_nct1008_board_info[0].irq =
				gpio_to_irq(GROUPER_TEMP_ALERT_GPIO);

	ret = gpio_request(GROUPER_TEMP_ALERT_GPIO, "temp_alert");
	if (ret < 0) {
		pr_err("%s: gpio_request failed\n", __func__);
		return ret;
	}

	ret = gpio_direction_input(GROUPER_TEMP_ALERT_GPIO);
	if (ret < 0) {
		pr_err("%s: set gpio to input failed\n", __func__);
		gpio_free(GROUPER_TEMP_ALERT_GPIO);
	}

	tegra_platform_edp_init(grouper_nct1008_pdata.trips,
				&grouper_nct1008_pdata.num_trips,
				0); /* edp temperature margin */

	return ret;
}

static struct i2c_board_info grouper_i2c1_al3010_board_info[] = {
	{
		I2C_BOARD_INFO("al3010", 0x1C),
	},
};

static int grouper_camera_init(void)
{
	int ret;

	if (grouper_get_project_id() == GROUPER_PROJECT_BACH)
		mi1040_rst_gpio = CAM2_RST_GPIO_BACH;

	pmic_id = grouper_query_pmic_id();

	ret = gpio_request(mi1040_rst_gpio, "yuv_sensor_rst_lo");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s, ret= %d\n",
			__func__, "CAM2_RST_GPIO", ret);

	/* AVDD_CAM1, 2.85V, controlled by CAM1_LDO_EN */
	ret = gpio_request(CAM2_LDO_EN_GPIO, "cam1_ldo_en");
	if (ret < 0)
		pr_err("%s: gpio_request failed for gpio %s, ret= %d\n",
			__func__, "CAM2_LDO_EN_GPIO", ret);

	return 0;
}

static int grouper_mi1040_power_on(void)
{
	if (grouper_1v8_ldo5 == NULL) {
		if (pmic_id == GROUPER_PMIC_MAXIM)
			grouper_1v8_ldo5 = regulator_get(NULL, "vdd_sensor_1v8");
		else if (pmic_id == GROUPER_PMIC_TI)
			grouper_1v8_ldo5 = regulator_get(NULL, "avdd_vdac");
		if (WARN_ON(IS_ERR(grouper_1v8_ldo5))) {
			pr_err("%s: couldn't get regulator vdd_1v8_ldo5: %d\n",
				__func__, (int)PTR_ERR(grouper_1v8_ldo5));
			goto reg_get_vdd_1v8_ldo5_fail;
		}
		regulator_set_voltage(grouper_1v8_ldo5, 1800000, 1800000);
	}
	regulator_enable(grouper_1v8_ldo5);
	msleep(10);

	gpio_direction_output(CAM2_LDO_EN_GPIO, 1);

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_NORMAL);

	msleep(10);

	gpio_direction_output(mi1040_rst_gpio, 1);

	return 0;

reg_get_vdd_1v8_ldo5_fail:
	if (grouper_1v8_ldo5) {
		regulator_put(grouper_1v8_ldo5);
	}
	grouper_1v8_ldo5 = NULL;

	return -ENODEV;
}

static int grouper_mi1040_power_off(void)
{
	gpio_direction_output(mi1040_rst_gpio, 0);
	msleep(10);

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CAM_MCLK, TEGRA_TRI_TRISTATE);

	gpio_direction_output(CAM2_LDO_EN_GPIO, 0);
	msleep(10);

	if (grouper_1v8_ldo5) {
		regulator_disable(grouper_1v8_ldo5);
	}

	return 0;
}

#ifdef CONFIG_VIDEO_MI1040
struct yuv_sensor_platform_data grouper_mi1040_data = {
	.power_on = grouper_mi1040_power_on,
	.power_off = grouper_mi1040_power_off,
};

static struct i2c_board_info grouper_i2c2_mi1040_board_info[] = {
	{
		I2C_BOARD_INFO("mi1040", 0x48),
		.platform_data = &grouper_mi1040_data,
	},
};
#endif

/* MPU board file definition */
static struct mpu_platform_data mpu_gyro_data = {
	.int_config	= 0x10,
	.level_shifter	= 0,
	.orientation	= MPU_GYRO_ORIENTATION,
	.sec_slave_type	= SECONDARY_SLAVE_TYPE_NONE,
	.key		= {221, 22, 205, 7,   217, 186, 151, 55,
			   206, 254, 35, 144, 225, 102,  47, 50},
};
static struct mpu_platform_data mpu_compass_data = {
	.orientation = MPU_COMPASS_ORIENTATION,
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

	pr_info("*** MPU START *** mpuirq_init...\n");

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

	inv_mpu_i2c2_board_info[0].irq = gpio_to_irq(MPU_GYRO_IRQ_GPIO);
	i2c_register_board_info(MPU_GYRO_BUS_NUM, inv_mpu_i2c2_board_info,
		ARRAY_SIZE(inv_mpu_i2c2_board_info));
}

static struct cap1106_i2c_platform_data grouper_cap1106_pdata = {
	.irq_gpio = TEGRA_GPIO_PR3,
};

static struct i2c_board_info grouper_i2c1_cap1106_board_info[] = {
    {
        I2C_BOARD_INFO("cap1106", 0x28),
        .platform_data	= &grouper_cap1106_pdata,
    },
};

static void capirq_init(void)
{
	if (GROUPER_PROJECT_BACH == grouper_get_project_id())
		i2c_register_board_info(2, grouper_i2c1_cap1106_board_info,
			ARRAY_SIZE(grouper_i2c1_cap1106_board_info));
}

static struct lid_sensor_platform_data grouper_lid_pdata = {
	.irq_gpio = TEGRA_GPIO_PS6,
};

static struct platform_device lid_device = {
	.name	= "lid-sensor",
	.id		= -1,
};

static void lid_init(void)
{
	pr_info("%s\n", __func__);
	lid_device.dev.platform_data = &grouper_lid_pdata;
	platform_device_register(&lid_device);
}

int __init grouper_sensors_init(void)
{
	int err;

	err = grouper_nct1008_init();
	if (err)
		pr_err("%s: nct1008 init failed\n", __func__);
	else
		i2c_register_board_info(4, grouper_i2c4_nct1008_board_info,
			ARRAY_SIZE(grouper_i2c4_nct1008_board_info));

	grouper_camera_init();

#ifdef CONFIG_VIDEO_MI1040
	i2c_register_board_info(2, grouper_i2c2_mi1040_board_info,
		ARRAY_SIZE(grouper_i2c2_mi1040_board_info));
#endif

	grouper_i2c1_al3010_board_info[0].irq = gpio_to_irq(TEGRA_GPIO_PZ2);
	i2c_register_board_info(2, grouper_i2c1_al3010_board_info,
		ARRAY_SIZE(grouper_i2c1_al3010_board_info));

	capirq_init();
	mpuirq_init();
	lid_init();

	return 0;
}
