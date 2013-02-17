#include <string.h>
#include <stdlib.h>
#include <assert.h>

static const char * table[4096]; // FIXME
static int top = 0;

unsigned long getSym(const char * s) {
	int i;
	for (i = 0; i < top; i++) {
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

const char * symToStr(unsigned long sym) {
	return table[sym];
}
