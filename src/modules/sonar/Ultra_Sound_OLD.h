#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <sys/types.h>
#include <stdbool.h>

#include <assert.h>
#include <debug.h>
#include <time.h>
#include <queue.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <arch/board/board.h>

#include <chip.h>
#include <up_internal.h>
#include <up_arch.h>

#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>
#include "input.h" 

static int set_timer(unsigned timer);
void attach_isr(void);
static int tim_isr14(void);
void enable_irq(void);
void trigger(void);

uint16_t rc5;
uint16_t rc5_last;
uint32_t status;
uint16_t flag=0;
uint16_t dist_obj;


#define MAX_PULSEWIDTH		15000
#define ECHO			GPIO_TIM3_CH4IN_1

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


struct Ultra_Sound_s 
{
	 uint16_t hightime; 					/**< High time of the echo pulse from the distant object **/
	 float dist_obj;						/**<Distance of the device from the object in cm **/
	 uint16_t rc5;
	 uint16_t rc5_last;
};

struct Ultra_Sound_s U_s_s;

float  Ultra_Sound_measure_once();
__EXPORT int Ultra_test_main(int argc, char *argv[]);

 
int Ultra_test_main(int argc, char *argv[])
{	float dist = 0;
	stm32_configgpio(ECHO);
	int i=0;	
	attach_isr();
	set_timer(2);						//timer3 Channel 4 (PB1)
	enable_irq();
	while(i<10)
	{
		//dist = Ultra_Sound_measure_once();
		printf("Distance is %.4f meters\n", U_s_s.dist_obj);
		usleep(2000000);
		i++;
	}	
	return;
}

float Ultra_Sound_measure_once()
{
	
	return U_s_s.dist_obj;

 }

static int set_timer(unsigned timer)
{

	/* enable the timer clock before we try to talk to it */
	modifyreg32(sonar_timers[timer].clock_register, 0, sonar_timers[timer].clock_bit);

	rCR1(timer) |= 0;
	rCR2(timer) |= 0;
	rSMCR(timer) |= 0;
	rDIER(timer) |= 0;
	rCCER(timer) |= 0;
	rCCMR1(timer) |= 0;
	rCCMR2(timer) |= 0;
	rCCER(timer) |= 0;
	rDCR(timer) |= 0;
	/* configure the timer to free-run at 1MHz */
	rPSC(timer) |= (sonar_timers[timer].clock_freq / 1000000) - 1;
	rARR(timer) |= 0xffff;

	rCCMR2(timer) |= ((GTIM_CCMR_CCS_CCIN1<<GTIM_CCMR2_CC4S_SHIFT)|(GTIM_CCMR_ICF_FCKINT8<<GTIM_CCMR2_IC4F_SHIFT));
	rCCMR1(timer) |= 0;
	rCCER(timer) |= (GTIM_CCER_CC4E|GTIM_CCER_CC4P|GTIM_CCER_CC4NP);
	rDIER(timer) |= (GTIM_DIER_CC4IE);


	rEGR(timer) |= GTIM_EGR_UG ;
	/* enable the timer */
	rCR1(timer) |= GTIM_CR1_CEN;
}

void attach_isr(void)
{
	irq_attach(sonar_timers[2].vector, tim_isr14);
	return;
} 

void enable_irq(void)
{
	up_enable_irq(sonar_timers[2].vector);
	return;		
}

static int tim_isr14(void)
{

	status = rSR(2);
	//ack the interrupts we just read 
	rSR(2) = ~status;
	
	if (status & (GTIM_SR_CC4IF | GTIM_SR_CC4OF)) 
	{
		uint16_t count1 = rCCR4(2);
		rc5 = count1 - rc5_last;
		rc5_last = count1;

		if (rc5 <= MAX_PULSEWIDTH)
		{
			U_s_s.dist_obj = rc5 * 170 * 1e-6 ;
		}

  	}
	
 return;
}
