/*
 * Utils.c
 *
 *  Created on: Mar 16, 2024
 *      Author: Administrator
 */
#include "Utils.h"


uint32_t Timer_CalcTimeDiff(uint32_t time1, uint32_t time2){
	if(time2 >= time1) return time2 - time1;
	else return 0xFFFFFFFFU - time2 + time1;
}
