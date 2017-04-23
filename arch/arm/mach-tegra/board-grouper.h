/*
 * arch/arm/mach-tegra/board-grouper.h
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

#ifndef _MACH_TEGRA_BOARD_GROUPER_H
#define _MACH_TEGRA_BOARD_GROUPER_H

#include <mach/irqs.h>
#include <linux/mfd/max77663-core.h>
#include "gpio-names.h"

/* External peripheral act as gpio */
/* TPS6591x GPIOs */
#define TPS6591X_GPIO_BASE	TEGRA_NR_GPIOS
#define TPS6591X_GPIO_0		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP0)
#define TPS6591X_GPIO_6		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP6)
#define TPS6591X_GPIO_7		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP7)
#define TPS6591X_GPIO_8		(TPS6591X_GPIO_BASE + TPS6591X_GPIO_GP8)

/* MAX77663 GPIO */
#define MAX77663_GPIO_BASE	TEGRA_NR_GPIOS

/* Camera related GPIOs */
#define CAM2_LDO_EN_GPIO	TEGRA_GPIO_PR6
#define CAM2_RST_GPIO		TEGRA_GPIO_PO0
#define CAM2_RST_GPIO_BACH	TEGRA_GPIO_PBB0

/* Audio-related GPIOs */
#define TEGRA_GPIO_CDC_IRQ		TEGRA_GPIO_PW3
#define TEGRA_GPIO_SPKR_EN		-1
#define TEGRA_GPIO_HP_DET		TEGRA_GPIO_PW2
#define TEGRA_GPIO_INT_MIC_EN		TEGRA_GPIO_PK3
#define TEGRA_GPIO_EXT_MIC_EN		TEGRA_GPIO_PK4

/* Tegra Modem related GPIOs */
#define TEGRA_GPIO_W_DISABLE		TEGRA_GPIO_PDD7
#define TEGRA_GPIO_MODEM_RSVD1		TEGRA_GPIO_PV0
#define TEGRA_GPIO_MODEM_RSVD2		TEGRA_GPIO_PH7

/* TPS6591x IRQs */
#define TPS6591X_IRQ_BASE	TEGRA_NR_IRQS

/* MAX77663 IRQs */
#define MAX77663_IRQ_BASE	TEGRA_NR_IRQS
#define MAX77663_IRQ_END	(MAX77663_IRQ_BASE + MAX77663_IRQ_NR)
#define MAX77663_IRQ_ACOK_RISING MAX77663_IRQ_ONOFF_ACOK_RISING
#define MAX77663_IRQ_ACOK_FALLING MAX77663_IRQ_ONOFF_ACOK_FALLING

/* UART port which is used by bluetooth */
#define BLUETOOTH_UART_DEV_NAME "/dev/ttyHS2"

int __init grouper_charge_init(void);
int __init grouper_sdhci_init(void);
int __init grouper_pinmux_init(void);
void __init grouper_pinmux_init_early(void);
int __init grouper_panel_init(void);
int __init grouper_sensors_init(void);
int __init grouper_keys_init(void);
int __init grouper_pins_state_init(void);
int __init grouper_emc_init(void);
int __init touch_init_synaptics_grouper(void);
void __init grouper_edp_init(void);
void __init grouper_regulator_init(void);
void __init grouper_tps6591x_regulator_init(void);
void __init grouper_max77663_regulator_init(void);
void __init grouper_suspend_init(void);
void __init grouper_tsensor_init(void);

#define SYNAPTICS_ATTN_GPIO		TEGRA_GPIO_PZ3
#define SYNAPTICS_RESET_GPIO	TEGRA_GPIO_PN5

#define GROUPER_TS_ID1		TEGRA_GPIO_PI7
#define GROUPER_TS_ID2		TEGRA_GPIO_PC7
#define GROUPER_TS_ID1_PG	TEGRA_PINGROUP_GMI_WAIT
#define GROUPER_TS_ID2_PG	TEGRA_PINGROUP_GMI_WP_N

/* Invensense MPU Definitions */
#define MPU_GYRO_NAME		"mpu6050"
#define MPU_GYRO_IRQ_GPIO	TEGRA_GPIO_PX1
#define MPU_GYRO_ADDR		0x68
#define MPU_GYRO_BUS_NUM	2
#define MPU_GYRO_ORIENTATION	{ 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#define MPU_COMPASS_NAME	"ami306"
#define MPU_COMPASS_IRQ_GPIO	TEGRA_GPIO_PW0
#define MPU_COMPASS_ADDR	0x0E
#define MPU_COMPASS_BUS_NUM	2
#define MPU_COMPASS_ORIENTATION	{ 0, -1, 0, -1, 0, 0, 0, 0, -1 }

#define GROUPER_TEMP_ALERT_GPIO		TEGRA_GPIO_PS3

#define EN_HSIC_GPIO			TEGRA_GPIO_PR7

#define XMM_GPIO_BB_ON			TEGRA_GPIO_PX7
#define XMM_GPIO_BB_RST			TEGRA_GPIO_PU3
#define XMM_GPIO_IPC_HSIC_ACTIVE	TEGRA_GPIO_PX0
#define XMM_GPIO_IPC_HSIC_SUS_REQ	TEGRA_GPIO_PY3
#define XMM_GPIO_IPC_BB_WAKE		TEGRA_GPIO_PY2
#define XMM_GPIO_IPC_AP_WAKE		TEGRA_GPIO_PU5

#define XMM_GPIO_BB_VBAT			TEGRA_GPIO_PC6
#define XMM_GPIO_BB_VBUS			TEGRA_GPIO_PD2
#define XMM_GPIO_BB_SW_SEL			TEGRA_GPIO_PP1
#define XMM_GPIO_IPC_BB_RST_IND		TEGRA_GPIO_PEE1
#define XMM_GPIO_SIM_CARD_DET		TEGRA_GPIO_PW3
#define XMM_GPIO_IPC_BB_FORCE_CRASH		TEGRA_GPIO_PN1

#endif
