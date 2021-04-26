#ifndef tbde_h
#define tbde_h

#include <globalDef.h>
#include <stddef.h>

#include <tbdeType.h>

#include "avl.h"
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/param.h>
#include <linux/preempt.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>

#define min_sys_ROOM 256

// Level 0 = no message
// Level 1 = err message
// Level 2 = notice messsage
// Level 3 = Info messsage
// Level 4 = dbg message
// Level 5 = all message
#define TBDE_VerboseLevel 2
#define TBDE_err _codeActive(1, TBDE_VerboseLevel)
#define TBDE_notice _codeActive(2, TBDE_VerboseLevel)
#define TBDE_info _codeActive(3, TBDE_VerboseLevel)
#define TBDE_Db _codeActive(4, TBDE_VerboseLevel)

#define tbde_err(str, ...)                                                                                             \
  do {                                                                                                                 \
    TBDE_err printk_STD(KERN_ERR, "TBDE", str, ##__VA_ARGS__);                                                         \
  } while (0)

#define tbde_notice(str, ...)                                                                                          \
  do {                                                                                                                 \
    TBDE_notice printk_STD(KERN_NOTICE, "TBDE", str, ##__VA_ARGS__);                                                   \
  } while (0)

#define tbde_info(str, ...)                                                                                            \
  do {                                                                                                                 \
    TBDE_info printk_STD(KERN_INFO, "TBDE", str, ##__VA_ARGS__);                                                       \
  } while (0)

#define tbde_db(str, ...)                                                                                              \
  do {                                                                                                                 \
    TBDE_Db printk_STD(KERN_DEBUG, "TBDE", str, ##__VA_ARGS__);                                                        \
  } while (0)

// Exange-zone Metadata
// Until this object is reachable, refCount is the count of reader inside,
// when a writer arrive, the room became unreachable
// The small race condition caused by free before locking is solved using freeLockCount
// int the Chat-room structure
typedef struct exangeRoom_ {
  refcount_t refCount; // Count how many thread aquire this obj
  char *mes;
  size_t len;
  unsigned char ready;
  unsigned char wakeUpALL;
  wait_queue_head_t readerQueue;
} exangeRoom;

// Chat-room Room Metadata
typedef struct chatRoom_ {
  // uso "atomic_t" pochè il ref potrebbe essere a 0 ed è giusto e non voglio warning
  atomic_t freeLockCnt; // To delete the race-condition in refCount inc
  exangeRoom *ex;
} chatRoom;

// Room Metadata
typedef struct room_ {
  refcount_t refCount; // structure point me
  int key;             // indexing with key (if !=-1)
  unsigned int tag;    // indexing with tag
  int uid_Creator;
  int perm;
  chatRoom lv[levelDeep];
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

#define waitUntil_unlock(atomic_lockCount_ptr)                                                                         \
  do {                                                                                                                 \
    preempt_disable();                                                                                                 \
    while (arch_atomic_read(atomic_lockCount_ptr) != 0) {                                                              \
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

// tagCounting && tagTree are module variable
#define roomTagInsert_Force(rm_ptr, roomCount)                                                                         \
  ({                                                                                                                   \
    while (true) {                                                                                                     \
      rm_ptr->tag = positiveAtomic_inc(tagCounting);                                                                   \
      if (Tree_Insert(tagTree, rm_ptr) == NULL) {                                                                      \
        break;                                                                                                         \
      }                                                                                                                \
    }                                                                                                                  \
    __sync_add_and_fetch(&roomCount, 1);                                                                               \
  })

// from the node in the tree, get the room pointer, and increase his ref count
#define treeNode2Room_refInc(trNode)                                                                                   \
  ({                                                                                                                   \
    room *__ret;                                                                                                       \
    __ret = (room *)trNode->data;                                                                                      \
    refcount_inc(&__ret->refCount);                                                                                    \
    __ret;                                                                                                             \
  })

// ritorna l'attuale exengeRoom, sostituendola con una nuova, in maniera atomica
#define createAndSwap_exangeRoom_refInc(ex_ptr)                                                                        \
  ({                                                                                                                   \
    exangeRoom *oldEx;                                                                                                 \
    exangeRoom *newEx = makeExangeRoom();                                                                              \
    do {                                                                                                               \
      oldEx = ex_ptr;                                                                                                  \
    } while (!__sync_bool_compare_and_swap(&ex_ptr, oldEx, newEx));                                                    \
    refcount_inc(&oldEx->refCount);                                                                                    \
    oldEx;                                                                                                             \
  })

#define exangeMessage(ex_ptr, buf, size)                                                                               \
  do {                                                                                                                 \
    ex_ptr->mes = buf;                                                                                                 \
    ex_ptr->len = size;                                                                                                \
    ex_ptr->wakeUpALL = 0;                                                                                             \
    ex_ptr->ready = 1;                                                                                                 \
  } while (0)

#define exangeWakeUpAll(ex_ptr)                                                                                        \
  do {                                                                                                                 \
    ex_ptr->mes = NULL;                                                                                                \
    ex_ptr->len = 0;                                                                                                   \
    ex_ptr->wakeUpALL = 1;                                                                                             \
    ex_ptr->ready = 1;                                                                                                 \
  } while (0)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define data2UserForce(src, dest, bSize)                                                                               \
  ({                                                                                                                   \
    ssize_t offset = 0, noCopy;                                                                                        \
    while (bSize - offset > 0) {                                                                                       \
      noCopy = copy_to_user(dest + offset, src + offset, bSize - offset);                                              \
      offset += (bSize - offset) - noCopy;                                                                             \
    }                                                                                                                  \
  })

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern Tree keyTree, tagTree;
extern rwlock_t searchLock;
extern unsigned int roomCount; // Safe increment thanks searchLock
extern int tagCounting;        // Safe increment thanks searchLock
extern int MAX_ROOM;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int initTBDE(void);     // shuld be call BEFORE installation of syscall
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
int try_freeExangeRoom_exex(exangeRoom *ex, atomic_t *freeLockCount, int execFree);
#define try_freeExangeRoom(exangeRoom_ptr, freeLockCount_ptr)                                                          \
  try_freeExangeRoom_exex(exangeRoom_ptr, freeLockCount_ptr, 1);
#define exitOnly_freeExangeRoom(exangeRoom_ptr, freeLockCount_ptr)                                                     \
  try_freeExangeRoom_exex(exangeRoom_ptr, freeLockCount_ptr, 0);

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

char *tbdeStatusString(size_t *len);
void printTrees(void);

#endif