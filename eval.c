#include <assert.h>
#include "error.h"
#include "eval.h"
#include "memory.h"
#include "structure.h"
#include "symbol.h"
#include "error.h"

static Node * frame[4096] = {0};
static Node * sliceFrame[4096] = {0};
static Node * sliceFrameLast[4096] = {0};
static unsigned int stack[2][4096];
static int top[2];

static void destory() {
	while (top[0] > 0) {
		top[0]--;
		frame[stack[0][top[0]]] = NULL;
	}
	while (top[1] > 0) {
		top[1]--;
		sliceFrame[stack[0][top[1]]] = NULL;
	}
}

static int match(Node * p, Node * v) {
	if (p->type == EMPTY && v->type == EMPTY) {
		return 0;
	} else if (p->type == OFFSET) {
		DUMP("temp");
		stack[0][top[0]++] = toOffset(p)->offset;
		frame[toOffset(p)->offset] = v;
	} else if (p->type == OFFSET_SLICE) {
		unsigned int offset = toOffset(p)->offset;
		if (sliceFrame[offset] == NULL) {
			stack[1][top[1]++] = offset;
			sliceFrame[offset] = sliceFrameLast[offset] = LIST1(v);
		} else {
			assert(toPair(sliceFrameLast[toOffset(p)->offset])->b->type == EMPTY);
			sliceFrameLast[offset] = toPair(sliceFrameLast[offset])->b = LIST1(v);
		}
	} else if (p->type == LIST && v->type == LIST) {
		DUMP("temp");
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (piter->len > viter->len) {
			return -1;
		}
		for (int i = 0; i < piter->len; i++) {
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
			piter = toPair(piter->b);
			viter = toPair(viter->b);
		}
	} else if (v->type == LIST) {
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (piter->len > viter->len || piter->len <= 0) {
			return -1;
		}
		int i = 0;
		if (match(piter->a, viter->a) != 0) {
			return -1;
		}
		for (i = 1; i < piter->len; i++) {
			piter = toPair(piter->b);
			viter = toPair(viter->b);
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
		}
		for (; i < viter->len; i++) {
			viter = toPair(viter->b);
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
		}
	} else if (p->type == PAIR && (v->type == PAIR || v->type == LIST)) {
		DUMP("temp");
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (piter->len > viter->len) {
			return -1;
		}
		for (int i = 0; i < piter->len; i++) {
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
			piter = toPair(piter->b);
			viter = toPair(viter->b);
		}
		match((Node *)piter, (Node *)viter);
	} else if (p->type == VECTOR && v->type == VECTOR) {
		DUMP("temp");
		int lenp = toVec(p)->len;
		int lenv = toVec(v)->len;
		if (lenp != lenv) {
			return -1;
		}
		Node ** vp = toVec(p)->vec;
		Node ** vv = toVec(v)->vec;
		for (int i = 0; i < lenp; i++) {
			if (match(vp[i], vv[i]) != 0) {
				return -1;
			}
		}
	} else if (p->type == VECTORELL && v->type == VECTOR) {
		int lenp = toVec(p)->len;
		int lenv = toVec(v)->len;
		if (lenp > lenv || lenp <= 0) {
			return -1;
		}
		Node ** vp = toVec(p)->vec;
		Node ** vv = toVec(v)->vec;
		int i;
		for (i = 0; i < lenp; i++) {
			if (match(vp[i], vv[i]) != 0) {
				return -1;
			}
		}
		for (; i < lenv; i++) {
			if (match(vp[lenp - 1], vv[i]) != 0) {
				return -1;
			}
		}
	} else if (!equal(v, p)) {
		DUMP("temp");
		return -1;
	}
	return 0;
}

static Node * defineSyntax(Node * expr) {
	return NULL;
}

Node * eval(Node * expr, Env * env) {
	return expr;
	if (expr->type != LIST) {
		if (expr->type == PAIR) {
			error("cannot eval a pair");
			abort();
		}
		return expr;
	}
	if (toPair(expr)->a->type == SYMBOL) {
		unsigned long sym = toSym(toPair(expr)->a)->sym;
		if (sym == getSym("define-syntax")) { // TODO
			defineSyntax(expr);
		} else {
		}
		return expr;
	} else {
		// TODO
		return expr;
	}
	assert(0);
	return NULL;
}
