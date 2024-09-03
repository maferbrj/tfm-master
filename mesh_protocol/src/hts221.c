#include "../include/hts221.h"

//#include "app_utils.h"

//#include "hal.h"

#define DRDY_PIN_CONFIG (                                     \
	(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) | \
	(GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     | \
	(GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos)   | \
	(GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  | \
	(GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos))

typedef struct hts221_calib_data_t {
	int16_t  x0;
	int16_t  x1;
	uint16_t y0;
	uint16_t y1;
} hts221_calib_data_t;

/******************************************************************************/
static nrfx_err_t twi_xfer_txtx(
	hts221_t* self,
	uint8_t reg,
	uint8_t value);
static nrfx_err_t twi_xfer_txrx(
	hts221_t* self,
	uint8_t reg,
	uint8_t* buffer);
static nrfx_err_t read_temp(hts221_t* self, hts221_value_t* temp);
static nrfx_err_t read_humd(hts221_t* self, hts221_value_t* humd);
static nrfx_err_t hts221_setup(hts221_t* self, hts221_conf_t* conf);
static nrfx_err_t hts221_clean(hts221_t* self);
static void gen_calib(hts221_calib_t* calib, hts221_calib_data_t* data);
static nrfx_err_t read_temp_calib(hts221_t* self, hts221_calib_data_t* data);
static nrfx_err_t read_humd_calib(hts221_t* self, hts221_calib_data_t* data);
static nrfx_err_t read_calib(hts221_t* self);
static hts221_value_t calc_value(int16_t value, hts221_calib_t* calib);

/******************************************************************************/

nrfx_err_t hts221_init(
	hts221_t* self,
	const nrfx_twi_t* twi,
	hts221_conf_t* conf)
{
	self->twi      = twi;
	self->cb       = conf->cb;
	self->drdy_pin = conf->drdy_pin;

	return hts221_setup(self, conf);

}

nrfx_err_t hts221_read(hts221_t* self, hts221_value_t* temp,
	hts221_value_t* humd)
{

	hts221_clean(self);
	(twi_xfer_txtx(self, HTS221_REG_CTRL2, HTS221_BIT_ONE_SHOT));


	uint8_t status = 0;
	while(status != (HTS221_BIT_T_DA | HTS221_BIT_H_DA)) {
		if((NRF_GPIO->IN & (1 << self->drdy_pin))) {
			(twi_xfer_txrx(self, HTS221_REG_STATUS, &status));
		}
	}

	read_temp(self, temp);
	read_humd(self, humd);

	return NRFX_SUCCESS;
}

static nrfx_err_t hts221_setup(hts221_t* self, hts221_conf_t* conf)
{

	uint8_t buffer = 0;

	(twi_xfer_txrx(self, HTS221_REG_WHO, &buffer));
	(buffer != HTS221_WHO);

	buffer = 0;
	buffer |= HTS221_BIT_BOOT;

	(twi_xfer_txtx(self, HTS221_REG_CTRL2, buffer));

	buffer = 0;
	buffer |= (conf->avtemp << HTS221_POS_AVGT);
	buffer |= (conf->avhumd << HTS221_POS_AVGH);

	(twi_xfer_txtx(self, HTS221_REG_CONF, buffer));

	if(conf->frec != HTS221_FREC_ONE_SHOT) {
		return NRFX_ERROR_NOT_SUPPORTED;
	}

	buffer = 0;
	buffer |= HTS221_BIT_PD;
	buffer |= (conf->frec << HTS221_POS_ODR);

	(twi_xfer_txtx(self, HTS221_REG_CTRL1, buffer));

	buffer = 0;
	buffer &= ~HTS221_BIT_DRDY_H_L;
	buffer &= ~HTS221_BIT_PP_OD;
	buffer |= HTS221_BIT_DRDY;

	(twi_xfer_txtx(self, HTS221_REG_CTRL3, buffer));

	NRF_GPIO->PIN_CNF[conf->drdy_pin] = DRDY_PIN_CONFIG;

	(read_calib(self));

	return NRFX_SUCCESS;
}

static nrfx_err_t twi_xfer_txtx(
	hts221_t* self,
	uint8_t reg,
	uint8_t value)
{
	const nrfx_twi_t* twi = self->twi;

	uint8_t buffer[2] = {reg, value};
	nrfx_twi_xfer_desc_t(desc) = {
		.type   = NRFX_TWI_XFER_TX,
		.address = HTS221_ADDR_7BITS,
		.primary_length = 2,
		.p_primary_buf = buffer,
		.secondary_length = 0,
		.p_secondary_buf = NULL
	};
	(nrfx_twi_xfer(twi, &desc, 0));
	return NRFX_SUCCESS;
}

static nrfx_err_t twi_xfer_txrx(
	hts221_t* self,
	uint8_t reg,
	uint8_t* buffer)
{
	const nrfx_twi_t* twi = self->twi;
	nrfx_twi_xfer_desc_t(desctx) = {
		.type   = NRFX_TWI_XFER_TX,
		.address = HTS221_ADDR_7BITS,
		.primary_length = 2,
		.p_primary_buf = &reg,
		.secondary_length = 0,
		.p_secondary_buf = NULL
	};
	nrfx_twi_xfer_desc_t(descrx) = {
		.type   = NRFX_TWI_XFER_RX,
		.address = HTS221_ADDR_7BITS,
		.primary_length = 1,
		.p_primary_buf = buffer,
		.secondary_length = 0,
		.p_secondary_buf = NULL
	};


	(nrfx_twi_xfer(twi, &desctx, NRFX_TWI_FLAG_TX_NO_STOP));
	(nrfx_twi_xfer(twi, &descrx, 0));

	return NRFX_SUCCESS;
}

static nrfx_err_t hts221_clean(hts221_t* self)
{
	if(NRF_GPIO->IN & (1 << self->drdy_pin)) {
		uint8_t buffer[2] = {0};
		(twi_xfer_txrx(self, HTS221_REG_HUM_L, buffer));
		(twi_xfer_txrx(self, HTS221_REG_HUM_H, buffer + 1));

		(twi_xfer_txrx(self, HTS221_REG_TEMP_L, buffer));
		(twi_xfer_txrx(self, HTS221_REG_TEMP_H, buffer + 1));
	}
	return NRFX_SUCCESS;
}

static nrfx_err_t read_temp(hts221_t* self, hts221_value_t* temp)
{
	uint8_t buffer[2] = {0};
	(twi_xfer_txrx(self, HTS221_REG_TEMP_L, buffer));
	(twi_xfer_txrx(self, HTS221_REG_TEMP_H, buffer + 1));

	int16_t t_out = *((int16_t*) buffer);

	*temp = calc_value(t_out, &(self->tCalib));

	return NRFX_SUCCESS;
}

static nrfx_err_t read_humd(hts221_t* self, hts221_value_t* humd)
{
	uint8_t buffer[2] = {0};
	(twi_xfer_txrx(self, HTS221_REG_HUM_L, buffer));
	(twi_xfer_txrx(self, HTS221_REG_HUM_H, buffer + 1));

	int16_t h_out = *((int16_t*) buffer);

	*humd = calc_value(h_out, &(self->hCalib));

	return NRFX_SUCCESS;
}

static void gen_calib(hts221_calib_t* calib, hts221_calib_data_t* data)
{
	int16_t y = data->y1 - data->y0;
	int16_t x = data->x1 - data->x0;

	calib->slope = ((float) y) / ((float) x);
	calib->slope /= 8.0;

	float inc = calib->slope * ((float)data->x0);

	calib->orig = ((float)data->y0) / 8.0 - inc;
}

static nrfx_err_t read_temp_calib(hts221_t* self, hts221_calib_data_t* data)
{
	uint8_t buffer;
	(twi_xfer_txrx(self, HTS221_REG_T0_DEG, &buffer));
	data->y0 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_T1_DEG, &buffer));
	data->y1 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_T_MSB, &buffer));
	uint8_t t0msb = (buffer & HTS221_MASK_T0_MSB) >> HTS221_POS_T0_MSB;
	uint8_t t1msb = (buffer & HTS221_MASK_T1_MSB) >> HTS221_POS_T1_MSB;
	data->y0 |= t0msb << 8;
	data->y1 |= t1msb << 8;

	(twi_xfer_txrx(self, HTS221_REG_T0_OUT_L, &buffer));
	data->x0 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_T0_OUT_H, &buffer));
	data->x0 |= buffer << 8;
	(twi_xfer_txrx(self, HTS221_REG_T1_OUT_L, &buffer));
	data->x1 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_T1_OUT_H, &buffer));
	data->x1 |= buffer << 8;

	return NRFX_SUCCESS;
}

static nrfx_err_t read_humd_calib(hts221_t* self, hts221_calib_data_t* data)
{
	uint8_t buffer;
	(twi_xfer_txrx(self, HTS221_REG_H0_RH, &buffer));
	data->y0 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_H1_RH, &buffer));
	data->y1 = buffer;

	(twi_xfer_txrx(self, HTS221_REG_H0_OUT_L, &buffer));
	data->x0 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_H0_OUT_H, &buffer));
	data->x0 |= buffer << 8;
	(twi_xfer_txrx(self, HTS221_REG_H1_OUT_L, &buffer));
	data->x1 = buffer;
	(twi_xfer_txrx(self, HTS221_REG_H1_OUT_H, &buffer));
	data->x1 |= buffer << 8;

	return NRFX_SUCCESS;
}

static nrfx_err_t read_calib(hts221_t* self)
{
	hts221_calib_data_t data = {0};

	(read_temp_calib(self, &data));
	gen_calib(&(self->tCalib), &data);
	(read_humd_calib(self, &data));
	gen_calib(&(self->hCalib), &data);

	return NRFX_SUCCESS;
}

static hts221_value_t calc_value(int16_t value, hts221_calib_t* calib)
{
	hts221_value_t out = ((float) value) * calib->slope;
	out += calib->orig;
	return out;
}
