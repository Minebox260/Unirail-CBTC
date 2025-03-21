#include "../include/trains.h"

position_t pos_trains[NB_TRAINS];
int next_bal_index_req[NB_TRAINS] = {0,0,0};
int next_bal_index_lib[NB_TRAINS] = {0,0,0};
pthread_mutex_t pos_trains_locks[NB_TRAINS];

void init_trains() {
	for (int i = 0; i < NB_TRAINS; i++) {
		pos_trains[i].pos_r = 0.0;
		pthread_mutex_init(&pos_trains_locks[i], NULL);
	}
	pos_trains[0].bal = 1;
	pos_trains[1].bal = 21;
	pos_trains[2].bal = 22;

}