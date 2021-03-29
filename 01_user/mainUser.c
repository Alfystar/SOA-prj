
#include "tbdeUser.h"
#include <stdio.h>

int tags[10];

int main(int argc, char **argv) {
  char buf[64];
  initTBDE();
  printf("tag_gets\n");
  for (int i = 0; i < 10; i++)
    tags[i] = tag_get(i, TBDE_O_CREAT, TBDE_OPEN_ROOM);

  printf("tag_send(4, 5, buf, sizeof(buf))=%d\n", tag_send(4, 5, buf, sizeof(buf)));

  printf("tag_receive(6, 7, buf, sizeof(buf))=%d\n", tag_receive(6, 7, buf, sizeof(buf)));

  printf("tag_ctls\n");
  for (int i = 0; i < 10; i++)
    tags[i] = tag_ctl(i, TBDE_REMOVE);
}
