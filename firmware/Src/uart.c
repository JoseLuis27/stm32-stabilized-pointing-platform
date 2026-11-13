/*
 * uart.c
 *
 *  Created on: Oct 12, 2026
 *      Author: joseluis
 */


#include <stdint.h>
#include "stm32f446xx.h"

// buffer size
#define UART_RX_BUF_SIZE (8U)

//end of byte value (own version of C's EOF to handle the case of nothing being sent.
#define UART_EOB (-1)

// buffer and head/tail initializations
static char rx_buffer[UART_RX_BUF_SIZE];
static volatile uint8_t rx_head=0;
static volatile uint8_t rx_tail=0;

// helper functions for circular buffer
static uint8_t rx_next_head(void){return ((rx_head+1)%UART_RX_BUF_SIZE);}
static uint8_t rx_next_tail(void){return ((rx_tail+1)%UART_RX_BUF_SIZE);}
static uint8_t rx_buf_is_full(void){return rx_next_head() == rx_tail;}
static uint8_t rx_buf_is_empty(void){return rx_head == rx_tail;}


static uint32_t brr(uint32_t baud, uint32_t MHz){
	//converting MHz to Hz
	uint32_t f_ck = 1000000*MHz;
	//adding half of baud for rounding
	return (f_ck+(baud/2))/baud;
}


void uart_init(void){

	// initializing clocks
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// clearing MODER2 and 3
	GPIOA->MODER &= ~(GPIO_MODER_MODER2_Msk | GPIO_MODER_MODER3_Msk);
	// setting MODER2 and 3 to AF
	GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
	// we clear PA2's AFRL
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
	// setting PA2's AFRL to 7 as shown in RMO390
	GPIOA->AFR[0] |= (7 << GPIO_AFRL_AFSEL2_Pos) | (7 << GPIO_AFRL_AFSEL3_Pos);
	USART2->BRR = brr(115200, 16);
	USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
	// enabling the IRQ handler for USART2 ( needed for interrupts)
	NVIC_EnableIRQ(USART2_IRQn);

}

void uart_send_char(char c){
	// polling TXE until it is 1.
	while((USART2->SR & USART_SR_TXE) == 0){}
	// send char into DR
	USART2->DR = c;
}

void uart_send_string(const char *str){
	while(*str != '\0'){
		uart_send_char(*str);
		str++;
	}
}

// overriding the printf function to send it through our board
int __io_putchar(int ch){
	uart_send_char((char) ch);
	return ch;
}

int uart_read_char(void){
	if(!rx_buf_is_empty()){
		char stored_char = rx_buffer[rx_tail];
		rx_tail = rx_next_tail();
		return (int)stored_char;
	}
	return UART_EOB;
}

void USART2_IRQHandler(void){
	char queued_char = (char)USART2->DR;
	if(!rx_buf_is_full()){
		rx_buffer[rx_head] = queued_char;
		rx_head = rx_next_head();
	}
}


