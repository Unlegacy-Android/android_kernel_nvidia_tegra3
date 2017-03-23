/*
 * arch/arm/mach-tegra/board-acer-t30-power.c
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
#include <linux/pda_power.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/tps6591x.h>
#include <linux/mfd/max77663-core.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/tps6591x-regulator.h>
#include <linux/regulator/tps62360.h>
#include <linux/power/gpio-charger.h>

#include <asm/mach-types.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/edp.h>

#include "gpio-names.h"
#include "board.h"
#include "board-acer-t30.h"
#include "pm.h"
#include "tegra3_tsensor.h"

#define PMC_CTRL		0x0
#define PMC_CTRL_INTR_LOW	(1 << 17)

static struct regulator_consumer_supply tps6591x_vdd2_supply_0[] = {
	REGULATOR_SUPPLY("en_vddio_ddr_1v2", NULL),
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
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.3"),
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
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.2"),
	REGULATOR_SUPPLY("pwrdet_sdmmc3", NULL),
	REGULATOR_SUPPLY("vcore1_lpddr2", NULL),
	REGULATOR_SUPPLY("vcom_1v8", NULL),
	REGULATOR_SUPPLY("pmuio_1v8", NULL),
	REGULATOR_SUPPLY("avdd_ic_usb", NULL),
	REGULATOR_SUPPLY("vdd", "1-004c"),
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
	REGULATOR_SUPPLY("vddio_sd_slot", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo3_supply_0[] = {
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.0"),
	REGULATOR_SUPPLY("pwrdet_sdmmc1", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_rtc", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo5_supply_0[] = {
	REGULATOR_SUPPLY("avdd_vdac", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo6_supply_0[] = {
	REGULATOR_SUPPLY("avdd_dsi_csi", NULL),
	REGULATOR_SUPPLY("pwrdet_mipi", NULL),
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
	_boot_on, _apply_uv, _init_uV, _init_enable, _init_apply, _ectrl, _flags) \
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
	}

TPS_PDATA_INIT(vdd2, 0,         600,  1500, 0, 1, 1, 0, -1, 0, 0, 0, 0);
TPS_PDATA_INIT(vddctrl, 0,      600,  1400, 0, 1, 1, 0, 1000, 0, 1, EXT_CTRL_EN1, 0);
TPS_PDATA_INIT(vio,  0,         1500, 3300, 0, 1, 1, 0, -1, 0, 0, 0, 0);
TPS_PDATA_INIT(ldo1, 0,         1000, 3300, tps6591x_rails(VDD_2), 1, 0, 0, 2850, 1, 1, 0, 0);
TPS_PDATA_INIT(ldo2, 0,         1000, 3300, tps6591x_rails(VDD_2), 0, 0, 1, 3300, 0, 1, 0, 0);
TPS_PDATA_INIT(ldo3, 0,     1000, 3300, 0, 0, 0, 1, 3300, 0, 1, 0, 0);
TPS_PDATA_INIT(ldo4, 0,         1000, 3300, 0, 1, 0, 0, -1, 0, 0, 0, LDO_LOW_POWER_ON_SUSPEND);
TPS_PDATA_INIT(ldo5, 0,     1000, 3300, 0, 1, 0, 0, 2800, 0, 1, 0, 0);
TPS_PDATA_INIT(ldo6, 0,         1000, 3300, tps6591x_rails(VIO), 0, 0, 0, 1200, 0, 1, 0, 0);
TPS_PDATA_INIT(ldo7, 0,         1200, 1200, tps6591x_rails(VIO), 1, 1, 1, -1, 0, 0, EXT_CTRL_SLEEP_OFF, LDO_LOW_POWER_ON_SUSPEND);
TPS_PDATA_INIT(ldo8, 0,         1000, 3300, tps6591x_rails(VIO), 1, 0, 0, -1, 0, 0, EXT_CTRL_SLEEP_OFF, LDO_LOW_POWER_ON_SUSPEND);

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

static struct tps6591x_subdev_info tps_devs_t30[] = {
	TPS_REG(VIO, vio, 0),
	TPS_REG(VDD_2, vdd2, 0),
	TPS_REG(VDDCTRL, vddctrl, 0),
	TPS_REG(LDO_1, ldo1, 0),
	TPS_REG(LDO_2, ldo2, 0),
	TPS_REG(LDO_3, ldo3, 0),
	TPS_REG(LDO_4, ldo4, 0),
	TPS_REG(LDO_5, ldo5, 0),
	TPS_REG(LDO_6, ldo6, 0),
	TPS_REG(LDO_7, ldo7, 0),
	TPS_REG(LDO_8, ldo8, 0),
#if defined(CONFIG_RTC_DRV_TPS6591x)
	TPS_RTC_REG(),
#endif
};

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

static struct i2c_board_info __initdata cardhu_regulators[] = {
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

#define AC_DETECT_GPIO TEGRA_GPIO_PO4

static char *battery[] = {
	"bq27541-0",
};

static struct gpio_charger_platform_data gpio_charger_pdata = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.gpio = AC_DETECT_GPIO,
	.gpio_active_low = 0,
	.supplied_to = battery,
	.num_supplicants = ARRAY_SIZE(battery),
};

static struct platform_device gpio_charger_device = {
	.name = "gpio-charger",
	.dev = {
		.platform_data = &gpio_charger_pdata,
	},
};

static struct i2c_board_info __initdata bq27541_boardinfo[] = {
	{
		I2C_BOARD_INFO("bq27541", 0x55),
	},
};

int __init cardhu_regulator_init(void)
{
	struct board_info board_info;
	struct board_info pmu_board_info;
	void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	u32 pmc_ctrl;
	bool ext_core_regulator = false;

	/* configure the power management controller to trigger PMU
	 * interrupts when low */

	pmc_ctrl = readl(pmc + PMC_CTRL);
	writel(pmc_ctrl | PMC_CTRL_INTR_LOW, pmc + PMC_CTRL);

	tegra_get_board_info(&board_info);
	tegra_get_pmu_board_info(&pmu_board_info);

	/* The regulator details have complete constraints */
	regulator_has_full_constraints();

	if ((board_info.board_id == BOARD_E1291) &&
		(board_info.sku & SKU_DCDC_TPS62361_SUPPORT))
		ext_core_regulator = true;

        tps_platform.num_subdevs = ARRAY_SIZE(tps_devs_t30);
	tps_platform.subdevs = tps_devs_t30;

	i2c_register_board_info(4, cardhu_regulators, 1);

	tps_platform.dev_slp_en = true;
	tps_platform.gpio_init_data = tps_gpio_pdata_e1291_a04;
	tps_platform.num_gpioinit_data = ARRAY_SIZE(tps_gpio_pdata_e1291_a04);

	/* Register the external core regulator if it is require */
	if (ext_core_regulator) {
		pr_info("Registering the core regulator\n");
		i2c_register_board_info(4, tps62361_boardinfo, 1);
	}

	i2c_register_board_info(4, bq27541_boardinfo, 1);
	platform_device_register(&gpio_charger_device);

	return 0;
}


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
	REGULATOR_SUPPLY("vpp_fuse", NULL),
	REGULATOR_SUPPLY("vdd_ddr_rx", NULL),
	REGULATOR_SUPPLY("avdd_usb", "tegra-udc.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.1"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.2"),
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
        REGULATOR_SUPPLY("vdd_nct1008", NULL),
	REGULATOR_SUPPLY("vdd", "4-004c"),
	REGULATOR_SUPPLY("avdd", "1-004c"),
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
	}

#define FIXED_REG(_id, _var, _name, _in_supply, _always_on, _boot_on,	\
		 _gpio_nr, _active_high, _boot_state, _millivolts)	\
	FIXED_REG_OD(_id, _var, _name, _in_supply, _always_on, _boot_on,  \
		_gpio_nr, _active_high, _boot_state, _millivolts, false)


/* common to most of boards*/
FIXED_REG(0, en_5v_cp,		en_5v_cp,	NULL,				1,	0,	TPS6591X_GPIO_0,	true,	1, 5000);
FIXED_REG(1, en_5v0,		en_5v0,		NULL,				1,      0,      TPS6591X_GPIO_8,	true,	1, 5000);
FIXED_REG(2, en_ddr,		en_ddr,		NULL,				1,      0,      TPS6591X_GPIO_7,	true,	1, 1500);
FIXED_REG(3, en_3v3_sys,	en_3v3_sys,	NULL,				1,      0,      TPS6591X_GPIO_6,	true,	1, 3300);

/*
 * Creating the fixed/gpio-switch regulator device tables for different boards
 */
#define ADD_FIXED_REG(_name)	(&fixed_reg_##_name##_dev)

#define ACER_T30_GPIO_REG			\
	ADD_FIXED_REG(en_5v_cp),		\
	ADD_FIXED_REG(en_5v0),			\
	ADD_FIXED_REG(en_ddr),			\
	ADD_FIXED_REG(en_3v3_sys),		\

/* Fixed regulator devices for ACER T30 */
static struct platform_device *fixed_reg_devs_acer_t30[] = {
	ACER_T30_GPIO_REG
};

int __init cardhu_fixed_regulator_init(void)
{
	struct platform_device **fixed_reg_devs;
	int    nfixreg_devs;

	nfixreg_devs = ARRAY_SIZE(fixed_reg_devs_acer_t30);
	fixed_reg_devs = fixed_reg_devs_acer_t30;

	return platform_add_devices(fixed_reg_devs, nfixreg_devs);
}
subsys_initcall_sync(cardhu_fixed_regulator_init);

static void cardhu_board_suspend(int lp_state, enum suspend_stage stg)
{
	if ((lp_state == TEGRA_SUSPEND_LP1) && (stg == TEGRA_SUSPEND_BEFORE_CPU))
		tegra_console_uart_suspend();
}

static void cardhu_board_resume(int lp_state, enum resume_stage stg)
{
	if ((lp_state == TEGRA_SUSPEND_LP1) && (stg == TEGRA_RESUME_AFTER_CPU))
		tegra_console_uart_resume();
}

static struct tegra_suspend_platform_data cardhu_suspend_data = {
	.cpu_timer	= 2000,
	.cpu_off_timer	= 200,
	.suspend_mode	= TEGRA_SUSPEND_LP0,
	.core_timer	= 0x7e7e,
	.core_off_timer = 0,
	.corereq_high	= true,
	.sysclkreq_high	= true,
	.cpu_lp2_min_residency = 2000,
	.board_suspend = cardhu_board_suspend,
	.board_resume = cardhu_board_resume,
#ifdef CONFIG_TEGRA_LP1_950
	.lp1_lowvolt_support = true,
	.i2c_base_addr = TEGRA_I2C5_BASE,
	.pmuslave_addr = 0x78,
	.core_reg_addr = 0x17,
	.lp1_core_volt_low = 0x0C,
	.lp1_core_volt_high = 0x20,
#endif
};

int __init cardhu_suspend_init(void)
{
	struct board_info board_info;
	struct board_info pmu_board_info;

	tegra_get_board_info(&board_info);
	tegra_get_pmu_board_info(&pmu_board_info);

	/* For PMU Fab A03, A04 and A05 make core_pwr_req to high */
	cardhu_suspend_data.corereq_high = true;

	tegra_init_suspend(&cardhu_suspend_data);
	return 0;
}

#ifdef CONFIG_TEGRA_EDP_LIMITS

int __init cardhu_edp_init(void)
{
	unsigned int regulator_mA;

	regulator_mA = get_maximum_cpu_current_supported();
	if (!regulator_mA) {
		regulator_mA = 6000; /* regular T30/s */
	}
	pr_info("%s: CPU regulator %d mA\n", __func__, regulator_mA);

	tegra_init_cpu_edp_limits(regulator_mA);
	return 0;
}
#endif

unsigned int boot_reason = 0;
void tegra_booting_info(void)
{
	static void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	unsigned int reg;
#define PMC_RST_STATUS_WDT (1)
#define PMC_RST_STATUS_SW  (3)

	reg = readl(pmc +0x1b4);
	printk("tegra_booting_info reg=%x\n",reg);

	if (reg == PMC_RST_STATUS_SW) {
		boot_reason=PMC_RST_STATUS_SW;
		printk("tegra_booting_info-SW reboot\n");
	} else if (reg ==PMC_RST_STATUS_WDT) {
		boot_reason=PMC_RST_STATUS_WDT;
		printk("tegra_booting_info-watchdog reboot\n");
	} else {
		boot_reason=0;
		printk("tegra_booting_info-normal\n");
	}
}
