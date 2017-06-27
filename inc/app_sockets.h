/*
 * app_sockets.h
 *
 *  Created on: 18 cze 2017
 *      Author: Rafal
 */

#ifndef APP_SOCKETS_H_
#define APP_SOCKETS_H_

#include "stm32f7xx_hal.h"

void udp_echo_socket(void *arg);
void tcp_echo_socket(void *arg);

#endif /* APP_SOCKETS_H_ */
