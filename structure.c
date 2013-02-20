#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "memory.h"
#include "symbol.h"
#include "structure.h"

#define isEllipsis(node) ((node)->type == SYMBOL && toSym(node)->sym == getSym("..."))

Node empty = {.type = EMPTY};

typedef struct Real {
	unsigned int len;
	bool pos;
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
	memcpy(ret->str, s, len);
	ret->type = STRLIT;
	ret->len = len;
	return (Node *)ret;
}

Node * newBool(bool b) {
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
	if (b->type == EMPTY || b->type == LIST) {
		ret->type = LIST;
	} else {
		ret->type = PAIR;
	}
	if (b->type == PAIR || b->type == LIST) {
		ret->len = toPair(b)->len + 1;
	} else {
		ret->len = 1;
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

Node * newOffset(enum NodeType type, unsigned int offset) {
	OffsetNode * ret = alloc(sizeof(OffsetNode));
	ret->type = type;
	ret->offset = offset;
	return (Node *)ret;
}

Node * newLambda(Node * formal, Node * body) { // TODO
	
	return NULL;
}

static bool symInList(unsigned int sym, Node * l) {
	if (l->type == EMPTY) {
		return false;
	}
	return (car(l)->type == SYMBOL && toSym(car(l))->sym == sym) || symInList(sym, cdr(l));
}

static unsigned int varn;
static unsigned int var[4096];
int compilePattern(Node * lit, Node ** pp) {
	static int ellipsisDepth = 0;
	Node * p = *pp;
	switch (p->type) {
		case SYMBOL:
			if (!symInList(toSym(p)->sym, lit)) {
				int i;
				for (i = 0; i < varn; i++) {
					if (var[i] == toSym(p)->sym) {
						break;
					}
				}
				if (i == varn) {
					var[varn++] = toSym(p)->sym;
				}
				*pp = newOffset((ellipsisDepth > 0) ? OFFSET_SLICE : OFFSET, i);
			}
			break;
		case PAIR:
			if (compilePattern(lit, &car(p))) {
				return -1;
			}
			if (compilePattern(lit, &cdr(p))) {
				return -1;
			}
			break;
		case LIST: {
				Node * next = cdr(p);
				if (next->type == LIST && isEllipsis(car(next))) {
					if (cdr(next)->type != EMPTY) {
						return -1;
					}
					ellipsisDepth++;
					int ret = compilePattern(lit, &car(p));
					ellipsisDepth--;
					if (ret) {
						return -1;
					}
					cdr(p) = &empty;
					toPair(p)->type = LISTELL;
				} else {
					if (compilePattern(lit, &car(p))) {
						return -1;
					}
					compilePattern(lit, &cdr(p));
					if (cdr(cdr(p))->type == LISTELL) {
						toPair(p)->type = LISTELL;
					}
				}
			}
			break;
		case VECTOR:
			if (toVec(p)->len > 0) {
				int i;
				for (i = 0; i < toVec(p)->len - 1; i++) {
					Node * now = toVec(p)->vec[i];
					if (isEllipsis(now)) {
						return -1;
					}
				}
				Node * now = toVec(p)->vec[i];
				if (isEllipsis(now)) {
					p->type = VECTORELL;
					toVec(p)->len--;
				}
				for (i = 0; i < toVec(p)->len - 1; i++) {
					if (compilePattern(lit, &toVec(p)->vec[i])) {
						return -1;
					}
				}
				ellipsisDepth++;
				int ret = compilePattern(lit, &toVec(p)->vec[i]);
				ellipsisDepth--;
				if (ret) {
					return -1;
				}
			}
			break;
		case EMPTY:
		case BOOLLIT:
		case NUMLIT:
		case CHARLIT:
		case STRLIT:
			break;
		case DUMMY:
		case LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case OFFSET:
		case OFFSET_SLICE:
			assert(0);
			break;
	}
	return 0;
}

static int compileTemplate(Node ** tt) {
	static bool nextIsEllpsis = false;
	Node * t = *tt;
	switch (t->type) {
		case SYMBOL: {
				int i;
				for (i = 0; i < varn; i++) {
					if (var[i] == toSym(t)->sym) {
						break;
					}
				}
				if (i == varn && nextIsEllpsis) {
					return -1;
				}
				*tt = newOffset(nextIsEllpsis ? OFFSET_SLICE : OFFSET, i);
			}
			break;
		case PAIR:
		case LIST: {
				Node * next = cdr(t);
				if (next->type == LIST && isEllipsis(car(next))) {
					nextIsEllpsis = true;
					cdr(t) = cdr(next);
				}
				int ret = compileTemplate(&car(t));
				nextIsEllpsis = false;
				if (ret) {
					return -1;
				}
				if (compileTemplate(&cdr(t))) {
					return -1;
				}
			}
			break;
		case VECTOR: {
				int j = 0;
				for (int i = 0; i < toVec(t)->len - 1; i++) {
					if (!isEllipsis(toVec(t)->vec[i + 1])) {
						nextIsEllpsis = true;
						int ret = compileTemplate(&toVec(t)->vec[i]);
						nextIsEllpsis = false;
						if (ret) {
							return -1;
						}
						toVec(t)->vec[j++] = toVec(t)->vec[i];
						i++;
					} else {
						if (compileTemplate(&toVec(t)->vec[i])) {
							return -1;
						}
						toVec(t)->vec[j++] = toVec(t)->vec[i];
					}
				}
				if (toVec(t)->len > 0) {
					if (compileTemplate(&toVec(t)->vec[toVec(t)->len - 1])) {
						return -1;
					}
					toVec(t)->vec[j++] = toVec(t)->vec[toVec(t)->len - 1];
				}
				toVec(t)->len = j;
			}
			break;
		case EMPTY:
		case BOOLLIT:
		case NUMLIT:
		case CHARLIT:
		case STRLIT:
			break;
		case DUMMY:
		case LAMBDA:
		case MACRO:
		case LISTELL:
		case VECTORELL:
		case OFFSET:
		case OFFSET_SLICE:
			assert(0);
			break;
	}
	return 0;
}

static int compile(Rule * ret, Node * lit, Node ** p, Node ** t) {
	varn = 1; // 0 for DUMMY
	if (compilePattern(lit, p)) {
		return -1;
	}
	if (compileTemplate(p)) {
		return -1;
	}
	ret->ptrn = *p;
	ret->tmpl = *t;
	return 0;
}

Macro * newMacro(Node * lit, Node * ps, Node * ts) {
	unsigned int len = length(ps);
	assert(len == length(ts));
	Macro * ret = alloc(sizeof(Macro) + len * sizeof(Rule));
	ret->type = MACRO;
	ret->ruleLen = len;
	for (int i = 0; i < len; i++) {
		if (compile(&ret->rules[i], lit, &car(ps), &car(ts))) {
			return NULL;
		}
	}
	return ret;
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
		case BOOLLIT:
			return toBool(a)->value == toBool(b)->value;
		case NUMLIT:
			return false; // TODO
		case CHARLIT:
			return toChar(a)->value == toChar(b)->value;
		case STRLIT:
			if (toString(a)->len != toString(b)->len) {
				return false;
			}
			return memcmp(toString(a)->str, toString(b)->str, toString(a)->len) == 0;
		case LAMBDA:
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
