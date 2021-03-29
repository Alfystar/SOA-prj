#ifndef tbde_h
#define tbde_h

#include <globalDef.h>
#include <stddef.h>

#include <tbdeType.h>

#include "avl.h"
#include <linux/errno.h>
#include <linux/syscalls.h>
#include <linux/version.h>

#define MAX_ROOM 256 // todo: renderlo un valore parametrico

#define TBDE_Audit if (1)
#define printk_tbde(str, ...) printk("[%s::%s]: " str, MODNAME, "TBDE", ##__VA_ARGS__)
#define printk_tbdeDB(str, ...) TBDE_Audit printk_tbde(str, ##__VA_ARGS__)

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
typedef struct room_ {
  refcount_t refCount; // structure point me
  int key;             // indexing with key (if !=-1)
  unsigned int tag;    // indexing with tag
  int uid_Creator;
  int perm;
  rcuWQ level[levelDeep];
} room;

void initTBDE(void);    // shuld be call BEFORE installation of syscall
void unmountTBDE(void); // shuld be call AFTER installation of syscall

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
extern unsigned long tag_get;
extern unsigned long tag_send;
extern unsigned long tag_receive;
extern unsigned long tag_ctl;
#else
int tag_get(int key, int command, int permission);
int tag_send(int tag, int level, char *buffer, size_t size);
int tag_receive(int tag, int level, char *buffer, size_t size);
int tag_ctl(int tag, int command);
#endif

int permissionValid(int perm);

room *roomMake(int key, unsigned int tag, int uid_Creator, int perm);
void roomRefLock(room *p);
void freeRoom(void *data);

int tagRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
int keyRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
size_t printRoom(void *data, char *buf, int size);

#endif