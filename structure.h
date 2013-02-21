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
	BOOLLIT,
	NUMLIT,
	CHARLIT,
	STRLIT,
	LIST_LAMBDA,
	PAIR_LAMBDA,
	MACRO,
	REF,
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

typedef struct StrLitNode {
	enum NodeType type;
	unsigned int len;
	char str[];
} StrLitNode;

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
	unsigned int envLen;
	Env * env;
	Node * body;
} LambdaNode;

typedef struct RefNode {
	enum NodeType type;
	unsigned int upn;
	unsigned int offset;
} RefNode;

Node empty;

Node * newSymbol(const char *);
Node * newVector(unsigned int);
Node * newStrLit(const char *, unsigned int);
Node * newBool(bool);
Node * newChar(char);
Node * newComplex(const char *);
Node * newLambda(Node *, Node *, Env *);
Node * newRef(Symbol);

unsigned int length(Node *);
Node * polar2Cart(Node *);
Node * cons(Node *, Node *);
bool equal(Node *, Node *);

#define toPair(p) ((PairNode *)(p))
#define toSym(p) ((SymNode *)(p))
#define toVec(p) ((VecNode *)(p))
#define toBool(p) ((BoolNode *)(p))
#define toNum(p) ((ComplexNode *)(p))
#define toChar(p) ((CharNode *)(p))
#define toString(p) ((StrLitNode *)(p))
#define toLambda(p) ((LambdaNode *)(p))
#define LIST1(a) cons((a), &empty)
#define LIST2(a, b) cons((a), LIST1(b))
#define LIST3(a, b, c) cons((a), LIST2((b), (c)))
#define LIST4(a, b, c, d) cons((a), LIST3((b), (c), (d)))
#define car(p) toPair(p)->a
#define cdr(p) toPair(p)->b

#endif
