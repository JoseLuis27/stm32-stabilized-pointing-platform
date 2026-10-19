/*
 * i2c.c
 *
 *  Created on: Oct 17, 2026
 *      Author: joseluis
 */

/* I2C1_SCL -> PB8
 * I2C1_SDA -> PB9
 *
 * alternate function number: AF4
 *
 * turn on clocks for GPIOBEN & I2C
 * RCC_AHB1ENR bit 1 for GPIOBEN
 * RCC_APB1ENR bit 21 for I2C1EN
 *
 *
 * GPIOB_MODER8 bits 16-17 MODER FOR SCL
 * GPIOB_MODER9 bits 18-19 MODER FOR SDA
 *
 *
 * GPIOB_AFRH8 bits 0-3
 * GPIOB_AFRH9 bits 4-7
 *
 * GPIOB_OTYPER bits 8-9
 */

#include <stdint.h>
#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define RCC_APB1ENR (*(volatile uint32_t *)0x40023840)
#define GPIOB_MODER (*(volatile uint32_t *) 0x40020400)
#define GPIOB_AFRH (*(volatile uint32_t *) 0x40020424)
#define GPIOB_OTYPER (*(volatile uint32_t *) 0x40020404)
#define I2C1_CR2 (*(volatile uint32_t *) 0x40005404)
#define I2C1_CCR (*(volatile uint32_t *) 0x4000541C)
#define I2C1_TRISE (*(volatile uint32_t *) 0x40005420)
#define I2C1_CR1 (*(volatile uint32_t *) 0x40005400)


static void i2c1_rcc_init(void){
	// turn on the GPIOB clock
	RCC_AHB1ENR |= (1 << 1);
	// turn on the I2C1 clock
	RCC_APB1ENR |= (1 << 21);

}

static void i2c1_gpio_init(void){

	// set the GPIOB_OTYPER OT8-9 to open-drain
	GPIOB_OTYPER |= (1 << 8) | (1 << 9);
	// clear AFRH using 0000 0000
	GPIOB_AFRH &= ~(0xFF << 0);
	// setting AFRH to 0100 on bit 0 and 4.
	GPIOB_AFRH |= ((4 << 0) | (4 << 4));
	// clear GPIOB_MODER8-9
	GPIOB_MODER &= ~((3 << 16) | (3 << 18));
	// set the GPIOB_MODER8-9 to AF
	GPIOB_MODER |= (2 << 16) | (2 << 18);

}

static void i2c1_controller_mode_init(void){
	// clear FREQ to 00 0000
	I2C1_CR2 &= ~(0x3F << 0);
	// set FREQ to 16 MHz
	I2C1_CR2 |= (0x10 << 0);
	// clear CCR to 0000 0000 0000 and also making sure F/S is at 0 for slow mode.
	I2C1_CCR &= ~((0xFFF << 0) | (1 << 15));
	// set CCR to 80 or 0x50 as calculated for 100kHz SCL freq.
	I2C1_CCR |= (0x50 << 0);
	// clear TRISE to 00 0000
	I2C1_TRISE &= ~(0x3F << 0);
	// set TRISE to 17 or 0x11 since FREQ is 0x10 and TRISE is FREQ + 1 in Sm Mode.
	I2C1_TRISE |= (0x11 << 0);
}

static void i2c1_enable_peripheral(void){
	I2C1_CR1 |= (1 << 0);
}



void i2c_init(void){


	i2c1_rcc_init();
	i2c1_gpio_init();
	i2c1_controller_mode_init();
	i2c1_enable_peripheral();

}









