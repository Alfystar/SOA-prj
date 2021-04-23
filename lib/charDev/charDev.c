#include "charDev.h"

instance *allocInstance() {
  instance *inst = kzalloc(sizeof(instance), GFP_USER); // todo: verificare che forse debba essere kernel space
  inst->len = OBJECT_MAX_SIZE;
  inst->text = tbdeStatusString(&inst->len);
  return inst;
}
void freeInstance(instance *inst) {
  vfree(inst->text);
  kfree(inst);
}

int dev_open(struct inode *inode, struct file *file) {
  printk("dev_open");
  file->private_data = allocInstance();
  return 0;
}

int dev_release(struct inode *inode, struct file *file) {
  printk("dev_release");
  freeInstance(file->private_data);
  return 0;
}

ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off) {
  printk("dev_read");
  return 0;
}

ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
  printk("dev_write");
  return 0;
}

struct file_operations fileOps = {
    .owner = THIS_MODULE,   // do not forget this
    .open = dev_open,       //
    .read = dev_read,       //
    .write = dev_write,     //
    .release = dev_release, //
};

int majorNum;
dev_t devNo;          // Major and Minor device numbers combined into 32 bits
struct class *pClass; // class_create will set this

int devkoInit(void) {
  struct device *pDev;

  // Register character device
  majorNum = register_chrdev(0, DEVICE_NAME, &fileOps);
  if (majorNum < 0) {
    printk(KERN_ALERT "Could not register device: %d\n", majorNum);
    return majorNum;
  }
  devNo = MKDEV(majorNum, 0); // Create a dev_t, 32 bit version of numbers

  // Create /sys/class/kmem in preparation of creating /dev/kmem

  pClass = class_create(THIS_MODULE, "kmem");
  if (IS_ERR(pClass)) {
    printk(KERN_WARNING "\ncan't create class");
    unregister_chrdev_region(devNo, 1);
    return -1;
  }

  // Create /dev/kmem for this char dev
  if (IS_ERR(pDev = device_create(pClass, NULL, devNo, NULL, "kmem"))) {
    printk(KERN_WARNING "devko.ko can't create device /dev/kmem\n");
    // class_destroy(pClass);
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

  printk(KERN_INFO "%s: new device unregistered, it was assigned major number %d\n", MODNAME, majorNum);

  return;
} // end of devkoExit
