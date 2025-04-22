#ifndef DDR_COMMON_FUNC_H
#define DDR_COMMON_FUNC_H

enum DDR_TYPE {
    DDR_TYPE_LPDDR4X = 0,
    DDR_TYPE_LPDDR4,
    DDR_TYPE_MAX,
};
enum DDR_BITWIDTH {
    DDR_BITWIDTH_16 = 16,
    DDR_BITWIDTH_32 = 32,
    DDR_BITWIDTH_64 = 64,
    DDR_BITWIDTH_MAX = DDR_BITWIDTH_64,
};

void ddr_sysreg_wr(unsigned long int addr,unsigned int wr_data);
unsigned int ddr_sysreg_rd(unsigned long int addr);

void ddr_phy_reg_wr(unsigned long int addr,unsigned int wr_data);
void ddr_phy0_reg_wr(unsigned long int addr,unsigned int wr_data);
void ddr_phy1_reg_wr(unsigned long int addr,unsigned int wr_data);
void enable_axi_port(int port);
void dfi_freq_change(int dfi_freq,int skip_dram_init);
void dfi_init(int dfi_init_complete_en);
void lpddr4_enter_selfrefresh(int pwdn_en,int dis_dram_clk,int mode);
void lpddr4_auto_ps_en(int pwdn_en,int selfref_en,int clock_auto_disable);
void de_assert_other_reset_ddr(void);
void assert_ddrc_and_areset_sysreg(void);

void pll_config(int speed);
void deassert_pwrok_apb(enum DDR_BITWIDTH bits);
void ctrl_init(int rank_num, int speed);
void addrmap(int rank_num, enum DDR_BITWIDTH bits);
void ctrl_en(enum DDR_BITWIDTH bits);
void enable_auto_refresh(void);
void lpddr4_auto_selref(void);
int lpddr4_query_boundary(enum DDR_TYPE type, int rank_num, int speed,
                    enum DDR_BITWIDTH bits, unsigned long size);
#endif // DDR_COMMON_FUNCE_H
