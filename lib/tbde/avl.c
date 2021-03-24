#include "avl.h"

avlNode *insert(avlNode *T, void *x, majorDataCompare_Callback mdC) {
  if (T == NULL) {
    T = (avlNode *)kzalloc(sizeof(avlNode), GFP_KERNEL | GFP_NOWAIT);
    T->data = x;
    T->left = NULL;
    T->right = NULL;
  } else if (mdC(x, T->data)) // insert in right subtree
  {
    T->right = insert(T->right, x, mdC);
    if (BF(T) == -2)
      if (mdC(x, T->right->data))
        T = RR(T);
      else
        T = RL(T);
  } else if (mdC(T->data > x)) {
    T->left = insert(T->left, x, mdC);
    if (BF(T) == 2)
      if (mdC(T->left->data, x))
        T = LL(T);
      else
        T = LR(T);
  }

  T->ht = height(T);

  return (T);
}

avlNode *delete (avlNode *T, void *x, majorDataCompare_Callback mdC) {
  avlNode *p;

  if (T == NULL) {
    return NULL;
  } else if (mdC(x, T->data)) // insert in right subtree
  {
    T->right = delete (T->right, x, mdC);
    if (BF(T) == 2)
      if (BF(T->left) >= 0)
        T = LL(T);
      else
        T = LR(T);
  } else if (mdC(T->data, x)) {
    T->left = delete (T->left, x, mdC);
    if (BF(T) == -2) // Rebalance during windup
      if (BF(T->right) <= 0)
        T = RR(T);
      else
        T = RL(T);
  } else {
    // data to be deleted is found
    if (T->right != NULL) { // delete its inorder succesor
      p = T->right;

      while (p->left != NULL)
        p = p->left;

      T->data = p->data;
      T->right = delete (T->right, p->data, mdC);

      if (BF(T) == 2) // Rebalance during windup
        if (BF(T->left) >= 0)
          T = LL(T);
        else
          T = LR(T);
    } else
      return (T->left);
  }
  T->ht = height(T);
  return (T);
}

int height(avlNode *T) {
  int lh, rh;
  if (T == NULL)
    return (0);

  if (T->left == NULL)
    lh = 0;
  else
    lh = 1 + T->left->ht;

  if (T->right == NULL)
    rh = 0;
  else
    rh = 1 + T->right->ht;

  if (lh > rh)
    return (lh);

  return (rh);
}

avlNode *rotateright(avlNode *x) {
  avlNode *y;
  y = x->left;
  x->left = y->right;
  y->right = x;
  x->ht = height(x);
  y->ht = height(y);
  return (y);
}

avlNode *rotateleft(avlNode *x) {
  avlNode *y;
  y = x->right;
  x->right = y->left;
  y->left = x;
  x->ht = height(x);
  y->ht = height(y);

  return (y);
}

// Right Right	==> X is rebalanced with a	simple	rotation rotate_Left
avlNode *RR(avlNode *T) {
  T = rotateleft(T);
  return (T);
}

// 	Left Left	==> X is rebalanced with a	simple	rotation rotate_Right
avlNode *LL(avlNode *T) {
  T = rotateright(T);
  return (T);
}

//	Right Left	==> X is rebalanced with a	double	rotation rotate_RightLeft
avlNode *LR(avlNode *T) {
  T->left = rotateleft(T->left);
  T = rotateright(T);

  return (T);
}

// 	Left Right	==> X is rebalanced with a	double	rotation rotate_LeftRight
avlNode *RL(avlNode *T) {
  T->right = rotateright(T->right);
  T = rotateleft(T);
  return (T);
}

int BF(avlNode *T) {
  int lh, rh;
  if (T == NULL)
    return (0);

  if (T->left == NULL)
    lh = 0;
  else
    lh = 1 + T->left->ht;

  if (T->right == NULL)
    rh = 0;
  else
    rh = 1 + T->right->ht;

  return (lh - rh);
}

void preorder(avlNode *T) {
  if (T != NULL) {
    printk("%d(Bf=%d)", T->data, BF(T));
    preorder(T->left);
    preorder(T->right);
  }
}

void inorder(avlNode *T) {
  if (T != NULL) {
    inorder(T->left);
    printk("%d(Bf=%d)", T->data, BF(T));
    inorder(T->right);
  }
}