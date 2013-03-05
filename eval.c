#include <assert.h>
#include "eval.h"
#include "structure.h"
#include "symbol.h"
#include "macro.h"
#include "error.h"
#include "environment.h"
#include "builtin.h"

Node * apply(Node * ff, Node * args) {
	if (ff->type != FIX_LAMBDA && ff->type != VAR_LAMBDA && ff->type != BUI_LAMBDA) {
		printf("%d\n", ff->type);
		error("not callable");
		abort();
	}
	if (ff->type == BUI_LAMBDA) {
		return callBuiltinLambda(ff, args);
	}
	LambdaNode * f = toLambda(ff);
	unsigned int argsLen = length(f->formal);
	unsigned int len = length(args);
	if ((f->type == FIX_LAMBDA && argsLen != len) || argsLen > len) {
		error("arguments mismatch");
		abort();
	}
	Env * env = newEnv(toLambda(f)->env);
	Node * fiter = f->formal;
	Node * aiter = args;
	while (fiter->type == LIST || fiter->type == PAIR) {
		updateEnv(env, toSym(car(fiter))->sym, car(aiter));
		fiter = cdr(fiter);
		aiter = cdr(aiter);
	}
	if (f->type == VAR_LAMBDA) {
		updateEnv(env, toSym(fiter)->sym, aiter);
	}
	Node * biter = f->body;
	Node * last;
	for (; biter->type == LIST; biter = cdr(biter)) {
		last = eval(car(biter), env);
	}
	return last;
}

Node * eval(Node * expr, Env * env) {
	if (expr == NULL) {
		return NULL;
	}
	switch (expr->type) {
		case SYMBOL:
			return lookup(env, toSym(expr)->sym);
		case PAIR:
			error("cannot eval a pair");
			abort();
			break;
		case LIST: {
			Node * first = eval(car(expr), env);
			if (first->type == MACRO || first->type == BUILTIN_MAC) {
				Node * ret = transform(first, expr, env);
				return eval(ret, env);
			} else {
				Node * args = &empty;
				Node ** last = &args;
				for (Node * iter = cdr(expr); iter->type == LIST; iter = cdr(iter)) {
					*last = cons(eval(car(iter), env), &empty);
					last = &cdr(*last);
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
		case FIX_LAMBDA:
		case VAR_LAMBDA:
		case MACRO:
		case BUILTIN_MAC:
		case BUI_LAMBDA:
		case UNSPECIFIED:
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
