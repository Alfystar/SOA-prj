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
 * https://www.thecrazyprogrammer.com/2014/03/c-program-for-avl-tree-implementation.html
 */

#ifndef avl_h
#define avl_h

#include <linux/slab.h>
#include <linux/stddef.h>

typedef struct avlNode {
  void *data;
  struct avlNode *left, *right;
  int ht;
} avlNode;

typedef int (*majorDataCompare_Callback)(void *a, void *b); // return 1 if a > b

avlNode *insert(avlNode *T, void *x, majorDataCompare_Callback mdC);
avlNode *delete (avlNode *T, void *x, majorDataCompare_Callback mdC);
void preorder(avlNode *T); // Print bfs
void inorder(avlNode *T);  // Print dfs

avlNode *RR(avlNode *T); //   Right Right	==> X is rebalanced with a	simple	rotation rotate_Left
avlNode *LL(avlNode *T); //   Left Left	==> X is rebalanced with a	simple	rotation rotate_Right
avlNode *LR(avlNode *T); //	Right Left	==> X is rebalanced with a	double	rotation rotate_RightLeft
avlNode *RL(avlNode *T); // 	Left Right	==> X is rebalanced with a	double	rotation rotate_LeftRight

// sub-Tree operation
avlNode *rotateright(avlNode *T);
avlNode *rotateleft(avlNode *T);
int BF(avlNode *T); // calculate the difference of level between left and right subtrees
int height(avlNode *T);

#endif