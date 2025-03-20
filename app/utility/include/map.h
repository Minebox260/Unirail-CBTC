#ifndef _UNIRAIL_MAP_H
	#define _UNIRAIL_MAP_H

	/**
	 * @struct position_t
	 * @brief Represents a position on the rail
	 * @var bal The balise ID
	 * @var pos_r The position relative to the balise on the rail (in cm)
	 */
	typedef struct {
		int bal;
		float pos_r;
	} position_t;

	extern const int B1[];
	extern const int B2[];
	extern const float D[];
	extern const int chemin3[];
	extern const int chemin2[];
	extern const int chemin1[];
	extern const int v_limit1[];
	extern const int v_limit2[];
	extern const int v_limit3[];

	extern const int * v_limits[3];
	extern const int * chemins[3];
	extern const int tailles_chemins[3];

	extern const float max_acceleration; // Acceleration en cm.s-2
	extern const float max_deacceleration; // Acceleration en cm.s-2

	extern const int start_chemins[3];

	/** 
	 *	@brief Calcule la distance entre deux positions sur un même chemin. Une position est définie par sa balise et sa position relative à cette balise.
	*	@param pos_current position de départ
	*	@param pos_destination position d'arrivée
	*	@param chemin_id identifiant du chemin à suivre
	*	@return La distance en cm entre les deux positions.
	**/
	float get_distance(position_t pos_current, position_t pos_destination, const int chemin_id);

#endif