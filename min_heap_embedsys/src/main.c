/*
 * main.c
 *
 *  Created on: 2017¦~9¤ë10¤é
 *      Author: Thx2OpenSrc
 */

#include <stdio.h>
#include <stdlib.h>
#include "min_heap_embedsys.h"

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

	deleteIndexedNode(&minHeap, 2);
	DBGPRINT(("\n"));
	levelorderTraversal(&minHeap);
	DBGPRINT(("\n"));
	while (minHeap.size){
		deleteMinNode(&minHeap);
	}

	return EXIT_SUCCESS;
}
