#include <string.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"

Node * newSymbol(const char * s) {
	Node * ret = alloc(sizeof(Node));
	ret->type = SYMBOL;
	ret->dscr = getSymbol(s);
	return ret;
}

Node * newVector(int len) {
	Node * ret = alloc(sizeof(Node) + len * sizeof(Node *));
	ret->type = VECTOR;
	ret->dscr = len;
	return ret;
}

Node * newStrLit(const char * s, int len) {
	Node * ret = alloc(sizeof(Node) + len);
	memcpy(ret + 1, s, len);
	ret->type = STRLIT;
	ret->dscr = len;
	return ret;
}

Node * newBool(int b) {
	Node * ret = alloc(sizeof(Node));
	ret->type = BOOLLIT;
	ret->dscr = b;
	return ret;
}

Node * newChar(char ch) {
	Node * ret = alloc(sizeof(Node));
	ret->type = CHARLIT;
	ret->dscr = ch;
	return ret;
}

Node * newComplex() {
	return NULL;
}

Node * cons(Node * a, Node * b) {
	Node * ret = alloc(sizeof(Node));
	ret->type = PAIR;
	ret->a = a;
	ret->b = b;
	return ret;
}
