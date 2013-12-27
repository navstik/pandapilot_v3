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
#include <drivers/drv_pwm_output.h>
#include <drivers/stm32/drv_pwm_servo.h>
#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>
#include "sonar.h" 

static int set_timer(unsigned timer);
static int set_trigger_timer(unsigned timer);
void attach_isr(void);
static int sonar_isr(void);
void enable_irq(void);
static void sonar_trigger(void);

#define MAX_PULSEWIDTH		15000
#define TRIGGER			GPIO_TIM2_CH4OUT_2
#define ECHO			GPIO_TIM3_CH4IN_1

uint16_t htime;			/*High Time of Echo Pulse*/
uint16_t htime_last;
uint32_t status;
__EXPORT volatile float sonar_distance = 0;

__EXPORT int sonar_main(int argc, char *argv[]);

int sonar_main(int argc, char *argv[])
{	
	stm32_configgpio(TRIGGER);	
	sonar_trigger();
	stm32_configgpio(ECHO);
	int i=0;	
	attach_isr();
	set_timer(0);						//timer3 Channel 4 (PB1)
	enable_irq();
	return 1;
}

static void sonar_trigger(void)
{	
	set_trigger_timer(1); 					//timer2 Channel 4 (PB11)
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
	/*Channel 4 is configured as Input Capture Mode*/
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
	irq_attach(sonar_timers[0].vector, sonar_isr);
	return;
} 

void enable_irq(void)
{
	up_enable_irq(sonar_timers[0].vector);
	return;		
}

static int sonar_isr(void)
{
	status = rSR(0);
	//ack the interrupts we just read 
	rSR(0) = ~status;
	
	if (status & (GTIM_SR_CC4IF | GTIM_SR_CC4OF)) 
	{
		uint16_t count1 = rCCR4(0);
		htime = count1 - htime_last;
		htime_last = count1;

		if (htime <= MAX_PULSEWIDTH)
		{
			sonar_distance = htime * 170 * 1e-6 ;
		}
  	}
 return;
}

static int set_trigger_timer(unsigned timer)
{

	/* enable the timer clock before we try to talk to it */
	modifyreg32(sonar_timers[timer].clock_register, 0, sonar_timers[timer].clock_bit);

	/* disable and configure the timer */
	rCR1(timer) = 0;
	rCR2(timer) = 0;
	rSMCR(timer) = 0;
	rDIER(timer) = 0;
	rCCER(timer) = 0;
	rCCMR1(timer) = 0;
	rCCMR2(timer) = 0;
	rCCER(timer) = 0;
	rDCR(timer) = 0;

	if ((sonar_timers[timer].base == STM32_TIM1_BASE) || (sonar_timers[timer].base == STM32_TIM8_BASE)) {
		/* master output enable = on */
		rBDTR(timer) = ATIM_BDTR_MOE;
	}

	/* configure the timer to free-run at 1MHz */
	rPSC(timer) = (sonar_timers[timer].clock_freq / 1000000) - 1;

	/* configure the timer to update at the desired rate */
	rARR(timer) = 1000000 / 20;		//Timer 2 update rate set at 20 Hz

	/* generate an update event; reloads the counter and all registers */
	rEGR(timer) = GTIM_EGR_UG;
	/*Channel 4 is configured as PWM Mode*/
	rCCMR2(timer) |= (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR2_OC4M_SHIFT) | GTIM_CCMR2_OC4PE;
	rCCR4(timer) = 10;			//Configured 10 usec high time pulse to trigger sonar
	rCCER(timer) |= GTIM_CCER_CC4E;

	/* enable the timer */
	rCR1(timer) |= GTIM_CR1_CEN | GTIM_CR1_ARPE;
}
