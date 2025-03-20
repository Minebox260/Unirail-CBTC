#include "../../EVC/include/automatique.h"
#include "../include/const_chemins.h"

const int L_res_req1[2] = {3,9};
const int L_res_lib1[2] = {12,1};
const int L_mask_req1[2] = {0b01000,0b00001};
const int L_mask_lib1[2] = {0b01000,0b00001};

const int L_res_req2[3] = {21,17,11};
const int L_res_lib2[4] = {17,11,251,20};
const int L_mask_req2[3] = {0b00010,0b00100,0b11000};
const int L_mask_lib2[4] = {0b00010,0b00100,0b01000,0b10000};

const int L_res_req3[4] = {24,29,11,9};
const int L_res_lib3[4] = {27,11,12,22};
const int L_mask_req3[4] = {0b10000,0b00100,0b01000,0b00011};
const int L_mask_lib3[4] = {0b10000,0b00100,0b01000,0b00011};

const int * L_res_req[3] = {L_res_req1, L_res_req2, L_res_req3};
const int * L_res_lib[3] = {L_res_lib1, L_res_lib2, L_res_lib3};
const int * L_mask_req[3] = {L_mask_req1, L_mask_req2, L_mask_req3};
const int * L_mask_lib[3] = {L_mask_lib1, L_mask_lib2, L_mask_lib3};

const int L_res_size_req[3] = {2, 3, 4};
const int L_res_size_lib[3] = {2, 4, 4};