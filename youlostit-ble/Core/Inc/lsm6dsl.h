/*
 * lsm6dsl.h
 *
 *  Created on: Nov 13, 2023
 *      Author: Johny, Ji
 */

#ifndef LSM6DSL_H_ // LSM6DSL_H_
#define LSM6DSL_H_ // LSM6DSL_H_

/* Include the type definitions for the i2c peripheral */
#include <stm32l475xx.h>

void lsm6dsl_init();
void lsm6dsl_read_xyz(int16_t* x, int16_t* y, int16_t* z);

#endif /* LSM6DSL_H_ */
