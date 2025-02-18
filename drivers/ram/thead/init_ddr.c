#include "common_lib.h"
#include "ddr_common_func.h"
#include "lpddr4_init.h"

const char board_name[] = "FM";

void init_ddr(void)
{
    enum DDR_TYPE type = DDR_TYPE_LPDDR4X;
    int freq = 3733;
    int rank_num = 1;
    enum DDR_BITWIDTH bits = DDR_BITWIDTH_64;
    bool dbi_off = false;
    int board_ver = 1;
    printf("enter init_ddr\n");
    bits = DDR_BITWIDTH_64;

    rank_num = 2;

    type = DDR_TYPE_LPDDR4X;

    freq = 3733;

    dbi_off = true;

    printf("%s[%d] lpddr4%c %s freq=%d %dbit dbi_off=%c sdram init\n", board_name, board_ver, (type==DDR_TYPE_LPDDR4X?'x':' '), (rank_num==1?"singlerank":"dualrank"), freq, bits, (dbi_off==true?'y':'n'));
    lpddr4_init(type, rank_num, freq, bits);

    printf("exit init_ddr\n");
}
