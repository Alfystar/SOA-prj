#ifndef charDev_H
#define charDev_H
#include <globalDef.h>

#define EXPORT_SYMTAB
#include "../tbde/tbde.h"
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h> /* For pid types */
#include <linux/sched.h>
#include <linux/tty.h>     /* For the tty declarations */
#include <linux/version.h> /* For LINUX_VERSION_CODE */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#define get_major(session) MAJOR(session->f_inode->i_rdev)
#define get_minor(session) MINOR(session->f_inode->i_rdev)
#else
#define get_major(session) MAJOR(session->f_dentry->d_inode->i_rdev)
#define get_minor(session) MINOR(session->f_dentry->d_inode->i_rdev)
#endif

#define DEVICE_NAME "tbde_stat" /* Device file name in /dev/ - not mandatory  */

// Level 0 = no message
// Level 1 = err message
// Level 2 = notice messsage
// Level 3 = Info messsage
// Level 4 = dbg message
// Level 5 = all message
#define DEVICE_VerboseLevel 5
#define DEVICE_err _codeActive(1, DEVICE_VerboseLevel)
#define DEVICE_notice _codeActive(2, DEVICE_VerboseLevel)
#define DEVICE_info _codeActive(3, DEVICE_VerboseLevel)
#define DEVICE_Db _codeActive(4, DEVICE_VerboseLevel)

#define device_err(str, ...)                                                                                           \
  do {                                                                                                                 \
    DEVICE_err printk_STD(KERN_ERR, "DEVICE", str, ##__VA_ARGS__);                                                     \
  } while (0)

#define device_notice(str, ...)                                                                                        \
  do {                                                                                                                 \
    DEVICE_notice printk_STD(KERN_NOTICE, "DEVICE", str, ##__VA_ARGS__);                                               \
  } while (0)

#define device_info(str, ...)                                                                                          \
  do {                                                                                                                 \
    DEVICE_info printk_STD(KERN_INFO, "DEVICE", str, ##__VA_ARGS__);                                                   \
  } while (0)

#define device_db(str, ...)                                                                                            \
  do {                                                                                                                 \
    DEVICE_Db printk_STD(KERN_DEBUG, "DEVICE", str, ##__VA_ARGS__);                                                    \
  } while (0)

typedef struct _instance {
  char *text;
  ssize_t size;
  struct mutex operation_synchronizer;
} instance;

#define OBJECT_MAX_SIZE (4096) // just one page

instance *allocInstance(void);
void freeInstance(instance *inst);

/* the actual driver */
int dev_open(struct inode *, struct file *);
int dev_release(struct inode *, struct file *);
ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off);
loff_t dev_llseek(struct file *filp, loff_t f_pos, int whence);

extern struct file_operations fileOps;
extern int majorNum;
extern dev_t devNo;          // Major and Minor device numbers combined into 32 bits
extern struct class *pClass; // class_create will set this

int devkoInit(void);
void devkoExit(void);

#endif