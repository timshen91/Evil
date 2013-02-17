#ifndef __ERROR_H__
#define __ERROR_H__

#include <setjmp.h>

#define abort() longjmp(jmpBuff, 1)

jmp_buf jmpBuff;

#endif
