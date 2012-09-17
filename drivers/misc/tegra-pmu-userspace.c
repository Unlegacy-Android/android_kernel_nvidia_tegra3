/*
 * drivers/misc/tegra-pmu-userspace.c
 *
 * Copyright (C) 2012, NVIDIA CORPORATION. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/smp.h>

static void __init enable_userspace_access(void *info)
{
	/* Disabling overflow interrupts. This is done to *
	 * potential kernel blocking from the userspace   */
	asm ("mcr p15, 0, %0, c9, c14, 2\n\t" : : "r"(0x8000000f));

	/* Enabling the userspace access to the performance counters */
	asm ("mcr p15, 0, %0, c9, c14, 0\n\t" : : "r"(1));

	printk(KERN_INFO "tegrapmu: performance counters access enabled" \
					 "on CPU #%d\n", smp_processor_id());
}

static void __exit disable_userspace_access(void *info)
{
	/* Disabling the userspace access to the performance counters */
	asm ("mcr p15, 0, %0, c9, c14, 0\n\t" : : "r"(0));

	printk(KERN_INFO "tegrapmu: performance counters access disabled" \
					"on CPU #%d\n", smp_processor_id());
}

static int __init nvlostcycles_init(void)
{
	printk(KERN_INFO "tegrapmu: init\n");

	/* Enabling performance counters access from userspace on all cores */
	(void) on_each_cpu(enable_userspace_access, NULL, \
					 1 /* wait for execution */);

	return 0;
}

static void __exit nvlostcycles_exit(void)
{
	printk(KERN_INFO "nvlostcycles: exit\n");

	/* Disabling performance counters access from userspace on all cores */
	(void) on_each_cpu(disable_userspace_access, NULL, \
					1 /* wait for execution */);

	return;
}

module_init(nvlostcycles_init);
module_exit(nvlostcycles_exit);

MODULE_LICENSE("GPL");

