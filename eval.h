#ifndef __EVAL_H__
#define __EVAL_H__

typedef struct Env {
	struct Env * parent;
} Env;

typedef struct Node Node;
Node * eval(Node * expr, Env * env);

#endif
