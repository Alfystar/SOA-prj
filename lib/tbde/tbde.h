#ifndef tbde_h
#define tbde_h
// -1073741824
#include <globalDef.h>
#include <stddef.h>

#include <tbdeType.h>

#include "avl.h"
#include <linux/errno.h>
#include <linux/param.h>
#include <linux/preempt.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#define MAX_ROOM 256 // todo: renderlo un valore parametrico

#define TBDE_Audit if (0)
#define printk_tbde(str, ...) printk("[%s::%s]: " str, MODNAME, "TBDE", ##__VA_ARGS__)
#define printk_tbdeDB(str, ...) TBDE_Audit printk_tbde(str, ##__VA_ARGS__)

// Exange-zone Metadata
typedef struct exangeRoom_ {
  refcount_t refCount; // Count how many thread aquire this obj (the small race condition is solved using freeLockCount)
  char *mes;
  size_t len;
  unsigned char ready;

  wait_queue_head_t readerQueue;
} exangeRoom;

// Chat-room Room Metadata
typedef struct chatRoom_ {
  atomic_t freeLockCount; // To delete the race-condition in refCount inc
  exangeRoom *ex;
} chatRoom;

// Room Metadata
typedef struct room_ {
  refcount_t refCount; // structure point me
  int key;             // indexing with key (if !=-1)
  unsigned int tag;    // indexing with tag
  int uid_Creator;
  int perm;
  chatRoom level[levelDeep];
} room;

extern Tree keyTree, tagTree;
extern rwlock_t searchLock;

// Non necessitano di essere atomiche, crescono solo al crescere dell'albero che
// è già in una sezione critica, gestita da searchLock
extern unsigned int roomCount;
extern int tagCounting;

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

int permCheck(int perm);
int operationValid(room *p);

exangeRoom *makeExangeRoom(void);
int try_freeExangeRoom(exangeRoom *ex, atomic_t *freeLockCount);

room *roomMake(int key, unsigned int tag, int uid_Creator, int perm);
void freeRoom(void *data);

// Function pointer for tree prototipe
int tagRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
int keyRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
size_t printRoom(void *data, char *buf, int size);

void printTrees(void);

#endif