#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

//#define DUMP(s) printf("%s : %d : %s\n", __FILE__, __LINE__, s)
#define DUMP(s) 

enum NodeType {
	SYMBOL,
	PAIR,
	LIST,
	VECTOR,
	BOOLLIT,
	NUMLIT,
	CHARLIT,
	STRLIT,
	LAMBDA,
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
	unsigned long len;
	struct Node * a;
	struct Node * b;
} PairNode;

typedef struct VecNode {
	enum NodeType type;
	unsigned int len;
} VecNode;

typedef struct StrLitNode {
	enum NodeType type;
	unsigned int len;
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

typedef struct LambdaNode {
	enum NodeType type;
	unsigned int argn;
} LambdaNode;

Node * newSymbol(const char * s);
Node * newVector(unsigned int len);
Node * newStrLit(const char * s, unsigned int len);
Node * newBool(int b);
Node * newChar(char ch);
Node * newComplex(const char * s);
Node * polar2Cart(Node * a);


Node * cons(Node * a, Node * b);
int equal(Node * a, Node * b);

#define toPair(p) ((PairNode *)(p))
#define toSym(p) ((SymNode *)(p))
#define toVec(p) ((VecNode *)(p))
#define toBool(p) ((BoolNode *)(p))
#define toNum(p) ((ComplexNode *)(p))
#define toChar(p) ((CharNode *)(p))
#define toString(p) ((StrLitNode *)(p))
#define toLambda(p) ((LambdaNode *)(p))

#endif
