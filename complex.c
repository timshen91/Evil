#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "structure.h"
#include "complex.h"
#include "memory.h"

#define ABS(n) (((n) < 0) ? -(n) : (n))

Integer gcd(Integer a, Integer b) {
	if (b == 0) {
		return a;
	}
	return gcd(b, a % b);
}

void reduce(ComplexNode * ret) {
	Integer g = gcd(ABS(ret->nu), ABS(ret->de));
	ret->nu /= g;
	ret->de /= g;
}

ComplexNode * makeRational(Integer a, Integer b) {
	ComplexNode * ret = alloc(sizeof(ComplexNode));
	ret->type = COMPLEX;
	ret->exact = true;
	if (b < 0) {
		b = -b;
		a = -a;
	}
	ret->nu = a;
	ret->de = b;
	reduce(ret);
	return ret;
}

Real rational2Real(ComplexNode * a) {
	return a->nu / a->de;
}
	
ComplexNode * makeComplex(Real a, Real b) {
	ComplexNode * ret = alloc(sizeof(ComplexNode));
	ret->type = COMPLEX;
	ret->exact = false;
	ret->re = a;
	ret->im = b;
	return ret;
}

ComplexNode * makeDecimal(Integer a, Integer b, Integer e) {
	char buff[4096]; // FIXME
	Real ret;
	sprintf(buff, "%lld.%llde%lld", a, b, e);
	sscanf(buff, "%lf", &ret);
	return makeComplex(ret, 0);
}

ComplexNode * polar2Cart(ComplexNode * c) { // TODO
	return c;
}

const char * cur;
bool succ;
int radix;

int getDigit(char ch) {
	switch (radix) {
		case 2: 
			if ('0' <= ch && ch <= '1') {
				return ch - '0';
			}
			break;
		case 8:
			if ('0' <= ch && ch <= '7') {
				return ch - '0';
			}
			break;
		case 10:
			if ('0' <= ch && ch <= '9') {
				return ch - '0';
			}
			break;
		case 16:
			if ('0' <= ch && ch <= '9') {
				return ch - '0';
			}
			if ('a' <= ch && ch <= 'z') {
				return ch - 'a' + 10;
			}
			if ('A' <= ch && ch <= 'Z') {
				return ch - 'A' + 10;
			}
			break;
		default:
			assert(0);
			break;
	}
	return -1;
}

static bool isExpo(char ch) {
	return ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l';
}

static Integer getInt() {
	Integer ret = 0;
	if (getDigit(*cur) == -1) {
		succ = false;
		return 0;
	}
	int digit;
	while ((digit = getDigit(*cur)) >= 0) {
		cur++;
		ret = ret * radix + digit;
	}
	return ret;
}

static ComplexNode * getReal() {
	int sign = 1;
	if (*cur == '+') {
		sign = 1;
		cur++;
	} else if (*cur == '-') {
		sign = -1;
		cur++;
	}
	const char * start = cur;
	Integer a = getInt();
	Integer b;
	if (!succ) {
		goto decimal;
	}
	if (*cur == '/') {
		cur++;
		b = getInt();
		if (!succ) {
			goto decimal;
		}
		return makeRational(sign * a, b);
	}
	return makeRational(sign * a, 1);
decimal:
	succ = true;
	cur = start;
	if (radix != 10) {
		succ = false;
		return NULL;
	}
	bool hasInt = false;
	bool hasHash = false;
	a = 0;
	b = 1;
	int digit;
	while ((digit = getDigit(*cur)) >= 0) {
		hasInt = true;
		cur++;
		a = a * radix + digit;
	}
	while (*cur == '#') {
		hasHash = true;
		cur++;
		a = a * radix;
	}
	if (!hasHash) {
		if (hasInt) {
			if (*cur == '.') {
				while ((digit = getDigit(*cur)) >= 0) {
					cur++;
					b = b * radix + digit;
				}
				while (*cur == '#') {
					cur++;
				}
			}
		} else {
			if (*cur != '.') {
				succ = false;
				return NULL;
			}
			cur++;
			if (!isdigit(*cur)) {
				succ = false;
				return NULL;
			}
			while ((digit = getDigit(*cur)) >= 0) {
				cur++;
				b = b * radix + digit;
			}
		}
	} else if (!hasInt) {
		succ = false;
		return NULL;
	} else {
		while (*cur == '#') {
			cur++;
		}
	}
	if (isExpo(*cur)) {
		cur++;
	} else {
		return makeDecimal(sign * a, b, 0);
	}
	int sign2 = 1;
	if (*cur == '+') {
		cur++;
	} else if (*cur == '-') {
		sign2 = -1;
		cur++;
	}
	Integer e = 0;
	while ((digit = getDigit(*cur)) >= 0) {
		cur++;
		e = e * radix + digit;
	}
	return makeDecimal(sign * a, b, sign2 * e);
}

ComplexNode * parseComplex(const char * start) {
	if (strcmp(cur, "+i") == 0) {
		return makeComplex(0., 1.);
	} else if (strcmp(cur, "-i") == 0) {
		return makeComplex(0., -1.);
	}
	int hasSign = (*cur == '+' || *cur == '-');
	ComplexNode * a = getReal();
	ComplexNode * b;
	if (!succ) {
		return NULL;
	}
	if (hasSign) {
		if (*cur == 'i') {
			return makeComplex(0, rational2Real(a));
		}
	}
	if (*cur == '@') {
		cur++;
		b = getReal();
		if (!succ) {
			return NULL;
		}
		return polar2Cart(makeComplex(rational2Real(a), rational2Real(b)));
	} else if (*cur == '+' || *cur == '-') {
		if (*(cur + 1) == 'i') {
			return makeComplex(rational2Real(a), (*cur == '+') ? 1. : -1.);
		}
		b = getReal();
		if (!succ) {
			return NULL;
		}
		return makeComplex(rational2Real(a), rational2Real(b));
	}
	if (*cur != '\0') {
		return NULL;
	}
	return a;
}

void exact2inexact(ComplexNode * a) {
	if (a->exact) {
		a->re = rational2Real(a);
		a->im = 0;
		a->exact = false;
	}
}

void inexact2exact(ComplexNode * a) { // TODO
}

Node * newComplex(const char * start) {
	succ = true;
	cur = start;
	radix = -1;
	int exact = -1;
	if (*cur == '#') {
#define switch_ch \
		switch (*cur) {\
			case 'b': radix = 2; break;\
			case 'o': radix = 8; break;\
			case 'd': radix = 10; break;\
			case 'x': radix = 16; break;\
			case 'e': exact = 1; break;\
			case 'i': exact = 0; break;\
		}
		cur++;
		switch_ch;
		cur++;
		if (*cur == '#') {
			cur++;
			switch (*cur) {
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
			cur++;
		}
	}
	if (radix != 2 && radix != 8 && radix != 10 && radix != 16) {
		radix = 10;
	}
	ComplexNode * ret = parseComplex(start);
	if (ret != NULL) {
		if (exact == 1 && ret->exact == 0) {
			exact2inexact(ret);
		} else if (exact == 0 && ret->exact == 1) {
			inexact2exact(ret);
		}
	}
	return (Node *)ret;
}
