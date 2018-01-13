/*
 ============================================================================
 Name        : valgrind_example.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

void f(void)
{
 int* x = malloc(10 * sizeof(int));
 x[10] = 0;        // problem 1: heap block overrun
}                  // problem 2: memory leak -- x not freed

int main(void) {
	//puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
    f();
    return 0;
}
