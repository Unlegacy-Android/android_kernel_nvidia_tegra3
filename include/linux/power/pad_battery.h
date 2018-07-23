/*
 * Asus charger hacks for Transformers
 *
 * Copyright (c) 2012, ASUSTek Corporation.
 * Copyright (c) 2018, Svyatoslav Ryhel
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
 */

#ifndef __PAD_BATTERY_H
#define __PAD_BATTERY_H

#include <linux/gpio.h>
#include <../gpio-names.h>
#include <../wakeups-t3.h>
#include <../tegra_usb_phy.h>
#include <mach/board-transformer-misc.h>

/* Enable or disable the callback for the other driver. */
#define BATTERY_CALLBACK_ENABLED 1
#define DOCK_EC_ENABLED 1
#define GET_USB_CABLE_STATUS_ENABLED 1

extern int usb_suspend_tag; /* defined in tegra-otg */
extern unsigned int previous_cable_status; /* defined in tegra_udc */

void fsl_dock_ec_callback(void); /* Tegra UDC charger export */
void tegra_usb3_smi_backlight_on_callback(void); /* EHCI Tegra USB export */

/* Battery export */
void battery_callback(unsigned usb_cable_state);
int docking_callback(int docking_in);
void register_usb_cable_status_cb(unsigned (*fn) (void));

/* AsusEC export */
int asusdec_is_ac_over_10v_callback(void);
int asuspec_battery_monitor(char *cmd);

#endif
