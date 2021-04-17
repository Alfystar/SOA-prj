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

#define TBDE_Audit if (1)
#define printk_tbde(str, ...) printk(KERN_INFO "[%s::%s]: " str, MODNAME, "TBDE", ##__VA_ARGS__)
#define printk_tbdeDB(str, ...) TBDE_Audit printk_tbde(str, ##__VA_ARGS__)

// Exange-zone Metadata
typedef struct exangeRoom_ {
  refcount_t refCount; // Count how many thread aquire this obj (the small race condition is solved using freeLockCount)
  char *mes;
  size_t len;
  unsigned char ready;
  unsigned char wakeUpALL;

  wait_queue_head_t readerQueue;
} exangeRoom;

// Chat-room Room Metadata
typedef struct chatRoom_ {
  // uso "atomic_t" pochè il ref potrebbe essere a 0 ed è giusto e non voglio warning
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Custom lock define
#define freeMem_Lock(atomic_freeLockCount_ptr)                                                                         \
  do {                                                                                                                 \
    preempt_disable();                                                                                                 \
    atomic_inc(atomic_freeLockCount_ptr);                                                                              \
  } while (0)

#define freeMem_unLock(atomic_freeLockCount_ptr)                                                                       \
  do {                                                                                                                 \
    atomic_dec(atomic_freeLockCount_ptr);                                                                              \
    preempt_enable();                                                                                                  \
  } while (0)

// preempt_enable_no_resched();

#define waitUntil_unlock(atomic_lockCount)                                                                             \
  do {                                                                                                                 \
    preempt_disable();                                                                                                 \
    while (arch_atomic_read(atomic_lockCount) != 0) {                                                                  \
    };                                                                                                                 \
    preempt_enable();                                                                                                  \
  } while (0)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// After add, if num is negative, remove the sign bit
#define positiveAtomic_inc(count)                                                                                      \
  ({                                                                                                                   \
    typeof(count) mask, __ret;                                                                                         \
    __ret = __sync_add_and_fetch(&count, 1);                                                                           \
    if (__ret < 0) {                                                                                                   \
      mask = ~(1 << ((sizeof(count) * 8) - 1));                                                                        \
      __ret = __sync_and_and_fetch(&count, mask);                                                                      \
    }                                                                                                                  \
    __ret;                                                                                                             \
  })

#define roomTagInsert_Force(rm)                                                                                        \
  while (true) {                                                                                                       \
    rm->tag = positiveAtomic_inc(tagCounting);                                                                         \
    ret = Tree_Insert(tagTree, p);                                                                                     \
    if (ret == NULL) {                                                                                                 \
      break;                                                                                                           \
    }                                                                                                                  \
  }

#define treeNode2Room_refInc(trNode)                                                                                   \
  ({                                                                                                                   \
    room *__ret;                                                                                                       \
    __ret = (room *)trNode->data;                                                                                      \
    refcount_inc(&__ret->refCount);                                                                                    \
    __ret;                                                                                                             \
  })

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern Tree keyTree, tagTree;
extern rwlock_t searchLock;
extern unsigned int roomCount; // Safe increment thanks searchLock
extern int tagCounting;        // Safe increment thanks searchLock
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

int permAmmisible(int perm);
int operationValid(room *p);

exangeRoom *makeExangeRoom(void);
int try_freeExangeRoom(exangeRoom *ex, atomic_t *freeLockCount);

room *roomMake(int key, unsigned int tag, int uid_Creator, int perm);
void freeRoom(void *data);
#define doobleFreeRoom(rm)                                                                                             \
  do {                                                                                                                 \
    freeRoom(rm);                                                                                                      \
    freeRoom(rm);                                                                                                      \
  } while (0)

size_t waitersInRoom(room *p); // shuld be used in write-lock context

// Function pointer for tree prototipe
int tagRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
int keyRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
size_t printRoom(void *data, char *buf, int size);

void printTrees(void);

#endif