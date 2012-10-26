/*
 * arch/arm/mach-tegra/fuse.h
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2010-2011 NVIDIA Corp.
 *
 * Author:
 *	Colin Cross <ccross@android.com>
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

#ifndef __MACH_TEGRA_FUSE_H
#define __MACH_TEGRA_FUSE_H

#define FUSE_SKU_USB_CALIB_0    0x1f0

#define SKU_ID_T20	8
#define SKU_ID_T25SE	20
#define SKU_ID_AP25	23
#define SKU_ID_T25	24
#define SKU_ID_AP25E	27
#define SKU_ID_T25E	28

#define TEGRA20		0x20
#define TEGRA30		0x30
#define TEGRA11X	0x35

#define INVALID_PROCESS_ID	99 /* don't expect to have 100 process id's */

extern int tegra_sku_id;
extern int tegra_chip_id;

extern int tegra_bct_strapping;

unsigned long long tegra_chip_uid(void);
unsigned int tegra_spare_fuse(int bit);
void tegra_init_fuse(void);
const char *tegra_get_revision_name(void);

#ifdef CONFIG_TEGRA_SILICON_PLATFORM

int tegra_soc_speedo_id(void);
void tegra_init_speedo_data(void);
int tegra_cpu_process_id(void);
int tegra_core_process_id(void);
int tegra_get_age(void);

#ifndef CONFIG_ARCH_TEGRA_2x_SOC
int tegra_package_id(void);
int tegra_cpu_speedo_id(void);
int tegra_cpu_speedo_mv(void);
int tegra_cpu_speedo_value(void);
int tegra_core_speedo_mv(void);
int tegra_get_sku_override(void);
int tegra_get_cpu_iddq_value(void);
#else
static inline int tegra_package_id(void) { return -1; }
static inline int tegra_cpu_speedo_id(void) { return 0; }
static inline int tegra_cpu_speedo_value(void) { return 1777; }
static inline int tegra_cpu_speedo_mv(void) { return 1000; }
static inline int tegra_core_speedo_mv(void) { return 1200; }
static inline int tegra_get_cpu_iddq_value(void) { return 0; }
#endif /* CONFIG_ARCH_TEGRA_2x_SOC */

#else

static inline int tegra_cpu_process_id(void) { return 0; }
static inline int tegra_core_process_id(void) { return 0; }
static inline int tegra_cpu_speedo_id(void) { return 0; }
static inline int tegra_cpu_speedo_value(void) { return 1777; }
static inline int tegra_soc_speedo_id(void) { return 0; }
static inline int tegra_package_id(void) { return -1; }
static inline int tegra_cpu_speedo_mv(void) { return 1250; }
static inline int tegra_core_speedo_mv(void) { return 1100; }
static inline void tegra_init_speedo_data(void) { }

#endif /* CONFIG_TEGRA_SILICON_PLATFORM */

u32 tegra_fuse_readl(unsigned long offset);
void tegra_fuse_writel(u32 val, unsigned long offset);

#endif /* MACH_TEGRA_FUSE_H */
