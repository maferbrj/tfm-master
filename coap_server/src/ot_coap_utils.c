/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/openthread.h>
#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include "ot_coap_utils.h"

//--------para latencia--------
#include <zephyr/drivers/gpio.h>
static const struct gpio_dt_spec *led ;
//-----------------------------



//--------para retransmiisiones--------
static uint32_t receive_count = 0;
//-------------------------------------


//---dir unicast----

//------------------
LOG_MODULE_REGISTER(ot_coap_utils, CONFIG_OT_COAP_UTILS_LOG_LEVEL);

struct server_context {
	struct otInstance *ot;
	/*
	*/
	bool provisioning_enabled;
	provisioning_request_callback_t on_provisioning_request;
	sensor_request_callback_t on_sensor_info_request;
};


static struct server_context srv_context = {
	.ot = NULL,
	.provisioning_enabled = false,
	.on_sensor_info_request = NULL,
	/*
	*/
	.on_provisioning_request = NULL,
};


static otCoapResource provisioning_resource = {
	.mUriPath = PROVISIONING_URI_PATH,
	.mHandler = NULL,
	.mContext = NULL,
	.mNext = NULL,
};


static otCoapResource sensor_resource = {
	.mUriPath = SENSOR_URI_PATH,
	.mHandler = NULL,
	.mContext = NULL,
	.mNext = NULL,
};
/*
*/
static otError provisioning_response_send(otMessage *request_message,
					  const otMessageInfo *message_info)
{
	otError error = OT_ERROR_NO_BUFS;
	otMessage *response;
	const void *payload;
	uint16_t payload_size;
	response = otCoapNewMessage(srv_context.ot, NULL);
	if (response == NULL) {
		goto end;
	}

	otCoapMessageInit(response, OT_COAP_TYPE_NON_CONFIRMABLE,
			  OT_COAP_CODE_CONTENT);

	error = otCoapMessageSetToken(
		response, otCoapMessageGetToken(request_message),
		otCoapMessageGetTokenLength(request_message));
	if (error != OT_ERROR_NONE) {
		goto end;
	}

	error = otCoapMessageSetPayloadMarker(response);
	if (error != OT_ERROR_NONE) {
		goto end;
	}

	payload = otThreadGetMeshLocalEid(srv_context.ot);
	payload_size = sizeof(otIp6Address);

	error = otMessageAppend(response, payload, payload_size);
	if (error != OT_ERROR_NONE) {
		goto end;
	}

	error = otCoapSendResponse(srv_context.ot, response, message_info);

	LOG_HEXDUMP_INF(payload, payload_size, "Sent provisioning response:");

end:
	if (error != OT_ERROR_NONE && response != NULL) {
		otMessageFree(response);
	}

	return error;
}


//ATENCION: esta es la función que envia su dir unicast: ver si da tiempo
/*
*/
static void provisioning_request_handler(void *context, otMessage *message,
					 const otMessageInfo *message_info)
{
	otError error;
	otMessageInfo msg_info;

	ARG_UNUSED(context);

	if (!srv_context.provisioning_enabled) {
		printk("WARNING:Received provisioning request but provisioning is disabled\n");
		return;
	}

	printk("INFO:Received provisioning request");
	if ((otCoapMessageGetType(message) == OT_COAP_TYPE_NON_CONFIRMABLE) &&
	    (otCoapMessageGetCode(message) == OT_COAP_CODE_GET)) {
		msg_info = *message_info;
		memset(&msg_info.mSockAddr, 0, sizeof(msg_info.mSockAddr));

		error = provisioning_response_send(message, &msg_info);
		if (error == OT_ERROR_NONE) {
			srv_context.on_provisioning_request();
			srv_context.provisioning_enabled = false;
		}
	}

}

//RECEPCIÓN DE VALORES SENSOR
static void sensor_request_handler(void *context, otMessage *message,
				  const otMessageInfo *message_info)
{
	//uint8_t *command;
	char command[50];
	ARG_UNUSED(context);


	if (otCoapMessageGetType(message) != OT_COAP_TYPE_NON_CONFIRMABLE) {
		LOG_ERR("ERROR:Sensor handler - Unexpected type of message\n");
		goto end;
	}


	otMessageRead(message, otMessageGetOffset(message), command, sizeof(command));
	//--------para latencia---------
	if(command[3]=='1'){
		gpio_pin_set(led->port,led->pin,0);	
	}

	LOG_INF("Recived: %s\n", command);

	char buffer[34];
	const otIp6Address src= message_info->mPeerAddr;
	otIp6AddressToString(&src, buffer, sizeof(buffer));
	printk ("Dirección envio: %s\n",buffer);
	//-----prueba latencia----
	gpio_pin_set(led->port,led->pin,1);
	//------------------------

	srv_context.on_sensor_info_request(command);

end:
	return;
}

static void   coap_default_handler(void *context, otMessage *message,
				 const otMessageInfo *message_info)
{
	ARG_UNUSED(context);
	ARG_UNUSED(message);
	ARG_UNUSED(message_info);
	printk("INFO:Received CoAP message that does not match any request or resource\n");
}

/*
*/
void ot_coap_activate_provisioning(void)
{
	srv_context.provisioning_enabled = true;
}

void ot_coap_deactivate_provisioning(void)
{
	srv_context.provisioning_enabled = false;
}

bool ot_coap_is_provisioning_active(void)
{
	return srv_context.provisioning_enabled;
}

int ot_coap_init(provisioning_request_callback_t on_provisioning_request,const struct gpio_dt_spec *ld )
{
	otError error;
	led=ld;

	srv_context.provisioning_enabled = false;
	srv_context.on_provisioning_request = on_provisioning_request;
	//srv_context.on_sensor_info_request = sensor_info_request;

	srv_context.ot = openthread_get_default_instance();
	if (!srv_context.ot) {
		printk("ERROR:There is no valid OpenThread instance");
		error = OT_ERROR_FAILED;
		goto end;
	}


	provisioning_resource.mContext = srv_context.ot;
	provisioning_resource.mHandler = provisioning_request_handler;

	sensor_resource.mContext = srv_context.ot;
	sensor_resource.mHandler = sensor_request_handler ; //message_print_handler

	otCoapSetDefaultHandler(srv_context.ot, coap_default_handler, NULL);
	otCoapAddResource(srv_context.ot, &sensor_resource);
	otCoapAddResource(srv_context.ot, &provisioning_resource);

	error = otCoapStart(srv_context.ot, COAP_PORT);
	if (error != OT_ERROR_NONE) {
		printk("ERROR:Failed to start OT CoAP. Error: %d\n", error);
		goto end;
	}
end:
	return error == OT_ERROR_NONE ? 0 : 1;
}
