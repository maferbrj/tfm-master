/*  - Application main entry point */

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
#include "../include/fsm.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <zephyr/kernel.h>
#include "../include/fsm_mesh.h"
#include "../include/fsm_protocol_controller.h"

//vamos a simular el provisioned con un boton
#include <dk_buttons_and_leds.h>
static int flag_provision = 0;
//----

#define TIMEOUT_MS (5 * 1000) //5segundos

struct k_timer timer_mesh;
//static uint8_t dev_uuid[16];
static int flag_send_mesh = 0;
/*Callback functions*/

/*static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 4,
	.output_actions = BT_MESH_DISPLAY_NUMBER,
	.output_number = NULL,
	.complete = NULL,
	.reset = NULL,
};

static const struct bt_mesh_comp comp = {
	.cid = BT_COMP_ID_LF,
	.elem = NULL,
	.elem_count = 0,
};*/
struct m_fsm_t{
	fsm_t fsm;
    fsm_t* fsm_p;
};
typedef struct m_fsm_t m_fsm_t;
void setTimerFlag_mesh(struct k_timer *my_timer)
{
        flag_send_mesh = 1;
       
}


/*

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

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}
N
	// This will be a no-op if settings_load() loaded provisioning info 
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}
*/
/* Variables compartidas */


/* List of check functions */
 static int check_configured(fsm_t* this){
    //printk("State : CHECK CONF\n");
    return bt_mesh_is_provisioned();
	//return 1;
	/*
	dk_buttons_init(on_button_changed);
    return (flag_provision);
	*/
 }
 static int check_mesh(fsm_t* this)
{   
    m_fsm_t* m_fsm = (m_fsm_t*) this;
    int f_mesh = get_flag_mesh(m_fsm->fsm_p);
    //printk("Flag Mesh: %d\n", f_mesh);
    return f_mesh;
}
 static int check_reading_mesh(fsm_t* this){
	  m_fsm_t* m_fsm = (m_fsm_t*) this;
    int f_mesh = get_flag_mesh(m_fsm->fsm_p);
      //printk("State : CHECK TIMER\n"); 
      return (f_mesh && flag_send_mesh);
      
 }
static int check_no_mesh(fsm_t* this)
{
	m_fsm_t* m_fsm = (m_fsm_t*) this;
    return (!get_flag_mesh(m_fsm->fsm_p));
}

 /*Funciones de acción*/
static void init_mesh(fsm_t* this)
{       
	printk("State : OFF MESH\n");
    k_timer_init(&timer_mesh, setTimerFlag_mesh, NULL);
    k_timer_start(&timer_mesh, K_MSEC(TIMEOUT_MS), K_MSEC(TIMEOUT_MS));
    
}


static void send_data_mesh(fsm_t* this)
{
        flag_send_mesh = 0;
        int temp = 10;
//que se pueda llamar desde read_send
	/*
	struct bt_mesh_msg_ctx ctx = {
		.app_idx = models[3].keys[0], // Use the bound key 
		.addr = BT_MESH_ADDR_ALL_NODES,
		.send_ttl = BT_MESH_TTL_DEFAULT,
	};
uint8_t buf[2]={1,2};
	bt_mesh_model_send(&models[3], &ctx, &buf, NULL, NULL);
	*/	

        printk("State READ MESH, TEMP: %d\n", temp);
       /* 
	   (bt_ready(0));
        bt_enable(bt_ready);
        k_sleep(K_MSEC(100));
        bt_mesh_sensor_srv_pub(&sensor_srv, &temp);
		*/
		
}
static void stop_mesh(fsm_t* this){
    k_timer_stop(&timer_mesh);
	flag_send_mesh = 0;
	printk("Disableling mesh\n");
}
static enum STATE{
        CONF,
		OFF,
        READ_MESH
};

fsm_t* fsm_new_fsm_mesh(fsm_t* fsm_p){
        static fsm_trans_t tt[] = {
                {CONF, check_configured, OFF, NULL},
                {OFF, check_mesh, READ_MESH, init_mesh},
				{READ_MESH, check_reading_mesh, READ_MESH, send_data_mesh},
                {READ_MESH, check_no_mesh, OFF, stop_mesh},
				{-1, NULL, -1, NULL},
        };
m_fsm_t* lm_fsm = (m_fsm_t*) malloc (sizeof (m_fsm_t));
fsm_init ((fsm_t*) lm_fsm, tt);
lm_fsm->fsm_p = fsm_p;
return (fsm_t*)lm_fsm;
}


/* State CONF: acciones que hace cuando esta en el estado */

