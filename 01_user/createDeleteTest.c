
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
  srand(myFork); // for each fork different seed

  printf("tag_gets\n");
  int keyAsk, commandAsk, permissionAsk;
  keyAsk = 1;
  commandAsk = TBDE_O_CREAT + myFork;
  permissionAsk = TBDE_OPEN_ROOM;
  printf("(%d)\n", myFork);
  tag = tag_get(keyAsk, commandAsk, permissionAsk);
  if (tag == -1) {
    tagGet_perror(keyAsk, commandAsk);
  }

  //----------------------------------------------------------------------------------

  printf("tag_ctls\n");
  printf("(%d)\n", myFork);
  if (tag_ctl(tag, TBDE_REMOVE) == -1) {
    tagCtl_perror(tag, TBDE_REMOVE);
  }
}
