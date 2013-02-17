#ifndef __SYMBOL_H__
#define __SYMBOL_H__

enum SpecSym {
	SYM_DEFINE_SYNTAX,
	SYM_DEFINE,
	SYM_LAMBDA,
};

unsigned long getSym(const char * s);
const char * symToStr(unsigned long);

#endif
