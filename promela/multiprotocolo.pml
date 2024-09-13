
//Response
//Semaforo principal------------------------------------------------------------------
ltl control1 {[]( (deadline && !flag_thread) -> <> (flag_mesh) )}
ltl control2 {[]( (deadline && !flag_mesh) -> <> (flag_thread) )}	 
ltl control3 {[]( (mesh_is_prov) -> <> (flag_mesh))}		
ltl control4 {[]( (thread_is_prov) -> <> (flag_thread))}	

//maquinas thread y mesh
ltl control5 {[]( (deadline_mesh) -> <> (envio_dato_mesh))}
ltl control6 {[]( (deadline_thread) -> <> (envio_dato_thread))}
ltl control7 {[]( (flag_mesh) -> <> (envio_dato_mesh))}
ltl control8 {[]( (flag_thread) -> <> (envio_dato_thread))}

ltl control9 {[]( !(flag_thread) -> <> !(envio_dato_thread))}
ltl control10 {[]( !(flag_mesh) -> <> !(envio_dato_mesh))}


//Seguridad----------------------------------------------------------------------------
ltl control11 {[]!( flag_thread && flag_mesh)}
ltl control12 {[]!( flag_thread && envio_dato_mesh)}
ltl control13 {[]!( flag_mesh && envio_dato_thread)}



// Estados
mtype = {CONF_BT, WAIT, MESH_P, CONF_OT, OT_P, OFF_MESH, READ_MESH, OFF_THREAD, READ_THREAD};
int protocol_controller;
int mesh;
int thread;

// Entradas
int thread_is_prov = 0; 
int mesh_is_prov = 0;
int deadline = 0;
int deadline_mesh = 0;
int deadline_thread = 0;
//variables
int flag_thread = 0;
int flag_mesh = 0;
int envio_dato_mesh = 0;
int envio_dato_thread = 0;

//Controller
active proctype protocol_fsm() {
	protocol_controller = CONF_BT
	do
	:: (protocol_controller == CONF_BT) -> atomic{
		if
		:: (mesh_is_prov) ->  printf("Sleep 60s \n"); protocol_controller = WAIT
		fi
	}
    :: (protocol_controller == WAIT) -> atomic{
        if 
        :: (true) ->  printf("rst_deadline() \n"); flag_mesh = 1; flag_thread = 0; protocol_controller = MESH_P
        fi
    }
    :: (protocol_controller == MESH_P) -> atomic{
        if 
        :: (deadline) -> printf("Mesh Disable()\n"); printf("CoAP Init() \n"); deadline= 0; flag_mesh = 0; flag_thread = 0; protocol_controller = CONF_OT
        fi
    }
    :: (protocol_controller == CONF_OT) -> atomic{
        if 
        :: (thread_is_prov) -> printf("rst_deadline() \n"); flag_thread = 1; flag_mesh = 0; protocol_controller = OT_P
        fi
    }
    :: (protocol_controller == OT_P) -> atomic{
        if 
        :: (deadline) -> printf("rst_deadline() \n"); printf("Thread Disable() \n");printf("Mesh Enable()\n") ; deadline = 0;flag_thread = 0; flag_mesh = 0; protocol_controller = CONF_BT
        fi
    }
	od
}

active proctype mesh_fsm() {
	mesh = OFF_MESH
	do
	:: (mesh == OFF_MESH) -> atomic{
		if
		:: (flag_mesh) ->  printf("Init Timer Mesh \n"); mesh = READ_MESH
		fi
	}
    :: (mesh == READ_MESH) -> atomic{
        if 
        :: (flag_mesh && envio_dato_mesh) ->  printf("Envio dato mesh() \n"); envio_dato_mesh = 0; mesh = READ_MESH
        :: (!flag_mesh) ->  printf("Stop timer mesh() \n"); envio_dato_mesh = 0; mesh = OFF_MESH
        fi
    }
	od

}

active proctype thread_fsm() {
	thread = OFF_THREAD
	do
	:: (thread == OFF_THREAD) -> atomic{
		if
		:: (flag_thread) ->  printf("Init Timer Thread \n"); thread = READ_THREAD
		fi
	}
    :: (thread == READ_THREAD) -> atomic{
        if 
        :: (flag_thread && envio_dato_thread) ->  printf("Envio dato mesh() \n"); envio_dato_thread = 0; thread = READ_THREAD
        :: (!flag_thread) ->  printf("Stop timer Thread() \n"); envio_dato_thread = 0; thread = OFF_THREAD
        fi
    }
	od

}

// Entorno
active proctype entorno() {
	do
		:: if
			:: thread_is_prov = 1; 
            :: mesh_is_prov = 1;
            :: deadline = 1;
            :: deadline_mesh = 1;
            :: deadline_thread = 1;
            
            :: flag_thread = 1;
            :: flag_mesh = 1;
            :: envio_dato_mesh = 1;
            :: envio_dato_thread = 1; 
			:: skip -> skip
		fi
	od	
}