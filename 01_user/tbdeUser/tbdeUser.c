#include "tbdeUser.h"

int sysCallIndex[4] = {0};

// int tag_get(int key, int command, int permission);
// int tag_send(int tag, int level, char *buffer, size_t size);
// int tag_receive(int tag, int level, char *buffer, size_t size);
// int tag_ctl(int tag, int command);

int convertStrtoArr(char *str, int numArr[], int limit) {
  int i = 0;
  char *token = strtok(str, ",");
  // loop through the string to extract all other tokens
  while (token != NULL && limit > 0) {
    numArr[i++] = atoi(token);
    token = strtok(NULL, ",");
    limit--;
  }
  return i;
}

void initTBDE() {
  char buf[64];
  FILE *fp;
  fp = fopen("/sys/module/TAG_DataExchange/parameters/sysCallNum", "r");
  if (!fp) {
    perror("fopen fail to read sysCallNum parameter");
    switch (errno) {
    case ENOENT:
      printf("Please, mound the module on the kernel:\n>>\tmake load\n");
      break;
    default:
      break;
    }
    exit(EXIT_FAILURE);
  }
  fgets(buf, sizeof(buf), fp);
  fclose(fp);
  int found = convertStrtoArr(buf, sysCallIndex, 4);
  for (int i = 0; i < found; i++) {
    printf("sysCallIndex[%d]=%d --> ", i, sysCallIndex[i]);
  }
  printf("END\n");
}

int tag_get(int key, int command, int permission) {
  if (sysCallIndex[0] != 0)
    return syscall(sysCallIndex[0], key, command, permission);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}
int tag_send(int tag, int level, char *buffer, size_t size) {
  if (sysCallIndex[1] != 0)
    return syscall(sysCallIndex[1], tag, level, buffer, size);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}
int tag_receive(int tag, int level, char *buffer, size_t size) {
  if (sysCallIndex[2] != 0)
    return syscall(sysCallIndex[2], tag, level, buffer, size);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}
int tag_ctl(int tag, int command) {
  if (sysCallIndex[3] != 0)
    return syscall(sysCallIndex[3], tag, command);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}

void tagGet_perror(int keyAsk, int commandAsk) {
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
  if (errno == EILSEQ) {
    printf("[tag_get] error, Command is not valid, commandAsk :%d\n", commandAsk);
    return;
  }

  if (commandAsk == TBDE_O_CREAT) {
    switch (errno) {
    case ETOOMANYREFS:
      printf("[tag_get] TBDE_O_CREAT @key=%d error, Too many room was created\n", keyAsk);
      break;
    case EBADRQC:
      printf("[tag_get] TBDE_O_CREAT @key=%d error, Permission Wrong parameter\n", keyAsk);
      break;
    case EBADR:
      printf("[tag_get] TBDE_O_CREAT @key=%d error, Key already in use\n", keyAsk);
      break;
    default:
      printf("[tag_get] TBDE_O_CREAT error, Unexpected error code return: %d\n", errno);
      break;
    }
    return;
  }

  if (commandAsk == TBDE_O_OPEN) {
    switch (errno) {
    case EBADSLT:
      printf("[tag_get] TBDE_O_OPEN @key=%d error, Key is TBDE_IPC_PRIVATE\n", keyAsk);
      break;
    case EBADRQC:
      printf("[tag_get] TBDE_O_OPEN @key=%d error, Permission off the room is invalid\n", keyAsk);
      break;
    case ENOMSG:
      printf("[tag_get] TBDE_O_OPEN @key=%d error, Key was not founs\n", keyAsk);
      break;
    default:
      printf("[tag_get] TBDE_O_OPEN error, Unexpected error code return: %d\n", errno);
      break;
    }
    return;
  }

  printf("[tag_get] error, Unexpected error code return: %d\n", errno);
}

void tagSend_perror(int tag) {
  // Return tag_send:
  //  succes            :=    return 0
  //  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE), or not present
  //  ENOMSG            :=    Tag not found
  //  EBADRQC           :=    Permission invalid to execute the operation
  //  EBADSLT           :=    asked level is over levelDeep
  // --
  //  EILSEQ            :=    Command not valid
  switch (errno) {
  case EXFULL:
    printf("[tag_send] @tag=%d error, Buffer too long (out of MAX_BUF_SIZE), or not present\n", tag);
    break;
  case ENOMSG:
    printf("[tag_send] @tag=%d error, Tag not found\n", tag);
    break;
  case EBADRQC:
    printf("[tag_send] @tag=%d error, Permission invalid to execute the operation\n", tag);
    break;
  case EBADSLT:
    printf("[tag_send] @tag=%d error, asked level is over levelDeep\n", tag);
    break;
  case EILSEQ:
    printf("[tag_send] @tag=%d error, Command not valid\n", tag);
    break;
  default:
    printf("[tag_send] error, Unexpected error code return: %d\n", errno);
    break;
  }
}

void tagRecive_perror(int tag) {
  // Return tag_receive:
  //  succes            :=    return len copied
  //  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE), or not present
  //  ENOMSG            :=    Tag not found
  //  EBADRQC           :=    Permission invalid to execute the operation
  //  EBADSLT           :=    asked level is over levelDeep
  //  ERESTART          :=    Signal wake_up the thread
  //  EUCLEAN           :=    Receved TBDE_AWAKE_ALL
  // --
  //  EILSEQ            :=    Command not valid
  switch (errno) {
  case EXFULL:
    printf("[tag_receive] @tag=%d error, Buffer too long (out of MAX_BUF_SIZE), or not present\n", tag);
    break;
  case ENOMSG:
    printf("[tag_receive] @tag=%d error, Tag not found\n", tag);
    break;
  case EBADRQC:
    printf("[tag_receive] @tag=%d error, Permission invalid to execute the operation\n", tag);
    break;
  case EBADSLT:
    printf("[tag_receive] @tag=%d error, asked level is over levelDeep\n", tag);
    break;
  case ERESTART:
    printf("[tag_receive] @tag=%d error, Signal wake_up the thread\n", tag);
    break;
  case EUCLEAN:
    printf("[tag_receive] @tag=%d error, Receved TBDE_AWAKE_ALL\n", tag);
    break;
  case EILSEQ:
    printf("[tag_receive] @tag=%d error, Command not valid\n", tag);
    break;
  default:
    printf("[tag_receive] error, Unexpected error code return: %d\n", errno);
    break;
  }
}

void tagCtl_perror(int tag, int commandAsk) {
  // Return TBDE_AWAKE_ALL:
  //  succes            :=    return 0
  //  ENOSR             :=    tag negative number
  //  ENODATA           :=    Notting to be wake_up was found (all ok, just notification)
  //  EBADE             :=    Permission invalid to execute the operation// --
  // Return TBDE_REMOVE:
  //  succes            :=    return 0
  //  ENOSR             :=    tag negative number
  //  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
  //  EBADE             :=    Permission invalid to execute the operation
  //  EADDRINUSE        :=    Reader in wait on some level
  // --
  //  EILSEQ            :=    Command not valid
  if (errno == EILSEQ) {
    printf("[tag_ctl] error, Command is not valid, commandAsk :%d\n", commandAsk);
    return;
  }

  if (commandAsk == TBDE_AWAKE_ALL) {
    switch (errno) {
    case ENOSR:
      printf("[tag_ctl] TBDE_AWAKE_ALL @tag=%d error, The number is negative\n", tag);
      break;
    case ENODATA:
      printf("[tag_ctl] TBDE_AWAKE_ALL @tag=%d error, Notting to be deleted was found\n", tag);
      break;
    case EBADE:
      printf("[tag_ctl] TBDE_AWAKE_ALL @tag=%d error, Permission invalid to execute the operation\n", tag);
      break;
    default:
      printf("[tag_ctl] TBDE_AWAKE_ALL, Unexpected error code return: %d\n", errno);
      break;
    }
    return;
  }

  if (commandAsk == TBDE_REMOVE) {
    switch (errno) {
    case ENOSR:
      printf("[tag_ctl] TBDE_REMOVE @tag=%d error, The number is negative\n", tag);
      break;
    case EBADE:
      printf("[tag_ctl] TBDE_REMOVE @tag=%d error, Permission invalid to execute the operation\n", tag);
      break;
    case EADDRINUSE:
      printf("[tag_ctl] TBDE_REMOVE @tag=%d error, Reader are waiting on some level\n", tag);
      break;
    case ENODATA:
      printf("[tag_ctl] TBDE_REMOVE @tag=%d error, Notting to be deleted was found (all ok, just notification)\n", tag);
      break;
    default:
      printf("[tag_ctl] TBDE_REMOVE @tag=%d error, Unexpected error code return: %d\n", tag, errno);
      break;
    }
    return;
  }

  printf("[tag_send] error, Unexpected error code return: %d\n", errno);
}
