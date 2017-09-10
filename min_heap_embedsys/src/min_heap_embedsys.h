/*
 * min_heap_embedsys.h
 *
 *  Created on: 2017¦~9¤ë7¤é
 *      Author:
 */

#ifndef MIN_HEAP_EMBEDSYS_H_
#define MIN_HEAP_EMBEDSYS_H_
#include "glue.h"

/**
Since in C array indexing begins at index 0, given a node at index i,
its left child shall be at (2 * i + 1) and
right child shall be at (2 * i + 2).
Also, given a node at index i, its parent node shall be at index (i / 2).
**/
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

#define MIN_HEAP_MAX_ELEM_NUM 16

typedef struct _NODE_T {
    INT_32 data ;
} NODE_T, *P_NODE_T ;

typedef struct _MIN_HEAP_T {
    UINT_32 size ;
    NODE_T elem[MIN_HEAP_MAX_ELEM_NUM] ;
} MIN_HEAP_T, *P_MIN_HEAP_T ;

void initMinHeap(P_MIN_HEAP_T pMinHeap);
void insertNode(P_MIN_HEAP_T pMinHeap, INT_32 data);
void deleteMinNode(P_MIN_HEAP_T pMinHeap);
void deleteIndexedNode(P_MIN_HEAP_T pMinHeap, UINT_32 i);
void levelorderTraversal(P_MIN_HEAP_T pMinHeap);

#endif /* MIN_HEAP_EMBEDSYS_H_ */
