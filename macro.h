#ifndef __MACRO_H__
#define __MACRO_H__

#include <stdbool.h>
#include "structure.h"

Node * frame[4096];
Node * sliceFrame[4096];
Node * sliceFrameLast[4096];

Node dummy;

bool match(Node *, Node *);
Node * transform(Node *, Node *, Env *);

#endif
