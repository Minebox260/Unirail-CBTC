#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "../../utility/include/map.h"
#include "../include/requests_handler.h"
#include "../include/ressources.h"
#include "../include/trains.h"
#include "../include/eoa_calculator.h"
#include "../../utility/include/const_chemins.h"
#include "../../utility/include/map.h"
#include "../../utility/include/can.h"
#include "../../utility/include/can_infra.h"
#include "../../utility/include/comm_message_listener.h"

void * handle_and_respond(void * args) {
	handle_and_respond_args_t * hra = (handle_and_respond_args_t *) args;
	message_t send_message;

	handle_request(hra->recv_message, &send_message, hra->can_socket, hra->client_adr, hra->sd);

	send_data(hra->sd, *hra->client_adr, send_message);

	free(hra->client_adr);
	free(hra);

	pthread_exit(NULL);

	return NULL;
}

void handle_request(message_t recv_message, message_t * send_message, int can_socket, struct sockaddr_in * recv_adr, int sd) {

	send_message->train_id = recv_message.train_id;
	send_message->req_id = recv_message.req_id;

	if (recv_message.train_id - 1 >= NB_TRAINS) {
		printf("Superviseur - ID de train inconnu\n");
		send_message->code = 405;
		send_message->data[0] = NULL;
		return;
	}

   	switch (recv_message.code) {

		// Rapport de position
	   	case 101:
			printf("Superviseur [%d] - Rapport de position reçu\n", recv_message.train_id-1);

			char * endptr; // Pointeur pour vérifier que la conversion s'est bien passée

			long bal = strtol(recv_message.data[0], &endptr, 10);
			if (endptr == recv_message.data[0]) { // Le pointeur n'a pas bougé, la conversion a échoué
				send_message->code = 401;
				send_message->data[0] = NULL;
				break;
			}

			double pos_r = strtod(recv_message.data[1], &endptr);
			if (endptr == recv_message.data[1]) {
				send_message->code = 401;
				send_message->data[0] = NULL;
				break;
			}

			pthread_mutex_lock(&pos_trains_locks[recv_message.train_id - 1]);
			pos_trains[recv_message.train_id - 1].bal = bal;
			pos_trains[recv_message.train_id - 1].pos_r = atof(recv_message.data[1]);
			free_resources(&next_bal_index_lib[recv_message.train_id - 1],recv_message.train_id - 1, pos_trains);
			if (DEBUG_RES_FREE){
				printf("Superviseur [%d] - Prochaine ressource libérée : %d\n", recv_message.train_id - 1, next_bal_index_lib[recv_message.train_id - 1]);
				printf("Superviseur [%d] - Ressources libres : %d\n", recv_message.train_id - 1, resources);
			}
			pthread_mutex_unlock(&pos_trains_locks[recv_message.train_id - 1]);


			send_message->code = 201;
			send_message->data[0] = NULL;
			break;

		case 102:
			printf("Superviseur [%d] - Demande d'autorisation de mouvement reçue\n", recv_message.train_id - 1);

			struct sockaddr_in * recv_adr_copy = malloc(sizeof(struct sockaddr_in));
			memcpy(recv_adr_copy, recv_adr, sizeof(struct sockaddr_in));

			send_message->code = 202;
			send_message->data[0] = NULL;

			handle_eoa_request_args_t *hea = malloc(sizeof(handle_eoa_request_args_t));

			hea->can_socket = can_socket;
			hea->recv_message = recv_message;
			hea->recv_adr = recv_adr_copy;
			hea->sd = sd;

			pthread_t tid;
			pthread_create(&tid, NULL, handle_eoa_request, hea);
			pthread_detach(tid);

			break;

		case 100:
			send_message->code = 200;
			send_message->data[0] = NULL;
			send_message->data[1] = NULL;

			break;
			
	   	default:
			printf("Superviseur - Code de requête inconnu\n");
			send_message->code = 400;
			send_message->data[0] = NULL;
			break;
   }
}

void * handle_eoa_request(void * args) {
	message_t send_message, recv_message;
	handle_eoa_request_args_t * hea = (handle_eoa_request_args_t *) args;
	int nb_attempts = 0;

	ask_resources(&next_bal_index_req[hea->recv_message.train_id - 1],hea->recv_message.train_id - 1, pos_trains, hea->can_socket);

	if (DEBUG_RES){
		printf("Superviseur [%d] - Ressource accordée : %d\n", hea->recv_message.train_id - 1, next_bal_index_req[hea->recv_message.train_id - 1]);
		printf("Superviseur [%d] - Ressources libres : %d\n", hea->recv_message.train_id - 1, resources);
	}

	position_t EOA = next_eoa(hea->recv_message.train_id-1,pos_trains,L_res_req[hea->recv_message.train_id - 1][next_bal_index_req[hea->recv_message.train_id - 1]], chemins, tailles_chemins);
	printf("Superviseur [%d] - Prochaine EOA pour train %d: balise %d, position %f\n", hea->recv_message.train_id - 1, hea->recv_message.train_id - 1, EOA.bal, EOA.pos_r);

	send_message.req_id = generate_unique_req_id();
	send_message.code = 104;
	send_message.train_id = hea->recv_message.train_id;
	
	char * data[2];

	data[0] = malloc(10);
	data[1] = malloc(10);

	snprintf(data[0], 10, "%d", EOA.bal);
	snprintf(data[1], 10, "%2.f", EOA.pos_r);

	for (int i = 0; i < 2; i++) {
		send_message.data[i] = data[i];
	}

	while (nb_attempts < 5) {
		send_data(hea->sd, *hea->recv_adr, send_message);

		wait_for_response(send_message.req_id, &recv_message, 3);

		if (recv_message.code == 204) {
			printf("Superviseur [%d] - EOA acquittée\n", hea->recv_message.train_id - 1);
			break;
		} else {
			printf("Superviseur [%d] - Pas de réponse à l'envoi d'EOA\n", hea->recv_message.train_id - 1);
			nb_attempts++;
		}
	}

	if (nb_attempts == 5) {
		printf("Superviseur [%d] - EOA non acquittée\n", hea->recv_message.train_id - 1);
	}

	pthread_exit(EXIT_SUCCESS);
}