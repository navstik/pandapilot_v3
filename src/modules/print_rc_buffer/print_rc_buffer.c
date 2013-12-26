#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>
 
__EXPORT int print_rc_buffer_main(int argc, char *argv[]);
extern uint16_t rc_buffer[6];
extern volatile float sonar_distance;
 
int print_rc_buffer_main(int argc, char *argv[])
{
	int i=0 ;

	for (i=0;i<6;i++)
	printf("%u\n",rc_buffer[i]);
	while(i<15)
	{
		printf("Ground Distance is %.4f meters\n", sonar_distance);
		usleep(2000000);
		i++;
	}
	return OK;
}
