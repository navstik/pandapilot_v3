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

/* PWM Input 
 * RC1 PA7	Timer 14 Channel 1 (AF9)
 * RC2 PA6	Timer 3 Channel 1 (AF2)
 * RC3 PB14 	Timer 12 Channel 1 (AF9)
 * RC4 PB15	Timer 12 Channel 2 (AF9)
 * RC5 PC7	Timer 8 Channel 2 (AF3)
 * RC6 PC6	Timer 8 Channel 1 (AF3)
 
 */

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
uint16_t dist=0;

#define MAX_PULSEWIDTH		15000
#define RC5  				GPIO_TIM8_CH2IN_1
#define RC6 				(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN6)


#define REG(_tmr, _reg)	(*(volatile uint32_t *)(input_timers[_tmr].base + _reg))

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

#define ArCR1(_tmr)    	REG(_tmr, STM32_ATIM_CR1_OFFSET)
#define ArCR2(_tmr)    	REG(_tmr, STM32_ATIM_CR2_OFFSET)
#define ArSMCR(_tmr)   	REG(_tmr, STM32_ATIM_SMCR_OFFSET)
#define ArDIER(_tmr)   	REG(_tmr, STM32_ATIM_DIER_OFFSET)
#define ArSR(_tmr)     	REG(_tmr, STM32_ATIM_SR_OFFSET)
#define ArEGR(_tmr)    	REG(_tmr, STM32_ATIM_EGR_OFFSET)
#define ArCCMR1(_tmr)  	REG(_tmr, STM32_ATIM_CCMR1_OFFSET)
#define ArCCMR2(_tmr)  	REG(_tmr, STM32_ATIM_CCMR2_OFFSET)
#define ArCCER(_tmr)   	REG(_tmr, STM32_ATIM_CCER_OFFSET)
#define ArCNT(_tmr)    	REG(_tmr, STM32_ATIM_CNT_OFFSET)
#define ArPSC(_tmr)    	REG(_tmr, STM32_ATIM_PSC_OFFSET)
#define ArARR(_tmr)    	REG(_tmr, STM32_ATIM_ARR_OFFSET)
#define ArCCR1(_tmr)   	REG(_tmr, STM32_ATIM_CCR1_OFFSET)
#define ArCCR2(_tmr)   	REG(_tmr, STM32_ATIM_CCR2_OFFSET)
#define ArCCR3(_tmr)   	REG(_tmr, STM32_ATIM_CCR3_OFFSET)
#define ArCCR4(_tmr)   	REG(_tmr, STM32_ATIM_CCR4_OFFSET)
#define ArBDTR(_tmr)	REG(_tmr, STM32_ATIM_BDTR_OFFSET)
#define ArDCR(_tmr)    	REG(_tmr, STM32_ATIM_DCR_OFFSET)
#define ArDMAR(_tmr)   	REG(_tmr, STM32_ATIM_DMAR_OFFSET)


struct Ultra_Sound_s 
{
	 uint16_t hightime; 					/**< High time of the echo pulse from the distant object **/
	 uint16_t dist_obj;						/**<Distance of the device from the object in mm **/
	 uint16_t rc5;
	 uint16_t rc5_last;
};

struct Ultra_Sound_s U_s_s;

int  Ultra_Sound_measure_once();

int Ultra_Sound_measure_once()
{
	stm32_configgpio(RC6);
	stm32_configgpio(RC5);
	
	attach_isr();
	set_timer(3);						//timer8
	enable_irq();

	volatile int i = 2;
	while (i != 0)	
	{
		trigger();
		i--;
	}	
	usleep(10000);	
	
	return U_s_s.dist_obj;

 }


void trigger(void)
{
	volatile int i = 0;

	stm32_gpiowrite(RC6,0);
	
	stm32_gpiowrite(RC6,1);
	for (i = 0; i < 160; i++);
	
	stm32_gpiowrite(RC6,0);
	for (i = 0; i < 160; i++);

	return;
}

static int set_timer(unsigned timer)
{

	/* enable the timer clock before we try to talk to it */
	modifyreg32(input_timers[timer].clock_register, 0, input_timers[timer].clock_bit);

	ArCR1(timer) |= 0;
	ArCR2(timer) |= 0;
	ArSMCR(timer) |= 0;
	ArDIER(timer) |= 0;
	ArCCER(timer) |= 0;
	ArCCMR1(timer) |= 0;
	ArCCMR2(timer) |= 0;
	ArCCER(timer) |= 0;
	ArDCR(timer) |= 0;
	/* configure the timer to free-run at 1MHz */
	ArPSC(timer) = (input_timers[timer].clock_freq / 1000000) - 1;
	ArARR(timer) |= 0xffff;


	ArCCMR1(timer) |= ((ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC1S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC1F_SHIFT)|(ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC2S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC2F_SHIFT));
	ArCCMR2(timer) |= 0;
	ArCCER(timer) |= (ATIM_CCER_CC1E|ATIM_CCER_CC2E|ATIM_CCER_CC1P|ATIM_CCER_CC1NP|ATIM_CCER_CC2P|ATIM_CCER_CC2NP);
	ArDIER(timer) |= (ATIM_DIER_CC1IE|ATIM_DIER_CC2IE);


	ArEGR(timer) |= ATIM_EGR_UG ;
	/* enable the timer */
	ArCR1(timer) |= GTIM_CR1_CEN;
}

void attach_isr(void)
{
	irq_attach(input_timers[3].vector, tim_isr14);
	return;
} 

void enable_irq(void)
{
	up_enable_irq(input_timers[3].vector);
	return;		
}

static int tim_isr14(void)
{

	status = rSR(3);
	//ack the interrupts we just read 
	rSR(3) = ~status;
	
	if (status & (ATIM_SR_CC2IF | ATIM_SR_CC2OF)) 
	{
		uint16_t count1 = rCCR2(3);
		//printf("Captured on RC5  %u\n", count1);
		flag += 1;
		
		if(rc5_last != 0)
		{
			rc5 = count1 - rc5_last;
		}

		rc5_last = count1;

		if (rc5 <= MAX_PULSEWIDTH && rc5 != 0)
		{
			dist_obj = rc5 * 170 * 0.001;
		}

  	}
	

	if (flag == 2)
	{
		flag = 0;
		U_s_s.dist_obj  = dist_obj;
		U_s_s.rc5 		= rc5;
		U_s_s.rc5_last	= rc5_last;
		U_s_s.hightime 	= rc5;	
	
		rc5 = 0;
		rc5_last = 0;
		up_disable_irq(input_timers[3].vector);
	}
	/*U_s_s.dist_obj  = dist_obj;
	U_s_s.rc5 		= rc5;
	U_s_s.rc5_last	= rc5_last;
	U_s_s.hightime 	= rc5;	*/
	/*
	printf("flagged %u times ",flag);
	printf("rc5_last:  %u\n", rc5_last);
	printf("The high Level time of the echo pulse is :  %u\n", rc5);
	printf("Object detected at %u mm distance \n", dist_obj);
	*/
  	return;
}

//////////////////////////////////////////// TRY CODE ///////////////////////////////////////////////
/*#include <nuttx/config.h>
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
#include <stdlib.h>
#include <math.h>


#define MAX_PULSEWIDTH 		15000
#define RC5  				GPIO_TIM8_CH2IN_1
#define RC6 				(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN6)

static int set_timer(unsigned timer);
void attach_isr(void);
static int tim_isr14(void);
static int tim_isr1(void);
void enable_irq(void);

void trigger(void);


uint32_t status;
uint16_t rc5 = 0;
uint16_t rc5_last = 0 ;
uint16_t dist_obj;
uint16_t flag =0;


#define REG(_tmr, _reg)	(*(volatile uint32_t *)(input_timers[_tmr].base + _reg))

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

#define ArCR1(_tmr)    	REG(_tmr, STM32_ATIM_CR1_OFFSET)
#define ArCR2(_tmr)    	REG(_tmr, STM32_ATIM_CR2_OFFSET)
#define ArSMCR(_tmr)   	REG(_tmr, STM32_ATIM_SMCR_OFFSET)
#define ArDIER(_tmr)   	REG(_tmr, STM32_ATIM_DIER_OFFSET)
#define ArSR(_tmr)     	REG(_tmr, STM32_ATIM_SR_OFFSET)
#define ArEGR(_tmr)    	REG(_tmr, STM32_ATIM_EGR_OFFSET)
#define ArCCMR1(_tmr)  	REG(_tmr, STM32_ATIM_CCMR1_OFFSET)
#define ArCCMR2(_tmr)  	REG(_tmr, STM32_ATIM_CCMR2_OFFSET)
#define ArCCER(_tmr)   	REG(_tmr, STM32_ATIM_CCER_OFFSET)
#define ArCNT(_tmr)    	REG(_tmr, STM32_ATIM_CNT_OFFSET)
#define ArPSC(_tmr)    	REG(_tmr, STM32_ATIM_PSC_OFFSET)
#define ArARR(_tmr)    	REG(_tmr, STM32_ATIM_ARR_OFFSET)
#define rcArCCR1(_tmr)  REG(_tmr, STM32_ATIM_CCR1_OFFSET)
#define ArCCR2(_tmr)   	REG(_tmr, STM32_ATIM_CCR2_OFFSET)
#define ArCCR3(_tmr)   	REG(_tmr, STM32_ATIM_CCR3_OFFSET)
#define ArCCR4(_tmr)   	REG(_tmr, STM32_ATIM_CCR4_OFFSET)
#define ArBDTR(_tmr)	REG(_tmr, STM32_ATIM_BDTR_OFFSET)
#define ArDCR(_tmr)    	REG(_tmr, STM32_ATIM_DCR_OFFSET)
#define ArDMAR(_tmr)   	REG(_tmr, STM32_ATIM_DMAR_OFFSET)


__EXPORT int  Ultra_Sound_main(int argc, char *argv[]);


int Ultra_Sound_main(int argc, char *argv[])
{
	

	//volatile int countu = atoi(argv[1]);
	
		stm32_configgpio(RC6);
		stm32_configgpio(RC5);

		attach_isr();
		set_timer(3);						//timer8
		enable_irq();
		
		
		printf ("\nTriggering the device !!! ") ;
		trigger();				
		
		return;
					
}

void trigger(void)
{

		volatile int i = 0;
	
		stm32_gpiowrite(RC6,0);
		
		stm32_gpiowrite(RC6,1);
		for (i = 0; i < 160; i++);
		
		stm32_gpiowrite(RC6,0);
		for (i = 0; i < 160; i++);

		return;
	
}

static int set_timer(unsigned timer)
{
	// enable the timer clock before we try to talk to it 
	modifyreg32(input_timers[timer].clock_register, 0, input_timers[timer].clock_bit);

	ArCR1(timer) 	|= 0;
	ArCR2(timer)	|= 0;
	ArSMCR(timer)   |= 0;
	ArDIER(timer)   |= 0;
	ArCCER(timer)   |= 0;
	ArCCMR1(timer)  |= 0;
	ArCCMR2(timer)  |= 0;
	ArCCER(timer)   |= 0;
	ArDCR(timer)    |= 0;
	ArARR(timer)    |= 0xffff;


	ArCCMR1(timer) |= ((ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC1S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC1F_SHIFT)|(ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC2S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC2F_SHIFT));
	ArCCMR2(timer) |= 0;
	ArCCER(timer) |= (ATIM_CCER_CC1E|ATIM_CCER_CC2E|ATIM_CCER_CC1P|ATIM_CCER_CC1NP|ATIM_CCER_CC2P|ATIM_CCER_CC2NP);
	ArDIER(timer) |= (ATIM_DIER_CC1IE|ATIM_DIER_CC2IE);
	
	// generate an update event; reloads the counter, all registers
	ArEGR(timer)    |= ATIM_EGR_UG ;
	// enable the timer 
	ArCR1(timer)    |= GTIM_CR1_CEN;
	//printf("Entering set_timer\n");
	
}

void enable_irq(void)
{
	up_enable_irq(input_timers[3].vector);
	return;		
}

void attach_isr(void)
{	
	irq_attach(input_timers[3].vector, tim_isr14);
	return;
} 

static int tim_isr14(void)
{

	status = rSR(3);
	//ack the interrupts we just read 
	rSR(3) = ~status;
	
	if (status & (ATIM_SR_CC2IF | ATIM_SR_CC2OF)) 
	{
		uint16_t count1 = rCCR2(3);
		//printf("Captured on RC5  %u\n", count1);
		flag += 1;
		
		if(rc5_last != 0)
		{
			rc5 = count1 - rc5_last;
		}

		rc5_last = count1;

		if (rc5 <= MAX_PULSEWIDTH && rc5 != 0)
		{
			dist_obj = rc5 * 170 * 0.001;
		}

  	}
	printf("flagged %u times ",flag);
	printf("rc5_last:  %u\n", rc5_last);
	printf("The high Level time of the echo pulse is :  %u\n", rc5);
	printf("Object detected at %u mm distance \n", dist_obj);
  	return;
}


/*
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
#include <stdlib.h>
#include <math.h>


#define MAX_PULSEWIDTH 		15000
#define RC5  				GPIO_TIM8_CH2IN_1
#define RC6 				(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN6)

static int set_timer(unsigned timer);
void attach_isr(void);
static int tim_isr14(void);
static int tim_isr1(void);
void enable_irq(void);

void trigger(void);


uint32_t status;
uint32_t flag;
uint16_t rc5 = 0;
uint16_t rc5_last = 0 ;
uint16_t reading;
uint16_t dist_obj;


#define REG(_tmr, _reg)	(*(volatile uint32_t *)(input_timers[_tmr].base + _reg))

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

#define ArCR1(_tmr)    	REG(_tmr, STM32_ATIM_CR1_OFFSET)
#define ArCR2(_tmr)    	REG(_tmr, STM32_ATIM_CR2_OFFSET)
#define ArSMCR(_tmr)   	REG(_tmr, STM32_ATIM_SMCR_OFFSET)
#define ArDIER(_tmr)   	REG(_tmr, STM32_ATIM_DIER_OFFSET)
#define ArSR(_tmr)     	REG(_tmr, STM32_ATIM_SR_OFFSET)
#define ArEGR(_tmr)    	REG(_tmr, STM32_ATIM_EGR_OFFSET)
#define ArCCMR1(_tmr)  	REG(_tmr, STM32_ATIM_CCMR1_OFFSET)
#define ArCCMR2(_tmr)  	REG(_tmr, STM32_ATIM_CCMR2_OFFSET)
#define ArCCER(_tmr)   	REG(_tmr, STM32_ATIM_CCER_OFFSET)
#define ArCNT(_tmr)    	REG(_tmr, STM32_ATIM_CNT_OFFSET)
#define ArPSC(_tmr)    	REG(_tmr, STM32_ATIM_PSC_OFFSET)
#define ArARR(_tmr)    	REG(_tmr, STM32_ATIM_ARR_OFFSET)
#define rcArCCR1(_tmr)  REG(_tmr, STM32_ATIM_CCR1_OFFSET)
#define ArCCR2(_tmr)   	REG(_tmr, STM32_ATIM_CCR2_OFFSET)
#define ArCCR3(_tmr)   	REG(_tmr, STM32_ATIM_CCR3_OFFSET)
#define ArCCR4(_tmr)   	REG(_tmr, STM32_ATIM_CCR4_OFFSET)
#define ArBDTR(_tmr)	REG(_tmr, STM32_ATIM_BDTR_OFFSET)
#define ArDCR(_tmr)    	REG(_tmr, STM32_ATIM_DCR_OFFSET)
#define ArDMAR(_tmr)   	REG(_tmr, STM32_ATIM_DMAR_OFFSET)


__EXPORT int  Ultra_Sound_main(int argc, char *argv[]);


int Ultra_Sound_main(int argc, char *argv[])
{
	

	stm32_configgpio(RC6);
	stm32_configgpio(RC5);
	attach_isr();
	set_timer(3);//timer8
	enable_irq();
	printf("Configuration done \n");
		
	trigger();				
}


void trigger(void)
{

	printf ("\nTriggering the device !!! ") ;

	set_timer(2);//timer3

	stm32_gpiowrite(RC6,1);

	rEGR(2) |= GTIM_EGR_UG ;
	rCR1(2) |= GTIM_CR1_CEN;

	stm32_gpiowrite(RC6,0);
	
	
	rEGR(2) |= GTIM_EGR_UG ;
	rCR1(2) |= GTIM_CR1_CEN;
	
	return;

}


static int set_timer(unsigned timer)
{
	//printf("Entering set_timer\n");
	// enable the timer clock before we try to talk to it 
	modifyreg32(input_timers[timer].clock_register, 0, input_timers[timer].clock_bit);
	
	if (timer == 3)
	{
		ArCR1(timer) |= 0;
		ArCR2(timer) |= 0;
		ArSMCR(timer) |= 0;
		ArDIER(timer) |= 0;
		ArCCER(timer) |= 0;
		ArCCMR1(timer) |= 0;
		ArCCMR2(timer) |= 0;
		ArCCER(timer) |= 0;
		ArDCR(timer) |= 0;

		//timer 8 - channel 1&2
	
		ArCCMR1(timer) |= ((ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC1S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC1F_SHIFT)|(ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC2S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC2F_SHIFT));
		ArCCMR2(timer) |= 0;
		ArCCER(timer) |= (ATIM_CCER_CC1E|ATIM_CCER_CC2E|ATIM_CCER_CC1P|ATIM_CCER_CC1NP|ATIM_CCER_CC2P|ATIM_CCER_CC2NP);
		ArDIER(timer) |= (ATIM_DIER_CC1IE|ATIM_DIER_CC2IE);
	

		ArPSC(timer) |= (input_timers[timer].clock_freq / 1000000) - 1;
		ArARR(timer) |= 0xffff;// Configuring at 1 Mhz Clock
	
		// generate an update event; reloads the counter, all registers
		ArEGR(timer) |= ATIM_EGR_UG ;
		// enable the timer 
		ArCR1(timer) |= GTIM_CR1_CEN;

	}

	else if ( timer == 2)
	{
		rCR1(timer) |= 0;
		rCR2(timer) |= 0;
		rSMCR(timer) |= 0;
		rDIER(timer) |= 0;
		rCCER(timer) |= 0;
		rCCMR1(timer) |= 0;
		rCCMR2(timer) |= 0;
		rCCER(timer) |= 0;
		rDCR(timer) |= 0;
		// configure the timer to free-run at 1MHz 
		rPSC(timer) |= (input_timers[timer].clock_freq / 1000000) - 1;
		rARR(timer) |= 65535;


		ArCCMR1(timer) |= ((ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC1S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC1F_SHIFT)|(ATIM_CCMR_CCS_CCIN1<<ATIM_CCMR1_CC2S_SHIFT)|(ATIM_CCMR_ICF_FCKINT8<<ATIM_CCMR1_IC2F_SHIFT));



	}
}

void enable_irq(void)
{
	//printf("Entering enable_irq\n");
	up_enable_irq(input_timers[3].vector);
	up_enable_irq(input_timers[2].vector);
	return;		
}

void attach_isr(void)
{	
	irq_attach(input_timers[2].vector, tim_isr1);
	irq_attach(input_timers[3].vector, tim_isr14);
	return;
} 

static int tim_isr14(void)
{

	status = rSR(3);

	 //ack the interrupts we just read 
	rSR(3) = ~status;
	
	if (status & (ATIM_SR_CC2IF | ATIM_SR_CC2OF)) 
	{
		uint16_t count1 = rCCR2(3);
		printf("Captured on RC5 %u\n", count1);
	  	
	  		
		// ow long since the last edge? 
		if(rc5_last != 0)
		{
			rc5 = count1 - rc5_last;
			
		}
		rc5_last = count1;

		if (rc5 <= MAX_PULSEWIDTH && rc5 != 0)
		{
			//reading = count1;
			//printf("The object is at a lesser distance than 4m \n");

  		printf("rc5_last:  %u\n", rc5_last);
  		printf("The high Level time of the echo pulse is :  %u\n", rc5);

		dist_obj = rc5 * 170 * 0.001;
		//printf("%f\n", );

		printf("Object detected at %u mm distance \n", dist_obj);
		}
		

  	}

 	return;
}

static int tim_isr1(void)
{
	
	flag = rSR(2);
	if( flag & (ATIM_SR_UIF))
	{
		rSR(2) &= ~ATIM_SR_UIF;
		stm32_gpiowrite(RC6,1);
 		ArARR(2) = 0x00;
 	}
 	printf("tim_isr1 done\n");
 	return;
}
*/