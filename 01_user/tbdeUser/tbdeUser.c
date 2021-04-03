#include "tbdeUser.h"

int sysCallIndex[4] = {0};

// int tag_get(int key, int command, int permission);
// int tag_send(int tag, int level, char *buffer, size_t size);
// int tag_receive(int tag, int level, char *buffer, size_t size);
// int tag_ctl(int tag, int command);

int convertStrtoArr(char *str, int numArr[], int limit) {
  int i = 0;
  char *token = strtok(str, ",");
  // loop through the string to extract all other tokens
  while (token != NULL && limit > 0) {
    numArr[i++] = atoi(token);
    token = strtok(NULL, ",");
    limit--;
  }
  return i;
}

void initTBDE() {
  char buf[64];
  FILE *fp;
  fp = fopen("/sys/module/TAG_DataExchange/parameters/sysCallNum", "r");
  if (!fp) {
    perror("fopen fail to read sysCallNum parameter");
    switch (errno) {
    case ENOENT:
      printf("Please, mound the module on the kernel:\n>>\tmake load\n");
      break;
    default:
      break;
    }
    exit(EXIT_FAILURE);
  }
  fgets(buf, sizeof(buf), fp);
  fclose(fp);
  int found = convertStrtoArr(buf, sysCallIndex, 4);
  for (int i = 0; i < found; i++) {
    printf("sysCallIndex[%d]=%d --> ", i, sysCallIndex[i]);
  }
  printf("END\n");
}

int tag_get(int key, int command, int permission) {
  if (sysCallIndex[0] != 0)
    return syscall(sysCallIndex[0], key, command, permission);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}
int tag_send(int tag, int level, char *buffer, size_t size) {
  if (sysCallIndex[1] != 0)
    return syscall(sysCallIndex[1], tag, level, buffer, size);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}
int tag_receive(int tag, int level, char *buffer, size_t size) {
  if (sysCallIndex[2] != 0)
    return syscall(sysCallIndex[2], tag, level, buffer, size);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}
int tag_ctl(int tag, int command) {
  if (sysCallIndex[3] != 0)
    return syscall(sysCallIndex[3], tag, command);
  else {
    printf("System not jet initialize!!\n Exiting...\n");
    exit(EXIT_FAILURE);
  }
}