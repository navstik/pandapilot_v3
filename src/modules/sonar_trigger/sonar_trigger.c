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


#define TRIGGER			(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_PORTB|GPIO_PIN1)

__EXPORT int sonar_trigger_main(int argc, char *argv[]);
int sonar_trigger_thread_main(int argc, char *argv[]);
static bool thread_should_exit = false; /**< Deamon exit flag */
static bool thread_running = false; /**< Deamon status flag */
static int sonar_trigger_task; /**< Handle of deamon task / thread */


int sonar_trigger_main(int argc, char *argv[])
{	
	if (argc < 1)
		warnx("missing command");

	if (!strcmp(argv[1], "start")) {
		if (thread_running) {
			printf("sonar_trigger already running\n");
			/* this is not an error */
			exit(0);
		}

		thread_should_exit = false;
		sonar_trigger_task = task_spawn_cmd("sonar_trigger",
					       SCHED_RR, SCHED_PRIORITY_MAX - 5, 2048,
					       sonar_trigger_thread_main,
					       (argv) ? (const char **) &argv[2] : (const char **) NULL);
		exit(0);
	}

	if (!strcmp(argv[1], "stop")) {
		thread_should_exit = true;
		exit(0);
	}
	
	exit(1);
}

int sonar_trigger_thread_main(int argc, char *argv[])
{
	stm32_configgpio(TRIGGER);
	thread_running = true;
	while(!thread_should_exit)
	{	
		stm32_gpiowrite(TRIGGER, true);
		usleep(10);
		stm32_gpiowrite(TRIGGER, false);
		usleep(100000);
	}
	thread_running = false;
	return 0;	
}
