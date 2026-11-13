/*
 * timer.c
 *
 *  Created on: Nov 8, 2026
 *      Author: joseluis
 */

#include <stdint.h>
#include "stm32f446xx.h"

static volatile uint16_t tick_count = 0;


void tim6_set_psc(uint16_t value){
	TIM6->PSC &= ~TIM_PSC_PSC_Msk;
	TIM6->PSC |= value;
}

void tim6_set_arr(uint16_t value){
	TIM6->ARR &= ~TIM_ARR_ARR_Msk;
	TIM6->ARR |= value;
}

void timer_init(void){
	//turning on tim6 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	//setting psc to 0 so we get 1
	tim6_set_psc((0 << 0));
	//setting arr to 31,999 so we get 32000
	tim6_set_arr((0x7CFF << 0));
	// setting the update interrupt
	TIM6->DIER |= TIM_DIER_UIE;
	// turning on NVIC_ISER1 at position 22 since ISER1 ranges from bits [31:0]
	// which are respectively for interrupt 32-63. 54 is the position for TIM6_DAC.
	NVIC_EnableIRQ(TIM6_DAC_IRQn);

	TIM6->CR1 |= TIM_CR1_CEN;
}

uint16_t tim6_read_tick_count(void){
	return tick_count;
}

void TIM6_DAC_IRQHandler(void){
	TIM6->SR &= ~TIM_SR_UIF;
	tick_count++;
}
