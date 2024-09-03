/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/pm/device.h>
#include <zephyr/net/coap.h>
#include "../include/coap_client_utils.h"
#include <time.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/dt-bindings/clock/ast10x0_clock.h>
#include <zephyr/drivers/gpio.h>
#include "../include/coap_server_client_interface.h"
#include "../include/coap_client.h"


/*
#include "nrfx_twi.h"
#include "../include/hts221.h"
#include <zephyr/drivers/i2c.h>
*/


#define ID_SENSOR 4 //cambiar para cada sensor
#define	PERIOD_SEND_MS	5000
 
#define LED0_NODE DT_ALIAS(led0)

//#define I2C0_NODE DT_NODELABEL(mysensor)
//static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
/*
static hts221_t m_hts;
nrfx_twi_t m_twi;

static void twi_init(void);
static nrfx_err_t hts_init(void);
*/
//DIRECCIÓN MULTICAST
static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),

	
	.sin6_addr.s6_addr = { 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
	.sin6_scope_id = 0U
};

//DIRECCIÓN UNICAST, VA CAMBIANDO MLEID
/*static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),

	//fe80::215:5dff:fee2:25a5

	.sin6_addr.s6_addr = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x02, 0x15, 0x5d, 0xff, 0xfe, 0xe2, 0x25, 0xa5 },
	.sin6_scope_id = 0U
};*/

static const char *const sensor_option[] = { SENSOR_URI_PATH, NULL };

//-----------------------

//para poder sacar LOGS
LOG_MODULE_REGISTER(coap_client);

#define OT_CONNECTION_LED DK_LED1
#define BLE_CONNECTION_LED DK_LED2
#define MTD_SED_LED DK_LED3

/*static void twi_init(void)
{

	const nrfx_twi_config_t twi_config = {
		.scl = nrf_twi_scl_pin_get(&dev_i2c),
		.sda =  nrf_twi_sda_pin_get(&dev_i2c),
		.frequency = NRF_TWI_FREQ_100K,
		.interrupt_priority = NRFX_TWI_DEFAULT_CONFIG_IRQ_PRIORITY,
		.hold_bus_uninit = false
	};
	// If a twi handler is not provided, the driver setup in blocking mode
	(nrfx_twi_init(&m_twi, &twi_config, NULL, NULL));
	nrfx_twi_enable(&m_twi);
	LOG_INF("TWI enable");
}
void get_data(uint8_t *buffer){
	hts221_value_t humd;
	hts221_value_t temp;
	(hts221_read(&m_hts, &temp, &humd));
	
		
    memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer),"ID:%d,t:%d,h:%d,p:%d\n",ID_SENSOR, temp, humd);

	

}

static nrfx_err_t hts_init(void)
{
	hts221_conf_t hts_conf = {
		.frec     = HTS221_FREC_ONE_SHOT,
		.avtemp   = HTS221_AVTEMP_128,
		.avhumd   = HTS221_AVHUMD_128,
		.cb       = NULL,
		.drdy_pin = PIN_DRDY_HTS221,
		.timeout  = 0,
	};

	(hts221_init(&m_hts, &m_twi, &hts_conf));
	LOG_INF("HTS init");
}
*/

/** @brief Checks if ot node is provisioned by checking the role
 */
bool thread_is_provisioned(otInstance *instance){
	otDeviceRole role = otThreadGetDeviceRole(instance);
	LOG_INF("ROL:%d", role);
	return (role == OT_DEVICE_ROLE_CHILD ||
            role == OT_DEVICE_ROLE_ROUTER ||
            role == OT_DEVICE_ROLE_LEADER);
			
}

int get_temp(void){

	int temp[5] = {17, 18, 19, 20, 21};
	return temp[rand() % 5];
}

int get_hum(void){
	int hum[5] = {55, 56, 57, 58, 59};
	return hum[rand() % 5];
}
int get_data(void){
	int temp;
	int hum;
	char buffer[50];
	int err;
	if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
		power_down_unused_ram();
	}
	temp = get_temp();
	hum = get_hum();
	memset(buffer, 0, sizeof(buffer));
	
	snprintf(buffer, sizeof(buffer),"ID:%d,t:%d,h:%d\n",ID_SENSOR, temp, hum);
	LOG_INF(" %s\n",buffer);
	//medir latencia
	//gpio_pin_set(led.port,led.pin,1);
	//funcion que envia datos
	err = coap_send_request(COAP_METHOD_PUT,
			  (const struct sockaddr *)&send_local_addr,
			  sensor_option, buffer,sizeof(buffer), NULL);
			  LOG_INF("Error: %d", err);
	//gpio_pin_set(led.port,led.pin,0);
	//k_sleep(K_MSEC(PERIOD_SEND_MS));

	return 0;
}


