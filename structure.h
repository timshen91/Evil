#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

#include <stdbool.h>
#include "symbol.h"

enum NodeType {
	SYMBOL,
	PAIR,
	LIST,
	EMPTY,
	VECTOR,
	BOOL,
	COMPLEX,
	CHAR,
	STRING,
	FIX_LAMBDA,
	VAR_LAMBDA,
	MACRO,
	BUILTIN,
// macro,
	DUMMY,
	LISTELL,
	VECTORELL,
	MARG,
};

typedef struct Node {
	enum NodeType type;
} Node;

typedef struct SymNode {
	enum NodeType type;
	Symbol sym;
	unsigned int fromLevel;
} SymNode;

typedef struct PairNode {
	enum NodeType type;
	struct Node * a;
	struct Node * b;
} PairNode;

typedef struct VecNode {
	enum NodeType type;
	unsigned int len;
	Node * vec[];
} VecNode;

typedef struct StringNode {
	enum NodeType type;
	unsigned int len;
	char str[];
} StringNode;

typedef struct BoolNode {
	enum NodeType type;
	bool value;
} BoolNode;

typedef struct CharNode {
	enum NodeType type;
	char value;
} CharNode;

typedef struct ComplexNode {
	enum NodeType type;
} ComplexNode;

typedef struct Env Env;
typedef struct LambdaNode {
	enum NodeType type;
	Node * formal;
	Env * env;
	Node * body;
} LambdaNode;

Node empty;

Node * newSymbol(const char *);
Node * newVector(unsigned int);
Node * newString(const char *, unsigned int);
Node * newBool(bool);
Node * newChar(char);
Node * newComplex(const char *);
Node * newLambda(Node *, Node *, Env *);

unsigned int length(Node *);
Node * polar2Cart(Node *);
Node * cons(Node *, Node *);
bool equal(Node *, Node *);

#define toPair(p) ((PairNode *)(p))
#define toSym(p) ((SymNode *)(p))
#define toVec(p) ((VecNode *)(p))
#define toBool(p) ((BoolNode *)(p))
#define toComplex(p) ((ComplexNode *)(p))
#define toChar(p) ((CharNode *)(p))
#define toString(p) ((StringNode *)(p))
#define toLambda(p) ((LambdaNode *)(p))
#define LIST1(a) cons((a), &empty)
#define LIST2(a, b) cons((a), LIST1(b))
#define LIST3(a, b, c) cons((a), LIST2((b), (c)))
#define LIST4(a, b, c, d) cons((a), LIST3((b), (c), (d)))
#define car(p) toPair(p)->a
#define cdr(p) toPair(p)->b

#endif
