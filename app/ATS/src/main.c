#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../utility/include/comm.h"

int main(int argc, char *argv[]) {

	if (argc != 2) printf("Utilisation: %s <port_onboard>\n", argv[0]);
	else {
		int train_port = atoi(argv[1]);
		
		client_udp_init_t client;
		
		char input_train[100];
		char input_mission[100];

		char *ip_trains[3] = {"192.168.1.151", "192.168.1.173", "192.168.1.172"}; 

		while (1) {
			printf("Envoi de mission\n\tEntrez le numéro du train (0 pour quitter): ");
			if (!fgets(input_train, sizeof(input_train), stdin)) {
				break;
			}
			input_train[strlen(input_train) - 1] = '\0';
			
			if (strcmp(input_train, "0") == 0) {
				break;
			}
			
			int train_id = atoi(input_train);

			if (train_id < 1 || train_id > 3) {
				printf("Numéro de train incorrect\n");
				continue;
			}

			printf("\tEntrez le nombre de tours à effectuer: ");
			if (!fgets(input_mission, sizeof(input_mission), stdin)) {
				break;
			}
			input_mission[strlen(input_mission) - 1] = '\0';

			int mission = atoi(input_mission);

			if (mission < 1) {
				printf("Nombre de tours incorrect\n");
				continue;
			}

			message_t message;
			message.req_id = generate_unique_req_id();
			message.train_id = train_id;
			message.code = 103;  
			message.data[0] = input_mission;
			message.data[1] = NULL;

			setup_udp_client(&client, ip_trains[train_id - 1], train_port, 0);			
			printf("Envoi de la mission à [%d]...", train_id);
			send_data(client.sd, client.adr_serv, message);

			message_t response;
			receive_data(client.sd, NULL, &response, 0);
			if (response.code == 204) {
				printf("OK\n");
			} else {
				printf("Erreur lors de l'envoi de la mission: %d\n", response.code);
			}

			usleep(1000000);
		}
		return(EXIT_SUCCESS);
	}
}
