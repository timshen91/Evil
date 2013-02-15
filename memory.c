#include <assert.h>
#include <stdlib.h>

static void * objs[4096]; // FIXME
static int top = 0;

void * alloc(unsigned long size) {
	assert(top < 4096);
	return objs[top++] = malloc(size);
}

void gc() {
	// TODO
}
