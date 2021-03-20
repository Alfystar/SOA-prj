
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("Start user code\n");
  syscall(134, 1, 2);
  printf("End user code\n");
}
