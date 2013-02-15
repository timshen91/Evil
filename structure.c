#include <string.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"

typedef struct Real {
	size_t len;
	int sign;
	unsigned long offset;
} Real;

Node * newSymbol(const char * s) {
	Node * ret = alloc(sizeof(Node));
	ret->type = SYMBOL;
	ret->sym = getSymbol(s);
	return ret;
}

Node * newVector(size_t len) {
	Node * ret = alloc(sizeof(Node) + len * sizeof(Node *));
	ret->type = VECTOR;
	ret->len = len;
	return ret;
}

Node * newStrLit(const char * s, size_t len) {
	Node * ret = alloc(sizeof(Node) + len);
	memcpy(ret + 1, s, len);
	ret->type = STRLIT;
	ret->len = len;
	return ret;
}

Node * newBool(int b) {
	Node * ret = alloc(sizeof(Node));
	ret->type = BOOLLIT;
	ret->lit = b;
	return ret;
}

Node * newChar(char ch) {
	Node * ret = alloc(sizeof(Node));
	ret->type = CHARLIT;
	ret->lit = ch;
	return ret;
}

Node * newComplex(const char * a, const char * b, int radix) { // TODO
	Node * ret = alloc(sizeof(Node));
	ret->type = NUMLIT;
	return ret;
}

Node * cons(Node * a, Node * b) {
	Node * ret = alloc(sizeof(Node));
	ret->type = PAIR;
	ret->a = a;
	ret->b = b;
	return ret;
}

Node * polar2Cart(Node * a) { // TODO
	return a;
}
