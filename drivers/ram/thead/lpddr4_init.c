#include "common_lib.h"
#include "pinmux.h"
#include "ddr_common_func.h"
#include "lpddr4_init.h"

#include <binman.h>
#include <binman_sym.h>

extern void lp4_phy_train1d2d(enum DDR_TYPE type, int speed, enum DDR_BITWIDTH bits);

struct th1520_ddr_fw {
	uint64_t magic;
	uint8_t type, bitwidth, freq;
	uint8_t reserved[8];
	uint32_t cfgnum;
};

binman_sym_declare(ulong, ddr_fw, image_pos);

static int lpddr4_load_firmware(void)
{
	struct th1520_ddr_fw *fw = (void*)binman_sym(ulong, ddr_fw, image_pos);

	printf("%s fw = 0x%lx\n", __func__, (unsigned long)fw);
	printf("%s: _binman_sym_magic = %lx\n, %lx\n", __func__,
	       &_binman_sym_magic, _binman_sym_magic);

	printf("Firmware information:\n");
	printf("magic = 0x%llx, type = 0x%x, bitwidth = %d, freq = %x",
	       fw->magic, fw->type, fw->bitwidth, fw->freq);

	printf("%d configuration entires in total\n", fw->cfgnum);

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

  dq_pinmux(bits); // pinmux config before training

  lp4_phy_train1d2d(type, speed, bits);

//  dwc_ddrphy_phyinit_regInterface(saveRegs);

  ctrl_en(bits);

  enable_axi_port(0x1f);

  enable_auto_refresh();

  lpddr4_auto_selref();
}
