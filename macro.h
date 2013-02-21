#ifndef __MACRO_H__
#define __MACRO_H__

#include <stdbool.h>
#include "structure.h"

typedef struct MargNode {
	enum NodeType type;
	unsigned int depth;
	unsigned int offset;
} MargNode;

typedef struct Macro {
	enum NodeType type;
	Env * env;
	unsigned int ruleLen;
	struct {
		Node * ptrn;
		Node * tmpl;
	} rules[];
} Macro;

Node * frame[4096];
Node * sliceFrame[4096];
Node * sliceFrameLast[4096];

bool match(Node * p, Node * v);
void matchClear(); // every time called match
Node * newMarg(unsigned int, unsigned int);
Macro * newMacro(Node *, Node *, Node *, Env *);
//Node * transform(Node *);

#endif
