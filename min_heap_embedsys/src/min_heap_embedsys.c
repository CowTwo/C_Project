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

void insertNode(P_MIN_HEAP_T *hp, int data) {
    if(hp->size) {
        hp->elem = realloc(hp->elem, (hp->size + 1) * sizeof(node)) ;
    } else {
        hp->elem = malloc(sizeof(node)) ;
    }

    node nd ;
    nd.data = data ;

    int i = (hp->size)++ ;
    while(i && nd.data < hp->elem[PARENT(i)].data) {
        hp->elem[i] = hp->elem[PARENT(i)] ;
        i = PARENT(i) ;
    }
    hp->elem[i] = nd ;
}


int main(void) {
	MIN_HEAP_T minHeap;

	initMinHeap(&minHeap);

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
