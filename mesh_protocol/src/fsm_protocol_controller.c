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
#include <bluetooth/mesh/dk_prov.h>

#include <zephyr/kernel.h>
#include "../include/fsm_protocol_controller.h"
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <openthread/coap.h>
#include <openthread/coap_secure.h>
#include "../include/coap_client.h"
#include "../include/model_handler.h"
#include "../include/main.h"
LOG_MODULE_REGISTER(fsm_protocol_controller);

//extern otInstance *instancia;

struct openthread_context *contexto;

struct p_fsm_t{
	fsm_t fsm;
    int flag_mesh;
    int flag_thread;
    uint32_t next;
    uint32_t Tprotocol;
};
typedef struct p_fsm_t p_fsm_t;

/*API */
int get_flag_mesh(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int ret = p_fsm->flag_mesh;
    return ret;
}
void set_flag_mesh(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    p_fsm->flag_mesh = 1;
 }
void clear_flag_mesh(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    p_fsm->flag_mesh = 0;
 }

int get_flag_thread(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int ret = p_fsm->flag_thread;
    return ret;
}
void set_flag_thread(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    p_fsm->flag_thread = 1;
 }
void clear_flag_thread(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    p_fsm->flag_thread = 0;
 }

//-----------------------------------------------------

/*List of check functions*/
static int always_check(fsm_t* this)
{
    return 1;
}
static int check_tim_protocol(fsm_t* this)
{   
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int tout = k_uptime_get() >= p_fsm->next;
    LOG_INF("TIMER PROTOCOL: %d MESH:%d THREAD:%d \n", tout, p_fsm->flag_mesh, p_fsm->flag_thread);
    return (tout);
}
static int check_tim_protocol_and_mesh(fsm_t* this)
{   
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int tout = k_uptime_get() >= p_fsm->next;
    LOG_INF("TIMER PROTOCOL %d MESH:%d THREAD:%d \n", tout, p_fsm->flag_mesh, p_fsm->flag_thread);
    return (tout && bt_mesh_is_provisioned());
}

static int check_tim_protocol_and_ot(fsm_t* this)
{   
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int tout = k_uptime_get() >= p_fsm->next;
    LOG_INF("TIMER PROTOCOL W THREAD: %d MESH:%d THREAD:%d \n", tout, p_fsm->flag_mesh, p_fsm->flag_thread);
    return (tout && thread_is_provisioned(contexto->instance));
}
static int check_not_thread_conf(fsm_t* this)
{
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int tout = k_uptime_get() >= p_fsm->next;
    LOG_INF("checking not thread prov");
    return (tout && !thread_is_provisioned(contexto->instance));
}

static int check_mesh_prov(fsm_t* this)
{
    int prov = bt_mesh_is_provisioned();
    LOG_INF("PROVISIONADO MESH: %d", prov);
    return prov;
}
static int check_not_mesh_conf(fsm_t* this)
{
    int prov = bt_mesh_is_provisioned();
    LOG_INF("PROVISIONADO MESH: %d", prov);
    return !prov;
}
static int check_thread_prov(fsm_t* this)
{
    LOG_INF ("PROVISIONADO THREAD: %d", thread_is_provisioned(contexto->instance));
    return thread_is_provisioned(contexto->instance);
}

/*List of action functions*/
static void configure_mesh(fsm_t* this)
{
	//int err = -1;
	LOG_INF("CONFIGURE MESH\n");
	//err = bt_enable(bt_ready);
	/*err = 1;
    if (err) {
		LOG_INF("Bluetooth init failed (err %d)\n", err);
	}*/
}

static void wait_prov_mesh(fsm_t* this)
{
        k_sleep(K_SECONDS(30));
}

static void configure_thread(fsm_t* this)
{
    int sus = bt_mesh_suspend();
    //bt_mesh_prov_disable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    clear_flag_mesh(this);
    clear_flag_thread(this);
   // LOG_INF("MESH SUSPENDIDO EN CONF THREAD: %d", sus);
    
    contexto = openthread_get_default_context();
	contexto->instance = coap_client_utils_init();
}
 static void init_multiprotocol(fsm_t* this)
 {
    p_fsm_t* p_fsm = (p_fsm_t*) this; 
    p_fsm->next = (p_fsm->next) + (p_fsm->Tprotocol);
    //set_flag_thread(this);
    //clear_flag_mesh(this);
    //LOG_INF("EMPIEZA MULTIPROTOCOLO EN THREAD");
    //bt_mesh_reset();

    bt_mesh_resume();

    set_flag_mesh(this);
    clear_flag_thread(this);
    LOG_INF("EMPIEZA MULTIPROTOCOLO EN MESH");

 }

static void enable_thread(fsm_t* this)
{
    p_fsm_t* p_fsm = (p_fsm_t*) this; 
    p_fsm->next = (p_fsm->next) + (p_fsm->Tprotocol);
    LOG_INF("Enable Thread\n");
    clear_flag_mesh(this);
    //int err = bt_mesh_suspend(); //durante un tiempo se suspende bt mesh
    //LOG_INF("mesh suspend: %d", err);
    k_msleep(1000);//esperamos un segundo entre disable mesh y enavle thread
    set_flag_thread(this);
    //openthread_start(contexto);
    
    
}

static void enable_mesh(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    p_fsm->next =(p_fsm->next) + (p_fsm->Tprotocol);
    int sus = -1;
    LOG_INF("Enable Mesh\n");
    clear_flag_thread(this);
    //disable thread
    //otInstanceFinalize(contexto->instance);
    //otError err = otThreadDetachGracefully(contexto->instance, NULL, contexto);
    //otError err = otThreadBecomeDetached(contexto->instance);
    LOG_INF("deshabilitando Thread");
    otError err = otThreadSetEnabled(contexto->instance, false);
    if (err != OT_ERROR_NONE) {
    LOG_INF("Error deteniendo Thread: %d", err);
    }
   
    k_msleep(100);
    //otCoapStop(contexto->instance);
    //otInstanceFinalize(contexto->instance);
   // k_msleep(100);
    LOG_INF("Habilitando Mesh");
//bt_mesh_reset();
    //k_msleep(100);
    //bt_enable(bt_ready);
    //bt_mesh_resume();
   // bt_mesh_reset();
    //k_msleep(2000);

    //bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
    //bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    bt_mesh_resume();
    k_msleep(1000);
    LOG_INF("COMPLETE MESH ENABLE()");
    //set_flag_mesh(this);
   LOG_INF("BT MESH RESUME: %d", sus);

}
static enum STATE{
      //  INIT, 
        CONF_BT,
        WAIT,
        CONF_OT,
        OT,
        MESH
};

fsm_t* fsm_new_fsm_protocol_controller(uint32_t protocolT){
        static fsm_trans_t tt[] = {
                //{INIT, always_check, CONF_BT, configure_mesh},
                //{CONF_BT, check_mesh_prov, CONF_OT, configure_thread}, ---- 
                {CONF_BT, check_mesh_prov, WAIT, wait_prov_mesh}, 
                {WAIT, always_check, MESH, init_multiprotocol},
                {MESH, check_tim_protocol, CONF_OT, configure_thread},
                {CONF_OT, check_thread_prov, OT, enable_thread}, //-------nuevo
                {OT, check_tim_protocol, MESH, enable_mesh},
                //{WAIT, always_check, CONF},
                //{OT, check_not_mesh_conf, configure_mesh},
                
                //{MESH, check_not_thread_conf, CONF_OT, NULL},
                //{OT, check_tim_protocol_and_mesh, MESH, enable_mesh},
                //{OT, check_not_mesh_conf, CONF_BT, NULL},
                //{CONF_OT, check_thread_prov, OT, init_multiprotocol}, -----esto antes
                //{OT, check_tim_protocol_and_mesh, MESH, enable_mesh},
                //{MESH, check_tim_protocol_and_ot, OT, enable_thread},
                
                {-1, NULL, -1, NULL},
        };
    p_fsm_t* lp_fsm = (p_fsm_t*) malloc (sizeof (p_fsm_t));
   fsm_init ((fsm_t*) lp_fsm, tt);
   lp_fsm->Tprotocol = protocolT;
   lp_fsm->next = k_uptime_get()+protocolT;
   clear_flag_mesh((fsm_t*) lp_fsm);
   clear_flag_thread((fsm_t*) lp_fsm);
   return (fsm_t*) lp_fsm;
}