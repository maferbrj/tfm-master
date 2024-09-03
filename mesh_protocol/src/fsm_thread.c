#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/pm/device.h>
#include <zephyr/net/coap.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include "../include/fsm.h"
#include "../include/fsm_protocol_controller.h"
#include "../include/fsm_thread.h"
#include "../include/coap_client.h"
#include <zephyr/drivers/gpio.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(fsm_thread);
//extern otInstance *instancia;


//#define LED0_NODE DT_ALIAS(led0)
//static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


#define TIMEOUT_MS (5 * 1000) 
char buffer[50];

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
/*static int check_commissioned(fsm_t* this)
{ 
    
    LOG_INF("CHECK COMM");
    LOG_INF ("Provisioned node: %d", thread_is_provisioned(instancia));
    return thread_is_provisioned(instancia);

}*/
static int check_thread(fsm_t* this)
{   
    t_fsm_t* t_fsm = (t_fsm_t*) this;
    int f_thread = get_flag_thread(t_fsm->fsm_p);
    //LOG_INF("CHECK THREAD FLAG THREAD: %d", f_thread);
    return f_thread;
}

static int check_reading_thread(fsm_t* this)
{
    t_fsm_t* t_fsm = (t_fsm_t*) this;
    //LOG_INF("CHECK read THREAD FLAG: %d, %d", get_flag_thread(t_fsm->fsm_p), flag_send_thread);
    return (get_flag_thread(t_fsm->fsm_p) && flag_send_thread);  
}
static int check_no_thread(fsm_t* this){
    
    t_fsm_t* t_fsm = (t_fsm_t*) this;
    return (!get_flag_thread(t_fsm->fsm_p));
}

/*Action functions*/
static void init_thread(fsm_t* this)
{   
	LOG_INF("Inicializando TIMER THREAD");
    k_timer_init(&timer_thread, setTimerFlag_thread, NULL);
    k_timer_start(&timer_thread, K_MSEC(TIMEOUT_MS), K_MSEC(TIMEOUT_MS));   

   // twi_init();
	//hts_init();
	//coap_client_utils_init();
	//control de gpios para calculo latencia
	//gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);
	//gpio_pin_set(led.port,led.pin,0);
}

static void send_data_thread(fsm_t* this){
    flag_send_thread = 0;
    //int temp = 14;
    //k_sleep(K_MSEC(5000));
    LOG_INF("Sending data thread");
    get_data();
//printk(" %s\n",buffer);

//medir latencia
//gpio_pin_set(led.port,led.pin,1);
//funcion que envia datos
//coap_send_request(COAP_METHOD_PUT, (const struct sockaddr *)&multicast_local_addr,
//				  light_option, buffer,sizeof(buffer), NULL);
//gpio_pin_set(led.port,led.pin,0);
    //LOG_INF("State: READ THREAD, TEMP: %d\n", temp);
    
    
}
static void stop_thread(fsm_t* this){
    k_timer_stop(&timer_thread);
    flag_send_thread = 0;
    k_sleep(K_SECONDS(30)); //-------- wait mientras se deshabilita thread
    LOG_INF("Disableling thread\n");
}

static enum STATE{
        //CONF,
        OFF,
        READ_THREAD
};

fsm_t* fsm_new_fsm_thread(fsm_t* fsm_p){
        static fsm_trans_t tt[] = {
                //{CONF, check_commissioned, OFF, NULL},
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