/*
 * Send.h
 *
 *  Created on: Mar 2, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_APP_SEND_SEND_H_
#define SOURCE_APP_SEND_SEND_H_

#include "app/framework/include/af.h"

void Send_SendCommandUnicast(uint8_t srcEP, uint8_t destEP, EmberNodeId nodeId);
void Send_LevelControl(EmberNodeId nodeId, uint8_t srcEp, uint8_t destEp, uint8_t value);
void Send_SendViaBinding(uint8_t localEndpoint, uint8_t remoteEndpoint, bool value, uint16_t nodeID);


#endif /* SOURCE_APP_SEND_SEND_H_ */
