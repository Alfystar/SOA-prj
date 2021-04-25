# Emanuele Alfano
# 07/03/2021
# Makefile for the "TAG_DataExchange" kernel module.
# Also, reference makefile for future kernel development tasks.

# The file name for the KO object to add and rm
MODNAME=TAG_DataExchange
USER_DIR=01_user
TEST_DIR=test
CMD_DIR=cmd

USER_LIB_INC=-I$(USER_DIR)/tbdeUser $(USER_DIR)/tbdeUser/tbdeUser.c

ifeq ($(KERNELRELEASE),)
# if KERNELRELEASE is not defined, we've been called directly from the command line.
# Invoke the kernel build system.
.PHONY: all install clean uninstall load unload
all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	mkdir -p $(CMD_DIR)
	$(CC) $(USER_DIR)/tbdeCmd/get.c $(USER_LIB_INC) -o $(CMD_DIR)/get.out -Iinclude
	$(CC) $(USER_DIR)/tbdeCmd/send.c $(USER_LIB_INC) -o $(CMD_DIR)/send.out -Iinclude
	$(CC) $(USER_DIR)/tbdeCmd/recive.c $(USER_LIB_INC) -o $(CMD_DIR)/recive.out -Iinclude
	$(CC) $(USER_DIR)/tbdeCmd/ctl.c $(USER_LIB_INC) -o $(CMD_DIR)/ctl.out -Iinclude

	mkdir -p $(TEST_DIR)
#	Create/Open-delete test	
	$(CC) $(USER_DIR)/test/01_createDeleteTest.c $(USER_LIB_INC) -o $(TEST_DIR)/1_createDeleteTest.out -Iinclude
	$(CC) $(USER_DIR)/test/02_createDelete_LOAD.c $(USER_LIB_INC) -o $(TEST_DIR)/2_createDelete_LOAD.out -Iinclude
#	Chatting on room test	
	$(CC) $(USER_DIR)/test/03_roomExange.c $(USER_LIB_INC) -o $(TEST_DIR)/3_roomExange.out -Iinclude
#	Wake_up test
	$(CC) $(USER_DIR)/test/04_wakeUpTest.c $(USER_LIB_INC) -o $(TEST_DIR)/4_wakeUpTest.out -Iinclude
#	Signal test	
	$(CC) $(USER_DIR)/test/05_signalWait.c $(USER_LIB_INC) -o $(TEST_DIR)/5_signalWait.out -Iinclude
#	Rand test
	$(CC) $(USER_DIR)/test/06_roomExange_signal_LOAD.c $(USER_LIB_INC) -o $(TEST_DIR)/6_roomExange_signal_LOAD.out -Iinclude

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -r $(TEST_DIR)
	rm -r $(CMD_DIR)
	
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
$(MODNAME)-y += ./lib/tbde/tbde.o ./lib/tbde/avl.o ./lib/tbde/supportTBDE.o

# sysCall_Discovery Sub-system
$(MODNAME)-y += ./lib/sysCall_Discovery/sysCall_Discovery.o ./lib/sysCall_Discovery/vtpmo/vtpmo.o

# charDevice Sub-system
$(MODNAME)-y += ./lib/charDev/charDev.o

# this ar need to add the include path to the kernel build system
ccflags-y := -I$(PWD)/include

endif
