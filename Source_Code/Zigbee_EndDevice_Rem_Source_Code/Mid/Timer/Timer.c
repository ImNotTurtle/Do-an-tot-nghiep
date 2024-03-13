/*
 * Timer.c
 *
 *  Created on: Feb 26, 2024
 *      Author: Administrator
 */
#include "Timer.h"

EmberEventControl systemTimerEventControl;

static uint32_t s_systemTick = 0;
static uint32_t s_systemCounter = 0;

void Timer_Init(uint32_t msTick){
	s_systemCounter = 0;
	s_systemTick = msTick;
	emberEventControlSetDelayMS(systemTimerEventControl, s_systemTick);
}
void Timer_SetTimerTick(uint32_t tick){
	s_systemTick = tick;
}
uint32_t Timer_GetCounter(void){
	return s_systemCounter;
}
//time2 need to be greater than time1, time2 smaller than time1 only when time counter is overflow
uint32_t Timer_CalcTimeDiff(uint32_t time1, uint32_t time2){
	if(time2 >= time1){
		return time2 - time1;
	}
	return 0xFFFFFFFFU - time1 + time2;
}
bool Timer_IsApproximate(uint32_t time1, uint32_t time2){
	return false;
}

void systemTimerEventHandler(void){
	emberEventControlSetInactive(systemTimerEventControl);

	s_systemCounter += s_systemTick;

	emberEventControlSetDelayMS(systemTimerEventControl, s_systemTick);
}
