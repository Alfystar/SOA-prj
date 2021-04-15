#include "tbde.h"

int permAmmisible(int perm) {
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
exangeRoom *makeExangeRoom(void) {
  exangeRoom *ex;
  ex = kzalloc(sizeof(exangeRoom), GFP_KERNEL | GFP_NOWAIT);
  refcount_set(&ex->refCount, 0);
  init_waitqueue_head(&ex->readerQueue);
  return ex;
}

void realExangeFree(exangeRoom *ex) {
  return;
  if (ex->mes != NULL)
    vfree(ex->mes);
  kfree(ex);
}

// 2 = i free a exangeRoom just empty
// 1 = i free the exangeRoom
// 0 = i don't free exangeRoom
// -1 = Problem in pointer
int try_freeExangeRoom(exangeRoom *ex, atomic_t *freeLockCount) {
  if (ex != NULL && freeLockCount != NULL) {
    if (refcount_read(&ex->refCount) == 0) { // è una stanza vuota, posso eliminarla subito
      // todo: capire se viene mai ragiunta in un momento inaspettato
      printk_tbdeDB("[try_freeExangeRoom] free exangeRoom %p, with no reference", ex);
      realExangeFree(ex);
      return 1;
    }
    if (refcount_dec_not_one(&ex->refCount)) { // Se non sono l'ultimo
      printk_tbdeDB("[try_freeExangeRoom] free exangeRoom %p, LOOP FREE", ex);
      return 0;
    }
    // Sono l'ultimo, verifico mi sia permesso di fare la free
    preempt_disable();
    while (arch_atomic_read(freeLockCount) != 0) { // Simil spinLock busy wait
    };
    preempt_enable();

    if (refcount_dec_not_one(&ex->refCount)) { // Il reader è entrato qui, ci penserà lui a pulire
      return 0;
    }
    // Sono effettivamente l'ultimo e nessuno più può entrare
    realExangeFree(ex);
    return 1;
  } else {
    printk_tbde("[freeChatRoom] Impossible kfree chatRoom because passing same NULL ptr");
    return -1;
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
  for (i = 0; i < levelDeep; i++) {
    arch_atomic_set(&p->level[i].freeLockCount, 0);
    p->level[i].ex = makeExangeRoom();
  }
  return p;
}

void freeRoom(void *data) {
  room *p;
  int i;
  p = (room *)data;
  if (p != NULL) {
    if (refcount_dec_and_test(&p->refCount)) {
      for (i = 0; i < levelDeep; i++)
        try_freeExangeRoom(p->level[i].ex, &p->level[i].freeLockCount);
      kfree(p);
    } else {
      return;
    }
  } else {
    printk_tbdeDB("[freeRoom] Impossible kfree room because passing NULL ptr");
  }
}

// shuld be used in write-lock context
size_t waitersInRoom(room *p) {
  int i;
  size_t waiters = 0;
  for (i = 0; i < levelDeep; i++) {
    waiters += refcount_read(&p->level[i].ex->refCount);
  }
  return waiters;
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