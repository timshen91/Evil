#ifndef __COMPLEX_H__
#define __COMPLEX_H__

typedef long long Integer;
typedef double Real;

typedef struct ComplexNode {
	enum NodeType type;
	bool exact;
	union {
		struct {
			Integer nu, de;
		};
		struct {
			Real re, im;
		};
	};
} ComplexNode;

ComplexNode * makeRational(Integer, Integer);
ComplexNode * makeComplex(Real, Real);
void exact2inexact(ComplexNode * a);

#endif
