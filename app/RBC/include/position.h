#ifndef _UNIRAIL_POSITION_H
	#define _UNIRAIL_POSITION_H

	#include "../../utility/include/comm.h"

	/**
	 * @struct position_t
	 * @brief Represents a position on the rail
	 * @var bal The balise ID
	 * @var pos_r The position relative to the balise on the rail
	 */
	typedef struct {
		int bal;
		float pos_r;
	} position_t;
#endif