#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <linux/can.h>
#include <sys/time.h>

#include "../include/position_calculator.h"
#include "../include/marvelmind.h"


kalman_state_t kalman_state = {0.0, 0.0};
kalman_covariance_t covariance = {1.0, 0.0, 0.0, 1.0};

float Q_pos_r = 0.5;
float Q_speed = 2;

float R_odometry_ratio = 15;
float R_speed = 9.0;

void kalman_predict(float speed_mm, float dt) {
    kalman_state.pos_r += dt * speed_mm;
	//printf("Predicted position: %f, speed: %f\n", kalman_state.pos_r, kalman_state.speed);
    float factor = 10 * dt;
    float newP00 = covariance.P00 + 2 * factor * covariance.P01 + (factor * factor) * covariance.P11 + Q_pos_r;
    float newP01 = covariance.P01 + factor * covariance.P11;
    float newP10 = covariance.P10 + factor * covariance.P11;
    float newP11 = covariance.P11 + Q_speed;
    
    covariance.P00 = newP00;
    covariance.P01 = newP01;
    covariance.P10 = newP10;
    covariance.P11 = newP11;
}

void kalman_update_pos(float odometry_pos_r, float R) {
    float y = odometry_pos_r - kalman_state.pos_r;
    float S = covariance.P00 + R;
    float K0 = covariance.P00 / S;
    float K1 = covariance.P10 / S;
    
    kalman_state.pos_r += K0 * y;
    kalman_state.speed += K1 * y;
    
    float newP00 = (1 - K0) * covariance.P00;
    float newP01 = (1 - K0) * covariance.P01;
    float newP10 = covariance.P10 - K1 * covariance.P00;
    float newP11 = covariance.P11 - K1 * covariance.P01;
    
    covariance.P00 = newP00;
    covariance.P01 = newP01;
    covariance.P10 = newP10;
    covariance.P11 = newP11;
}


void kalman_update_speed(float speed, float R) {
    float y = speed - kalman_state.speed; 
    float S = covariance.P11 + R;
    float K0 = covariance.P01 / S;
    float K1 = covariance.P11 / S;
    
    kalman_state.pos_r += K0 * y;
    kalman_state.speed += K1 * y;
    
    float newP00 = covariance.P00 - K0 * covariance.P01;
    float newP01 = covariance.P01 - K0 * covariance.P11;
    float newP10 = covariance.P10 - K1 * covariance.P01;
    float newP11 = covariance.P11 - K1 * covariance.P11;
    
    covariance.P00 = newP00;
    covariance.P01 = newP01;
    covariance.P10 = newP10;
    covariance.P11 = newP11;
}


int read_speed_from_frame(struct can_frame frame){
	int speed;
	if (frame.data[1] >127)
	{
		frame.data[1] = ~frame.data[1];
		speed = -(frame.data[1]+1);
	}
	else
		speed = frame.data[1];
	return speed;
}

double get_time_sec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

float read_relative_pos_from_frame(struct can_frame frame){
	uint16_t wd = ((uint16_t)frame.data[3] << 8) | frame.data[2];
	float pos = 10*wd*PAS_ROUE_CODEUSE;
	return pos;
}

void * position_calculator(void * args) {
	position_calculator_args_t * pca = (position_calculator_args_t *) args;
	clock_t start, end;
	struct can_frame frame;
	int last_balise;
	int initialized = 0;
	int latest_can_bal;
	float latest_can_pos_r;
	float latest_can_speed;
	double last_timestamp;
	int is_marvelmind_on = !pca->hedge->terminationRequired;
	position_t start_pos;
	float speed_mm;
	double dt;

	start_pos.bal = start_chemins[pca->chemin_id - 1];
	start_pos.pos_r = 0.0;

	struct FusionIMUValue fusionIMU;

	if (is_marvelmind_on) {
		getFusionIMUFromMarvelmindHedge(pca->hedge, &fusionIMU);

		last_timestamp = fusionIMU.timestamp.timestamp64;
		pca->hedge->haveNewValues_ = false;
	} else {
		fprintf(stderr, "ATO/ATP [%d] - Impossible d'utiliser le marvelmind, utilisation de l'odométrie uniquement\n", pca->chemin_id);
		last_timestamp = get_time_sec();
	}

	while (1) {

		if (is_marvelmind_on) {
			if (pca->hedge->terminationRequired) {
				fprintf(stderr, "Perte de connexion avec le marvelmind, utilisation de l'odométrie uniquement\n");
				is_marvelmind_on = 0;
			}
		} else {
			if (!pca->hedge->terminationRequired) {
				fprintf(stderr, "Reconnexion avec le marvelmind\n");
				is_marvelmind_on = 1;
			}
		}

		if (!is_marvelmind_on || (is_marvelmind_on && pca->hedge->haveNewValues_)) {

			if (!initialized) {
				pthread_mutex_lock(pca->state_lock);
				if (pca->state->bal != 0) initialized = 1;
				pthread_mutex_unlock(pca->state_lock);
			} else {

				if (is_marvelmind_on) {
			
					getFusionIMUFromMarvelmindHedge(pca->hedge, &fusionIMU);

					speed_mm = sqrt((double)fusionIMU.vx * (double)fusionIMU.vx + (double)fusionIMU.vy * (double)fusionIMU.vy);
					
					dt = ((double)fusionIMU.timestamp.timestamp64 - last_timestamp) / 1000.0;

					kalman_predict(speed_mm, dt);

					kalman_update_speed(speed_mm / 10.0, R_speed);
					last_timestamp = fusionIMU.timestamp.timestamp64;
				} else {
					double now = get_time_sec();
					dt = (now - last_timestamp);

					kalman_predict(kalman_state.speed * 10.0, dt);

					last_timestamp = now;
				}

				latest_can_bal = 0;
				latest_can_pos_r = 0.0;
				latest_can_speed = 0.0;

				while (read(pca->can_socket, &frame, sizeof(struct can_frame)) > 0) {
					if (frame.can_id == 0x30) { 
						latest_can_bal = frame.data[5];
					} else if (frame.can_id == 0x02F) {
						latest_can_pos_r = read_relative_pos_from_frame(frame);
						latest_can_speed = read_speed_from_frame(frame);
					}
				}

				if (latest_can_bal != 0) {
					if (latest_can_bal != last_balise) {
						if (latest_can_bal == start_pos.bal) {
							// On a fait un tour complet
							pthread_mutex_lock(pca->mission_mutex);
							*(pca->mission) -= 1;
							printf("ATO/ATP [%d] - Tours restants: %d\n", pca->chemin_id, *(pca->mission));
							pthread_mutex_unlock(pca->mission_mutex);
							
						}
						last_balise = latest_can_bal;
						pthread_mutex_lock(pca->state_lock);
						pca->state->bal = latest_can_bal;
						pca->state->pos_r = 0.0;
						pthread_mutex_unlock(pca->state_lock);
					}

					kalman_state.pos_r = 0.0;
					covariance.P00 = 1.0;
					covariance.P01 = 0.0;
					covariance.P10 = 0.0;
				}
				
				if (latest_can_pos_r != 0.0) {

					kalman_update_pos(latest_can_pos_r, R_odometry_ratio * latest_can_pos_r);
					kalman_update_speed(latest_can_speed, R_speed);

					pthread_mutex_lock(pca->state_lock);
					pca->state->pos_r = kalman_state.pos_r;
					pca->state->speed = kalman_state.speed;
					pthread_mutex_unlock(pca->state_lock);

				}
				printf("ATO/ATP [%d] - Speed_mm:[%f] - Dt: [%f] - Position: [%d]: %.2f|%.2f,   %.2f|%.2f\n", pca->chemin_id, speed_mm, dt, pca->state->bal, pca->state->pos_r, latest_can_pos_r, pca->state->speed, latest_can_speed);
			}
			
			if (is_marvelmind_on) pca->hedge->haveNewValues_ = false;
			else usleep(100000);
		}
		
	}
}