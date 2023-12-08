/*
 * timer.c
 *
 *  Created on: Oct 5, 2023
 *      Author: schulman
 */

#include "timer.h"

void timer_init(TIM_TypeDef* timer)
{
  // 1. Stop the timer and clear out any timer state and reset all counters.
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	timer->CR1 &= ~TIM_CR1_CEN;  // ~(0x00000001) => CEN = 0 : counter disabled ; CR1.CEN - 31.4.1
	timer->CNT = 0;    // reset all counters. ; CNT - 31.4.12

  // 2. Setup the timer to auto-reload when the max value is reached.
	// No need to if statement since TIM_ARR_ARR is the max value itself
	timer->ARR |= TIM_ARR_ARR;	// timer = auto-reload ; ARR - 31.4.15

  // 3. Enable the timer’s interrupt both internally and in the interrupt controller (NVIC).
	timer->DIER |= TIM_DIER_UIE; // Enable the timer’s interrupt ; interrupt enable 31.4.4
    //a. You will need to use the NVIC functions NVIC_EnableIRQ, NVIC_SetPriority with 	the parameter TIM2_IRQn
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 0); // priority = 0

  // 4. Setup the clock tree to pass into the timer and divide it down as needed ; default clock speed MHz.
	// wanted = 20hz -> 50ms per cycle , given = 4Mhz -> 0.25us per cycle.
	// 0.25us * 2000000cnt = 50ms
	// therefore whenever 2000000 count -> interrupt
	//timer->ARR = 2000000; // 0x001E8480;
	//timer_set_ms(TIM2, 50);

  // 5. Enable the timer
	timer->CR1 |= TIM_CR1_CEN; //Enable the timer.
}


/*
 * Reset timer 2’s (TIM2) counters, but do not reset the entire TIM peripheral. The timer can be in the middle
of execution when it is reset and it’s counter will return to 0 when this function is called
 */
void timer_reset(TIM_TypeDef* timer)
{
    timer->DIER &= ~TIM_DIER_UIE;
    timer->CNT = 0; // counter = 0
    timer->SR &= ~TIM_SR_UIF; // SR = 0
    timer->DIER |= TIM_DIER_UIE;
}

/*
 * Set the period that the timer will fire (in milliseconds). A timer interrupt should be fired for each timer
period.
 */
void timer_set_ms(TIM_TypeDef* timer, uint16_t period_ms)
{
	timer_reset(TIM2);
	timer->ARR = 8000*period_ms; // 0x003D0900 = 4,000,000 -> 80000
}
