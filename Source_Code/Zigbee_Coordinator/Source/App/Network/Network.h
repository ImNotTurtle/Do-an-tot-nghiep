/*
 * Network.h
 *
 *  Created on: Feb 20, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_APP_NETWORK_NETWORK_H_
#define SOURCE_APP_NETWORK_NETWORK_H_
#include "app/framework/include/af.h"

#define NETWORK_CREATOR_PANID				0xABCD
#define NETWORK_CREATOR_RADIO_TX_POWER		10
#define NETWORK_CREATOR_CHANNEL				15


typedef enum{
	NETWORK_OPENING,
	NETWORK_CLOSING,
	NETWORK_CREATING,
}NETWORK_STATE_e;



typedef void (*networkHandle)(NETWORK_STATE_e);


void Network_Init(networkHandle);
void Network_OpenNetwork(void);
void Network_CloseNetwork(void);
void Network_CreateNetwork(void);




#endif /* SOURCE_APP_NETWORK_NETWORK_H_ */
