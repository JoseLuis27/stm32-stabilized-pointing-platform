/*
 * i2c.h
 *
 *  Created on: Oct 17, 2026
 *      Author: joseluis
 */

#ifndef I2C_H_
#define I2C_H_

void i2c_init(void);

uint8_t i2c1_read_register(uint8_t target_address, uint8_t register_address);

#endif /* I2C_H_ */
