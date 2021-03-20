
#include <stdio.h>
#include <unistd.h>

// int tag_get(int key, int command, int permission);
// int tag_send(int tag, int level, char *buffer, size_t size);
// int tag_receive(int tag, int level, char *buffer, size_t size);
// int tag_ctl(int tag, int command);

int main(int argc, char **argv) {
  char buf[60];
  printf("## Start user code ##\n");

  printf("tag_get(1, 2, 3)\n");
  syscall(134, 1, 2, 3);

  printf("tag_send(4, 5, buf, sizeof(buf))\n");
  syscall(174, 4, 5, buf, sizeof(buf));

  printf("tag_receive(6, 7, buf, sizeof(buf))\n");
  syscall(177, 6, 7, buf, sizeof(buf));

  printf("tag_ctl(8, 9)\n");
  syscall(178, 8, 9);

  printf("End user code\n");
}
