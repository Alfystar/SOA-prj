
#include <stdio.h>
#include <stdlib.h>
#include <tbdeUser.h>

#define nTag 300
int tags[nTag];

int main(int argc, char **argv) {
  int myFork;
  for (myFork = 0; myFork < 10; myFork++) {
    int pid = fork();
    if (pid == 0) // son
      break;
  }
  char buf[64];
  initTBDE();
  srand(myFork); // for each fork different seed

  printf("tag_gets\n");
  int keyAsk, commandAsk, permissionAsk;
  for (int i = 0; i < nTag; i++) {
    keyAsk = rand() % nTag;
    commandAsk = rand() % 2;
    permissionAsk = rand() % 2;
    printf("(%d;%d)\n", myFork, i);
    tags[i] = tag_get(keyAsk, commandAsk, permissionAsk);
    if (tags[i] == -1) {
      tagGet_perror(keyAsk, commandAsk);
    }
  }
  //----------------------------------------------------------------------------------

  printf("tag_ctls\n");
  for (int i = 0; i < nTag; i++) {
    printf("(%d;%d)\n", myFork, i);
    if (tag_ctl(tags[i], TBDE_REMOVE) == -1) {
      tagCtl_perror(tags[i], TBDE_REMOVE);
    }
  }
}
