#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parse.h"
#include "structure.h"
#include "symbol.h"
#include "eval.h"
#include "error.h"

jmp_buf jmpBuff;

static void printNode(Node * node) {
	if (node == NULL) {
		DUMP("temp");
		printf("()");
		return;
	}
	switch (node->type) {
		case SYMBOL:
			DUMP("temp");
			printf("%s", symToStr(((SymNode *)node)->sym));
			break;
		case LIST:
		case PAIR: {
			DUMP("temp");
				PairNode * now = (PairNode *)node;
				if (now != NULL) {
					DUMP("temp");
					if (now->a == NULL) {
						printf("()");
					} else {
						if (now->a->type == SYMBOL && now->b != NULL && (now->b->type == node->type)) {
							SymNode * a = (SymNode *)now->a;
							PairNode * b = (PairNode *)now->b;
							if (a->sym == getSym("quote")) {
								printf("'");
								printNode(b->a);
							} else if (a->sym == getSym("unquote")) {
								printf(",");
								if (b->a->type == SYMBOL && b->a->type == SYMBOL && symToStr(((SymNode *)b->a)->sym)[0] == '@') {
									printf(" ");
								}
								printNode(b->a);
							} else if (a->sym == getSym("quasiquote")) {
								printf("`");
								printNode(b->a);
							} else if (a->sym == getSym("unquote-splicing")) {
								printf(",@");
								printNode(b->a);
							} else {
								goto there;
							}
						} else {
there:
							printf("(");
							while (1) {
								printNode(now->a);
								if (now->b == NULL) {
									break;
								}
								printf(" ");
								if (now->b->type != node->type) {
									printf(". ");
									printNode(now->b);
									break;
								}
								now = (PairNode *)now->b;
							}
							printf(")");
						}
					}
				}
			}
			break;
		case VECTOR: {
				DUMP("temp");
				int i;
				VecNode * now = (VecNode *)node;
				printf("#(");
				for (i = 0; i < now->len; i++) {
					printNode(((Node **)(now + 1))[i]);
					if (i + 1 < now->len) {
						printf(" ");
					}
				}
				printf(")");
			}
		case BOOLLIT:
			DUMP("temp");
			printf("%s", (((BoolNode *)node)->value) ? "#t" : "#f");
			break;
		case NUMLIT:
			DUMP("temp");
			printf("{NUMLIT}");
			break;
		case CHARLIT:
			DUMP("temp");
			printf("%c", ((CharNode *)node)->value);
			break;
		case STRLIT: {
				DUMP("temp");
				printf("\"");
				int i;
				StrLitNode * now = (StrLitNode *)node;
				for (i = 0; i < now->len; i++) {
					printf("%c", ((char *)(now + 1))[i]);
				}
				printf("\"");
			}
			break;
		case LAMBDA:
			assert(0);
			break;
	}
}

void init() {
	if (getSym("define_syntax") != SYM_DEFINE_SYNTAX ||
		getSym("define") != SYM_DEFINE ||
		getSym("lambda") != SYM_LAMBDA) {
		exit(0);
	}
}

int depth;
int main(int argc, char * argv[]) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin);
	}
	init();
	static char mem[4096];
	Env * topEnv = (Env *)mem;
	topEnv->parent = NULL;
	while (1) {
		if (!setjmp(jmpBuff)) {
			depth = 0;
			printNode(eval(parse(), topEnv));
			printf("\n");
		}
	}
	return 0;
}
