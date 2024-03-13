/*
 * Network.c
 *
 *  Created on: Feb 20, 2024
 *      Author: Administrator
 */
#include "Network.h"


networkHandle networkHandlerCallback = NULL;

void Network_Init(networkHandle cb){
	networkHandlerCallback = cb;
}

void Network_OpenNetwork(void){
	emberAfPluginNetworkCreatorSecurityOpenNetwork();

	networkHandlerCallback(NETWORK_OPENING);
}
void Network_CloseNetwork(void){
	emberAfPluginNetworkCreatorSecurityCloseNetwork();

	networkHandlerCallback(NETWORK_CLOSING);
}
void Network_CreateNetwork(void){
	emberAfPluginNetworkCreatorStart(true);

	networkHandlerCallback(NETWORK_CREATING);
}
