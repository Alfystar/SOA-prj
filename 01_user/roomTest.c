
#include "tbdeUser/tbdeUser.h"
#include <stdio.h>
#include <stdlib.h>

int tag;

int main(int argc, char **argv) {
  int myFork;
  for (myFork = 0; myFork < 1; myFork++) {
    int pid = fork();
    if (pid == 0) // son
      break;
  }
  char buf[64];
  initTBDE();

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
  printf("(%d) tag_gets(...)\n", myFork);
  int keyAsk, commandAsk, permissionAsk;
  keyAsk = 10;
  permissionAsk = TBDE_OPEN_ROOM;
  commandAsk = TBDE_O_CREAT;
  tag = tag_get(keyAsk, commandAsk, permissionAsk);

  if (tag == -1) {
    if (errno == EILSEQ) {
      printf("tag_get(...) Command is not valid, commandAsk :%d\n", commandAsk);
    }

    if (commandAsk == TBDE_O_CREAT) {
      switch (errno) {
      case ETOOMANYREFS:
        printf("Creating the room with key=%d impossible because there are too many room opened\n", keyAsk);
        break;
      case EBADRQC:
        printf("Creating the room with key=%d impossible because the Permission have Wrong parameter\n", keyAsk);
        break;
      case EBADR:
        printf("Creating the room with key=%d impossible because the Key are already in use\n", keyAsk);
        commandAsk = TBDE_O_OPEN;
        tag = tag_get(keyAsk, commandAsk, permissionAsk);
        break;
      default:
        printf("Unexpected error code return: %d\n", errno);
        break;
      }
    }
  }

  if (commandAsk == TBDE_O_CREAT) {
    sleep(1);
    int size = sprintf(buf, "Salve professore sono il processo : %d\n", myFork);
    size++;
    printf("(%d) tag_send(...)\n", myFork);
    int ret = tag_send(tag, 1, buf, size);
    // Return tag_send:
    //  succes            :=    return 0
    //  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE) or no size
    //  ENOMSG            :=    Tag not found
    //  EBADRQC           :=    Permission invalid to execute the operation
    //  EBADSLT           :=    asked level is over levelDeep
    // --
    //  EILSEQ            :=    Command not valid
    switch (ret) {
    case EXFULL:
      /* code */
      break;
    case ENOMSG:
      /* code */
      break;
    case EBADRQC:
      /* code */
      break;
    case EBADSLT:
      /* code */
      break;
    case EILSEQ:
      /* code */
      break;
    default:
      break;
    }
  }
  if (commandAsk == TBDE_O_OPEN) {
    printf("(%d) tag_receive(...)\n", myFork);
    int bRead = tag_receive(tag, 1, buf, sizeof(buf));
    // Return tag_receive:
    //  succes            :=    return len copied
    //  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE) or no size
    //  ENOMSG            :=    Tag not found
    //  EBADRQC           :=    Permission invalid to execute the operation
    //  EBADSLT           :=    asked level is over levelDeep
    //  ERESTARTSYS       :=    Signal wake_up the thread
    // --
    //  EILSEQ            :=    Command not valid
    switch (bRead) {
    case EXFULL:
      /* code */
      break;
    case ENOMSG:
      /* code */
      break;
    case EBADRQC:
      /* code */
      break;
    case EBADSLT:
      /* code */
      break;
    case ERESTARTSYS:
      /* code */
      break;
    case EILSEQ:
      /* code */
      break;
    default:
      printf("%s\nReturn value = %d\n", buf, bRead);
      break;
    }
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

  printf("(%d) tag_ctls(...)\n", myFork);
  if (tag_ctl(tag, TBDE_REMOVE) == -1) {
    switch (errno) {
    case ENOSR:
      printf("Deleting the room with tag=%d impossible because the number is negative\n", tag);
      break;
    case EBADRQC:
      printf("Deleting the room with tag=%d impossible because the Permission invalid to execute the operation\n", tag);
      break;
    case ENODATA:
      printf("Deleting the room with tag=%d impossible Notting to be deleted was found (all ok, just notification)\n",
             tag);
    case EILSEQ:
      printf("tag_ctl Command is not valid\n");
      break;
    default:
      printf("Unexpected error code return: %d\n", errno);
      break;
    }
  }
}
