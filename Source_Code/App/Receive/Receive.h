/*
 * Receive.h
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */

#ifndef SOURCE_APP_RECEIVE_RECEIVE_H_
#define SOURCE_APP_RECEIVE_RECEIVE_H_

#include "app/framework/include/af.h"

#include <stdbool.h>

void RECEIVE_HandleOnOffCluster(EmberAfClusterCommand* cmd);

#endif /* SOURCE_APP_RECEIVE_RECEIVE_H_ */
