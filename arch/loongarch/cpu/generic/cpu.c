// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <irq_func.h>
#include <asm/cache.h>

/*
 * cleanup_before_linux() is called just before we call Linux, it prepares the
 * processor for linux
 *
 * We disable interrupts and flush caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	cache_flush();

	return 0;
}
