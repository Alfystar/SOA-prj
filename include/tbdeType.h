#ifndef tbdeType_h
#define tbdeType_h

// ###### Define Key tag_get ######
// int tag_get(int key, int command, int permission);
enum tag_get_KEY { TBDE_IPC_PRIVATE = -1 };
enum tag_get_command { TBDE_O_CREAT, TBDE_O_OPEN };
enum tag_get_permission { TBDE_OPEN_ROOM, TBDE_PRIVATE_ROOM };
// ################################

// #### Define command tag_ctl ####
// int tag_ctl(int tag, int command);
enum tag_ctl_command { TBDE_AWAKE_ALL, TBDE_REMOVE };
// ################################

// << Module Define >>
#define levelDeep 32 // Max number of sub-rooms

#endif