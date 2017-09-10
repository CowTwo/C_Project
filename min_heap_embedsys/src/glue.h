/*
 * glue.h
 *
 *  Created on: 2017¦~9¤ë10¤é
 *      Author:
 */

#ifndef GLUE_H_
#define GLUE_H_
#include <stdio.h>
#include <stdlib.h>

#define UINT_32 unsigned int
#define INT_32	int

#define DBGPRINT(_fmt) 	printf _fmt
#define ASSERT(expr)	\
	if (!expr){		\
		DBGPRINT(("ASSERT FAIL\n"));	\
		while(1);	\
	}

#endif /* GLUE_H_ */
