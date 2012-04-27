/*
 * arch/arm/mach-tegra/include/mach/io.h
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2011-2012 NVIDIA Corporation.
 *
 * Author:
 *	Colin Cross <ccross@google.com>
 *	Erik Gilling <konkers@google.com>
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

#ifndef __MACH_TEGRA_IO_H
#define __MACH_TEGRA_IO_H

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
#define IO_SPACE_LIMIT 0xffff
#else
#define IO_SPACE_LIMIT 0xffffffff
#endif

/* On TEGRA, many peripherals are very closely packed in
 * two 256MB io windows (that actually only use about 64KB
 * at the start of each).
 *
 * We will just map the first 1MB of each window (to minimize
 * pt entries needed) and provide a macro to transform physical
 * io addresses to an appropriate void __iomem *.
 *
 */

#ifdef __ASSEMBLY__
#define IOMEM(x)	(x)
#else
#define IOMEM(x)	((void __force __iomem *)(x))
#endif

#define IO_VIRT_BASE	0xFA000000

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
#define IO_PCIE_PHYS	0x80000000
#else
#define IO_PCIE_PHYS	0x00000000
#endif
#define IO_PCIE_VIRT	IOMEM(IO_VIRT_BASE)
#define IO_PCIE_SIZE	(SZ_16M * 3)

#define IO_CPU_PHYS	0x50000000
#define IO_CPU_VIRT	IOMEM(IO_PCIE_VIRT + IO_PCIE_SIZE)
#define IO_CPU_SIZE	SZ_1M

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
#define IO_PPCS_PHYS	0xC4000000
#else
#define IO_PPCS_PHYS	0x7C000000
#endif
#define IO_PPCS_VIRT	IOMEM(IO_CPU_VIRT + IO_CPU_SIZE)
#define IO_PPCS_SIZE	SZ_1M

#define IO_PPSB_PHYS	0x60000000
#define IO_PPSB_VIRT	IOMEM(IO_PPCS_VIRT + IO_PPCS_SIZE)
#define IO_PPSB_SIZE	SZ_1M

#define IO_APB_PHYS	0x70000000
#define IO_APB_VIRT	IOMEM(IO_PPSB_VIRT + IO_PPSB_SIZE)
#define IO_APB_SIZE	SZ_1M

#define IO_IRAM_PHYS	0x40000000
#define IO_IRAM_VIRT	IOMEM(IO_APB_VIRT + IO_APB_SIZE)
#define IO_IRAM_SIZE	SZ_256K

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
#define IO_USB_PHYS	0xC5000000
#else
#define IO_USB_PHYS	0x7D000000
#endif
#define IO_USB_VIRT	IOMEM(IO_IRAM_VIRT + IO_IRAM_SIZE)
#define IO_USB_SIZE	SZ_1M

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
#define IO_SDMMC_PHYS	0xC8000000
#else
#define IO_SDMMC_PHYS	0x78000000
#endif
#define IO_SDMMC_VIRT	IOMEM(IO_USB_VIRT + IO_USB_SIZE)
#define IO_SDMMC_SIZE	SZ_1M

#define IO_HOST1X_PHYS	0x54000000
#define IO_HOST1X_VIRT	IOMEM(IO_SDMMC_VIRT + IO_SDMMC_SIZE)
#define IO_HOST1X_SIZE	SZ_8M

#define IO_TO_VIRT_BETWEEN(p, st, sz)	((p) >= (st) && (p) < ((st) + (sz)))
#define IO_TO_VIRT_XLATE(p, pst, vst)	(((p) - (pst) + (vst)))

#define IO_TO_VIRT(n) ( \
	IO_TO_VIRT_BETWEEN((n), IO_PPSB_PHYS, IO_PPSB_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_PPSB_PHYS, IO_PPSB_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_APB_PHYS, IO_APB_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_APB_PHYS, IO_APB_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_CPU_PHYS, IO_CPU_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_CPU_PHYS, IO_CPU_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_IRAM_PHYS, IO_IRAM_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_IRAM_PHYS, IO_IRAM_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_HOST1X_PHYS, IO_HOST1X_SIZE) ?	\
		IO_TO_VIRT_XLATE((n), IO_HOST1X_PHYS, IO_HOST1X_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_USB_PHYS, IO_USB_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_USB_PHYS, IO_USB_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_SDMMC_PHYS, IO_SDMMC_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_SDMMC_PHYS, IO_SDMMC_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_PPCS_PHYS, IO_PPCS_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_PPCS_PHYS, IO_PPCS_VIRT) :	\
	IO_TO_VIRT_BETWEEN((n), IO_PCIE_PHYS, IO_PCIE_SIZE) ?		\
		IO_TO_VIRT_XLATE((n), IO_PCIE_PHYS, IO_PCIE_VIRT) :	\
	NULL)

#ifndef __ASSEMBLER__

#define IO_ADDRESS(n) (IO_TO_VIRT(n))

#ifdef CONFIG_TEGRA_PCI
extern void __iomem *tegra_pcie_io_base;

static inline void __iomem *__io(unsigned long addr)
{
	return tegra_pcie_io_base + (addr & IO_SPACE_LIMIT);
}
#else
static inline void __iomem *__io(unsigned long addr)
{
	return (void __iomem *)addr;
}
#endif

#define __io(a)         __io(a)

#endif

#endif
