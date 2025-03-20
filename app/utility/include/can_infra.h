#ifndef _UNIRAIL_CAN_INFRA_H
    #define _UNIRAIL_CAN_INFRA_H
    
    typedef struct {
        int name;
        int dir; //0 si droit, 1 si biais
    } commande_aig;


    extern const commande_aig command_aig_t1_r1[];
    extern const commande_aig command_aig_t1_r4[];
    extern const commande_aig * command_aig_t1[2];

    extern const commande_aig command_aig_t2_r2[];
    extern const commande_aig command_aig_t2_r3[];
    extern const commande_aig command_aig_t2_r4[];
    extern const commande_aig * command_aig_t2[3];

    extern const commande_aig command_aig_t3_r5[];
    extern const commande_aig command_aig_t3_r3[];
    extern const commande_aig command_aig_t3_r4[];
    extern const commande_aig command_aig_t2_r1_2[];
    extern const commande_aig * command_aig_t3[4];

    extern const commande_aig ** all_command_aig[3];

    /// @brief Commande l'ensemble des aiguillages qui sont sensée être modifié pour le passage du train num_train lors du passage de next_bal_index
    /// @param num_train numero du train, entre 0 et 2
    /// @param next_bal_index index du chemin correspondant à la balise qu'on vient de rejoindre
    /// @param can_socket can
    /// @return 1 en cas d'erreur, 0 sinon

    int set_all_switch(int num_train, int next_bal_index, int can_socket);

    int set_switch_straight(int aig_name, int can_socket);

    int set_switch_turn(int aig_name, int can_socket);
#endif
