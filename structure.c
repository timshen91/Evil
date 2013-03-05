#include <string.h>
#include <assert.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"
#include "complex.h"

Node * newSymbol(const char * s) {
	SymNode * ret = alloc(sizeof(SymNode));
	ret->type = SYMBOL;
	ret->sym = getSym(s);
	ret->fromLevel = 0;
	return (Node *)ret;
}

Node * newVector(Node ** vec, unsigned int len) {
	VecNode * ret = alloc(sizeof(VecNode));
	ret->vec = alloc(sizeof(Node *) * len);
	memcpy(ret->vec, vec, sizeof(Node *) * len);
	ret->type = VECTOR;
	ret->len = len;
	return (Node *)ret;
}

Node * newString(const char * s, unsigned int len) {
	StringNode * ret = alloc(sizeof(StringNode));
	ret->str = alloc((size_t)len);
	memcpy(ret->str, s, len);
	ret->type = STRING;
	ret->len = len;
	return (Node *)ret;
}

Node * newBool(bool b) {
	if (b) {
		return &boolTrue;
	} else {
		return &boolFalse;
	}
}

Node * newChar(char ch) {
	CharNode * ret = alloc(sizeof(CharNode));
	ret->type = CHAR;
	ret->value = ch;
	return (Node *)ret;
}

Node * cons(Node * a, Node * b) {
	PairNode * ret = alloc(sizeof(PairNode));
	if (b->type == EMPTY || b->type == LIST) {
		ret->type = LIST;
	} else {
		ret->type = PAIR;
	}
	ret->a = a;
	ret->b = b;
	return (Node *)ret;
}

Node * newLambda(Node * formal, Node * body, Env * env) {
	Node * iter = formal;
	for (; iter->type == LIST || iter->type == PAIR; iter = cdr(iter)) {
		if (car(iter)->type != SYMBOL) {
			return NULL;
		}
	}
	if (iter->type != EMPTY) {
		if (iter->type != SYMBOL) {
			return NULL;
		}
	}
	LambdaNode * ret = alloc(sizeof(LambdaNode));
	ret->type = (formal->type == SYMBOL || formal->type == PAIR) ? VAR_LAMBDA : FIX_LAMBDA;
	ret->formal = formal;
	ret->env = env;
	ret->body = body;
	return (Node *)ret;
}

bool equal(Node * a, Node * b) {
	if (a->type != b->type) {
		return false;
	}
	switch (a->type) {
		case EMPTY:
			return true;
		case SYMBOL:
			return toSym(a)->sym == toSym(b)->sym;
		case LIST:
		case PAIR:
			return equal(car(a), car(b)) && equal(cdr(a), cdr(b));
		case VECTOR: {
			if (toVec(a)->len != toVec(b)->len) {
				return false;
			}
			Node ** va = toVec(a)->vec;
			Node ** vb = toVec(b)->vec;
			for (int i = 0; i < toVec(a)->len; i++) {
				if (!equal(va[i], vb[i])) {
					return false;
				}
			}
			return 1;
		}
		case BOOL:
			return a == b;
		case COMPLEX: // FIXME
			if (toComplex(a)->exact && toComplex(b)->exact) {
				return toComplex(a)->nu == toComplex(b)->nu && toComplex(a)->de == toComplex(b)->de;
			}
			return toComplex(a)->nu / toComplex(a)->de == toComplex(b)->nu / toComplex(b)->de;
		case CHAR:
			return toChar(a)->value == toChar(b)->value;
		case STRING:
			if (toString(a)->len != toString(b)->len) {
				return false;
			}
			return memcmp(toString(a)->str, toString(b)->str, toString(a)->len) == 0;
		case FIX_LAMBDA:
		case VAR_LAMBDA:
			return a == b;
		default:
			return false;
	}
	assert(0);
	return false;
}

unsigned int length(Node * l) {
	if (l->type != LIST && l->type != PAIR) {
		return 0;
	}
	return 1 + length(cdr(l));
}
