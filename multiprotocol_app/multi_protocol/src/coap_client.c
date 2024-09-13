/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/pm/device.h>
#include <zephyr/net/coap.h>
#include "../include/coap_client_utils.h"
#include <time.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/dt-bindings/clock/ast10x0_clock.h>
#include <zephyr/drivers/gpio.h>
#include "../include/coap_server_client_interface.h"
#include "../include/coap_client.h"


#define ID_SENSOR 1 


/*DIRECCIÓN MULTICAST
static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),

	
	.sin6_addr.s6_addr = { 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
	.sin6_scope_id = 0U
};
*/
//DIRECCIÓN UNICAST
static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),
// example: fe80:0:0:0:4d8:2aa0:fb9e:fe37

	

	.sin6_addr.s6_addr = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x04, 0xd8, 0x2a, 0xa0, 0xfb, 0x9e, 0xfe, 0x37},
	.sin6_scope_id = 0U
};

static const char *const sensor_option[] = { SENSOR_URI_PATH, NULL };

LOG_MODULE_REGISTER(coap_client);

#define OT_CONNECTION_LED DK_LED1
#define BLE_CONNECTION_LED DK_LED2
#define MTD_SED_LED DK_LED3


/** @brief Checks if ot node is provisioned by checking the role
 */
bool thread_is_provisioned(otInstance *instance){
	otDeviceRole role = otThreadGetDeviceRole(instance);
	LOG_INF("ROL:%d", role);
	return (role == OT_DEVICE_ROLE_CHILD ||
            role == OT_DEVICE_ROLE_ROUTER ||
            role == OT_DEVICE_ROLE_LEADER);
			
}

int get_temp(void){

	int temp[5] = {17, 18, 19, 20, 21};
	return temp[rand() % 5];
}

int get_hum(void){
	int hum[5] = {55, 56, 57, 58, 59};
	return hum[rand() % 5];
}
int get_data(void){
	int temp;
	int hum;
	char buffer[50];
	int err;
	if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
		power_down_unused_ram();
	}
	temp = get_temp();
	hum = get_hum();
	memset(buffer, 0, sizeof(buffer));
	
	snprintf(buffer, sizeof(buffer),"ID:%d,t:%d,h:%d\n",ID_SENSOR, temp, hum);
	LOG_INF(" %s\n",buffer);

	err = coap_send_request(COAP_METHOD_PUT,
			  (const struct sockaddr *)&send_local_addr,
			  sensor_option, buffer,sizeof(buffer), NULL);
			  LOG_INF("Error: %d", err);


	return 0;
}


