/*
 * debug_pin.c
 *
 *  Created on: Nov 9, 2026
 *      Author: joseluis
 */

// DEBUG: this file would be used to toggle pins for logic analyzer timings

#include <stdint.h>
#include <stdio.h>
#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t *)0x40020000)
#define GPIOA_BSRR (*(volatile uint32_t *)0x40020018)
#define GPIOA_ODR (*(volatile uint32_t *)0x40020014)



void debug_pin_toggle(void){

	if((GPIOA_ODR & (1 << 5))==0){
		GPIOA_BSRR = (1 << 5);
	}else{
		GPIOA_BSRR = (1 << 21);
	}
}

void debug_pin_init(void){
	// enable GPIOA clock (bit 0)
	RCC_AHB1ENR |= (1 << 0);
	// clear bit 11 so MODER5 ends up 01
	GPIOA_MODER &= ~(1 << 11);
	// set PA5 to output (MODER5=01)
	GPIOA_MODER |= (1 << 10);
}
