/*
 * min_heap_embedsys.h
 *
 *  Created on: 2017¦~9¤ë7¤é
 *      Author:
 */

#ifndef MIN_HEAP_EMBEDSYS_H_
#define MIN_HEAP_EMBEDSYS_H_

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
    int data ;
} NODE_T, *P_NODE_T ;

typedef struct _MIN_HEAP_T {
    int size ;
    node elem[MIN_HEAP_MAX_ELEM_NUM] ;
} MIN_HEAP_T, *P_MIN_HEAP_T ;


#endif /* MIN_HEAP_EMBEDSYS_H_ */
