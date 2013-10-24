#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>
#include "Ultra_Sound.h"

uint16_t dist;



__EXPORT int Ultra_test_main(int argc, char *argv[]);

 
int Ultra_test_main(int argc, char *argv[])
{
	
	dist = Ultra_Sound_measure_once();
	
	printf("Distance is %u\n", dist);	
	return;
}
