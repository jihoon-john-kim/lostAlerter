/*
 * leds.c
 *
 *  Created on: Nov 9, 2023
 *      Author: Ji & Johnny
 */


/* Include memory map of our MCU */
#include <stm32l475xx.h>


void i2c_init(){ // SCL -> PB10
	             // SDA -> PB11
				 // Fast-mode (up to 400 kbit/s) -> 39.4.3 & 39.4.5
/*
 * Configure and enable the I2C2 peripheral (it is connected to pins PB10 and PB11)
 * to communicate in master mode,
 * at a standard BAUD RATE i.e., 400 kHz or less
 * (You will need to read the I2C timing characteristics (Section 39.4.3 & 39.4.5)
 * to determine how to set the baud rate).
 * This function also should configure the appropriate pins on the MCU
 * so they are connected to the correct I2C peripheral
 * rather than operating as GPIO pins using the I/O Alternate function GPIO peripheral config (STM32 Datasheet - Table 17).
 *
 * */

	/* Enable the I2C and GPIOB clock */
	RCC->APB1ENR1 |= (1<<22); // enable I2C clock - 6.4.19
	RCC->AHB2ENR |= (1<<1); // enable GPIOB clock - 6.4.17

	/* GPIO pins using the I/O Alternate function *///- 8.5.1
	// SCL -> PB10
	/* Configure PB10 as an output by clearing all bits and setting the mode */
	GPIOB->MODER &= ~GPIO_MODER_MODE10; // clearing
	GPIOB->MODER |= (2<<20); // 10: Alternate function mode
	GPIOB->OTYPER |= (1<<10); // Output open-drain - 8.5.2
	GPIOB->PUPDR |= (1<<20); // 01: Pull-up  - 8.5.4
	GPIOB->AFR[1] |= (4<<8); // 0100: AF4 alternate function high - 8.5.10

	// SDA -> PB11
	/* Configure PB11 as an output by clearing all bits and setting the mode */
	GPIOB->MODER &= ~GPIO_MODER_MODE11; // clearing
	GPIOB->MODER |= (2<<22); // 10: Alternate function mode
	GPIOB->OTYPER |= (1<<11); // Output open-drain - 8.5.2
	GPIOB->PUPDR |= (1<<22); // 01: Pull-up  - 8.5.4
	GPIOB->AFR[1] |= (4<<12); // 0100: AF4 alternate function high - 8.5.10


	//I2C_CR1 PE = 1 (Peripheral enable)

	// clock resource HSI16 - 6.2.2
	// turn on HSI16 w/ 8MHz
	RCC->CR |= (1<<8); // HSION = 1 : HSI16 oscillator ON -> HSI16 clock enable
	RCC->CR |= (0111<<4);// MSIRANGE[3:0] = 0111: range 7 around 8 MHz -> change to 8MHz
	// choose HSI16 as resource clock in MUX - 6.4.28
	// The HSI16 clock can be selected as system clock after wakeup from Stop modes ?? - 6.2.2 -> 6.3
	RCC->CCIPR |= (10<<14);// I2C2SEL[1:0] = 10: HSI16 clock selected as I2C2 clock

	// 39.4.9
	// Before enabling the peripheral, the I2C master clock must be configured
	// by setting the SCLH and SCLL bits in the I2C_TIMINGR register
	// I2C_TIMINGR // fI2CCLK = 8 MHz - 39.4.10(example) -> 39.7.5(registers)
	I2C2->TIMINGR |= (0<<28); // PRESC[3:0] = 0
	I2C2->TIMINGR |= (0x9<<20); // SCLDEL[3:0] = 0x9
	I2C2->TIMINGR |= (0x3<<16); // SDADEL[3:0] = 0x3
	I2C2->TIMINGR |= (0x1<<8); // SCLH[7:0] = 0x1
	I2C2->TIMINGR |= (0x3<<0); // SCLL[7:0] = 0x3

	/* PE */
	// 1: Peripheral enable(PE) - 39.7.1
	// NACKIE = 1: Not acknowledge (NACKF) received interrupts enabled
	I2C2->CR1 |= (1<<0) | (1<<4);

}


//39.4.7
// primary = MCU, 2nd = peripheral
// address = peripheral's
// dir -> 0 : Writing to 2nd
//     -> 1 : Reading from 2nd
// This is a blocking function: it should return only when the entire transaction has been completed.
//																	TC==0 -> return
// A serial data transfer always begins with a START condition and ends with a STOP condition.
// Both START and STOP conditions are generated in master mode by software
uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len){
	// AUTOEND
	// HOW MANY TIME : NBYTES[7:0] // set The number of bytes to be transmitted/received to NBYTES[7:0]
	// WHERE : SADD[9:0] <- 110101 SAD[6:1] // set target address to SADD[7:1]
	// WHAT DO : RD_WRN = 0: Master requests a write transfer // RD_WRN = 1: Master requests a read transfer
	// START = 1: Restart/Start generation:

	// Transmission : writing : dir == 0

	if (dir == 0) {  // Figure 406
					//  AUTOEND    NBYTES        SADD        START
		I2C2->CR2 |= ( (1<<25) | (0x2<<16) | (address<<1) | (1<<13)); // AUTOEND = 1: Automatic end mode

		// I2C_ISR.NACKF = 1 -> END
		uint32_t NACKF = (I2C2->ISR)&(1<<4);
		if (NACKF == 1){ // NACKF: Not Acknowledge received flag - 39.7.7
			// END - probably wrong address
			return 1;
		}
		// I2C_ISR.NACKF = 0 -> connected well
		// I2C_ISR.TXIS = 1 (by hardware) -> auto re-start?
		//
		uint8_t TXIS = (I2C2->ISR)&(1<<1);
		for(int i = len; i>0; i--){
			while( TXIS == 0){
				TXIS = (I2C2->ISR)&(1<<1);
			}
			// I2C_ISR.TXIS = 1 -> Write I2C_TXDR
			I2C2->TXDR = *data; // -> TXE == 0
			data++;
			while( TXIS == 0){ // waiting until Write done
				TXIS = (I2C2->ISR)&(1<<1);
			}
		}
		//STOP
			//I2C2->CR2 |= (1<<14);
			//I2C2->CR2 |= I2C_CR2_START;
		// I2C_ISR.TC : Transfer Complete (master mode)
		uint32_t TC = (I2C2->ISR)&(1<<6);
		if( TC == 0 ){
			// I2C_ISR.TC = 1 (by hardware) -> END
			return 0;
		}

	}
	// Reception : Reading : dir == 1
	else if (dir == 1){            // Figure 409
	//Device (Slave) Address (7 bits
		dir = 0;
		//NBYTES clear
		I2C2->CR2 &= (0x0<<17);
		            //  AUTOEND    NBYTES        SADD        START
		I2C2->CR2 |= ( (1<<25) | (0x1<<16) | (address<<1) | (1<<13)); // AUTOEND = 1: Automatic end mode

		// I2C_ISR.NACKF = 1 -> END
		uint32_t NACKF = (I2C2->ISR)&(1<<4);
		if (NACKF == 1){ // NACKF: Not Acknowledge received flag - 39.7.7
			// END - probably wrong address
			return 1;
		}
		// I2C_ISR.NACKF = 0 -> connected well
		// I2C_ISR.TXIS = 1 (by hardware) -> auto re-start?
	//Register Address N (8 bits)
		uint8_t TXIS = (I2C2->ISR)&(1<<1);
		while( TXIS == 0){
			TXIS = (I2C2->ISR)&(1<<1);
		}
		// I2C_ISR.TXIS = 1 -> Write I2C_TXDR

		I2C2->TXDR = *data; // 30 : STATUS_REG(addr) (1Eh) , else
		while( TXIS == 0 ){ // waiting until read done
			TXIS = (I2C2->ISR)&(1<<1);
		}
		data++;


	//Repeated START
		uint8_t address1 = 0b11010101; // 11010101
		I2C2->CR2 |= ( (1<<25) | (0x1<<16) | (address1) | (0x1<<10) | (1<<13)); // AUTOEND = 1: Automatic end mode

	//Data Byte From Register N (8 bits)
		uint32_t RXNE = (I2C2->ISR)&(1<<2);
		while (RXNE == 0){ // I2C_ISR.RXNE == 1 : Read I2C_RXDR
			RXNE = (I2C2->ISR)&(1<<2);
		}
		*data = I2C2->RXDR; // data input to data[2] from RXDR
		// put in the data from register
		// I2C_ISR.TC = 1 (by hardware) -> EN
		uint32_t TC = (I2C2->ISR)&(1<<6);
		if( TC == 0 ){
		// I2C_ISR.TC = 1 (by hardware) -> END
			return 0;
		}
	}
}
