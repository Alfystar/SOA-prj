#ifndef GLOBALDEF_H
#define GLOBALDEF_H
#include <linux/printk.h>

#define MODNAME "TAG_DataExchange"

#define printk_STD(kernLev, SUB_MODULE, str, ...) printk(kernLev "[%s::%s]: " str, MODNAME, SUB_MODULE, ##__VA_ARGS__)
// printk(kernLev "[%s::%s]: " str, MODNAME, SUB_MODULE, ##__VA_ARGS__)

// Level used from parametric printk
// Level 0 = no message
// Level 1 = err message
// Level 2 = Info messsage
// Level 3 = notice messsage
// Level 4 = dbg message
// Level 5 = all message
#define _codeActive(lv, VermoseLevel) if (lv <= VermoseLevel)
#endif