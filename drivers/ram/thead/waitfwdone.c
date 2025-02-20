#include "common_lib.h"
#include "ddr_common_func.h"

// L0 remove DDR_FW_STAGE_MSG and DDR_FW_DETAIL_MSG , only show init OK
#ifdef CONFIG_FW_MSG_L1
#define DDR_FW_STAGE_MSG    //L1  only show stage message
#endif

#ifdef CONFIG_FW_MSG_L2
#define DDR_FW_DETAIL_MSG   //1D and 2D Stream Message display
#define DDR_FW_2D_BRIEF_MSG       //L2 Message  show 2D brief message
#endif

#ifdef CONFIG_FW_MSG_L3
#define DDR_FW_DETAIL_MSG   //L3  Full  message (should define DDR_FW_STAGE_MSG)
#undef DDR_FW_2D_BRIEF_MSG        //L3 Message  show 2D detail message
#endif

#ifdef CONFIG_FW_MSG_TRAIN_1D
#define DDR_FW_TRAIN_1D_MSG
#endif

#ifdef DDR_FW_DETAIL_MSG
 #undef DDR_FW_STAGE_MSG
 #define DDR_FW_STAGE_MSG
#endif

unsigned int get_mails(void)
{
    unsigned int read;
    unsigned int msg0;
    unsigned int msg1;
	
	//wait ack
	read = 0x1;
	do{
        //read = (unsigned int)(*(volatile unsigned short*)(0xfe7a0008));
        read = ddr_phy_reg_rd(0xd0004);
    }while((read&0x1) == 1);

	//read msg
    //msg0 = (unsigned int)(*(volatile unsigned short*)(0xfe7a0064));
    msg0 = ddr_phy_reg_rd(0xd0032);
	//msg1 = (unsigned int)(*(volatile unsigned short*)(0xfe7a0068));
	msg1 = ddr_phy_reg_rd(0xd0034);
	
   //write-back
   //*(volatile unsigned short*)(0xfe7a0062) = 0;
   ddr_phy0_reg_wr(0xd0031,0);
   
   //wait ack end
   read = 0x0;
	do{
        //read = (unsigned int)(*(volatile unsigned short*)(0xfe7a0008));
        read = ddr_phy_reg_rd(0xd0004);
    }while((read&0x1) == 0);
	
	//re-enable
	//*(volatile unsigned short*)(0xfe7a0062) = 1;
	ddr_phy0_reg_wr(0xd0031,1);
	
	return (msg0 + (msg1<<16));//{uctDATwriteonlyshadow,uctwriteonlyshadow}
}

unsigned int get_phy1_mails(void)
{
    unsigned int read;
    unsigned int msg0;
    unsigned int msg1;

	//wait ack
	read = 0x1;
	do{
        //read = (unsigned int)(*(volatile unsigned short*)(0xfe7a0008));
        read = ddr_phy1_reg_rd(0xd0004);
    }while((read&0x1) == 1);

	//read msg
    //msg0 = (unsigned int)(*(volatile unsigned short*)(0xfe7a0064));
    msg0 = ddr_phy1_reg_rd(0xd0032);
	//msg1 = (unsigned int)(*(volatile unsigned short*)(0xfe7a0068));
	msg1 = ddr_phy1_reg_rd(0xd0034);

   //write-back
   //*(volatile unsigned short*)(0xfe7a0062) = 0;
   ddr_phy1_reg_wr(0xd0031,0);

   //wait ack end
   read = 0x0;
	do{
        //read = (unsigned int)(*(volatile unsigned short*)(0xfe7a0008));
        read = ddr_phy1_reg_rd(0xd0004);
    }while((read&0x1) == 0);

	//re-enable
	//*(volatile unsigned short*)(0xfe7a0062) = 1;
	ddr_phy1_reg_wr(0xd0031,1);

	return (msg0 + (msg1<<16));//{uctDATwriteonlyshadow,uctwriteonlyshadow}
}

void dwc_ddrphy_phyinit_userCustom_G_waitFwDone(unsigned char train2d) {

unsigned int train_result;

unsigned int stream_msg[32],i;

train_result = 0x1;
while(((train_result&0xffff)!=0x7) & ((train_result&0xffff)!=0xff))
{
    train_result = get_mails();
    #ifdef DDR_FW_DETAIL_MSG
    //printf("\n");
    #endif

    #ifdef DDR_FW_STAGE_MSG
    msg_display(train_result,0x0);
    #else
    if((train_result&0xff)==0x7){
#ifdef CONFIG_DDR_MSG
       printf("PHY0 DDR_INIT_OK\n");
#endif
    }
    else{
       if((train_result&0xff)==0xff){
          printf("PHY0 %s DDR_INIT_ERR\n", train2d?"train2d":"");
          while(1);
	} else {	
       //printf("PHY0 DDR_INIT_STAGE is %x \n",train_result&&0xff);
	}
    }
    #endif

    //Steam MSG
    if((train_result & 0xffff) == 0x8){
        stream_msg[0] = get_mails(); //msg first byte

        for(i=1;i<=(stream_msg[0]&0xffff);i++){
            stream_msg[i] = get_mails();
        }
       //printf("ST_MSG: CODE=%x, ",stream_msg[0]);
       #ifdef DDR_FW_DETAIL_MSG
       st_msg_display(train2d,stream_msg);
       //st_msg_display(train2d,stream_msg[0]);
       #endif
       //for(i=1;i<=(stream_msg[0]&0xffff);i++){
       //     printf("ST_MSG: DATA%d = %x\n",i,stream_msg[i]);
       //}
     }
 }
}


void dwc_ddrphy1_phyinit_userCustom_G_waitFwDone(unsigned char train2d) {

unsigned int train_result;

unsigned int stream_msg[32],i;

train_result = 0x1;
while(((train_result&0xffff)!=0x7) & ((train_result&0xffff)!=0xff))
{
    train_result = get_phy1_mails();
    #ifdef DDR_FW_DETAIL_MSG
    //printf("\n");
    #endif

    #ifdef DDR_FW_STAGE_MSG
    msg_display(train_result,0x1);
    #else
    if((train_result&0xff)==0x7) {
#ifdef CONFIG_DDR_MSG
       printf("PHY1 DDR_INIT_OK\n");
#endif
     }
    else{
       if((train_result&0xff)==0xff) {
          printf("PHY1 %s DDR_INIT_ERR\n", train2d?"train2d":"");
          while(1);
       } else {
          //printf("PHY1 DDR_INIT_STAGE is %x \n",train_result&&0xff);
       }
    }
    #endif

    //Steam MSG
    if((train_result & 0xffff) == 0x8){
        stream_msg[0] = get_phy1_mails(); //msg first byte

        for(i=1;i<=(stream_msg[0]&0xffff);i++){
            stream_msg[i] = get_phy1_mails();
        }
       //printf("ST_MSG: CODE=%x, ",stream_msg[0]);
       #ifdef DDR_FW_DETAIL_MSG
       st_msg_display(train2d,stream_msg);
       //st_msg_display(train2d,stream_msg[0]);
       #endif
       //for(i=1;i<=(stream_msg[0]&0xffff);i++){
       //     printf("ST_MSG: DATA%d = %x\n",i,stream_msg[i]);
       //}
     }
 }
}
