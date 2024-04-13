/*
 * Send.h
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */

#ifndef SOURCE_APP_SEND_SEND_H_
#define SOURCE_APP_SEND_SEND_H_

#include "app/framework/include/af.h"

#define SOURCE_ENDPOINT_PRIMARY				1
#define DESTINATION_ENDPOINT				1
#define HC_NETWORK_ADDRESS					0x0000

void SEND_SendCommandUnicast(uint8_t srcEP, uint8_t destEP, uint32_t nwkAddress);
void SEND_ReportInfoToHC();
void SEND_ZigDevRequest();

void SEND_OnOffStateReport(uint8_t srcEP, uint8_t state);
void SEND_LevelStateReport(uint8_t srcEP, uint8_t value);
void SEND_PIRStateReport(uint8_t srcEP, uint8_t value);
#endif /* SOURCE_APP_SEND_SEND_H_ */
