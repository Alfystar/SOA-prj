#include "tbde.h"

Tree keyTree, tagTree;
rwlock_t searchLock;

// Non necessitano di essere atomiche, crescono solo al crescere dell'albero che
// è già in una sezione critica, gestita da searchLock
unsigned int roomCount;
int tagCounting;

// To restore interrupt mask after critical section
// unsigned long flags;
// READ LOCK API
// read_lock_irqsave(&searchLock, flags);
//.. critical section that only reads the info ...
// read_unlock_irqrestore(&searchLock, flags);
// WRITE LOCK API
// write_lock_irqsave(&searchLock, flags);
//..read and write exclusive access to the info...
// write_unlock_irqrestore(&searchLock, flags);

void initTBDE() {
  tagTree = Tree_New(tagRoomCMP, printRoom, freeRoom);
  keyTree = Tree_New(keyRoomCMP, printRoom, freeRoom);
  roomCount = 0;
  tagCounting = 0;
  rwlock_init(&searchLock);
}

void unmountTBDE() {
  Tree_DelAll(tagTree);
  tagTree = NULL;
  Tree_DelAll(keyTree);
  keyTree = NULL;
}

// Reset to zero if value is negative
#define negativeReset(x)                                                                                               \
  do {                                                                                                                 \
    if (x < 0)                                                                                                         \
      x = 0;                                                                                                           \
  } while (0)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return CREATE:
//  succes            :=    tag value
//  ETOOMANYREFS      :=    Too many room was created
//  EBADRQC           :=    Permission Wrong parameter
//  EBADR             :=    Key already in use
int tag_get_CREATE(int key, int command, int permission) {
  room *p;
  Node ret;
  unsigned long flags;

  if (__sync_add_and_fetch(&roomCount, 0) >= MAX_ROOM) {
    printk_tbdeDB("Impossible Create another room");
    return -ETOOMANYREFS;
  }

  if (permCheck(permission)) {
    printk_tbdeDB("Permission are Invalid");
    return -EBADRQC;
  }

  p = roomMake(key, 0, current->tgid, permission);
  if (p->key == TBDE_IPC_PRIVATE) {
    refcount_inc(&p->refCount); // Lock per conto di tagTree
    write_lock_irqsave(&searchLock, flags);
    while (true) {
      __sync_add_and_fetch(&tagCounting, 1);
      negativeReset(tagCounting);
      p->tag = tagCounting;
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL) {
        break;
      }
    }
    write_unlock_irqrestore(&searchLock, flags);
    __sync_add_and_fetch(&roomCount, 1);
  } else {                         // Key public
    refcount_add(2, &p->refCount); // Lock per conto di keyTree e tagTree

    write_lock_irqsave(&searchLock, flags);
    ret = Tree_Insert(keyTree, p);
    if (ret != NULL) { // Key in use
      write_unlock_irqrestore(&searchLock, flags);
      freeRoom(p); // unLock per conto di keyTree
      freeRoom(p); // unLock per conto di tagTree -> free
      printk_tbdeDB("Impossible to execute, key are just in use");
      return -EBADR;
    }
    // Nodo aggiunto con successo all'albero delle key
    // Sono ancora in sezione critica
    while (true) { // Devo certamente aggiungere il nodo tag
      __sync_add_and_fetch(&tagCounting, 1);
      negativeReset(tagCounting);
      p->tag = tagCounting;
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL) {
        break;
      }
    }
    write_unlock_irqrestore(&searchLock, flags);

    __sync_add_and_fetch(&roomCount, 1);
  }
  printk_tbde("New room Create and added to the Searches Tree");
  // TBDE_Audit
  printTrees();

  return p->tag;
}

// Return OPEN:
//  succes            :=    tag value
//  EBADSLT           :=    asked key is TBDE_IPC_PRIVATE
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENOMSG            :=    Key not found
int tag_get_OPEN(int key, int command, int permission) {
  room roomSearch;
  Node ret;
  unsigned long flags;

  if (key == TBDE_IPC_PRIVATE) {
    printk_tbdeDB("Impossible to execute, the asked key is TBDE_IPC_PRIVATE");
    return -EBADSLT;
  }
  roomSearch.key = key;

  read_lock_irqsave(&searchLock, flags);
  ret = Tree_SearchNode(keyTree, &roomSearch);
  read_unlock_irqrestore(&searchLock, flags);

  if (ret) {
    int tagRet;
    room *p = (room *)ret->data;
    if (!operationValid(p))
      return -EBADRQC;
    tagRet = p->tag;
    return tagRet;
  }
  printk_tbdeDB("No key are found");
  return -ENOMSG;
}

// Return CREATE:
//  succes            :=    tag value
//  ETOOMANYREFS      :=    Too many room was created
//  EBADRQC           :=    Permission Wrong parameter
//  EBADR             :=    Key already in use
// --
// Return OPEN:
//  succes            :=    tag value
//  EBADSLT           :=    asked key is TBDE_IPC_PRIVATE
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENOMSG            :=    Key not found
// --
//  EILSEQ            :=    Command not valid

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
    return -EILSEQ;
    break;
  }
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_get = (unsigned long)__x64_sys_tag_get;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return tag_send:
//  succes            :=    return 0
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE) or no size
//  ENOMSG            :=    Tag not found
//  EBADRQC           :=    Permission invalid to execute the operation
//  EBADSLT           :=    asked level is over levelDeep
// --
//  EILSEQ            :=    Command not valid

// int tag_send(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_send(int tag, int level, char *buffer, size_t size) {
#endif
  room roomSearch, *p;
  exangeRoom *newEx, *oldEx;
  Node ret;
  char *buf;
  unsigned long flags;

  printk("%s: thread %d call [tag_send(%d,%d,%p,%ld)]\n", MODNAME, current->pid, tag, level, buffer, size);

  if (size > MAX_MESSAGE_SIZE && size < 1)
    return -EXFULL;
  buf = vzalloc(size);
  copy_from_user(buf, buffer, size);

  roomSearch.tag = tag;
  read_lock_irqsave(&searchLock, flags);
  ret = Tree_SearchNode(tagTree, &roomSearch);

  if (!ret) {
    read_unlock_irqrestore(&searchLock, flags);
    vfree(buf); // Libero la memoria di appoggio
    return -ENOMSG;
  }
  p = (room *)ret->data;
  refcount_inc(&p->refCount); // da ora ,sicuramente non mi eliminano più la stanza, al massimo non sarà più indicizzata
  read_unlock_irqrestore(&searchLock, flags);

  if (!operationValid(p)) {
    freeRoom(p);
    vfree(buf); // Libero la memoria di appoggio
    return -EBADRQC;
  }
  if (level >= levelDeep) {
    vfree(buf); // Libero la memoria di appoggio
    return -EBADSLT;
  }

  // Operazione valida, essendo un writer creo una stanza vuota
  // e la sostituisco a quella presente che mi interessa

  newEx = makeExangeRoom();
  do {
    oldEx = p->level[level].ex;
  } while (!__sync_bool_compare_and_swap(&p->level[level].ex, oldEx, newEx));
  // Ora oldChat è isolata dal resto del sistema, sono l'unico write dentro

  refcount_inc(&oldEx->refCount);
  oldEx->mes = buf;
  oldEx->len = size;
  oldEx->ready = 1;

  wake_up_all(&oldEx->readerQueue);

  try_freeExangeRoom(oldEx, &p->level[level].freeLockCount); // Libero il mio puntatore
  freeRoom(p);                                               // Libero il mio puntatore
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_send = (unsigned long)__x64_sys_tag_send;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return tag_receive:
//  succes            :=    return len copied
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE) or no size
//  ENOMSG            :=    Tag not found
//  EBADRQC           :=    Permission invalid to execute the operation
//  EBADSLT           :=    asked level is over levelDeep
//  ERESTART       :=    Signal wake_up the thread
// --
//  EILSEQ            :=    Command not valid

// int tag_receive(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_receive(int tag, int level, char *buffer, size_t size) {
#endif

  room roomSearch, *p;
  exangeRoom *curExange;
  Node ret;
  int retWait = 0;
  unsigned long flags;
  size_t bSize, noCopy, copied, offset;

  printk("%s: thread %d call [tag_receive(%d,%d,%p,%ld)]\n", MODNAME, current->pid, tag, level, buffer, size);

  if (size > MAX_MESSAGE_SIZE && size < 1)
    return -EXFULL;

  roomSearch.tag = tag;
  read_lock_irqsave(&searchLock, flags);
  ret = Tree_SearchNode(tagTree, &roomSearch);

  if (!ret) {
    read_unlock_irqrestore(&searchLock, flags);
    return -ENOMSG;
  }
  p = (room *)ret->data;
  refcount_inc(&p->refCount); // da ora ,sicuramente non mi eliminano più la stanza, al massimo non sarà più indicizzata
  read_unlock_irqrestore(&searchLock, flags);

  if (!operationValid(p)) {
    freeRoom(p);
    return -EBADRQC;
  }
  if (level >= levelDeep) {
    return -EBADSLT;
  }

  // Creo un lock con un soft-lock sulle free, alla stanza che mi interessa
  preempt_disable();
  refcount_inc(&p->level[level].freeLockCount); // Impedisco momentaneamente le free sul livello che mi interessa
  curExange = p->level[level].ex;               // In base a quelo attualmente serializzato
  refcount_inc(&curExange->refCount);           // Impedisco la distruzione della mia stanza
  refcount_dec(&p->level[level].freeLockCount); // Ri-abilito le free sul livello che mi interessa
  preempt_enable_notrace();

  retWait = wait_event_interruptible(curExange->readerQueue, __sync_add_and_fetch(&curExange->ready, 0) == 1);

  if (retWait != 0) {                                              // wake_up for signal
    try_freeExangeRoom(curExange, &p->level[level].freeLockCount); // Libero il puntatore
    freeRoom(p);
    return -ERESTART;
  }

  bSize = min(size, curExange->len);
  while (bSize > 0) {
    noCopy = copy_to_user(buffer + offset, curExange->mes + offset, bSize);
    copied = bSize - noCopy;
    offset += copied;
    bSize -= offset;
  }

  try_freeExangeRoom(curExange, &p->level[level].freeLockCount); // Libero il puntatore
  freeRoom(p);
  return bSize;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_receive = (unsigned long)__x64_sys_tag_receive;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Return TBDE_REMOVE:
//  succes            :=    return 0
//  ENOSR             :=    tag negative number
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
int tag_ctl_TBDE_REMOVE(int tag, int command) {
  Node retTag, retKey;
  room *p, searchRoom;
  unsigned long flags;

  printk_tbdeDB("Request TBDE_REMOVE at Tag = %d", tag);
  if (tag == -1)
    return -ENOSR;
  searchRoom.tag = tag;

  write_lock_irqsave(&searchLock, flags);
  retTag = Tree_SearchNode(tagTree, &searchRoom);
  if (retTag) {
    p = (room *)retTag->data;
    printk_tbdeDB("Tag to delete found");
    if (!operationValid(p)) {
      write_unlock_irqrestore(&searchLock, flags);
      return -EBADE;
    }
    p = Tree_DeleteNode(tagTree, retTag);
    if (Tree_SearchNode(tagTree, &searchRoom) != NULL) {
      printk_tbde("!!!! Key wasn't deleted form the key tree !!!!");
    }
    printk_tbde("room %p, will free after tagTree search", p);
    freeRoom(p);

    if (p->key != TBDE_IPC_PRIVATE) { // devo trovare la key
      searchRoom.key = p->key;
      printk_tbdeDB("Now will be search key=%d", p->key);
      retKey = Tree_SearchNode(keyTree, &searchRoom);

      // todo: dopo aver creato la recive, Verificare che la stanza sia senza nessun thread in lettura
      if (retKey) {
        printk_tbdeDB("Tag to delete had a key");
        // freeRoom(Tree_DeleteNode(keyTree, retKey)); // decrease the pointer
        p = Tree_DeleteNode(keyTree, retKey);
        if (Tree_SearchNode(keyTree, &searchRoom) != NULL) {
          printk_tbde("!!!! Key wasn't deleted form the key tree !!!!");
        }
        printk_tbde("room %p, will free after keyTree search", p);
        freeRoom(p);
      } else {
        printk_tbdeDB("ERROR!!! key=%d should be present!!", p->key);
      }
    }
    // libero il lock
    write_unlock_irqrestore(&searchLock, flags);
    __sync_sub_and_fetch(&roomCount, 1);
    printk_tbde("Room (%d) are now Deleted, remaning %d rooms", tag, __sync_add_and_fetch(&roomCount, 0));
    TBDE_Audit printTrees();
    return 0;
  } // if (retTag)
  write_unlock_irqrestore(&searchLock, flags);
  return -ENODATA; // Se non lo trovo, non c'è nulla da eliminare ma lo notifico
}

// Return AWAKE_ALL:
//  succes            :=    return 0
// --
// Return TBDE_REMOVE:
//  succes            :=    return 0
//  ENOSR             :=    tag negative number
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
// --
//  EILSEQ            :=    Command not valid
// int tag_ctl(int tag, int command);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(2, _tag_ctl, int, tag, int, command) {
#else
asmlinkage long tag_ctl(int tag, int command) {
#endif
  printk("%s: thread %d call [tag_ctl(%d,%d)]\n", MODNAME, current->pid, tag, command);
  switch (command) {
  case TBDE_AWAKE_ALL:
    // todo: Implementare dopo aver creato la recive
    break;
  case TBDE_REMOVE:
    return tag_ctl_TBDE_REMOVE(tag, command);
    break;
  default:
    printk_tbdeDB("[tag_ctl] Invalid Command");
    return -EILSEQ;
    break;
  }
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_ctl = (unsigned long)__x64_sys_tag_ctl;
#else
#endif
