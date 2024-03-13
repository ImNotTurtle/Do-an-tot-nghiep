/*
 * Receive.c
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */
#include "Receive.h"

void RECEIVE_HandleOnOffCluster(EmberAfClusterCommand* cmd){
	switch(cmd->commandId){
	case ZCL_ON_COMMAND_ID:
//		LedControl_On(LED_1, RED);
		break;
	case ZCL_OFF_COMMAND_ID:
//		LedControl_Off(LED_1);
		break;
	default:
		break;
	}
}

boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* im){
	return false;
}

bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd){

	return false;
}
