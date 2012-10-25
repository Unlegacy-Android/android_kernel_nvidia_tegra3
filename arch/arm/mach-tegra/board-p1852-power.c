/*
 * arch/arm/mach-tegra/board-p1852-power.c
 *
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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
 *
 */

#include <linux/io.h>
#include "board-p1852.h"
#include "pm.h"
#include "wakeups-t3.h"


static struct tegra_suspend_platform_data p1852_suspend_data = {
	/* FIXME: This value needs to come from SysEng */
	.cpu_timer	= 2000,
	.cpu_off_timer	= 200,
	.suspend_mode	= TEGRA_SUSPEND_NONE,
	.cpu_lp2_min_residency = 2000,
};

int __init p1852_suspend_init(void)
{
	tegra_init_suspend(&p1852_suspend_data);
	return 0;
}
