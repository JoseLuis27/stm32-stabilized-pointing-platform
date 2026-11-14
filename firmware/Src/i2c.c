/*
 * i2c.c
 *
 *  Created on: Oct 17, 2026
 *      Author: joseluis
 */


#include <stdint.h>
#include <assert.h>
#include "dwt.h"
#include "stm32f446xx.h"


static void i2c1_rcc_init(void){
	// turn on clocks
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
}



static void i2c1_sda_bus_recovery(void){

	// is sda low?
	if((GPIOB->IDR & GPIO_IDR_ID9) == 0){
		// shouldn't exceed 9 pulses
		uint8_t count = 0;
		// pre loads pin
		GPIOB->BSRR = GPIO_BSRR_BS8 | GPIO_BSRR_BS9;
		// set it to open drain
		GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;
		// clear the MODER
		GPIOB->MODER &= ~(GPIO_MODER_MODER8_Msk | GPIO_MODER_MODER9_Msk);
		// set the MODER to general output mode
		GPIOB->MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;

		while(((GPIOB->IDR & GPIO_IDR_ID9) == 0) && count < 9){
			//smashing the scl
			GPIOB->BSRR = GPIO_BSRR_BR8;
			dwt_delay_us(5);
			GPIOB->BSRR = GPIO_BSRR_BS8;
			dwt_delay_us(5);
			count++;
		}

		GPIOB->BSRR = GPIO_BSRR_BR9;
		dwt_delay_us(5);
		GPIOB->BSRR = GPIO_BSRR_BS9;
	};
}


static void i2c1_gpio_init(void){

	i2c1_sda_bus_recovery();
	// clear GPIOB_MODER8-9
	GPIOB->MODER &= ~(GPIO_MODER_MODER8_Msk | GPIO_MODER_MODER9_Msk);
	// set the GPIOB_MODER8-9 to AF
	GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
	// set the GPIOB_OTYPER to open-drain
	GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;
	//clear AFRH
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8_Msk | GPIO_AFRH_AFSEL9_Msk);
	// setting PB8 and PB9 to AF4 (I2C1)
	GPIOB->AFR[1] |= (4 << GPIO_AFRH_AFSEL8_Pos) | (4 << GPIO_AFRH_AFSEL9_Pos);

}

static void i2c1_controller_mode_init(void){
	// clear FREQ
	I2C1->CR2 &= ~I2C_CR2_FREQ_Msk;
	// set FREQ to 16 MHz
	I2C1->CR2 |= (0x10 << I2C_CR2_FREQ_Pos);
	// clear CCR and also making sure F/S is at 0 for slow mode.
	I2C1->CCR &= ~(I2C_CCR_CCR_Msk | I2C_CCR_FS);
	// set CCR to 80 or 0x50 as calculated for 100kHz SCL freq.
	I2C1->CCR |= (0x50 << I2C_CCR_CCR_Pos);
	// clear TRISE
	I2C1->TRISE &= ~I2C_TRISE_TRISE_Msk;
	// set TRISE to 17 or 0x11 since FREQ is 0x10 and TRISE is FREQ + 1 in Sm Mode.
	I2C1->TRISE |= (0x11 << I2C_TRISE_TRISE_Pos);
}




static void i2c1_reset_peripheral(void){

	uint8_t is_idle_high = (GPIOB->IDR & (GPIO_IDR_ID8 | GPIO_IDR_ID9)) == (GPIO_IDR_ID8 | GPIO_IDR_ID9);
	uint8_t is_busy = (I2C1->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY;

	if(is_idle_high && is_busy){
		// turn on SWRST
		I2C1->CR1 |= I2C_CR1_SWRST;
		// turn off SWRST
		I2C1->CR1 &= ~I2C_CR1_SWRST;
	}

}


static void i2c1_enable_peripheral(void){
	I2C1->CR1 |= I2C_CR1_PE;
}


static void i2c1_start_sent(void){
    // set the START condition
    I2C1->CR1 |= I2C_CR1_START;
    //polling SB until bit is set
    while((I2C1->SR1 & I2C_SR1_SB) == 0){}
}

static void i2c1_address_acked(uint8_t target_address){
    // write the target address to DR
    I2C1->DR = target_address;
    // polling ADDR until bit is set
    while((I2C1->SR1 & I2C_SR1_ADDR) == 0){}
}

static void i2c1_addr_flag_cleared(void){
    // read SR2 to clear ADDR flag
    (void)I2C1->SR2;
}

static void i2c1_btf_bit_set(void){
    //polling until BTF bit is set
    while((I2C1->SR1 & I2C_SR1_BTF) == 0){}
}

static void i2c1_stop_bit_enabled(void){
    // set STOP
    I2C1->CR1 |= I2C_CR1_STOP;
}

static void i2c1_stop_bit_cleared(void){
    // polling until STOP hardware clears stop
    while((I2C1->CR1 & I2C_CR1_STOP) != 0){}
}

static void i2c1_ack_bit_enabled(void){
    // ACK bit is enabled (MAK)
    I2C1->CR1 |= I2C_CR1_ACK;
}

static void i2c1_ack_bit_disabled(void){
    // ACK bit is cleared (NMAK)
    I2C1->CR1 &= ~I2C_CR1_ACK;
}


static void i2c1_byte_sent(uint8_t byte){
    // polling until TxE bit is set
    while((I2C1->SR1 & I2C_SR1_TXE) == 0){}
    I2C1->DR = byte;
    //polling until BTF bit is set
    i2c1_btf_bit_set();
}

static void i2c1_single_byte_stop_requested(void){
	// clear ACK
	i2c1_ack_bit_disabled();
	i2c1_addr_flag_cleared();
	// set STOP
	i2c1_stop_bit_enabled();
}

static uint8_t i2c1_read_dr(void){
    // polling RXNE until bit is set
    while((I2C1->SR1 & I2C_SR1_RXNE) == 0){}
    // return DR value
    return I2C1->DR;
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
/* Multi-byte read: LSM6DSO32 Table 15 frame,
	RM0390 Fig. 275 (transmit part) and Fig. 276 (receive part). */
void i2c1_read_registers(uint8_t target_address, uint8_t register_address, uint8_t *buffer, uint8_t n){
	assert(n > 2);
	uint8_t write_byte = (target_address << 1);
	uint8_t read_byte = (write_byte | (1 << 0));

	i2c1_start_sent(); // ST (EV5: wait SB)
	i2c1_address_acked(write_byte); // SAD+W, SAK (clears EV5, waits EV6)
	i2c1_addr_flag_cleared(); // (clears EV6: read SR2)
	i2c1_byte_sent(register_address); // SUB, SAK  (EV8_1: wait TXE, write & EV8_2: wait BTF, then SR instead of P)
	i2c1_start_sent(); // SR (EV5: wait SB)
	i2c1_address_acked(read_byte); // SAD+R, SAK (clears EV5, waits EV6)
	i2c1_ack_bit_enabled(); // MAK
	i2c1_addr_flag_cleared();
	for(uint8_t i = 0; i < (n-3); i++){
		buffer[i] = i2c1_read_dr(); // DATA
	}
	// N > 2-byte reception stop procedure
	i2c1_btf_bit_set();
	i2c1_ack_bit_disabled();
	// N-2 Data read
	buffer[n - 3] = i2c1_read_dr();
	i2c1_btf_bit_set();
	i2c1_stop_bit_enabled();
	// N-1 and N Data read
	buffer[n - 2] = i2c1_read_dr();
	buffer[n - 1] = i2c1_read_dr();
	i2c1_stop_bit_cleared();
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
	i2c1_stop_bit_enabled(); // SP (EV8_2, P: Program stop request)
	i2c1_stop_bit_cleared(); // waits until hardware clears it
}








