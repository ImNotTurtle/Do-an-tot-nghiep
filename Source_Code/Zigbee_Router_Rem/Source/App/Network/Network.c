/*
 * Network.c
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */
#include "app/framework/include/af.h"
#include "Network.h"

EmberEventControl joinNetworkEventControl;
uint32_t timeFindAndJoin = 0;

networkHandle networkHandlerCallback = NULL;

void NETWORK_Init(networkHandle cb){
	networkHandlerCallback = cb;
}

void NETWORK_FindAndJoin(){
	if(emberAfNetworkState() == EMBER_NO_NETWORK){
		timeFindAndJoin = 0;
		emberEventControlSetDelayMS(joinNetworkEventControl, 2000);
	}
}
void NETWORK_StopFindAndJoin(){
	if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
		emberEventControlSetDelayMS(joinNetworkEventControl, 1000);
	}
}
void joinNetworkEventHandler(){
	emberEventControlSetInactive(joinNetworkEventControl);
	EmberStatus status = emberAfNetworkState();
	if(status == EMBER_NO_NETWORK){
		emberAfPluginNetworkSteeringStart();
		timeFindAndJoin++;
		networkHandlerCallback(NETWORK_FINDING_NETWORK);
		if(timeFindAndJoin < NETWORK_STEERING_REPEAT_MAX_COUNT){
			emberEventControlSetDelayMS(joinNetworkEventControl, NETWORK_FIND_AND_JOIN_REPEAT_INTERVAL);
		}
	}
	else if(status == EMBER_JOINED_NETWORK){
		emberAfPluginNetworkSteeringStop();
		timeFindAndJoin = 0;
	}
}

bool emberAfStackStatusCallback(EmberStatus status){
	if(status == EMBER_NETWORK_UP){
		if(timeFindAndJoin > 0){//join success
			NETWORK_StopFindAndJoin();
			if(networkHandlerCallback != NULL){
				networkHandlerCallback(NETWORK_JOIN_SUCCESS);
			}
		}
		else{//has parent already and connected to parent success
			if(networkHandlerCallback != NULL){
				networkHandlerCallback(NETWORK_HAS_PARENT);
			}
		}
	}
	else{
		EmberNetworkStatus nwkCurrentStatus = emberAfNetworkState();
		if(nwkCurrentStatus == EMBER_NO_NETWORK){
			if(networkHandlerCallback != NULL){
				networkHandlerCallback(NETWORK_OUT_NETWORK);
			}
		}
		else if(nwkCurrentStatus == EMBER_JOIN_FAILED){
			if(networkHandlerCallback){
				networkHandlerCallback(NETWORK_JOIN_FAIL);
			}
		}
		else if(nwkCurrentStatus == EMBER_JOINED_NETWORK_NO_PARENT){
			if(networkHandlerCallback != NULL){
			    networkHandlerCallback(NETWORK_LOST_PARENT);
			}
		}
	}
	return false;
}
