#ifndef sysCall_Discovery_h
#define sysCall_Discovery_h

#include "globalDef.h"

#define EXPORT_SYMTAB
#include "vtpmo.h"
#include <asm/apic.h>
#include <asm/cacheflush.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

extern unsigned long *hacked_ni_syscall;
extern unsigned long **hacked_syscall_tbl;

extern unsigned long sys_call_table_address;
extern unsigned long sys_ni_syscall_address;

#define MAX_FREE 15
extern int free_entries[MAX_FREE];
int foundFree_entries(void);

extern int free_used;
int add_syscall(unsigned long sysPtr); // Return index of syscall, or -1 for error
void removeAllSyscall(void);
#endif