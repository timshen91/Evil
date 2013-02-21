#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "symbol.h"

static const char * table[4096]; // FIXME
static int top = 0;

Symbol getSym(const char * s) {
	for (int i = 0; i < top; i++) {
		if (strcmp(s, table[i]) == 0) {
			return i;
		}
	}
	assert(top < 4096);
	char * buff = malloc(strlen(s) + 1);
	strcpy(buff, s);
	table[top++] = buff;
	return top - 1;
}

const char * symToStr(Symbol sym) {
	return table[sym];
}
