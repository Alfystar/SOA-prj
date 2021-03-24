
#include "tbdeUser.h"
#include <stdio.h>

int main(int argc, char **argv) {
  char buf[64];
  initTBDE();
  printf("tag_get(1, 2, 3)=%d\n", tag_get(1, 2, 3));

  printf("tag_send(4, 5, buf, sizeof(buf))=%d\n", tag_send(4, 5, buf, sizeof(buf)));

  printf("tag_receive(6, 7, buf, sizeof(buf))=%d\n", tag_receive(6, 7, buf, sizeof(buf)));

  printf("tag_ctl(8, 9)=%d\n", tag_ctl(8, 9));
}
