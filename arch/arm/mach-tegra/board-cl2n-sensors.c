
/*
 * arch/arm/mach-tegra/board-kai-sensors.c
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
#include <linux/lis3dh.h>
#include <linux/regulator/consumer.h>
#include <linux/jsa1127.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <media/s5k5cag.h>
#include <media/yuv_sensor_cl2n.h>

#include <mach/gpio-tegra.h>

#include "board.h"
#include "board-cl2n.h"
#include "cpu-tegra.h"


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

static int __init kai_throttle_init(void)
{
	if (machine_is_kai())
		balanced_throttle_register(&tj_throttle, "tegra-balanced");
	return 0;
}
module_init(kai_throttle_init);

/* rear camera */
#define CAM1_RST_GPIO			TEGRA_GPIO_PBB3
#define CAM1_POWER_DWN_GPIO		TEGRA_GPIO_PBB5
/* front camera */
#define CAM2_RST_GPIO			TEGRA_GPIO_PBB4
#define CAM2_POWER_DWN_GPIO		TEGRA_GPIO_PBB6

#define CAM1_LDO_GPIO	TEGRA_GPIO_PR6
#define CAM2_LDO_GPIO	TEGRA_GPIO_PR7
#define CAM_VDD_GPIO	TEGRA_GPIO_PS0


struct kai_cam_gpio {
	int gpio;
	const char *label;
	int value;
	int active_high;
};

#define TEGRA_CAM_GPIO(_gpio, _label, _value, _active_high) \
	{					\
		.gpio = _gpio,			\
		.label = _label,		\
		.value = _value,		\
		.active_high = _active_high	\
	}

struct kai_cam_power_rail {
	struct regulator *cam_1v8_reg;
	struct regulator *cam_2v8_reg;
	struct kai_cam_gpio *gpio_pwdn;
	struct kai_cam_gpio *gpio_rst;
};

enum CAM_INDEX {
	CAM_REAR,
	CAM_FRONT
};

static struct kai_cam_gpio kai_cam_gpio_data[] = {
	[0] = TEGRA_CAM_GPIO(CAM1_POWER_DWN_GPIO, "cam1_power_en", 1, 1),
	[1] = TEGRA_CAM_GPIO(CAM1_RST_GPIO, "cam1_reset", 0, 0),
	[2] = TEGRA_CAM_GPIO(CAM2_POWER_DWN_GPIO, "cam2_power_en", 1, 1),
	[3] = TEGRA_CAM_GPIO(CAM2_RST_GPIO, "cam2_reset", 0, 0),
};

static struct kai_cam_power_rail kai_cam_pwr[] = {
	[0] = {
		.cam_1v8_reg = NULL,
		.cam_2v8_reg = NULL,
		.gpio_pwdn   = &kai_cam_gpio_data[0],
		.gpio_rst    = &kai_cam_gpio_data[1],
	},
	[1] = {
		.cam_1v8_reg = NULL,
		.cam_2v8_reg = NULL,
		.gpio_pwdn   = &kai_cam_gpio_data[2],
		.gpio_rst    = &kai_cam_gpio_data[3],
	},
};

static int kai_s5kcag_init(void)
{
	struct kai_cam_power_rail *cam_pwr = &kai_cam_pwr[CAM_REAR];
	printk("[camera](%s) \n",__FUNCTION__);
	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_3M low
	gpio_set_value(CAM_VDD_GPIO, 1);//1.8 high
	msleep(1);
	gpio_set_value(CAM1_LDO_GPIO, 1);//2.8v_3M high
	msleep(20);
	gpio_set_value(cam_pwr->gpio_pwdn->gpio, cam_pwr->gpio_pwdn->active_high);//pwdn_3M high
	msleep(20);
	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_3M low
	msleep(20);
	gpio_set_value(cam_pwr->gpio_rst->gpio, !cam_pwr->gpio_rst->active_high);//rst_3M high
	msleep(20);

	return 0;
}

static int kai_camera_init(void)
{
	int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(kai_cam_gpio_data); i++) {
		ret = gpio_request(kai_cam_gpio_data[i].gpio,
				   kai_cam_gpio_data[i].label);

		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio #%d\n",
				__func__, kai_cam_gpio_data[i].gpio);
			goto fail_free_gpio;
		}
		gpio_direction_output(kai_cam_gpio_data[i].gpio,
				      kai_cam_gpio_data[i].value);
		gpio_export(kai_cam_gpio_data[i].gpio, false);
	}

	ret = gpio_request(CAM1_LDO_GPIO, "cam1_ldo_en");
	if (ret < 0) {
		pr_err("%s: gpio_request failed for gpio %s\n",
			__func__, "CAM1_LDO_GPIO");
			goto fail_cam1_gpio;
	}

	ret = gpio_request(CAM2_LDO_GPIO, "cam2_ldo_en");
	if (ret < 0) {
		pr_err("%s: gpio_request failed for gpio %s\n",
			__func__, "CAM2_LDO_GPIO");
			goto fail_cam2_gpio;
	}

	ret = gpio_request(CAM_VDD_GPIO, "cam_vdd_en");
	if (ret < 0) {
		pr_err("%s: gpio_request failed for gpio %s\n",
			__func__, "CAM_VDD_GPIO");
			goto fail_vdd_gpio;
	}
	gpio_direction_output(CAM1_LDO_GPIO,0);
	gpio_direction_output(CAM2_LDO_GPIO,0);
	gpio_direction_output(CAM_VDD_GPIO,0);

	gpio_direction_output(CAM1_POWER_DWN_GPIO,0);
	gpio_set_value(CAM1_POWER_DWN_GPIO, 0);
	return 0;

fail_free_gpio:
	pr_err("%s failed!", __func__);
	while(i--)
		gpio_free(kai_cam_gpio_data[i].gpio);
	return ret;
fail_vdd_gpio:
	gpio_free(CAM_VDD_GPIO);
	return ret;
fail_cam1_gpio:
	gpio_free(CAM1_LDO_GPIO);
	return ret;
fail_cam2_gpio:
	gpio_free(CAM2_LDO_GPIO);
	return ret;

}

static int kai_s5k5cag_power_on(int delay_time)
{
	struct kai_cam_power_rail *cam_pwr = &kai_cam_pwr[CAM_REAR];
	printk("[Jimmy][camera](%s) \n",__FUNCTION__);

	msleep(10);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_3M low

	msleep(20);

	return 0;
}

static int kai_s5k5cag_power_off(void)
{

	struct kai_cam_power_rail *cam_pwr = &kai_cam_pwr[CAM_REAR];
	printk("[Jimmy][camera](%s) \n",__FUNCTION__);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, cam_pwr->gpio_pwdn->active_high);//pwdn_3M high

	msleep(20);

	return 0;
}

static int kai_s5k5cag_suspend(void)
{

	struct kai_cam_power_rail *cam_pwr = &kai_cam_pwr[CAM_REAR];
	printk("[Jimmy][camera](%s) \n",__FUNCTION__);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, cam_pwr->gpio_pwdn->active_high);//pwdn_3M high

	msleep(20);

	gpio_set_value(cam_pwr->gpio_rst->gpio, cam_pwr->gpio_rst->active_high);//rst_3M low

	msleep(1);

	gpio_set_value(CAM_VDD_GPIO, 0);//1.8v low

	gpio_set_value(CAM1_LDO_GPIO, 0);//2.8v_3M low

	msleep(20);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_3M low

	return 0;
}

static int kai_mt9m114_power_on(int delay_time)
{
	struct kai_cam_power_rail *cam_pwr = &kai_cam_pwr[CAM_FRONT];
	printk("[Jimmy][camera](%s) \n",__FUNCTION__);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_1.3M low

	gpio_set_value(CAM2_LDO_GPIO, 1);//2.8v_1.3M high

	msleep(10);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, cam_pwr->gpio_pwdn->active_high);//pwdn_1.3M high

	msleep(10);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_1.3M low

	msleep(10);

	gpio_set_value(cam_pwr->gpio_rst->gpio, !cam_pwr->gpio_rst->active_high);//rst_1.3M high

	msleep(10);

	return 0;
}

static int kai_mt9m114_power_off(void)
{
	struct kai_cam_power_rail *cam_pwr = &kai_cam_pwr[CAM_FRONT];
	printk("[Jimmy][camera](%s) \n",__FUNCTION__);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, cam_pwr->gpio_pwdn->active_high);//pwdn_1.3M high

	msleep(20);

	gpio_set_value(cam_pwr->gpio_rst->gpio, cam_pwr->gpio_rst->active_high);//rst_1.3M low

	msleep(1);

	gpio_set_value(CAM2_LDO_GPIO, 0);//2.8v_1.3M low

	msleep(20);

	gpio_set_value(cam_pwr->gpio_pwdn->gpio, !cam_pwr->gpio_pwdn->active_high);//pwdn_1.3M low

	return 0;
}

struct yuv_sensor_platform_data kai_s5k5cag_data = {
	.init = kai_s5kcag_init,
	.suspend = kai_s5k5cag_suspend,
	.power_on = kai_s5k5cag_power_on,
	.power_off = kai_s5k5cag_power_off,
};

struct yuv_sensor_platform_data kai_mt9m114_data = {
	.power_on = kai_mt9m114_power_on,
	.power_off = kai_mt9m114_power_off,
};

static struct i2c_board_info kai_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("s5k5cag", 0x2d),
		.platform_data = &kai_s5k5cag_data,
	},
	{
		I2C_BOARD_INFO("mt9m114", 0x48),
		.platform_data = &kai_mt9m114_data,
	},
};

static void jsa1127_configure_platform(void)
{
	/* regulator_get regulator_enable*/
}

static struct jsa1127_platform_data kai_jsa1127_pdata = {
    .configure_platform = jsa1127_configure_platform,
};

static struct lis3dh_acc_platform_data kai_lis3dh_acc_pdata = {
	.poll_interval	= 200,
	.min_interval	= 1,

	.g_range	= LIS3DH_ACC_G_4G,

	.axis_map_x	= 0,
	.axis_map_y	= 1,
	.axis_map_z	= 2,

	.negate_x	= 1,
	.negate_y	= 1,
	.negate_z	= 0,

	/* set gpio_int[1,2] either to the choosen gpio pin number or to -EINVAL
	 * if left unconnected
	 */
	.gpio_int1	= -EINVAL,
	.gpio_int2	= -EINVAL,
};

static struct i2c_board_info kai_i2c0_board_info[] = {
	{
		I2C_BOARD_INFO("jsa1127", 0x39),
		.platform_data = &kai_jsa1127_pdata,
	},
	{
		I2C_BOARD_INFO(LIS3DH_ACC_DEV_NAME, 0x18),
		.platform_data = &kai_lis3dh_acc_pdata,
	},
};

static inline void kai_msleep(u32 t)
{
        /*
        If timer value is between ( 10us - 20ms),
        usleep_range() is recommended.
        Please read Documentation/timers/timers-howto.txt.
        */
        usleep_range(t*1000, t*1000 + 500);
}


int __init kai_sensors_init(void)
{
	kai_camera_init();
	kai_s5kcag_init();
	kai_s5k5cag_power_off();

	i2c_register_board_info(2, kai_i2c2_board_info,
		ARRAY_SIZE(kai_i2c2_board_info));
	i2c_register_board_info(0, kai_i2c0_board_info,
		ARRAY_SIZE(kai_i2c0_board_info));

	return 0;
}
