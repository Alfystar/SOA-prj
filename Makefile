# Emanuele Alfano
# 07/03/2021
# Makefile for the "TAG_DataExchange" kernel module.
# Also, reference makefile for future kernel development tasks.

# The file name for the KO object to add and rm
MODNAME=TAG_DataExchange


ifeq ($(KERNELRELEASE),)
# if KERNELRELEASE is not defined, we've been called directly from the command line.
# Invoke the kernel build system.
.PHONY: all install clean uninstall load unload
all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
	$(CC) 00_user/user.c -o user.out

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
load:
	echo "$(MODNAME) Loading..."
	sudo insmod $(MODNAME).ko
unload:
	echo "$(MODNAME) Removing..."
	sudo rmmod $(MODNAME).ko

else
# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
EXTRA_CFLAGS = -Wall 

obj-m += $(MODNAME).o 
$(MODNAME)-y += main.o 

# TAG-based data exchange Sub-system
$(MODNAME)-y += ./lib/tbde/tbde.o 

# sysCall_Discovery Sub-system
$(MODNAME)-y += ./lib/sysCall_Discovery/sysCall_Discovery.o ./lib/sysCall_Discovery/vtpmo/vtpmo.o

# this ar need to add the include path to the kernel build system
ccflags-y := -I$(PWD)/include

endif
