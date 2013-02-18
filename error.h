#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdio.h>
#include <setjmp.h>

//#define DUMP(s) printf("%s : %d : %s\n", __FILE__, __LINE__, s)
#define DUMP(s) 

#define error(s) { fprintf(stderr, "%s\n", s); }
#define abort() longjmp(jmpBuff, 1)

jmp_buf jmpBuff;

#endif
