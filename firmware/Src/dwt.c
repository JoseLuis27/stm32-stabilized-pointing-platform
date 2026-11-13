/*
 * DWT.c
 *
 *  Created on: Nov 10, 2026
 *      Author: joseluis
 */
#include <stdint.h>
#include "stm32f446xx.h"


#define SYSTEM_CLOCK_HZ  16000000U

void dwt_init(void){
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void dwt_delay_us(uint32_t us){
	uint32_t cycles_per_us = SYSTEM_CLOCK_HZ/1000000U;
	uint32_t start_cycle = DWT->CYCCNT;
	uint32_t delay_cycles = us * cycles_per_us;
	while((DWT->CYCCNT - start_cycle) < delay_cycles){}
}
