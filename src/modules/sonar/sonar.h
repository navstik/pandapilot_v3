#include <stdint.h>

#include <arch/board/board.h>

#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>

#define SONAR_INPUT_MAX_TIMERS 1
#define REG(_tmr, _reg)	(*(volatile uint32_t *)(sonar_timers[_tmr].base + _reg))

#define rCR1(_tmr)    	REG(_tmr, STM32_GTIM_CR1_OFFSET)
#define rCR2(_tmr)    	REG(_tmr, STM32_GTIM_CR2_OFFSET)
#define rSMCR(_tmr)   	REG(_tmr, STM32_GTIM_SMCR_OFFSET)
#define rDIER(_tmr)   	REG(_tmr, STM32_GTIM_DIER_OFFSET)
#define rSR(_tmr)     	REG(_tmr, STM32_GTIM_SR_OFFSET)
#define rEGR(_tmr)    	REG(_tmr, STM32_GTIM_EGR_OFFSET)
#define rCCMR1(_tmr)  	REG(_tmr, STM32_GTIM_CCMR1_OFFSET)
#define rCCMR2(_tmr)  	REG(_tmr, STM32_GTIM_CCMR2_OFFSET)
#define rCCER(_tmr)   	REG(_tmr, STM32_GTIM_CCER_OFFSET)
#define rCNT(_tmr)    	REG(_tmr, STM32_GTIM_CNT_OFFSET)
#define rPSC(_tmr)    	REG(_tmr, STM32_GTIM_PSC_OFFSET)
#define rARR(_tmr)    	REG(_tmr, STM32_GTIM_ARR_OFFSET)
#define rCCR1(_tmr)   	REG(_tmr, STM32_GTIM_CCR1_OFFSET)
#define rCCR2(_tmr)   	REG(_tmr, STM32_GTIM_CCR2_OFFSET)
#define rCCR3(_tmr)   	REG(_tmr, STM32_GTIM_CCR3_OFFSET)
#define rCCR4(_tmr)   	REG(_tmr, STM32_GTIM_CCR4_OFFSET)
#define rDCR(_tmr)    	REG(_tmr, STM32_GTIM_DCR_OFFSET)
#define rDMAR(_tmr)   	REG(_tmr, STM32_GTIM_DMAR_OFFSET)
#define rBDTR(_tmr)	REG(_tmr, STM32_ATIM_BDTR_OFFSET)

struct sonar_input_timer 
{
	uint32_t	base;
	uint32_t	clock_register;
	uint32_t	clock_bit;
	uint32_t	vector;
	uint32_t	clock_freq;
};

__EXPORT const struct sonar_input_timer sonar_timers[SONAR_INPUT_MAX_TIMERS] = {
	{	/* Timer 2 */
		.base 		= STM32_TIM2_BASE,
		.clock_register = STM32_RCC_APB1ENR,
		.clock_bit 	= RCC_APB1ENR_TIM2EN,
		.vector		= STM32_IRQ_TIM2,
		.clock_freq 	= STM32_APB1_TIM2_CLKIN
	}
};
