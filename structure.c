#include <string.h>
#include <ctype.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"

typedef struct Real {
	size_t len;
	int sign;
	unsigned long offset;
} Real;

Node * newSymbol(const char * s) {
	SymNode * ret = alloc(sizeof(SymNode));
	ret->type = SYMBOL;
	ret->sym = getSymbol(s);
	return (Node *)ret;
}

Node * newVector(size_t len) {
	VecNode * ret = alloc(sizeof(VecNode) + len * sizeof(Node *));
	ret->type = VECTOR;
	ret->len = len;
	return (Node *)ret;
}

Node * newStrLit(const char * s, size_t len) {
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
	ret->type = PAIR;
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

static const char * getRealStr(char * ret, const char * s, int dec) {
	const char * cur = s;
	const char * t;
	ret[0] = '\0';
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
	goto out;
decimal:
	if (!dec) {
		cur = s;
		goto out;
	}
	cur = s;
	t = cur;
	cur = getIntStr(cur);
	if (cur == t) {
		if (*cur != '.') {
			cur = s;
			goto out;
		}
		cur++;
		t = cur;
		if ((cur = getIntStr(cur)) == t) {
			cur = s;
			goto out;
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
				goto out;
			}
			while (*cur == '#') {
				cur++;
			}
		}
	}
	if (isExpo(*cur)) {
		cur++;
	} else {
		goto out;
	}
	if (*cur == '+' || *cur == '-') {
		cur++;
	}
	t = cur;
	if ((cur = getIntStr(cur)) != t) {
		cur = s;
		goto out;
	}
out:
	memcpy(ret, s, cur - s);
	ret[cur - s] = '\0';
	return cur;
}

Node * makeComplex(const char * a, const char * b, int radix) { // TODO
	ComplexNode * ret = alloc(sizeof(ComplexNode));
	ret->type = NUMLIT;
	return (Node *)ret;
}

Node * newComplex(const char * s) {
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
	a[0] = 0;
	b[0] = 0;
	const char * t = s;
	s = getRealStr(a, s, radix == 10);
	if (t == s) {
		return NULL;
	}
	if (*t == '+' || *t == '-') {
		if (*s == 'i') {
			return makeComplex("0", a, radix);
		}
	}
	if (*s == '@') {
		s++;
		t = s;
		s = getRealStr(b, s, radix == 10);
		if (t == s) {
			return NULL;
		}
		return polar2Cart(makeComplex(a, b, radix));
	} else if (*s == '+' || *s == '-') {
		t = s;
		s = getRealStr(b, s, radix == 10);
		return makeComplex(a, b, radix);
	}
	if (*s != '\0') {
		return NULL;
	}
	return makeComplex(a, "0", radix);
}

Node * polar2Cart(Node * a) { // TODO
	return a;
}
