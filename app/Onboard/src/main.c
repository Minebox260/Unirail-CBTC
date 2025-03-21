#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
                                    { perror(msg); \
                                        exit(EXIT_FAILURE); }

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

#include "../include/position_reporter.h"
#include "../include/position_calculator.h"
#include "../include/automatique.h"
#include "../include/eoa_handler.h"
#include "../include/requests_handler.h"
#include "../../utility/include/comm_message_listener.h"
#include "../../utility/include/can.h"

#define MARVELMIND_TTY "/dev/ttyACM0"

state_t state = {0, 0.0, 0.0};
position_t eoa = {-1, 0.0};
position_t destination = {3, 20.0};

pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dest_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t eoa_mutex = PTHREAD_MUTEX_INITIALIZER;

int pos_initialized = 0;
pthread_mutex_t init_pos_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init_pos_cond = PTHREAD_COND_INITIALIZER;

int eoa_initialized = 0;
pthread_mutex_t init_eoa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init_eoa_cond = PTHREAD_COND_INITIALIZER;

int received_new_eoa = 0;
pthread_cond_t eoa_cond = PTHREAD_COND_INITIALIZER;

int can_socket;
int mission = 0;
pthread_mutex_t mission_mutex = PTHREAD_MUTEX_INITIALIZER;

void install_signal_deroute(int numSig, void (*pfct)(int));
void stop_train();

int main(int argc, char *argv[]) {
    if (argc != 6) printf("ATO/ATP | Utilisation : ./onboard <id train> <nombre de tours> <adresse_serveur> <port serveur> <port train>\n");
    else {
        
		pthread_t posreport_tid, eoa_tid, message_listener_tid, requests_handler_tid, pos_calculator_tid;
		client_udp_init_t client;
		message_t init_message;
		int init_attempts = 0;

		int train_id = atoi(argv[1]);

		int chemin_id = train_id;

		mission = atoi(argv[2]);
		
        printf("ATO/ATP [%d] - Initialisation\n", train_id);

		setup_udp_client(&client, argv[3], atoi(argv[4]), atoi(argv[5]));

		can_socket = init_can_socket();
		if (can_socket == -1) {
			fprintf(stderr, "L'initialisation de la connexion au bus CAN a échouée\n");
			exit(EXIT_FAILURE);
		}

		int flags = fcntl(can_socket, F_GETFL, 0);
		fcntl(can_socket, F_SETFL, flags | O_NONBLOCK);

		printf("Initialisation du marvelmind...");

		const char * ttyFileName = MARVELMIND_TTY;
		struct MarvelmindHedge * hedge=createMarvelmindHedge();
		if (hedge==NULL) {
			fprintf(stderr, "Impossible d'initialiser la structure du marvelmind\n");
			exit(EXIT_FAILURE);
		} else {
			hedge->ttyFileName=ttyFileName;
			startMarvelmindHedge(hedge);
			printf("OK\n");
		}

		install_signal_deroute(SIGINT, stop_train);

		while (init_attempts < 3) {

			printf("ATO/ATP [%d] - Tentative de connexion au Superviseur...", train_id);

			init_message.req_id = generate_unique_req_id();
			init_message.train_id = train_id;
			init_message.code = 100;
			init_message.data[0] = NULL;

			send_data(client.sd, client.adr_serv, init_message);

			receive_data(client.sd, NULL, &init_message, 5);

			if (init_message.code == 200) {
				break;
			} else {
				fprintf(stderr, "\nATO/ATP [%d] - Erreur lors de la connexion au Superviseur: %d\n", train_id, init_message.code);
				init_attempts++;
			}
		}

		if (init_attempts == 3) {
			fprintf(stderr, "\nATO/ATP [%d] - Impossible de se connecter au Superviseur\n", train_id);
			exit(EXIT_FAILURE);
		}

		printf("OK\nATO/ATP [%d] - Connexion établie\n", train_id);

		pending_message_t pending_request = {-1, {0, 0, 0, {NULL, NULL}}, NULL, 0, PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, NULL};

		message_listener_args_t mla = { client.sd, &pending_request };

		pthread_create(&message_listener_tid, NULL, message_listener, &mla);
		pthread_detach(message_listener_tid);

		requests_handler_args_t rha = {client.sd, &mission_mutex, &mission, &pending_request, &received_new_eoa, &eoa_cond, &eoa_mutex, &eoa};

		pthread_create(&requests_handler_tid, NULL, requests_handler, &rha);
		pthread_detach(requests_handler_tid);
		
		report_position_args_t rpa = {client, train_id, &state, &state_mutex, &pos_initialized, &init_pos_cond, &init_pos_mutex};

		pthread_create(&posreport_tid, NULL, report_position, &rpa);
		pthread_detach(posreport_tid);

		position_calculator_args_t pca = {hedge, can_socket, &state, &state_mutex, &mission, &mission_mutex, chemin_id};

		pthread_create(&pos_calculator_tid, NULL, position_calculator, &pca);
		pthread_detach(pos_calculator_tid);

		eoa_handler_args_t eha = {client, train_id, chemin_id,
			&eoa, &eoa_cond, &received_new_eoa, &eoa_mutex,
			&state, &state_mutex, 
			&pos_initialized, &init_pos_cond, &init_pos_mutex,
			&eoa_initialized, &init_eoa_cond, &init_eoa_mutex};

		pthread_create(&eoa_tid, NULL, eoa_handler, &eha);
		pthread_detach(eoa_tid);

		boucle_automatique_args_t baa = {&state, &state_mutex, &eoa, &eoa_mutex, &mission, &mission_mutex,
			chemin_id, can_socket, &eoa_initialized, &init_eoa_cond, &init_eoa_mutex};

		boucle_automatique(&baa);

    exit(EXIT_SUCCESS);
    }
}

void stop_train() {
	printf("\nArrêt de l'ATO/ATP...\n");
	mc_consigneVitesse(can_socket, 0);
	fflush(stdout);
	close(can_socket);
	exit(EXIT_SUCCESS);
}

void install_signal_deroute(int numSig, void (*pfct)(int)) {
	struct sigaction newAct;
	newAct.sa_handler = pfct;
	CHECK_ERROR(sigemptyset (&newAct.sa_mask), -1, "-- sigemptyset() --");
	newAct.sa_flags = SA_RESTART;
	CHECK_ERROR(sigaction (numSig, &newAct, NULL), -1, "-- sigaction() --");
}