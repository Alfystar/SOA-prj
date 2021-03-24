#ifndef tbde_h
#define tbde_h

#include <globalDef.h>
#include <stddef.h>

#include <tbdeType.h>

#include <linux/syscalls.h>
#include <linux/version.h>

#include "avl.h"

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

int avl_tag_entryMajorCMP(void *a, void *b); // Return 1 if a>b
int avl_key_entryMajorCMP(void *a, void *b); // Return 1 if a>b

#endif