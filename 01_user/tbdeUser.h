#ifndef tbdeUser_h
#define tbdeUser_h

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <tbdeType.h>


extern int sysCallIndex[4];
int convertStrtoArr(char *str, int numArr[], int limit);

void initTBDE();
// This array expose the sysCallIndex of the syscall function:
// [0] int tag_get(int key, int command, int permission);
// [1] int tag_send(int tag, int level, char *buffer, size_t size);
// [2] int tag_receive(int tag, int level, char *buffer, size_t size);
// [3] int tag_ctl(int tag, int command);
int tag_get(int key, int command, int permission);
int tag_send(int tag, int level, char *buffer, size_t size);
int tag_receive(int tag, int level, char *buffer, size_t size);
int tag_ctl(int tag, int command);

#endif