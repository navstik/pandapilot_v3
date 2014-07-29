/****************************************************************************
 *
 *   Copyright (C) 2012 PX4 Development Team. All rights reserved.
 *   Author: @author Lorenz Meier <lm@inf.ethz.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file sensor_sonar.h
 * Definition of the ultrasonic sensor uORB topic.
 */

#ifndef TOPIC_SENSOR_SONAR_H_
#define TOPIC_SENSOR_SONAR_H_

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"

/**
 * @addtogroup topics
 */

/**
 * Ultrasonic sensor data in 6 directions.
 *
 * @see http://en.wikipedia.org/wiki/International_System_of_Units
 */
struct sensor_sonar_s {

	uint64_t timestamp;		/**< in microseconds since system start          */
	float front_x_plus_m;	/**< Altitude / distance to object in front in meters */
	float back_x_minus_m;	/**< Altitude / distance to object in back in meters */
	float left_y_minus_m;	/**< Altitude / distance to object on left in meters */
	float right_y_plus_m;	/**< Altitude / distance to object to right in meters */
	float up_z_plus_m;	/**< Altitude / distance to object above in meters */
	float ground_z_minus_m;	/**< Altitude / distance to object below in meters */
	
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(sensor_sonar);

#endif
