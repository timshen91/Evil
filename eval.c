#include <assert.h>
#include "eval.h"
#include "structure.h"
#include "symbol.h"
#include "macro.h"
#include "error.h"
#include "environment.h"

Node * apply(Node * f, Node * args) {
	if (f->type != LIST_LAMBDA && f->type != PAIR_LAMBDA) {
		error("not callable");
		abort();
	}
	// TODO
	return NULL;
}

Node * eval(Node * expr, Env * env) {
	return expr;
	switch (expr->type) {
		case SYMBOL:
			return lookup(env, toSym(car(expr))->sym);
		case PAIR:
			error("cannot eval a pair");
			abort();
			break;
		case LIST: {
			Node * first = eval(car(expr), env);
			if (first->type == MACRO || first->type == BUILTIN) {
				Node * ret = transform(first, expr, env);
				return eval(ret, env);
			} else {
				Node * args = &empty;
				for (Node * iter = cdr(expr); iter->type == LIST; iter = cdr(iter)) {
					args = cons(eval(iter, env), args);
				}
				return apply(first, args);
			}
		}
		case EMPTY:
			error("cannot eval an empty list");
			abort();
			break;
		case VECTOR:
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
		case LIST_LAMBDA:
		case PAIR_LAMBDA:
		case MACRO:
		case BUILTIN:
			return expr;
		case DUMMY:
		case LISTELL:
		case VECTORELL:
		case MARG:
			assert(0);
			break;
	}
	abort();
	return NULL;
}
