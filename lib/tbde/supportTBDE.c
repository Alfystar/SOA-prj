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
  if (current_cred()->uid.val == 0) // se è il super utente è SEMPRE valido
    return 1;

  switch (p->perm) {
  case TBDE_OPEN_ROOM:
    return 1;
  case TBDE_PRIVATE_ROOM:
    if (p->uid_Creator == current->tgid) // se è il mio processo è valido
      return 1;
    else
      return 0;
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
  if (ex->mes != NULL)
    vfree(ex->mes);
  kfree(ex);
  tbde_info("[realExangeFree] free exangeRoom %p", ex);
}

// 1 = i free the exangeRoom
// 0 = i don't free exangeRoom
// -1 = Problem in pointer
int try_freeExangeRoom_exex(exangeRoom *ex, atomic_t *freeLockCount, int execFree) {
  if (ex == NULL || freeLockCount == NULL) {
    tbde_err("[freeChatRoom] Impossible kfree chatRoom because passing same NULL ptr");
    return -1;
  }

  if (refcount_read(&ex->refCount) == 0) { // è una stanza vuota, posso eliminarla subito
    tbde_db("[try_freeExangeRoom] free exangeRoom %p, with no reference", ex);
    if (execFree)
      realExangeFree(ex);
    return 1;
  }
  if (refcount_dec_not_one(&ex->refCount)) { // Se non sono l'ultimo
    tbde_db("[try_freeExangeRoom] free exangeRoom %p, LOOP FREE", ex);
    return 0;
  }
  // Sono l'ultimo, verifico mi sia permesso di fare la free
  waitUntil_unlock(freeLockCount);

  if (refcount_dec_and_test(&ex->refCount)) { // tolgo il mio riferimento
    if (execFree)
      realExangeFree(ex); // Sono effettivamente l'ultimo
    return 1;
  }
  // Nel frattempo qualcuno è rientrato
  return 0;
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
      tbde_info("[realFreeRoom] kfree of %p room", rm);
      kfree(rm);
    } else {
      tbde_db("[freeRoom] Decrease refCount of %p room, actual refCount = %d", rm, refcount_read(&rm->refCount));
      return;
    }
  } else {
    tbde_err("[freeRoom] Impossible kfree room because passing NULL ptr");
  }
}

// shuld be used in write-lock context
size_t waitersInRoom(room *p) {
  int i, wLev;
  size_t waiters = 0;
  for (i = 0; i < levelDeep; i++) {
    wLev = refcount_read(&p->lv[i].ex->refCount); // 1 mean notting inside, but valid memory area
    waiters += wLev;
    tbde_db("[waitersInRoom] @level = %d had %d waiters reader", i, wLev);
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

#define levelSizeBuf 300
size_t printRoom(void *data, char *buf, int size) {
  room *p;
  char levState[levelSizeBuf];
  size_t indexBuf = 0, indexBufLev = 0;
  int i, waiters, waitersAll = 0;
  p = (room *)data;

  for (i = 0; i < levelDeep; i++) {
    waiters = refcount_read(&p->lv[i].ex->refCount);
    waitersAll += waiters;
    if (waiters > 0)
      indexBufLev += scnprintf(levState + indexBufLev, levelSizeBuf - indexBufLev, "|L%d=%d", i, waiters);
  }
  if (waitersAll == 0)
    indexBufLev += scnprintf(levState + indexBufLev, levelSizeBuf - indexBufLev, "<no Waiters>");
  else
    indexBufLev += scnprintf(levState + indexBufLev, levelSizeBuf - indexBufLev, "| sum = %d", waitersAll);

  // TAG-key TAG-creator TAG-level Waiting-threads
  indexBuf += scnprintf(buf, size, "{@tag(%d)-@key(%d)-->%p} [Creator=%d-perm=%d %s]\n", p->tag, p->key, p,
                        p->uid_Creator, p->perm, levState);
  return indexBuf;
}

// *len := pointer to a desidered maxSize, and at the end are the character writed
char *tbdeStatusString(size_t *len) {
  size_t writeLen = 0;
  char *text = vzalloc(*len);
  writeLen += scnprintf(text, *len, "tagTree:\n");
  writeLen += Tree_Print(tagTree, text + writeLen, *len - writeLen);

  writeLen += scnprintf(text, *len, "\nkeyTree:\n");
  writeLen += Tree_Print(keyTree, text + writeLen, *len - writeLen);
  writeLen++; // the last '/0' character
  *len = writeLen;
  return text;
}

void printTrees() {
  char *text;
  size_t len = 4096;
  text = tbdeStatusString(&len);
  printk(KERN_DEBUG "\n%s", text);
  vfree(text);
}