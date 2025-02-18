// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <cpu_func.h>

#define TH1520_PMP_BASE		(void *)0xffdc020000

void th1520_invalidate_pmp(void)
{
	writel(0x0, TH1520_PMP_BASE + 0x0);

	invalidate_icache_all();
}
