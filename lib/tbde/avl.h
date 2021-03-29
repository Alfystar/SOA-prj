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

#define avl_Audit if (0)
#define printk_avl(str, ...) printk("[%s::%s]: " str, MODNAME, "AVL", ##__VA_ARGS__)
#define printk_avlDB(str, ...) avl_Audit printk_avl(str, ##__VA_ARGS__)

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