#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>

#include "../include/fsm.h"
#include "../include/fsm_protocol_controller.h"
#include "../include/fsm_thread.h"

#include <dk_buttons_and_leds.h>
static int flag_comission = 1;

#define TIMEOUT_MS (5 * 1000) //5segundos

struct k_timer timer_thread;
static int flag_send_thread = 0;
struct t_fsm_t{
	fsm_t fsm;
    fsm_t* fsm_p;
};
typedef struct t_fsm_t t_fsm_t;

void setTimerFlag_thread(struct k_timer *my_timer)
{
        flag_send_thread = 1;
        
}
 /*Check functions*/
static int check_commissioned(fsm_t* this)
{ 
    return flag_comission;
}
static int check_thread(fsm_t* this)
{   
    t_fsm_t* t_fsm = (t_fsm_t*) this;
    int f_thread = get_flag_thread(t_fsm->fsm_p);
    //printk("Flag Thread: %d\n", f_thread);
    return f_thread;
}

static int check_reading_thread(fsm_t* this)
{
    t_fsm_t* t_fsm = (t_fsm_t*) this;
    return (get_flag_thread(t_fsm->fsm_p) && flag_send_thread);  
}
static int check_no_thread(fsm_t* this){
    t_fsm_t* t_fsm = (t_fsm_t*) this;
    return (!get_flag_thread(t_fsm->fsm_p));
}

/*Action functions*/
static void init_thread(fsm_t* this)
{   
	printk("State : OFF THREAD\n");
    k_timer_init(&timer_thread, setTimerFlag_thread, NULL);
    k_timer_start(&timer_thread, K_MSEC(TIMEOUT_MS), K_MSEC(TIMEOUT_MS));   
}

static void send_data_thread(fsm_t* this){
    flag_send_thread = 0;
    int temp = 14;
    printk("State: READ THREAD, TEMP: %d\n", temp);
    
    
}
static void stop_thread(fsm_t* this){
    k_timer_stop(&timer_thread);
    flag_send_thread = 0;
    printk("Disableling thread\n");
}

static enum STATE{
        CONF,
        OFF,
        READ_THREAD
};

fsm_t* fsm_new_fsm_thread(fsm_t* fsm_p){
        static fsm_trans_t tt[] = {
                {CONF, check_commissioned, OFF, NULL},
                {OFF, check_thread, READ_THREAD, init_thread},
                {READ_THREAD, check_reading_thread, READ_THREAD, send_data_thread},
                {READ_THREAD, check_no_thread, OFF, stop_thread},
                {-1, NULL, -1, NULL},
        };
t_fsm_t* lt_fsm = (t_fsm_t*) malloc (sizeof (t_fsm_t));
fsm_init ((fsm_t*) lt_fsm, tt);
lt_fsm->fsm_p = fsm_p;
return (fsm_t*)lt_fsm;
}