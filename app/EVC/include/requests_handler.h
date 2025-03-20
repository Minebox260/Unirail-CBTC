#ifndef _UNIRAIL_RESPONSE_LISTENER_H
	#define _UNIRAIL_RESPONSE_LISTENER_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <time.h>
	#include <sys/time.h>
	#include <errno.h>

	#include "../../utility/include/comm.h"
	#include "../../utility/include/comm_message_listener.h"
	#include "../../utility/include/map.h"


	typedef struct {
		int sd;
		pthread_mutex_t * mission_mutex;
		int * mission;
		pending_message_t * pending_request;
		int * received_new_eoa;
		pthread_cond_t * eoa_cond;
		pthread_mutex_t * eoa_mutex;
		position_t * eoa;
	} requests_handler_args_t;

	void * requests_handler(void * args);

	void handle_request(message_t recv_message, message_t * send_message, requests_handler_args_t * rha);
#endif