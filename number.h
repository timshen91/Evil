#ifndef __NUMBER_H__
#define __NUMBER_H__

typedef struct Integer {
	int sign;
	unsigned long len;
} Integer;

typedef struct Rational {
} Rational;

typedef struct Real {
	int sign;
	unsigned long offset;
	unsigned long len;
} Real;

typedef struct Complex {
} Complex;

#endif
