/*
 * Timer.h
 *
 *  Created on: Feb 26, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_MID_TIMER_TIMER_H_
#define SOURCE_MID_TIMER_TIMER_H_
#include "app/framework/include/af.h"
#include <stdbool.h>

#define TIMER_EPSILON

//void Timer_Init();
void Timer_Init(uint32_t msTick);
void Timer_SetTimerTick(uint32_t tick);
uint32_t Timer_GetCounter(void);
uint32_t Timer_CalcTimeDiff(uint32_t time1, uint32_t time2);
bool Timer_IsApproximate(uint32_t time1, uint32_t time2);





#endif /* SOURCE_MID_TIMER_TIMER_H_ */
