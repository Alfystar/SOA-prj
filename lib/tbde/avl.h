/**
 * @file avl.h
 * @author Emanuele Alfano
 * @version 0.1
 * @date 2021-03-24
 *
 * @copyright Copyright (c) 2021
 *
 * @brief Small and smart library to implement generic AVL tree,
 * code based from the example foundable at:
 * https://rosettacode.org/wiki/AVL_tree/C
 */

#ifndef avl_h
#define avl_h

#include <globalDef.h>
#include <linux/slab.h>
#include <linux/stddef.h>

// Level 0 = no message
// Level 1 = err message
// Level 2 = Info messsage
// Level 3 = notice messsage
// Level 4 = dbg message
// Level 5 = all message
#define AVL_VerboseLevel 1
#define AVL_err _codeActive(1, AVL_VerboseLevel)
#define AVL_notice _codeActive(2, AVL_VerboseLevel)
#define AVL_info _codeActive(3, AVL_VerboseLevel)
#define AVL_Db _codeActive(4, AVL_VerboseLevel)

#define avl_err(str, ...)                                                                                              \
  do {                                                                                                                 \
    AVL_err printk_STD(KERN_ERR, "AVL", str, ##__VA_ARGS__);                                                           \
  } while (0)

#define avl_db(str, ...)                                                                                               \
  do {                                                                                                                 \
    AVL_Db printk_STD(KERN_DEBUG, "AVL", str, ##__VA_ARGS__);                                                          \
  } while (0)

// Comp is used to search node, must be the keySearc comparation function
typedef int (*compCallBack)(void *dataA, void *dataB); // return -1:a<b | 0:a==b | 1:a>b
typedef size_t (*printCallBack)(void *data, char *buf, int size);
typedef void (*freeDataCallBack)(void *data);

struct Node_ {
  struct Node_ *parent;
  struct Node_ *left;
  struct Node_ *right;
  void *data;
  int balance;
};
typedef struct Node_ *Node;

struct Tree {
  Node root;
  compCallBack comp;
  printCallBack print;
  freeDataCallBack freeData;
};
typedef struct Tree *Tree;

Tree Tree_New(compCallBack comp, printCallBack print, freeDataCallBack freeData);
void Tree_DelAll(Tree T);

Node Tree_Insert(Tree t, void *data); // if data already present, return node clone pointer
void *Tree_DeleteNode(Tree t, Node node);
Node Tree_SearchNode(Tree t, void *data);

Node Tree_FirstNode(Tree t);
Node Tree_LastNode(Tree t);

Node Tree_PrevNode(Tree t, Node n);
Node Tree_NextNode(Tree t, Node n);

size_t Tree_Print(Tree t, char *buf, size_t size);

void *Node_GetData(Node n);

#endif