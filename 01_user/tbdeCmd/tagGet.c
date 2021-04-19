
//#include "tbdeUser/tbdeUser.h"
#include <stdio.h>
#include <stdlib.h>
#include <tbdeUser.h>

void help() {
  printf("usage: <keyAsk> <commandAsk>, <permissionAsk>\n");
  printf("commandAsk:\n");
  printf("\tTBDE_O_CREAT = %d\n", TBDE_O_CREAT);
  printf("\tTBDE_O_OPEN = %d\n", TBDE_O_OPEN);
  printf("permissionAsk:\n");
  printf("\tTBDE_OPEN_ROOM = %d\n", TBDE_OPEN_ROOM);
  printf("\tTBDE_PRIVATE_ROOM = %d\n", TBDE_PRIVATE_ROOM);
}

int tag;
int keyAsk, commandAsk, permissionAsk;

int main(int argc, char **argv) {
  char buf[64];
  if (argc != 3) {
    help();
    exit(-1);
  }
  initTBDE();

  keyAsk = atoi(argv[1]);
  commandAsk = atoi(argv[2]);
  permissionAsk = atoi(argv[3]);

  printf("tag_get(%d,%d,%d)n", keyAsk, commandAsk, permissionAsk);
  tag = tag_get(keyAsk, commandAsk, permissionAsk);
  if (tag == -1) {
    tagGet_perror(keyAsk, commandAsk);
  }
}
