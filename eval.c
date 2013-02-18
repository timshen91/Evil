#include <assert.h>
#include "error.h"
#include "eval.h"
#include "memory.h"
#include "structure.h"
#include "symbol.h"
#include "error.h"

static unsigned int length(Node * l) {
	if (l == NULL || (l->type != LIST && l->type != PAIR)) {
		return 0;
	}
	return 1 + length(toPair(l)->b);
}

static int symInList(unsigned long sym, Node * l) {
	if (l == NULL) {
		return 0;
	}
	if (toPair(l)->a->type == SYMBOL && sym == (toSym(toPair(l)->a)->sym)) {
		return 1;
	}
	return symInList(sym, toPair(l)->b);
}

static unsigned long symk[4096];
static Node * symv[4096];
static int top = 0;
static int match(Node * lit, Node * p, Node * v) {
	if (p->type == SYMBOL) {
		if (!symInList(((SymNode *)p)->sym, lit)) {
			symk[top] = ((SymNode *)p)->sym;
			symv[top] = v;
			top++;
		} else {
			if (v->type != SYMBOL || toSym(v)->sym != toSym(p)->sym) {
				return -1;
			}
		}
	} else if (p->type == LIST && v->type == LIST) {
		if (length(p) > length(v)) {
			return -1;
		}
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		PairNode * last = NULL;
		assert(!(piter->a->type == SYMBOL && toSym(piter->a)->sym == getSym("...")));
		while (piter != NULL) {
			if (match(lit, piter->a, viter->a) != 0) {
				return -1;
			}
			last = piter;
			piter = toPair(piter->b);
			viter = toPair(viter->b);
			if (piter->a->type == SYMBOL && toSym(piter->a)->sym == getSym("...")) {
				assert(piter->b == NULL);
				while (viter != NULL) {
					if (match(lit, last->a, viter->a) != 0) {
						return -1;
					}
					viter = toPair(viter->b);
				}
			}
		}
	} else if (p->type == PAIR && (v->type == PAIR || v->type == LIST)) {
		if (length(p) > length(v)) {
			return -1;
		}
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		while (piter->type == PAIR) {
			if (match(lit, piter->a, viter->a) != 0) {
				return -1;
			}
			piter = toPair(piter->b);
			viter = toPair(viter->b);
		}
		match(lit, (Node *)piter, (Node *)viter);
	} else if (p->type == VECTOR && v->type == VECTOR) {
		if (toVec(p)->len > toVec(v)->len) {
			return -1;
		}
		int i = 0;
		Node ** vp = (Node **)(p + 1);
		Node ** vv = (Node **)(v + 1);
		assert(!(vp[0]->type == SYMBOL && toSym(vp[0])->sym == getSym("...")));
		while (i < toVec(p)->len) {
			if (match(lit, vp[i], vv[i]) != 0) {
				return -1;
			}
			i++;
			if (vp[i]->type == SYMBOL && toSym(vp[i])->sym == getSym("...")) {
				assert(i == toVec(p)->len - 1);
				int j = i;
				i--;
				while (j < toVec(v)->len) {
					if (match(lit, vp[i], vv[j]) != 0) {
						return -1;
					}
					j++;
				}
			}
		}
	}
	if (!equal(v, p)) {
		return -1;
	}
	return 0;
}

Node * eval(Node * expr, Env * env) {
	return expr;
	if (expr->type != LIST) { // NOTICE : eval a pair (not a list) will return itself.
		return expr;
	}
	if (toPair(expr)->a->type == SYMBOL) {
		unsigned long sym = toSym(toPair(expr)->a)->sym;
		switch (sym) { // TODO
			case SYM_DEFINE_SYNTAX:
				//int lit = {getSym("syntax-rules")};
				//match(lit, DEFINE_SYNTAX_PATTERNS, expr);
			case SYM_DEFINE:
			case SYM_LAMBDA:
			default:
				break;
		}
	} else {
		// TODO
	}
	assert(0);
	return NULL;
}
