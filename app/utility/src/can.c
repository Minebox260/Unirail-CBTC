#include <arpa/inet.h>
#include <linux/can.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../include/can.h"
#include "../include/debug.h"

// These files were initially included but dont seem useful :
// #include <fcntl.h>
// #include <linux/can/raw.h>
// #include <stdlib.h>
// #include <sys/socket.h>
// #include <time.h>

int send_can_data(int can_socket, int can_id, int can_dlc, int data[8]){
    struct can_frame frame;

    frame.can_id = can_id;  // Identifiant du message CAN
    frame.can_dlc = can_dlc;    // Taille des données 
    for (int i = 0;i<8;i++){
        frame.data[i] = data[i];
    }

    // Envoyer le message
    if (write(can_socket, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("write");
        return 1;
    }	

    DEBUG_PRINT("Message CAN envoyé avec succès sur can0\n");

    return 0;
}


int init_can_socket(){
	struct sockaddr_can addr;
	struct ifreq ifr;
	int sock;
	// Ouvrir le socket CAN
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket CAN");
        return -1;
    }

    // Récupérer l'interface can0
    strncpy(ifr.ifr_name, "can0", IFNAMSIZ);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        return -1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Lier le socket à l'interface CAN
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
	return sock;
}