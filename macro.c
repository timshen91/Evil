#include <assert.h>
#include <stddef.h>
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
Node * sliceFrame[4096] = {0};
Node * sliceFrameLast[4096] = {0};
static unsigned int stack[2][4096];
static int top[2];

static void matchClear() {
	while (top[0] > 0) {
		top[0]--;
		frame[stack[0][top[0]]] = NULL;
	}
	while (top[1] > 0) {
		top[1]--;
		sliceFrame[stack[0][top[1]]] = NULL;
	}
}

bool matchImpl(Node * p, Node * v) {
	if (p->type == DUMMY) {
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
			if (matchImpl(piter->a, viter->a) != 0) {
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
		if (matchImpl(piter->a, viter->a) != 0) {
			return false;
		}
		for (i = 1; i < plen; i++) {
			piter = toPair(cdr(piter));
			viter = toPair(cdr(viter));
			if (matchImpl(piter->a, viter->a) != 0) {
				return false;
			}
		}
		for (; i < vlen; i++) {
			viter = toPair(cdr(viter));
			if (matchImpl(piter->a, viter->a) != 0) {
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
			if (matchImpl(piter->a, viter->a) != 0) {
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
			if (matchImpl(vp[i], vv[i]) != 0) {
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
			if (matchImpl(vp[i], vv[i]) != 0) {
				return false;
			}
		}
		for (; i < lenv; i++) {
			if (matchImpl(vp[lenp - 1], vv[i]) != 0) {
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
	matchClear();
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
int compilePattern(Node * lit, Node ** pp) {
	static int ellipsisDepth = 0;
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
					return -1;
				}
				var[varTop] = toSym(p)->sym;
				varDepth[varTop] = ellipsisDepth;
				varTop++;
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
		case DUMMY:
		case EMPTY:
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
			break;
		case LIST_LAMBDA:
		case PAIR_LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case MARG:
		case BUILTIN:
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
				for (i = 0; i < varTop; i++) {
					if (var[i] == toSym(t)->sym) {
						break;
					}
				}
				if (i < varTop) { // arg
					if (ellipsisDepth != varDepth[i]) {
						return -1;
					}
					*tt = newMarg(i, ellipsisDepth);
				} else { // free var
					if (ellipsisDepth > 0) { 
						return -1;
					}
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
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
			break;
		case DUMMY:
		case LIST_LAMBDA:
		case PAIR_LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case MARG:
		case BUILTIN:
			assert(0);
			break;
	}
	return 0;
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
		if (compilePattern(lit, &car(ps))) {
			return NULL;
		}
		if (compileTemplate(&car(ts))) {
			return NULL;
		}
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

static Node * copy(Node * item) { // TODO
	switch (item->type) {
		case EMPTY:
		case BOOL:
		case COMPLEX:
		case CHAR:
		case STRING:
		default:
			assert(0);
			break;
	}
}

void render(Node ** t) { // TODO
}

Node * transform(Node * mac, Node * expr, Env * env) {
	if (expr->type != LIST) {
		return NULL;
	}
	if (mac->type == BUILTIN) {
		return toBuil(mac)->f(expr, env);
	}
	for (int i = 0; i < toMacro(mac)->ruleLen; i++) {
		if (match(toMacro(mac)->rules[i].ptrn, expr)) {
			Node * ret = copy(toMacro(mac)->rules[i].tmpl);
			render(&ret);
			return ret;
		}
	}
	return NULL;
}

Node * defineSyntaxPattern;
Node * syntaxRulesPattern;
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
	if (lit->type != LIST) {
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

Node * cbDefine(Node * expr, Env * env) { // TODO
	if (match(definePattern, expr)) {
		if (frame[0]->type != SYMBOL) {
			error("bad syntax");
			abort();
		}
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
		return ret;
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

Env * topEnv;
void initMacro() {
	// (`define-syntax` name
	//   (`syntax-rules` lit
	//                   (((DUMMY . ps) ts) ...)))
	syntaxRulesPattern =
		LIST3(
			&dummy,
			newMarg(0, 0), // lit, should be a LIST
			LIST2(
				LIST2(
					cons(
						&dummy,
						newMarg(0, 1) // ps ...
					),
					newMarg(1, 1) // ts ...
				),
				newSymbol("...")
			)
		);
	defineSyntaxPattern =
		LIST3(
			&dummy,
			newMarg(1, 0), // name
			syntaxRulesPattern
		);
	// (`lambda` formal body0 body1 ...)
	lambdaPattern =
		LIST4(
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
		LIST4(
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
}
