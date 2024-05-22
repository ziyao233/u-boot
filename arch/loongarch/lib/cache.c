// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <cpu_func.h>
#include <asm/cache.h>
#include <asm/loongarch.h>

void invalidate_icache_all(void)
{
	asm volatile ("\tibar 0\n"::);
}

__weak void flush_dcache_all(void)
{
	asm volatile ("\tdbar 0\n"::);
}

__weak void flush_dcache_range(unsigned long start, unsigned long end)
{
	/* Placeholder */
	flush_dcache_all();
}

__weak void invalidate_icache_range(unsigned long start, unsigned long end)
{
	/* LoongArch mandatory hardware I-Cache coherence */
	invalidate_icache_all();
}

__weak void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	/* Placeholder */
	flush_dcache_all();
}

__weak void cache_flush(void)
{
	/* Placeholder */
	flush_dcache_all();
}

__weak void cache_invalidate(void)
{
	/* Placeholder */
	flush_dcache_all();
}

__weak void flush_cache(unsigned long addr, unsigned long size)
{
	cache_flush();
}

__weak void dcache_enable(void)
{
}

__weak void dcache_disable(void)
{
}

__weak int dcache_status(void)
{
	return 0;
}

__weak void enable_caches(void)
{
	cache_invalidate();
	/* Enable cache for direct address translation mode */
	csr_xchg64(1 << CSR_CRMD_DACM_SHIFT, CSR_CRMD_DACM, LOONGARCH_CSR_CRMD);
}
