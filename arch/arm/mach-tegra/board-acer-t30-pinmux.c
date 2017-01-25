/*
 * arch/arm/mach-tegra/board-acer-t30-pinmux.c
 *
 * Copyright (C) 2011-2012, NVIDIA Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <mach/pinmux.h>
#include <mach/pinmux-tegra30.h>
#include <mach/gpio-tegra.h>
#include "board.h"
#include "board-acer-t30.h"
#include "devices.h"
#include "gpio-names.h"

extern int acer_board_id;
extern int acer_board_type;
extern int acer_sku;

#define DEFAULT_DRIVE(_name)					\
	{							\
		.pingroup = TEGRA_DRIVE_PINGROUP_##_name,	\
		.hsm = TEGRA_HSM_DISABLE,			\
		.schmitt = TEGRA_SCHMITT_ENABLE,		\
		.drive = TEGRA_DRIVE_DIV_1,			\
		.pull_down = TEGRA_PULL_31,			\
		.pull_up = TEGRA_PULL_31,			\
		.slew_rising = TEGRA_SLEW_SLOWEST,		\
		.slew_falling = TEGRA_SLEW_SLOWEST,		\
	}
/* Setting the drive strength of pins
 * hsm: Enable High speed mode (ENABLE/DISABLE)
 * Schimit: Enable/disable schimit (ENABLE/DISABLE)
 * drive: low power mode (DIV_1, DIV_2, DIV_4, DIV_8)
 * pulldn_drive - drive down (falling edge) - Driver Output Pull-Down drive
 *                strength code. Value from 0 to 31.
 * pullup_drive - drive up (rising edge)  - Driver Output Pull-Up drive
 *                strength code. Value from 0 to 31.
 * pulldn_slew -  Driver Output Pull-Up slew control code  - 2bit code
 *                code 11 is least slewing of signal. code 00 is highest
 *                slewing of the signal.
 *                Value - FASTEST, FAST, SLOW, SLOWEST
 * pullup_slew -  Driver Output Pull-Down slew control code -
 *                code 11 is least slewing of signal. code 00 is highest
 *                slewing of the signal.
 *                Value - FASTEST, FAST, SLOW, SLOWEST
 */
#define SET_DRIVE(_name, _hsm, _schmitt, _drive, _pulldn_drive, _pullup_drive, _pulldn_slew, _pullup_slew) \
	{                                               \
		.pingroup = TEGRA_DRIVE_PINGROUP_##_name,   \
		.hsm = TEGRA_HSM_##_hsm,                    \
		.schmitt = TEGRA_SCHMITT_##_schmitt,        \
		.drive = TEGRA_DRIVE_##_drive,              \
		.pull_down = TEGRA_PULL_##_pulldn_drive,    \
		.pull_up = TEGRA_PULL_##_pullup_drive,		\
		.slew_rising = TEGRA_SLEW_##_pulldn_slew,   \
		.slew_falling = TEGRA_SLEW_##_pullup_slew,	\
	}

/* !!!FIXME!!!! POPULATE THIS TABLE */
static __initdata struct tegra_drive_pingroup_config cardhu_drive_pinmux[] = {
	/* DEFAULT_DRIVE(<pin_group>), */
	/* SET_DRIVE(ATA, DISABLE, DISABLE, DIV_1, 31, 31, FAST, FAST) */
	SET_DRIVE(DAP2, 	DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),
	SET_DRIVE(DAP1, 	DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* All I2C pins are driven to maximum drive strength */
	/* GEN1 I2C */
	SET_DRIVE(DBG,		DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* GEN2 I2C */
	SET_DRIVE(AT5,		DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* CAM I2C */
	SET_DRIVE(GME,		DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* DDC I2C */
	SET_DRIVE(DDC,		DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* PWR_I2C */
	SET_DRIVE(AO1,		DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* UART3 */
	SET_DRIVE(UART3,	DISABLE, ENABLE, DIV_1, 31, 31, FASTEST, FASTEST),

	/* SDMMC1 */
	SET_DRIVE(SDIO1,	DISABLE, DISABLE, DIV_1, 46, 42, FAST, FAST),

	/* SDMMC3 */
	SET_DRIVE(SDIO3,	DISABLE, DISABLE, DIV_1, 46, 42, FAST, FAST),

	/* SDMMC4 */
	SET_DRIVE(GMA,		DISABLE, DISABLE, DIV_1, 9, 9, SLOWEST, SLOWEST),
	SET_DRIVE(GMB,		DISABLE, DISABLE, DIV_1, 9, 9, SLOWEST, SLOWEST),
	SET_DRIVE(GMC,		DISABLE, DISABLE, DIV_1, 9, 9, SLOWEST, SLOWEST),
	SET_DRIVE(GMD,		DISABLE, DISABLE, DIV_1, 9, 9, SLOWEST, SLOWEST),

};

#define CEC_PINMUX(_pingroup, _mux, _pupd, _tri, _io, _lock, _od) \
	{                                                       \
		.pingroup       = TEGRA_PINGROUP_##_pingroup,   \
			.func           = TEGRA_MUX_##_mux,             \
			.pupd           = TEGRA_PUPD_##_pupd,           \
			.tristate       = TEGRA_TRI_##_tri,             \
			.io             = TEGRA_PIN_##_io,              \
			.lock           = TEGRA_PIN_LOCK_##_lock,       \
			.od             = TEGRA_PIN_OD_##_od,           \
			.ioreset        = TEGRA_PIN_IO_RESET_DEFAULT,   \
	}

#define DEFAULT_PINMUX(_pingroup, _mux, _pupd, _tri, _io)		\
	{								\
		.pingroup	= TEGRA_PINGROUP_##_pingroup,		\
		.func		= TEGRA_MUX_##_mux,			\
		.pupd		= TEGRA_PUPD_##_pupd,			\
		.tristate	= TEGRA_TRI_##_tri,			\
		.io		= TEGRA_PIN_##_io,			\
		.lock		= TEGRA_PIN_LOCK_DEFAULT,		\
		.od		= TEGRA_PIN_OD_DEFAULT,			\
		.ioreset	= TEGRA_PIN_IO_RESET_DEFAULT,		\
	}

#define I2C_PINMUX(_pingroup, _mux, _pupd, _tri, _io, _lock, _od)	\
	{								\
		.pingroup	= TEGRA_PINGROUP_##_pingroup,		\
		.func		= TEGRA_MUX_##_mux,			\
		.pupd		= TEGRA_PUPD_##_pupd,			\
		.tristate	= TEGRA_TRI_##_tri,			\
		.io		= TEGRA_PIN_##_io,			\
		.lock		= TEGRA_PIN_LOCK_##_lock,		\
		.od		= TEGRA_PIN_OD_##_od,			\
		.ioreset	= TEGRA_PIN_IO_RESET_DEFAULT,		\
	}

#define VI_PINMUX(_pingroup, _mux, _pupd, _tri, _io, _lock, _ioreset)	\
	{								\
		.pingroup	= TEGRA_PINGROUP_##_pingroup,		\
		.func		= TEGRA_MUX_##_mux,			\
		.pupd		= TEGRA_PUPD_##_pupd,			\
		.tristate	= TEGRA_TRI_##_tri,			\
		.io		= TEGRA_PIN_##_io,			\
		.lock		= TEGRA_PIN_LOCK_##_lock,		\
		.od		= TEGRA_PIN_OD_DEFAULT,			\
		.ioreset	= TEGRA_PIN_IO_RESET_##_ioreset		\
	}

static __initdata struct tegra_pingroup_config picasso2_pinmux_common[] = {
	DEFAULT_PINMUX(GMI_WP_N,        GMI,             PULL_UP,       NORMAL,     INPUT), //
	DEFAULT_PINMUX(CLK1_OUT,        EXTPERIPH1,      NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(DAP1_DIN,        I2S0,            PULL_DOWN,     NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(DAP1_DOUT,       I2S0,            PULL_DOWN,     TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(DAP1_FS,         I2S0,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(DAP1_SCLK,       I2S0,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(DAP2_DIN,        I2S1,            NORMAL,        NORMAL,     INPUT), ////// check config state
	DEFAULT_PINMUX(DAP2_DOUT,       I2S1,            NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(DAP2_FS,         I2S1,            NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(DAP2_SCLK,       I2S1,            NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(SPDIF_IN,        SPDIF,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SPDIF_OUT,       SPDIF,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SPI1_CS0_N,      SPI1,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SPI1_MISO,       SPI1,            PULL_DOWN,     TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(SPI1_MOSI,       SPI1,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SPI1_SCK,        SPI1,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SPI2_CS0_N,      SPI2,            NORMAL,        TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(SPI2_CS1_N,      SPI2,            PULL_UP,       TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(SPI2_CS2_N,      SPI2,            PULL_DOWN,     TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(SPI2_MISO,       SPI2,            NORMAL,        TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(SPI2_MOSI,       SPI2,            NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(SPI2_SCK,        SPI2,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(DAP3_DIN,        I2S2,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(DAP3_DOUT,       RSVD1,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(DAP3_FS,         RSVD1,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(DAP3_SCLK,       I2S2,            PULL_DOWN,     NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GPIO_PV0,        RSVD,            NORMAL,        TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(GPIO_PV1,        RSVD,            NORMAL,        TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(ULPI_CLK,        UARTD,           NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(ULPI_DATA0,      UARTA,           NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(ULPI_DATA1,      UARTA,           NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(ULPI_DATA2,      UARTA,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(ULPI_DATA3,      UARTA,           NORMAL,        TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(ULPI_DATA4,      UARTA,           PULL_UP,       NORMAL,     INPUT), // GI
	DEFAULT_PINMUX(ULPI_DATA5,      UARTA,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(ULPI_DATA6,      UARTA,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(ULPI_DATA7,      UARTA,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(ULPI_DIR,        UARTD,           NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(ULPI_NXT,        UARTD,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(ULPI_STP,        UARTD,           NORMAL,        NORMAL,     OUTPUT), //G

	/* I2C3 pinmux */
	I2C_PINMUX(CAM_I2C_SCL,		I2C3,		 NORMAL,        NORMAL,     INPUT,   DISABLE,   ENABLE),
	I2C_PINMUX(CAM_I2C_SDA,		I2C3,		 NORMAL,        NORMAL,     INPUT,   DISABLE,   ENABLE),

	DEFAULT_PINMUX(CAM_MCLK,        VI_ALT2,         NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(GPIO_PBB0,       RSVD1,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GPIO_PBB3,       VGP3,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GPIO_PBB4,       VGP4,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GPIO_PBB5,       VGP5,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GPIO_PBB6,       VGP6,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GPIO_PBB7,       I2S4,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(GPIO_PCC1,       RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(GPIO_PCC2,       I2S4,            NORMAL,        NORMAL,     OUTPUT), // G (NC)

	/* I2C2 pinmux */
	I2C_PINMUX(GEN2_I2C_SCL,	I2C2,		NORMAL,	NORMAL,	INPUT,	DISABLE,	ENABLE),
	I2C_PINMUX(GEN2_I2C_SDA,	I2C2,		NORMAL,	NORMAL,	INPUT,	DISABLE,	ENABLE),

	DEFAULT_PINMUX(GMI_A16,         SPI4,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_A17,         SPI4,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_A18,         SPI4,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_A19,         RSVD3,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_AD0,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD1,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD2,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD3,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD4,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD5,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD6,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD7,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD8,         PWM0,            NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(GMI_AD9,         NAND,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_AD10,        NAND,            PULL_DOWN,     NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_AD11,        NAND,            NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_AD12,        NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD13,        NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD14,        NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_AD15,        NAND,            NORMAL,        NORMAL,     INPUT), //G
	DEFAULT_PINMUX(GMI_ADV_N,       NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CLK,         NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS0_N,       NAND,            PULL_UP,       TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS1_N,       NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS2_N,       RSVD1,           NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS3_N,       NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS4_N,       NAND,            PULL_UP,       TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS6_N,       NAND_ALT,        NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_CS7_N,       NAND_ALT,        NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_DQS,         RSVD1,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_IORDY,       RSVD1,           NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_OE_N,        NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(GMI_RST_N,       RSVD3,           NORMAL,        NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_WAIT,        NAND,            PULL_UP,       NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(GMI_WP_N,        RSVD1,           PULL_UP,       NORMAL,     INPUT), //GI
	DEFAULT_PINMUX(GMI_WR_N,        NAND,            NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(CRT_HSYNC,       CRT,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(CRT_VSYNC,       CRT,             NORMAL,        NORMAL,     OUTPUT), // G (NC)

	/* I2C4 pinmux */
	I2C_PINMUX(DDC_SCL,		I2C4,		NORMAL,	NORMAL,	INPUT,	DISABLE,	ENABLE),
	I2C_PINMUX(DDC_SDA,		I2C4,		NORMAL,	NORMAL,	INPUT,	DISABLE,	ENABLE),

	DEFAULT_PINMUX(HDMI_INT,        RSVD0,           NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(LCD_CS0_N,       DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_CS1_N,       DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_D0,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D1,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D2,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D3,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D4,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D5,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D6,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D7,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D8,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D9,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D10,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D11,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D12,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D13,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D14,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D15,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D16,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D17,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D18,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D19,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D20,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D21,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D22,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_D23,         DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_DC0,         DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_DC1,         DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_DE,          DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_HSYNC,       DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_M1,          DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_PCLK,        DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_PWR0,        DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_PWR1,        DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_PWR2,        DISPLAYA,        PULL_DOWN,     NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(LCD_SCK,         DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_SDIN,        DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_SDOUT,       DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(LCD_VSYNC,       DISPLAYA,        NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(LCD_WR_N,        DISPLAYA,        NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(PEX_L2_CLKREQ_N, PCIE,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(PEX_L2_PRSNT_N,  PCIE,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(PEX_L2_RST_N,    PCIE,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(PEX_WAKE_N,      PCIE,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(CLK2_OUT,        EXTPERIPH2,      PULL_UP,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(CLK2_REQ,        DAP,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(GPIO_PV2,        OWR,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(GPIO_PV3,        RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SDMMC1_CLK,      SDMMC1,          NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC1_CMD,      SDMMC1,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC1_DAT0,     SDMMC1,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC1_DAT1,     SDMMC1,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC1_DAT2,     SDMMC1,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC1_DAT3,     SDMMC1,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_CLK,      SDMMC3,          NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_CMD,      SDMMC3,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_DAT0,     SDMMC3,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_DAT1,     SDMMC3,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_DAT2,     SDMMC3,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_DAT3,     SDMMC3,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC3_DAT4,     SDMMC3,          NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SDMMC3_DAT5,     SDMMC3,          NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SDMMC3_DAT6,     SDMMC3,          NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SDMMC3_DAT7,     SDMMC3,          NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(SDMMC4_CLK,      SDMMC4,          NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_CMD,      SDMMC4,          NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT0,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT1,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT2,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT3,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT4,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT5,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT6,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_DAT7,     SDMMC4,          PULL_UP,       NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(SDMMC4_RST_N,    RSVD1,           PULL_UP,       NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(CLK_32K_OUT,     BLINK,           NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(HDMI_CEC,        CEC,             NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(KB_COL0,         KBC,             PULL_UP,       TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_COL1,         KBC,             PULL_UP,       TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_COL2,         KBC,             PULL_UP,       TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_COL3,         KBC,             NORMAL,        NORMAL,     OUTPUT), ////// check config state
	DEFAULT_PINMUX(KB_COL4,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(KB_COL5,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_COL6,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_COL7,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_ROW0,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(KB_ROW1,         RSVD2,           NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_ROW2,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(KB_ROW3,         KBC,             PULL_UP,       NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_ROW4,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_ROW5,         OWR,             NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(KB_ROW6,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_ROW7,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (Test point)
	DEFAULT_PINMUX(KB_ROW8,         KBC,             PULL_DOWN,     TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_ROW9,         KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_ROW10,        KBC,             NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_ROW11,        KBC,             NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(KB_ROW12,        KBC,             PULL_UP,       TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_ROW13,        KBC,             PULL_DOWN,     TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_ROW14,        KBC,             NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(KB_ROW15,        KBC,             NORMAL,        TRISTATE,   INPUT), //GI
	DEFAULT_PINMUX(OWR,             OWR,             NORMAL,        NORMAL,     OUTPUT), // G (NC)

	/* Power I2C pinmux */
	I2C_PINMUX(PWR_I2C_SCL,		I2CPWR,		NORMAL,	NORMAL,	INPUT,	DISABLE,	ENABLE),
	I2C_PINMUX(PWR_I2C_SDA,		I2CPWR,		NORMAL,	NORMAL,	INPUT,	DISABLE,	ENABLE),

	DEFAULT_PINMUX(SYS_CLK_REQ,     SYSCLK,          NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(CLK3_OUT,        EXTPERIPH3,      NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(CLK3_REQ,        DEV3,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(DAP4_DIN,        I2S3,            NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(DAP4_DOUT,       I2S3,            NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(DAP4_FS,         I2S3,            NORMAL,        NORMAL,     INPUT), // Function o
	DEFAULT_PINMUX(DAP4_SCLK,       I2S3,            NORMAL,        NORMAL,     INPUT), // Function o

	/* I2C1 pinmux */
	I2C_PINMUX(GEN1_I2C_SCL,	I2C1,            NORMAL,        NORMAL,     INPUT,   DISABLE,   ENABLE),
	I2C_PINMUX(GEN1_I2C_SDA,	I2C1,            NORMAL,        NORMAL,     INPUT,   DISABLE,   ENABLE),

	DEFAULT_PINMUX(GPIO_PU0,        RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(GPIO_PU1,        RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(GPIO_PU2,        RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(GPIO_PU3,        RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(GPIO_PU4,        PWM1,            NORMAL,        NORMAL,     OUTPUT), // G (NC)
	DEFAULT_PINMUX(GPIO_PU5,        PWM2,            NORMAL,        TRISTATE,   INPUT), // GI
	DEFAULT_PINMUX(GPIO_PU6,        RSVD1,           NORMAL,        NORMAL,     OUTPUT), // G
	DEFAULT_PINMUX(UART2_CTS_N,     UARTB,           NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(UART2_RTS_N,     UARTB,           NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(UART2_RXD,       IRDA,            NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(UART2_TXD,       IRDA,            NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(UART3_CTS_N,     UARTC,           NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(UART3_RTS_N,     UARTC,           NORMAL,        NORMAL,     OUTPUT), // Function o
	DEFAULT_PINMUX(UART3_RXD,       UARTC,           NORMAL,        NORMAL,     INPUT), // Function
	DEFAULT_PINMUX(UART3_TXD,       UARTC,           NORMAL,        NORMAL,     OUTPUT), // Function o

	/*Add from 14r2*/
	DEFAULT_PINMUX(CLK1_REQ,        DAP,             NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(PEX_L0_PRSNT_N,  PCIE,            NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(PEX_L0_RST_N,    PCIE,            NORMAL,    NORMAL,     OUTPUT),
	DEFAULT_PINMUX(PEX_L0_CLKREQ_N, PCIE,            NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(PEX_L1_PRSNT_N,  PCIE,            NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(PEX_L1_RST_N,    PCIE,            NORMAL,    NORMAL,     OUTPUT),
	DEFAULT_PINMUX(PEX_L1_CLKREQ_N, PCIE,            NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(JTAG_RTCK,       RTCK,            NORMAL,        NORMAL,     OUTPUT),
	VI_PINMUX(VI_D6,           VI,              NORMAL,    NORMAL,     OUTPUT, DISABLE, DISABLE),
	VI_PINMUX(VI_D8,           SDMMC2,          NORMAL,    NORMAL,     INPUT,  DISABLE, DISABLE),
	VI_PINMUX(VI_D9,           SDMMC2,          NORMAL,    NORMAL,     INPUT,  DISABLE, DISABLE),
	VI_PINMUX(VI_PCLK,         RSVD1,           PULL_UP,   TRISTATE,   INPUT,  DISABLE, DISABLE),
	VI_PINMUX(VI_HSYNC,        RSVD1,           NORMAL,    NORMAL,     INPUT,  DISABLE, DISABLE),
	VI_PINMUX(VI_VSYNC,        RSVD1,           NORMAL,    NORMAL,     INPUT,  DISABLE, DISABLE),
	/* SDMMC1 WP gpio */
	DEFAULT_PINMUX(VI_D11,          RSVD1,           PULL_UP,       NORMAL,     INPUT),
};

static __initdata struct tegra_pingroup_config cardhu_pinmux_dock_internal_pull_up[] = {
	DEFAULT_PINMUX(GPIO_PBB0,       RSVD1,           NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(GPIO_PBB6,       VGP6,            PULL_UP,   NORMAL,     INPUT),
};

static __initdata struct tegra_pingroup_config cardhu_pinmux_sensor_dvt2[] = {
	DEFAULT_PINMUX(DAP3_SCLK,       I2S2,            PULL_DOWN,    NORMAL,     OUTPUT), //G
	DEFAULT_PINMUX(SPI2_CS0_N,      SPI2,            PULL_UP,   TRISTATE,     INPUT), // GI
};

/* Clone from cardhu_pinmux_e1291_a04 */
static __initdata struct tegra_pingroup_config acer_t30_pinmux[] = {
	DEFAULT_PINMUX(GMI_AD15,        NAND,            PULL_DOWN,   NORMAL,   OUTPUT),
	DEFAULT_PINMUX(ULPI_DATA5,      UARTA,           PULL_UP,   NORMAL,     INPUT),
	DEFAULT_PINMUX(ULPI_DATA6,      UARTA,           NORMAL,    NORMAL,     OUTPUT),
	DEFAULT_PINMUX(SPI2_MOSI,       SPI6,            NORMAL,    NORMAL,     INPUT),
	DEFAULT_PINMUX(DAP3_SCLK,       RSVD1,           NORMAL,    NORMAL,     OUTPUT),
};

static void __init cardhu_audio_gpio_init(void)
{
	int ret = gpio_request(TEGRA_GPIO_CDC_IRQ, "wm8903");
	if (ret < 0)
		pr_err("%s: error %d requesting gpio %d\n",
			__func__, ret, TEGRA_GPIO_CDC_IRQ);

	ret = gpio_direction_input(TEGRA_GPIO_CDC_IRQ);
	if (ret < 0) {
		pr_err("%s: error %d setting gpio %d to input\n",
			__func__, ret, TEGRA_GPIO_CDC_IRQ);
		gpio_free(TEGRA_GPIO_CDC_IRQ);
	}
}

#define GPIO_INIT_PIN_MODE(_gpio, _is_input, _value)	\
	{					\
		.gpio_nr	= _gpio,	\
		.is_input	= _is_input,	\
		.value		= _value,	\
	}

/* Clone from init_gpio_mode_e1291_a04 */
static struct gpio_init_pin_info acer_t30_init_gpio_mode[] = {
	GPIO_INIT_PIN_MODE(TEGRA_GPIO_PDD6, false, 0),
	GPIO_INIT_PIN_MODE(TEGRA_GPIO_PDD4, false, 0),
	GPIO_INIT_PIN_MODE(TEGRA_GPIO_PR2, false, 0),
};

static struct tegra_gpio_table gpio_table[] = {
	{ .gpio = TEGRA_GPIO_PC3,	.enable = false	}, // UART2 RX
	{ .gpio = TEGRA_GPIO_PC2,	.enable = false	}, // UART2 TX
	{ .gpio = TEGRA_GPIO_PJ5,	.enable = false	}, // UART2 CTS
	{ .gpio = TEGRA_GPIO_PJ6,	.enable = false	}, // UART2 RTS
	{ .gpio = TEGRA_GPIO_PW7,	.enable = false	}, // UART3 RX
	{ .gpio = TEGRA_GPIO_PW6,	.enable = false	}, // UART3 TX
	{ .gpio = TEGRA_GPIO_PA1,	.enable = false	}, // UART3 CTS
	{ .gpio = TEGRA_GPIO_PC0,	.enable = false	}, // UART3 RTS
	{ .gpio = TEGRA_GPIO_PH0,	.enable = false	}, // PWM0
};

static void __init acer_t30_gpio_init_configure(void)
{
	int len;
	int i;
	struct gpio_init_pin_info *pins_info;

	len = ARRAY_SIZE(acer_t30_init_gpio_mode);
	pins_info = acer_t30_init_gpio_mode;
	for (i = 0; i < len; ++i) {
		tegra_gpio_init_configure(pins_info->gpio_nr,
			pins_info->is_input, pins_info->value);
		pins_info++;
	}

	tegra_gpio_config(gpio_table, ARRAY_SIZE(gpio_table));
}

int __init cardhu_gpio_init(void)
{
	acer_t30_gpio_init_configure();
	cardhu_audio_gpio_init();

	return 0;
}

int __init cardhu_pinmux_init(void)
{
	tegra30_default_pinmux();

        /* common pinmux connfiguration */
	switch (acer_board_type) {
	case BOARD_PICASSO_M:
	case BOARD_PICASSO_MF:
		tegra_pinmux_config_table(picasso2_pinmux_common, ARRAY_SIZE(picasso2_pinmux_common));
		break;
	}

	switch (acer_board_type) {
	case BOARD_PICASSO_M:
		tegra_pinmux_config_table(cardhu_pinmux_dock_internal_pull_up,
					ARRAY_SIZE(cardhu_pinmux_dock_internal_pull_up));
		tegra_pinmux_config_table(cardhu_pinmux_sensor_dvt2,
					ARRAY_SIZE(cardhu_pinmux_sensor_dvt2));
		break;
	case BOARD_PICASSO_MF:
		tegra_pinmux_config_table(cardhu_pinmux_sensor_dvt2,
					ARRAY_SIZE(cardhu_pinmux_sensor_dvt2));
		break;
	}
	tegra_drive_pinmux_config_table(cardhu_drive_pinmux,
					ARRAY_SIZE(cardhu_drive_pinmux));
	tegra_pinmux_config_table(acer_t30_pinmux,
					ARRAY_SIZE(acer_t30_pinmux));

	return 0;
}

#define PIN_GPIO_LPM(_name, _gpio, _is_input, _value)	\
	{					\
		.name		= _name,	\
		.gpio_nr	= _gpio,	\
		.is_gpio	= true,		\
		.is_input	= _is_input,	\
		.value		= _value,	\
	}

struct gpio_init_pin_info pin_lpm_cardhu_common[] = {
};

static void set_unused_pin_gpio(struct gpio_init_pin_info *lpm_pin_info,
		int list_count)
{
	int i;
	struct gpio_init_pin_info *pin_info;
	int ret;

	for (i = 0; i < list_count; ++i) {
		pin_info = (struct gpio_init_pin_info *)(lpm_pin_info + i);
		if (!pin_info->is_gpio)
			continue;

		ret = gpio_request(pin_info->gpio_nr, pin_info->name);
		if (ret < 0) {
			pr_err("%s() Error in gpio_request() for gpio %d\n",
					__func__, pin_info->gpio_nr);
			continue;
		}
		if (pin_info->is_input)
			ret = gpio_direction_input(pin_info->gpio_nr);
		else
			ret = gpio_direction_output(pin_info->gpio_nr,
							pin_info->value);
		if (ret < 0) {
			pr_err("%s() Error in setting gpio %d to in/out\n",
				__func__, pin_info->gpio_nr);
			gpio_free(pin_info->gpio_nr);
			continue;
		}
	}
}

/* Initialize the pins to desired state as per power/asic/system-eng
 * recomendation */
int __init cardhu_pins_state_init(void)
{
	set_unused_pin_gpio(&pin_lpm_cardhu_common[0],
	ARRAY_SIZE(pin_lpm_cardhu_common));

	return 0;
}
