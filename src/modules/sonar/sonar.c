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
#include <uORB/uORB.h>
#include <uORB/topics/optical_flow.h>
#include <drivers/drv_hrt.h>
#include <systemlib/systemlib.h>
#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>
#include "sonar.h" 

static int set_timer(unsigned timer);
void attach_isr(void);
static int sonar_isr(void);
void enable_irq(void);

#define MAX_PULSEWIDTH		50000
#define ECHO			GPIO_TIM2_CH4IN_2

uint16_t htime;			/*High Time of Echo Pulse*/
uint16_t htime_last;
uint32_t status;
__EXPORT volatile float sonar_distance = 0;

__EXPORT int sonar_main(int argc, char *argv[]);
int sonar_thread_main(int argc, char *argv[]);
static bool thread_should_exit = false; /**< Deamon exit flag */
static bool thread_running = false; /**< Deamon status flag */
static int sonar_task; /**< Handle of deamon task / thread */

int sonar_main(int argc, char *argv[])
{	
	if (argc < 1)
		warnx("missing command");

	if (!strcmp(argv[1], "start")) {
		if (thread_running) {
			printf("sonar already running\n");
			/* this is not an error */
			exit(0);
		}

		thread_should_exit = false;
		sonar_task = task_spawn_cmd("sonar",
					       SCHED_RR, SCHED_PRIORITY_MAX - 5, 2048,
					       sonar_thread_main,
					       (argv) ? (const char **) &argv[2] : (const char **) NULL);
		exit(0);
	}

	if (!strcmp(argv[1], "stop")) {
		thread_should_exit = true;
		exit(0);
	}
	
	exit(1);
}

int sonar_thread_main(int argc, char *argv[])
{
	/* declare and safely initialize all structs */
	struct optical_flow_s flow;
	memset(&flow, 0, sizeof(flow));
	/* advertise */
	orb_advert_t optical_flow_pub = orb_advertise(ORB_ID(optical_flow), &flow);
	thread_running = true;
	stm32_configgpio(ECHO);
	attach_isr();
	set_timer(0);						//timer2 Channel 4 (PB11)
	enable_irq();
	while(!thread_should_exit)
	{	
		flow.timestamp = hrt_absolute_time();
		flow.flow_raw_x=0;
		flow.flow_raw_y=0;
		flow.flow_comp_x_m=0;
		flow.flow_comp_y_m=0;
		flow.ground_distance_m=sonar_distance;
		flow.quality=0;
		flow.sensor_id=0;
		orb_publish(ORB_ID(optical_flow), optical_flow_pub, &flow);
		usleep(130000);
	}
	thread_running = false;
	return 0;	
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
		
		sonar_distance = htime * 170 * 1e-6 ;

		if (sonar_distance >3.5f)
		{
			sonar_distance = 0 ;
		}		

  	}
 return;
}
