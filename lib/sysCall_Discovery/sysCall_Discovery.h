#ifndef sysCall_Discovery_h
#define sysCall_Discovery_h

#include <globalDef.h>

#define EXPORT_SYMTAB
#include "vtpmo/vtpmo.h"
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

/* Correctly access read_cr0() and write_cr0(). */
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
#include <asm/switch_to.h>
#else
#include <asm/system.h>
#endif

#ifndef X86_CR0_WP
#define X86_CR0_WP 0x00010000
#endif

#define sysCall_Audit if (0)
#define MAX_FREE 15

#define printk_sub(str, ...) printk("[%s::%s]: " str, MODNAME, "Discovery", ##__VA_ARGS__)
#define printk_subDB(str, ...) sysCall_Audit printk_sub(str, ##__VA_ARGS__)

int foundFree_entries(int num);

extern int free_used;
int add_syscall(unsigned long sysPtr); // Return index of syscall, or -1 for error
void removeAllSyscall(void);

#endif