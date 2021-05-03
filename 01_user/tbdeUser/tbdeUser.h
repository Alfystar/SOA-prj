#ifndef tbdeUser_h
#define tbdeUser_h

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tbdeType.h>

extern int sysCallIndex[4];
int convertStrtoArr(char *str, int numArr[], int limit);

void initTBDE();
// This array expose the sysCallIndex of the syscall function:
// [0] int tag_get(int key, int command, int permission);
// [1] int tag_send(int tag, int level, char *buffer, size_t size);
// [2] int tag_receive(int tag, int level, char *buffer, size_t size);
// [3] int tag_ctl(int tag, int command);

// Return CREATE:
//  succes            :=    tag value
//  ETOOMANYREFS      :=    Too many room was created
//  EBADR             :=    Key already in use
// --
// Return OPEN:
//  succes            :=    tag value
//  EBADSLT           :=    asked key is TBDE_IPC_PRIVATE
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENOMSG            :=    Key not found
// --
//  ENOSYS            :=    sysCall removed
//  EBADRQC           :=    Permission Wrong parameter
//  EILSEQ            :=    Command not valid
int tag_get(int key, int command, int permission);

// Return tag_send:
//  succes            :=    return 0
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE), or not present
//  ENODATA           :=    Problem to transmit data to the sub-system
//  ENOMSG            :=    Tag not found
//  EBADRQC           :=    Permission invalid to execute the operation
//  EBADSLT           :=    asked level is over levelDeep
// --
//  ENOSYS            :=    sysCall removed
//  EILSEQ            :=    Command not valid
int tag_send(int tag, int level, char *buffer, size_t size);

// Return tag_receive:
//  succes            :=    return len copied
//  EXFULL            :=    Buffer too long (out of MAX_BUF_SIZE), or not present
//  ENOMSG            :=    Tag not found
//  EBADRQC           :=    Permission invalid to execute the operation
//  EBADSLT           :=    asked level is over levelDeep
//  ERESTART          :=    Signal wake_up the thread
//  EUCLEAN           :=    Receved TBDE_AWAKE_ALL
// --
//  ENOSYS            :=    sysCall removed
//  EILSEQ            :=    Command not valid
int tag_receive(int tag, int level, char *buffer, size_t size);

// Return TBDE_AWAKE_ALL:
//  succes            :=    return 0
//  ENODATA           :=    Notting to be wake_up was found (all ok, just notification)
//  EBADE             :=    Permission invalid to execute the operation// --
// Return TBDE_REMOVE:
//  succes            :=    return 0
//  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
//  EBADE             :=    Permission invalid to execute the operation
//  EADDRINUSE        :=    Reader in wait on some level
// --
//  ENOSYS            :=    sysCall removed
//  ENOSR             :=    tag negative number
//  EILSEQ            :=    Command not valid
int tag_ctl(int tag, int command);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void tagGet_perror(int keyAsk, int commandAsk);
void tagSend_perror(int tag);
void tagRecive_perror(int tag);
void tagCtl_perror(int tag, int commandAsk);
#endif