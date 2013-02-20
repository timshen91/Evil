#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

#include <stdbool.h>

enum NodeType {
	SYMBOL,
	PAIR,
	LIST,
	EMPTY,
	DUMMY,
	LISTELL,
	VECTOR,
	VECTORELL,
	BOOLLIT,
	NUMLIT,
	CHARLIT,
	STRLIT,
	LAMBDA,
	MACRO,
	OFFSET,
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

typedef struct LambdaNode {
	enum NodeType type;
	unsigned int argn;
	Node * body;
} LambdaNode;

typedef struct OffsetNode {
	enum NodeType type;
	unsigned int offset;
	unsigned int depth;
} OffsetNode;

typedef struct Rule {
	Node * ptrn;
	Node * tmpl;
} Rule;

typedef struct Macro {
	enum NodeType type;
	unsigned int ruleLen;
	Rule rules[];
} Macro;

Node empty;

Node * newSymbol(const char *);
Node * newVector(unsigned int len);
Node * newStrLit(const char * s, unsigned int len);
Node * newBool(bool b);
Node * newChar(char ch);
Node * newComplex(const char * s);
Node * newLambda(Node * formal, Node * body);
Node * newOffset(unsigned int offset, unsigned int depth);
Macro * newMacro(Node * lit, Node * ps, Node * ts);

unsigned int length(Node * a);
Node * polar2Cart(Node * a);
Node * cons(Node * a, Node * b);
bool equal(Node * a, Node * b);

#define toPair(p) ((PairNode *)(p))
#define toSym(p) ((SymNode *)(p))
#define toVec(p) ((VecNode *)(p))
#define toBool(p) ((BoolNode *)(p))
#define toNum(p) ((ComplexNode *)(p))
#define toChar(p) ((CharNode *)(p))
#define toString(p) ((StrLitNode *)(p))
#define toLambda(p) ((LambdaNode *)(p))
#define toMacro(p) ((Macro *)(p))
#define toOffset(p) ((OffsetNode *)(p))
#define LIST1(a) cons((a), &empty)
#define LIST2(a, b) cons((a), LIST1(b))
#define LIST3(a, b, c) cons((a), LIST2((b), (c)))
#define LIST4(a, b, c, d) cons((a), LIST3((b), (c), (d)))
#define car(p) toPair(p)->a
#define cdr(p) toPair(p)->b

#endif
