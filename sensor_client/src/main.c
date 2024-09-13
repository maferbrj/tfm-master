/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic mesh sensor observer sample
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"

//-----para latencia------
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led1)
//-------------------
static const struct gpio_dt_spec gpio0_3 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
//-------------------

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");


	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init(&gpio0_3)); //inicio del cli
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

int main(void)
{
	int err;

	printk("Initializing...\n");

	//-----para latencia------
	gpio_pin_configure_dt(&gpio0_3,GPIO_OUTPUT_ACTIVE);
	gpio_pin_set(gpio0_3.port,gpio0_3.pin,0);
	//-------------------

	err = bt_enable(bt_ready);

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	return 0;
}
