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

unsigned long get_ddr_density() {
    int div =1, mul=1;
#ifdef CONFIG_DDR_DUAL_RANK
	mul = 2;
#endif
#ifdef CONFIG_DDR_DDP
	mul *= 2;
#endif
#ifdef CONFIG_DDR_H32_MODE
    div = 2;
#endif
	return CONFIG_DDR_RANK_SIZE*mul/div;
}

enum DDR_TYPE get_ddr_type() {
    return DDR_TYPE_LPDDR4X;
}

int get_ddr_rank_number() {
#ifdef CONFIG_DDR_SINGLE_RANK
	return 1;
#elif defined CONFIG_DDR_DUAL_RANK
	return 2;
#else
#ifdef CONFIG_DDR_MSG
	DDR_DEBUG("unsupported ddr rank type!!!\n");
#endif
    return 0;
#endif
}

int get_ddr_freq() {
#ifdef CONFIG_DDR_4266
    return 4266;
#elif CONFIG_DDR_3733
    return 3733;
#elif CONFIG_DDR_3200
    return 3200;
#elif CONFIG_DDR_2133
	return 2133;
#else
    printf("unsupport lpddr4 freq!!!\n");
    return -1;
#endif
}

enum DDR_BITWIDTH get_ddr_bitwidth() {
#ifdef CONFIG_DDR_H32_MODE
    return DDR_BITWIDTH_32;
#elif CONFIG_DDR_H16_MODE
    return DDR_BITWIDTH_16;
#else
    return DDR_BITWIDTH_64;
#endif
}

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

unsigned int ddr_phy1_reg_rd(unsigned long int addr) {
    //unsigned long int ddr_phy_sel,addr_low,rd_data;
    unsigned int rd_data;
    addr<<=1;
    rd_data=rd16(_DDR_PHY1_BADDR+addr);
    return rd_data;
}
unsigned int ddr_phy0_reg_rd(unsigned long int addr) {
    //unsigned long int ddr_phy_sel,addr_low,rd_data;
    unsigned int rd_data;
    addr<<=1;
    rd_data=rd16(_DDR_PHY_BADDR+addr);
    return rd_data;
}
unsigned int ddr_phy_reg_rd(unsigned long int addr) {
    //unsigned long int ddr_phy_sel,addr_low,rd_data;
    unsigned int rd_data;
    addr<<=1;
    rd_data=rd16(_DDR_PHY_BADDR+addr);
    return rd_data;
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

 void pll_config (int speed) {
 if(speed==4266) {
 //4266
  ddr_sysreg_wr(DDR_CFG0+0xc,0x4b000000);
  ddr_sysreg_wr(DDR_CFG0+0x8,0x01205801);
  udelay(2);
  ddr_sysreg_wr(DDR_CFG0+0xc,0x0b000000);
 } else if(speed==3733) {
 //3733
  ddr_sysreg_wr(DDR_CFG0+0xc,0x4b000000);
  ddr_sysreg_wr(DDR_CFG0+0x8,0x01204d01);
  udelay(2);
  ddr_sysreg_wr(DDR_CFG0+0xc,0x0b000000);
 } else if(speed==3200) {
 //3200
 } else if(speed==2133) {
 //2133
  ddr_sysreg_wr(DDR_CFG0+0xc,0x4b000000);
  ddr_sysreg_wr(DDR_CFG0+0x8,0x01608501);
  udelay(2);
  ddr_sysreg_wr(DDR_CFG0+0xc,0x0b000000);
 } else {
#ifdef CONFIG_DDR_MSG
 printf("Reserved Pll setting\n");
#endif
  ddr_sysreg_wr(DDR_CFG0+0xc,0x4b000000);
  ddr_sysreg_wr(DDR_CFG0+0x8,0x01606601);
  ddr_sysreg_wr(DDR_CFG0+0xc,0x0b000000);
}
#ifdef CONFIG_DDR_MSG
  printf("Freq    is %0x \n",ddr_sysreg_rd(DDR_CFG0+0x8));
#endif
  while((ddr_sysreg_rd(DDR_CFG0+0x18)&1)!=0x1); //pll lock
  ddr_sysreg_wr(DDR_CFG0+0x18,0x10000);// core clock cg off
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

 void deassert_pwrok_apb (enum DDR_BITWIDTH bits) {
  ddr_sysreg_wr(DDR_CFG0,0x40);  // release PwrOkIn
  ddr_sysreg_wr(DDR_CFG0,0x40);
  ddr_sysreg_wr(DDR_CFG0,0x40);
  ddr_sysreg_wr(DDR_CFG0,0x40);
  ddr_sysreg_wr(DDR_CFG0,0x40);
  ddr_sysreg_wr(DDR_CFG0,0x40);

  ddr_sysreg_wr(DDR_CFG0,0xc0);  // release Phyrst
  ddr_sysreg_wr(DDR_CFG0,0xc0);  // release Phyrst
  ddr_sysreg_wr(DDR_CFG0,0xc0);  // release Phyrst
  ddr_sysreg_wr(DDR_CFG0,0xc0);  // release Phyrst

  ddr_sysreg_wr(DDR_CFG0,0xd0);  // release apb presetn
  ddr_sysreg_wr(DDR_CFG0,0xd0);
  ddr_sysreg_wr(DDR_CFG0,0xd0);
  ddr_sysreg_wr(DDR_CFG0,0xd0);
  ddr_sysreg_wr(DDR_CFG0,0xd0);
  ddr_sysreg_wr(DDR_CFG0,0xd0);
  if(bits==32) {
  ddr_sysreg_wr(DDR_CFG0,0xd2);
  }

 }

 //void ctrl_init(int speed,int rank_num) {
 void ctrl_init(int rank_num, int speed) {
  wr(DBG1,0x00000001);
  wr(PWRCTL,0x00000001);
  while(rd(STAT)!=0x00000000);
  if(rank_num==2) {
#ifdef CONFIG_DDR_MSG
  printf("Ctrl in dual rank mode \n");
#endif
  wr(MSTR,0x03080020);
  } else {
  wr(MSTR,0x01080020);
  }
  wr(MRCTRL0,0x00003030);
  wr(MRCTRL1,0x0002d90f);

 if(speed==4266) {
#ifdef CONFIG_DDR_MSG
  printf("Enter LP4 4266 init\n");
#endif
  wr(DERATEEN,0x000014f5); //[0] dereate enable  [7:4] derate byte input
  wr(DERATEINT,0x40000000);
  wr(DERATECTL,0x00000001);
  wr(PWRCTL,0x00000020);
  wr(PWRTMG,0x0040ae04);
  wr(HWLPCTL,0x00430002);
#ifdef ALL_BANK_REF
  wr(RFSHCTL0,0x00210000);
  wr(RFSHCTL1,0x000f0026);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x00828196);//[9:0] trfcb 380/0.937=406;//0x196 --> if 512MB on channel -> trfcb 280ns
  wr(RFSHTMG1,0x00610000);
#else
  wr(RFSHCTL0,0x00210004);
  wr(RFSHCTL1,0x000f0026);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x82000098);
//[31] t_rfc_nom_x1_sel ,1
//[27:16] -tREFI 488ns/0.94=520=0x208
//[9:0] -t_rfc_min 140ns/0.94=149=0x95
  //wr(RFSHTMG,0x81c00088); //for 3733 test

  wr(RFSHTMG1,0x00610000);
#endif
  wr(CRCPARCTL0,0x00000000);
  wr(INIT0,0xc0020002);
  wr(INIT1,0x00010002);
  wr(INIT2,0x00002300);
  wr(INIT3,0x0074003f);//OP[2:0] RL
  wr(INIT4,0x00330008);
  wr(INIT5,0x0005000c);
  wr(INIT6,0x0000004d);
  wr(INIT7,0x0000004d);
  wr(DIMMCTL,0x00000000);
  wr(RANKCTL,0x0000ab9f);//RANKCTL
  wr(RANKCTL1,0x0000001a);
  wr(DRAMTMG0,0x2221482d);
  wr(DRAMTMG1,0x00090941);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DRAMTMG2,0x09121219);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
#ifdef CONFIG_DDR_MSG
  printf("Enter LP4 DBI OFF mode\n");
#endif
#else
  //wr(DRAMTMG2,0x09141619);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
  wr(DRAMTMG2,0x09141f1a);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
#endif
  wr(DRAMTMG3,0x00f0f000);
  wr(DRAMTMG4,0x14040914);//[19:16] tCCD
  //wr(DRAMTMG4,0x14050914);//[19:16] tCCD
  wr(DRAMTMG5,0x02061111);
  wr(DRAMTMG6,0x0101000a);
  wr(DRAMTMG7,0x00000602);
  wr(DRAMTMG8,0x00000101);
  wr(DRAMTMG12,0x00020000);
  wr(DRAMTMG13,0x0e100002);
  wr(DRAMTMG14,0x00000133);
  //if(rank_num==2) {
  //wr(ZQCTL0,0x242d0021);
  //printf("zq_share_mode! \n");
  //} else {
  wr(ZQCTL0,0x042d0026);
  //}
  //wr(ZQCTL1,0x0360ccda);// ccda-> zq cal interval
  wr(ZQCTL1,0x03600800);// 
  wr(ZQCTL2,0x00000000);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DFITMG0,0x059f820e);//[22:16] dfi_t_rddate_en=RL-5
#else
  wr(DFITMG0,0x05a3820e);//[28:24] dft_t_ctrl_delay [22:16] dfi_t_rddate_en=RL-5
#endif
  wr(DFITMG1,0x000c0303);
  wr(DFILPCFG0,0x0351a101); //[8]=1: enable dfi lp mode during selfref
  //wr(DFIUPD0,0x00400018); //[31:30]=0 use ctrlupd enable
  //wr(DFIUPD1,0x00b700c4);
  //wr(DFIUPD2,0x00000000);//[31]=0 disable phy ctrlupdate
  wr(DFIMISC,0x00000011);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DFITMG2,0x00001f0e);//[14:9] dfi_tphy_rdcslat
  wr(DBICTL,0x00000001); //dbi-off
#else
  wr(DFITMG2,0x0000230e);//[14:9] dfi_tphy_rdcslat
  wr(DBICTL,0x00000007); //dbi-on
#endif
  wr(DFIPHYMSTR,0x14000001);
  wr(ADDRMAP0,0x0004001f);  // +2
  wr(ADDRMAP1,0x00090909); //bank +2
  wr(ADDRMAP2,0x00000000); //col b5+5 ~ col b2  +2
  wr(ADDRMAP3,0x01010101); //col b9 ~ col b6
  wr(ADDRMAP4,0x00001f1f); //col b11~ col b10
  wr(ADDRMAP5,0x080f0808); //row_b11 row b2_10 row b1 row b0  +6
  wr(ADDRMAP6,0x08080808);
  wr(ADDRMAP7,0x00000f0f);
  wr(ADDRMAP9,0x08080808);
  wr(ADDRMAP10,0x08080808);
  wr(ADDRMAP11,0x00000008);
  wr(ODTCFG,0x060a0c44);
 } else if(speed==3733) {
#ifdef CONFIG_DDR_MSG
  printf("Enter LP4 3733 init\n");
#endif
  //wr(DERATEEN,0x00001302);// 3733 disable derate
  wr(DERATEEN,0x000013f3);// derate enable
  wr(DERATEINT,0x40000000);
  wr(DERATECTL,0x00000001);
  wr(PWRCTL,0x00000020);
  wr(PWRTMG,0x0040ae04);
  wr(HWLPCTL,0x00430000);
#ifdef ALL_BANK_REF
  wr(RFSHCTL0,0x00210000);
  wr(RFSHCTL1,0x000d0021);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x006f8164);//tREFI=0x6F*32*1.083=3.846us  trfcpb 280ns :0x106  trfcpb 380ns/1.083ns= 0x164
  wr(RFSHTMG1,0x00540000);
#else
  wr(RFSHCTL0,0x00210004);
  wr(RFSHCTL1,0x000d0021);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x81c00084);
//[31] t_rfc_nom_x1_sel ,1
//[27:16] -tREFI 488ns/1.083=450=0x1c2
//[9:0] -t_rfc_min 140ns/1.083=130=0x82
  wr(RFSHTMG1,0x00540000);
#endif
  wr(CRCPARCTL0,0x00000000);
  wr(INIT0,0xc0020002);
  wr(INIT1,0x00010002);
  wr(INIT2,0x00001f00);
  wr(INIT3,0x00640036);
  wr(INIT4,0x00f20008);
  wr(INIT5,0x0004000b);
  wr(INIT6,0x00440012);
  wr(INIT7,0x0004001a);
  wr(DIMMCTL,0x00000000);
  wr(RANKCTL,0x0000ab9f);
  wr(RANKCTL1,0x00000017);
  wr(DRAMTMG0,0x1f263f28);//[30:24]-wr2pre 0x1e->0x1f
  wr(DRAMTMG1,0x00080839);//[20:16]-txp [13:8]-rd2pre 0x7->0x8 [6:0] 0x38->0x39
#ifdef CONFIG_DDR_DBI_OFF
  wr(DRAMTMG2,0x08101116);
#else
  //wr(DRAMTMG2,0x08121516);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
//  if(rank_num==2) {
//  wr(DRAMTMG2,0x08121b1a);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
//} else {
  wr(DRAMTMG2,0x08121d17);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
//}
#endif
  wr(DRAMTMG3,0x00d0e000);
  wr(DRAMTMG4,0x11040a12);//[4:0] t_rp
  wr(DRAMTMG5,0x02050e0e);
  wr(DRAMTMG6,0x01010008);
  wr(DRAMTMG7,0x00000502);
  wr(DRAMTMG8,0x00000101);
  wr(DRAMTMG12,0x00020000);
  wr(DRAMTMG13,0x0d100002);
  wr(DRAMTMG14,0x0000010c);
//  if(rank_num==2) {
//  wr(ZQCTL0,0x23a5001c); //[29] - zq share
//  printf("zq_share_mode! \n");
//} else {
  wr(ZQCTL0,0x03a50021);
//}
  //wr(ZQCTL1,0x02f0ccda);//ccda->zq cal interval
  wr(ZQCTL1,0x02f00800);
  wr(ZQCTL2,0x00000000);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DFITMG0,0x059b820c);//[28:24] dfi_t_ctrl_delay [22:16] dfi_t_rdata_en=RL-5
#else
  wr(DFITMG0,0x059f820c);//[28:24] dfi_t_ctrl_delay
#endif
  wr(DFITMG1,0x000c0303);//dfi_t_wrdata_delay=tctrl+6+BL/2+trainedTdqsdly=24, may need take care cmd pipe
  wr(DFILPCFG0,0x0351a101); //[8]=1: enable dfi lp mode during selfref
  //wr(DFIUPD0,0xc0400018);
  //wr(DFIUPD1,0x00b700c4);
  //wr(DFIUPD2,0x80000000);
  wr(DFIMISC,0x00000011);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DFITMG2,0x00001b0c);//[14:9] dfi_tphy_rdcslat
  wr(DBICTL,0x00000001);//[2:0] dbi-off
#else
  wr(DFITMG2,0x00001f0c);//
  wr(DBICTL,0x00000007);//[2:0] dbi-on
#endif
  wr(DFIPHYMSTR,0x14000001);
  wr(ADDRMAP0,0x0002001f);
  wr(ADDRMAP1,0x00090909);
  wr(ADDRMAP2,0x01010000);
  wr(ADDRMAP3,0x01010101);
  wr(ADDRMAP4,0x00001f1f);
  wr(ADDRMAP5,0x080f0808);
  wr(ADDRMAP6,0x08080808);
  wr(ADDRMAP7,0x00000f0f);
  wr(ADDRMAP9,0x08080808);
  wr(ADDRMAP10,0x08080808);
  wr(ADDRMAP11,0x00000008);
  wr(ODTCFG,0x06090b40);

 } else if(speed==3200) {
#ifdef CONFIG_DDR_MSG
  printf("Enter LP4 3200 init\n");
#endif
  wr(DERATEEN,0x000012f3);
  wr(DERATEINT,0x726d4ada);
  wr(DERATECTL,0x00000001);
  wr(PWRCTL,0x00000020);
  wr(PWRTMG,0x0040ae04);
  wr(HWLPCTL,0x00430002);
#ifdef ALL_BANK_REF
  wr(RFSHCTL0,0x00210000);//[2] per-bank ref enable
  wr(RFSHCTL1,0x000b001c);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x00618130);//[9:0] trfcab 380ns/1.25=304=0x130 [27:16]=3904/32/1.25=97 = 0x61
  wr(RFSHTMG1,0x00480000);
#else
  wr(RFSHCTL0,0x00210004);//[2] per-bank ref enable
  wr(RFSHCTL1,0x000b001c);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x81860070);//tREFI=3.9us lower(tREFI/(1/790MHz)/32)=96
  //[9:0] t_rfc_min must be set to RoundUp(RoundUp(tRFCmin/tCK)/2) trfcpb or trfcab
  //[27:16]-tREFI, per-ref  488ns/1.25=390=0x186
  wr(RFSHTMG1,0x00480000);
#endif
  wr(CRCPARCTL0,0x00000000);
  wr(INIT0,0xc0020002);
  wr(INIT1,0x00010002);
  wr(INIT2,0x00001a00);
  wr(INIT3,0x0054002e); //OP[2:0] RL
  wr(INIT4,0x0c310008);//[31:16] LP4 MR3
  wr(INIT5,0x00040009);
  wr(INIT6,0x00000012);
  wr(INIT7,0x0000001a);
  wr(DIMMCTL,0x00000000);
  wr(RANKCTL,0x0000ab9f);
  wr(RANKCTL1,0x00000017);
  wr(DRAMTMG0,0x1b203622);
  wr(DRAMTMG1,0x00060630);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DRAMTMG2,0x070e0f14);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
#else
  wr(DRAMTMG2,0x07101b15);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
#endif
  wr(DRAMTMG3,0x00b0c000);
  wr(DRAMTMG4,0x0f04080f);
  wr(DRAMTMG5,0x02040c0c);
  wr(DRAMTMG6,0x01010007);
  wr(DRAMTMG7,0x00000402);
  wr(DRAMTMG8,0x00000101);
  wr(DRAMTMG12,0x00020000);
  wr(DRAMTMG13,0x0c100002);
  wr(DRAMTMG14,0x000000e6);
  wr(ZQCTL0,0x03200018);
  wr(ZQCTL1,0x0280ccda);
  wr(ZQCTL2,0x00000000);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DFITMG0,0x0597820a);
#else
  wr(DFITMG0,0x059b820a); //[22:16] dfi_t_rddate_en=RL-5
#endif
  wr(DFITMG1,0x000b0303);
  wr(DFILPCFG0,0x0351a101); //[8]=1: enable dfi lp mode during selfref
  //wr(DFIUPD0,0xc0400018);
  //wr(DFIUPD1,0x00b700c4);
  //wr(DFIUPD2,0x80000000);
  wr(DFIMISC,0x00000011);
#ifdef CONFIG_DDR_DBI_OFF
  wr(DFITMG2,0x0000170a); //[14:8] dfi_tphy_rdcslat
  wr(DBICTL,0x00000001); //dbi-off
#else
  wr(DFITMG2,0x00001b0a); //[14:8] dfi_tphy_rdcslat
  wr(DBICTL,0x00000007); //dbi-on
#endif
  wr(DFIPHYMSTR,0x14000001);
  wr(ADDRMAP0,0x0004001f);  // +2
  wr(ADDRMAP1,0x00090909); //bank +2
  wr(ADDRMAP2,0x00000000); //col b5+5 ~ col b2  +2
  wr(ADDRMAP3,0x01010101); //col b9 ~ col b6
  wr(ADDRMAP4,0x00001f1f); //col b11~ col b10
  wr(ADDRMAP5,0x080f0808); //row_b11 row b2_10 row b1 row b0  +6
  wr(ADDRMAP6,0x08080808);
  wr(ADDRMAP7,0x00000f0f);
  wr(ADDRMAP9,0x08080808);
  wr(ADDRMAP10,0x08080808);
  wr(ADDRMAP11,0x00000008);
  wr(ODTCFG,0x06080a38);
 } else if(speed==2133){
#ifdef CONFIG_DDR_MSG
  printf("Enter LP4 2133 init\n");
#endif
  wr(DERATEEN,0x000011f1);
  wr(DERATEINT,0x726d4ada);
  wr(DERATECTL,0x00000001);
  wr(PWRCTL,0x00000020);
  wr(PWRTMG,0x0040ae04);
  wr(HWLPCTL,0x00430002);
#ifdef ALL_BANK_REF
  wr(RFSHCTL0,0x00210000);
  wr(RFSHCTL1,0x00070013);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x004180cc);// 380ns =0xcc
  wr(RFSHTMG1,0x00300000);
#else
  wr(RFSHCTL0,0x00210004);
  wr(RFSHCTL1,0x00070013);
  wr(RFSHCTL3,0x00000001);
  wr(RFSHTMG,0x81050048);// 
  //[9:0] t_rfc_min must be set to RoundUp(RoundUp(tRFCmin/tCK)/2) trfcpb or trfcab
  //[27:16]-tREFI, per-ref  488ns/1.87=261=0x105
//[9:0] -t_rfc_min 140ns/1.87=75=0x48
  wr(RFSHTMG1,0x00300000);
#endif
  wr(CRCPARCTL0,0x00000000);
  wr(INIT0,0xc0020002);
  wr(INIT1,0x00010002);
  wr(INIT2,0x00001200);
  wr(INIT3,0x0034001b);
  wr(INIT4,0x00310008);
  wr(INIT5,0x00030007);
  wr(INIT6,0x0000004d);
  wr(INIT7,0x0000004d);
  wr(DIMMCTL,0x00000000);
  wr(RANKCTL,0x0000ab9f);
  wr(RANKCTL1,0x00000012);
  wr(DRAMTMG0,0x14162417);
  wr(DRAMTMG1,0x00040420);
  //wr(DRAMTMG2,0x050a0c0f);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
  wr(DRAMTMG2,0x050a1212);//[29:24] -write latency [21:16] read latency [13:8] rd2wr [5:0] wr2rd
  wr(DRAMTMG3,0x00708000);
  wr(DRAMTMG4,0x0a04060a);
  wr(DRAMTMG5,0x02030808);
  wr(DRAMTMG6,0x01010005);
  wr(DRAMTMG7,0x00000302);
  wr(DRAMTMG8,0x00000101);
  wr(DRAMTMG12,0x00020000);
  wr(DRAMTMG13,0x0a100002);
  wr(DRAMTMG14,0x00000099);
//  if(rank_num==2) {
//  wr(ZQCTL0,0x22160010);
//  } else {
  wr(ZQCTL0,0x02160010);
//  }
  wr(ZQCTL1,0x01b0ccda);
  wr(ZQCTL2,0x00000000);
  wr(DFITMG0,0x048f8206);
  wr(DFITMG1,0x000b0303);
  wr(DFILPCFG0,0x0351a101); //[8]=1: enable dfi lp mode during selfref
  //wr(DFIUPD0,0xc0400018);
  //wr(DFIUPD1,0x00b700c4);
  //wr(DFIUPD2,0x80000000);
  wr(DFIMISC,0x00000011);
  wr(DFITMG2,0x00000f06);
  wr(DBICTL,0x00000001);
  wr(DFIPHYMSTR,0x14000001);
  wr(ADDRMAP0,0x0004001f);  // +2
  wr(ADDRMAP1,0x00090909); //bank +2
  wr(ADDRMAP2,0x00000000); //col b5+5 ~ col b2  +2
  wr(ADDRMAP3,0x01010101); //col b9 ~ col b6
  wr(ADDRMAP4,0x00001f1f); //col b11~ col b10
  wr(ADDRMAP5,0x080f0808); //row_b11 row b2_10 row b1 row b0  +6
  wr(ADDRMAP6,0x08080808);
  wr(ADDRMAP7,0x00000f0f);
  wr(ADDRMAP9,0x08080808);
  wr(ADDRMAP10,0x08080808);
  wr(ADDRMAP11,0x00000008);
  wr(ODTCFG,0x06060828);
 } else {
  printf("Wrong Speed setting!!!\n");
 }
  wr(DFIUPD0,0x00400018); //[31:30]=0 use ctrlupd enable
  //wr(DFIUPD1,0x005b0062); // less ctrl interval
  wr(DFIUPD1,0x00280032); // less ctrl interval
  wr(DFIUPD2,0x00000000);//[31]=0 disable phy ctrlupdate
  //wr(DFIUPD0,0xc0400018);
  //wr(DFIUPD1,0x00b700c4);
  //wr(DFIUPD2,0x80000000);
  wr(ODTMAP,0x00000000);
  wr(SCHED,0x1f829b1c);  //[2]  page-close enable [14:8] 0x1b: lpr entry num=28, hpr entry num=4
  wr(SCHED1,0x4400b00f); //[7:0] page-close timer
  wr(PERFHPR1,0x0f000001);
  wr(PERFLPR1,0x0f00007f);
  wr(PERFWR1,0x0f00007f);
  wr(SCHED3,0x00000208);
  wr(SCHED4,0x08400810);
  wr(DBG0,0x00000000);
  wr(DBG1,0x00000000);
  wr(DBGCMD,0x00000000);
  wr(SWCTL,0x00000001);
  wr(SWCTLSTATIC,0x00000000);
  wr(POISONCFG,0x00000001);
  wr(PCTRL_0,0x00000001);
  wr(PCTRL_1,0x00000001);
  wr(PCTRL_2,0x00000001);
  wr(PCTRL_3,0x00000001);
  wr(PCTRL_4,0x00000001);
  wr(DCH1_MRCTRL0,0x00003030);
  wr(DCH1_MRCTRL1,0x0002d90f);
  wr(DCH1_DERATECTL,0x00000001);
  wr(DCH1_PWRCTL,0x00000020);
  wr(DCH1_HWLPCTL,0x00430002);
  wr(DCH1_CRCPARCTL0,0x00000000);
  wr(DCH1_ZQCTL2,0x00000000);
  wr(DCH1_ODTMAP,0x00000000);
  wr(DCH1_DBG1,0x00000000);
  wr(DCH1_DBGCMD,0x00000000);
   while(rd(RFSHCTL3)!=0x00000001);
//update by perf sim
  wr(PCCFG,0x00000010);   //[4] page match limit,limits the number of consecutive same page DDRC transactions that can be granted by the Port Arbiter to four
  wr(PCFGR_0,0x0000500f); //CPU read
  wr(PCFGW_0,0x0000500f); //CPU write
  wr(PCFGR_1,0x00005020); //VI Read   max 32
  wr(PCFGW_1,0x0000501f); //VI Write, sensor/isp/dw/dsp
  wr(PCFGR_2,0x0000501f); //VO Read, DPU/GPU
  wr(PCFGW_2,0x0000503f); //VO Write, GPU
  wr(PCFGR_3,0x000051ff);
  wr(PCFGW_3,0x000051ff);
  wr(PCFGR_4,0x0000503f);
  wr(PCFGW_4,0x0000503f);
   while(rd(PWRCTL)!=0x00000020);
  wr(PWRCTL,0x00000020);
   while(rd(DCH1_PWRCTL)!=0x00000020);
  wr(DCH1_PWRCTL,0x00000020);
  wr(DBG1,0x00000000);
   while(rd(PWRCTL)!=0x00000020);
  wr(PWRCTL,0x00000020);
   while(rd(PWRCTL)!=0x00000020);
  wr(PWRCTL,0x00000020);
  wr(DCH1_DBG1,0x00000000);
   while(rd(DCH1_PWRCTL)!=0x00000020);
  wr(DCH1_PWRCTL,0x00000020);
   while(rd(DCH1_PWRCTL)!=0x00000020);
  wr(DCH1_PWRCTL,0x00000020);
   //while(rd(DFIPHYMSTR)!=0x14000000);
  wr(DFIPHYMSTR,0x14000001);
  wr(SWCTL,0x00000000);
  wr(DFIMISC,0x00000010);
  wr(DFIMISC,0x00000010);
 // wr(SWCTL,0x00000001);
 // while(rd(SWSTAT)!=0x00000001);
  wr(DBG1,0x00000002);
  wr(DCH1_DBG1,0x00000002);
   //while(rd(MSTR)!=0x01080020);
#ifdef CONFIG_DDR_MSG
  printf("RankCTL   is %0x \n",rd(RANKCTL));
  printf("DRAMTMG2   is %0x \n",rd(DRAMTMG2));
  printf("DFITMG0   is %0x \n",rd(DFITMG0));
  printf("DFITMG1   is %0x \n",rd(DFITMG1));
  printf("DRAMTMG4  is %0x \n",rd(DRAMTMG4));//[19:16] tCCD
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

  //put sdram into selfrefresh state, pwden_en=1 enable selfrefresh power-down, otherwise stay in selfrefresh
void lpddr4_enter_selfrefresh(int pwdn_en,int dis_dram_clk,int mode) {
   DWC_DDR_UMCTL2_C_STRUCT_REG_S umctl2_reg;

  umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.u32 = rd(PWRCTL);
  umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.selfref_sw = 1;
  if(pwdn_en) umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.stay_in_selfref = 0;//goto selfrefresh-powerdown after enter selfrefresh
  else umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.stay_in_selfref = 1;//do not goto selfrefresh-powerdown, stay in selfrefresh
  umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.lpddr4_sr_allowed = 1;
  if(dis_dram_clk) umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.en_dfi_dram_clk_disable = 1; //turn off sdram clk when in self-refresh power-down state
  wr(PWRCTL,umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.u32);//put sdram into selfrefresh state
  wr(PWRCTL_DCH1,umctl2_reg.dwc_ddr_umctl2_c_struct_pwrctl.u32);//put sdram into selfrefresh state

    //STAT:0x4 bit8,9:selfref_state
    umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32 = rd(STAT);
    if(pwdn_en) {
    while( umctl2_reg.dwc_ddr_umctl2_c_struct_stat.selfref_state != 2) //wait sdram enter selfrefresh-powerdown state
        umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32 = rd(STAT);
    }
    else {
    while( umctl2_reg.dwc_ddr_umctl2_c_struct_stat.selfref_state != 1) //wait sdram enter selfrefresh state
        umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32 = rd(STAT);
    }
#ifdef CONFIG_DDR_MSG
    printf("[lpddr4_enter_selfrefresh]: CH0 STAT is :%x after enter selfrefresh state\n",umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32);
#endif
   if(mode==0) {
    umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32 = rd(STAT_DCH1);
    if(pwdn_en) {
    while( umctl2_reg.dwc_ddr_umctl2_c_struct_stat.selfref_state != 2) //wait sdram enter selfrefresh-powerdown state
        umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32 = rd(STAT_DCH1);
    }
    else {
    while( umctl2_reg.dwc_ddr_umctl2_c_struct_stat.selfref_state != 1) //wait sdram enter selfrefresh state
        umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32 = rd(STAT_DCH1);
    }
#ifdef CONFIG_DDR_MSG
    printf("[lpddr4_enter_selfrefresh]: CH1 STAT is :%x after enter selfrefresh state\n",umctl2_reg.dwc_ddr_umctl2_c_struct_stat.u32);
#endif
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
