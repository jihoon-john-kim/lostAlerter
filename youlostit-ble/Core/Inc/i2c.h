/*
 * leds.h
 *
 *  Created on: Nov 9, 2023
 *      Author: Ji & Johnny
 */

#ifndef I2CS_H_
#define I2CS_H_

/* Include the type definitions for the i2c peripheral */
#include <stm32l475xx.h>

void i2c_init();
uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len);

#endif /* I2CS_H_ */
