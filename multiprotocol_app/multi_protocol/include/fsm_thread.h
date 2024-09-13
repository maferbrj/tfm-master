
#ifndef FSM_THREAD_H
#define FSM_THREAD_H

#include "fsm.h"

#include <zephyr/drivers/gpio.h>
fsm_t* fsm_new_fsm_thread(fsm_t* fsm_p);
#endif