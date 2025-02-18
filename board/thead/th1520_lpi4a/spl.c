// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025, Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/sysctl_regs.h>
#include <hang.h>
#include <spl.h>

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

	if (ret) {
		debug("spl_early_init() failed %d\n", ret);
		hang();
	}

	preloader_console_init();

	extern void init_ddr(void);
	init_ddr();
}
