# Emanuele Alfano
# 07/03/2021
# Makefile for the "TAG_DataExchange" kernel module.
# Also, reference makefile for future kernel development tasks.

# The file name for the KO object to add and rm
MODNAME=TAG_DataExchange


ifeq ($(KERNELRELEASE),)
# if KERNELRELEASE is not defined, we've been called directly from the command line.
# Invoke the kernel build system.
.PHONY: all install clean uninstall
all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(CC) user/user.c -o user.out

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm user.out

install:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	ln -s /lib/modules/$(shell uname -r)/extra/$(MODNAME).ko /lib/modules/$(shell uname -r)
	depmod -a

uninstall:
	rm /lib/modules/$(shell uname -r)/extra/$(MODNAME).ko
	rm /lib/modules/$(shell uname -r)/$(MODNAME).ko
	depmod -a
else
# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
obj-m += TAG_DataExchange.o 
TAG_DataExchange-objs += main.o ./lib/sysCall_Discovery.o ./lib/vtpmo.o
#the_usctm-objs += ./lib/sysCall_Discovery.o ./lib/vtpmo.o

endif
