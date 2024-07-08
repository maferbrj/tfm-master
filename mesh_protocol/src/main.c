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
#include <zephyr/bluetooth/mesh.h>

#include<bluetooth/mesh/sensor_types.h>
#include "../include/fsm_protocol_controller.h"
#include "../include/fsm_mesh.h"
#include "../include/fsm_thread.h"

//#include "fsm.h"
//por nosotras
#include <zephyr/kernel.h>
#define PERIOD_MS 1000 //

/*void setTimeFlag(struct k_timer *dummy);
K_TIMER_DEFINE(my_timer, setTimeFlag, NULL);



//added
#define TIMEOUT_MS (5 * 1000) // 2 minutos en milisegundos
void setTimeFlag(struct k_timer *dummy)
{
    printk("Timer expired and setTimeFlag called!\n");
	if(bt_mesh_is_provisioned()){
		printk("Provisionado\n");
		return;
	}
	printk("No provisionado\n");
    // Aquí puedes agregar el código que desees ejecutar cada vez que el temporizador expire
}*/


void delay_until (uint32_t* p_due_date, uint32_t period)
{
	uint32_t now = k_uptime_get();
	*p_due_date += period; //sumamos el periodo (5ms) al instante incial
	uint32_t slack = *p_due_date - now; //calculamos cuanto falta para que pasen los 5ms
	if (slack <= period) { //si todavia falta tiempo para que cumplan los 5ms, esperamos ese tiempo
		k_msleep(slack);
	}
}





//static struct bt_mesh_model models[] = {
//		     BT_MESH_MODEL_LIST(BT_MESH_MODEL_SENSOR_SRV(&chip_temp_sensor_srv))
//};

static int gen_onoff_get(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	//onoff_status_send(model, ctx);
	return 11;
}

static int gen_onoff_set(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	//(void)gen_onoff_set_unack(model, ctx, buf);
	//onoff_status_send(model, ctx);

	return 0;
}

#define OP_ONOFF_GET       BT_MESH_MODEL_OP_2(0x82, 0x01)
#define OP_ONOFF_SET       BT_MESH_MODEL_OP_2(0x82, 0x02)
static const struct bt_mesh_model_op sensor_srv_op[] = {
        { OP_ONOFF_GET,       BT_MESH_LEN_EXACT(0), gen_onoff_get },
        { OP_ONOFF_SET,       BT_MESH_LEN_MIN(2),   gen_onoff_set },
        BT_MESH_MODEL_OP_END,
};
struct bt_mesh_model models[] ={
        BT_MESH_MODEL(BT_MESH_MODEL_ID_SENSOR_SRV, sensor_srv_op, NULL,
                      NULL),        
};
static struct bt_mesh_elem elements[] = {
        BT_MESH_ELEM(0,models, BT_MESH_MODEL_NONE),
};
static const struct bt_mesh_comp comp = {
        .cid = BT_COMP_ID_LF,
        .elem = elements,
        .elem_count = ARRAY_SIZE(elements),
};





/* Provisioning */
//no borrar
static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
	printk("OOB Number: %u\n", number);

	return 0;
}

static void prov_complete(uint16_t net_idx, uint16_t addr)
{
	//
	printk("Provisioning completed\n");
}

static void prov_reset(void)
{
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

static uint8_t dev_uuid[16];

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 4,
	.output_actions = BT_MESH_DISPLAY_NUMBER,
	.output_number = output_number,
	.complete = prov_complete,
	.reset = prov_reset,
};

/** Send an OnOff Set message from the Generic OnOff Client to all nodes. */

static void button_pressed(struct k_work *work)
{
	
	if (bt_mesh_is_provisioned()) {
		(void)gen_onoff_send(0);
		return;
	}

}

static void bt_ready(int err)
{
	
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	//creo que por settings load es que una vez provisionado no se desprovisiona más
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

int main(void)
{

	/*por defecto vamos a hacer que tenga bluetooth enable, en un ppio
	* luego lo hará la maquina de estados protocol_FSM con una flag bla bla
	*  SE SIGUEN LOS SIGUIENTES PASOS:
	*/

	//Este paso no le entiendo bn pero lo dejo por si acaso, luego lo quito
	int err = -1;

	printk("Initializing...\n");

	if (IS_ENABLED(CONFIG_HWINFO)) {
		err = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	}

	if (err < 0) {
		dev_uuid[0] = 0xdd;
		dev_uuid[1] = 0xdd;
	}
	//Paso 1: iniciar BT
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	fsm_t* fsm_protocol = fsm_new_fsm_protocol_controller(15000);
	fsm_t* fsm_mesh = fsm_new_fsm_mesh(fsm_protocol);
	fsm_t* fsm_thread  = fsm_new_fsm_thread(fsm_protocol);
	
    while (1) {
        // Aquí puedes agregar otras tareas o poner el sistema en modo de ahorro de energía
        //k_sleep(K_FOREVER); // Pone el sistema a dormir hasta que se active otro evento
		//con esta manera ya funciona la fsm, debemos añadir que se habilite mesh ya que no se puede provisionar aun
		uint32_t now = k_uptime_get();
		fsm_fire(fsm_protocol);
		fsm_fire(fsm_mesh);
		fsm_fire(fsm_thread);
		delay_until(&now, PERIOD_MS);
		
    }
	return 0;
}
