/*
 * Network.h
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */

#ifndef SOURCE_APP_NETWORK_NETWORK_H_
#define SOURCE_APP_NETWORK_NETWORK_H_
#include "app/framework/include/af.h"

#define NETWORK_FIND_AND_JOIN_REPEAT_INTERVAL			5000
#define NETWORK_STEERING_REPEAT_MAX_COUNT				30

typedef enum{
	NETWORK_JOIN_SUCCESS,
	NETWORK_JOIN_FAIL,
	NETWORK_HAS_PARENT,
	NETWORK_OUT_NETWORK,
	NETWORK_LOST_PARENT,
	NETWORK_FINDING_NETWORK,
}NETWORK_STATE_e;

typedef void (*networkHandle)(NETWORK_STATE_e);

void NETWORK_Init(void (*cb)(NETWORK_STATE_e));
void NETWORK_FindAndJoin();
void NETWORK_StopFindAndJoin();


#endif /* SOURCE_APP_NETWORK_NETWORK_H_ */
