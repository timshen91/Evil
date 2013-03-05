#include "structure.h"
#include "environment.h"
#include "memory.h"
#include "macro.h"
#include "complex.h"
#include "error.h"
#include "eval.h"

#define LISTELL1(a) consEll((a), &empty)
#define LISTELL2(a, b) consEll((a), LISTELL1((b)))
#define LISTELL3(a, b, c) consEll((a), LISTELL2((b), (c)))
#define LISTELL4(a, b, c, d) consEll((a), LISTELL3((b), (c), (d)))

Node empty = {.type = EMPTY};
Node boolTrue = {.type = BOOL};
Node boolFalse = {.type = BOOL};
Node unspecified = {.type = UNSPECIFIED};

typedef struct BuiltinMacro {
	enum NodeType type;
	Node * (*f)(Node *, Env *);
} BuiltinMacro;

typedef struct BuiltinLambda {
	enum NodeType type;
	Node * (*f)(Node *);
} BuiltinLambda;

Node * newBuiltinMacro(Node * (*f)(Node *, Env *)) {
	BuiltinMacro * ret = alloc(sizeof(BuiltinMacro));
	ret->type = BUILTIN_MAC;
	ret->f = f;
	return (Node *)ret;
}

Node * newBuiltinLambda(Node * (*f)(Node *)) {
	BuiltinLambda * ret = alloc(sizeof(BuiltinLambda));
	ret->type = BUI_LAMBDA;
	ret->f = f;
	return (Node *)ret;
}

Node * callBuiltinMac(Node * mac, Node * expr, Env * env) {
	return ((BuiltinMacro *)mac)->f(expr, env);
}

Node * callBuiltinLambda(Node * ff, Node * expr) {
	return ((BuiltinLambda *)ff)->f(expr);
}

Node * consEll(Node * a, Node * b) {
	Node * ret = cons(a, b);
	ret->type = LISTELL;
	return ret;
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

Node * cbIf(Node * expr, Env * env) {
	expr = cdr(expr);
	unsigned int len = length(expr);
	if (len != 3 && len != 2) {
		error("mismatch");
		abort();
	}
	Node * cond = eval(car(expr), env);
	if (cond != &boolFalse) {
		return eval(car(cdr(expr)), env);
	} else {
		if (len == 2) {
			return &unspecified;
		}
		return eval(car(cdr(cdr(expr))), env);
	}
	return NULL;
}

Node * cbAdd(Node * args) {
	ComplexNode * a = makeRational(0, 1);
	Node * iter = args;
	for (; iter->type == LIST; iter = cdr(iter)) {
		ComplexNode * b = toComplex(car(iter));
		if (b->exact) {
			a = makeRational(a->nu * b->de + a->de * b->nu, a->de * b->de);
		} else {
			break;
		}
	}
	if (iter->type != LIST) {
		return (Node *)a;
	}
	exact2inexact(a);
	for (; iter->type == LIST; iter = cdr(iter)) {
		ComplexNode b = *toComplex(car(iter));
		exact2inexact(&b);
		a = makeComplex(a->re + b.re, a->im + b.im);
	}
	return (Node *)a;
}

Node * cbSub(Node * args) {
	if (args->type != LIST) {
		error("mismatch");
		abort();
	}
	ComplexNode * a = makeRational(toComplex(car(args))->nu, toComplex(car(args))->de);
	Node * iter = cdr(args);
	for (; iter->type == LIST; iter = cdr(iter)) {
		ComplexNode * b = toComplex(car(iter));
		if (b->exact) {
			a = makeRational(a->nu * b->de - a->de * b->nu, a->de * b->de);
		} else {
			break;
		}
	}
	if (iter->type != LIST) {
		return (Node *)a;
	}
	exact2inexact(a);
	for (; iter->type == LIST; iter = cdr(iter)) {
		ComplexNode b = *toComplex(car(iter));
		exact2inexact(&b);
		a = makeComplex(a->re - b.re, a->im - b.im);
	}
	return (Node *)a;
}

Node * cbMul(Node * args) {
	ComplexNode * a = makeRational(1, 1);
	Node * iter = args;
	for (; iter->type == LIST; iter = cdr(iter)) {
		ComplexNode * b = toComplex(car(iter));
		if (b->exact) {
			a = makeRational(a->nu * b->nu, a->de * b->de);
		} else {
			break;
		}
	}
	if (iter->type != LIST) {
		return (Node *)a;
	}
	exact2inexact(a);
	for (; iter->type == LIST; iter = cdr(iter)) {
		ComplexNode b = *toComplex(car(iter));
		exact2inexact(&b);
		a = makeComplex(a->re * b.re - a->im * b.im, a->re * b.im + a->im * b.re);
	}
	return (Node *)a;
}

Node * cbEqu(Node * args) {
	if (args->type != LIST) {
		return &boolTrue;
	}
	Node * last = car(args);
	if (last->type != COMPLEX) {
		return &boolFalse;
	}
	for (Node * iter = cdr(args); iter->type == LIST; iter = cdr(iter)) {
		if (car(iter)->type != COMPLEX) {
			return &boolFalse;
		}
		if (!equal(last, car(iter))) {
			return &boolFalse;
		}
	}
	return &boolTrue;
}

void initBuiltin() {
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
	for (int i = 0; i < sizeof(sliceFrame) / sizeof(*sliceFrame); i++) {
		sliceFrame[i] = &empty;
		sliceFrameLast[i] = &empty;
	}
	updateEnv(&topEnv, getSym("define-syntax"), newBuiltinMacro(cbDefineSyntax));
	updateEnv(&topEnv, getSym("define"), newBuiltinMacro(cbDefine));
	updateEnv(&topEnv, getSym("lambda"), newBuiltinMacro(cbLambda));
	updateEnv(&topEnv, getSym("if"), newBuiltinMacro(cbIf));
	updateEnv(&topEnv, getSym("+"), newBuiltinLambda(cbAdd));
	updateEnv(&topEnv, getSym("-"), newBuiltinLambda(cbSub));
	updateEnv(&topEnv, getSym("*"), newBuiltinLambda(cbMul));
	updateEnv(&topEnv, getSym("="), newBuiltinLambda(cbEqu));
}
