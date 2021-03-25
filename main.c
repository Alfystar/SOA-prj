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
#include <linux/time.h>
#include <linux/vmalloc.h>

#include "lib/sysCall_Discovery/sysCall_Discovery.h"
#include "lib/tbde/tbde.h"

#define AUDIT if (0)
#define printk_Main(str, ...) printk("[%s]: " str, MODNAME, ##__VA_ARGS__)
#define printk_DB(str, ...) AUDIT printk_Main(str, ##__VA_ARGS__)

// This array expose the syscall number of the syscall function:
// [0] int tag_get(int key, int command, int permission);
// [1] int tag_send(int tag, int level, char *buffer, size_t size);
// [2] int tag_receive(int tag, int level, char *buffer, size_t size);
// [3] int tag_ctl(int tag, int command);
int sysCallNum[4];
module_param_array(sysCallNum, int, NULL, 0444); // only readable

#define STR_VALUE(arg) #arg
#define FUNCTION_NAME(name) STR_VALUE(name)

#define TEST_FUNC test_func
#define TEST_FUNC_NAME FUNCTION_NAME(TEST_FUNC)

#define exposeNewSyscall(sysPtr, num)                                                                                  \
  do {                                                                                                                 \
    sysCallNum[num] = add_syscall(sysPtr);                                                                             \
    if (sysCallNum[num] == -1) {                                                                                       \
      printk_Main("Module fail to mount at add_syscall(%s) \n", FUNCTION_NAME(sysPtr));                                \
      return -ENOMEM;                                                                                                  \
    } else                                                                                                             \
      printk_Main("sysCallNum[%d]=%d [%s(...)]\n", num, sysCallNum[num], FUNCTION_NAME(sysPtr));                       \
  } while (0)

/* Routine to execute when loading the module. */
int init_module_Default(void) {
  int freeFound;
  printk_Main("Initializing\n");

  freeFound = foundFree_entries(4);
  printk_Main("Found %d entries\n", freeFound);
  if (freeFound > 0) {
    initTBDE();
    exposeNewSyscall(tag_get, 0);
    exposeNewSyscall(tag_send, 1);
    exposeNewSyscall(tag_receive, 2);
    exposeNewSyscall(tag_ctl, 3);
  }
  printk_Main("Module correctly mounted\n");

  return 0;
}

void cleanup_module_Default(void) {
  removeAllSyscall();
  unmountTBDE();
  printk_Main("Shutting down\n");
}

module_init(init_module_Default);
module_exit(cleanup_module_Default);

MODULE_AUTHOR("Emanuele Alfano <alfystar1701@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tag based Data Exchange");