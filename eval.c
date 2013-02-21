#include <assert.h>
#include "eval.h"
#include "structure.h"
#include "symbol.h"
#include "macro.h"
#include "error.h"

struct LL * lexStack[4096] = {0};

static Node * defineSyntaxPattern;
static Node * lambdaPattern;

static void updateEnv(Env * env, Symbol sym, Node * value) { // TODO
}

static void defineSyntax(Node * expr, Env * env) {
	if (!match(defineSyntaxPattern, expr)) {
		error("bad syntax");
		abort();
	}
	Symbol name = toSym(frame[1])->sym;
	Node * lit = frame[2];
	Node * ps = sliceFrame[0];
	Node * ts = sliceFrame[1];
	matchClear();
	if (lit->type != LIST) {
		error("bad syntax");
		abort();
	}
	Macro * ret;
	if ((ret = newMacro(lit, ps, ts, env)) == NULL) {
		error("bad syntax");
		abort();
	}
	updateEnv(env, name, (Node *)ret);
}

static Node * evalLambda(Node * expr, Env * env) {
	if (!match(lambdaPattern, expr)) {
		error("bad syntax");
		abort();
	}
	Node * formal = frame[1];
	Node * body = frame[2];
	matchClear();
	Node * ret;
	if ((ret = newLambda(formal, body, env)) == NULL) {
		error("invalid lambda construction");
		abort();
	}
	return ret;
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
		Symbol sym = toSym(car(expr))->sym;
		if (sym == getSym("define-syntax")) {
			defineSyntax(expr, env);
		} else if (sym == getSym("define")){
			//if (newDefine(expr)) {
			//	abort();
			//}
		} else if (sym == getSym("lambda")) {
			return evalLambda(expr, env);
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
			newMarg(MARG, 1), // name
			LIST3(
				newSymbol("syntax-rules"),
				newMarg(MARG, 2), // lit, should be a LIST
				LIST2(
					LIST2(
						newMarg(0, 1), // ps ...
						newMarg(1, 1) // ts ...
					),
					newSymbol("...")
				)
			)
		);
	// (`lambda` formal body)
	lambdaPattern =
		LIST3(
			newSymbol("lambda"),
			newMarg(MARG, 1), // formal
			newMarg(MARG, 2) // body
		);
}
