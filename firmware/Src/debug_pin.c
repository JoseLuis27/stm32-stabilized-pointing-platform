/*
 * debug_pin.c
 *
 *  Created on: Nov 9, 2026
 *      Author: joseluis
 */

// DEBUG: this file would be used to toggle pins for logic analyzer timings

#include <stdint.h>
#include "stm32f446xx.h"


void debug_pin_high(void){
	GPIOA->BSRR = GPIO_BSRR_BS5;
}

void debug_pin_low(void){
	GPIOA->BSRR = GPIO_BSRR_BR5;
}



void debug_pin_toggle(void){
	if((GPIOA->ODR & GPIO_ODR_OD5_Msk)==0){
		debug_pin_high();
	}else{
		debug_pin_low();
	}
}


void debug_pin_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER5_Msk;
	GPIOA->MODER |= GPIO_MODER_MODER5_0;
}
