
#include <stdio.h>
#include <stdlib.h>
#include <tbdeUser.h>
#include <unistd.h>

#define rangeKey 10
int tag;

int main(int argc, char **argv) {
  int myFork;
  for (myFork = 0; myFork < rangeKey * 100; myFork++) {
    int pid = fork();
    if (pid == 0) // son
      break;
  }
  char buf[64];
  initTBDE();
  srand(myFork); // for each fork different seed

  printf("(%d) tag_gets(...)\n", myFork);
  int keyAsk, commandAsk, permissionAsk;
  keyAsk = rand() % rangeKey;
  commandAsk = TBDE_O_CREAT;
  permissionAsk = rand() % 2;
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
    usleep(10 * 1000UL); // 10 ms
    int size = sprintf(buf, "Salve figliolo sono il processo : %d", myFork);
    size++; // null caracter at end
    printf("(%d) tag_send(...)\n", myFork);
    int ret = tag_send(tag, 1, buf, size);
    if (ret < 0)
      tagSend_perror(tag);
  }
  // \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if (commandAsk == TBDE_O_OPEN) {
    printf("(%d) tag_receive(...)\n", myFork);
    //alarm(1);
    int bRead = tag_receive(tag, 1, buf, sizeof(buf));
    if (bRead < 0)
      tagRecive_perror(tag);
    else {
      //alarm(0);
      printf("[reader %d]%s\tReturn value = %d\n", myFork, buf, bRead);
    }
  }
  //----------------------------------------------------------------------------------

  printf("(%d) tag_ctls(...)\n", myFork);
  if (tag_ctl(tag, TBDE_REMOVE) == -1) {
    tagCtl_perror(tag, TBDE_REMOVE);
  }
}
