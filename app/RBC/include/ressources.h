#ifndef _UNIRAIL_GESTION_RESSOURCES_H
	#define _UNIRAIL_GESTION_RESSOURCES_H

	#include <stdio.h>
	#include "../../utility/include/can_infra.h"
	#include "../../utility/include/map.h"

	#define DEBUG_RES 0
	#define DEBUG_AIG 0
	#define DEBUG_RES_FREE 0

	/**
	 * @brief Locks the resources specified by the mask. Blocks until the resources are available.
	 * @param res_mask The mask of the resources to lock
	 */
	void lock_ressources(unsigned char res_mask);

	/**
	 * @brief Unlocks the resources specified by the mask.
	 * @param res_mask The mask of the resources to unlock
	 */
	void unlock_ressources(unsigned char res_mask);

	/**
	 * @brief Asks for resources to be locked. Blocks until the resources are available.
	 * @param next_bal_index The index of the next resource to lock
	 * @param no_train The train number
	 * @param pos_trains The positions of the trains
	 * @param can_socket The CAN socket
	 */
	void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains, int can_socket);

	/**
	 * @brief Frees the resources locked by the train
	 * @param next_bal_index The index of the next resource to lock
	 * @param no_train The train number
	 * @param pos_trains The positions of the trains
	 */
	void free_resources(int * next_bal_index, int no_train, position_t * pos_trains);

	extern unsigned char resources; // 6 bits pour représenter les ressources

#endif