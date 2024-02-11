#pragma once
// SPM.h : Include file for standard system include files,
// or project specific include files.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef void V;
typedef int I;
typedef char C;

#define SIZE	100
#define SSIZE	1024
#define MSIZE	1024
#define WKSIZE  (2*SIZE)

#define ER(m,v)	{fprintf(stderr,"%%ALLOC-FAILURE-%s-%02XH",m,v);exit(v);}

FILE* fi;
FILE* fo;

C* bf;
C* bf2;
C** stk;
C** mem;
C* zr;
C* one;
I cr;
I sp;

I mt[] = {	0,0, 0, 0, 0, 0, 0, 0, 0, 0,
			0,1, 2, 3, 4, 5, 6, 7, 8, 9,
			0,2, 4, 6, 8,10,12,14,16,18,
			0,3, 6, 9,12,15,18,21,24,27,
			0,4, 8,12,16,20,24,28,32,36,
			0,5,10,15,20,25,30,35,40,45,
			0,6,12,18,24,30,36,42,48,54,
			0,7,14,21,28,35,42,49,56,63,
			0,8,16,24,32,40,48,56,64,72,
			0,9,18,27,36,45,54,63,72,81 };

// TODO: Reference additional headers your program requires here.
