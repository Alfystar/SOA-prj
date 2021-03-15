#ifndef sysCall_Discovery_h
#define sysCall_Discovery_h

#include "globalDef.h"


#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/kprobes.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/apic.h>
#include <linux/syscalls.h>
#include "vtpmo.h"

#define ENTRIES_TO_EXPLORE 256

extern unsigned long *hacked_ni_syscall;
extern unsigned long **hacked_syscall_tbl;

extern unsigned long sys_call_table_address;
extern unsigned long sys_ni_syscall_address;

#define MAX_FREE 15
extern int free_entries[MAX_FREE];


void syscall_table_finder(void);


#endif