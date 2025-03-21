#ifndef _UNIRAIL_EOA_CALCULATOR_H
	#define _UNIRAIL_EOA_CALCULATOR_H
	#include "../../utility/include/map.h"
	#include "../include/trains.h"

	#define DEBUG_EOA 0

	/** 
	 *	@brief Renvoie l'EOA du train num_train, en prenant en compte les autres trains ainsi que la prochaine balise avant une ressource.
	*	@param num_train numero du train entre 0 et 2
	*	@param pos_trains ensemble des trois positions des trains
	*	@param next_balise_avant_ressource numero de la prochaine balise sur le chemin du train avant que celui-ci ne doive demander une ressource
	*	@param chemins ensembles des 3 chemins des 3 trois (composée d'une liste de balise signalant leur ordre)
	*	@param len_chemins longueur des 3 chemins de "chemins"
	*	@return Renvoie l'EOA
	**/
	position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins);
#endif