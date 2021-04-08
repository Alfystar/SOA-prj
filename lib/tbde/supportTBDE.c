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
chatRoom *makeChatRoom(void) {
  chatRoom *cr;
  cr = kzalloc(sizeof(chatRoom), GFP_KERNEL | GFP_NOWAIT);
  refcount_set(&cr->refCount, 0);
  init_waitqueue_head(&cr->readerQueue);
  return cr;
}

inline void chatRoomRefLock(chatRoom *cr) { chatRoomRefLock_n(cr, 1); }

void chatRoomRefLock_n(chatRoom *cr, unsigned int n) {
  if (cr) {
    refcount_add(n, &cr->refCount);
    printk_tbdeDB("[chatRoomRefLock_n] refCount or the chatRoom %p new value = %d", cr, refcount_read(&cr->refCount));
  } else {
    printk_tbdeDB("[chatRoomRefLock_n] Impossible increase refCount because passing NULL ptr");
  }
}

void freeChatRoom(chatRoom *cr) {
  if (cr != NULL) {
    if (refcount_dec_and_test(&cr->refCount)) {
      if (cr->mes != NULL)
        vfree(cr->mes);

      wake_up_all(&cr->readerQueue);
      printk_tbdeDB("[freeChatRoom] kfree chatRoom %p executable, remain %d room", cr, refcount_read(&cr->refCount));
      kfree(cr);
    } else {
      printk_tbdeDB("[freeChatRoom] Impossible kfree chatRoom %p because room is pointed %d", cr,
                    refcount_read(&cr->refCount));
    }
  } else {
    printk_tbdeDB("[freeChatRoom] Impossible kfree chatRoom because passing NULL ptr");
  }
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
    p->level[i] = makeChatRoom();
  return p;
}

inline void roomRefLock(room *p) { roomRefLock_n(p, 1); }

void roomRefLock_n(room *p, unsigned int n) {
  if (p) {
    refcount_add(n, &p->refCount);
    printk_tbdeDB("[roomRefLock_n] refCount or the room %p new value = %d", p, refcount_read(&p->refCount));
  } else {
    printk_tbdeDB("[roomRefLock_n] Impossible increase refCount because passing NULL ptr");
  }
}

void freeRoom(void *data) {
  room *p;
  int i;
  p = (room *)data;
  if (p != NULL) {
    if (refcount_dec_and_test(&p->refCount)) {
      for (i = 0; i < levelDeep; i++)
        freeChatRoom(p->level[i]);
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