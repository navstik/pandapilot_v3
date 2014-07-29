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
#include <uORB/topics/sensor_sonar.h>
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
#define TRIGGER			(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_PORTB|GPIO_PIN1) 

uint16_t echo_start;			// echo high timestamp
uint16_t echo_end;				// echo low timestamp
uint16_t dist_count;			// temp variable for calculating distance
int sync_error;					// for resuming sonar after prolonged errors with proper synchronisation
uint32_t status;
bool rise_fall;        // rising interrupt == true, falling interrupt == false
bool cycle_end;      	// true = one reading cycle is finished and can trigger again, false = rading is ongoing
//bool overflowflag;
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
		sonar_task = task_spawn_cmd("sonar", SCHED_DEFAULT, 50, 2048,
					       sonar_thread_main, (argv) ? (const char **) &argv[2] : (const char **) NULL);
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
		struct sensor_sonar_s sonar_att;
		memset(&sonar_att, 0, sizeof(sonar_att));
		/* advertise */
		orb_advert_t sensor_sonar_pub = orb_advertise(ORB_ID(sensor_sonar), &sonar_att);
		thread_running = true;
		stm32_configgpio(TRIGGER);
		stm32_configgpio(ECHO);
		usleep(300000);
		attach_isr();
		set_timer(0);						//timer2 Channel 4 (PB11)
		enable_irq();
		rise_fall = true;
		cycle_end = false;
		sync_error = 0;
		sonar_trigger();
		while(!thread_should_exit)
		{	
			usleep(20000);
		//nothing to do here...just waiting
			if(cycle_end == true)
			{		
				if (echo_start > echo_end)
					dist_count = 65535 - echo_start + echo_end;
				else
					dist_count = echo_end - echo_start;
				sonar_distance = dist_count * 1360 * 1e-6;
				if (sonar_distance > 6.0f)
				{
					sonar_distance = 0.0f;
				}
				sonar_att.timestamp = hrt_absolute_time();
				sonar_att.front_x_plus_m=0;
				sonar_att.back_x_minus_m=0;
				sonar_att.left_y_minus_m=0;
				sonar_att.right_y_plus_m=0;
				sonar_att.up_z_plus_m=0;
				sonar_att.ground_z_minus_m=sonar_distance;
				orb_publish(ORB_ID(sensor_sonar), sensor_sonar_pub, &sonar_att);
				cycle_end = false;
				rise_fall = true;
				sonar_trigger();
			}
			sync_error += 1;
			if (sync_error > 20)
				{
				cycle_end = false;
				rise_fall = true;
				sonar_trigger();
				sync_error = 0;
				}	

		}
		thread_running = false; 
		return 0;	
	}

static int sonar_isr(void)
	{
		status = rSR(0);
		//ack the interrupts we just read 
		rSR(0) = ~status;
		
		if (status & (GTIM_SR_CC4IF | GTIM_SR_CC4OF)) 
		{
			
			if (rise_fall == true)
			{
				uint16_t count1 = rCCR4(0);
				echo_start = count1;
				rise_fall = false;
			}
			else
			{
				uint16_t count2 = rCCR4(0);
				echo_end = count2;
				cycle_end = true;
				rise_fall = true;
				sync_error = 0;
			} 
		}
	 return;
	}

// #################################Supporting functions##############################


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
	rPSC(timer) |= (sonar_timers[timer].clock_freq / 125000) - 1;
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



int sonar_trigger(void)
	{
		usleep(5000);
		stm32_gpiowrite(TRIGGER, true);
		usleep(10);
		stm32_gpiowrite(TRIGGER, false);
		return 0;
	}





