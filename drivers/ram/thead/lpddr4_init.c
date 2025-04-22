#include "common_lib.h"
#include "lpddr4_init.h"
#include "waitfwdone.h"

#include <binman.h>
#include <binman_sym.h>
#include <linux/bitfield.h>

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

binman_sym_declare(ulong, ddr_fw, image_pos);

static int lpddr4_load_firmware(void)
{
	struct th1520_ddr_fw *fw = (void*)binman_sym(ulong, ddr_fw, image_pos);
	union th1520_ddr_cfg *cfg;
	size_t i, j;

	/* TODO: validate magic */

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
			dwc_ddrphy_phyinit_userCustom_G_waitFwDone(0);
			break;
		case TH1520_DDR_CFG_WAITFW1:
			dwc_ddrphy1_phyinit_userCustom_G_waitFwDone(0);
			break;
		default:
			break;
		}

		if (op == TH1520_DDR_CFG_RANGE)
			cfg = (void *)cfg + sizeof(cfg->range) +
				      cfg->range.num * sizeof(uint16_t);
		else
			cfg = (union th1520_ddr_cfg *)(&cfg->phy + 1);
	}

	return 0;
}

void lpddr4_init(enum DDR_TYPE type, int rank_num, int speed, enum DDR_BITWIDTH bits)
{
	//4266 3733 3200 2133
	//Others RSVD
	pll_config(speed);

	deassert_pwrok_apb(bits);

	//4266 3733 3200 2133
	//Others RSVD
	ctrl_init(rank_num, speed);

	//mode support: 16 32 64
	addrmap(rank_num, bits);

	de_assert_other_reset_ddr();

	lpddr4_load_firmware();

	ctrl_en(bits);

	enable_axi_port(0x1f);

	enable_auto_refresh();

	lpddr4_auto_selref();
}
