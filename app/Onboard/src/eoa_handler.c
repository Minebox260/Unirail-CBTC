#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/eoa_handler.h"
#include "../include/automatique.h"
#include "../include/map.h"
#include "../../utility/include/comm_message_listener.h"
#include "../../utility/include/debug.h"

void * eoa_handler(void * args) {
	eoa_handler_args_t * eha = (eoa_handler_args_t *) args;
	position_t local_pos;
	message_t send_message, recv_message;
	char * data[MAXDATA];
	char pos_str[50];
	int nbcar;
	int request_new_eoa = 0;
	int initialized = 0;

	const int distance_triggers[] = { 5000, 2500, 1000 };
	const int distance_triggers_size = sizeof(distance_triggers) / sizeof(int);

	double distance;
	double last_distance_triggered = distance_triggers[0];

	pthread_mutex_lock(eha->init_pos_mutex);
	while (!*(eha->pos_initialized)) {
		pthread_cond_wait(eha->init_pos_cond, eha->init_pos_mutex);
	}
	pthread_mutex_unlock(eha->init_pos_mutex);

	request_new_eoa = 1;

	while (1) {
		pthread_mutex_lock(eha->state_mutex);
		local_pos.bal = eha->state->bal;
		local_pos.pos_r = eha->state->pos_r;
		pthread_mutex_unlock(eha->state_mutex);

		if (local_pos.bal  != 0) {
			pthread_mutex_lock(eha->eoa_mutex);
			if (eha->eoa->bal != -1) distance = get_distance(local_pos, *(eha->eoa), eha->chemin_id);
			pthread_mutex_unlock(eha->eoa_mutex);

			DEBUG_PRINT("ATO/ATP [%d] - [EOA] Distance jusqu'à l'EOA : %.2f\n", eha->train_id, distance);
			fflush(stdout);

			// Sends a request once when the train reaches each threshold, or continuously if it is past the last threshold
			for (int i = 0; i < distance_triggers_size - 1; i++) {
				if (distance < distance_triggers[i] && last_distance_triggered >= distance_triggers[i]) {
					request_new_eoa = 1;
				}
			}
			if (distance < distance_triggers[distance_triggers_size - 1]) {
				request_new_eoa = 1;
			}

			if (request_new_eoa) {
				printf("ATO/ATP [%d] - Envoi d'une nouvelle demande d'autorisation de mouvement...", eha->train_id);
				send_message.req_id = generate_unique_req_id();
				send_message.train_id = eha->train_id;
				send_message.code = 102;
				for (int i = 0; i < MAXDATA; i++) {
					send_message.data[i] = NULL;
				}

				send_data(eha->client.sd, eha->client.adr_serv, send_message);

				request_new_eoa = 0;
				last_distance_triggered = distance;

				wait_for_response(send_message.req_id, &recv_message, 0);

				if (recv_message.code == 202) {
					printf("OK\n");

					pthread_mutex_lock(eha->eoa_mutex);
					while (!*eha->received_new_eoa) {
						pthread_cond_wait(eha->eoa_cond, eha->eoa_mutex);
					}
					*eha->received_new_eoa = 0;
					pthread_mutex_unlock(eha->eoa_mutex);

					if (!initialized) {
						printf("ATO/ATP [%d] - Initialisation terminée\n", eha->train_id);
						pthread_mutex_lock(eha->init_eoa_mutex);
						pthread_cond_signal(eha->init_eoa_cond);
						*eha->eoa_initialized = 1;
						pthread_mutex_unlock(eha->init_eoa_mutex);
						initialized = 1;
					}

				} else {
					printf("ATO/ATP [%d] - Erreur lors de l'envoi de la demande d'autorisation de mouvement: %d\n", eha->train_id, recv_message.code);
				}
			}
		}

		usleep(1000000);
	}

	pthread_exit(NULL);

	return NULL;
}