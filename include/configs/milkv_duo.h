/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_SDRAM_BASE         0x80000000

#define CFG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x80400000"

#endif /* __CONFIG_H */
