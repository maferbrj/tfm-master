#ifndef HTS221_H
#define HTS221_H

#include "hts221_defines.h"
#include <zephyr/kernel.h>
#include "nrfx_config.h"
#include "nrfx_twi.h"
#include <drivers/include/nrfx_twi.h>
typedef struct hts221_t hts221_t;
typedef float hts221_value_t;

typedef void(* hts221_cb_t)(
	hts221_t* self, hts221_value_t temp, hts221_value_t humd);

typedef struct hts221_conf_t {
	hts221_conf_frec_t   frec;
	hts221_conf_avtemp_t avtemp;
	hts221_conf_avhumd_t avhumd;
	hts221_cb_t          cb;
	uint8_t              drdy_pin;
	uint16_t             timeout;
} hts221_conf_t;

typedef struct hts221_calib_t {
	float slope;
	float orig;
} hts221_calib_t;

struct hts221_t {
	const nrfx_twi_t* twi;
	hts221_cb_t       cb;
	uint8_t           drdy_pin;
	hts221_calib_t    tCalib;
	hts221_calib_t    hCalib;
};

nrfx_err_t hts221_init(
	hts221_t* self,
	const nrfx_twi_t* twi,
	hts221_conf_t* conf);
nrfx_err_t hts221_read(
	hts221_t* self,
	hts221_value_t* temp,
	hts221_value_t* humd);



#endif // HTS221_H
