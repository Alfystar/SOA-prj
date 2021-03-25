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

#define SUBMODULE_NAME "AVL"
#define avl_Audit if (1)
#define printk_avl(str, ...) printk("[%s::%s]: " str, MODNAME, SUBMODULE_NAME, ##__VA_ARGS__)
#define printk_avlDB(str, ...) sysCall_Audit printk_avl(str, ##__VA_ARGS__)

typedef struct Tree *Tree;
typedef struct Node *Node;

// Comp is used to search node, must be the keySearc comparation function
typedef int (*compCallBack)(void *dataA, void *dataB); // return -1:a<b | 0:a==b | 1:a>b
typedef void (*printCallBack)(void *data);
typedef void (*freeDataCallBack)(void *data);
Tree Tree_New(compCallBack comp, printCallBack print, freeDataCallBack freeData);
void Tree_DelAll(Tree T);

Node Tree_Insert(Tree t, void *data); // if data already present, return node clone pointer
void *Tree_DeleteNode(Tree t, Node node);
Node Tree_SearchNode(Tree t, void *data);

Node Tree_FirstNode(Tree t);
Node Tree_LastNode(Tree t);

Node Tree_PrevNode(Tree t, Node n);
Node Tree_NextNode(Tree t, Node n);

void Tree_Print(Tree t);

void *Node_GetData(Node n);
#endif