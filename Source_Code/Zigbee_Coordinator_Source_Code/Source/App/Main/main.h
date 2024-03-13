
/*
 * main.h
 *
 *  Created on: Mar 9, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_APP_MAIN_MAIN_H_
#define SOURCE_APP_MAIN_MAIN_H_
#include "app/framework/include/af.h"

#include "Source/Driver/USART/USART.h"

#include "Source/Mid/Timer/Timer.h"
#include "Source/Mid/Led/Led.h"
#include "Source/Mid/Button/Button.h"

#include "Source/App/Send/Send.h"
#include "Source/App/Receive/Receive.h"
#include "Source/App/Network/Network.h"

#include "Source/Utils/DeviceManager/DeviceManager.h"


void Main_Init(void);
void Main_Usart1Handle(USART_FRAME);
void Main_ButtonPressHandle(uint8_t);
void Main_NetworkHandle(NETWORK_STATE_e);
void Main_ReceiveHandle(EmberNodeId nodeId, RECEIVE_CMD_ID_e receiveId, uint8_t length, uint8_t *modelId);
void Main_NetworkSync(void);


#endif /* SOURCE_APP_MAIN_MAIN_H_ */
