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
  refcount_set(&ex->refCount, 1); // problem bevause the api cal WARM if refcount are 0 during add or inc
  init_waitqueue_head(&ex->readerQueue);
  return ex;
}

void realExangeFree(exangeRoom *ex) {
  if (ex->mes != NULL)
    vfree(ex->mes);
  kfree(ex);
}

// 2 = i free a exangeRoom just empty
// 1 = i free the exangeRoom
// 0 = i don't free exangeRoom
// -1 = Problem in pointer
int try_freeExangeRoom_exex(exangeRoom *ex, atomic_t *freeLockCount, int execFree) {
  if (ex == NULL || freeLockCount == NULL) {
    printk_tbde("[freeChatRoom] Impossible kfree chatRoom because passing same NULL ptr");
    return -1;
  }

  if (refcount_read(&ex->refCount) == 1) { // è una stanza vuota, posso eliminarla subito
    printk_tbdeDB("[try_freeExangeRoom] free exangeRoom %p, with no reference", ex);
    if (execFree)
      realExangeFree(ex);
    return 1;
  }
  if (refcount_dec_not_one(&ex->refCount)) { // Se non sono l'ultimo
    printk_tbdeDB("[try_freeExangeRoom] Decrease exangeRoom %p, actual refCount = %d", ex,
                  refcount_read(&ex->refCount));
    return 0;
  }
  // Sono l'ultimo, verifico mi sia permesso di fare la free
  waitUntil_unlock(freeLockCount);

  if (refcount_dec_not_one(&ex->refCount)) { // Il reader è entrato qui, ci penserà lui a pulire
    return 0;
  }
  // Sono effettivamente l'ultimo dentro la stanza
  if (execFree)
    realExangeFree(ex); // Se posso cancellare, cancello

  return 1;
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
    arch_atomic_set(&p->lv[i].freeLockCnt, 0);
    p->lv[i].ex = makeExangeRoom();
  }
  return p;
}

void freeRoom(void *data) {
  room *rm;
  int i;
  rm = (room *)data;
  if (rm != NULL) {
    if (refcount_dec_and_test(&rm->refCount)) {
      for (i = 0; i < levelDeep; i++)
        try_freeExangeRoom(rm->lv[i].ex, &rm->lv[i].freeLockCnt);
      printk_tbdeDB("[freeRoom] kfree of %p room", rm);
      kfree(rm);
    } else {
      printk_tbdeDB("[freeRoom] Decrease refCount of %p room, actual refCount = %d", rm, refcount_read(&rm->refCount));
      return;
    }
  } else {
    printk_tbdeDB("[freeRoom] Impossible kfree room because passing NULL ptr");
  }
}

// shuld be used in write-lock context
size_t waitersInRoom(room *p) {
  int i, wLev;
  size_t waiters = 0;
  for (i = 0; i < levelDeep; i++) {
    wLev = refcount_read(&p->lv[i].ex->refCount) - 1; // 1 mean notting inside, but valid memory area
    waiters += wLev;
    printk_tbdeDB("[waitersInRoom] @level = %d had %d waiters reader", i, wLev);
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