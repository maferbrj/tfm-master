/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/gpio.h>
#ifdef __cplusplus
extern "C" {
#endif

//const struct bt_mesh_comp *model_handler_init(void);
const struct bt_mesh_comp *model_handler_init(const struct gpio_dt_spec *ld);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
