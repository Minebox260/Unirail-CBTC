#include "../include/can.h"
#include "../include/can_infra.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/can.h>


// 1 = turn
// 0 = straight
const commande_aig command_aig_t1_r4[] = {{0x1A2,1},{0x192,1},{0x212,0},{0x202,0},{0x1D2,1},{0x1E2,1}};
const commande_aig command_aig_t1_r1[] = {{0x112,1},{0x102,0}};
const commande_aig * command_aig_t1[] = {command_aig_t1_r4, command_aig_t1_r1};

const commande_aig command_aig_t2_r2[] = {{0x132,0},{0x142,0}};
const commande_aig command_aig_t2_r3[] = {{0x152,0},{0x162,0}};
const commande_aig command_aig_t2_r4[] = {{0x192,0},{0x212,1},{0x1F2,0},{0x1B2,1},{0x182,0}};
const commande_aig * command_aig_t2[] = {command_aig_t2_r2,command_aig_t2_r3,command_aig_t2_r4};

const commande_aig command_aig_t3_r5[] = {{0x1B2,0},{0x1F2,1}};
const commande_aig command_aig_t3_r3[] = {{0x152,1},{0x162,0}};
const commande_aig command_aig_t3_r4[] = {{0x192,0},{0x212,0},{0x202,0},{0x1D2,1},{0x1E2,1}};
const commande_aig command_aig_t2_r1_2[] = {{0x102,0},{0x112,0},{0x122,1},{0x132,1},{0x142,1}};
const commande_aig * command_aig_t3[] = {command_aig_t3_r5,command_aig_t3_r3,command_aig_t3_r4,command_aig_t2_r1_2};

const commande_aig ** all_command_aig[] = {command_aig_t1,command_aig_t2,command_aig_t3};

const int length_bal_1[] = {6,2};
const int length_bal_2[] = {2,2,5};
const int length_bal_3[] = {2,2,5,5};
const int * length_bal[] = {length_bal_1, length_bal_2, length_bal_3};

#define DEBUG_SWITCHES 0

pthread_mutex_t switch_mutex = PTHREAD_MUTEX_INITIALIZER;

/// @brief Commande l'ensemble des aiguillages qui sont sensée être modifié pour le passage du train num_train lors du passage de next_bal_index
/// @param num_train numero du train, entre 0 et 2
/// @param next_bal_index index du chemin correspondant à la balise qu'on vient de rejoindre
/// @param can_socket can
/// @return 1 en cas d'erreur, 0 sinon
int set_all_switch(int num_train, int next_bal_index, int can_socket){
    const commande_aig *list_command = all_command_aig[num_train][next_bal_index];
    const int longueur = length_bal[num_train][next_bal_index];
    int result;

    if (DEBUG_SWITCHES) printf("Setting %d switches for train %d, bal index %d, on socket %d\n", longueur, num_train, next_bal_index, can_socket);
    for(int i = 0; i<longueur; i++){
        commande_aig command = list_command[i];
        pthread_mutex_lock(&switch_mutex);
        if(command.dir == 1){
            if (DEBUG_SWITCHES) printf("Setting %x to turn...", command.name);
            result = set_switch_turn(command.name,can_socket);
            if (DEBUG_SWITCHES) printf("%d\n", result);
        } 
        else {
            if (DEBUG_SWITCHES) printf("Setting %x to straight...", command.name);
            result = set_switch_straight(command.name,can_socket);
            printf("%d\n", result);
        }

        pthread_mutex_unlock(&switch_mutex);
        if(result) return 1;

        usleep(200000);
    }
    return 0;
}

int set_switch_straight(int aig_name, int can_socket){
    struct can_frame frame_response;
    int can_dlc = 3;
    int command_mode = 'W'; // For positioning, L to lock, U to unlock, I to reset
    int turn_mode = 0x00; // For turn, 0x01 for straight
    int data[8] = {command_mode,turn_mode,0,0,0,0,0,0};

    return send_can_data(can_socket, aig_name, can_dlc, data);
    
}

int set_switch_turn(int aig_name, int can_socket){
    int can_dlc = 3;
    int command_mode = 'W'; // For positioning, L to lock, U to unlock, I to reset
    int turn_mode = 0x01; // For straight, 0x00 for turn
    int data[8] = {command_mode,turn_mode,0,0,0,0,0,0};

    return send_can_data(can_socket, aig_name, can_dlc, data);
}
