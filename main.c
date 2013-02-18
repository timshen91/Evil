#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parse.h"
#include "structure.h"
#include "symbol.h"
#include "eval.h"
#include "error.h"

jmp_buf jmpBuff;

void printNode(Node * node) {
	switch (node->type) {
		case DUMMY:
			printf("_");
			break;
		case EMPTY:
			printf("()");
			break;
		case SYMBOL:
			DUMP("temp");
			printf("%s", symToStr(((SymNode *)node)->sym));
			break;
		case LIST:
		case PAIR: {
			DUMP("temp");
				DUMP("temp");
				if (toPair(node)->a->type == SYMBOL && toPair(node)->b->type != EMPTY && (toPair(node)->b->type == node->type)) {
					SymNode * a = (SymNode *)toPair(node)->a;
					PairNode * b = (PairNode *)toPair(node)->b;
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
						printNode(toPair(node)->a);
						if (toPair(node)->b->type == EMPTY) {
							break;
						}
						printf(" ");
						if (toPair(node)->b->type != node->type) {
							printf(". ");
							printNode(toPair(node)->b);
							break;
						}
						node = toPair(node)->b;
					}
					printf(")");
				}
			}
			break;
		case VECTOR:
			DUMP("temp");
			printf("#(");
			for (int i = 0; i < toVec(node)->len; i++) {
				printNode(toVec(node)->vec[i]);
				if (i + 1 < toVec(node)->len) {
					printf(" ");
				}
			}
			printf(")");
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
		case STRLIT:
			DUMP("temp");
			printf("\"");
			for (int i = 0; i < toString(node)->len; i++) {
				printf("%c", toString(node)->str[i]);
			}
			printf("\"");
			break;
		case LAMBDA:
			assert(0);
			break;
		case OFFSET:
			printf("{OFFSET}");
			break;
		case OFFSET_SLICE:
			printf("{OFFSET_SLICE}");
			break;
		case LISTELL:
			printf("{LISTELL}");
			break;
		case VECTORELL:
			printf("{VECTORELL}");
			break;
	}
}

int depth;
int main(int argc, char * argv[]) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin);
	}
	static char mem[4096];
	Env * topEnv = (Env *)mem;
	topEnv->parent = NULL;
	while (1) {
		if (!setjmp(jmpBuff)) {
			depth = 0;
			//eval(parse(), topEnv);
			printNode(eval(parse(), topEnv));
			printf("\n");
		}
	}
	return 0;
}
