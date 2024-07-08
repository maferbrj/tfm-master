
#ifndef FSM_PROTOCOL_CONTROLLER_H
#define FSM_PROTOCOL_CONTROLLER_H

#include "fsm.h"
#include <stdint.h>


struct p_fsm_t;
typedef struct p_fsm_t p_fsm_t;


fsm_t* fsm_new_fsm_protocol_controller(uint32_t Tprotocol);
void set_flag_mesh (fsm_t* this);
void clear_flag_mesh (fsm_t* this);
int get_flag_mesh (fsm_t* this);
void set_flag_thread (fsm_t* this);
void clear_flag_thread (fsm_t* this);
int get_flag_thread (fsm_t* this);


#endif