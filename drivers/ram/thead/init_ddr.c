#include "ddr_common_func.h"
#include "lpddr4_init.h"

const char board_name[] = "FM";

void init_ddr(void)
{
    enum DDR_TYPE type = DDR_TYPE_LPDDR4X;
    int freq = 3733;
    int rank_num = 1;
    enum DDR_BITWIDTH bits = DDR_BITWIDTH_64;
    bits = DDR_BITWIDTH_64;

    rank_num = 2;

    type = DDR_TYPE_LPDDR4X;

    freq = 3733;

    lpddr4_init(type, rank_num, freq, bits);
}
