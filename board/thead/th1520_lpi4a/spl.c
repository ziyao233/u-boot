// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025, Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/spl.h>
#include <asm/arch/sysctl_regs.h>
#include <cpu_func.h>
#include <dm.h>
#include <hang.h>
#include <spl.h>

#define TH1520_PMP_BASE		(void *)0xffdc020000

u32 spl_boot_device(void)
{
	switch (readl(TH1520_SYSCTL_BASE + TH1520_SYSCTL_BOOT_SEL) & 0x7) {
	case 0:
	case 1:
	case 2:
	case 3:
		return BOOT_DEVICE_RAM;
	case 4:
		return BOOT_DEVICE_MMC1;
	case 5:
		return BOOT_DEVICE_MMC2;
	case 6:
		return BOOT_DEVICE_NAND;
	case 7:
		return BOOT_DEVICE_SPI;
	}

	unreachable();
}

void board_init_f(ulong dummy)
{
	int ret = spl_early_init();
	struct udevice *dev;

	if (ret)
		panic("spl_early_init() failed %d\n", ret);

	preloader_console_init();

	/*
	 * Manually bind CPU ahead of time to make sure in-core timers are
	 * available in SPL.
	 */
	ret = uclass_get_device(UCLASS_CPU, 0, &dev);
	if (ret)
		panic("failed to bind CPU: %d\n", ret);

	spl_dram_init();

	icache_enable();
	dcache_enable();

	th1520_invalidate_pmp();
}
