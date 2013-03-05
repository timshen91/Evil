#include <assert.h>
#include <stdlib.h>

static void * objs[4096]; // FIXME
static int top = 0;

void * alloc(size_t size) {
	assert(top < 4096);
	// FIXME mysterious core dump without the "+1"
	objs[top++] = malloc(size);
	return objs[top - 1];
}

void gc() {
	// TODO
}
