
#include "tbdeUser/tbdeUser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    // usleep(10 * 1000UL); // 10 ms
    sleep(3);
    int size = sprintf(buf, "Salve figliolo sono il processo : %d", myFork);
    size++; // last null caracter
    printf("(%d) tag_send(...)\n", myFork);
    int ret = tag_send(tag, 1, buf, size);
    if (ret < 0)
      tagSend_perror(tag);
  }
  // \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if (commandAsk == TBDE_O_OPEN) {
    printf("(%d) tag_receive(...)\n", myFork);
    // alarm(1);
    int bRead = tag_receive(tag, 1, buf, sizeof(buf));
    if (bRead < 0)
      tagRecive_perror(tag);
    else {
      // alarm(0);
      printf("[reader %d]%s\tReturn value = %d\n", myFork, buf, bRead);
    }
  }
  //----------------------------------------------------------------------------------

  printf("(%d) tag_ctls(...)\n", myFork);
  if (tag_ctl(tag, TBDE_REMOVE) == -1) {
    tagCtl_perror(tag, TBDE_REMOVE);
  }
}
