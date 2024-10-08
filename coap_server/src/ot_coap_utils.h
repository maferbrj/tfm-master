/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __OT_COAP_UTILS_H__
#define __OT_COAP_UTILS_H__

#include <coap_server_client_interface.h>
#include <zephyr/drivers/gpio.h>
/**@brief Type definition of the function used to handle light resource change.
 */
typedef void (*sensor_request_callback_t)(uint8_t cmd);
typedef void (*provisioning_request_callback_t)();

/*
int ot_coap_init(provisioning_request_callback_t on_provisioning_request,
		 light_request_callback_t on_light_request);
*/
int ot_coap_init(provisioning_request_callback_t on_provisioning_request,const struct gpio_dt_spec *ld );

//int ot_coap_init(const struct gpio_dt_spec *ld);
void ot_coap_activate_provisioning(void);

void ot_coap_deactivate_provisioning(void);

bool ot_coap_is_provisioning_active(void);

#endif
