#ifndef __ENV_H__
#define __ENV_H__

#include "symbol.h"

typedef struct Node Node;

typedef struct Env {
	struct Env * parent;
} Env;

Env * newEnv(Env *);
void updateEnv(Env * env, Symbol sym, Node * value);
Node * lookup(Env * env, Symbol sym);

#endif
