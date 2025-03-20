#include "automatique.h"
#include "../../utility/include/can.h"
#include "../../utility/include/debug.h"

int min_speed = 2;

//Donne la consigne en vitesse à partir de state qui est [vitesse,distance_a_objectif]
int compute_new_speed(int state[2],double elapsed_time, double *retenue_sur_vitesse){ 
    int v = state[0];
	int d = state[1];
    int new_speed = v;
	float dist_freinage = - 10*((v*v))/(2*max_deacceleration);
	if(d<=dist_freinage){
		*retenue_sur_vitesse = (double) v;
		DEBUG_PRINT("vitesse : %d -- distance freinage: %f\n", v, dist_freinage);
		return((int) (sqrtf(v*v + 0.2* max_deacceleration * (dist_freinage - d))));
	}
	*retenue_sur_vitesse += max_acceleration * elapsed_time; //L'idée est qu'on souhaite incrémenter v par cette valeur (<1) , mais v est un int. Donc on retient cette valeur et on continue à l'incrémenter à chaque appel de la fonction
	DEBUG_PRINT("vitesse : %d -- retenue : %f\n", v, *retenue_sur_vitesse);
	return (int) *retenue_sur_vitesse;
}

//Envoie sur le bus can de l'EVC une commande de vitesse pour faire avancer le train à la vitesse v
int mc_consigneVitesse(int can_socket, int speed){
    struct can_frame frame;

	int data[8];
	data[0] = speed > 0 ? speed : -speed;
	data[1] = speed > 0 ? 0x01 : 0x00;

	return send_can_data(can_socket, 0x17, 2, data);
}

int get_limit_speed(position_t pos_current, const int chemin_id){
	const int* chemin = chemins[chemin_id - 1];
	const int taille_chemin = tailles_chemins[chemin_id - 1];
	int bal1 = pos_current.bal;
	int i = 0; 
	float a;
	int v_limite_pente_next = 100000000;
	int v_limite_pente_last = 100000000;
	while(chemin[i] != bal1){ //On se place sur la bal1
		i++;
	}
	int v_last = v_limits[chemin_id - 1][(i-1+taille_chemin)%taille_chemin];
	int v1 = v_limits[chemin_id - 1][i];
	int v_next = v_limits[chemin_id - 1][(i+1)%taille_chemin];
	if(v1<= v_next && v1<=v_last) return v1;
	if(v1>v_next){ // On ralentis à la fin du tronçon
		int j = 0;
		while((B1[j] != chemin[i] || B2[j] != chemin[(i+1)%taille_chemin]) && (B1[j] != chemin[(i+1)%taille_chemin] || B2[j] != chemin[i])){ //On trouve l'arc qui va de notre balise jusqu'a la prochaine balise
			j++;
		}
		a = max_deacceleration;
		float pos_r_debut_deacceleration = D[j] - 10*((v_next*v_next - v1*v1))/(2*a);
		DEBUG_PRINT("v1 v_next : %d %d -- pos_r %f  -- pos acceleration %f -- distance_fin %f \n" , v1, v_next, pos_current.pos_r, pos_r_debut_deacceleration, D[j]);
		if(pos_current.pos_r < pos_r_debut_deacceleration) (v_limite_pente_next = v1);
		else if(pos_current.pos_r > D[j]) (v_limite_pente_next = v_next);
		else (v_limite_pente_next = sqrtf(v1*v1 + 0.2 * a * (pos_current.pos_r - pos_r_debut_deacceleration)));
	}
	if(v1>v_last){ //On accelere au début du tronçon
		a = max_acceleration;
		float pos_r_fin_acceleration = 10*((v1*v1 - v_last*v_last))/(2*a);
		if(pos_current.pos_r > pos_r_fin_acceleration) (v_limite_pente_last = v1);
		else (v_limite_pente_last =(sqrtf(v_last*v_last + 0.2 * a * (pos_r_fin_acceleration - pos_current.pos_r))));
	}
	if(v_limite_pente_last > v_limite_pente_next) return v_limite_pente_next;
	else return v_limite_pente_last;
}

int init_train(int can_socket) {
    struct can_frame frame;
	int last_balise = 0;

	mc_consigneVitesse(can_socket, 20);
	printf("EVC - Recherche de balise...");
	fflush(stdout);
    while (last_balise == 0) {
        int n = read(can_socket, &frame, sizeof(struct can_frame));
        if (n < 0) continue;
        if (frame.can_id == 0x30) {
            last_balise = frame.data[5];
        }
    }
	printf("OK\nEVC - Balise trouvée: %d\n",last_balise);
	fflush(stdout);
	return last_balise;
}

void * boucle_automatique(void * args) {
	boucle_automatique_args_t * baa = (boucle_automatique_args_t *) args;
    struct can_frame frame;
	clock_t start, end;
	position_t pos_current;
    double elapsed_time;
	float integration_erreur = 0;
	float erreur;
	int current_speed;
	float dist, dist_eoa, dist_start;
	int initialized = 0;
	int speed_limit = 0;
	int min_speed = 2;
	int local_mission = 0;
	double retenue_sur_vitesse = 0.0;
	position_t start_pos;

	int init_balise = init_train(baa->can_socket);
	int last_balise = init_balise;

	start_pos.bal = start_chemins[baa->chemin_id - 1];
	start_pos.pos_r = 0.0;

	mc_consigneVitesse(baa->can_socket, 0);

	pthread_mutex_lock(baa->state_lock);
	baa->state->bal = init_balise;
	baa->state->pos_r = 0.0;
	pthread_mutex_unlock(baa->state_lock);
	
	pos_current.bal = init_balise;
	pos_current.pos_r = 0.0;
	
	start = clock();
    while (1) {

		pthread_mutex_lock(baa->mission_mutex);
		local_mission = *(baa->mission);
		pthread_mutex_unlock(baa->mission_mutex);

		pthread_mutex_lock(baa->state_lock);
		current_speed = baa->state->speed;
		pos_current.bal = baa->state->bal;
		pos_current.pos_r = baa->state->pos_r;
		pthread_mutex_unlock(baa->state_lock);

		end = clock();
		// Calcul du temps écoulé en secondes
		elapsed_time = 100 * ((double)(end - start)) / CLOCKS_PER_SEC;
		start = end;

		if (!initialized) {
			pthread_mutex_lock(baa->init_mutex);
			while (!*(baa->initialized)) {
				pthread_cond_wait(baa->init_cond, baa->init_mutex);
			}
			pthread_mutex_unlock(baa->init_mutex);
			initialized = 1;
		} else {

			pthread_mutex_lock(baa->eoa_lock);
			if (baa->eoa->bal < 1) dist = 0;
			else{
				dist_eoa = get_distance(pos_current, *baa->eoa, baa->chemin_id);
				dist_start = get_distance(pos_current, start_pos, baa->chemin_id);
			}
			pthread_mutex_unlock(baa->eoa_lock);

			if (local_mission > 0 || dist_eoa < dist_start) dist = dist_eoa; 
			else dist = dist_start;
			speed_limit = get_limit_speed(pos_current, baa->chemin_id);

			int state[2] = {current_speed, (int) dist};
			int newSpeed = compute_new_speed(state, elapsed_time,&retenue_sur_vitesse);
			
			DEBUG_PRINT("Bal %d -- Pos %f -- Distance %f -- Speed %d -- newSpeed %d -- speedLimit %d \n",pos_current.bal,pos_current.pos_r,dist,current_speed,newSpeed, speed_limit);
			fflush(stdout);

			if(newSpeed > speed_limit){
				newSpeed = speed_limit;
				retenue_sur_vitesse = (double) speed_limit;
			}
			mc_consigneVitesse(baa->can_socket, newSpeed);
		}
	

		usleep(5000);

	}
    
}