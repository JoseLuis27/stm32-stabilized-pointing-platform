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
// RCC register addresses
#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define RCC_APB1ENR (*(volatile uint32_t *)0x40023840)

//GPIOB register addresses
#define GPIOB_MODER (*(volatile uint32_t *) 0x40020400)
#define GPIOB_IDR (*(volatile uint32_t *) 0x40020410)
#define GPIOB_AFRH (*(volatile uint32_t *) 0x40020424)
#define GPIOB_OTYPER (*(volatile uint32_t *) 0x40020404)

// I2C1 register addresses
#define I2C1_CR2 (*(volatile uint32_t *) 0x40005404)
#define I2C1_CCR (*(volatile uint32_t *) 0x4000541C)
#define I2C1_TRISE (*(volatile uint32_t *) 0x40005420)
#define I2C1_CR1 (*(volatile uint32_t *) 0x40005400)
#define I2C1_SR1 (*(volatile uint32_t *) 0x40005414)
#define I2C1_SR2 (*(volatile uint32_t *) 0x40005418)
#define I2C1_DR (*(volatile uint32_t *) 0x40005410)


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

static void i2c1_reset_peripheral(void){

	uint8_t is_idle_high = (GPIOB_IDR & ((1 << 8) | (1 << 9))) == ((1 << 8) | (1 << 9));
	uint8_t is_busy = ((I2C1_SR2 & (1 << 1)) == (1 << 1));

	if(is_idle_high && is_busy){
		// turn on SWRST
		I2C1_CR1 |= (1 << 15);
		// turn off SWRST
		I2C1_CR1 &= ~(1 << 15);
	}

}


static void i2c1_enable_peripheral(void){
	I2C1_CR1 |= (1 << 0);
}


static void i2c1_start_sent(void){
	// set the START condition
	I2C1_CR1 |= (1 << 8);
	//polling SB
	while((I2C1_SR1 & (1 << 0)) == 0){}
}

static void i2c1_address_acked(uint8_t target_address){
	// write the target address to DR
	I2C1_DR = target_address;
	// polling ADDR unitl it pulls high
	while((I2C1_SR1 & (1 << 1)) == 0){}
}

static void i2c1_addr_flag_cleared(void){
	// read SR2 to clear ADDR flag
	I2C1_SR2;
}
static void i2c1_byte_sent(uint8_t byte){
	// polling for TxE until it pulls high
	while((I2C1_SR1 & (1 << 7)) == 0){}
	I2C1_DR = byte;
	//polling for BTF until it pulls high
	while((I2C1_SR1 & (1 << 2)) == 0){}
}

static void i2c1_single_byte_stop_requested(void){
	// clear ACK
	I2C1_CR1 &= ~(1 << 10);
	i2c1_addr_flag_cleared();
	// set STOP
	I2C1_CR1 |= (1 << 9);
}

static uint8_t i2c1_read_dr(void){
	// polling RXNE until it pulls high
	while((I2C1_SR1 & (1 << 6)) == 0){}
	// return DR value
	return I2C1_DR;
}

static void i2c1_stop_bit_cleared(void){
	// polling until STOP hardware clears stop
	while ((I2C1_CR1 & (1 << 9)) != 0){}
}

static void i2c1_stop_sent(void){
	// set STOP
	I2C1_CR1 |= (1 << 9);
	i2c1_stop_bit_cleared();
}
void i2c_init(void){

	i2c1_rcc_init();
	i2c1_gpio_init();
	i2c1_reset_peripheral();
	i2c1_controller_mode_init();
	i2c1_enable_peripheral();

}

/* Single-byte read: LSM6DSO32 Table 14 frame,
	RM0390 Fig. 275 (transmit part) and Fig. 276 (receive part). */
uint8_t i2c1_read_register(uint8_t target_address, uint8_t register_address){

	uint8_t write_byte = (target_address << 1);
	uint8_t read_byte = (write_byte | (1 << 0));
	i2c1_start_sent(); // ST (EV5: wait SB)
	i2c1_address_acked(write_byte); // SAD+W, SAK (clears EV5, waits EV6)
	i2c1_addr_flag_cleared(); // (clears EV6: read SR2)
	i2c1_byte_sent(register_address); // SUB, SAK  (EV8_1: wait TXE, write & EV8_2: wait BTF, then SR instead of P)
	i2c1_start_sent(); // SR (EV5: wait SB)
	i2c1_address_acked(read_byte); // SAD+R, SAK (clears EV5, waits EV6)
	i2c1_single_byte_stop_requested(); //sets up NMAK, SP (P) hardware send them after DATA
	uint8_t byte = i2c1_read_dr(); // DATA (EV7: wait RXNE, read DR)
	i2c1_stop_bit_cleared(); // NMAK, SP done ( hardware cleared STOP)

	return byte;

}

/* Single-byte write: LSM6DSO32 Table 12 frame,
	RM0390 Fig. 275 (transmit part). */
void i2c1_write_register(uint8_t target_address, uint8_t register_address, uint8_t register_value){
	uint8_t write_byte = (target_address << 1);
	i2c1_start_sent();// ST (EV5: wait SB)
	i2c1_address_acked(write_byte); // SAD+W, SAK (clears EV5, waits EV6)
	i2c1_addr_flag_cleared(); // (clears EV6: read SR2)
	i2c1_byte_sent(register_address);  // SUB, SAK (wait TXE, write, wait BTF)
	i2c1_byte_sent(register_value);  // DATA, SAK (wait TXE, write, wait BTF)
	i2c1_stop_sent(); // SP (EV8_2, P: Program stop request and waits until hardware clears it)
}







