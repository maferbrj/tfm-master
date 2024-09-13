/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"



//--------para latencia--------
#include <zephyr/drivers/gpio.h>
static const struct gpio_dt_spec *gpio0_3;
//-----------------------------


//esta función es la que recibe los datos del sensor, SIN haber hecho un GET
static void sensor_cli_data_cb(struct bt_mesh_sensor_cli *cli,
			       struct bt_mesh_msg_ctx *ctx,
			       const struct bt_mesh_sensor_type *sensor,
			       const struct sensor_value *value)
{
	//-----para latencia-------
	gpio_pin_set(gpio0_3->port,gpio0_3->pin,1);
	//-----------------------

	if (sensor->id == bt_mesh_sensor_present_indoor_amb_temp.id) {
		if(value->val1 == 1){
			gpio_pin_set(gpio0_3->port,gpio0_3->pin,1);
		}
		printk("temperature: %s\n", bt_mesh_sensor_ch_str(value));
	}else if (sensor->id == bt_mesh_sensor_present_indoor_relative_humidity.id) {
		printk("Humidity: %s\n", bt_mesh_sensor_ch_str(value));
		if(value->val1 == 1){
			gpio_pin_set(gpio0_3->port,gpio0_3->pin,1);
		}
	}
	//-----para latencia-------
	gpio_pin_set(gpio0_3->port,gpio0_3->pin,0);
	//-----------------------

}


static const struct bt_mesh_sensor_cli_handlers bt_mesh_sensor_cli_handlers = {
	.data = sensor_cli_data_cb,
};

static struct bt_mesh_sensor_cli sensor_cli =
	BT_MESH_SENSOR_CLI_INIT(&bt_mesh_sensor_cli_handlers);


static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1,
		     BT_MESH_MODEL_LIST(BT_MESH_MODEL_CFG_SRV,
								BT_MESH_MODEL_SENSOR_CLI(&sensor_cli)),
		     					BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(const struct gpio_dt_spec *ld)
{
	//-----para latencia--------
	gpio0_3 = ld;
	//--------------------------
	return &comp;
}
