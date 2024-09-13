
#ifndef COAP_CLIENT_H
#define COAP_CLIENT_H

#include "coap_client_utils.h"
#include "coap_server_client_interface.h"
#include <openthread/thread.h>
#include <openthread/instance.h>
/*
static hts221_t m_hts;
nrfx_twi_t m_twi;

static void twi_init(void);
static nrfx_err_t hts_init(void);
*/
int get_data(void);
bool thread_is_provisioned(otInstance *instance);

#endif