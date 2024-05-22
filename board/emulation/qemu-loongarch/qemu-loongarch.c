// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <dm.h>
#include <dm/ofnode.h>
#include <env.h>
#include <fdtdec.h>
#include <image.h>
#include <lmb.h>
#include <log.h>
#include <spl.h>
#include <init.h>
#include <usb.h>
#include <virtio_types.h>
#include <virtio.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_MTD_NOR_FLASH)
int is_flash_available(void)
{
	if (!ofnode_equal(ofnode_by_compatible(ofnode_null(), "cfi-flash"),
			  ofnode_null()))
		return 1;

	return 0;
}
#endif

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x10000000)
		return 0x10000000;

	return gd->ram_top;
}

int board_init(void)
{
	return 0;
}

#define addr_alloc(lmb, size) lmb_alloc(lmb, size, SZ_64K)

int board_late_init(void)
{
	struct lmb lmb;
	u32 status = 0;

	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);

	status |= env_set_hex("kernel_addr_r", addr_alloc(&lmb, SZ_128M));
	status |= env_set_hex("ramdisk_addr_r", addr_alloc(&lmb, SZ_128M));
	status |= env_set_hex("kernel_comp_addr_r", addr_alloc(&lmb, SZ_64M));
	status |= env_set_hex("kernel_comp_size", SZ_64M);
	status |= env_set_hex("scriptaddr", addr_alloc(&lmb, SZ_4M));
	status |= env_set_hex("pxefile_addr_r", addr_alloc(&lmb, SZ_4M));
	status |= env_set_hex("fdt_addr_r", addr_alloc(&lmb, SZ_2M));

	if (status)
		log_warning("late_init: Failed to set run time variables\n");

	/* start usb so that usb keyboard can be used as input device */
	if (CONFIG_IS_ENABLED(USB_KEYBOARD))
		usb_init();

	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	/* Stored the DTB address there during our init */
	return (void *)(ulong)0x100000;
}
