
#include <stdio.h>
#include <stdlib.h>
#include <tbdeUser.h>

void help() {
  printf("usage: <tag> <Cmd>\n");
  printf("\tTBDE_REMOVE = %d\n", TBDE_REMOVE);
  printf("\tTBDE_AWAKE_ALL = %d\n", TBDE_AWAKE_ALL);
}

int tag;
int commandAsk;

int main(int argc, char **argv) {
  char buf[64];
  if (argc != 3) {
    help();
    exit(-1);
  }
  initTBDE();

  tag = atoi(argv[1]);
  commandAsk = atoi(argv[2]);

  printf("tag_ctls(%d, %d)\n", tag, commandAsk);
  if (tag_ctl(tag, commandAsk) == -1) {
    tagCtl_perror(tag, commandAsk);
  }
}
