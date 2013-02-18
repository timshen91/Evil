#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"
#include "error.h"

typedef struct Real {
	unsigned int len;
	int sign;
	unsigned long offset;
} Real;

Node * newSymbol(const char * s) {
	SymNode * ret = alloc(sizeof(SymNode));
	ret->type = SYMBOL;
	ret->sym = getSym(s);
	return (Node *)ret;
}

Node * newVector(unsigned int len) {
	VecNode * ret = alloc(sizeof(VecNode) + len * sizeof(Node *));
	ret->type = VECTOR;
	ret->len = len;
	return (Node *)ret;
}

Node * newStrLit(const char * s, unsigned int len) {
	StrLitNode * ret = alloc(sizeof(StrLitNode) + len);
	memcpy(ret + 1, s, len);
	ret->type = STRLIT;
	ret->len = len;
	return (Node *)ret;
}

Node * newBool(int b) {
	BoolNode * ret = alloc(sizeof(BoolNode));
	ret->type = BOOLLIT;
	ret->value = b;
	return (Node *)ret;
}

Node * newChar(char ch) {
	CharNode * ret = alloc(sizeof(CharNode));
	ret->type = CHARLIT;
	ret->value = ch;
	return (Node *)ret;
}

Node * cons(Node * a, Node * b) {
	PairNode * ret = alloc(sizeof(PairNode));
	if (b == NULL || b->type == LIST) {
		ret->type = LIST;
	} else {
		ret->type = PAIR;
	}
	ret->a = a;
	ret->b = b;
	return (Node *)ret;
}

static int isExpo(char ch) {
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

static const char * getRealStr(const char * s, int dec) {
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
	ret->type = NUMLIT;
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

int equal(Node * a, Node * b) {
	if (a == NULL || b == NULL) {
		return a == b;
	}
	if (a->type != b->type) {
		return 0;
	}
	switch (a->type) {
		case SYMBOL:
			return toSym(a)->sym == toSym(b)->sym;
		case LIST:
		case PAIR:
			return equal(toPair(a)->a, toPair(b)->a) && equal(toPair(a)->b, toPair(b)->b);
		case VECTOR: {
			if (toVec(a)->len != toVec(b)->len) {
				return 0;
			}
			int i;
			Node ** va = (Node **)(toVec(a) + 1);
			Node ** vb = (Node **)(toVec(b) + 1);
			for (i = 0; i < toVec(a)->len; i++) {
				if (!equal(va[i], vb[i])) {
					return 0;
				}
			}
			return 1;
		}
		case BOOLLIT:
			return toBool(a)->value == toBool(b)->value;
		case NUMLIT:
			return 0; // TODO
		case CHARLIT:
			return toChar(a)->value == toChar(b)->value;
		case STRLIT:
			if (toString(a)->len != toString(b)->len) {
				return 0;
			}
			return memcmp((const char *)(toString(a) + 1), (const char *)(toString(b) + 1), toString(a)->len) == 0;
		case LAMBDA:
			return a == b;
	}
	assert(0);
	return 0;
}
