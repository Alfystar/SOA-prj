#include "tbde.h"

int permCheck(int perm) {
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

int operationValid(room *p) {
  switch (p->perm) {
  case TBDE_OPEN_ROOM:
    return 1;
  case TBDE_PRIVATE_ROOM:
    if (p->uid_Creator != current->tgid)
      return 0;
    else
      return 1;
  default:
    return 0;
    break;
  }
}
void makeChatRoom(chatRoom *cr) {
  cr->mes = NULL;
  cr->len = 0;
}

room *roomMake(int key, unsigned int tag, int uid_Creator, int perm) {
  room *p;
  int i;
  p = kzalloc(sizeof(room), GFP_KERNEL | GFP_NOWAIT);
  refcount_set(&p->refCount, 0);
  p->key = key;
  p->tag = tag;
  p->uid_Creator = uid_Creator;
  p->perm = perm;
  for (i = 0; i < levelDeep; i++)
    makeChatRoom(&p->level[i]);
  return p;
}

inline void roomRefLock(room *p) { roomRefLock_n(p, 1); }

void roomRefLock_n(room *p, unsigned int n) {
  if (p) {
    refcount_add(n, &p->refCount);
    printk_tbdeDB("[roomRefLock_Add] refCount or the room %p new value = %d", p, refcount_read(&p->refCount));
  } else {
    printk_tbdeDB("[roomRefLock_Add] Impossible increase refCount because passing NULL ptr");
  }
}

void freeChatRoom(chatRoom *cr) {
  if (cr->mes != NULL)
    vfree(cr->mes);
}

void freeRoom(void *data) {
  room *p;
  int i;
  p = (room *)data;
  if (p != NULL) {
    if (refcount_dec_and_test(&p->refCount)) {
      for (i = 0; i < levelDeep; i++)
        freeChatRoom(&p->level[i]);
      printk_tbdeDB("[freeRoom] kfree room %p executable, remain %d room", p, refcount_read(&p->refCount));
      kfree(p);
    } else {
      printk_tbdeDB("[freeRoom] Impossible kfree room %p because room is pointed %d", p, refcount_read(&p->refCount));
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
  indexBuf += scnprintf(buf, size, "(%d-%d) [Creator=%d-perm=%d] {%p} \n", p->tag, p->key, p->uid_Creator, p->perm, p);
  return indexBuf;
  // todo: implementare il print dei livelli per i driver
}

void printTrees() {
  char *text;
  size_t len;
  text = vzalloc(4096);
  printk_tbde("tagTree:");
  len = Tree_Print(tagTree, text, 4096);
  printk("\n%s", text);
  printk_tbde("keyTree:");
  len = Tree_Print(keyTree, text, 4096);
  printk("\n%s", text);
  vfree(text);
}