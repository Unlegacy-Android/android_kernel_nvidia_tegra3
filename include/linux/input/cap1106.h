/*
 * An I2C driver for SMSC Proximity Sensor CAP1106.
 *
 * Copyright (c) 2012, ASUSTek Corporation.
 * Copyright (c) 2016, André Pinela
 *
 */

#ifndef _LINUX_CAP1106_H
#define _LINUX_CAP1106_H

#define NAME_RIL_PROX		"ril_proximity"
#define CAP1106_NAME		"cap1106"
#define CAP1106_IRQ_NAME	"cap1106_irq"
#define CAP1106_GPIO_NAME	"cap1106_gpio"

struct cap1106_i2c_platform_data {
	int irq_gpio;
};

#endif /* _LINUX_CAP1106_H */
