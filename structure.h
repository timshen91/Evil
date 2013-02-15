#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

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
		unsigned long dscr;
	};
} Node;

typedef struct Integer {
	int sign;
	unsigned long len;
} Integer;

typedef struct Rational { // Two compact Integer
} Rational;

typedef struct Real {
	int sign;
	unsigned long offset;
	unsigned long len;
} Real;

typedef struct Complex { // Two compact Real
} Complex;

Node * newSymbol(const char * s);
Node * newVector(int len);
Node * newStrLit(const char * s, int len);
Node * newBool(int b);
Node * newChar(char ch);
Node * newComplex();
Node * cons(Node * a, Node * b);

#endif
