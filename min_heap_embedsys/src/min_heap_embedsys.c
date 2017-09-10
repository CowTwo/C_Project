/*
 ============================================================================
 Name        : min_heap_embedsys.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include "min_heap_embedsys.h"

void initMinHeap(P_MIN_HEAP_T pMinHeap)
{
	pMinHeap->size=0;
}

void insertNode(P_MIN_HEAP_T pMinHeap, INT_32 data)
{
	ASSERT((pMinHeap->size<MIN_HEAP_MAX_ELEM_NUM));

    NODE_T nd ;
    nd.data = data ;

    INT_32 i = (pMinHeap->size)++ ;
    while(i && nd.data < pMinHeap->elem[PARENT(i)].data) {
    	pMinHeap->elem[i] = pMinHeap->elem[PARENT(i)] ;
        i = PARENT(i) ;
    }
    pMinHeap->elem[i] = nd ;
}

void swap(P_NODE_T pN1, P_NODE_T pN2)
{
    NODE_T temp = *pN1 ;
    *pN1 = *pN2 ;
    *pN2 = temp ;
}

void heapify(P_MIN_HEAP_T pMinHeap, UINT_32 i)
{
    int smallest = (LCHILD(i) < pMinHeap->size && pMinHeap->elem[LCHILD(i)].data < pMinHeap->elem[i].data) ? LCHILD(i) : i ;
    if(RCHILD(i) < pMinHeap->size && pMinHeap->elem[RCHILD(i)].data < pMinHeap->elem[smallest].data) {
        smallest = RCHILD(i) ;
    }
    if(smallest != i) {
        swap(&(pMinHeap->elem[i]), &(pMinHeap->elem[smallest])) ;
        heapify(pMinHeap, smallest) ;
    }
}

void deleteMinNode(P_MIN_HEAP_T pMinHeap)
{
    if(pMinHeap->size) {
        DBGPRINT(("Deleting min node(data=%d)\n", pMinHeap->elem[0].data));
        pMinHeap->elem[0] = pMinHeap->elem[--(pMinHeap->size)] ;
        heapify(pMinHeap, 0) ;
    } else {
    	DBGPRINT(("\nMin Heap is empty!\n"));
        ASSERT((0)) ;
    }
}

void deleteIndexedNode(P_MIN_HEAP_T pMinHeap, UINT_32 i)
{
	ASSERT((i<MIN_HEAP_MAX_ELEM_NUM));
	DBGPRINT(("Deleting node(index=%d, data=%d)\n", i, pMinHeap->elem[i].data));
    pMinHeap->elem[i] = pMinHeap->elem[--(pMinHeap->size)] ;

    // Making sure that heap property is also satisfied
    for(INT_32 j = (pMinHeap->size - 1) / 2; j >= 0; j--) {
        heapify(pMinHeap, j) ;
    }
}

void levelorderTraversal(P_MIN_HEAP_T pMinHeap)
{
    UINT_32 i ;
    DBGPRINT(("Show the heap in levelorderTraversal\n"));
    for(i = 0; i < pMinHeap->size; i++) {
        DBGPRINT(("%d ", pMinHeap->elem[i].data));
    }
    DBGPRINT(("\n"));
}

