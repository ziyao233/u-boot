// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023, Yixun Lan <dlan@gentoo.org>
 *
 */

#include <cpu_func.h>

int board_init(void)
{
	enable_caches();

	return 0;
}

void board_boot_order(u32 *spl_boot_list)
{
	while (1);
}
