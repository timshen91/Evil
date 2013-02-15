#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

#include <stdint.h>

enum NodeType {
	SYMBOL,
	PAIR,
	VECTOR,
	BOOLLIT,
	NUMLIT,
	CHARLIT,
	STRLIT,
};

typedef struct Node {
	enum NodeType type;
	union {
		struct {
			struct Node * a;
			struct Node * b;
		};
		size_t len;
		unsigned long lit;
		unsigned long sym;
	};
} Node;

typedef struct Complex { // Two compact Real
} Complex;

Node * newSymbol(const char * s);
Node * newVector(size_t len);
Node * newStrLit(const char * s, size_t len);
Node * newBool(int b);
Node * newChar(char ch);
Node * newComplex(const char * a, const char * b, int radix);
Node * cons(Node * a, Node * b);
Node * polar2Cart(Node * a);

#endif
