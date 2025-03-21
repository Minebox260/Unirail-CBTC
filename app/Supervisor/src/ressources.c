#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "../include/ressources.h"
#include "../include/trains.h"
#include "../../utility/include/const_chemins.h"

unsigned char resources = 0b000000; // 6 bits pour représenter les ressources
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void lock_ressources(unsigned char res_mask) {
    pthread_mutex_lock(&lock);
    
    while ((resources & res_mask) != 0) { // Attente jusqu'à disponibilité
        pthread_mutex_unlock(&lock);
        usleep(50000); // Petit délai pour éviter de bloquer le thread
        pthread_mutex_lock(&lock);
    }
    resources |= res_mask; // Prendre les ressources
    pthread_mutex_unlock(&lock);
}

void unlock_ressources(unsigned char res_mask) {
    pthread_mutex_lock(&lock);
    resources &= ~res_mask; // Libérer les ressources
    pthread_mutex_unlock(&lock);
}


void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains, int can_socket){
	pthread_mutex_lock(&pos_trains_locks[no_train]);
	if (DEBUG_RES){
		printf("Superviseur [%d] - \n\nDans demande ressource\nPosition bal: %d, pos: %f\n",no_train, pos_trains[no_train].bal, pos_trains[no_train].pos_r);
		printf("Superviseur [%d] - Indice de la prochaine ressource : %d \nRessource à verrouiller : %d\n",no_train, *next_bal_index, L_mask_req[no_train][*next_bal_index]);
		printf("Superviseur [%d] - Balise actuelle: %d, balise à attendre pour demande: %d | %d | %d\n",no_train,pos_trains[no_train].bal, L_res_req[no_train][(*next_bal_index - 1 + tailles_chemins[no_train])%tailles_chemins[no_train]], no_train, (*next_bal_index - 1 + tailles_chemins[no_train])%tailles_chemins[no_train]);
	}

	// Si la position du train correspond à la position de demande de la prochaine ressource
	// On veut vérifier : bal actuelle = bal de ressource - 1
	if (DEBUG_RES){
		printf("Superviseur [%d] - Taille chemin %d: %d\n",no_train, no_train, tailles_chemins[no_train]);
	}
	int resource_to_lock = 0;
	for (int i=0;i<tailles_chemins[no_train];i++){
		if (DEBUG_RES){
			printf("Superviseur [%d] - Dans loop\n",no_train);
		}
		// donc bal actuelle = bal d'indice i - i
		int condition_1 = (pos_trains[no_train].bal == chemins[no_train][(i-1 + tailles_chemins[no_train])%tailles_chemins[no_train]]);
		// ou bal actuelle = bal d'indice i
		int condition_1bis = (pos_trains[no_train].bal == chemins[no_train][i]);
		// ET bal de ressource = bal d'indice i
		int condition_2 = (L_res_req[no_train][*next_bal_index] == chemins[no_train][i]);
		if (DEBUG_RES){
			printf("Superviseur [%d] - Condition 1: %d 1bis: %d 2: %d\n",no_train, condition_1, condition_1bis, condition_2);
			printf("Superviseur [%d] - Balise actuelle %d\nBalise d'indice i - 1 %d\nBalise de ressource %d\nBalise d'indice i %d\n",no_train, pos_trains[no_train].bal, chemins[no_train][(i-1 + tailles_chemins[no_train])%tailles_chemins[no_train]], L_res_req[no_train][*next_bal_index], chemins[no_train][i]);
		}

		if ((condition_1 || condition_1bis) && condition_2){
			resource_to_lock = 1;
			if (DEBUG_RES){
				printf("Superviseur [%d] - La ressource doit être verrouillée\n",no_train);
			}
			break;
		}
	}
	if (resource_to_lock && (pos_trains[no_train].pos_r >= 0)){
		if (DEBUG_RES){
			printf("Superviseur [%d] - Ressources avant lock: %d\n",no_train, resources);
			printf("Superviseur [%d] - Ressource à lock: %d\n",no_train, L_mask_req[no_train][*next_bal_index]);
		}
		pthread_mutex_unlock(&pos_trains_locks[no_train]);
		lock_ressources(L_mask_req[no_train][*next_bal_index]);
		pthread_mutex_lock(&pos_trains_locks[no_train]);
		if (DEBUG_RES){
			printf("Superviseur [%d] - Ressources après lock: %d\n",no_train, resources);
		}

		if (DEBUG_RES){
			printf("Superviseur [%d] - Test condition :\n Indice actuel %d\n Limite %ld\n",no_train, *next_bal_index, sizeof(L_res_req[no_train])/sizeof(int));
		}

		if (DEBUG_AIG){
			printf("Superviseur [%d] - Setting switches\n",no_train);
		}
		set_all_switch(no_train, *next_bal_index, can_socket);
		if (DEBUG_AIG){
			printf("Superviseur [%d] - Switches set\n",no_train);
		}

		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		// Sinon, c'est la prochaine dans le chemin
		*next_bal_index = (*next_bal_index + 1)%L_res_size_req[no_train];
	}
	
	if (DEBUG_RES){
		printf("Superviseur [%d] - Indice de la prochaine ressource : %d \nProchaine ressource à verrouiller : %d\n",no_train, *next_bal_index, L_mask_req[no_train][*next_bal_index]);

		printf("Superviseur [%d] - Sortie demande ressource\n\n",no_train);
	}
	pthread_mutex_unlock(&pos_trains_locks[no_train]);
}

void free_resources(int * next_bal_index, int no_train, position_t * pos_trains){
	if ((pos_trains[no_train].bal == L_res_lib[no_train][*next_bal_index]) && (pos_trains[no_train].pos_r >= 0)){
		unlock_ressources(L_mask_lib[no_train][*next_bal_index]);
		if (DEBUG_RES_FREE){
			printf("Superviseur [%d] - Balise %d, libèration de la ressource %d\n", no_train, pos_trains[no_train].bal, L_mask_lib[no_train][*next_bal_index]);
		}
		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		// Sinon, c'est la prochaine dans le chemin
		*next_bal_index = (*next_bal_index + 1)%L_res_size_lib[no_train];
	}
}