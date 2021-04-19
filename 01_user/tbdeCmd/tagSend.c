
#include <stdio.h>
#include <stdlib.h>
#include <tbdeUser.h>

void help() { printf("usage: <tag> <level> <msg>\n"); }

int tag, level;

int main(int argc, char **argv) {
  if (argc != 3) {
    help();
    exit(-1);
  }
  initTBDE();

  tag = atoi(argv[1]);
  level = atoi(argv[2]);

  int size = strlen(argv[3]);
  size++; // last null caracter
  printf("tag_send(%d,%d,%p,%d)\n", tag, level, argv[3], size);
  int ret = tag_send(tag, level, argv[3], size);
  if (ret < 0)
    tagSend_perror(tag);
}
