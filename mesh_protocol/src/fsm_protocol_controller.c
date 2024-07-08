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
#include "../include/fsm_protocol_controller.h"




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
    //------------------
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

/*List of check functions*/
static int check_tim_protocol(fsm_t* this)
{   
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    int tout = k_uptime_get() >= p_fsm->next;
    printk("Timeout: %d f_mesh:%d f_thread:%d \n", tout, p_fsm->flag_mesh, p_fsm->flag_thread);
    return (tout);
}

/*List of action functions*/
static void enable_thread(fsm_t* this)
{
    p_fsm_t* p_fsm = (p_fsm_t*) this; 
    printk("Enable Thread\n");
    set_flag_thread(this);
    clear_flag_mesh(this);
    p_fsm->next = (p_fsm->next) + (p_fsm->Tprotocol);
}
static void enable_mesh(fsm_t* this){
    p_fsm_t* p_fsm = (p_fsm_t*) this;
    printk("Enable Mesh\n");
    set_flag_mesh(this);
    clear_flag_thread(this);
    p_fsm->next =(p_fsm->next) + (p_fsm->Tprotocol);

}
static enum STATE{
        THREAD,
        MESH
};

fsm_t* fsm_new_fsm_protocol_controller(uint32_t protocolT){
        static fsm_trans_t tt[] = {
                {THREAD, check_tim_protocol, MESH, enable_mesh},
                {MESH, check_tim_protocol, THREAD, enable_thread},
                {-1, NULL, -1, NULL},
        };
    p_fsm_t* lp_fsm = (p_fsm_t*) malloc (sizeof (p_fsm_t));
   fsm_init ((fsm_t*) lp_fsm, tt);
   lp_fsm->Tprotocol = protocolT;
   lp_fsm->next = k_uptime_get()+protocolT;
   clear_flag_mesh((fsm_t*) lp_fsm);
   set_flag_thread((fsm_t*) lp_fsm);
   return (fsm_t*) lp_fsm;
}