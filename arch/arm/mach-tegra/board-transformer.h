/*
 * arch/arm/mach-tegra/board-transformer.h
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

#ifndef _MACH_TEGRA_BOARD_CARDHU_H
#define _MACH_TEGRA_BOARD_CARDHU_H

#include <mach/irqs.h>
#include <linux/mfd/tps6591x.h>
#include "gpio-names.h"

/* External peripheral act as gpio */
/* TPS6591x GPIOs */
#define TPS6591X_GPIO_BASE			TEGRA_NR_GPIOS
#define TPS6591X_GPIO_0				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP0)
#define TPS6591X_GPIO_1				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP1)
#define TPS6591X_GPIO_2				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP2)
#define TPS6591X_GPIO_3				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP3)
#define TPS6591X_GPIO_4				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP4)
#define TPS6591X_GPIO_5				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP5)
#define TPS6591X_GPIO_6				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP6)
#define TPS6591X_GPIO_7				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP7)
#define TPS6591X_GPIO_8				(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP8)
#define TPS6591X_GPIO_END			(TPS6591X_GPIO_BASE + TPS6591X_GPIO_NR)

/* PMU_TCA6416 GPIO assignment */
#define EN_HSIC_GPIO				TEGRA_GPIO_PR7  /* PMU_GPIO25 */

/* WM8903 GPIOs */
#define CARDHU_GPIO_WM8903(_x_)			(TPS6591X_GPIO_END + 32 + (_x_))
#define CARDHU_GPIO_WM8903_END			CARDHU_GPIO_WM8903(4)

/* Audio-related GPIOs */
#define TEGRA_GPIO_CDC_IRQ			TEGRA_GPIO_PW3
#define TEGRA_GPIO_SPKR_EN			CARDHU_GPIO_WM8903(2)
#define TEGRA_GPIO_HP_DET			TEGRA_GPIO_PW2

/* CAMERA RELATED GPIOs on TF201 */
#define ISP_POWER_1V2_EN_GPIO			TEGRA_GPIO_PS3      //ISP_1V2_EN VDD_ISP_1V2
#define ISP_POWER_RESET_GPIO			TEGRA_GPIO_PBB0     //CAM_RST_5M, RSTX
#define CAM3_POWER_DWN_GPIO			TEGRA_GPIO_PBB7
#define FRONT_YUV_SENSOR_RST_GPIO		TEGRA_GPIO_PO0      //1.2M CAM_RST

/* CAMERA RELATED GPIOs on TF700T */
#define TF700T_ISP_POWER_1V2_EN_GPIO		TEGRA_GPIO_PR7      //ISP_1V2_EN VDD_ISP_1V2
#define TF700T_ISP_POWER_1V8_EN_GPIO		TEGRA_GPIO_PBB7     //ISP_1V8_EN VDD_ISP_1V8

/* CAMERA RELATED GPIOs on TF300T */
#define ICATCH7002A_RST_GPIO			TEGRA_GPIO_PBB0
#define ICATCH7002A_VDDIO_EN_GPIO		TEGRA_GPIO_PBB4
#define ICATCH7002A_PWR_DN_GPIO			TEGRA_GPIO_PBB5
#define ICATCH7002A_VDDC_EN_GPIO		TEGRA_GPIO_PBB7

/*****************Interrupt tables ******************/
/* External peripheral act as interrupt controller */
/* TPS6591x IRQs */
#define TPS6591X_IRQ_BASE			TEGRA_NR_IRQS
#define TPS6591X_IRQ_END			(TPS6591X_IRQ_BASE + 18)
#define DOCK_DETECT_GPIO			TEGRA_GPIO_PU4

int __init cardhu_regulator_init(void);
void __init cardhu_suspend_init(void);
int __init cardhu_sdhci_init(void);
int __init cardhu_pinmux_init(void);
void __init cardhu_pinmux_init_early(void);
int __init cardhu_gpio_init(void);
int __init cardhu_panel_init(void);
int __init cardhu_sensors_init(void);
int __init cardhu_keys_init(void);
int __init cardhu_pins_state_init(void);
int __init cardhu_emc_init(void);
void __init cardhu_edp_init(void);
//struct platform_device *tegra_cardhu_usb_utmip_host_register(void);
//void tegra_cardhu_usb_utmip_host_unregister(struct platform_device *pdev);

/* Invensense MPU Definitions */
#define MPU_GYRO_NAME				"mpu3050"
#define MPU_GYRO_IRQ_GPIO			TEGRA_GPIO_PX1
#define MPU_GYRO_ADDR				0x68
#define MPU_GYRO_BUS_NUM			2

#define MPU_ACCEL_NAME				"kxtf9"
#define MPU_ACCEL_IRQ_GPIO			TEGRA_GPIO_PO5
#define MPU_ACCEL_ADDR				0x0F
#define MPU_ACCEL_BUS_NUM			2

#define MPU_COMPASS_NAME			"ami306"
#define MPU_COMPASS_IRQ_GPIO			TEGRA_GPIO_PW0
#define MPU_COMPASS_ADDR			0x0E
#define MPU_COMPASS_BUS_NUM			2

//Sensors orientation matrix for TF201
#define MPU_GYRO_ORIENTATION			{ 0, -1, 0, -1, 0, 0, 0, 0, -1 }
#define MPU_ACCEL_ORIENTATION			{ -1, 0, 0, 0, 1, 0, 0, 0, -1 }
#define MPU_COMPASS_ORIENTATION			{ -1, 0, 0, 0, 1, 0, 0, 0, -1 }

//Sensors orientation matrix for TF300T
#define TF300T_GYRO_ORIENTATION			{ -1, 0, 0, 0, 1, 0, 0, 0, -1 }
#define TF300T_ACCEL_ORIENTATION		{ 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#define TF300T_COMPASS_ORIENTATION		{ 0, -1, 0, -1, 0, 0, 0, 0, -1 }

//Sensors orientation matrix for TF300TG
#define TF300TG_GYRO_ORIENTATION		{ -1, 0, 0, 0, 1, 0, 0, 0, -1 }
#define TF300TG_ACCEL_ORIENTATION		{ 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#define TF300TG_COMPASS_ORIENTATION		{ 1, 0, 0, 0, -1, 0, 0, 0, -1 }

//Sensors orientation matrix for TF700T
#define TF700T_GYRO_ORIENTATION			{ 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#define TF700T_ACCEL_ORIENTATION		{ 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#define TF700T_COMPASS_ORIENTATION		{ 1, 0, 0, 0, -1, 0, 0, 0, -1 }

//Sensors orientation matrix for TF300TL
#define TF300TL_GYRO_ORIENTATION		{ -1, 0, 0, 0, 1, 0, 0, 0, -1 }
#define TF300TL_ACCEL_ORIENTATION		{ 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#define TF300TL_COMPASS_ORIENTATION		{ -1, 0, 0, 0, -1, 0, 0, 0, 1 }

/* Baseband GPIO addresses */
#define XMM_GPIO_BB_ON				TEGRA_GPIO_PX7
#define XMM_GPIO_BB_RST				TEGRA_GPIO_PU3
#define XMM_GPIO_IPC_HSIC_ACTIVE		TEGRA_GPIO_PX0
#define XMM_GPIO_IPC_HSIC_SUS_REQ		TEGRA_GPIO_PY3
#define XMM_GPIO_IPC_BB_WAKE			TEGRA_GPIO_PY2
#define XMM_GPIO_IPC_AP_WAKE			TEGRA_GPIO_PU5

/* Asus baseband GPIO addresses */
#define XMM_GPIO_BB_VBAT			TEGRA_GPIO_PC6
#define XMM_GPIO_BB_VBUS			TEGRA_GPIO_PD2
#define XMM_GPIO_BB_SW_SEL			TEGRA_GPIO_PP1
#define XMM_GPIO_IPC_BB_RST_IND			TEGRA_GPIO_PEE1
#define BB_GPIO_SAR_DET				TEGRA_GPIO_PR3  //SAR_DET#_3G
#define XMM_GPIO_SIM_CARD_DET			TEGRA_GPIO_PW3
#define XMM_GPIO_IPC_BB_FORCE_CRASH		TEGRA_GPIO_PN1

enum tegra_bb_type {
	TEGRA_BB_TANGO = 1,
};

#endif
