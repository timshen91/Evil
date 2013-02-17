#include <assert.h>
#include "error.h"
#include "eval.h"
#include "memory.h"
#include "structure.h"
#include "symbol.h"

#define error(s) { fprintf(stderr, "%s\n", s); }

//static unsigned int listLength(PairNode * l) {
//	if (l == NULL) {
//		return 0;
//	}
//	return 1 + listLength((PairNode *)l->b);
//}

Node * eval(Node * expr, Env * env) {
	if (expr->type != LIST) { // NOTICE : eval a pair (not a list) will return itself.
		return expr;
	}
	PairNode * now = (PairNode *)expr;
	if (now->a->type == SYMBOL) {
		unsigned long sym = ((SymNode *)now->a)->sym;
		switch (sym) { // TODO
			case SYM_DEFINE_SYNTAX:
			case SYM_DEFINE:
			case SYM_LAMBDA:
			default:
				break;
		}
	} else {
		// TODO
	}
	assert(0);
}
