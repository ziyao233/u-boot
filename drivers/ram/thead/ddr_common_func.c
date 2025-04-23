#include <asm/io.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include "common_lib.h"
#include "ddr_common_func.h"

DDR_SYSREG_REG_SW_REG_S ddr_sysreg;

#ifdef CONFIG_DDR_MSG
#define DDR_DEBUG(x) printf(x)
#endif

#ifndef CONFIG_DDR_RANK_SIZE
#define CONFIG_DDR_RANK_SIZE SZ_4G
#endif

#define rd16(addr)		readw((void *)(addr))
#define rd(addr)		readl((void *)(addr))
#define wr16(addr, value)	writew(value, (void *)(addr))
#define wr(addr, value)		writel(value, (void *)(addr))

void ddr_sysreg_wr(unsigned long int addr,unsigned int wr_data) {
  wr(addr+DDR_SYSREG_BADDR,wr_data);
}

unsigned int ddr_sysreg_rd(unsigned long int addr) {
  int rdata;
  rdata = rd(addr+DDR_SYSREG_BADDR);
  return rdata;
}

void ddr_phy_reg_wr(unsigned long int addr,unsigned int wr_data) {
    //unsigned long int ddr_phy_sel,addr_low;
    addr<<=1;
    //ddr_phy_sel=(addr)&0x00ffffff;
    //ddr_phy_sel>>=21;
    //addr_low=(addr)&0x001fffff;
    //wr(_SYS_REG_DDR_PHY_PS_SEL,ddr_phy_sel);
    //ddr_sysreg_rd(DDR_CFG0);
    wr16(_DDR_PHY_BADDR+addr, wr_data);
#ifndef CONFIG_DDR_H32_MODE
    wr16(_DDR_PHY1_BADDR+addr, wr_data);
#endif

}

void ddr_phy0_reg_wr(unsigned long int addr,unsigned int wr_data) {
    addr<<=1;
    wr16(_DDR_PHY_BADDR+addr, wr_data);

}

void ddr_phy1_reg_wr(unsigned long int addr,unsigned int wr_data) {
    addr<<=1;
    wr16(_DDR_PHY1_BADDR+addr, wr_data);

}

  void enable_axi_port(int port) {

   //wr(0xffff004008,0xff400000);//Full bypass scramble
   //wr(0xffff004008,0xff400000);//Full bypass scramble
    //axi rst->release
    ddr_sysreg_wr(DDR_CFG0,0x00f0);
    ddr_sysreg_wr(DDR_CFG0,0x1ff0);
    wr(DBG1,0);
    wr(DBG1_DCH1,0);
    if(port & 0x1) wr(PCTRL_0,1);
    if(port & 0x2) wr(PCTRL_1,1);
    if(port & 0x4) wr(PCTRL_2,1);
    if(port & 0x8) wr(PCTRL_3,1);
    if(port & 0x10) wr(PCTRL_4,1);
  }

void enable_auto_refresh() {
    wr(RFSHCTL3,0);//enable auto_refresh
}

 void ctrl_en(enum DDR_BITWIDTH bits) {
 // wr(SWCTL,0x00000000);
  wr(DFIMISC,0x00000030);// [5]dfi_init_start
 // wr(SWCTL,0x00000001);
 // while(rd(SWSTAT)!=0x00000001);
  while(rd(DFISTAT)!=0x00000001); //polling dfi_init_complete
if(bits==64) {
  while(rd(DCH1_DFISTAT)!=0x00000001);
 }
 // wr(SWCTL,0x00000000);
  wr(DFIMISC,0x00000010);
  wr(DFIMISC,0x00000011);
  wr(PWRCTL,0x0000000a); //[3] dfi_dram_clk_disable [1] powerdown_en
  wr(DCH1_PWRCTL,0x0000000a);
  wr(SWCTL,0x00000001);
   while(rd(SWSTAT)!=0x00000001);
   while(rd(STAT)!=0x00000001);
if(bits==64) {
   while(rd(DCH1_STAT)!=0x00000001);
 }
  wr(DFIPHYMSTR,0x14000001);
  wr(SWCTL,0x00000000);
  wr(INIT0,0x00020002);
  wr(SWCTL,0x00000001);
  while(rd(SWSTAT)!=0x00000001);
  //wr(PWRCTL,0x0000000b);
  //wr(DCH1_PWRCTL,0x0000000b);
#ifdef CONFIG_DDR_MSG
  printf("DFIPHYMSTR is %0x \n",rd(DFIPHYMSTR));
  printf("DFIUPD0    is %0x \n",rd(DFIUPD0));
  printf("DFIUPD1    is %0x \n",rd(DFIUPD1));
  printf("ZQCTL0     is %0x \n",rd(ZQCTL0));
  printf("ADDRMAP0     is %0x \n",rd(ADDRMAP0));
  printf("ADDRMAP1     is %0x \n",rd(ADDRMAP1));
#endif
 }

 void addrmap (int rank_num, enum DDR_BITWIDTH bits) {
 if(bits==DDR_BITWIDTH_16) {
#ifdef CONFIG_DDR_MSG
  printf("DDR 16bit mode\n");
#endif
  wr(MSTR,0x01080020|0x1000);
  wr(ADDRMAP0,0x001f1f16);  //
  wr(ADDRMAP1,0x00070707); //bank +2
  wr(ADDRMAP2,0x00000000); //col b5+5 ~ col b2  +2
  wr(ADDRMAP3,0x1f000000); //col b9 ~ col b6
  wr(ADDRMAP4,0x00001f1f); //col b11~ col b10
  wr(ADDRMAP5,0x060f0606); //row_b11 row b2_10 row b1 row b0  +6
  wr(ADDRMAP6,0x06060606); //max row 15
  wr(ADDRMAP7,0x00000f0f);
  wr(ADDRMAP9,0x06060606);
  wr(ADDRMAP10,0x06060606);
  wr(ADDRMAP11,0x00000006);
 } else if(bits==DDR_BITWIDTH_32){
#ifdef CONFIG_DDR_MSG
 printf("DDR 32bit mode\n");
#endif
  if(rank_num==2) {
#ifdef CONFIG_DDR_DDP
    wr(ADDRMAP0,0x001f0018);//max 8GB
#else
    wr(ADDRMAP0,0x001f0017); //4GB
#endif
  }
  else {
    wr(ADDRMAP0,0x001f001f); //cs_bit0: NULL
  }
  wr(ADDRMAP1,0x00080808); //bank +2
  wr(ADDRMAP2,0x00000000); //col b5+5 ~ col b2  +2
  wr(ADDRMAP3,0x00000000); //col b9 ~ col b6
  wr(ADDRMAP4,0x00001f1f); //col b11~ col b10
  wr(ADDRMAP5,0x070f0707); //row_b11 row b2_10 row b1 row b0  +6
  wr(ADDRMAP6,0x07070707); //row 15
  wr(ADDRMAP7,0x00000f0f); //row16: NULL
#ifdef CONFIG_DDR_DDP
  if(rank_num==2) {
    wr(ADDRMAP7,0x00000f07); //max row16
  }
#endif
  wr(ADDRMAP9,0x07070707);
  wr(ADDRMAP10,0x07070707);
  wr(ADDRMAP11,0x00000007);
 } else if(bits==DDR_BITWIDTH_64){
#ifdef CONFIG_DDR_MSG
 printf("DDR 64bit mode, 256B interleaving\n");
#endif
  wr(ADDRMAP0,0x0004001f); //cs_bit0: NULL
  if(rank_num==2) {
#ifdef CONFIG_DDR_DDP
    wr(ADDRMAP0,0x00040019);//max 16GB
#else
    wr(ADDRMAP0,0x00040018);//8GB
#endif
  }
  wr(ADDRMAP1,0x00090909); //bank +2
  wr(ADDRMAP2,0x00000000); //col b5+5 ~ col b2  +2
  wr(ADDRMAP3,0x01010101); //col b9 ~ col b6
  wr(ADDRMAP4,0x00001f1f); //col b11~ col b10
  wr(ADDRMAP5,0x080f0808); //row_b11 row b2_10 row b1 row b0  +6
  wr(ADDRMAP6,0x08080808); //row15
#ifdef CONFIG_DDR_DDP
  wr(ADDRMAP7,0x00000f08); //row16
#else
  wr(ADDRMAP7,0x00000f0f); //row16: NULL
#endif
  wr(ADDRMAP9,0x08080808);
  wr(ADDRMAP10,0x08080808);
  wr(ADDRMAP11,0x00000008);
 } else {
 printf("Wrong address map setting!!!\n");
 }
}

//de_assert umctl2_reset, phy_crst, and all areset
  void de_assert_other_reset_ddr() {
#ifdef CONFIG_DDR_MSG
    printf("de-assert areset and ctrl_crst_n,ddr_phy_crst_n by sysreg or tb \n");
#endif
    //wr(0xf0000000,0xf);
    //ddr_sysreg_wr(DDR_CFG0,0x1ff0);
    //ddr_sysreg_wr(DDR_CFG0,0x1ff0);
    //ddr_sysreg_wr(DDR_CFG0,0x1ff0);
    ddr_sysreg.ddr_sysreg_registers_struct_ddr_cfg0.u32 = ddr_sysreg_rd(DDR_CFG0);
    //ddr_sysreg.ddr_sysreg_registers_struct_ddr_cfg0.rg_ctl_ddr_usw_rst_reg |= 0x1F2;
    ddr_sysreg.ddr_sysreg_registers_struct_ddr_cfg0.rg_ctl_ddr_usw_rst_reg |= 0x1FA;
    ddr_sysreg_wr(DDR_CFG0,ddr_sysreg.ddr_sysreg_registers_struct_ddr_cfg0.u32);
  }
void lpddr4_auto_selref(void)
{
  ddr_sysreg_wr(DDR_CFG1,0xa0000);   //remove core clock after xx
  wr(SWCTL,0);
  wr(SWCTLSTATIC,1);
  wr(PWRTMG,0x40ae04);
  wr(HWLPCTL,0x430003);
  wr(DCH1_HWLPCTL,0x430003);
  wr(SWCTL,1);
  wr(SWCTLSTATIC,0);

  wr(PWRCTL,0x0000000b); //[3] dfi_dram_clk_disable [1] powerdown_en [0]serref_en
  wr(DCH1_PWRCTL,0x0000000b);
}
