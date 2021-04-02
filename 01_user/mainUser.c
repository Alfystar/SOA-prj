
#include "tbdeUser.h"
#include <stdio.h>
#include <stdlib.h>

#define nTag 300
int tags[nTag];

int main(int argc, char **argv) {
  int myFork;
  for (myFork = 0; myFork < 9; myFork++) {
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
  printf("tag_gets\n");
  int keyAsk, commandAsk, permissionAsk;
  for (int i = 0; i < nTag; i++) {
    keyAsk = rand() % nTag * 2;
    commandAsk = rand() % 2;
    permissionAsk = rand() % 2;
    printf("(%d;%d)\n", myFork, i);
    tags[i] = tag_get(keyAsk, commandAsk, permissionAsk);
    if (tags[i] == -1) {
      if (errno == EILSEQ) {
        printf("tag_get(...) Command is not valid, commandAsk :%d\n", commandAsk);
        continue;
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
          break;
        default:
          printf("Unexpected error code return: %d\n", errno);
          break;
        }
        continue;
      }

      if (commandAsk == TBDE_O_OPEN) {
        switch (errno) {
        case EBADSLT:
          printf("Opening the room with key=%d impossible because there jey is TBDE_IPC_PRIVATE\n", keyAsk);
          break;
        case EBADRQC:
          printf("Opening the room with key=%d impossible because the Permission off the room is invalid\n", keyAsk);
          break;
        case ENOMSG:
          printf("Opening the room with key=%d impossible because the Key was not founs\n", keyAsk);
          break;
        default:
          printf("Unexpected error code return: %d\n", errno);
          break;
        }
        continue;
      }
    } // if (tags[i] == -1)
  }

  printf("(%d)\n", myFork);
  printf("tag_send(4, 5, buf, sizeof(buf))=%d\n", tag_send(4, 5, buf, sizeof(buf)));

  printf("(%d)\n", myFork);
  printf("tag_receive(6, 7, buf, sizeof(buf))=%d\n", tag_receive(6, 7, buf, sizeof(buf)));

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
  printf("tag_ctls\n");
  for (int i = 0; i < nTag; i++) {
    printf("(%d;%d)\n", myFork, i);
    if (tag_ctl(tags[i], TBDE_REMOVE) == -1) {
      switch (errno) {
      case ENOSR:
        printf("Deleting the room with tag=%d impossible because the number is negative\n", tags[i]);
        break;
      case EBADRQC:
        printf("Deleting the room with tag=%d impossible because the Permission invalid to execute the operation\n",
               tags[i]);
        break;
      case ENODATA:
        printf("Deleting the room with tag=%d impossible Notting to be deleted was found (all ok, just notification)\n",
               tags[i]);
      case EILSEQ:
        printf("tag_ctl Command is not valid\n");
        break;
      default:
        printf("Unexpected error code return: %d\n", errno);
        break;
      }
    }
  }
}
