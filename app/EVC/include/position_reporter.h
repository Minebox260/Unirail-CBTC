#ifndef _UNIRAIL_POSITION_H
	#define _UNIRAIL_POSITION_H

	#include "../../utility/include/comm.h"
	#include "../../utility/include/map.h"
	#include "../include/position_calculator.h"


	/**
	 * @struct report_position_args_t
	 * @brief Arguments to pass to the report_position thread function
	 * @param client The client struct to use for communication
	 * @param train_id The ID of the train to report the position of
	 * @param state The state with the position to report
	 * @param state_mutex The mutex to lock when accessing the state
	 * @param pos_initialized Flag to indicate if the position has been initialized
	 * @param init_pos_cond The condition to signal when the position is initialized
	 * @param init_pos_mutex The mutex to lock when accessing the initialization condition
	 */
	typedef struct {
		client_udp_init_t client;
		int train_id;
		state_t * state;
		pthread_mutex_t * state_mutex;
		int * pos_initialized;
		pthread_cond_t * init_pos_cond;
		pthread_mutex_t * init_pos_mutex;
	} report_position_args_t;

	void * report_position(void * args);
#endif