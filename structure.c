#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"

Node empty = {.type = EMPTY};

typedef struct Real {
	unsigned int len;
	bool pos;
	unsigned int offset;
} Real;

Node * newSymbol(const char * s) {
	SymNode * ret = alloc(sizeof(SymNode));
	ret->type = SYMBOL;
	ret->sym = getSym(s);
	ret->fromLevel = 0;
	return (Node *)ret;
}

Node * newVector(unsigned int len) {
	VecNode * ret = alloc(sizeof(VecNode) + len * sizeof(Node *));
	ret->type = VECTOR;
	ret->len = len;
	return (Node *)ret;
}

Node * newString(const char * s, unsigned int len) {
	StringNode * ret = alloc(sizeof(StringNode) + len);
	memcpy(ret->str, s, len);
	ret->type = STRING;
	ret->len = len;
	return (Node *)ret;
}

Node * newBool(bool b) {
	BoolNode * ret = alloc(sizeof(BoolNode));
	ret->type = BOOL;
	ret->value = b;
	return (Node *)ret;
}

Node * newChar(char ch) {
	CharNode * ret = alloc(sizeof(CharNode));
	ret->type = CHAR;
	ret->value = ch;
	return (Node *)ret;
}

Node * cons(Node * a, Node * b) {
	PairNode * ret = alloc(sizeof(PairNode));
	if (b->type == EMPTY || b->type == LIST) {
		ret->type = LIST;
	} else {
		ret->type = PAIR;
	}
	ret->a = a;
	ret->b = b;
	return (Node *)ret;
}

static bool isExpo(char ch) {
	return ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l';
}

static const char * getIntStr(const char * s) {
	if (!isdigit(*s)) {
		return s;
	}
	while (isdigit(*s)) {
		s++;
	}
	while (*s == '#') {
		s++;
	}
	return s;
}

static const char * getRealStr(const char * s, bool dec) {
	const char * cur = s;
	const char * t;
	if (*cur == '+' || *cur == '-') {
		cur++;
	}
	t = cur;
	if ((cur = getIntStr(cur)) == t) {
		goto decimal;
	}
	if (*cur == '/') {
		cur++;
		t = cur;
		if ((cur = getIntStr(cur)) == t) {
			goto decimal;
		}
	}
	return cur;
decimal:
	if (!dec) {
		cur = s;
		return cur;
	}
	cur = s;
	t = cur;
	cur = getIntStr(cur);
	if (cur == t) {
		if (*cur != '.') {
			cur = s;
			return cur;
		}
		cur++;
		t = cur;
		if ((cur = getIntStr(cur)) == t) {
			cur = s;
			return cur;
		}
	} else {
		while (isdigit(*cur)) {
			cur++;
		}
		if (*cur == '.') {
			cur = getIntStr(cur);
		} else if (*cur == '#') {
			while (*cur == '#') {
				cur++;
			}
			if (*cur != '.') {
				cur = s;
				return cur;
			}
			while (*cur == '#') {
				cur++;
			}
		}
	}
	if (isExpo(*cur)) {
		cur++;
	} else {
		return cur;
	}
	if (*cur == '+' || *cur == '-') {
		cur++;
	}
	t = cur;
	if ((cur = getIntStr(cur)) != t) {
		cur = s;
		return cur;
	}
	return cur;
}

Node * makeComplex(const char * a, const char * b, int radix) { // TODO
	ComplexNode * ret = alloc(sizeof(ComplexNode));
	ret->type = COMPLEX;
	return (Node *)ret;
}

Node * newComplex(const char * old) {
	const char * s = old;
	int radix = -1;
	int exact = -1;
	if (*s == '#') {
#define switch_ch \
		switch (*s) {\
			case 'b':\
					 radix = 2;\
			break;\
			case 'o':\
					 radix = 8;\
			break;\
			case 'd':\
					 radix = 10;\
			break;\
			case 'x':\
					 radix = 16;\
			break;\
			case 'e':\
					 exact = 1;\
			break;\
			case 'i':\
					 exact = 0;\
			break;\
		}
		s++;
		switch_ch;
		s++;
		if (*s == '#') {
			s++;
			switch (*s) {
				case 'e':
				case 'i':
					if (exact != -1) {
						return NULL;
					}
					break;
				case 'b':
				case 'o':
				case 'd':
				case 'x':
					if (radix != -1) {
						return NULL;
					}
					break;
				default:
					return NULL;
			}
			switch_ch;
			s++;
		}
	}
	if (radix != 2 && radix != 8 && radix != 10 && radix != 16) {
		radix = 10;
	}
	if (strcmp(s, "+i") == 0) {
		return makeComplex("0", "1", 2);
	} else if (strcmp(s, "-i") == 0) {
		return makeComplex("0", "-1", 2);
	}
	char a[4096], b[4096];
	const char * t = s;
	s = getRealStr(s, radix == 10);
	if (t == s) {
		return NULL;
	}
	memcpy(a, old, s - old);
	a[s - old] = '\0';
	if (*t == '+' || *t == '-') {
		if (*s == 'i') {
			return makeComplex("0", a, radix);
		}
	}
	if (*s == '@') {
		s++;
		t = s;
		s = getRealStr(s, radix == 10);
		if (t == s) {
			return NULL;
		}
		memcpy(b, old, s - old);
		b[s - old] = '\0';
		return polar2Cart(makeComplex(a, b, radix));
	} else if (*s == '+' || *s == '-') {
		t = s;
		s = getRealStr(s, radix == 10);
		if (t != s) {
			memcpy(b, old, s - old);
			b[s - old] = '\0';
			return makeComplex(a, b, radix);
		} else {
			if (*(s + 1) != 'i') {
				return NULL;
			}
			return makeComplex(a, (*s == '+') ? "1" : "-1", radix);
		}
	}
	if (*s != '\0') {
		return NULL;
	}
	return makeComplex(a, "0", radix);
}

Node * polar2Cart(Node * a) { // TODO
	return a;
}

Node * newLambda(Node * formal, Node * body, Env * env) {
	Node * iter = formal;
	for (; iter->type == LIST || iter->type == PAIR; iter = cdr(iter)) {
		if (car(iter)->type != SYMBOL) {
			return NULL;
		}
	}
	if (iter->type != EMPTY) {
		if (iter->type != SYMBOL) {
			return NULL;
		}
	}
	LambdaNode * ret = alloc(sizeof(LambdaNode));
	ret->type = (formal->type == SYMBOL || formal->type == PAIR) ? VAR_LAMBDA : FIX_LAMBDA;
	ret->formal = formal;
	ret->env = env;
	ret->body = body;
	return (Node *)ret;
}

bool equal(Node * a, Node * b) {
	if (a->type != b->type) {
		return false;
	}
	switch (a->type) {
		case EMPTY:
			return true;
		case SYMBOL:
			return toSym(a)->sym == toSym(b)->sym;
		case LIST:
		case PAIR:
			return equal(car(a), car(b)) && equal(cdr(a), cdr(b));
		case VECTOR: {
			if (toVec(a)->len != toVec(b)->len) {
				return false;
			}
			Node ** va = toVec(a)->vec;
			Node ** vb = toVec(b)->vec;
			for (int i = 0; i < toVec(a)->len; i++) {
				if (!equal(va[i], vb[i])) {
					return false;
				}
			}
			return 1;
		}
		case BOOL:
			return toBool(a)->value == toBool(b)->value;
		case COMPLEX:
			return false; // TODO
		case CHAR:
			return toChar(a)->value == toChar(b)->value;
		case STRING:
			if (toString(a)->len != toString(b)->len) {
				return false;
			}
			return memcmp(toString(a)->str, toString(b)->str, toString(a)->len) == 0;
		case FIX_LAMBDA:
		case VAR_LAMBDA:
			return a == b;
		default:
			return false;
	}
	assert(0);
	return false;
}

unsigned int length(Node * l) {
	if (l->type != LIST && l->type != PAIR) {
		return 0;
	}
	return 1 + length(cdr(l));
}
