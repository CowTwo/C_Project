/*
 * Main.c
 *
 *  Created on: 2017¦~9¤ë6¤é
 *      Author:
 */
#include <stdio.h>
#include <stdlib.h>
#include "MinHeap.h"

int main(void) {
	puts("!!! MinHeap Test !!!");
	minHeap hp = initMinHeap();
	insertNode(&hp, 5);
	insertNode(&hp, 4);
	insertNode(&hp, 3);
	insertNode(&hp, 2);
	insertNode(&hp, 7);
	insertNode(&hp, 0);
	insertNode(&hp, 6);
	insertNode(&hp, 7);

	levelorderTraversal(&hp);
	puts("\n");
	while (hp.size){
		deleteNode(&hp);
	}

	return EXIT_SUCCESS;
}


