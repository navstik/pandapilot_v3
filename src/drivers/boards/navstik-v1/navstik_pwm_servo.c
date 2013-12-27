/****************************************************************************
 *   Copyright (C) 2013 NavStik Development Team. All rights reserved. Based on PX4 port.
 *   Copyright (C) 2012 PX4 Development Team. All rights reserved.
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

/*
 * @file navstik_pwm_servo.c
 *
 * Configuration data for the stm32 pwm_servo driver.
 *
 * Note that these arrays must always be fully-sized.
 */

#include <stdint.h>

#include <drivers/stm32/drv_pwm_servo.h>

#include <arch/board/board.h>
#include <drivers/drv_pwm_output.h>

#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>
#include "board_config.h"
/* PWM
 *
 * The Navstik has six PWM outputs
 * The mapping are as follows :
 * PB5	Servo 1 Timer 3 Channel 2 (AF2)
 * PA10	Servo 2 Timer 1 channel 3 (AF1)
 * PC8	Servo 3 Timer 3 Channel 3 (AF2)
 * PB11	Servo 4 Timer 2 Channel 4 (AF1)
 * PB1	Servo 5 Timer 3 Channel 4 (AF2)
 * PB0	Servo 6 Timer 1 Channel 2 (AF1)
 *
 */

__EXPORT const struct pwm_servo_timer pwm_timers[PWM_SERVO_MAX_TIMERS] = {
	{	/* Timer 1 */
		.base = STM32_TIM1_BASE,
		.clock_register = STM32_RCC_APB2ENR,
		.clock_bit = RCC_APB2ENR_TIM1EN,
		.clock_freq = STM32_APB2_TIM1_CLKIN
	},
	{	/* Timer 2 */
		.base = 0,
		.clock_register = 0,
		.clock_bit = 0,
		.clock_freq = 0
	},
	{	/* Timer 3 */
		.base = STM32_TIM3_BASE,
		.clock_register = STM32_RCC_APB1ENR,
		.clock_bit = RCC_APB1ENR_TIM3EN,
		.clock_freq = STM32_APB1_TIM3_CLKIN
	},
	{	/* Timer 8 */
		.base = 0,
		.clock_register = 0,
		.clock_bit = 0,
		.clock_freq = 0
	}
};

__EXPORT const struct pwm_servo_channel pwm_channels[PWM_SERVO_MAX_CHANNELS] = {
	{
		.gpio = GPIO_TIM3_CH2OUT,
		.timer_index = 2,
		.timer_channel = 2,
		.default_value = 1000,
	},
	{
		.gpio = GPIO_TIM1_CH3OUT,
		.timer_index = 0,
		.timer_channel = 3,
		.default_value = 1000,
	},
	{	
		.gpio = GPIO_TIM3_CH3OUT,
		.timer_index = 2,
		.timer_channel = 3,
		.default_value = 1000,
		
	},
	{
		.gpio = GPIO_TIM1_CH2N,
		.timer_index = 0,
		.timer_channel = 5,
		.default_value = 1000,
	},
	{
		.gpio = 0,
		.timer_index = 0,
		.timer_channel = 0,
		.default_value = 0,
	},
	{	
		.gpio = 0,
		.timer_index = 0,
		.timer_channel = 0,
		.default_value = 0,
	}
};
