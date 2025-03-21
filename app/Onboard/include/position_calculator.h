#ifndef _UNIRAIL_POSITION_CALCULATOR_H
	#define _UNIRAIL_POSITION_CALCULATOR_H

		
	#include "marvelmind.h"
	#include "../../utility/include/map.h"

	typedef struct {
		float pos_r;
		float speed;
	} kalman_state_t;

	typedef struct {
		float P00;
		float P01;
		float P10;
		float P11;
	} kalman_covariance_t;

	typedef struct {
		int bal;
		float pos_r;
		float speed;
	} state_t;

	typedef struct {
		int is_balise;
		float pos_r;
	} can_message_t;

	typedef struct {
		struct MarvelmindHedge * hedge;
		int can_socket;
		state_t * state;
		pthread_mutex_t * state_lock;
		int * mission;
		pthread_mutex_t * mission_mutex;
		int chemin_id;
	} position_calculator_args_t;

	#define PAS_ROUE_CODEUSE 0.016944 

	void * position_calculator(void * args);

#endif