/*
 * uart.c
 *
 *  Created on: Oct 12, 2026
 *      Author: joseluis
 */


#include <stdint.h>
#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define RCC_APB1ENR (*(volatile uint32_t *)0x40023840)
#define GPIOA_AFRL (*(volatile uint32_t *)0x40020020)
#define GPIOA_MODER (*(volatile uint32_t *) 0x40020000)
#define USART2_BRR (*(volatile uint32_t *)0x40004408)
#define USART2_CR1 (*(volatile uint32_t *)0x4000440C)
#define USART2_SR (*(volatile uint32_t *) 0x40004400)
#define USART2_DR (*(volatile uint32_t *) 0x40004404)
#define NVIC_ISER1 (*(volatile uint32_t *) 0xE000E104)
#define UART_RX_BUF_SIZE 8U
#define UART_EOB (-1)

static char rx_buffer[UART_RX_BUF_SIZE];
static volatile unsigned int rx_head=0;
static volatile unsigned int rx_tail=0;

static unsigned int rx_next_head(void){return ((rx_head+1)%UART_RX_BUF_SIZE);}

static unsigned int rx_next_tail(void){return ((rx_tail+1)%UART_RX_BUF_SIZE);}

static unsigned int rx_buf_is_full(void){return rx_next_head() == rx_tail;}

static unsigned int rx_buf_is_empty(void){return rx_head == rx_tail;}


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

	// clearing MODER2 bits 4-5 & MODER3 bits 6-7 to 00 make sure it is 0
	GPIOA_MODER &= ~((3 << 4) | (3 << 6));

	// setting MODER2 bits 4-5 and MODER3 bits 6-7 to 10 to enable AF
	GPIOA_MODER |= (2 << 4) | (2 << 6);

	// we clear PA2's AFRL bits 8-11 and PA3'S AFRL bits 12-15 to 0
	GPIOA_AFRL &= ~((0xF << 8) | (0xF << 12));

	// setting PA2's AFRL bits 8-10 and PA3's AFRL bits 12-14 to 1
	GPIOA_AFRL |= (7 << 8) | (7 << 12);

	// setting the BRR
	USART2_BRR = brr(115200, 16);

	// setting up TE, RE, UE, and RXNEIE to 1
	USART2_CR1 |= (1 << 3) | (1 << 2) | (1 << 13) | (1 << 5);

	// turning on NVIC_ISER1 at position 6 since ISER1 ranges from bits [31:0]
	// which are respectively for interrupt 32-63. 38 is the position for USART2.
	NVIC_ISER1 = (1 << 6);

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
	char queued_char = (char)USART2_DR;
	if(!rx_buf_is_full()){
		rx_buffer[rx_head] = queued_char;
		rx_head = rx_next_head();
	}
}


