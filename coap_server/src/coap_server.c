/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <zephyr/drivers/gpio.h>
#include "ot_coap_utils.h"
#include <zephyr/device.h>

//------------para latencia.--------
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
//----------------------------------

LOG_MODULE_REGISTER(coap_server, CONFIG_COAP_SERVER_LOG_LEVEL);


static struct k_work provisioning_work;

static struct k_timer provisioning_timer;



static void activate_provisioning(struct k_work *item)
{
	ARG_UNUSED(item);

	ot_coap_activate_provisioning();

	printk("Provisioning activated\n");
}

static void deactivate_provisionig(void)
{

	k_timer_stop(&provisioning_timer);

	if (ot_coap_is_provisioning_active()) {
		ot_coap_deactivate_provisioning();
		printk("INFO:Provisioning deactivated\n");
	}
}

static void on_provisioning_timer_expiry(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);

	deactivate_provisionig();
}





static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context,
				    void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
		}
	}
}
static struct openthread_state_changed_cb ot_state_chaged_cb = { .state_changed_cb =
									 on_thread_state_changed };


int main(void)
{
	int ret;

	printk("----------Start CoAP-server sample-----------\n");


  	//------------para latencia.--------
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set(led.port,led.pin,1);
	//----------------------------------

	k_timer_init(&provisioning_timer, on_provisioning_timer_expiry, NULL);

	k_work_init(&provisioning_work, activate_provisioning);

	ret = ot_coap_init(&deactivate_provisionig,&led);
	
	if (ret) {
		printk("ERROR:Could not initialize OpenThread CoAP\n");
		goto end;
	}

	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_chaged_cb);
	openthread_start(openthread_get_default_context());

end:
	return 0;
}


