#include <stdint.h>

#include <arch/board/board.h>

#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>

#define SONAR_INPUT_MAX_TIMERS 1

struct sonar_input_timer 
{
	uint32_t	base;
	uint32_t	clock_register;
	uint32_t	clock_bit;
	uint32_t	vector;
	uint32_t	clock_freq;
};

__EXPORT const struct sonar_input_timer sonar_timers[SONAR_INPUT_MAX_TIMERS] = {
	{	/* Timer 3 */
		.base 		= STM32_TIM3_BASE,
		.clock_register = STM32_RCC_APB1ENR,
		.clock_bit 	= RCC_APB1ENR_TIM3EN,
		.vector		= STM32_IRQ_TIM3,
		.clock_freq 	= STM32_APB1_TIM3_CLKIN
	}
};
