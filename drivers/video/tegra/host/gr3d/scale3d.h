/*
 * drivers/video/tegra/host/gr3d/scale3d.h
 *
 * Tegra Graphics Host 3D Clock Scaling
 *
 * Copyright (c) 2010-2012, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NVHOST_T30_SCALE3D_H
#define NVHOST_T30_SCALE3D_H

enum power_profile_status {
	DEVICE_UNKNOWN = 0,
	DEVICE_IDLE = 1,
	DEVICE_BUSY = 2
};

struct platform_device;
struct device;
struct dentry;

/* Initialization and de-initialization for module */
void nvhost_scale3d_init(struct platform_device *);
void nvhost_scale3d_deinit(struct platform_device *);

/*
 * call when performing submit to notify scaling mechanism that 3d module is
 * in use
 */
void nvhost_scale3d_notify_busy(struct platform_device *);
void nvhost_scale3d_notify_idle(struct platform_device *);

/*
 * 20.12 fixed point arithmetic
 */
#define INT_TO_FX(x) ((x) << 12)
#define FX_TO_INT(x) ((x) >> 12)
int FXMUL(unsigned int x, unsigned int y);
int FXDIV(unsigned int x, unsigned int y);

#define FX_HALF 0x800

#define MHZ_TO_HZ(x) ((x) * 1000000)
#define HZ_TO_MHZ(x) ((x) / 1000000)

#endif
