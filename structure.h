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
} Node;

typedef struct SymNode {
	enum NodeType type;
	unsigned long sym;
} SymNode;

typedef struct PairNode {
	enum NodeType type;
	struct Node * a;
	struct Node * b;
} PairNode;

typedef struct VecNode {
	enum NodeType type;
	size_t len;
} VecNode;

typedef struct StrLitNode {
	enum NodeType type;
	size_t len;
} StrLitNode;

typedef struct BoolNode {
	enum NodeType type;
	char value;
} BoolNode;

typedef struct CharNode {
	enum NodeType type;
	char value;
} CharNode;

typedef struct ComplexNode {
	enum NodeType type;
} ComplexNode;

Node * newSymbol(const char * s);
Node * newVector(size_t len);
Node * newStrLit(const char * s, size_t len);
Node * newBool(int b);
Node * newChar(char ch);
Node * newComplex(const char * s);
Node * cons(Node * a, Node * b);
Node * polar2Cart(Node * a);

#endif
