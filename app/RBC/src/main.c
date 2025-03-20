#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
                                    { perror(msg); \
                                        exit(EXIT_FAILURE); }

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "../../utility/include/comm.h"
#include "../../utility/include/comm_message_listener.h"
#include "../../utility/include/const_chemins.h"
#include "../../utility/include/can_infra.h"
#include "../../utility/include/can.h"
#include "../include/requests_handler.h"
#include "../include/trains.h"

int main(int argc, char *argv[]) {
    if (argc != 2) printf("RBC | Utilisation : ./rbc <port serveur> \n");
    else {
        
		pthread_t message_listener_tid, tid;

        printf("RBC - Initialisation\n");

		init_trains();

        int sd = setup_udp_server(atoi(argv[1]));

		int can_socket = init_can_socket();

		pending_message_t pending_request = {-1, {0, 0, 0, {NULL, NULL}}, NULL, 0, PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, NULL};

		message_listener_args_t mla = { sd, &pending_request };

		pthread_create(&message_listener_tid, NULL, message_listener, &mla);
		pthread_detach(message_listener_tid);

        while (1) {

			pthread_mutex_lock(&pending_request.mutex);
			while (!pending_request.ready) {
				pthread_cond_wait(&pending_request.cond, &pending_request.mutex);
			}

			handle_and_respond_args_t *hra = malloc(sizeof(handle_and_respond_args_t));
			if (!hra) {
				fprintf(stderr, "Erreur d'allocation lors du traitement de la requête : %s\n", strerror(errno));
				continue;
			}

			message_t *copied_message = copy_message(&pending_request.message);

			hra->sd = sd;
			hra->client_adr = pending_request.recv_adr;
			hra->recv_message = *copied_message;
			hra->can_socket = can_socket;

            pthread_create(&tid, NULL, handle_and_respond, hra);
			pthread_detach(tid);

			free(copied_message);

			pending_request.ready = 0;
			pending_request.message = (message_t) {0, 0, 0, {NULL, NULL}};

			pthread_mutex_unlock(&pending_request.mutex);

        }

    exit(EXIT_SUCCESS);
    }
}