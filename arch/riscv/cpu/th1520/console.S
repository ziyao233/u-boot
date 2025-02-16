#include"uart.h"
	.global		th1520_putchar

	.text

th1520_putchar:
	li		t0,		BOARD_UART0_BASE
.wait:
	lw		t1,		BOARD_UART0_LSR(t0)
	andi		t1,		t1,		BOARD_UART0_LSR_TX_IDLE
	beqz		t1,		.wait

	sw		a0,		BOARD_UART0_THR(t0)
	ret
