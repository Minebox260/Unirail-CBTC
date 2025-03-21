#ifndef _UNIRAIL_AUTOMATIQUE_H
	#define _UNIRAIL_AUTOMATIQUE_H
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <linux/can.h>
	#include <sys/socket.h>
	#include <net/if.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <linux/can.h>
	#include <linux/can/raw.h>
	#include <fcntl.h>
	#include <time.h>
	#include <pthread.h>
	#include <math.h>

	#include "../../utility/include/map.h"
	#include "../include/position_calculator.h"

	#define PAS_ROUE_CODEUSE 0.016944 // en cm

	/** 
	 *	@brief Donne la consigne en vitesse à partir de l'etat actuel du train.
	*	@param state état du train, composée de [distance à la destination,vitesse_actuelle] en [cm,cm/s].
	*	@param elapsed_time temps écoulé depuis le dernier appel de la fonction.
	*	@param retenue_sur_vitesse valeur de la vitesse à ajouter à la vitesse actuelle.
	*	@return La commande en vitesse (en cm/s) nécessaire pour arriver à destination sans dépassement et en respectant les courbes de freinage (TODO).
	**/
	int compute_new_speed(int state[2],double elapsed_time, double *retenue_sur_vitesse);

	/** 
	 *	@brief Envoie une commande sur le bus CAN pour commander la vitesse.
	*	@param can_socket socket du bus CAN.
	*	@param v vitesse demandée en cm/s.
	*	@return valeur d'erreur (1 si erreur, sinon 0).
	**/
	int mc_consigneVitesse(int can_socket, int v);

	/** 
	 *	@brief Récupère la valeur de la vitesse dans le frame du can_frame.
	 *	@param frame frame récuperée sur le can0 avec read(sock, &frame, sizeof(struct can_frame).
	 *	@return vitesse en cm/s.
	**/
	int read_speed_from_frame(struct can_frame frame);

	/** 
	 *	@brief Récupère la valeur de la position dans le frame du can_frame.
	 *	@param frame frame récuperée sur le can0 avec read(sock, &frame, sizeof(struct can_frame).
	 *	@return position relative à la dernière balise detectée (calculée par endométrie).
	**/
	float read_relative_pos_from_frame(struct can_frame frame);

	int get_limit_speed(position_t pos_current, const int chemin_id);

	/** 
	 *	@brief Initie l'automatique du train en le faisant avancer jusqu'a la première balise (afin qu'il connaisse sa position initiale).
	 *	@param can_socket socket du bus CAN.
	 *	@return numéro de la première balise rencontrée.
	**/
	int init_train(int can_socket);

	/**
	 * @struct boucle_automatique_args_t
	 * @brief Structure pour stocker les arguments de la boucle automatique.
	 * @param state pointeur vers l'état actuel.
	 * @param state_lock mutex pour la lecture de l'état
	 * @param eoa Pointeur vers l'eoa.
	 * @param eoa_lock Mutex pour la lecture de l'eoa.
	 * @param mission Pointeur vers le nombre de tours à effectuer.
	 * @param mission_mutex Mutex pour l'écriture/lecture de la mission.
	 * @param chemin_id Identifiant du chemin à suivre.
	 * @param can_socket socket du bus CAN.
	 * @param initialized Flag to indicate if the position has been initialized
	 * @param init_cond The condition to signal when the position is initialized
	 * @param init_mutex The mutex to lock when accessing the initialization condition
	 */
	typedef struct {
		state_t * state;
		pthread_mutex_t * state_lock;
		position_t * eoa;
		pthread_mutex_t * eoa_lock;
		int * mission;
		pthread_mutex_t * mission_mutex;
		int chemin_id;
		int can_socket;
		int * initialized;
		pthread_cond_t * init_cond;
		pthread_mutex_t * init_mutex;
	} boucle_automatique_args_t;

	/** 
	 *	@brief Fonction qui tourne en permanence sur l'ATO, afin de faire avancer le train jusqu'a la destion pointée par pos_destination. Actualise aussi la position actuelle en permanence.
	 *  @param args Structure contenant les arguments de la boucle automatique.
	**/
	void * boucle_automatique(void * args);
#endif