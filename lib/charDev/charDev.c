#include "charDev.h"

instance *allocInstance() {
  instance *inst = kzalloc(sizeof(instance), GFP_USER); // todo: verificare che forse debba essere kernel space
  inst->size = OBJECT_MAX_SIZE;                         // verrà ridimensionata con la chiamata successiva
  inst->text = tbdeStatusString(&inst->size);
  return inst;
}
void freeInstance(instance *inst) {
  vfree(inst->text);
  kfree(inst);
}

int dev_open(struct inode *inode, struct file *file) {
  device_info("[dev_open]");
  file->private_data = allocInstance();
  return 0;
}

int dev_release(struct inode *inode, struct file *file) {
  device_info("[dev_release] (close callBack)");
  freeInstance(file->private_data);
  return 0;
}

ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *f_pos) {
  instance *dev = filp->private_data;
  device_info("[dev_read] @len=%ld @f_pos=%lld, @dev->size=%ld", len, *f_pos, dev->size);
  device_db("data read:\n%s", dev->text);

  if (*f_pos >= dev->size) // se sono oltre il file
    return 0;

  if (*f_pos + len > dev->size) // se la richiesta è superiore alla dimensione, ritornerò i byte mancanti
    len = dev->size - *f_pos;
  device_db("data to copy =%ld", len);

  data2UserForce(dev->text + *f_pos, buff, len);
  *f_pos += len;
  return len;
}

loff_t dev_llseek(struct file *filp, loff_t f_pos, int whence) {
  instance *dev = filp->private_data;
  loff_t newpos;
  device_info("[dev_llseek] @f_pos=%lld, @whence=%d", f_pos, whence);

  switch (whence) {
  case 0: /* SEEK_SET */
    newpos = f_pos;
    break;

  case 1: /* SEEK_CUR */
    newpos = filp->f_pos + f_pos;
    break;

  case 2: /* SEEK_END */
    newpos = dev->size + f_pos;
    // "+ f_pos" because f_pos in this case are negative
    break;

  default: /* can't happen */
    return -EINVAL;
  }
  if (newpos < 0 || newpos >= dev->size)
    return -EINVAL;
  filp->f_pos = newpos;
  return newpos;
}

struct file_operations fileOps = {
    .owner = THIS_MODULE,   // do not forget this
    .open = dev_open,       // Allocate in private_data the buffer to be read
    .release = dev_release, // De-Allocate in private_data the buffer readed
    .read = dev_read,       // Read the buffer
    .llseek = dev_llseek,   // Move position in the buffer
};

int majorNum;
dev_t devNo;          // Major and Minor device numbers combined into 32 bits
struct class *pClass; // class_create will set this

static char *dev_devnode(struct device *dev, umode_t *mode) {
  printk("\n\n****%s: %d\n\n", __func__, __LINE__);
  if (mode != NULL)
    *mode = 0444;
  return kasprintf(GFP_KERNEL, "%s/%s", MODNAME, dev_name(dev));
  ;
}

int devkoInit(void) {
  struct device *pDev;

  // Register character device
  majorNum = register_chrdev(0, DEVICE_NAME, &fileOps);
  if (majorNum < 0) {
    device_err("Could not register device: %d\n", majorNum);
    return majorNum;
  }
  devNo = MKDEV(majorNum, 0); // Create a dev_t, 32 bit version of numbers

  // Create /sys/class/DEVICE_NAME in preparation of creating /dev/DEVICE_NAME

  pClass = class_create(THIS_MODULE, DEVICE_NAME);
  if (IS_ERR(pClass)) {
    device_err("can't create /sys/%s class", DEVICE_NAME);
    unregister_chrdev_region(devNo, 1);
    return -1;
  }
  pClass->devnode = dev_devnode;
  // Create /dev/DEVICE_NAME for this char dev
  if (IS_ERR(pDev = device_create(pClass, NULL, devNo, NULL, DEVICE_NAME))) {
    device_err("can't create device /dev/%s/%s\n", MODNAME, DEVICE_NAME);
    class_destroy(pClass);
    unregister_chrdev_region(devNo, 1);
    return -1;
  }
  return 0;
} // end of devkoInit

void devkoExit(void) {
  unregister_chrdev(majorNum, DEVICE_NAME);
  // Clean up after ourselves
  device_destroy(pClass, devNo);            // Remove the /dev/kmem
  class_destroy(pClass);                    // Remove class /sys/class/kmem
  unregister_chrdev(majorNum, DEVICE_NAME); // Unregister the device

  device_info("new device unregistered, it was assigned major number %d\n", majorNum);

  return;
} // end of devkoExit
