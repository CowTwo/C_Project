/*
 ============================================================================
 Name        : min_heap_embedsys.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "min_heap_embedsys.h"

void initMinHeap(P_MIN_HEAP_T pMinHeap)
{
	pMinHeap->size=0;
}

void insertNode(P_MIN_HEAP_T pMinHeap, int data) {
	ASSERT(pMinHeap->size<MIN_HEAP_MAX_ELEM_NUM);

    NODE_T nd ;
    nd.data = data ;

    int i = (pMinHeap->size)++ ;
    while(i && nd.data < pMinHeap->elem[PARENT(i)].data) {
    	pMinHeap->elem[i] = pMinHeap->elem[PARENT(i)] ;
        i = PARENT(i) ;
    }
    pMinHeap->elem[i] = nd ;
}

void swap(P_NODE_T pN1, P_NODE_T pN2) {
    NODE_T temp = *pN1 ;
    *pN1 = *pN2 ;
    *pN2 = temp ;
}

void heapify(P_MIN_HEAP_T pMinHeap, int i) {
    int smallest = (LCHILD(i) < pMinHeap->size && pMinHeap->elem[LCHILD(i)].data < pMinHeap->elem[i].data) ? LCHILD(i) : i ;
    if(RCHILD(i) < pMinHeap->size && pMinHeap->elem[RCHILD(i)].data < pMinHeap->elem[smallest].data) {
        smallest = RCHILD(i) ;
    }
    if(smallest != i) {
        swap(&(pMinHeap->elem[i]), &(pMinHeap->elem[smallest])) ;
        heapify(pMinHeap, smallest) ;
    }
}

void deleteNode(P_MIN_HEAP_T pMinHeap) {
    if(pMinHeap->size) {
        printf("Deleting node %d\n\n", pMinHeap->elem[0].data) ;
        pMinHeap->elem[0] = pMinHeap->elem[--(pMinHeap->size)] ;
        heapify(pMinHeap, 0) ;
    } else {
        printf("\nMin Heap is empty!\n") ;
        ASSERT(0) ;
    }
}

void levelorderTraversal(P_MIN_HEAP_T pMinHeap) {
    int i ;
    for(i = 0; i < pMinHeap->size; i++) {
        printf("%d ", pMinHeap->elem[i].data) ;
    }
}

int main(void) {
	MIN_HEAP_T minHeap;

	initMinHeap(&minHeap);
	insertNode(&minHeap, 5);
	insertNode(&minHeap, 4);
	insertNode(&minHeap, 3);
	insertNode(&minHeap, 2);
	insertNode(&minHeap, 7);
	insertNode(&minHeap, 0);
	insertNode(&minHeap, 6);
	insertNode(&minHeap, 7);

	levelorderTraversal(&minHeap);
	puts("\n");
	while (minHeap.size){
		deleteNode(&minHeap);
	}

	return EXIT_SUCCESS;
}
