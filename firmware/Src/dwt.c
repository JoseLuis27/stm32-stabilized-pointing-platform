/*
 * DWT.c
 *
 *  Created on: Nov 10, 2026
 *      Author: joseluis
 */
#include <stdint.h>

#define DWT_DEMCR       (*(volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL        (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT      (*(volatile uint32_t *)0xE0001004)

#define SYSTEM_CLOCK_HZ  16000000U

void dwt_init(void){
	DWT_DEMCR |= (1 << 24);
	DWT_CYCCNT = 0;
	DWT_CTRL |= (1 << 0);
}

void dwt_delay_us(uint32_t us){
	uint32_t cycles_per_us = SYSTEM_CLOCK_HZ/1000000U;
	uint32_t start_cycle = DWT_CYCCNT;
	uint32_t delay_cycles = us * cycles_per_us;
	while((DWT_CYCCNT - start_cycle) < delay_cycles){

	}
}
