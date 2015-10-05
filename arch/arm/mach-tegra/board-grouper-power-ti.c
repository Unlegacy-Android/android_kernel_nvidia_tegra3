/*
 * arch/arm/mach-tegra/board-grouper-power-ti.c
 *
 * Copyright (C) 2011-2012, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/i2c.h>
#include <linux/mfd/tps6591x.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/tps6591x-regulator.h>
#include <linux/regulator/tps62360.h>

#include <asm/mach-types.h>

#include "board-grouper.h"
#include <mach/board-grouper-misc.h>

static struct regulator_consumer_supply tps6591x_vdd1_supply_skubit0_1[] = {
	REGULATOR_SUPPLY("en_vddio_ddr_1v2", NULL),
};

static struct regulator_consumer_supply tps6591x_vdd2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_gen1v5", NULL),
	REGULATOR_SUPPLY("vcore_lcd", NULL),
	REGULATOR_SUPPLY("track_ldo1", NULL),
	REGULATOR_SUPPLY("external_ldo_1v2", NULL),
	REGULATOR_SUPPLY("vcore_cam1", NULL),
	REGULATOR_SUPPLY("vcore_cam2", NULL),
};

static struct regulator_consumer_supply tps6591x_vddctrl_supply_0[] = {
	REGULATOR_SUPPLY("vdd_cpu_pmu", NULL),
	REGULATOR_SUPPLY("vdd_cpu", NULL),
	REGULATOR_SUPPLY("vdd_sys", NULL),
};

static struct regulator_consumer_supply tps6591x_vio_supply_0[] = {
	REGULATOR_SUPPLY("vdd_gen1v8", NULL),
	REGULATOR_SUPPLY("avdd_hdmi_pll", NULL),
	REGULATOR_SUPPLY("avdd_usb_pll", "tegra-udc.0"),
	REGULATOR_SUPPLY("avdd_usb_pll", "tegra-ehci.0"),
	REGULATOR_SUPPLY("avdd_usb_pll", "tegra-ehci.1"),
	REGULATOR_SUPPLY("avdd_osc", NULL),
	REGULATOR_SUPPLY("vddio_sys", NULL),
	REGULATOR_SUPPLY("pwrdet_sdmmc4", NULL),
	REGULATOR_SUPPLY("vdd1v8_satelite", NULL),
	REGULATOR_SUPPLY("vddio_uart", NULL),
	REGULATOR_SUPPLY("pwrdet_uart", NULL),
	REGULATOR_SUPPLY("vddio_audio", NULL),
	REGULATOR_SUPPLY("pwrdet_audio", NULL),
	REGULATOR_SUPPLY("vddio_bb", NULL),
	REGULATOR_SUPPLY("pwrdet_bb", NULL),
	REGULATOR_SUPPLY("vddio_lcd_pmu", NULL),
	REGULATOR_SUPPLY("pwrdet_lcd", NULL),
	REGULATOR_SUPPLY("vddio_cam", NULL),
	REGULATOR_SUPPLY("pwrdet_cam", NULL),
	REGULATOR_SUPPLY("vddio_vi", NULL),
	REGULATOR_SUPPLY("pwrdet_vi", NULL),
	REGULATOR_SUPPLY("ldo6", NULL),
	REGULATOR_SUPPLY("ldo7", NULL),
	REGULATOR_SUPPLY("ldo8", NULL),
	REGULATOR_SUPPLY("vcore_audio", NULL),
	REGULATOR_SUPPLY("avcore_audio", NULL),
	REGULATOR_SUPPLY("pwrdet_sdmmc3", NULL),
	REGULATOR_SUPPLY("vcore1_lpddr2", NULL),
	REGULATOR_SUPPLY("vcom_1v8", NULL),
	REGULATOR_SUPPLY("pmuio_1v8", NULL),
	REGULATOR_SUPPLY("avdd_ic_usb", NULL),
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.3"),
};

static struct regulator_consumer_supply tps6591x_ldo1_supply_0[] = {
	REGULATOR_SUPPLY("avdd_pexb", NULL),
	REGULATOR_SUPPLY("vdd_pexb", NULL),
	REGULATOR_SUPPLY("avdd_pex_pll", NULL),
	REGULATOR_SUPPLY("avdd_pexa", NULL),
	REGULATOR_SUPPLY("vdd_pexa", NULL),
	REGULATOR_SUPPLY("vmmc", "sdhci-tegra.3"),
};

static struct regulator_consumer_supply tps6591x_ldo2_supply_0[] = {
	REGULATOR_SUPPLY("avdd_sata", NULL),
	REGULATOR_SUPPLY("vdd_sata", NULL),
	REGULATOR_SUPPLY("avdd_sata_pll", NULL),
	REGULATOR_SUPPLY("avdd_plle", NULL),
	REGULATOR_SUPPLY("vddio_sd_slot", "sdhci-tegra.0"),
};

static struct regulator_consumer_supply tps6591x_ldo3_supply_e118x[] = {
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.0"),
	REGULATOR_SUPPLY("pwrdet_sdmmc1", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_rtc", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo5_supply_e118x[] = {
	REGULATOR_SUPPLY("avdd_vdac", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo6_supply_0[] = {
	REGULATOR_SUPPLY("avdd_dsi_csi", NULL),
	REGULATOR_SUPPLY("pwrdet_mipi", NULL),
	REGULATOR_SUPPLY("vddio_hsic", NULL),
};
static struct regulator_consumer_supply tps6591x_ldo7_supply_0[] = {
	REGULATOR_SUPPLY("avdd_plla_p_c_s", NULL),
	REGULATOR_SUPPLY("avdd_pllm", NULL),
	REGULATOR_SUPPLY("avdd_pllu_d", NULL),
	REGULATOR_SUPPLY("avdd_pllu_d2", NULL),
	REGULATOR_SUPPLY("avdd_pllx", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo8_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr_hs", NULL),
};

#define TPS_PDATA_INIT(_name, _sname, _minmv, _maxmv, _supply_reg, _always_on, \
	_boot_on, _apply_uv, _init_uV, _init_enable, _init_apply, _ectrl, _flags, _off) \
	static struct tps6591x_regulator_platform_data pdata_##_name##_##_sname = \
	{								\
		.regulator = {						\
			.constraints = {				\
				.min_uV = (_minmv)*1000,		\
				.max_uV = (_maxmv)*1000,		\
				.valid_modes_mask = (REGULATOR_MODE_NORMAL |  \
						     REGULATOR_MODE_STANDBY), \
				.valid_ops_mask = (REGULATOR_CHANGE_MODE |    \
						   REGULATOR_CHANGE_STATUS |  \
						   REGULATOR_CHANGE_VOLTAGE), \
				.always_on = _always_on,		\
				.boot_on = _boot_on,			\
				.apply_uV = _apply_uv,			\
			},						\
			.num_consumer_supplies =			\
				ARRAY_SIZE(tps6591x_##_name##_supply_##_sname),	\
			.consumer_supplies = tps6591x_##_name##_supply_##_sname,	\
			.supply_regulator = _supply_reg,		\
		},							\
		.init_uV =  _init_uV * 1000,				\
		.init_enable = _init_enable,				\
		.init_apply = _init_apply,				\
		.ectrl = _ectrl,					\
		.flags = _flags,					\
		.shutdown_state_off = _off,			\
	};

TPS_PDATA_INIT(vdd1, skubit0_1, 600,  1500, 0, 1, 1, 0, -1, 0, 0, EXT_CTRL_SLEEP_OFF, 0, true);
TPS_PDATA_INIT(vdd2, 0,         600,  1500, 0, 1, 1, 0, -1, 0, 0, 0, 0, false);
TPS_PDATA_INIT(vddctrl, 0,      600,  1400, 0, 1, 1, 0, -1, 0, 0, EXT_CTRL_EN1, 0, true);
TPS_PDATA_INIT(vio,  0,         1500, 3300, 0, 1, 1, 0, -1, 0, 0, 0, 0, false);

TPS_PDATA_INIT(ldo1, 0,         1000, 3300, tps6591x_rails(VDD_2), 1, 0, 0, -1, 1, 1, 0, 0, true);
TPS_PDATA_INIT(ldo2, 0,         1050, 1050, tps6591x_rails(VDD_2), 0, 0, 1, -1, 0, 1, 0, 0, false);

TPS_PDATA_INIT(ldo3, e118x,     1000, 3300, 0, 0, 0, 0, -1, 0, 0, 0, 0, true);
TPS_PDATA_INIT(ldo4, 0,         1000, 3300, 0, 1, 0, 0, -1, 0, 0, 0, 0, false);
TPS_PDATA_INIT(ldo5, e118x,     1000, 3300, 0, 0, 0, 0, -1, 0, 0, 0, 0, true);

TPS_PDATA_INIT(ldo6, 0,         1200, 1200, tps6591x_rails(VIO), 0, 0, 1, -1, 0, 0, 0, 0, true);
TPS_PDATA_INIT(ldo7, 0,         1200, 1200, tps6591x_rails(VIO), 1, 1, 1, -1, 0, 0, EXT_CTRL_SLEEP_OFF, LDO_LOW_POWER_ON_SUSPEND, false);
TPS_PDATA_INIT(ldo8, 0,         1000, 3300, tps6591x_rails(VIO), 1, 0, 0, -1, 0, 0, EXT_CTRL_SLEEP_OFF, LDO_LOW_POWER_ON_SUSPEND, false);

#if defined(CONFIG_RTC_DRV_TPS6591x)
static struct tps6591x_rtc_platform_data rtc_data = {
	.irq = TEGRA_NR_IRQS + TPS6591X_INT_RTC_ALARM,
	.time = {
		.tm_year = 2000,
		.tm_mon = 0,
		.tm_mday = 1,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	},
};

#define TPS_RTC_REG()					\
	{						\
		.id	= 0,				\
		.name	= "rtc_tps6591x",		\
		.platform_data = &rtc_data,		\
	}
#endif

#define TPS_REG(_id, _name, _sname)				\
	{							\
		.id	= TPS6591X_ID_##_id,			\
		.name	= "tps6591x-regulator",			\
		.platform_data	= &pdata_##_name##_##_sname,	\
	}

#define TPS6591X_DEV_COMMON_E118X 		\
	TPS_REG(VDD_2, vdd2, 0),		\
	TPS_REG(VDDCTRL, vddctrl, 0),		\
	TPS_REG(LDO_1, ldo1, 0),		\
	TPS_REG(LDO_2, ldo2, 0),		\
	TPS_REG(LDO_3, ldo3, e118x),		\
	TPS_REG(LDO_4, ldo4, 0),		\
	TPS_REG(LDO_5, ldo5, e118x),		\
	TPS_REG(LDO_6, ldo6, 0),		\
	TPS_REG(LDO_7, ldo7, 0),		\
	TPS_REG(LDO_8, ldo8, 0)

static struct tps6591x_subdev_info tps_devs_e118x_skubit0_1[] = {
	TPS_REG(VIO, vio, 0),
	TPS_REG(VDD_1, vdd1, skubit0_1),
	TPS6591X_DEV_COMMON_E118X,
#if defined(CONFIG_RTC_DRV_TPS6591x)
	TPS_RTC_REG(),
#endif
};

#define TPS6591X_DEV_COMMON_GROUPER		\
	TPS_REG(VDD_2, vdd2, 0),		\
	TPS_REG(VDDCTRL, vddctrl, 0),		\
	TPS_REG(LDO_1, ldo1, 0),		\
	TPS_REG(LDO_2, ldo2, 0),		\
	TPS_REG(LDO_3, ldo3, e1198),		\
	TPS_REG(LDO_4, ldo4, 0),		\
	TPS_REG(LDO_5, ldo5, e1198),		\
	TPS_REG(LDO_6, ldo6, 0),		\
	TPS_REG(LDO_7, ldo7, 0),		\
	TPS_REG(LDO_8, ldo8, 0)

#define TPS_GPIO_INIT_PDATA(gpio_nr, _init_apply, _sleep_en, _pulldn_en, _output_en, _output_val)	\
	[gpio_nr] = {					\
			.sleep_en	= _sleep_en,	\
			.pulldn_en	= _pulldn_en,	\
			.output_mode_en	= _output_en,	\
			.output_val	= _output_val,	\
			.init_apply	= _init_apply,	\
		     }
static struct tps6591x_gpio_init_data tps_gpio_pdata_e1291_a04[] =  {
	TPS_GPIO_INIT_PDATA(0, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(1, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(2, 1, 1, 0, 1, 1),
	TPS_GPIO_INIT_PDATA(3, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(4, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(5, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(6, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(7, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(8, 0, 0, 0, 0, 0),
};

static struct tps6591x_sleep_keepon_data tps_slp_keepon = {
	.clkout32k_keepon = 1,
};

static struct tps6591x_platform_data tps_platform = {
	.irq_base	= TPS6591X_IRQ_BASE,
	.gpio_base	= TPS6591X_GPIO_BASE,
	.dev_slp_en	= true,
	.slp_keepon	= &tps_slp_keepon,
	.use_power_off	= true,
};

static struct i2c_board_info __initdata grouper_ti_regulators[] = {
	{
		I2C_BOARD_INFO("tps6591x", 0x2D),
		.irq		= INT_EXTERNAL_PMU,
		.platform_data	= &tps_platform,
	},
};

/* TPS62361B DC-DC converter */
static struct regulator_consumer_supply tps62361_dcdc_supply[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
};

static struct tps62360_regulator_platform_data tps62361_pdata = {
	.reg_init_data = {					\
		.constraints = {				\
			.min_uV = 500000,			\
			.max_uV = 1770000,			\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |  \
					     REGULATOR_MODE_STANDBY), \
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |    \
					   REGULATOR_CHANGE_STATUS |  \
					   REGULATOR_CHANGE_VOLTAGE), \
			.always_on = 1,				\
			.boot_on =  1,				\
			.apply_uV = 0,				\
		},						\
		.num_consumer_supplies = ARRAY_SIZE(tps62361_dcdc_supply), \
		.consumer_supplies = tps62361_dcdc_supply,	\
		},						\
	.en_discharge = true,					\
	.vsel0_gpio = -1,					\
	.vsel1_gpio = -1,					\
	.vsel0_def_state = 1,					\
	.vsel1_def_state = 1,					\
};

static struct i2c_board_info __initdata tps62361_boardinfo[] = {
	{
		I2C_BOARD_INFO("tps62361", 0x60),
		.platform_data	= &tps62361_pdata,
	},
};

/**************** GPIO based fixed regulator *****************/
/* EN_5V_CP from PMU GP0 */
static struct regulator_consumer_supply fixed_reg_en_5v_cp_supply[] = {
	REGULATOR_SUPPLY("vdd_5v0_sby", NULL),
	REGULATOR_SUPPLY("vdd_hall", NULL),
	REGULATOR_SUPPLY("vterm_ddr", NULL),
	REGULATOR_SUPPLY("v2ref_ddr", NULL),
};

/* EN_5V0 From PMU GP2 */
static struct regulator_consumer_supply fixed_reg_en_5v0_supply[] = {
	REGULATOR_SUPPLY("vdd_5v0_sys", NULL),
};

/* EN_DDR From PMU GP6 */
static struct regulator_consumer_supply fixed_reg_en_ddr_supply[] = {
	REGULATOR_SUPPLY("mem_vddio_ddr", NULL),
	REGULATOR_SUPPLY("t30_vddio_ddr", NULL),
};

/* EN_3V3_SYS From PMU GP7 */
static struct regulator_consumer_supply fixed_reg_en_3v3_sys_supply[] = {
	REGULATOR_SUPPLY("vdd_lvds", NULL),
	REGULATOR_SUPPLY("vdd_pnl", NULL),
	REGULATOR_SUPPLY("vcom_3v3", NULL),
	REGULATOR_SUPPLY("vdd_3v3", NULL),
	REGULATOR_SUPPLY("vcore_mmc", NULL),
	REGULATOR_SUPPLY("vddio_pex_ctl", NULL),
	REGULATOR_SUPPLY("pwrdet_pex_ctl", NULL),
	REGULATOR_SUPPLY("hvdd_pex_pmu", NULL),
	REGULATOR_SUPPLY("avdd_hdmi", NULL),
	REGULATOR_SUPPLY("avdd_usb", "tegra-udc.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.1"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.2"),
	REGULATOR_SUPPLY("vdd_ddr_rx", NULL),
	REGULATOR_SUPPLY("vcore_nand", NULL),
	REGULATOR_SUPPLY("hvdd_sata", NULL),
	REGULATOR_SUPPLY("vddio_gmi_pmu", NULL),
	REGULATOR_SUPPLY("pwrdet_nand", NULL),
	REGULATOR_SUPPLY("avdd_cam1", NULL),
	REGULATOR_SUPPLY("vdd_af", NULL),
	REGULATOR_SUPPLY("avdd_cam2", NULL),
	REGULATOR_SUPPLY("vdd_acc", NULL),
	REGULATOR_SUPPLY("vdd_phtl", NULL),
	REGULATOR_SUPPLY("vddio_tp", NULL),
	REGULATOR_SUPPLY("vdd_led", NULL),
	REGULATOR_SUPPLY("vddio_cec", NULL),
	REGULATOR_SUPPLY("vdd_cmps", NULL),
	REGULATOR_SUPPLY("vdd_temp", NULL),
	REGULATOR_SUPPLY("vpp_kfuse", NULL),
	REGULATOR_SUPPLY("vddio_ts", NULL),
	REGULATOR_SUPPLY("vdd_ir_led", NULL),
	REGULATOR_SUPPLY("vddio_1wire", NULL),
	REGULATOR_SUPPLY("avddio_audio", NULL),
	REGULATOR_SUPPLY("vdd_ec", NULL),
	REGULATOR_SUPPLY("vcom_pa", NULL),
	REGULATOR_SUPPLY("vdd_3v3_devices", NULL),
	REGULATOR_SUPPLY("vdd_3v3_dock", NULL),
	REGULATOR_SUPPLY("vdd_3v3_edid", NULL),
	REGULATOR_SUPPLY("vdd_3v3_hdmi_cec", NULL),
	REGULATOR_SUPPLY("vdd_3v3_gmi", NULL),
	REGULATOR_SUPPLY("vdd_spk_amp", "tegra-snd-wm8903.0"),
	REGULATOR_SUPPLY("vdd_3v3_sensor", NULL),
	REGULATOR_SUPPLY("vdd_3v3_cam", NULL),
	REGULATOR_SUPPLY("vdd_3v3_als", NULL),
	REGULATOR_SUPPLY("debug_cons", NULL),
};

/* DIS_5V_SWITCH from AP SPI2_SCK X02 */
static struct regulator_consumer_supply fixed_reg_dis_5v_switch_supply[] = {
	REGULATOR_SUPPLY("master_5v_switch", NULL),
};

/* EN_VDD_BL */
static struct regulator_consumer_supply fixed_reg_en_vdd_bl_supply[] = {
	REGULATOR_SUPPLY("vdd_backlight", NULL),
	REGULATOR_SUPPLY("vdd_backlight1", NULL),
};

/* EN_3V3_MODEM from AP GPIO VI_VSYNCH D06*/
static struct regulator_consumer_supply fixed_reg_en_3v3_modem_supply[] = {
	REGULATOR_SUPPLY("vdd_3v3_mini_card", NULL),
	REGULATOR_SUPPLY("vdd_mini_card", NULL),
};

/* EN_VDD_PNL1 from AP GPIO VI_D6 L04*/
static struct regulator_consumer_supply fixed_reg_en_vdd_pnl1_supply[] = {
	REGULATOR_SUPPLY("vdd_lcd_panel", NULL),
};

/* CAM1_LDO_EN from AP GPIO KB_ROW6 R06*/
static struct regulator_consumer_supply fixed_reg_cam1_ldo_en_supply[] = {
	REGULATOR_SUPPLY("vdd_2v8_cam1", NULL),
	REGULATOR_SUPPLY("avdd", "6-0072"),
	REGULATOR_SUPPLY("vdd", "6-000e"),
};

/* CAM3_LDO_EN from AP GPIO KB_ROW8 S00*/
static struct regulator_consumer_supply fixed_reg_cam3_ldo_en_supply[] = {
	REGULATOR_SUPPLY("vdd_cam3", NULL),
};

/* EN_VDD_COM from AP GPIO SDMMC3_DAT5 D00*/
static struct regulator_consumer_supply fixed_reg_en_vdd_com_supply[] = {
	REGULATOR_SUPPLY("vdd_com_bd", NULL),
};

/* EN_3V3_EMMC from AP GPIO SDMMC3_DAT4 D01*/
static struct regulator_consumer_supply fixed_reg_en_3v3_emmc_supply[] = {
	REGULATOR_SUPPLY("vdd_emmc_core", NULL),
};

/* EN_3v3_FUSE from AP GPIO VI_D08 L06*/
static struct regulator_consumer_supply fixed_reg_en_3v3_fuse_supply[] = {
	REGULATOR_SUPPLY("vpp_fuse", NULL),
};

/* EN_1V8_CAM from AP GPIO GPIO_PBB4 PBB04*/
static struct regulator_consumer_supply fixed_reg_en_1v8_cam_supply[] = {
	REGULATOR_SUPPLY("vdd_1v8_cam1", NULL),
	REGULATOR_SUPPLY("vdd_1v8_cam2", NULL),
	REGULATOR_SUPPLY("vdd_1v8_cam3", NULL),
	REGULATOR_SUPPLY("dvdd", "6-0072"),
	REGULATOR_SUPPLY("dvdd", "7-0072"),
	REGULATOR_SUPPLY("vdd_i2c", "6-000e"),
	REGULATOR_SUPPLY("vdd_i2c", "7-000e"),
	REGULATOR_SUPPLY("vdd_i2c", "2-0033"),
};

/* EN_USB1_VBUS_OC*/
static struct regulator_consumer_supply fixed_reg_en_usb1_vbus_oc_supply[] = {
	REGULATOR_SUPPLY("usb_vbus", "tegra-ehci.0"),
};

/* EN_VDDIO_VID_OC from AP GPIO VI_PCLK T00*/
static struct regulator_consumer_supply fixed_reg_en_vddio_vid_oc_supply[] = {
	REGULATOR_SUPPLY("vdd_hdmi_con", NULL),
};

/* Macro for defining fixed regulator sub device data */
#define FIXED_SUPPLY(_name) "fixed_reg_"#_name
#define FIXED_REG_OD(_id, _var, _name, _in_supply, _always_on,		\
		_boot_on, _gpio_nr, _active_high, _boot_state,		\
		_millivolts, _od_state)					\
	static struct regulator_init_data ri_data_##_var =		\
	{								\
		.supply_regulator = _in_supply,				\
		.num_consumer_supplies =				\
			ARRAY_SIZE(fixed_reg_##_name##_supply),		\
		.consumer_supplies = fixed_reg_##_name##_supply,	\
		.constraints = {					\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |	\
					REGULATOR_MODE_STANDBY),	\
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |	\
					REGULATOR_CHANGE_STATUS |	\
					REGULATOR_CHANGE_VOLTAGE),	\
			.always_on = _always_on,			\
			.boot_on = _boot_on,				\
		},							\
	};								\
	static struct fixed_voltage_config fixed_reg_##_var##_pdata =	\
	{								\
		.supply_name = FIXED_SUPPLY(_name),			\
		.microvolts = _millivolts * 1000,			\
		.gpio = _gpio_nr,					\
		.enable_high = _active_high,				\
		.enabled_at_boot = _boot_state,				\
		.init_data = &ri_data_##_var,				\
		.gpio_is_open_drain = _od_state,			\
	};								\
	static struct platform_device fixed_reg_##_var##_dev = {	\
		.name   = "reg-fixed-voltage",				\
		.id     = _id,						\
		.dev    = {						\
			.platform_data = &fixed_reg_##_var##_pdata,	\
		},							\
	};

#define FIXED_REG(_id, _var, _name, _in_supply, _always_on, _boot_on,	\
		 _gpio_nr, _active_high, _boot_state, _millivolts)	\
	FIXED_REG_OD(_id, _var, _name, _in_supply, _always_on, _boot_on,  \
		_gpio_nr, _active_high, _boot_state, _millivolts, false)


/* common to most of boards*/
FIXED_REG(0, en_5v_cp,		en_5v_cp,	NULL,				1,	0,	TPS6591X_GPIO_0,	true,	1, 5000);
FIXED_REG(5, en_3v3_modem,	en_3v3_modem,	NULL,				1,      0,      TEGRA_GPIO_PD6,		true,	1, 3300);
FIXED_REG(7, cam3_ldo_en,	cam3_ldo_en,	FIXED_SUPPLY(en_3v3_sys),	0,      0,      TEGRA_GPIO_PS0,		true,	0, 3300);
FIXED_REG(8, en_vdd_com,	en_vdd_com,	FIXED_SUPPLY(en_3v3_sys),	1,      0,      TEGRA_GPIO_PD0,		true,	1, 3300);
FIXED_REG(10, en_3v3_emmc,	en_3v3_emmc,	FIXED_SUPPLY(en_3v3_sys), 	1,      0,      TEGRA_GPIO_PD1,		true,	1, 3300);
FIXED_REG(13, en_1v8_cam,	en_1v8_cam,	tps6591x_rails(VIO),		0,      0,      TEGRA_GPIO_PBB4,	true,	0, 1800);

/* Specific to E1187/E1186/E1256 */
FIXED_REG(14, dis_5v_switch_e118x,	dis_5v_switch,	FIXED_SUPPLY(en_5v0), 	0,      0,      TEGRA_GPIO_PX2,		false,	0, 5000);

/* E1291-A04/A05 specific */
FIXED_REG(1, en_5v0_a04,	en_5v0,		NULL,				0,      0,      TPS6591X_GPIO_8,	true,	0, 5000);
FIXED_REG(2, en_ddr_a04,	en_ddr,		NULL,				1,      0,      TPS6591X_GPIO_7,	true,	1, 1500);
FIXED_REG(3, en_3v3_sys_a04,	en_3v3_sys,	NULL,				0,      0,      TPS6591X_GPIO_6,	true,	1, 3300);

/* Specific to pm269 */
FIXED_REG(4, en_vdd_bl_pm269,		en_vdd_bl,		NULL, 				0,      0,      TEGRA_GPIO_PH3,	true,	1, 5000);
FIXED_REG(6, en_vdd_pnl1_pm269,		en_vdd_pnl1,		FIXED_SUPPLY(en_3v3_sys),	0,      0,      TEGRA_GPIO_PW1,	true,	1, 3300);
FIXED_REG(9, en_3v3_fuse_pm269,		en_3v3_fuse,		FIXED_SUPPLY(en_3v3_sys), 	0,      0,      TEGRA_GPIO_PC1,	true,	0, 3300);
//FIXED_REG(12, en_3v3_pex_hvdd_pm269,	en_3v3_pex_hvdd,	FIXED_SUPPLY(en_3v3_sys), 	0,      0,      TEGRA_GPIO_PC6,	true,	0, 3300);

/* E1198/E1291 specific*/
FIXED_REG(18, cam1_ldo_en,	cam1_ldo_en,	FIXED_SUPPLY(en_3v3_sys),	0,      0,      TEGRA_GPIO_PR6,		true,	0, 2800);
//FIXED_REG(19, cam2_ldo_en,	cam2_ldo_en,	FIXED_SUPPLY(en_3v3_sys),	0,      0,      TEGRA_GPIO_PR7,		true,	0, 2800);

/****************** Open collector Load switches *******/
/*Specific to pm269*/
FIXED_REG_OD(17, en_vddio_vid_oc_pm269,	en_vddio_vid_oc,	FIXED_SUPPLY(dis_5v_switch),	0,      0,	TEGRA_GPIO_PP2,		true,	0, 5000, true);

/* Specific to E1187/E1186/E1256 */
FIXED_REG_OD(15, en_usb1_vbus_oc_e118x,	en_usb1_vbus_oc,	FIXED_SUPPLY(dis_5v_switch), 	0,      0,      TEGRA_GPIO_PI4,		true,	0, 5000, true);

/*
 * Creating the fixed/gpio-switch regulator device tables for different boards
 */
#define ADD_FIXED_REG(_name)	(&fixed_reg_##_name##_dev)

#define PM269_FIXED_REG				\
	ADD_FIXED_REG(en_5v_cp),		\
	ADD_FIXED_REG(en_5v0_a04),		\
	ADD_FIXED_REG(en_ddr_a04),		\
	ADD_FIXED_REG(en_3v3_sys_a04),		\
	ADD_FIXED_REG(en_3v3_modem),		\
	ADD_FIXED_REG(cam1_ldo_en),		\
	ADD_FIXED_REG(cam3_ldo_en),		\
	ADD_FIXED_REG(en_vdd_com),		\
	ADD_FIXED_REG(en_3v3_fuse_pm269),	\
	ADD_FIXED_REG(en_3v3_emmc),		\
	ADD_FIXED_REG(en_1v8_cam),		\
	ADD_FIXED_REG(dis_5v_switch_e118x),	\
	ADD_FIXED_REG(en_usb1_vbus_oc_e118x),	\
	ADD_FIXED_REG(en_vddio_vid_oc_pm269),

#define E1247_DISPLAY_FIXED_REG			\
	ADD_FIXED_REG(en_vdd_bl_pm269),		\
	ADD_FIXED_REG(en_vdd_pnl1_pm269),

/* Fixed regulator devices for PM269 */
static struct platform_device *fixed_reg_devs_pm269[] = {
	PM269_FIXED_REG
	E1247_DISPLAY_FIXED_REG
};

void __init grouper_tps6591x_regulator_init(void) {
	/* The regulator details have complete constraints */
	regulator_has_full_constraints();

	pdata_ldo3_e118x.slew_rate_uV_per_us = 250;

	tps_platform.num_subdevs = ARRAY_SIZE(tps_devs_e118x_skubit0_1);
	tps_platform.subdevs = tps_devs_e118x_skubit0_1;

	/*
	 *E1291-A04/A05/A07 or PM269-alike:
	 *	Enable DEV_SLP and enable sleep on GPIO2
	 */
	pr_info("%s: TPS6591x GPIO2 was reprogrammed to follow state of "
		"SLEEP input\n", __func__);
	tps_platform.gpio_init_data = tps_gpio_pdata_e1291_a04;
	tps_platform.num_gpioinit_data =
				ARRAY_SIZE(tps_gpio_pdata_e1291_a04);

	i2c_register_board_info(4, grouper_ti_regulators, 1);

	/* Register the external core regulator */
	pr_info("%s: Registering the core regulator\n", __func__);
	i2c_register_board_info(4, tps62361_boardinfo, 1);
}

static int __init grouper_ti_fixed_regulator_init(void)
{
	if (!machine_is_grouper() || !grouper_query_pmic_id())
		return 0;

	return platform_add_devices(fixed_reg_devs_pm269,
				ARRAY_SIZE(fixed_reg_devs_pm269));
}
subsys_initcall_sync(grouper_ti_fixed_regulator_init);
