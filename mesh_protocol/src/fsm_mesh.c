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
#include <zephyr/logging/log.h>
#include "../include/model_handler.h"

LOG_MODULE_REGISTER(fsm_mesh);
static int flag_provision = 0;

#define TIMEOUT_MS (5 * 1000) //5segundos

struct k_timer timer_mesh;
static int flag_send_mesh = 0;

struct m_fsm_t{
	fsm_t fsm;
    fsm_t* fsm_p;
};
typedef struct m_fsm_t m_fsm_t;
void setTimerFlag_mesh(struct k_timer *my_timer)
{
        flag_send_mesh = 1;
       
}

/* List of check functions */
/* static int check_configured(fsm_t* this){
    
	return bt_mesh_is_provisioned();
    
	
 }*/
 static int check_mesh(fsm_t* this)
{   
    m_fsm_t* m_fsm = (m_fsm_t*) this;
    int f_mesh = get_flag_mesh(m_fsm->fsm_p);
   // LOG_INF("Flag Mesh: %d\n", f_mesh);
    return f_mesh;
}
 static int check_reading_mesh(fsm_t* this){
	m_fsm_t* m_fsm = (m_fsm_t*) this;
    int f_mesh = get_flag_mesh(m_fsm->fsm_p);
    LOG_INF("State : CHECK TIMER flag send mesh: %d", flag_send_mesh); 
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
	LOG_INF("INICIALIZANDO TIMER MESH\n");
    k_timer_init(&timer_mesh, setTimerFlag_mesh, NULL);
    k_timer_start(&timer_mesh, K_MSEC(TIMEOUT_MS), K_MSEC(TIMEOUT_MS));
    
}


static void send_data_mesh(fsm_t* this)
{
        flag_send_mesh = 0;
        send_data();
        LOG_INF("SENDING DATA MESH");

		
}
static void stop_mesh(fsm_t* this){
    k_timer_stop(&timer_mesh);
	flag_send_mesh = 0;
	LOG_INF("Disableling mesh\n");
}
static enum STATE{
        //CONF,
		OFF,
        READ_MESH
};

fsm_t* fsm_new_fsm_mesh(fsm_t* fsm_p){
        static fsm_trans_t tt[] = {
                //{CONF, check_configured, OFF, NULL},
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

