#include "tbde.h"

Tree keyTree, tagTree;
rwlock_t searchLock;

// Non necessitano di essere atomiche, crescono solo al crescere dell'albero che
// è già in una sezione critica, gestita da searchLock
unsigned int roomCount;
int tagCounting;

int MAX_ROOM;
module_param(MAX_ROOM, int, 0444); // only to read

ssize_t var_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
  tbde_db("[var_show]");
  return sprintf(buf, "%d\n", MAX_ROOM);
}

ssize_t var_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
  unsigned int passed;
  tbde_db("[var_store] @count = %ld", count);
  sscanf(buf, "%du", &passed);
  if (passed >= min_sys_ROOM && passed >= roomCount)
    MAX_ROOM = passed;
  return count;
}
struct kobject *MAX_ROOM_kobj;
struct kobj_attribute kobj_attr_MAX_ROOM = __ATTR(MAX_ROOM, 0664, var_show, var_store); // for read and write
int initTBDE() {
  int error = 0;
  tbde_info("Initialization MAX_ROOM kobj\n");

  // the kernel_kobj variable points to the /sys/kernel/MODNAME object
  MAX_ROOM_kobj = kobject_create_and_add(MODNAME, kernel_kobj);
  if (!MAX_ROOM_kobj)
    return -ENOMEM;

  error = sysfs_create_file(MAX_ROOM_kobj, &kobj_attr_MAX_ROOM.attr); // actually creating an attribute

  if (error) {
    tbde_info("failed to create the target kobj\n");
    return error;
  }

  tagTree = Tree_New(tagRoomCMP, printRoom, freeRoom);
  keyTree = Tree_New(keyRoomCMP, printRoom, freeRoom);
  roomCount = 0;
  tagCounting = 0;
  rwlock_init(&searchLock);
  MAX_ROOM = min_sys_ROOM;
  return 0;
}

void unmountTBDE() {
  kobject_put(MAX_ROOM_kobj);
  Tree_DelAll(tagTree);
  tagTree = NULL;
  Tree_DelAll(keyTree);
  keyTree = NULL;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return CREATE:
//  succes            :=    tag value
//  ETOOMANYREFS      :=    Too many room was created
//  EBADR             :=    Key already in use
int tag_get_CREATE(int key, int command, int permission) {
  room *rm;
  unsigned long flags;

  if (__sync_add_and_fetch(&roomCount, 0) >= MAX_ROOM) {
    tbde_err("[tag_get_CREATE] Impossible Create another room");
    return -ETOOMANYREFS;
  }

  rm = roomMake(key, 0, current->tgid, permission);

  if (rm->key == TBDE_IPC_PRIVATE) {
    refcount_set(&rm->refCount, 1); // Lock per conto di tagTree
    tbde_db("[tag_get_CREATE a] room %p, actual refCount = %d", rm, refcount_read(&rm->refCount));

    write_lock_irqsave(&searchLock, flags);
    roomTagInsert_Force(rm, roomCount);
    write_unlock_irqrestore(&searchLock, flags);

  } else { // Key public

    refcount_set(&rm->refCount, 2); // Lock per conto di keyTree e tagTree
    tbde_db("[tag_get_CREATE a] room %p, actual refCount = %d", rm, refcount_read(&rm->refCount));
    write_lock_irqsave(&searchLock, flags);
    if (Tree_Insert(keyTree, rm) != NULL) { // Key in use
      write_unlock_irqrestore(&searchLock, flags);
      tbde_db("[tag_get_CREATE] Impossible to execute, key are just in use");
      doobleFreeRoom(rm); // unLock per conto di keyTree e per conto di tagTree -> free
      return -EBADR;
    }
    // Nodo aggiunto con successo all'albero delle key
    // Sono ancora in sezione critica
    roomTagInsert_Force(rm, roomCount);
    write_unlock_irqrestore(&searchLock, flags);
  }
  tbde_info("[tag_get_CREATE] New room Create and added to the Searches Tree");
  TBDE_Db printTrees();
  return rm->tag;
}

// Return OPEN:
//  succes            :=    tag value
//  EBADSLT           :=    asked key is TBDE_IPC_PRIVATE
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENOMSG            :=    Key not found
int tag_get_OPEN(int key, int command, int permission) {
  Node nodeSearch;
  room roomSearch;
  unsigned long flags;

  if (key == TBDE_IPC_PRIVATE) {
    tbde_db("[tag_get_OPEN] Impossible to execute, the asked key is TBDE_IPC_PRIVATE");
    return -EBADSLT;
  }
  roomSearch.key = key;

  read_lock_irqsave(&searchLock, flags);
  nodeSearch = Tree_SearchNode(keyTree, &roomSearch);
  if (nodeSearch) {
    int tagRet;
    room *p = treeNode2Room_refInc(nodeSearch);
    read_unlock_irqrestore(&searchLock, flags);

    if (!operationValid(p)) {
      freeRoom(p);
      return -EBADRQC;
    }

    tagRet = p->tag;
    freeRoom(p);
    return tagRet;
  }
  read_unlock_irqrestore(&searchLock, flags);
  tbde_db("[tag_get_OPEN] No key are found");
  return -ENOMSG;
}

// Return CREATE:
//  succes            :=    tag value
//  ETOOMANYREFS      :=    Too many room was created
//  EBADR             :=    Key already in use
// --
// Return OPEN:
//  succes            :=    tag value
//  EBADSLT           :=    asked key is TBDE_IPC_PRIVATE
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENOMSG            :=    Key not found
// --
//  ENOSYS            :=    sysCall removed
//  EBADRQC           :=    Permission Wrong parameter
//  EILSEQ            :=    Command not valid

// int tag_get(int key, int command, int permission);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(3, _tag_get, int, key, int, command, int, permission) {
#else
asmlinkage long tag_get(int key, int command, int permission) {
#endif
  int ret;
  if (!try_module_get(THIS_MODULE))
    return -ENOSYS;

  tbde_info("[tag_get] thread %d call [tag_get(%d,%d,%d)]\n", current->pid, key, command, permission);

  if (permAmmisible(permission)) {
    tbde_db("[tag_get] Permission passed are Invalid");
    module_put(THIS_MODULE);
    return -EBADRQC;
  }

  switch (command) {
  case TBDE_O_CREAT:
    ret = tag_get_CREATE(key, command, permission);
    break;
  case TBDE_O_OPEN:
    ret = tag_get_OPEN(key, command, permission);
    break;
  default:
    tbde_db("[tag_get] Invalid Command");
    ret = -EILSEQ;
    break;
  }
  module_put(THIS_MODULE);
  return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_get = (unsigned long)__x64_sys_tag_get;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return tag_send:
//  succes            :=    return 0
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE), or not present
//  ENODATA           :=    Problem to transmit data to the sub-system
//  ENOMSG            :=    Tag not found
//  EBADRQC           :=    Permission invalid to execute the operation
//  EBADSLT           :=    asked level is over levelDeep
// --
//  ENOSYS            :=    sysCall removed
//  EILSEQ            :=    Command not valid

// int tag_send(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_send(int tag, int level, char *buffer, size_t size) {
#endif
  Node nodeSearch;
  room roomSearch, *rm;
  exangeRoom *exRoom;
  unsigned long flags;
  char *buf;

  if (!try_module_get(THIS_MODULE))
    return -ENOSYS;

  tbde_info("[tag_send] thread %d call [tag_send(%d,%d,%p,%ld)]\n", current->pid, tag, level, buffer, size);

  if (size > MAX_MESSAGE_SIZE || buffer == NULL) {
    module_put(THIS_MODULE);
    return -EXFULL;
  }
  buf = vzalloc(size);
  if (copy_from_user(buf, buffer, size) != 0) {
    tbde_err("[tag_send] impossible copy buffer from sender");
    return -ENODATA;
  }

  roomSearch.tag = tag;

  read_lock_irqsave(&searchLock, flags);
  nodeSearch = Tree_SearchNode(tagTree, &roomSearch);
  if (!nodeSearch) {
    read_unlock_irqrestore(&searchLock, flags);
    vfree(buf); // Libero la memoria di appoggio
    module_put(THIS_MODULE);
    return -ENOMSG;
  }
  rm = treeNode2Room_refInc(nodeSearch);
  read_unlock_irqrestore(&searchLock, flags);

  if (!operationValid(rm)) {
    freeRoom(rm);
    vfree(buf); // Libero la memoria di appoggio
    module_put(THIS_MODULE);
    return -EBADRQC;
  }
  if (level >= levelDeep) {
    freeRoom(rm);
    vfree(buf); // Libero la memoria di appoggio
    module_put(THIS_MODULE);
    return -EBADSLT;
  }

  tbde_db("[tag_send] Making new exange room ...");
  exRoom = createAndSwap_exangeRoom_refInc(rm->lv[level].ex); // exRoom è isolata dal resto del sistema
  exangeMessage(exRoom, buf, size);

  tbde_db("[tag_send] Wake_upping readers ...");
  wake_up_all(&exRoom->readerQueue);

  try_freeExangeRoom(exRoom, &rm->lv[level].freeLockCnt); // Libero il mio puntatore
  freeRoom(rm);                                           // Libero il mio puntatore
  tbde_db("[tag_send] Ending ...");
  module_put(THIS_MODULE);
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_send = (unsigned long)__x64_sys_tag_send;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return tag_receive:
//  succes            :=    return len copied
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE), or not present
//  ENOMSG            :=    Tag not found
//  EBADRQC           :=    Permission invalid to execute the operation
//  EBADSLT           :=    asked level is over levelDeep
//  ERESTART          :=    Signal wake_up the thread
//  EUCLEAN           :=    Receved TBDE_AWAKE_ALL
// --
//  ENOSYS            :=    sysCall removed
//  EILSEQ            :=    Command not valid

// int tag_receive(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_receive(int tag, int level, char *buffer, size_t size) {
#endif

  room roomSearch, *rm;
  exangeRoom *curExange;
  Node ret;
  int retWait;
  unsigned long flags;
  size_t bSize; //, noCopy, offset;

  if (!try_module_get(THIS_MODULE))
    return -ENOSYS;

  tbde_info("[tag_receive] thread %d call [tag_receive(%d,%d,%p,%ld)]\n", current->pid, tag, level, buffer, size);

  if (size > MAX_MESSAGE_SIZE || buffer == NULL) {
    module_put(THIS_MODULE);
    return -EXFULL;
  }

  roomSearch.tag = tag;

  read_lock_irqsave(&searchLock, flags);
  ret = Tree_SearchNode(tagTree, &roomSearch);
  if (!ret) {
    read_unlock_irqrestore(&searchLock, flags);
    module_put(THIS_MODULE);
    return -ENOMSG;
  }
  rm = treeNode2Room_refInc(ret);
  read_unlock_irqrestore(&searchLock, flags);

  if (!operationValid(rm)) {
    freeRoom(rm);
    module_put(THIS_MODULE);
    return -EBADRQC;
  }
  if (level >= levelDeep) {
    freeRoom(rm);
    module_put(THIS_MODULE);
    return -EBADSLT;
  }

  // Creo un lock con un soft-lock sulle free, alla stanza che mi interessa
  tbde_db("[tag_receive] free disable lock ...");

  freeMem_Lock(&rm->lv[level].freeLockCnt);
  curExange = rm->lv[level].ex; // In base a quelo attualmente serializzato
  atomic_inc(&curExange->refCount.refs);
  freeMem_unLock(&rm->lv[level].freeLockCnt);

  tbde_db("[tag_receive] enqueuing 1 ..."); // mostrato prima del wake_up
  tbde_db("[tag_receive] enqueuing 2 ..."); // mostrato dopo del wake_up (?)
  retWait = wait_event_interruptible(curExange->readerQueue, __sync_add_and_fetch(&curExange->ready, 0) == 1);
  tbde_db("[tag_receive] get upped"); // mostrato dopo del wake_up (?)

  // wake_up for signal
  if (retWait == -ERESTARTSYS) {
    // mi tolgo dalla coda, SENZA eliminare la stanza
    exitOnly_freeExangeRoom(curExange, &rm->lv[level].freeLockCnt);
    freeRoom(rm);
    tbde_notice("[tag_receive] Wake_upped by a signal: sig[0] is 0x%08lx, sig[1] is 0x%08lx\n",
                current->pending.signal.sig[0], current->pending.signal.sig[1]);
    module_put(THIS_MODULE);
    return -ERESTART;
  }

  if (__sync_add_and_fetch(&curExange->wakeUpALL, 0) == 1) {   // Recived wakeUpAll form tag_ctl
    try_freeExangeRoom(curExange, &rm->lv[level].freeLockCnt); // Libero il puntatore
    freeRoom(rm);
    tbde_db("[tag_receive] Recived wakeUpAll form tag_ctl");
    module_put(THIS_MODULE);
    return -EUCLEAN;
  }

  tbde_db("[tag_receive] Data sending ...");
  bSize = min(size, curExange->len);
  data2UserForce(curExange->mes, buffer, bSize);
  try_freeExangeRoom(curExange, &rm->lv[level].freeLockCnt); // Libero il puntatore
  freeRoom(rm);
  tbde_db("[tag_receive] Return data copied");
  module_put(THIS_MODULE);
  return bSize;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_receive = (unsigned long)__x64_sys_tag_receive;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Return TBDE_AWAKE_ALL:
//  succes            :=    return 0
//  ENODATA           :=    Notting to be wake_up was found (all ok, just notification)
//  EBADE             :=    Permission invalid to execute the operation
int tag_ctl_TBDE_AWAKE_ALL(int tag, int command) {
  Node retTagSearch;
  room *rm, searchRoom;
  exangeRoom *exLev;
  unsigned long flags;
  int i;

  tbde_info("[tag_ctl_TBDE_AWAKE_ALL] Request TBDE_AWAKE_ALL at Tag = %d", tag);

  searchRoom.tag = tag;

  write_lock_irqsave(&searchLock, flags);
  retTagSearch = Tree_SearchNode(tagTree, &searchRoom);
  if (!retTagSearch) {
    write_unlock_irqrestore(&searchLock, flags);
    return -ENODATA; // Se non lo trovo, non c'è nulla da eliminare ma lo notifico
  }

  rm = treeNode2Room_refInc(retTagSearch);
  tbde_db("[tag_ctl_TBDE_AWAKE_ALL] Tag to delete found");
  if (!operationValid(rm)) {
    freeRoom(rm);
    write_unlock_irqrestore(&searchLock, flags);
    return -EBADE;
  }

  tbde_db("[tag_ctl_TBDE_REMOVE] Making new exanges rooms ...");
  for (i = 0; i < levelDeep; i++) {
    exLev = createAndSwap_exangeRoom_refInc(rm->lv[i].ex); // exLev è isolata dal resto del sistema
    exangeWakeUpAll(exLev);

    tbde_db("[tag_ctl_TBDE_REMOVE] Wake_upping readers on level %d ...", i);
    wake_up_all(&exLev->readerQueue);

    try_freeExangeRoom(exLev, &rm->lv[i].freeLockCnt); // Libero il mio puntatore
  }
  freeRoom(rm); // Libero il mio puntatore
  tbde_info("[tag_ctl_TBDE_REMOVE] All rooms wake_upped");
  write_unlock_irqrestore(&searchLock, flags);
  return 0;
}

// Return TBDE_REMOVE:
//  succes            :=    return 0
//  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
//  EBADE             :=    Permission invalid to execute the operation
//  EADDRINUSE        :=    Reader in wait on some level
int tag_ctl_TBDE_REMOVE(int tag, int command) {
  Node retTag, retKey;
  room *rm, searchRoom;
  unsigned long flags;
  int waiters;

  tbde_info("[tag_ctl_TBDE_REMOVE] Request TBDE_REMOVE at Tag = %d", tag);

  searchRoom.tag = tag;

  write_lock_irqsave(&searchLock, flags);
  retTag = Tree_SearchNode(tagTree, &searchRoom);
  if (!retTag) {
    write_unlock_irqrestore(&searchLock, flags);
    return -ENODATA; // Se non lo trovo, non c'è nulla da eliminare ma lo notifico
  }
  rm = treeNode2Room_refInc(retTag);
  tbde_db("[tag_ctl_TBDE_REMOVE] Tag to delete found");
  if (!operationValid(rm)) {
    freeRoom(rm);
    write_unlock_irqrestore(&searchLock, flags);
    tbde_db("[tag_ctl_TBDE_REMOVE] Tag to delete permission invalid");
    return -EBADE;
  }

  waiters = waitersInRoom(rm);
  if (waiters != 0) {
    freeRoom(rm);
    write_unlock_irqrestore(&searchLock, flags);
    tbde_db("[tag_ctl_TBDE_REMOVE] Tag to delete had %d waiters reader", waiters);
    return -EADDRINUSE;
  }

  tbde_db("[tag_ctl_TBDE_REMOVE] room %p freeing... (tagTree search)", rm);
  Tree_DeleteNode(tagTree, retTag);
  freeRoom(rm); // free on account of tagTree

  if (rm->key != TBDE_IPC_PRIVATE) { // devo trovare la key
    searchRoom.key = rm->key;
    tbde_db("[tag_ctl_TBDE_REMOVE] Now will be search key=%d", rm->key);

    retKey = Tree_SearchNode(keyTree, &searchRoom);
    if (retKey) {
      tbde_db("[tag_ctl_TBDE_REMOVE] room %p freeing... (keyTree search)", rm);
      Tree_DeleteNode(keyTree, retKey);
      freeRoom(rm); // free on account of keyTree
    } else {
      tbde_err("[tag_ctl_TBDE_REMOVE] ERROR!!! key=%d should be present!!", rm->key);
    }
  }
  __sync_sub_and_fetch(&roomCount, 1);
  write_unlock_irqrestore(&searchLock, flags); // libero il lock

  freeRoom(rm); // free for my personal lock (after tagTree & keyTree search && remote)
  tbde_info("[tag_ctl_TBDE_REMOVE] Room (%d) are now Deleted, remaning %d rooms", tag,
            __sync_add_and_fetch(&roomCount, 0));
  TBDE_Db printTrees();
  return 0;
}

// Return TBDE_AWAKE_ALL:
//  succes            :=    return 0
//  ENODATA           :=    Notting to be wake_up was found (all ok, just notification)
//  EBADE             :=    Permission invalid to execute the operation// --
// Return TBDE_REMOVE:
//  succes            :=    return 0
//  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
//  EBADE             :=    Permission invalid to execute the operation
//  EADDRINUSE        :=    Reader in wait on some level
// --
//  ENOSYS            :=    sysCall removed
//  ENOSR             :=    tag negative number
//  EILSEQ            :=    Command not valid
// int tag_ctl(int tag, int command);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(2, _tag_ctl, int, tag, int, command) {
#else
asmlinkage long tag_ctl(int tag, int command) {
#endif
  int ret;
  if (!try_module_get(THIS_MODULE))
    return -ENOSYS;

  tbde_info("[tag_ctl] thread %d call [tag_ctl(%d,%d)]\n", current->pid, tag, command);
  if (tag < 0) {
    module_put(THIS_MODULE);
    return -ENOSR;
  }

  switch (command) {
  case TBDE_AWAKE_ALL:
    ret = tag_ctl_TBDE_AWAKE_ALL(tag, command);
    break;
  case TBDE_REMOVE:
    ret = tag_ctl_TBDE_REMOVE(tag, command);
    break;
  default:
    tbde_db("[tag_ctl] Invalid Command");
    ret = -EILSEQ;
    break;
  }
  module_put(THIS_MODULE);
  return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_ctl = (unsigned long)__x64_sys_tag_ctl;
#else
#endif
