#ifndef _UNIRAIL_EOA_H
	#define _UNIRAIL_EOA_H

	#include "../../utility/include/comm.h"
	#include "../../utility/include/map.h"

	#include "../include/position_calculator.h"

	/**
	 * @struct eoa_handler_args_t
	 * @brief Arguments to pass to the eoa handler thread function
	 * @param client The client struct to use for communication
	 * @param train_id The ID of the train to track the eoa of
	 * @param chemin_id The ID of the track to use
	 * @param eoa The current end of authorization
	 * @param eoa_cond The condition to signal when the eoa is updated
	 * @param received_new_eoa Flag to indicate if a new eoa has been received
	 * @param eoa_mutex The mutex to lock when accessing the eoa
	 * @param state The current state of the train
	 * @param state_mutex The mutex to lock when accessing the state
	 * @param pos_inittialized Flag to indicate if the position has been initialized
	 * @param init_pos_cond The condition that signals when the position is initialized
	 * @param init_pos_mutex The mutex to lock when accessing the initialization condition
	 * @param eoa_inittialized Flag to indicate if the eoa has been initialized
	 * @param init_eoa_cond The condition to signal when the eoa is initialized
	 * @param init_eoa_mutex The mutex to lock when accessing the eoa initialization condition
	 */
	typedef struct {
		client_udp_init_t client;
		int train_id;
		int chemin_id;
		position_t * eoa;
		pthread_cond_t * eoa_cond;
		int * received_new_eoa;
		pthread_mutex_t * eoa_mutex;
		state_t * state;
		pthread_mutex_t * state_mutex;
		int * pos_initialized;
		pthread_cond_t * init_pos_cond;
		pthread_mutex_t * init_pos_mutex;
		int * eoa_initialized;
		pthread_cond_t * init_eoa_cond;
		pthread_mutex_t * init_eoa_mutex;
	} eoa_handler_args_t;

	void * eoa_handler(void * args);
#endif