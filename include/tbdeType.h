
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

// Chat-room Room Metadata
typedef struct WQentry_ {
  // currentWQ*
  // AtomicReaderCount
} WQentry;

typedef struct rcuWQ_ {
  WQentry *currentWQ;
  // Spinlock Writer
} rcuWQ;

// Room Metadata
typedef struct avl_tag_entry_ {
  int key;
  int tag;
  int uid_Creator;
  int perm;
  rcuWQ level[levelDeep];
} avl_tag_entry;

// Public Key Metadata
typedef struct avl_key_entry_ {
  int key;
  avl_tag_entry *tagNode;
} avl_key_entry;

// Tag AVL structure
// typedef struct _avlTag {
//  avl_tag_entry *root;
//  int nTag;
//} avlTag;

// Tag Key structure
// typedef struct _avlKey {
//  avl_key_entry *root;
//  int nKey;
//} avlKey;

#endif