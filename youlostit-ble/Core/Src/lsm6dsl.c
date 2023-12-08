/*
 * lsm6dsl.c
 *
 *  Created on: Nov 13, 2023
 *      Author: Ji & Johnny
 */


/* Include memory map of our MCU */
#include <stm32l475xx.h>

// Configure and enable the LSM6DSL. Refer to section 4.1 of the LSM6DSL application note.
void lsm6dsl_init(){
	// 1. CTRL1_XL = 60h; // {hx10, hx60};
	uint8_t addr = 0b1101010; // LSM6DSL (D4h) read // LSM6DSL (D4h) write 1101010 + 1(R)/0(W) ??
	uint8_t dir = 0; // writing
	uint8_t data1[2] = {0x10, 0x60}; // CTRL1_XL (10h) value(60h)
	//uint8_t len = 2;
	i2c_transaction(addr, dir, data1, 2);

	// 2. INT1_CTRL = 01h; // {hx0D, hx01};
	uint8_t data2[2] = {0x0D, 0x01}; // INT1_CTRL (0Dh) value(01h)
	i2c_transaction(addr, dir, data2, 2);
}

// Read the current X,Y, and Z acceleration data from the accelerometer. This data will reflect the orientation and
// movement of the hardware. Refer to the data sheet for the units that these values are reported in.
void lsm6dsl_read_xyz(int16_t* x, int16_t* y, int16_t* z){
	/* 4.2(app note) */

	// read STATUS_REG register
	uint8_t addr = 0b1101010; // LSM6DSL (D4h)
	uint8_t dir = 1; // reading
	uint8_t data[2] = {0x1E, 0}; // STATUS_REG(addr) (1Eh), LSM6DSL(addr)+1(read) (D5h)
	//uint8_t len = 1; // ???

	//while((data[2]&1)==0){ 	// STATUS_REG->XLDA == 0
	//	i2c_transaction(addr, dir, data, 1);
	//}
	// STATUS_REG->XLDA == 1

	// Read OUTX_L_XL (28h) 42

	uint8_t dataxl[2] = {0x28, 0}; // OUTX_L_XL(addr) (28h), LSM6DSL(addr)+1(read) (D5h)
	i2c_transaction(addr, 1, dataxl, 1);
	// Read OUTX_H_XL (29h) 43
	uint8_t dataxh[2] = {0x29, 0}; // OUTX_H_XL(addr) (29h), LSM6DSL(addr)+1(read) (D5h)
	i2c_transaction(addr, 1, dataxh, 1);

	// little-edian
	// put into x
	int16_t tmpx = ((int16_t) dataxl[1]) | (((int16_t) dataxh[1])<<8);
    *x = tmpx;

	// Read OUTY_L_XL (2Ah) 44
	uint8_t datayl[2] = {0x2A, 0}; // OUTY_L_XL(addr) (2Ah), LSM6DSL(addr)+1(read) (D5h)
	i2c_transaction(addr, 1, datayl, 1);
	// Read OUTY_H_XL (2Bh) 45
	uint8_t datayh[2] = {0x2B, 0}; // OUTY_H_XL(addr) (2Bh), LSM6DSL(addr)+1(read) (D5h)
	i2c_transaction(addr, 1, datayh, 1);

	// put into y
	int16_t tmpy = ((int16_t) datayl[1]) | (((int16_t) datayh[1])<<8);
	*y = tmpy;

	// Read OUTZ_L_XL (2Ch) 46
	uint8_t datazl[2] = {0x2C, 0}; // OUTZ_L_XL(addr) (2Ch), LSM6DSL(addr)+1(read) (D5h)
	i2c_transaction(addr, 1, datazl, 1);
	// Read OUTZ_H_XL (2Dh) 47
	uint8_t datazh[2] = {0x2D, 0}; // OUTZ_H_XL(addr) (2Dh), LSM6DSL(addr)+1(read) (D5h)
	i2c_transaction(addr, 1, datazh, 1);

	// put into z
	int16_t tmpz = ((int16_t) datazl[1]) | (((int16_t) datazh[1])<<8);
	*z = tmpz;

	printf("x: %d\n",tmpx);
	printf("y: %d\n",tmpy);
	printf("z: %d\n",tmpz);
}
