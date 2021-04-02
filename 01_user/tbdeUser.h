#ifndef tbdeUser_h
#define tbdeUser_h

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
//  EBADRQC           :=    Permission Wrong parameter
//  EBADR             :=    Key already in use
// --
// Return OPEN:
//  succes            :=    tag value
//  EBADSLT           :=    asked key is TBDE_IPC_PRIVATE
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENOMSG            :=    Key not found
// --
//  EILSEQ            :=    Command not valid
int tag_get(int key, int command, int permission);


int tag_send(int tag, int level, char *buffer, size_t size);


int tag_receive(int tag, int level, char *buffer, size_t size);


// Return AWAKE_ALL:
//  succes            :=    return 0
// --
// Return TBDE_REMOVE:
//  succes            :=    return 0
//  ENOSR             :=    tag negative number
//  EBADRQC           :=    Permission invalid to execute the operation
//  ENODATA           :=    Notting to be deleted was found (all ok, just notification)
// --
//  EILSEQ            :=    Command not valid
int tag_ctl(int tag, int command);

#endif