#ifndef __EVAL_H__
#define __EVAL_H__

#include "symbol.h"

typedef struct Env Env;
typedef struct Node Node;
Node * eval(struct Node * expr, Env * env);

#endif
