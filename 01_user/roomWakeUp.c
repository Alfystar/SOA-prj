
#include "tbdeUser/tbdeUser.h"
#include <stdio.h>
#include <stdlib.h>

int tag;

int main(int argc, char **argv) {
  int myFork;
  for (myFork = 0; myFork < 4; myFork++) {
    int pid = fork();
    if (pid == 0) // son
      break;
  }
  char buf[64];
  initTBDE();

  printf("(%d) tag_gets(...)\n", myFork);
  int keyAsk, commandAsk, permissionAsk;
  keyAsk = 10;
  permissionAsk = TBDE_OPEN_ROOM;
  commandAsk = TBDE_O_CREAT;
  tag = tag_get(keyAsk, commandAsk, permissionAsk);

  if (tag == -1) {
    tagGet_perror(keyAsk, commandAsk);
    commandAsk = TBDE_O_OPEN;
    tag = tag_get(keyAsk, commandAsk, permissionAsk);
    if (tag == -1) {
      tagGet_perror(keyAsk, commandAsk);
      exit(-1);
    }
  }
  //----------------------------------------------------------------------------------

  if (commandAsk == TBDE_O_CREAT) {
    sleep(1); // 10 ms
    printf("(%d) tag_ctl(tag, TBDE_AWAKE_ALL)\n", myFork);
    int ret = tag_ctl(tag, TBDE_AWAKE_ALL);
    if (ret == -1)
      tagCtl_perror(tag, TBDE_AWAKE_ALL);
  }
  // \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if (commandAsk == TBDE_O_OPEN) {
    printf("(%d) tag_receive(...)\n", myFork);
    int bRead = tag_receive(tag, 1, buf, sizeof(buf));
    if (bRead < 0)
      tagRecive_perror(tag);
    else
      printf("[reader %d]%s\tReturn value = %d\n", myFork, buf, bRead);
  }
  //----------------------------------------------------------------------------------

  printf("(%d) tag_ctls(...)\n", myFork);
  if (tag_ctl(tag, TBDE_REMOVE) == -1) {
    tagCtl_perror(tag, TBDE_REMOVE);
  }
}
