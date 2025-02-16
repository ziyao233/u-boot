/*
 *	cocox-rv64
 *	/src/boards/virt/board/uart.h
 *	By MIT License.
 *	Copyright (c) 2023 Ziyao.
 */


#ifndef __BOARD_UART_H_INC__
#define __BOARD_UART_H_INC__

#define BOARD_UART_NR			1
#define BOARD_UART0_BASE		0xFFE7014000
#define BOARD_UART_REG_SHIFT		4
#define BOARD_UART_REG(n)		((n) * (BOARD_UART_REG_SHIFT))
#define BOARD_UART0_THR			BOARD_UART_REG(0x0)
#define BOARD_UART0_IER			BOARD_UART_REG(0x1)
#define BOARD_UART0_FCR			BOARD_UART_REG(0x2)
#define BOARD_UART0_ISR			BOARD_UART_REG(0x2)
#define BOARD_UART0_LCR			BOARD_UART_REG(0x3)
#define BOARD_UART0_LSR			BOARD_UART_REG(0x5)
#define BOARD_UART0_LSR_RX_READY	0x1
#define BOARD_UART0_LSR_TX_IDLE		0x40

#endif
