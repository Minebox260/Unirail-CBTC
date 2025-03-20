#include "../../utility/include/can.h"
#include "../../utility/include/can_infra.h"
#include <unistd.h>

int main(){
    int RC = 0x1F2;

    int can_socket = init_can_socket();
    
    set_switch_straight(RC, can_socket);

    sleep(3);

    set_switch_turn(RC, can_socket);

    sleep(3);

    set_switch_straight(RC, can_socket);

}
