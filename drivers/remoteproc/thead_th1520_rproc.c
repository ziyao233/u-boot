// SPDX-License-Identifier: GPL-2.0-only
/*
 * Remoteproc support for E902 core in T-Head TH1520 SoC
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <remoteproc.h>
#include <reset.h>

struct th1520_rproc_priv {
	struct reset_ctl rst_core, rst_had;
	void __iomem *sysreg;
};

#define TH1520_E902_RST_ADDR		0x44

static void *th1520_rproc_device_to_virt(struct udevice *dev, ulong da,
					 ulong size)
{
	if (da >= 0xffef8000 && da <= 0xfff40000)
		return (void __iomem *)0xffffef8000;

	return NULL;
}

static int th1520_rproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct th1520_rproc_priv *priv = dev_get_priv(dev);

	writel(rproc_elf_get_boot_addr(dev, addr),
	       priv->sysreg + TH1520_E902_RST_ADDR);

	return rproc_elf32_load_image(dev, addr, size);
}

static int th1520_rproc_start(struct udevice *dev)
{
	struct th1520_rproc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = reset_assert(&priv->rst_core);
	if (ret) {
		dev_err(dev, "failed to assert core reset: %d\n", ret);
		return ret;
	}

	ret = reset_assert(&priv->rst_had);
	if (ret) {
		dev_err(dev, "failed to assert had reset: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(&priv->rst_had);
	if (ret) {
		dev_err(dev, "failed to deassert had reset: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(&priv->rst_core);
	if (ret) {
		dev_err(dev, "failed to deassert core reset: %d\n", ret);
		return ret;
	}

	return 0;
}

struct dm_rproc_ops th1520_rproc_ops = {
	.load = th1520_rproc_load,
	.start = th1520_rproc_start,
	.device_to_virt = th1520_rproc_device_to_virt,
};

static int th1520_rproc_probe(struct udevice *dev)
{
	struct th1520_rproc_priv *priv = dev_get_priv(dev);
	int ret;

	priv->sysreg = dev_remap_addr(dev);
	if (!priv->sysreg) {
		dev_err(dev, "failed to map sysreg\n");
		return -EINVAL;
	}

	ret = reset_get_by_name(dev, "core", &priv->rst_core);
	if (ret) {
		dev_err(dev, "failed to get core reset: %d\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "had", &priv->rst_had);
	if (ret) {
		dev_err(dev, "failed to get had reset: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id th1520_rproc_ids[] = {
	{ .compatible = "thead,th1520-sysreg-ao" },
	{ },
};

U_BOOT_DRIVER(th1520_rproc) = {
	.name = "th1520-rproc",
	.of_match = th1520_rproc_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &th1520_rproc_ops,
	.probe = th1520_rproc_probe,
	.priv_auto = sizeof(struct th1520_rproc_priv),
};
