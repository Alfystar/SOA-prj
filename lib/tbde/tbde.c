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
      if (ret == NULL)
        break;
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
    }
    while (true) {
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL)
        break;
      p->tag = tagCounting++;
      negativeReset(tagCounting);
      negativeReset(p->tag);
    }
  }
  printk_tbde("New room Create and added to the Searches Tree");
  text = kzalloc(4096, GFP_KERNEL | GFP_NOWAIT);
  len = Tree_Print(tagTree, text, 4096);
  printk("\n%s", text);
  kfree(text);
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
    printk_tbdeDB("Invalid Command");
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

  printk("%s: thread %d call [tag_ctl(%d,%d)]\n", MODNAME, current->pid, tag, command);

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
  p->key = key;
  p->tag = tag;
  p->uid_Creator = uid_Creator;
  p->perm = perm;
  return p;
}

void freeRoom(void *data) {
  room *p;
  p = (room *)data;
  kfree(p);
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

  indexBuf +=
      scnprintf(buf, size, "key=%d | tag=%d | uid_Creator=%d | perm=%d\n", p->key, p->tag, p->uid_Creator, p->perm);
  return indexBuf;
  // todo: implementare il print
}
