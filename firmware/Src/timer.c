/*
 * timer.c
 *
 *  Created on: Nov 8, 2026
 *      Author: joseluis
 */

#include <stdint.h>
#include <stdio.h>
#include "debug_pin.h"

#define RCC_APB1ENR (*(volatile uint32_t *)0x40023840)
#define TIM6_CR1 (*(volatile uint32_t *) 0x40001000)
#define TIM6_PSC (*(volatile uint32_t *) 0x40001028)
#define TIM6_ARR (*(volatile uint32_t *) 0x4000102C)
#define TIM6_SR (*(volatile uint32_t *) 0x40001010)
#define TIM6_DIER (*(volatile uint32_t *) 0x4000100C)
#define NVIC_ISER1 (*(volatile uint32_t *) 0xE000E104)


static volatile uint16_t tick_count = 0;


void tim6_set_psc(uint16_t value){
	TIM6_PSC &= ~(0xFFFF << 0);
	TIM6_PSC |= value;
}

void tim6_set_arr(uint16_t value){
	TIM6_ARR &= ~(0xFFFF << 0);
	TIM6_ARR |= value;
}

void timer_init(void){
	//turning on tim6 clock
	RCC_APB1ENR |= (1 << 4);
	//setting psc to 0 so we get 1
	tim6_set_psc((0 << 0));
	//setting arr to 31,999 so we get 32000
	tim6_set_arr((0x7CFF << 0));
	// setting the update interrupt
	TIM6_DIER |= (1 << 0);
	// turning on NVIC_ISER1 at position 22 since ISER1 ranges from bits [31:0]
	// which are respectively for interrupt 32-63. 54 is the position for TIM6_DAC.
	NVIC_ISER1 |= (1 << 22);

	TIM6_CR1 |= (1 << 0);


}

uint16_t tim6_read_tick_count(void){
	return tick_count;
}

void TIM6_DAC_IRQHandler(void){
	TIM6_SR &= ~(1 << 0);
	tick_count++;
	if(tick_count > 65000){
			tick_count -= 65000;
	}
	debug_pin_toggle();

}
