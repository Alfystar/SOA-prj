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

  if (__sync_fetch_and_add(&roomCount, 0) >= MAX_ROOM) {
    printk_tbdeDB("Impossible Create another room");
    return -ETOOMANYREFS;
  }

  if (permCheck(permission)) {
    printk_tbdeDB("Permission are Invalid");
    return -EBADRQC;
  }

  p = roomMake(key, 0, current->tgid, permission);
  if (p->key == TBDE_IPC_PRIVATE) {
    roomRefLock(p); // Lock per conto di tagTree
    write_lock_irqsave(&searchLock, flags);
    while (true) {
      tagCounting++;
      negativeReset(tagCounting);
      p->tag = tagCounting;
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL) {
        break;
      }
    }
    write_unlock_irqrestore(&searchLock, flags);
    __sync_fetch_and_add(&roomCount, 1);
  } else {          // Key public
    roomRefLock(p); // Lock per conto di keyTree
    roomRefLock(p); // Lock per conto di tagTree

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
      tagCounting++;
      negativeReset(tagCounting);
      p->tag = tagCounting;
      ret = Tree_Insert(tagTree, p);
      if (ret == NULL) {
        break;
      }
    }
    write_unlock_irqrestore(&searchLock, flags);

    __sync_fetch_and_add(&roomCount, 1);
  }
  printk_tbde("New room Create and added to the Searches Tree");
  TBDE_Audit printTrees();

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
// Return ...:
//  succes            :=    return 0
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE)
// --
//  EILSEQ            :=    Command not valid

// int tag_send(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_send(int tag, int level, char *buffer, size_t size) {
#endif
  // unsigned long flags;
  char *buf;

  printk("%s: thread %d call [tag_send(%d,%d,%p,%ld)]\n", MODNAME, current->pid, tag, level, buffer, size);

  if (size > MAX_BUF_SIZE)
    return -EXFULL;
  buf = vzalloc(size);
  copy_from_user(buf, buffer, size);

  printk(buf);
  vfree(buf);
  // rcu_read_lock();
  // rcu_read_unlock();
  // synchronize_rcu();
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_send = (unsigned long)__x64_sys_tag_send;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Return ...:
//  succes            :=    return 0
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE)
// --
//  EILSEQ            :=    Command not valid
// int tag_receive(int tag, int level, char *buffer, size_t size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char *, buffer, size_t, size) {
#else
asmlinkage long tag_receive(int tag, int level, char *buffer, size_t size) {
#endif
  // unsigned long flags;
  char *buf;
  size_t bSize;

  printk("%s: thread %d call [tag_receive(%d,%d,%p,%ld)]\n", MODNAME, current->pid, tag, level, buffer, size);
  if (size > MAX_BUF_SIZE)
    return -EXFULL;

  buf = vzalloc(128);
  bSize = sprintf(buf, "Sono il kernel e ti saluto\n");
  bSize = min(min(bSize, size - 1), 128UL - 1);
  buf[bSize++] = '\0';
  copy_to_user(buffer, buf, bSize);
  vfree(buf);
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
unsigned long tag_receive = (unsigned long)__x64_sys_tag_receive;
#else
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
  Node retTag, retKey;
  room *p, searchRoom;
  unsigned long flags;

  printk("%s: thread %d call [tag_ctl(%d,%d)]\n", MODNAME, current->pid, tag, command);
  switch (command) {
  case TBDE_AWAKE_ALL:
    // todo: Implementare dopo aver creato la recive
    break;
  case TBDE_REMOVE:
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
      freeRoom(Tree_DeleteNode(tagTree, retTag));

      if (p->key != TBDE_IPC_PRIVATE) { // devo trovare la key
        searchRoom.key = p->key;
        printk_tbdeDB("Now will be search key=%d", p->key);
        retKey = Tree_SearchNode(keyTree, &searchRoom);

        // todo: dopo aver creato la recive, Verificare che la stanza sia senza nessun thread in lettura
        if (retKey) {
          printk_tbdeDB("Tag to delete had a key");
          freeRoom(Tree_DeleteNode(keyTree, retKey)); // decrease the pointer
          write_unlock_irqrestore(&searchLock, flags);
        } else {
          printk_tbdeDB("ERROR!!! key=%d should be present!!", p->key);
          write_unlock_irqrestore(&searchLock, flags);
        }
      } else { // se non ha key da cercare, libero il lock
        write_unlock_irqrestore(&searchLock, flags);
      }
      __sync_fetch_and_sub(&roomCount, 1);
      printk_tbde("Room (%d) are now Deleted, remaning %d rooms", tag, __sync_fetch_and_add(&roomCount, 0));
      TBDE_Audit printTrees();
      return 0;
    } // if (retTag)
    write_unlock_irqrestore(&searchLock, flags);
    return -ENODATA; // Se non lo trovo, non c'è nulla da eliminare ma lo notifico
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
