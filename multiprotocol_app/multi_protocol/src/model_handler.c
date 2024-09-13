/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/drivers/sensor.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/sensor_types.h>
#include <dk_buttons_and_leds.h>
#include <float.h>
#include "../include/model_handler.h"
#include <bluetooth/mesh/dk_prov.h>

#define SENSOR_ID 1
static struct bt_mesh_sensor temperature_indoor_sensor = {
	.type = &bt_mesh_sensor_present_indoor_amb_temp,
};
static struct bt_mesh_sensor *const temperature_sensor[] = {
	&temperature_indoor_sensor,
};
static struct bt_mesh_sensor_srv temperaturita_sensor_srv =
	BT_MESH_SENSOR_SRV_INIT(temperature_sensor, ARRAY_SIZE(temperature_sensor));



static struct bt_mesh_sensor humidity_indoor_sensor_level = {
	.type = &bt_mesh_sensor_present_indoor_relative_humidity,
};
static struct bt_mesh_sensor *const humidity_sensor[] = {
	&humidity_indoor_sensor_level,
};
static struct bt_mesh_sensor_srv humidity_sensor_srv =
	BT_MESH_SENSOR_SRV_INIT(humidity_sensor, ARRAY_SIZE(humidity_sensor));


static struct bt_mesh_elem elements[] = {
	//humidity
	BT_MESH_ELEM(1,
		     BT_MESH_MODEL_LIST(BT_MESH_MODEL_CFG_SRV,
								BT_MESH_MODEL_SENSOR_SRV(&humidity_sensor_srv)),
		     					BT_MESH_MODEL_NONE),
	//temperature
	BT_MESH_ELEM(2,
		     BT_MESH_MODEL_LIST(BT_MESH_MODEL_SENSOR_SRV(&temperaturita_sensor_srv)),
		     BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	return &comp;
}

struct sensor_value get_temperature(void){
	struct sensor_value temp;
	sensor_value_from_double(&temp, (20+SENSOR_ID));
	return temp;
}

struct sensor_value get_humidity(void){
	struct sensor_value hum;
	double rand_hum[5] = {0.5, 0.51, 0.52, 0.53, 0.54};
	int i = rand() % 5;
	sensor_value_from_double(&hum, rand_hum[i]);
	return hum;
}

int send_data(void){
	int err;
	struct sensor_value temp;
	struct sensor_value hum;

	//---latency
	//sensor_value_from_double(&temp, 1.0);
	//sensor_value_from_double(&hum, 1.0);
	
	//----------------------------
	temp = get_temperature();
	hum = get_humidity();
	//humidity
	err = bt_mesh_sensor_srv_pub(&humidity_sensor_srv, NULL,&humidity_indoor_sensor_level, &hum);
	if (err) {
		printk("Error publishing humidity(%d)\n", err);
	}
	
	//temperature
	err = bt_mesh_sensor_srv_pub(&temperaturita_sensor_srv, NULL, &temperature_indoor_sensor, &temp);
	if (err) {
		printk("Error publishing temperature(%d)\n", err);
	}
	return err;
}