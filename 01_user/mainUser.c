
#include "tbdeUser.h"
#include <stdio.h>

#define nTag 5
int tags[nTag];

int main(int argc, char **argv) {
  char buf[64];
  initTBDE();
  printf("tag_gets\n");
  for (int i = 0; i < nTag; i++)
    tags[i] = tag_get(i, TBDE_O_CREAT, TBDE_OPEN_ROOM);

  printf("tag_send(4, 5, buf, sizeof(buf))=%d\n", tag_send(4, 5, buf, sizeof(buf)));

  printf("tag_receive(6, 7, buf, sizeof(buf))=%d\n", tag_receive(6, 7, buf, sizeof(buf)));

  printf("tag_ctls\n");
  for (int i = 0; i < nTag; i++) {
    tag_ctl(tags[i], TBDE_REMOVE);
  }
}
