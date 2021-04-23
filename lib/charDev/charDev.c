#include "charDev.h"

object_state objects[MINORS];

static int dev_open(struct inode *inode, struct file *file) {
  printk("DevOpen");
  return 0;
  int minor;
  minor = get_minor(file);

  if (minor >= MINORS) {
    return -ENODEV;
  }
#ifdef SINGLE_INSTANCE
  // this device file is single instance
  if (!mutex_trylock(&device_state)) {
    return -EBUSY;
  }
#endif

#ifdef SINGLE_SESSION_OBJECT
  if (!mutex_trylock(&(objects[minor].object_busy))) {
    goto open_failure;
  }
#endif

  printk("%s: device file successfully opened for object with minor %d\n", MODNAME, minor);
  // device opened by a default nop
  return 0;

#ifdef SINGLE_SESSION_OBJECT
open_failure:
#ifdef SINGE_INSTANCE
  mutex_unlock(&device_state);
#endif
  return -EBUSY;
#endif
}

static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off) {

  int minor = get_minor(filp);
  int ret;
  object_state *the_object;

  the_object = objects + minor;
  printk("%s: somebody called a read on dev with [major,minor] number [%d,%d]\n", MODNAME, get_major(filp),
         get_minor(filp));

  // need to lock in any case
  mutex_lock(&(the_object->operation_synchronizer));
  if (*off > the_object->valid_bytes) {
    mutex_unlock(&(the_object->operation_synchronizer));
    return 0;
  }
  if ((the_object->valid_bytes - *off) < len)
    len = the_object->valid_bytes - *off;
  ret = copy_to_user(buff, &(the_object->stream_content[*off]), len);

  *off += (len - ret);
  mutex_unlock(&(the_object->operation_synchronizer));

  return len - ret;
  printk("%s: somebody called a read on dev with [major,minor] number [%d,%d]\n", MODNAME, get_major(filp),
         get_minor(filp));

  return 0;
}

static int dev_release(struct inode *inode, struct file *file) {

  int minor;
  minor = get_minor(file);

#ifdef SINGLE_SESSION_OBJECT
  mutex_unlock(&(objects[minor].object_busy));
#endif

#ifdef SINGLE_INSTANCE
  mutex_unlock(&device_state);
#endif

  printk("%s: device file closed\n", MODNAME);
  // device closed by default nop
  return 0;
}

static ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off) {

  int minor = get_minor(filp);
  int ret;
  object_state *the_object;

  the_object = objects + minor;
  printk("%s: somebody called a write on dev with [major,minor] number [%d,%d]\n", MODNAME, get_major(filp),
         get_minor(filp));

  // need to lock in any case
  mutex_lock(&(the_object->operation_synchronizer));
  if (*off >= OBJECT_MAX_SIZE) { // offset too large
    mutex_unlock(&(the_object->operation_synchronizer));
    return -ENOSPC; // no space left on device
  }
  if (*off > the_object->valid_bytes) { // offset bwyond the current stream size
    mutex_unlock(&(the_object->operation_synchronizer));
    return -ENOSR; // out of stream resources
  }
  if ((OBJECT_MAX_SIZE - *off) < len)
    len = OBJECT_MAX_SIZE - *off;
  ret = copy_from_user(&(the_object->stream_content[*off]), buff, len);

  *off += (len - ret);
  the_object->valid_bytes = *off;
  mutex_unlock(&(the_object->operation_synchronizer));

  return len - ret;
}

static long dev_ioctl(struct file *filp, unsigned int command, unsigned long param) {

  int minor = get_minor(filp);
  object_state *the_object;

  the_object = objects + minor;
  printk("%s: somebody called an ioctl on dev with [major,minor] number [%d,%d] and command %u \n", MODNAME,
         get_major(filp), get_minor(filp), command);

  // do here whathever you would like to control the state of the device
  return 0;
}

int majorNum;
dev_t devNo;          // Major and Minor device numbers combined into 32 bits
struct class *pClass; // class_create will set this

int devkoInit(void) {
  majorNum = __register_chrdev(0, 0, 256, DEVICE_NAME, &fileOps);

  if (majorNum < 0) {
    printk("%s: registering device failed\n", MODNAME);
    return majorNum;
  }

  printk(KERN_INFO "%s: new device registered, it is assigned major number %d\n", MODNAME, majorNum);

  return 0;
} // end of devkoInit

void devkoExit(void) {
  unregister_chrdev(majorNum, DEVICE_NAME);

  printk(KERN_INFO "%s: new device unregistered, it was assigned major number %d\n", MODNAME, majorNum);

  return;
} // end of devkoExit
