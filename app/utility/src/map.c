#include "../include/map.h"
#include <stdio.h>
#include <stdlib.h>

const int B1[] = {1,2,3,3,3,4,5,6,7,8,9,10,11,11,12,13,14,15,15,16,17,12,18,19,20,21,21,22,23,24,25,250,26,251,27,28,29,13,9,9,16,16,16,9,25,11};
const int B2[] = {2,3,4,12,26,5,6,7,8,9,1,11,12,26,13,14,15,16,21,10,11,18,19,20,21,17,22,23,24,25,250,26,251,27,28,29,11,7,17,22,22,1,17,10,20,251};
const float D[] = {1671.0, 1898.0, 2264.0, 2673.0, 3895.0, 1617.0, 1612.0, 2134.0, 1835.0, 1628.0, 2785.0, 2769.0, 2419.0, 3500.0, 2084.0, 2104.0, 1489.0, 1975.0, 2636.0, 3754.0, 2968.0, 2056.0, 1447.0, 2261.0, 2494.0, 2608.0, 3272.0, 1664.0, 1716.0, 1703.0, 643.0, 1165.0, 1025.0, 2015.0, 1580.0, 1903.0, 3008.0, 2628.0, 3416.0, 4089.0, 4242.0, 2943.0, 3570.0, 3620.0, 2543.0 ,2534.8};

const int chemin1[] = {1,2,3,12,13,7,8,9};
const int chemin2[] = {21,17,11,251,26,250,25,20};
const int chemin3[] = {22,23,24,25,250,26,251,27,28,29,11,12,13,7,8,9};

const int v_limit3[] = {40,40,35,40,40,30,40,40,40,30,35,40,30,40,40,25};
const int v_limit2[] = {30,40,40,40,30,40,35,35};
const int v_limit1[] = {40,40,30,40,40,30,40,35};

const int * v_limits[3] = {v_limit1,v_limit2,v_limit3};
const int * chemins[3] = {chemin1, chemin2, chemin3};
const int tailles_chemins[3] = {sizeof(chemin1)/sizeof(int), sizeof(chemin2)/sizeof(int), sizeof(chemin3)/sizeof(int)};

const int start_chemins[3] = {1, 20, 22};

const float max_acceleration = 10.0; // Acceleration en cm.s-2
const float max_deacceleration = -10.0; // Acceleration en cm.s-2
// Calcule la distance entre deux positions sur un même chemin. Les positions sont définies par leurs numéros de balises (int bal1 et bal2) et leurs postions relatives à cette balise (pos_r_1 et pos_r_2)
float get_distance(position_t pos_current, position_t pos_destination, const int chemin_id){
	const int* chemin = chemins[chemin_id - 1];
	const int taille_chemin = tailles_chemins[chemin_id - 1];
	float pos_r_1 = pos_current.pos_r;
	float pos_r_2 = pos_destination.pos_r;
	int bal1 = pos_current.bal;
	int bal2 = pos_destination.bal;
	float distance = pos_r_2 - pos_r_1; //Il est clair qu'on doit ajouter la pos relative à la bal2. De plus dans la suite on calcule la distance entière entre bal1 et la bal suivante. Donc il faut soustraire bal2
	int i = 0; 
	while(chemin[i] != bal1){ //On se place sur la bal1
		i++;
		if (i == taille_chemin) {
			fprintf(stderr, "Balise %d non trouvée dans le chemin %d\n", bal1, chemin_id);
			fflush(stderr);
			exit(EXIT_FAILURE);
		}
	}
	int do_first_loop = 0; // Sert à forcer le processus de faire la premiere loop du while dans certains cas. Cf if suivante
	if(bal1  == bal2 && (pos_r_1 - pos_r_2 >= 50)){ //On verrifie si on a dépassé la pos_2, avec une petite marge d'erreur.
		do_first_loop = 1; //Dans ce cas, on est dans le même canton mais devant notre pos_2. Donc on doit faire un tour entier du circuit. Cela se traduit pas une entrée forcée dans la boucle while lors de la premiere boucle
	}
	while(chemin[i] != bal2 || do_first_loop == 1){ //On continue notre chemin et sommons les distances tant qu'on est pas arrivé à bal2
		int j = 0;
		do_first_loop = 0;
		int next_index = (i==(taille_chemin-1))? 0 : (i+1);
		while((B1[j] != chemin[i] || B2[j] != chemin[next_index]) && (B1[j] != chemin[next_index] || B2[j] != chemin[i])){ //On trouve l'arc qui va de notre balise jusqu'a la prochaine balise
			j++;
		}
		distance += D[j]; //On sommes les distances de balises en balises
		i = next_index;
	}
	return distance;
}