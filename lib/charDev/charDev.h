#ifndef charDev_H
#define charDev_H
#include <globalDef.h>

#define EXPORT_SYMTAB
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h> /* For pid types */
#include <linux/sched.h>
#include <linux/tty.h>     /* For the tty declarations */
#include <linux/version.h> /* For LINUX_VERSION_CODE */

#define DEVICE_NAME "myNew-dev" /* Device file name in /dev/ - not mandatory  */

//#define SINGLE_INSTANCE //just one session at a time across all I/O node
#define SINGLE_SESSION_OBJECT // just one session per I/O node at a time

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#define get_major(session) MAJOR(session->f_inode->i_rdev)
#define get_minor(session) MINOR(session->f_inode->i_rdev)
#else
#define get_major(session) MAJOR(session->f_dentry->d_inode->i_rdev)
#define get_minor(session) MINOR(session->f_dentry->d_inode->i_rdev)
#endif

#ifdef SINGLE_INSTANCE
static DEFINE_MUTEX(device_state);
#endif

typedef struct _object_state {
#ifdef SINGLE_SESSION_OBJECT
  struct mutex object_busy;
#endif
  struct mutex operation_synchronizer;
  int valid_bytes;
  char *stream_content; // the I/O node is a buffer in memory

} object_state;

#define MINORS 8
extern object_state objects[MINORS];

#define OBJECT_MAX_SIZE (4096) // just one page

/* the actual driver */
static int dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl(struct file *filp, unsigned int command, unsigned long param);

static struct file_operations fileOps = {.owner = THIS_MODULE, // do not forget this
                                         .write = dev_write,
                                         .read = dev_read,
                                         .open = dev_open,
                                         .release = dev_release,
                                         .unlocked_ioctl = dev_ioctl};

extern int majorNum;
extern dev_t devNo;          // Major and Minor device numbers combined into 32 bits
extern struct class *pClass; // class_create will set this

int devkoInit(void);
void devkoExit(void);

#endif