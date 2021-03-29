#include "tbde.h"

Tree keyTree, tagTree;
unsigned int roomCount; // todo: increase atomico
int tagCounting;        // todo: increase atomico

void initTBDE() {
  tagTree = Tree_New(tagRoomCMP, printRoom, freeRoom);
  keyTree = Tree_New(keyRoomCMP, printRoom, freeRoom);
  roomCount = 0;
  tagCounting = 0;
}

void unmountTBDE() {
  Tree_DelAll(tagTree);
  tagTree = NULL;
  Tree_DelAll(keyTree);
  keyTree = NULL;
}

#define negativeReset(x)                                                                                               \
  do {                                                                                                                 \
    if (x < 0)                                                                                                         \
      x = 0;                                                                                                           \
  } while (0)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int tag_get_CREATE(int key, int command, int permission) {
  room *p;
  Node ret;
  char *text;
  size_t len;
  if (roomCount >= MAX_ROOM) {
    printk_tbdeDB("Impossible Create another room");
    return -ETOOMANYREFS;
  }

  if (permissionValid(permission)) {
    printk_tbdeDB("Permission are Invalid");
    return -EBADRQC;
  }

  if (key == TBDE_IPC_PRIVATE) {
    negativeReset(tagCounting);
    p = roomMake(TBDE_IPC_PRIVATE, tagCounting++, current->tgid, permission);
    while (true) {
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL) {
        roomRefLock(p);
        break;
      }
      p->tag = tagCounting++;
      negativeReset(tagCounting);
      negativeReset(p->tag);
    }
  } else {
    negativeReset(tagCounting);
    p = roomMake(key, tagCounting++, current->tgid, permission);
    ret = Tree_Insert(keyTree, p);
    if (ret != NULL) { // Key in use
      freeRoom(p);
      printk_tbdeDB("Impossible to execute, key are just in use");
      return -EBADR;
    } else {
      roomRefLock(p);
    }

    while (true) {
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL) {
        roomRefLock(p);
        break;
      }
      p->tag = tagCounting++;
      negativeReset(tagCounting);
      negativeReset(p->tag);
    }
  }
  printk_tbde("New room Create and added to the Searches Tree");
  text = vmalloc(4096);
  len = Tree_Print(tagTree, text, 4096);
  printk("\n%s", text);
  vfree(text);
  return p->tag;
}

int tag_get_OPEN(int key, int command, int permission) {
  room roomSearch;
  Node ret;

  if (key == TBDE_IPC_PRIVATE) {
    printk_tbdeDB("Impossible to execute, the asked key are TBDE_IPC_PRIVATE");
    return -EBADRQC;
  }
  roomSearch.key = key;
  ret = Tree_SearchNode(keyTree, &roomSearch);

  if (ret) {
    int tagRet;
    void *data = ret->data;
    tagRet = ((room *)data)->tag;
    return tagRet;
  }
  printk_tbdeDB("No key are found");
  return -ENOMSG;
}
// int tag_get(int key, int command, int permission);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(3, _tag_get, int, key, int, command, int, permission) {
#else
asmlinkage long tag_get(int key, int command, int permission) {
#endif

  printk("%s: thread %d call [tag_get(%d,%d,%d)]\n", MODNAME, current->pid, key, command, permission);

  switch (command) {
  case TBDE_O_CREAT:
    return tag_get_CREATE(key, command, permission);
    break;
  case TBDE_O_OPEN:
    return tag_get_OPEN(key, command, permission);
    break;
  default:
    printk_tbdeDB("[tag_get]Invalid Command");
    return -EBADRQC;
    break;
  }
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_get = (unsigned long)__x64_sys_tag_get;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// int tag_send(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_send(int tag, int level, char *buffer, size_t size) {
#endif

  printk("%s: thread %d call [tag_send(%d,%d,%p,%ld)]\n", MODNAME, current->pid, tag, level, buffer, size);

  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_send = (unsigned long)__x64_sys_tag_send;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// int tag_receive(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_receive(int tag, int level, char *buffer, size_t size) {
#endif

  printk("%s: thread %d call [tag_receive(%d,%d,%p,%ld)]\n", MODNAME, current->pid, tag, level, buffer, size);

  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_receive = (unsigned long)__x64_sys_tag_receive;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// int tag_ctl(int tag, int command);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(2, _tag_ctl, int, tag, int, command) {
#else
asmlinkage long tag_ctl(int tag, int command) {
#endif
  Node retTag, retKey;
  room *p, searchRoom;
  char *text;
  size_t len;

  printk("%s: thread %d call [tag_ctl(%d,%d)]\n", MODNAME, current->pid, tag, command);
  switch (command) {
  case TBDE_AWAKE_ALL:
    // todo: Implementare dopo aver creato la recive
    break;
  case TBDE_REMOVE:
    printk_tbdeDB("Request TBDE_REMOVE at Tag = %d", tag);
    searchRoom.tag = tag;
    retTag = Tree_SearchNode(tagTree, &searchRoom);
    if (retTag) {
      printk_tbdeDB("Tag to delete found");
      p = (room *)retTag->data;
      if (p->key != TBDE_IPC_PRIVATE) {
        searchRoom.key = p->key;
        printk_tbdeDB("Now will be search key=%d", p->key);
        retKey = Tree_SearchNode(keyTree, &searchRoom);
        // todo: dopo aver creato la recive, Verificare che la stanza sia senza nessun thread in lettura
        // Delete boot, keyTree and Tag tree should pointer to the same room
        if (retKey) {
          printk_tbdeDB("Tag to delete had a key");
          freeRoom(Tree_DeleteNode(keyTree, retKey)); // decrease the pointer
        } else {
          printk_tbdeDB("ERROR!!! key=%d should be present!!", p->key);
        }
      }
      freeRoom(Tree_DeleteNode(tagTree, retTag));
      printk_tbdeDB("Room are deleted");
      printk_tbdeDB("Tree are now:");

      text = vmalloc(4096);
      printk_tbdeDB("tagTree:");
      len = Tree_Print(tagTree, text, 4096);
      printk("\n%s", text);
      printk_tbdeDB("keyTree:");
      len = Tree_Print(keyTree, text, 4096);
      printk("\n%s", text);
      vfree(text);
      return 0;
    }               // if (retTag)
    return -ENOMSG; // Se non lo trovo, non c'è nulla da eliminare ma lo notifico
    break;
  default:
    printk_tbdeDB("[tag_ctl] Invalid Command");
    return -EBADRQC;
    break;
  }
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_ctl = (unsigned long)__x64_sys_tag_ctl;
#else
#endif

int permissionValid(int perm) {
  switch (perm) {
  case TBDE_OPEN_ROOM:
  case TBDE_PRIVATE_ROOM:
    return 0;
    break;
  default:
    return -1;
    break;
  }
}

room *roomMake(int key, unsigned int tag, int uid_Creator, int perm) {
  room *p;
  p = kzalloc(sizeof(room), GFP_KERNEL | GFP_NOWAIT);
  refcount_set(&p->refCount, 0);
  p->key = key;
  p->tag = tag;
  p->uid_Creator = uid_Creator;
  p->perm = perm;
  return p;
}

void roomRefLock(room *p) {
  if (p) {
    refcount_inc(&p->refCount);
    printk_tbdeDB("[roomRefLock_Add] refCount new value = %d", refcount_read(&p->refCount));
  } else {
    printk_tbdeDB("[roomRefLock_Add] Impossible increase refCount because passing NULL ptr");
  }
}

void freeRoom(void *data) {
  char buf[512];
  room *p;
  if (data) {
    p = (room *)data;
    if (refcount_dec_and_test(&p->refCount)) {
      printk_tbdeDB("[freeRoom] kfree room %lu", p);
      printRoom(data, buf, 512);
      // kfree(p);
    } else {
      printk_tbdeDB("[freeRoom] Impossible kfree room because room is pointed %d", refcount_read(&p->refCount));
    }
  } else {
    printk_tbdeDB("[freeRoom] Impossible kfree room because passing NULL ptr");
  }
}

int tagRoomCMP(void *a, void *b) { // return -1:a<b | 0:a==b | 1:a>b
  room *nd1 = (room *)a;
  room *nd2 = (room *)b;

  if (nd1->tag < nd2->tag) {
    return -1;
  } else if (nd1->tag > nd2->tag) {
    return +1;
  } else {
    return 0;
  }
}

int keyRoomCMP(void *a, void *b) { // return -1:a<b | 0:a==b | 1:a>b
  room *nd1 = (room *)a;
  room *nd2 = (room *)b;

  if (nd1->key < nd2->key) {
    return -1;
  } else if (nd1->key > nd2->key) {
    return +1;
  } else {
    return 0;
  }
}

size_t printRoom(void *data, char *buf, int size) {
  room *p;
  size_t indexBuf = 0;

  p = (room *)data;
  // TAG-key TAG-creator TAG-level Waiting-threads
  indexBuf += scnprintf(buf, size, "(%d-%d) [Creator=%d-perm=%d] \n", p->tag, p->key, p->uid_Creator, p->perm);
  return indexBuf;
  // todo: implementare il print dei livelli per i driver
}
