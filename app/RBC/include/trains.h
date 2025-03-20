#ifndef _UNIRAIL_TRAINS_H_
	#define _UNIRAIL_TRAINS_H_
	#define NB_TRAINS 3

	#include <pthread.h>
	#include "../../utility/include/map.h"

	/**
	 * @brief Initializes the trains positions and their mutexes.
	 */
	void init_trains();

	extern position_t pos_trains[NB_TRAINS]; 
	extern pthread_mutex_t pos_trains_locks[NB_TRAINS];
	extern int next_bal_index_req[NB_TRAINS];
	extern int next_bal_index_lib[NB_TRAINS];

#endif