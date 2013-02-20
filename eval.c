#include <assert.h>
#include "error.h"
#include "eval.h"
#include "memory.h"
#include "structure.h"
#include "symbol.h"
#include "error.h"

static Node * defineSyntaxPattern;
static Node * lambdaPattern;
static Node * frame[4096] = {0};
static Node * sliceFrame[4096] = {0};
static Node * sliceFrameLast[4096] = {0};
static unsigned int stack[2][4096];
static int top[2];

static int updateEnv(Env * env, unsigned int sym, Node * value) { // TODO
	return -1;
}

static void clear() {
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
	if (p->type == DUMMY || (p->type == EMPTY && v->type == EMPTY)) {
		return 0;
	} else if (p->type == OFFSET) {
		DUMP("temp");
		unsigned int offset = toOffset(p)->offset;
		if (toOffset(p)->depth == 0) {
			stack[0][top[0]++] = offset;
			frame[offset] = v;
		} else {
			if (sliceFrame[offset] == NULL) {
				stack[1][top[1]++] = offset;
				sliceFrame[offset] = sliceFrameLast[offset] = LIST1(v);
			} else {
				assert(cdr(sliceFrameLast[offset])->type == EMPTY);
				sliceFrameLast[offset] = cdr(sliceFrameLast[offset]) = LIST1(v);
			}
		}
	} else if (p->type == LIST && v->type == LIST) {
		DUMP("temp");
		unsigned plen = length(p);
		unsigned vlen = length(p);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen != vlen) {
			return -1;
		}
		while (piter->type == LIST) {
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
		}
	} else if (p->type == LISTELL && v->type == LIST) {
		unsigned plen = length(p);
		unsigned vlen = length(p);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen > vlen) {
			return -1;
		}
		int i = 0;
		if (match(piter->a, viter->a) != 0) {
			return -1;
		}
		for (i = 1; i < plen; i++) {
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
		}
		for (; i < vlen; i++) {
			viter = toPair(cdr(viter));
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
		}
	} else if (p->type == PAIR && (v->type == PAIR || v->type == LIST)) {
		DUMP("temp");
		unsigned plen = length(p);
		unsigned vlen = length(p);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen >= vlen) {
			return -1;
		}
		while (piter->type == PAIR) {
			if (match(piter->a, viter->a) != 0) {
				return -1;
			}
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
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

static int defineSyntax(Node * expr, Env * env) {
	match(defineSyntaxPattern, expr);
	unsigned long name = toSym(frame[1])->sym;
	Node * lit = frame[2];
	Node * ps = sliceFrame[0];
	Node * ts = sliceFrame[1];
	clear();
	if (lit->type != LIST) {
		return -1;
	}
	Macro * ret;
	if ((ret = newMacro(lit, ps, ts)) == NULL) {
		return -1;
	}
	if (updateEnv(env, name, (Node *)ret)) {
		return -1;
	}
	return 0;
}

static Node * evalLambda(Node * expr) {
	match(lambdaPattern, expr);
	Node * formal = frame[1];
	Node * body = frame[2];
	clear();
	if (formal -> type != EMPTY || formal->type != SYMBOL || formal->type != LIST || formal->type != PAIR) {
		return NULL;
	}
	return newLambda(formal, body);
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
	if (car(expr)->type == SYMBOL) {
		unsigned long sym = toSym(car(expr))->sym;
		if (sym == getSym("define-syntax")) {
			if (defineSyntax(expr, env)) {
				error("bad syntax");
				abort();
			}
		} else if (sym == getSym("defulat")){
		} else if (sym == getSym("lambda")) {
			Node * ret;
			if ((ret = evalLambda(expr)) == NULL) {
				error("bad lambda definition");
				abort();
			}
			return ret;
		}
		return expr;
	} else {
		// TODO
		return expr;
	}
	assert(0);
	return NULL;
}

void initEval() {
	// (`define-syntax` name
	//   (`syntax-rules` lit
	//                   ((ps ts) ...)))
	defineSyntaxPattern =
		LIST3(
			newSymbol("define-syntax"),
			newOffset(OFFSET, 1), // name
			LIST3(
				newSymbol("syntax-rules"),
				newOffset(OFFSET, 2), // lit, should be a LIST
				LIST2(
					LIST2(
						newOffset(0, 1), // ps ...
						newOffset(1, 1) // ts ...
					),
					newSymbol("...")
				)
			)
		);
	// (`lambda` formal body)
	lambdaPattern =
		LIST3(
			newSymbol("lambda"),
			newOffset(OFFSET, 1), // formal, EMPTY || SYMBOL || LIST || PAIR
			newOffset(OFFSET, 2) // body
		);
}
