#ifndef __EVAL_H__
#define __EVAL_H__

#include "symbol.h"

struct LL {
	unsigned int offset;
	struct LL * next;
} * lexStack[4096];

typedef struct Env {
	struct Env * parent;
} Env;

typedef struct Node Node;
Node * eval(struct Node * expr, Env * env);

#endif
