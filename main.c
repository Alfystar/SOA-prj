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
#include <linux/syscalls.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/apic.h>



/* Correctly access read_cr0() and write_cr0(). */
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,3,0)
    #include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif

#ifndef X86_CR0_WP
#define X86_CR0_WP 0x00010000
#endif


#include "include/sysCall_Discovery.h"

/* Routine to execute when loading the module. */
int init_module_Default(void) {
	
	int i,j;
		
        printk("%s: initializing\n",MODNAME);
	
	syscall_table_finder();

	if(!hacked_syscall_tbl){
		printk("%s: failed to find the sys_call_table\n",MODNAME);
		return -1;
	}

	j=0;
	for(i=0;i<ENTRIES_TO_EXPLORE;i++)
		if(hacked_syscall_tbl[i] == hacked_ni_syscall){
			printk("%s: found sys_ni_syscall entry at syscall_table[%d]\n",MODNAME,i);	
			free_entries[j++] = i;
			if(j>=MAX_FREE) break;
		}

#ifdef SYS_CALL_INSTALL
	cr0 = read_cr0();
        unprotect_memory();
        hacked_syscall_tbl[FIRST_NI_SYSCALL] = (unsigned long*)sys_trial;
        protect_memory();
	printk("%s: a sys_call with 2 parameters has been installed as a trial on the sys_call_table at displacement %d\n",MODNAME,FIRST_NI_SYSCALL);	
#else
#endif

        printk("%s: module correctly mounted\n",MODNAME);

        return 0;

}

void cleanup_module_Default(void) {
                
#ifdef SYS_CALL_INSTALL
	cr0 = read_cr0();
        unprotect_memory();
        hacked_syscall_tbl[FIRST_NI_SYSCALL] = (unsigned long*)hacked_ni_syscall;
        protect_memory();
#else
#endif
        printk("%s: shutting down\n",MODNAME);
        
}


module_init(init_module_Default);
module_exit(cleanup_module_Default);

MODULE_AUTHOR("Emanuele Alfano <alfystar1701@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tag based Data Exchange");