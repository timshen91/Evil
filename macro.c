#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "macro.h"
#include "memory.h"
#include "symbol.h"
#include "structure.h"
#include "error.h"
#include "environment.h"
#include "eval.h"

#define isEllipsis(node) ((node)->type == SYMBOL && toSym(node)->sym == getSym("..."))
#define toMarg(p) ((MargNode *)(p))
#define toMacro(p) ((Macro *)(p))
#define toBuil(p) ((Builtin *)(p))
#define LISTELL1(a) consEll((a), &empty)
#define LISTELL2(a, b) consEll((a), LISTELL1((b)))
#define LISTELL3(a, b, c) consEll((a), LISTELL2((b), (c)))
#define LISTELL4(a, b, c, d) consEll((a), LISTELL3((b), (c), (d)))

Node dummy = {.type = DUMMY};

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

typedef struct Builtin {
	enum NodeType type;
	Node * (*f)(Node *, Env *);
} Builtin;

Node * frame[4096] = {0};
Node * sliceFrame[4096];
Node * sliceFrameLast[4096];
static unsigned int stack[2][4096];
static int top[2];

unsigned int lengthEll(Node * l) {
	if (l->type != LISTELL) {
		return 0;
	}
	return 1 + lengthEll(cdr(l));
}

static bool matchImpl(Node * p, Node * v) {
	if (p->type == DUMMY) {
		return true;
	} else if (p->type == MARG) {
		DUMP("temp");
		unsigned int offset = toMarg(p)->offset;
		if (toMarg(p)->depth == 0) {
			stack[0][top[0]++] = offset;
			frame[offset] = v;
		} else {
			if (sliceFrame[offset] == &empty) {
				stack[1][top[1]++] = offset;
				sliceFrame[offset] = sliceFrameLast[offset] = cons(v, &empty);
			} else {
				assert(cdr(sliceFrameLast[offset])->type == EMPTY);
				sliceFrameLast[offset] = cdr(sliceFrameLast[offset]) = LIST1(v);
			}
		}
	} else if (p->type == LIST && v->type == LIST) {
		DUMP("temp");
		unsigned int plen = length(p);
		unsigned int vlen = length(v);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen != vlen) {
			return false;
		}
		while (piter->type == LIST) {
			if (!matchImpl(piter->a, viter->a)) {
				return false;
			}
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
		}
	} else if (p->type == LISTELL && v->type == LIST) {
		unsigned int plen = lengthEll(p);
		unsigned int vlen = length(v);
		if (plen - 1 > vlen) {
			return false;
		}
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		int i = 0;
		for (i = 0; i < plen - 1; i++) {
			if (!matchImpl(piter->a, viter->a)) {
				return false;
			}
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
		}
		for (; i < vlen; i++) {
			if (!matchImpl(piter->a, viter->a)) {
				return false;
			}
			viter = toPair(cdr(viter));
		}
	} else if (p->type == PAIR && (v->type == PAIR || v->type == LIST)) {
		DUMP("temp");
		unsigned int plen = length(p);
		unsigned int vlen = length(v);
		PairNode * piter = toPair(p);
		PairNode * viter = toPair(v);
		if (plen > vlen) {
			return false;
		}
		while (piter->type == PAIR) {
			if (!matchImpl(piter->a, viter->a)) {
				return false;
			}
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
		}
		matchImpl((Node *)piter, (Node *)viter);
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
			if (!matchImpl(vp[i], vv[i])) {
				return false;
			}
		}
	} else if (p->type == VECTORELL && v->type == VECTOR) {
		int lenp = toVec(p)->len;
		int lenv = toVec(v)->len;
		if (lenp <= 0 || lenp - 1 > lenv) {
			return false;
		}
		Node ** vp = toVec(p)->vec;
		Node ** vv = toVec(v)->vec;
		int i;
		for (i = 0; i < lenp - 1; i++) {
			if (!matchImpl(vp[i], vv[i])) {
				return false;
			}
		}
		for (; i < lenv; i++) {
			if (!matchImpl(vp[lenp - 1], vv[i])) {
				return false;
			}
		}
	} else if (!equal(v, p)) {
		DUMP("temp");
		return false;
	}
	return true;
}

bool match(Node * p, Node * v) {
	while (top[0] > 0) {
		top[0]--;
		frame[stack[0][top[0]]] = NULL;
	}
	while (top[1] > 0) {
		top[1]--;
		sliceFrame[stack[0][top[1]]] = sliceFrameLast[stack[0][top[1]]] = &empty;
	}
	return matchImpl(p, v);
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

static unsigned int varTop;
static Symbol var[4096];
static unsigned int varDepth[4096];
static int ellipsisDepth = 0;
static void compilePattern(Node * lit, Node ** pp) {
	Node * p = *pp;
	switch (p->type) {
		case SYMBOL:
			if (!symInList(toSym(p)->sym, lit)) {
				int i;
				for (i = 0; i < varTop; i++) {
					if (var[i] == toSym(p)->sym) {
						break;
					}
				}
				if (i != varTop) {
					error("compile pattern");
					abort();
				}
				var[varTop] = toSym(p)->sym;
				varDepth[varTop] = ellipsisDepth;
				varTop++;
				*pp = newMarg(i, ellipsisDepth);
			}
			break;
		case PAIR:
			compilePattern(lit, &car(p));
			compilePattern(lit, &cdr(p));
			break;
		case LIST: {
				Node * next = cdr(p);
				if (next->type == LIST && isEllipsis(car(next))) {
					if (cdr(next)->type != EMPTY) {
						error("compile pattern");
						abort();
					}
					ellipsisDepth++;
					compilePattern(lit, &car(p));
					ellipsisDepth--;
					cdr(p) = p;
					cdr(p) = &empty;
					toPair(p)->type = LISTELL;
				} else {
					compilePattern(lit, &car(p));
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
						error("compile pattern");
						abort();
					}
				}
				Node * now = toVec(p)->vec[i];
				if (isEllipsis(now)) {
					p->type = VECTORELL;
					toVec(p)->len--;
				}
				for (i = 0; i < toVec(p)->len - 1; i++) {
					compilePattern(lit, &toVec(p)->vec[i]);
				}
				ellipsisDepth++;
				compilePattern(lit, &toVec(p)->vec[i]);
				ellipsisDepth--;
			}
			break;
		case DUMMY:
		case EMPTY:
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
			break;
		case FIX_LAMBDA:
		case VAR_LAMBDA:
		case LISTELL:
		case VECTORELL:
		case BUILTIN:
		case MACRO:
		case MARG:
			error("unreachable");
			abort();
			break;
	}
}

static void compileTemplate(Node ** tt, Env * env) {
	Node * t = *tt;
	switch (t->type) {
		case SYMBOL: {
				if (isEllipsis(t)) {
					error("compile template");
					abort();
				}
				int i;
				for (i = 0; i < varTop; i++) {
					if (var[i] == toSym(t)->sym) {
						break;
					}
				}
				if (i < varTop) { // arg
					if (ellipsisDepth != varDepth[i]) {
						error("compile template");
						abort();
					}
					*tt = newMarg(i, ellipsisDepth);
				} else { // free var
					if (ellipsisDepth > 0) { 
						error("compile template");
						abort();
					}
					*tt = lookup(env, toSym(t)->sym);
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
				compileTemplate(&car(t), env);
				compileTemplate(&cdr(t), env);
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
					compileTemplate(&toVec(t)->vec[i], env);
					toVec(t)->vec[j++] = toVec(t)->vec[i];
					i = k;
				}
				toVec(t)->len = j;
			}
			break;
		case EMPTY:
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
			break;
		case DUMMY:
		case FIX_LAMBDA:
		case VAR_LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case MARG:
		case BUILTIN:
			error("unreachable");
			abort();
			break;
	}
}

Node * newMacro(Node * lit, Node * ps, Node * ts, Env * env) {
	unsigned int len = length(ps);
	assert(len == length(ts));
	Macro * ret = alloc(sizeof(Macro) + len * sizeof(*ret->rules));
	ret->type = MACRO;
	ret->env = env;
	ret->ruleLen = len;
	for (int i = 0; i < len; i++) {
		varTop = 0;
		ellipsisDepth = 0;
		compilePattern(lit, &car(ps));
		compileTemplate(&car(ts), env);
		car(ps) = cons(&dummy, car(ps));
		ret->rules[i].ptrn = car(ps);
		ret->rules[i].tmpl = car(ts);
	}
	return (Node *)ret;
}

Node * newBuiltin(Node * (*f)(Node *, Env *)) {
	Builtin * ret = alloc(sizeof(Builtin));
	ret->type = BUILTIN;
	ret->f = f;
	return (Node *)ret;
}

Node * render(Node * t, Env * env) {
	switch (t->type) {
		case MARG: // FIXME : not hygiene
			if (toMarg(t)->depth > 0) {
				return sliceFrame[toMarg(t)->offset];
			}
			return frame[toMarg(t)->offset];
		case PAIR: {
			Node * iter = t;
			Node * head = &empty;
			Node ** lst = &head;
			while (iter->type == PAIR) {
				Node * item = render(car(iter), env);
				if (car(iter)->type == MARG && toMarg(car(iter))->depth > 0) {
					*lst = item;
					while (*lst != &empty) {
						cdr(*lst)->type = PAIR;
						lst = &cdr(*lst);
					}
				} else {
					*lst = cons(item, &empty);
					lst = &cdr(*lst);
				}
			}
			*lst = iter;
			return head;
		}
		case LIST: {
			Node * iter = t;
			Node * head = &empty;
			Node ** lst = &head;
			while (iter->type == LIST) {
				Node * item = render(car(iter), env);
				if (car(iter)->type == MARG && toMarg(car(iter))->depth > 0) {
					*lst = item;
					while (*lst != &empty) {
						cdr(*lst)->type = LIST;
						lst = &cdr(*lst);
					}
				} else {
					*lst = cons(item, &empty);
					lst = &cdr(*lst);
				}
			}
			*lst = &empty;
			return head;
		}
		case VECTOR: {
			Node * buff[4096]; // FIXME
			int top = 0;
			for (int i = 0; i < toVec(t)->len; i++) {
				Node * iter = toVec(t)->vec[i];
				Node * item = render(iter, env);
				if (iter->type == MARG && toMarg(iter)->depth > 0) {
					Node * lst = item;
					while (lst != &empty) {
						buff[top++] = car(lst);
						lst = cdr(lst);
					}
				} else {
					buff[top++] = item;
				}
			}
			Node * ret = newVector(buff, top);
			return ret;
		}
		case SYMBOL:
		case EMPTY:
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
		case FIX_LAMBDA:
		case VAR_LAMBDA:
		case MACRO:
		case BUILTIN:
			return t;
		case DUMMY:
		case LISTELL:
		case VECTORELL:
			error("unreachable");
			abort();
			break;
	}
}

Node * transform(Node * mac, Node * expr, Env * env) {
	if (mac->type == BUILTIN) {
		return toBuil(mac)->f(expr, env);
	}
	for (int i = 0; i < toMacro(mac)->ruleLen; i++) {
		if (match(toMacro(mac)->rules[i].ptrn, expr)) {
			return render(toMacro(mac)->rules[i].tmpl, env);
		}
	}
	abort();
	return NULL;
}

Node * defineSyntaxPattern;
Node * lambdaPattern;
Node * definePattern;
Node * defineLambdaPattern;
Node * cbDefineSyntax(Node * expr, Env * env) {
	if (!match(defineSyntaxPattern, expr)) {
		error("bad syntax");
		abort();
	}
	if (frame[1]->type != SYMBOL) {
		error("bad syntax");
		abort();
	}
	Symbol name = toSym(frame[1])->sym;
	Node * lit = frame[0];
	Node * ps = sliceFrame[0];
	Node * ts = sliceFrame[1];
	if (lit->type != LIST && lit->type != EMPTY) {
		error("bad syntax");
		abort();
	}
	Node * ret;
	if ((ret = newMacro(lit, ps, ts, env)) == NULL) {
		error("bad syntax");
		abort();
	}
	updateEnv(env, name, ret);
	return NULL;
}

Node * cbDefine(Node * expr, Env * env) {
	if (match(definePattern, expr) && frame[0]->type == SYMBOL) {
		Symbol name = toSym(frame[0])->sym;
		Node * body = frame[1];
		updateEnv(env, name, eval(body, env));
		return NULL;
	}
	if (match(defineLambdaPattern, expr)) {
		if (frame[0]->type != SYMBOL) {
			error("bad syntax");
			abort();
		}
		Symbol name = toSym(frame[0])->sym;
		Node * formal = frame[1];
		Node * body0 = frame[2];
		Node * body1 = sliceFrame[0];
		Node * ret;
		if ((ret = newLambda(formal, cons(body0, body1), env)) == NULL) {
			error("invalid lambda construction");
			abort();
		}
		updateEnv(env, name, ret);
		return NULL;
	}
	abort();
	return NULL;
}

Node * cbLambda(Node * expr, Env * env) {
	if (!match(lambdaPattern, expr)) {
		error("bad syntax");
		abort();
	}
	Node * formal = frame[0];
	Node * body0 = frame[1];
	Node * body1 = sliceFrame[0];
	Node * ret;
	if ((ret = newLambda(formal, cons(body0, body1), env)) == NULL) {
		error("invalid lambda construction");
		abort();
	}
	return ret;
}

Node * consEll(Node * a, Node * b) {
	Node * ret = cons(a, b);
	ret->type = LISTELL;
	return ret;
}

void initMacro() {
	// (`define-syntax` name
	//   (`syntax-rules` lit
	//                   ((DUMMY . ps) ts) ...))
	defineSyntaxPattern =
		LIST3(
			&dummy,
			newMarg(1, 0), // name
			LISTELL3(
				newSymbol("syntax-rules"),
				newMarg(0, 0), // lit, should be a LIST
				LIST2(
					cons(
						&dummy,
						newMarg(0, 1) // ps ...
					),
					newMarg(1, 1) // ts ...
				)
			)
		);
	// (`lambda` formal body0 body1 ...)
	lambdaPattern =
		LISTELL4(
			&dummy,
			newMarg(0, 0), // formal
			newMarg(1, 0), // body0
			newMarg(0, 1) // body1 ...
		);
	// (`define` symbol body)
	definePattern =
		LIST3(
			&dummy,
			newMarg(0, 0), // symbol
			newMarg(1, 0) // body
		);
	// (`define` formal body0 body1 ...)
	defineLambdaPattern =
		LISTELL4(
			&dummy,
			cons(
				newMarg(0, 0), // name
				newMarg(1, 0) // formal
			),
			newMarg(2, 0), // body0
			newMarg(0, 1) // body1 ...
		);
	updateEnv(topEnv, getSym("define-syntax"), newBuiltin(cbDefineSyntax));
	updateEnv(topEnv, getSym("define"), newBuiltin(cbDefine));
	updateEnv(topEnv, getSym("lambda"), newBuiltin(cbLambda));
	for (int i = 0; i < sizeof(sliceFrame) / sizeof(*sliceFrame); i++) {
		sliceFrame[i] = &empty;
		sliceFrameLast[i] = &empty;
	}
}
