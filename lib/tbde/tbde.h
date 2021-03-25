#ifndef tbde_h
#define tbde_h

#include <globalDef.h>
#include <stddef.h>

#include <tbdeType.h>

#include <linux/errno.h>
#include <linux/syscalls.h>
#include <linux/version.h>

#include "avl.h"

int initTBDE();    // shuld be call BEFORE installation of syscall
int unmountTBDE(); // shuld be call AFTER installation of syscall

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
extern unsigned long tag_get;
extern unsigned long tag_send;
extern unsigned long tag_receive;
extern unsigned long tag_ctl;
#else
int tag_get(int key, int command, int permission);
int tag_send(int tag, int level, char *buffer, size_t size);
int tag_receive(int tag, int level, char *buffer, size_t size);
int tag_ctl(int tag, int command);
#endif

room *roomMake(int key, unsigned int tag, int uid_Creator, int perm);
void freeRoom(void *data);

int tagRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
int keyRoomCMP(void *a, void *b); // return -1:a<b | 0:a==b | 1:a>b
void printRoom(void *data);

#endif