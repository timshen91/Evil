#include <stdlib.h>
#include <string.h>
#include "environment.h"
#include "structure.h"
#include "memory.h"
#include "error.h"

Env topEnv = {.parent = NULL, .mem = {0}};

Env * newEnv(Env * env) { // FIXME
	Env * ret = alloc(sizeof(Env));
	ret->parent = env;
	memset(ret->mem, 0, sizeof(ret->mem));
	return ret;
}

void updateEnv(Env * env, Symbol sym, Node * value) { // FIXME
	if (env != &topEnv) {
		if (env->mem[sym]) {
			error("duplicate definition");
			abort();
		}
	}
	env->mem[sym] = value;
}

Node * lookup(Env * env, Symbol sym) { // FIXME
	while (env) {
		if (env->mem[sym]) {
			return env->mem[sym];
		}
		env = env->parent;
	}
	error(symToStr(sym));
	error("not defined");
	abort();
}
