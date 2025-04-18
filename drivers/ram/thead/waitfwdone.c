#include "common_lib.h"
#include "ddr_common_func.h"

unsigned int get_mails(void)
{
	unsigned int read, msg0, msg1;

	// wait ack
	read = 0x1;
	do {
		read = ddr_phy_reg_rd(0xd0004);
	} while ((read & 0x1) == 1);

	// read msg
	msg0 = ddr_phy_reg_rd(0xd0032);
	msg1 = ddr_phy_reg_rd(0xd0034);

	// write-back
	ddr_phy0_reg_wr(0xd0031, 0);

	// wait ack end
	read = 0x0;
	do {
		read = ddr_phy_reg_rd(0xd0004);
	} while ((read & 0x1) == 0);

	// re-enable
	ddr_phy0_reg_wr(0xd0031, 1);

	// { uctDATwriteonlyshadow, uctwriteonlyshadow }
	return (msg0 + (msg1 << 16));
}

unsigned int get_phy1_mails(void)
{
	unsigned int read, msg0, msg1;

	// wait ack
	read = 0x1;
	do {
		read = ddr_phy1_reg_rd(0xd0004);
	} while ((read & 0x1) == 1);

	// read msg
	msg0 = ddr_phy1_reg_rd(0xd0032);
	msg1 = ddr_phy1_reg_rd(0xd0034);

	// write-back
	ddr_phy1_reg_wr(0xd0031, 0);

	// wait ack end
	read = 0x0;
	do {
		read = ddr_phy1_reg_rd(0xd0004);
	} while ((read & 0x1) == 0);

	// re-enable
	ddr_phy1_reg_wr(0xd0031,1);

	// { uctDATwriteonlyshadow, uctwriteonlyshadow }
	return (msg0 + (msg1 << 16));
}

void dwc_ddrphy_phyinit_userCustom_G_waitFwDone(unsigned char train2d)
{
	unsigned int train_result, msg_size, tmp, i;

	train_result = 0x1;
	/* 0x07 is okay, 0xff is error, 0x08 is a stream message */
	while(((train_result & 0xffff) != 0x7) &
	      ((train_result & 0xffff) != 0xff)) {
		train_result = get_mails();

		if((train_result & 0xffff) == 0x8){
			msg_size = get_mails() & 0xffff; // msg first byte

			for(i = 0; i < msg_size; i++)
				tmp = get_mails();
		}
	}
}

void dwc_ddrphy1_phyinit_userCustom_G_waitFwDone(unsigned char train2d)
{
	unsigned int train_result, msg_size, tmp, i;

	train_result = 0x1;
	/* 0x07 is okay, 0xff is error, 0x08 is a stream message */
	while(((train_result & 0xffff) != 0x7) &
	      ((train_result & 0xffff) != 0xff)) {
		train_result = get_phy1_mails();

		if((train_result & 0xffff) == 0x8){
			msg_size = get_phy1_mails() & 0xffff; // msg first byte

			for(i = 0; i < msg_size; i++)
				tmp = get_phy1_mails();
		}
	}
}
