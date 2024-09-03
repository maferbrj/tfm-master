/* main.c - Application main entry point */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/printk.h>

#include <zephyr/settings/settings.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/bluetooth/bluetooth.h>
///#include <zephyr/bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/net/openthread.h>
#include<bluetooth/mesh/sensor_types.h>
#include "../include/fsm_protocol_controller.h"
#include "../include/fsm_mesh.h"
#include "../include/fsm_thread.h"
#include "../include/coap_client_utils.h"
#include "../include/coap_client.h"
#include <openthread/platform/radio.h>
#include <zephyr/kernel.h>
//#include "../include/model_handler.h"

#define PERIOD_MS 500 //
//otInstance *instancia;
//extern struct openthread_context *contexto;

void delay_until (uint32_t* p_due_date, uint32_t period)
{
	uint32_t now = k_uptime_get();
	*p_due_date += period; //sumamos el periodo (5ms) al instante incial
	uint32_t slack = *p_due_date - now; //calculamos cuanto falta para que pasen los 5ms
	if (slack <= period) { //si todavia falta tiempo para que cumplan los 5ms, esperamos ese tiempo
		k_msleep(slack);
	}
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");


	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}


int main(void)
{

	int err = -1;
	
	printk("Initializing...\n");

	
	err = bt_enable(bt_ready);
	//bt_mesh_suspend(); 
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	
	
	//-----para latencia-------
	//gpio_pin_configure_dt(&gpio0_3,GPIO_OUTPUT_ACTIVE);
	//gpio_pin_set(gpio0_3.port,gpio0_3.pin,0);
	//--------------------------			
	
	
	
	fsm_t* fsm_protocol = fsm_new_fsm_protocol_controller(60000);
	fsm_t* fsm_mesh = fsm_new_fsm_mesh(fsm_protocol);
	fsm_t* fsm_thread  = fsm_new_fsm_thread(fsm_protocol);
	
    while (1) {
        // Aquí puedes agregar otras tareas o poner el sistema en modo de ahorro de energía
        //k_sleep(K_FOREVER); // Pone el sistema a dormir hasta que se active otro evento
		uint32_t now = k_uptime_get();
		fsm_fire(fsm_protocol);
		fsm_fire(fsm_mesh);
		fsm_fire(fsm_thread);
		
		
		delay_until(&now, PERIOD_MS);
		
    }
	return 0;
}
