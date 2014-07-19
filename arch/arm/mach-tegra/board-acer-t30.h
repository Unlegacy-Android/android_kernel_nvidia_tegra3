/*
 * arch/arm/mach-tegra/board-acer-t30.h
 *
 * Copyright (c) 2011, NVIDIA Corporation.
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

#ifndef _MACH_TEGRA_BOARD_ACER_T30_H
#define _MACH_TEGRA_BOARD_ACER_T30_H

#include <mach/gpio.h>
#include <mach/irqs.h>
#include <linux/mfd/tps6591x.h>
#include <linux/mfd/ricoh583.h>

/* Processor Board  ID */
#define BOARD_E1187   0x0B57
#define BOARD_E1186   0x0B56
#define BOARD_E1198   0x0B62
#define BOARD_E1256   0x0C38
#define BOARD_E1257   0x0C39
#define BOARD_E1291   0x0C5B
#define BOARD_PM267   0x0243
#define BOARD_PM269   0x0245
#define BOARD_E1208   0x0C08
#define BOARD_PM305   0x0305
#define BOARD_PM311   0x030B
#define BOARD_PMU_PM298   0x0262
#define BOARD_PMU_PM299   0x0263

/* SKU Information */
#define BOARD_SKU_B11	0xb11

#define SKU_DCDC_TPS62361_SUPPORT	0x1
#define SKU_SLT_ULPI_SUPPORT		0x2
#define SKU_T30S_SUPPORT		0x4
#define SKU_TOUCHSCREEN_MECH_FIX	0x0100

#define SKU_TOUCH_MASK			0xFF00
#define SKU_TOUCH_2000			0x0B00

#define SKU_MEMORY_TYPE_BIT		0x3
#define SKU_MEMORY_TYPE_MASK		0x7
/* If BOARD_PM269 */
#define SKU_MEMORY_SAMSUNG_EC		0x0
#define SKU_MEMORY_ELPIDA		0x2
#define SKU_MEMORY_SAMSUNG_EB		0x4
/* If BOARD_PM272 */
#define SKU_MEMORY_1GB_1R_HYNIX		0x0
#define SKU_MEMORY_2GB_2R_HYH9		0x2
/* If other BOARD_ variants */
#define SKU_MEMORY_CARDHU_1GB_1R	0x0
#define SKU_MEMORY_CARDHU_2GB_2R	0x2
#define SKU_MEMORY_CARDHU_2GB_1R_HYK0	0x4
#define SKU_MEMORY_CARDHU_2GB_1R_HYH9	0x6
#define SKU_MEMORY_CARDHU_2GB_1R_HYNIX	0x1
#define MEMORY_TYPE(sku) (((sku) >> SKU_MEMORY_TYPE_BIT) & SKU_MEMORY_TYPE_MASK)

/* Board Fab version */
#define BOARD_FAB_A00			0x0
#define BOARD_FAB_A01			0x1
#define BOARD_FAB_A02			0x2
#define BOARD_FAB_A03			0x3
#define BOARD_FAB_A04			0x4
#define BOARD_FAB_A05			0x5
#define BOARD_FAB_A06			0x6
#define BOARD_FAB_A07			0x7

/* External peripheral act as gpio */
/* TPS6591x GPIOs */
#define TPS6591X_GPIO_BASE	TEGRA_NR_GPIOS
#define TPS6591X_GPIO_0		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP0)
#define TPS6591X_GPIO_1		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP1)
#define TPS6591X_GPIO_2		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP2)
#define TPS6591X_GPIO_3		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP3)
#define TPS6591X_GPIO_4		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP4)
#define TPS6591X_GPIO_5		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP5)
#define TPS6591X_GPIO_6		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP6)
#define TPS6591X_GPIO_7		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP7)
#define TPS6591X_GPIO_8		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP8)
#define TPS6591X_GPIO_END	(TPS6591X_GPIO_BASE + TPS6591X_GPIO_NR)

/* WM8903 GPIOs */
#define CARDHU_GPIO_WM8903(_x_)		(TPS6591X_GPIO_END + (_x_))
#define CARDHU_GPIO_WM8903_END		CARDHU_GPIO_WM8903(4)

/* Audio-related GPIOs */
#define TEGRA_GPIO_CDC_IRQ		TEGRA_GPIO_PW3
#define TEGRA_GPIO_SPKR_EN		CARDHU_GPIO_WM8903(2)
#define TEGRA_GPIO_HP_DET		TEGRA_GPIO_PW2
#define TEGRA_GPIO_BYPASS_SWITCH_EN	TEGRA_GPIO_PN3
#define TEGRA_GPIO_DEBUG_SWITCH_EN	TEGRA_GPIO_PR5

/* Sensor GPIOs*/
#define SENSOR_3V3			TEGRA_GPIO_PH7
#define SENSOR_3V3_2			TEGRA_GPIO_PP3
#define EN_SENSOR_VLOGIC		TEGRA_GPIO_PY3
#define GYRO_INT_R			TEGRA_GPIO_PX1
#define G_ACC_INT			TEGRA_GPIO_PS5
#define COMPASS_DRDY			TEGRA_GPIO_PX7

/* CAMERA RELATED GPIOs on CARDHU */
#define OV5650_RESETN_GPIO			TEGRA_GPIO_PBB0
#define CAM1_POWER_DWN_GPIO			TEGRA_GPIO_PBB5
#define CAM2_POWER_DWN_GPIO			TEGRA_GPIO_PBB6
#define CAM3_POWER_DWN_GPIO			TEGRA_GPIO_PBB7
#define CAMERA_CSI_CAM_SEL_GPIO		TEGRA_GPIO_PBB4
#define CAMERA_CSI_MUX_SEL_GPIO		TEGRA_GPIO_PCC1
#define CAM1_LDO_EN_GPIO			TEGRA_GPIO_PR6
#define CAM2_LDO_EN_GPIO			TEGRA_GPIO_PR7
#define CAM3_LDO_EN_GPIO			TEGRA_GPIO_PS0
#define OV14810_RESETN_GPIO			TEGRA_GPIO_PBB0

#define CAMERA_FLASH_SYNC_GPIO		TEGRA_GPIO_PBB3
#define CAMERA_FLASH_MAX_TORCH_AMP	7
#define CAMERA_FLASH_MAX_FLASH_AMP	7

#define AC_PRESENT_GPIO		TPS6591X_GPIO_4

/*****************Interrupt tables ******************/
/* External peripheral act as interrupt controller */
/* TPS6591x IRQs */
#define TPS6591X_IRQ_BASE	TEGRA_NR_IRQS
#define TPS6591X_IRQ_END	(TPS6591X_IRQ_BASE + 18)
#define DOCK_DETECT_GPIO TEGRA_GPIO_PU4

/* RICOH583 IRQs */
#define RICOH583_IRQ_BASE	TEGRA_NR_IRQS
#define RICOH583_IRQ_END	(RICOH583_IRQ_BASE + RICOH583_NR_IRQS)

/* MAX77663 IRQs */
#define MAX77663_IRQ_BASE	TEGRA_NR_IRQS
#define MAX77663_IRQ_END	(MAX77663_IRQ_BASE + MAX77663_IRQ_NR)

int cardhu_charge_init(void);
int cardhu_regulator_init(void);
int cardhu_suspend_init(void);
int cardhu_sdhci_init(void);
int cardhu_pinmux_init(void);
int cardhu_gpio_init(void);
int acer_panel_init(void);
int cardhu_sensors_init(void);
int cardhu_kbc_init(void);
int cardhu_scroll_init(void);
int cardhu_keys_init(void);
int cardhu_pins_state_init(void);
int cardhu_emc_init(void);
int cardhu_edp_init(void);
int acer_keys_init(void);
int cardhu_pmon_init(void);
int cardhu_pm298_gpio_switch_regulator_init(void);
int cardhu_pm298_regulator_init(void);
int cardhu_pm299_gpio_switch_regulator_init(void);
int cardhu_pm299_regulator_init(void);

#define MPU_TYPE_MPU3050	1
#define MPU_TYPE_MPU6050	2
#define MPU_GYRO_TYPE		MPU_TYPE_MPU3050
#define MPU_GYRO_IRQ_GPIO	TEGRA_GPIO_PX1
#define MPU_GYRO_ADDR		0x68
#define MPU_GYRO_BUS_NUM	2
#define MPU_GYRO_ORIENTATION	{ 0, -1, 0, -1, 0, 0, 0, 0, -1 }
#define MPU_ACCEL_NAME		"kxtf9"
#define MPU_ACCEL_IRQ_GPIO	TEGRA_GPIO_PL1
#define MPU_ACCEL_ADDR		0x0F
#define MPU_ACCEL_BUS_NUM	2
#define MPU_ACCEL_ORIENTATION	{ 0, -1, 0, -1, 0, 0, 0, 0, -1 }
#define MPU_COMPASS_NAME	"ak8975"
#define MPU_COMPASS_IRQ_GPIO	0
#define MPU_COMPASS_ADDR	0x0C
#define MPU_COMPASS_BUS_NUM	2
#define MPU_COMPASS_ORIENTATION	{ 1, 0, 0, 0, 1, 0, 0, 0, 1 }

/* Baseband GPIO addresses */
#define BB_GPIO_BB_EN			TEGRA_GPIO_PR5
#define BB_GPIO_BB_RST			TEGRA_GPIO_PS4
#define BB_GPIO_SPI_INT			TEGRA_GPIO_PS6
#define BB_GPIO_SPI_SS			TEGRA_GPIO_PV0
#define BB_GPIO_AWR			TEGRA_GPIO_PS7
#define BB_GPIO_CWR			TEGRA_GPIO_PU5

#define XMM_GPIO_BB_ON			BB_GPIO_BB_EN
#define XMM_GPIO_BB_RST			BB_GPIO_BB_RST
#define XMM_GPIO_IPC_HSIC_ACTIVE	BB_GPIO_SPI_INT
#define XMM_GPIO_IPC_HSIC_SUS_REQ	BB_GPIO_SPI_SS
#define XMM_GPIO_IPC_BB_WAKE		BB_GPIO_AWR
#define XMM_GPIO_IPC_AP_WAKE		BB_GPIO_CWR

/* Board Type */
#define BOARD_PICASSO_2  0
#define BOARD_PICASSO_M  1
#define BOARD_PICASSO_MF 2

/* Board ID */
#define BOARD_EVT       0
#define BOARD_DVT1      1
#define BOARD_DVT2      2
#define BOARD_PVT1      3
#define BOARD_PVT2      4
#define BOARD_PRE_MP    5

/* SKU type */
#define BOARD_SKU_WIFI  0
#define BOARD_SKU_3G    1
#define BOARD_SKU_LTE   2

/* Wifi module */
#define BOARD_WIFI_AH663 0
#define BOARD_WIFI_NH660 1
#endif
