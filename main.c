/* Hello world, from the kernel. */
/*
 * This is free software.
 * You can redistribute it and/or modify this file under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3 of the License, or (at your option) any later
 * version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this file; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */
/*
 * @brief To be descripted
 *
 * @author Emanuele Alfano
 *
 * @date March 07, 2021
 */

#include "include/globalDef.h"

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

#include "include/sysCall_Discovery.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
__SYSCALL_DEFINEx(2, _trial, unsigned long, A, unsigned long, B) {
#else
asmlinkage long sys_trial(unsigned long A, unsigned long B) {
#endif

  printk("%s: thread %d requests a trial sys_call with %lu and %lu as "
         "parameters\n",
         MODNAME, current->pid, A, B);

  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
static unsigned long sys_trial = (unsigned long)__x64_sys_trial;
#else
#endif

/* Routine to execute when loading the module. */
int init_module_Default(void) {
  int freeFound;
  printk("%s: initializing\n", MODNAME);

  freeFound = foundFree_entries();
  printk("%s: found %d entries\n", MODNAME, freeFound);
  if (freeFound > 0) {
    add_syscall(sys_trial);
  }
  printk("%s: module correctly mounted\n", MODNAME);

  return 0;
}

void cleanup_module_Default(void) {
  removeAllSyscall();
  printk("%s: shutting down\n", MODNAME);
}

module_init(init_module_Default);
module_exit(cleanup_module_Default);

MODULE_AUTHOR("Emanuele Alfano <alfystar1701@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tag based Data Exchange");