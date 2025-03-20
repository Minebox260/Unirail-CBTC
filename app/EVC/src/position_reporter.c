#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/position_reporter.h"
#include "../../utility/include/comm_message_listener.h"

#define DEBUG_POS 0

void * report_position(void * args) {
	report_position_args_t * rpa = (report_position_args_t *) args;
	position_t local_pos;
	message_t send_message, recv_message;
	char * data[MAXDATA];
	char pos_str[50];
	int nbcar;
	int initialized = 0;

	while (1) {
		pthread_mutex_lock(rpa->state_mutex);
		local_pos.bal = rpa->state->bal;
		local_pos.pos_r = rpa->state->pos_r;
		pthread_mutex_unlock(rpa->state_mutex);

		if (local_pos.bal != 0) {

			data[0] = malloc(10);
			data[1] = malloc(10);

			snprintf(data[0], 10, "%d", local_pos.bal);
			snprintf(data[1], 10, "%.2f", local_pos.pos_r);

			if (DEBUG_POS){
				printf("Current position : %d, %f\n", local_pos.bal, local_pos.pos_r);
			}

			send_message.req_id = generate_unique_req_id();
			send_message.train_id = rpa->train_id;
			send_message.code = 101;
			for (int i = 0; i < MAXDATA; i++) {
				send_message.data[i] = data[i];
			}

			send_data(rpa->client.sd, rpa->client.adr_serv, send_message);

			//if (!initialized) printf("EVC [%d] - Position initialisée, envoi au RBC...", rpa->train_id);

			wait_for_response(send_message.req_id, &recv_message, 5);

			if (recv_message.code != 201) {
				printf("EVC [%d] - Erreur lors de l'envoi du rapport de position: %d\n", rpa->train_id, recv_message.code);
			} else {
				// Signals to the EOA handler that the RBC acknowledged the first position
				if (!initialized) {
					printf("OK\nEVC [%d] - Position initiale acquittée par le RBC\n", rpa->train_id);
					pthread_mutex_lock(rpa->init_pos_mutex);
					*rpa->pos_initialized = 1;
					initialized = 1;
					pthread_cond_signal(rpa->init_pos_cond);
					pthread_mutex_unlock(rpa->init_pos_mutex);
				}
			}
		
		}

		usleep(1000000);
	}

	pthread_exit(NULL);

	return NULL;
}