#include <stdio.h>

#include "../include/eoa_calculator.h"

position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins){
	pthread_mutex_lock(&pos_trains_locks[0]);
	pthread_mutex_lock(&pos_trains_locks[1]);
	pthread_mutex_lock(&pos_trains_locks[2]);
	if (DEBUG_EOA){
		printf("Entrée dans next EOA\nNo train: %d\n Pos train actuel: %d, %f\nProchaine balise avant ressource: %d\n", num_train, pos_trains[num_train].bal, pos_trains[num_train].pos_r, next_balise_avant_ressource);
	}
	float distance_secu = 800.0;
    int i = 0; 
	float d;
	float d_min;
    position_t current_position = pos_trains[num_train];
	position_t pos_balise = (position_t) {next_balise_avant_ressource,0.0};
	position_t pos_t2 = (position_t) {pos_trains[(num_train+1)%3].bal,pos_trains[(num_train+1)%3].pos_r - distance_secu};
	position_t pos_t3 = (position_t) {pos_trains[(num_train+2)%3].bal,pos_trains[(num_train+2)%3].pos_r - distance_secu};
	if (DEBUG_EOA) printf("Pos 2: %d:%f\t Pos 3: %d:%f", pos_t2.bal, pos_t2.pos_r, pos_t3.bal, pos_t3.pos_r);
	const int * chemin = chemins[num_train];
    int len_chemin = len_chemins[num_train];
	int train2_on_tracks = 0;
	int train3_on_tracks = 0;
    // On se place sur la balise de la position actuelle
	if (DEBUG_EOA) printf("Before while\n");
	while(i != len_chemin){ 
		if(pos_t2.bal == chemin[i]) train2_on_tracks = 1;
		if(pos_t3.bal == chemin[i]) train3_on_tracks = 1;
		i++;
	}
	if (DEBUG_EOA) printf("Trains on track: %d:%d\n", train2_on_tracks, train3_on_tracks);
	fflush(stdout);
	d_min = get_distance(current_position,pos_balise,num_train + 1);
	if (DEBUG_EOA) printf("Distance to EOA: %f\n", d_min);
	position_t next_eoa = pos_balise;
	if(train2_on_tracks){
		d = get_distance(current_position,pos_t2,num_train + 1);
		if (DEBUG_EOA) printf("Distance to train 2: %f\n", d);
		if(d<d_min){
			d_min = d;
			next_eoa = pos_t2;
		}
	}
	if(train3_on_tracks){
		d = get_distance(current_position,pos_t3,num_train + 1);
		if (DEBUG_EOA) printf("Distance to train 3: %f\n", d);
		if(d<d_min){
			d_min = d;
			next_eoa = pos_t3;
		}
	}
	pthread_mutex_unlock(&pos_trains_locks[0]);
	pthread_mutex_unlock(&pos_trains_locks[1]);
	pthread_mutex_unlock(&pos_trains_locks[2]);
	return next_eoa;
} 