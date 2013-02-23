#include <assert.h>
#include <stddef.h>
#include "macro.h"
#include "memory.h"
#include "symbol.h"
#include "structure.h"
#include "error.h"

#define isEllipsis(node) ((node)->type == SYMBOL && toSym(node)->sym == getSym("..."))
#define toMarg(p) ((MargNode *)(p))

Node * frame[4096] = {0};
Node * sliceFrame[4096] = {0};
Node * sliceFrameLast[4096] = {0};
static unsigned int stack[2][4096];
static int top[2];

void matchClear() {
	while (top[0] > 0) {
		top[0]--;
		frame[stack[0][top[0]]] = NULL;
	}
	while (top[1] > 0) {
		top[1]--;
		sliceFrame[stack[0][top[1]]] = NULL;
	}
}

bool match(Node * p, Node * v) {
	if (p->type == DUMMY || (p->type == EMPTY && v->type == EMPTY)) {
		return true;
	} else if (p->type == MARG) {
		DUMP("temp");
		unsigned int offset = toMarg(p)->offset;
		if (toMarg(p)->depth == 0) {
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
		unsigned int plen = length(p);
		unsigned int vlen = length(p);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen != vlen) {
			return false;
		}
		while (piter->type == LIST) {
			if (match(piter->a, viter->a) != 0) {
				return false;
			}
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
		}
	} else if (p->type == LISTELL && v->type == LIST) {
		unsigned int plen = length(p);
		unsigned int vlen = length(p);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen > vlen) {
			return false;
		}
		int i = 0;
		if (match(piter->a, viter->a) != 0) {
			return false;
		}
		for (i = 1; i < plen; i++) {
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
			if (match(piter->a, viter->a) != 0) {
				return false;
			}
		}
		for (; i < vlen; i++) {
			viter = toPair(cdr(viter));
			if (match(piter->a, viter->a) != 0) {
				return false;
			}
		}
	} else if (p->type == PAIR && (v->type == PAIR || v->type == LIST)) {
		DUMP("temp");
		unsigned int plen = length(p);
		unsigned int vlen = length(p);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen >= vlen) {
			return false;
		}
		while (piter->type == PAIR) {
			if (match(piter->a, viter->a) != 0) {
				return false;
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
			return false;
		}
		Node ** vp = toVec(p)->vec;
		Node ** vv = toVec(v)->vec;
		for (int i = 0; i < lenp; i++) {
			if (match(vp[i], vv[i]) != 0) {
				return false;
			}
		}
	} else if (p->type == VECTORELL && v->type == VECTOR) {
		int lenp = toVec(p)->len;
		int lenv = toVec(v)->len;
		if (lenp > lenv || lenp <= 0) {
			return false;
		}
		Node ** vp = toVec(p)->vec;
		Node ** vv = toVec(v)->vec;
		int i;
		for (i = 0; i < lenp; i++) {
			if (match(vp[i], vv[i]) != 0) {
				return false;
			}
		}
		for (; i < lenv; i++) {
			if (match(vp[lenp - 1], vv[i]) != 0) {
				return false;
			}
		}
	} else if (!equal(v, p)) {
		DUMP("temp");
		return false;
	}
	return true;
}

Node * newMarg(unsigned int offset, unsigned int depth) {
	MargNode * ret = alloc(sizeof(MargNode));
	ret->type = MARG;
	ret->depth = depth;
	ret->offset = offset;
	return (Node *)ret;
}

static bool symInList(Symbol sym, Node * l) {
	if (l->type == EMPTY) {
		return false;
	}
	return (car(l)->type == SYMBOL && toSym(car(l))->sym == sym) || symInList(sym, cdr(l));
}

static unsigned int varn;
static Symbol var[4096];
static unsigned int varDepth[4096];
int compilePattern(Node * lit, Node ** pp) {
	static int ellipsisDepth = 0;
	Node * p = *pp;
	switch (p->type) {
		case SYMBOL:
			if (!symInList(toSym(p)->sym, lit)) {
				int i;
				for (i = 0; i < varn; i++) {
					if (var[i] == toSym(p)->sym) {
						break;
					}
				}
				if (i != varn) {
					return -1;
				}
				var[varn] = toSym(p)->sym;
				varDepth[varn] = ellipsisDepth;
				varn++;
				*pp = newMarg(i, ellipsisDepth);
			}
			break;
		case PAIR:
			if (compilePattern(lit, &car(p))) {
				return -1;
			}
			if (compilePattern(lit, &cdr(p))) {
				return -1;
			}
			break;
		case LIST: {
				Node * next = cdr(p);
				if (next->type == LIST && isEllipsis(car(next))) {
					if (cdr(next)->type != EMPTY) {
						return -1;
					}
					ellipsisDepth++;
					int ret = compilePattern(lit, &car(p));
					ellipsisDepth--;
					if (ret) {
						return -1;
					}
					cdr(p) = p;
					cdr(p) = &empty;
					toPair(p)->type = LISTELL;
				} else {
					if (compilePattern(lit, &car(p))) {
						return -1;
					}
					compilePattern(lit, &cdr(p));
					if (cdr(cdr(p))->type == LISTELL) {
						toPair(p)->type = LISTELL;
					}
				}
			}
			break;
		case VECTOR:
			if (toVec(p)->len > 0) {
				int i;
				for (i = 0; i < toVec(p)->len - 1; i++) {
					Node * now = toVec(p)->vec[i];
					if (isEllipsis(now)) {
						return -1;
					}
				}
				Node * now = toVec(p)->vec[i];
				if (isEllipsis(now)) {
					p->type = VECTORELL;
					toVec(p)->len--;
				}
				for (i = 0; i < toVec(p)->len - 1; i++) {
					if (compilePattern(lit, &toVec(p)->vec[i])) {
						return -1;
					}
				}
				ellipsisDepth++;
				int ret = compilePattern(lit, &toVec(p)->vec[i]);
				ellipsisDepth--;
				if (ret) {
					return -1;
				}
			}
			break;
		case EMPTY:
		case BOOLLIT:
		case NUMLIT:
		case CHARLIT:
		case STRLIT:
			break;
		case DUMMY:
		case LIST_LAMBDA:
		case PAIR_LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case REF:
		case MARG:
			assert(0);
			break;
	}
	return 0;
}

static int compileTemplate(Node ** tt) {
	static bool ellipsisDepth = 0;
	Node * t = *tt;
	switch (t->type) {
		case SYMBOL: {
				if (isEllipsis(t)) {
					return -1;
				}
				int i;
				for (i = 0; i < varn; i++) {
					if (var[i] == toSym(t)->sym) {
						break;
					}
				}
				if (i < varn) { // arg
					*tt = newMarg(i, ellipsisDepth);
				} else { // free var
					if (ellipsisDepth > 0) { 
						return -1;
					}
					*tt = newRef(toSym(t)->sym);
				}
			}
			break;
		case PAIR:
		case LIST: {
				if (car(t)->type == SYMBOL) {
					ellipsisDepth = 0;
					Node * next = cdr(t);
					while (next->type == LIST && isEllipsis(car(next))) {
						ellipsisDepth++;
						next = cdr(next);
					}
					cdr(t) = next;
				}
				int ret = compileTemplate(&car(t));
				if (ret) {
					return -1;
				}
				if (compileTemplate(&cdr(t))) {
					return -1;
				}
			}
			break;
		case VECTOR: {
				int i = 0;
				int j = 0;
				while (i < toVec(t)->len) {
					int k = i + 1;
					if (toVec(t)->vec[i]->type == SYMBOL) {
						while (k < toVec(t)->len && isEllipsis(toVec(t)->vec[k])) {
							k++;
						}
						ellipsisDepth = k - i - 1;
					}
					if (compileTemplate(&toVec(t)->vec[i])) {
						return -1;
					}
					toVec(t)->vec[j++] = toVec(t)->vec[i];
					i = k;
				}
				toVec(t)->len = j;
			}
			break;
		case EMPTY:
		case BOOLLIT:
		case NUMLIT:
		case CHARLIT:
		case STRLIT:
			break;
		case DUMMY:
		case LIST_LAMBDA:
		case PAIR_LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case REF:
		case MARG:
			assert(0);
			break;
	}
	return 0;
}

Macro * newMacro(Node * lit, Node * ps, Node * ts, Env * env) {
	unsigned int len = length(ps);
	assert(len == length(ts));
	Macro * ret = alloc(sizeof(Macro) + len * sizeof(*ret->rules));
	ret->type = MACRO;
	ret->env = env;
	ret->ruleLen = len;
	for (int i = 0; i < len; i++) {
		varn = 1; // 0 for DUMMY
		if (compilePattern(lit, &car(ps))) {
			return NULL;
		}
		if (compileTemplate(&car(ts))) {
			return NULL;
		}
		ret->rules[i].ptrn = car(ps);
		ret->rules[i].tmpl = car(ts);
	}
	return ret;
}

Node * transform(Macro * mac, Node * expr) {
	for (int i = 0; i < mac->ruleLen; i++) {
		if (match(mac->rules[i]->ptrn, expr)) {
			// TODO
		}
	}
	return NULL;
}
