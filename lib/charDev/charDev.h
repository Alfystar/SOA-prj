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
#include <linux/tty.h>          /* For the tty declarations */
#include <linux/version.h>      /* For LINUX_VERSION_CODE */
#define DEVICE_NAME "tbde_stat" /* Device file name in /dev/ - not mandatory  */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#define get_major(session) MAJOR(session->f_inode->i_rdev)
#define get_minor(session) MINOR(session->f_inode->i_rdev)
#else
#define get_major(session) MAJOR(session->f_dentry->d_inode->i_rdev)
#define get_minor(session) MINOR(session->f_dentry->d_inode->i_rdev)
#endif

typedef struct _instance {
  char *text;
  size_t len;
  struct mutex operation_synchronizer;
} instance;

#define OBJECT_MAX_SIZE (4096) // just one page

instance *allocInstance(void);
void freeInstance(instance *inst);

/* the actual driver */
int dev_open(struct inode *, struct file *);
int dev_release(struct inode *, struct file *);
ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off);
ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

extern struct file_operations fileOps;
extern int majorNum;
extern dev_t devNo;          // Major and Minor device numbers combined into 32 bits
extern struct class *pClass; // class_create will set this

int devkoInit(void);
void devkoExit(void);

#endif