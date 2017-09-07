/*
 * MinHeap.h
 *
 *  Created on: 2017¦~9¤ë4¤é
 *      Author: Refer to https://github.com/robin-thomas/min-heap/blob/master/minHeap.c
 */

#ifndef MINHEAP_H_
#define MINHEAP_H_

/**
Since in C array indexing begins at index 0, given a node at index i,
its left child shall be at (2 * i + 1) and
right child shall be at (2 * i + 2).
Also, given a node at index i, its parent node shall be at index (i / 2).
**/
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

typedef struct node {
    int data ;
} node ;

typedef struct minHeap {
    int size ;
    node *elem ;
} minHeap ;

minHeap initMinHeap();
void insertNode(minHeap *hp, int data);
void deleteNode(minHeap *hp);
void levelorderTraversal(minHeap *hp);

#endif /* MINHEAP_H_ */
