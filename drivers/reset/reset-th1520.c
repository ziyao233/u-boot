// SPDX-License-Identifier: GPL-2.0-only
/*
 * T-Head TH1520 reset driver
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <dm.h>
#include <reset-uclass.h>
#include <dt-bindings/reset/thead,th1520-reset.h>

struct th1520_reset {
	void __iomem *base;
};

struct th1520_reset_map {
	u32 off;
	u32 bit;
};

struct th1520_reset_map th1520_ao_resets[] = {
	[TH1520_RESET_ID_E902_CORE] = {
		.off = 0x24,
		.bit = BIT(0),
	},
	[TH1520_RESET_ID_E902_HAD] = {
		.off = 0x24,
		.bit = BIT(1),
	},
};

static int th1520_reset_update(struct reset_ctl *rst, bool assert)
{
	struct th1520_reset *priv = dev_get_priv(rst->dev);
	struct th1520_reset_map *map = &th1520_ao_resets[rst->id];

	if (assert)
		clrbits_le32(priv->base + map->off, map->bit);
	else
		setbits_le32(priv->base + map->off, map->bit);

	return 0;
}

static int th1520_reset_assert(struct reset_ctl *rst)
{
	return th1520_reset_update(rst, true);
}

static int th1520_reset_deassert(struct reset_ctl *rst)
{
	return th1520_reset_update(rst, false);
}

struct reset_ops th1520_reset_ops = {
	.rst_assert	= th1520_reset_assert,
	.rst_deassert	= th1520_reset_deassert,
};

static int th1520_reset_probe(struct udevice *dev)
{
	struct th1520_reset *reset = dev_get_priv(dev);

	reset->base = dev_remap_addr(dev);
	if (!reset->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id th1520_reset_ids[] = {
	{ .compatible = "thead,th1520-reset-ao" },
	{ },
};

U_BOOT_DRIVER(th1520_reset) = {
	.name = "th1520-reset",
	.id = UCLASS_RESET,
	.of_match = th1520_reset_ids,
	.probe = th1520_reset_probe,
	.priv_auto = sizeof(struct th1520_reset),
	.ops = &th1520_reset_ops,
};
