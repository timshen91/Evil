#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parse.h"
#include "structure.h"
#include "symbol.h"
#include "eval.h"
#include "error.h"
#include "environment.h"

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
				if (car(node)->type == SYMBOL && cdr(node)->type != EMPTY && (cdr(node)->type == node->type)) {
					SymNode * a = (SymNode *)car(node);
					PairNode * b = (PairNode *)cdr(node);
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
						printNode(car(node));
						if (cdr(node)->type == EMPTY) {
							break;
						}
						printf(" ");
						if (cdr(node)->type != node->type) {
							printf(". ");
							printNode(cdr(node));
							break;
						}
						node = cdr(node);
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
		case BOOL:
			DUMP("temp");
			printf("%s", (((BoolNode *)node)->value) ? "#t" : "#f");
			break;
		case COMPLEX:
			DUMP("temp");
			printf("{COMPLEX}");
			break;
		case CHAR:
			DUMP("temp");
			printf("%c", ((CharNode *)node)->value);
			break;
		case STRING:
			DUMP("temp");
			printf("\"");
			for (int i = 0; i < toString(node)->len; i++) {
				printf("%c", toString(node)->str[i]);
			}
			printf("\"");
			break;
		case LIST_LAMBDA:
			printf("{LAMBDA}");
			break;
		case PAIR_LAMBDA:
			printf("{LAMBDA}");
			break;
		case MARG:
			printf("{MARG}");
			break;
		case LISTELL:
			printf("{LISTELL}");
			break;
		case VECTORELL:
			printf("{VECTORELL}");
			break;
		case MACRO:
			printf("{MACRO_DEF}"); // TODO
			break;
		case BUILTIN:
			printf("{BUILTIN}");
			break;
	}
}

char mem[4096];
Env * topEnv = (Env *)mem;
int main(int argc, char * argv[]) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin);
	}
	void initMacro();
	initMacro();
	topEnv->parent = NULL;
	while (1) {
		if (!setjmp(jmpBuff)) {
			//eval(parse(), topEnv);
			printNode(eval(parse(), topEnv));
			printf("\n");
		}
	}
	return 0;
}
