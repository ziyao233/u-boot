#include "common_lib.h"
#include "lpddr4_init.h"

#include <binman.h>
#include <binman_sym.h>
#include <dm.h>
#include <init.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <ram.h>

DECLARE_GLOBAL_DATA_PTR;

#pragma pack(push, 1)

struct th1520_ddr_fw {
	uint64_t magic;
	uint8_t type, ranknum, bitwidth, freq;
	uint8_t reserved[8];

	uint32_t cfgnum;
	union th1520_ddr_cfg {
		uint32_t opaddr;

		struct th1520_ddr_phy {
			uint32_t opaddr;
			uint16_t data;
		} phy;

		struct th1520_ddr_range {
			uint32_t opaddr;
			uint32_t num;
			uint16_t data[];
		} range;
	} cfgs[];
};

#pragma pack(pop)

/* Firmware constants */
#define TH1520_DDR_MAGIC	0x4452444445415448

#define TH1520_DDR_TYPE_LPDDR4	0
#define TH1520_DDR_TYPE_LPDDR4X	1

#define TH1520_DDR_FREQ_2133	0
#define TH1520_DDR_FREQ_3200	1
#define TH1520_DDR_FREQ_3733	2
#define TH1520_DDR_FREQ_4266	3

#define TH1520_DDR_CFG_OP	GENMASK(31, 24)
#define TH1520_DDR_CFG_ADDR	GENMASK(23, 0)

#define TH1520_DDR_CFG_PHY0	0
#define TH1520_DDR_CFG_PHY1	1
#define TH1520_DDR_CFG_PHY	2
#define TH1520_DDR_CFG_RANGE	3
#define TH1520_DDR_CFG_WAITFW0	4
#define TH1520_DDR_CFG_WAITFW1	5

/* Driver constants */
#define TH1520_PHY_MSG_TIMEOUT_US	1000000
#define TH1520_SYS_PLL_TIMEOUT_US	30

/* System configuration registers */
#define TH1520_SYS_PLL_CFG0			0x08
#define  TH1520_SYS_PLL_CFG0_POSTDIV2		GENMASK(26, 24)
#define  TH1520_SYS_PLL_CFG0_POSTDIV1		GENMASK(22, 20)
#define  TH1520_SYS_PLL_CFG0_FBDIV		GENMASK(19, 8)
#define  TH1520_SYS_PLL_CFG0_REFDIV		GENMASK(5, 0)
#define TH1520_SYS_PLL_CFG1			0x0c
#define  TH1520_SYS_PLL_CFG1_RST		BIT(30)
#define  TH1520_SYS_PLL_CFG1_FOUTPOSTDIVPD	BIT(27)
#define  TH1520_SYS_PLL_CFG1_FOUT4PHASEPD	BIT(25)
#define  Th1520_SYS_PLL_CFG1_DACPD		BIT(24)
#define TH1520_SYS_PLL_CFG2		0x10
#define TH1520_SYS_PLL_CFG3		0x14
#define TH1520_SYS_PLL_STS		0x18
#define  TH1520_SYS_PLL_STS_EN		BIT(16)
#define  TH1520_SYS_PLL_STS_LOCKED	BIT(0)

/* PHY configuration registers */
#define TH1520_DDR_PHY_REG(regid)	((regid) * 2)

/* UctShadowRegs */
#define TH1520_PHY_MSG_STATUS		TH1520_DDR_PHY_REG(0xd0004)
#define  TH1520_PHY_MSG_STATUS_EMPTY	BIT(0)
/* DctWriteProt */
#define TH1520_PHY_MSG_ACK		TH1520_DDR_PHY_REG(0xd0031)
#define  TH1520_PHY_MSG_ACK_EN		BIT(0)
/* UctWriteOnlyShadow */
#define TH1520_PHY_MSG_ID		TH1520_DDR_PHY_REG(0xd0032)
#define  TH1520_PHY_MSG_ID_COMPLETION	0x7
#define  TH1520_PHY_MSG_ID_ERROR	0xff
/* UctDatWriteOnlyShadow */
#define TH1520_PHY_MSG_DATA		TH1520_DDR_PHY_REG(0xd0034)

struct th1520_ddr_priv {
	void __iomem *phy0;
	void __iomem *phy1;
	void __iomem *ctrl;
	void __iomem *sys;
};

binman_sym_declare(ulong, ddr_fw, image_pos);

static int th1520_ddr_pll_config(void __iomem *sysreg, unsigned int frequency)
{
	u32 tmp;
	int ret;

	tmp = TH1520_SYS_PLL_CFG1_RST			|
	      TH1520_SYS_PLL_CFG1_FOUTPOSTDIVPD		|
	      TH1520_SYS_PLL_CFG1_FOUT4PHASEPD		|
	      Th1520_SYS_PLL_CFG1_DACPD;
	writel(tmp, sysreg + TH1520_SYS_PLL_CFG1);

	switch (frequency) {
	case TH1520_DDR_FREQ_3733:
		writel(FIELD_PREP(TH1520_SYS_PLL_CFG0_REFDIV, 1)	|
		       FIELD_PREP(TH1520_SYS_PLL_CFG0_FBDIV, 77)	|
		       FIELD_PREP(TH1520_SYS_PLL_CFG0_POSTDIV1, 2)	|
		       FIELD_PREP(TH1520_SYS_PLL_CFG0_POSTDIV2, 1),
		       sysreg + TH1520_SYS_PLL_CFG0);
		break;
	default:
		return -EINVAL;
	}

	udelay(2);
	tmp &= ~TH1520_SYS_PLL_CFG1_RST;
	writel(tmp, sysreg + TH1520_SYS_PLL_CFG1);

	ret = readl_poll_timeout(sysreg + TH1520_SYS_PLL_STS, tmp,
				 tmp & TH1520_SYS_PLL_STS_LOCKED,
				 TH1520_SYS_PLL_TIMEOUT_US);

	writel(TH1520_SYS_PLL_STS_EN, sysreg + TH1520_SYS_PLL_STS);

	return ret;
}

static int th1520_ddr_read_msg(void __iomem *phyreg, u16 *id, u16 *data)
{
	u32 tmp;
	int ret;

	ret = readw_poll_timeout(phyreg + TH1520_PHY_MSG_STATUS, tmp,
				 !(tmp & TH1520_PHY_MSG_STATUS_EMPTY),
				 TH1520_PHY_MSG_TIMEOUT_US);
	if (ret)
		return ret;

	*id   = readw(phyreg + TH1520_PHY_MSG_ID);
	*data = readw(phyreg + TH1520_PHY_MSG_DATA);

	writew(0, phyreg + TH1520_PHY_MSG_ACK);

	ret = readw_poll_timeout(phyreg + TH1520_PHY_MSG_STATUS, tmp,
				 tmp & TH1520_PHY_MSG_STATUS_EMPTY,
				 TH1520_PHY_MSG_TIMEOUT_US);
	if (ret)
		return ret;

	writew(TH1520_PHY_MSG_ACK_EN, phyreg + TH1520_PHY_MSG_ACK);

	return 0;
}

static int th1520_phy_wait_pmu_completion(void __iomem *phyreg)
{
	u16 id, data;
	int ret;

	do {
		ret = th1520_ddr_read_msg(phyreg, &id, &data);

		if (ret)
			return ret;
	} while (id != TH1520_PHY_MSG_ID_COMPLETION	&&
		 id != TH1520_PHY_MSG_ID_ERROR		&&
		 !ret);

	return id == TH1520_PHY_MSG_ID_COMPLETION ? ret : -EIO;
}

static int lpddr4_load_firmware(struct th1520_ddr_priv *priv,
				struct th1520_ddr_fw *fw)
{
	union th1520_ddr_cfg *cfg;
	size_t i, j;
	int ret;

	for (cfg = fw->cfgs, i = 0; i < fw->cfgnum; i++) {
		uint32_t addr = FIELD_GET(TH1520_DDR_CFG_ADDR, cfg->opaddr);
		uint32_t op = FIELD_GET(TH1520_DDR_CFG_OP, cfg->opaddr);

		switch (op) {
		case TH1520_DDR_CFG_PHY0:
			ddr_phy0_reg_wr(addr, cfg->phy.data);
			break;
		case TH1520_DDR_CFG_PHY1:
			ddr_phy1_reg_wr(addr, cfg->phy.data);
			break;
		case TH1520_DDR_CFG_PHY:
			ddr_phy_reg_wr(addr, cfg->phy.data);
			break;
		case TH1520_DDR_CFG_RANGE:
			for (j = 0; j < cfg->range.num; j++)
				ddr_phy_reg_wr(addr + j, cfg->range.data[j]);
			break;
		case TH1520_DDR_CFG_WAITFW0:
			ret = th1520_phy_wait_pmu_completion(priv->phy0);

			if (ret) {
				pr_err("phy 0 training failed: %d\n", ret);
				return ret;
			}

			break;
		case TH1520_DDR_CFG_WAITFW1:
			ret = th1520_phy_wait_pmu_completion(priv->phy1);

			if (ret) {
				pr_err("phy 1 training failed: %d\n", ret);
				return ret;
			}

			break;
		default:
			pr_err("Unknown DRAM configuration %d\n", op);

			return -EOPNOTSUPP;
		}

		if (op == TH1520_DDR_CFG_RANGE)
			cfg = (void *)cfg + sizeof(cfg->range) +
				      cfg->range.num * sizeof(uint16_t);
		else
			cfg = (union th1520_ddr_cfg *)(&cfg->phy + 1);
	}

	return 0;
}

static int th1520_ddr_init(struct th1520_ddr_priv *priv)
{
	struct th1520_ddr_fw *fw = (void *)binman_sym(ulong, ddr_fw, image_pos);
	int ret;

	ret = th1520_ddr_pll_config(priv->sys, fw->freq);
	if (ret) {
		pr_err("failed to configure PLL: %d\n", ret);
		return ret;
	}

	deassert_pwrok_apb(fw->bitwidth);

	ctrl_init(fw->ranknum, fw->freq == TH1520_DDR_FREQ_3733 ? 3733 : 0);

	// mode support: 16 32 64
	addrmap(fw->ranknum, fw->bitwidth);

	de_assert_other_reset_ddr();

	lpddr4_load_firmware(priv, fw);

	ctrl_en(fw->bitwidth);

	enable_axi_port(0x1f);

	enable_auto_refresh();

	lpddr4_auto_selref();

	return 0;
}

static int th1520_ddr_probe(struct udevice *dev)
{
	struct th1520_ddr_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr_name(dev, "phy-0");
	priv->phy0 = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	addr = dev_read_addr_name(dev, "phy-1");
	priv->phy1 = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	addr = dev_read_addr_name(dev, "ctrl");
	priv->ctrl = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	addr = dev_read_addr_name(dev, "sys");
	priv->sys = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	return th1520_ddr_init(priv);
}

static int th1520_ddr_get_info(struct udevice *dev, struct ram_info *info)
{
	info->base = gd->ram_base;
	info->size = gd->ram_size;

	return 0;
}

static struct ram_ops th1520_ddr_ops = {
	.get_info = th1520_ddr_get_info,
};

static const struct udevice_id th1520_ddr_ids[] = {
	{ .compatible = "thead,th1520-ddrc" },
	{ }
};

U_BOOT_DRIVER(th1520_ddr) = {
	.name = "th1520_ddr",
	.id = UCLASS_RAM,
	.ops = &th1520_ddr_ops,
	.of_match = th1520_ddr_ids,
	.probe = th1520_ddr_probe,
	.priv_auto = sizeof(struct th1520_ddr_priv),
};
