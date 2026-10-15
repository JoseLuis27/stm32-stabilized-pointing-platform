/*
 * uart.c
 *
 *  Created on: Oct 12, 2026
 *      Author: joseluis
 */


/* UART2_TX STM32 PIN: PA2
 * rcc base address 0x40023800
 * RCC_APB1ENR address 0x40023840 offset:0x40
 * USART2EN bit 17
 * the AF num for USART2 is AF7 USING GPIOX_AFRL[31:0]
 * USART2 base address 0x40004400
 * offsets:
 * SR: 0x00 TXE BIT 7
 * DR: 0x04
 * BRR: 0x8B VALUE FOR 115200 baud at 16MHz
 * CR1: 0x0C UE BIT 13 AND TE BIT 3
 *
 * GPIOA base address: 0x40020000
 *  GPIOA_AFRL 0x40020020
 *  */
#include <stdint.h>
#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define RCC_APB1ENR (*(volatile uint32_t *)0x40023840)
#define GPIOA_AFRL (*(volatile uint32_t *)0x40020020)
#define GPIOA_MODER (*(volatile uint32_t *) 0x40020000)
#define USART2_BRR (*(volatile uint32_t *)0x40004408)
#define USART2_CR1 (*(volatile uint32_t *)0x4000440C)
#define USART2_SR (*(volatile uint32_t *) 0x40004400)
#define USART2_DR (*(volatile uint32_t *) 0x40004404)

static uint32_t brr(uint32_t baud, uint32_t MHz){
	//converting MHz to Hz
	uint32_t f_ck = 1000000*MHz;
	//adding half of baud for rounding
	return (f_ck+(baud/2))/baud;
}


void uart_init(void){
	// enabling GPIOA's clock (bit 0)
	RCC_AHB1ENR |= (1 << 0);
	// enabling USART2EN clock (bit 17)
	RCC_APB1ENR |= (1 << 17);
	// clearing MODER2 bits 4-5 to 00 make sure it is 0
	GPIOA_MODER &= ~(3 << 4);
	// setting MODER2 bits 4-5 to 10 to enable AF
	GPIOA_MODER |= (2 << 4);
	// we clear PA2's AFRL bits 8-11 to 0
	GPIOA_AFRL &= ~(0xF << 8);
	// setting PA2's AFRL bits 8-10 to 1
	GPIOA_AFRL |= (7 << 8);
	// setting the BRR
	USART2_BRR = brr(115200, 16);
	// setting up TE and UE to 1
	USART2_CR1 |= (1 << 3) | (1 << 13);

}

void uart_send_char(char c){
	// polling TXE until it is 1.
	while((USART2_SR & (1 << 7))== 0){}
	// send char into DR
	USART2_DR = c;
}

void uart_send_string(const char *str){

	while(*str != '\0'){
		uart_send_char(*str);
		str++;
	}
}

