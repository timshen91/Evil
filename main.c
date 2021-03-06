#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parse.h"
#include "structure.h"
#include "symbol.h"
#include "eval.h"
#include "error.h"
#include "complex.h"
#include "environment.h"
#include "builtin.h"

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
			printf("%s", node == &boolTrue ? "#t" : "#f");
			break;
		case COMPLEX:
			DUMP("temp");
			if (toComplex(node)->exact) {
				if (toComplex(node)->de != 1) {
					printf("%lld/%lld", toComplex(node)->nu, toComplex(node)->de);
				} else {
					printf("%lld", toComplex(node)->nu);
				}
			} else {
				if (toComplex(node)->im) {
					printf("%lf-%lfi", toComplex(node)->re, -toComplex(node)->im);
				} else {
					printf("%lf+%lfi", toComplex(node)->re, toComplex(node)->im);
				}
			}
			break;
		case CHAR:
			DUMP("temp");
			printf("%c", toChar(node)->value);
			break;
		case STRING:
			DUMP("temp");
			printf("\"");
			for (int i = 0; i < toString(node)->len; i++) {
				printf("%c", toString(node)->str[i]);
			}
			printf("\"");
			break;
		case FIX_LAMBDA:
			printf("{LAMBDA}");
			break;
		case VAR_LAMBDA:
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
			printf("{MACRO_DEF}");
			break;
		case BUILTIN_MAC:
			printf("{BUILTIN_MAC}");
		case BUI_LAMBDA:
			printf("{BUI_LAMBDA}");
		case UNSPECIFIED:
			printf("{UNSPECIFIED}");
			break;
	}
}

int main(int argc, char * argv[]) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin);
	}
	void initBuiltin();
	initBuiltin();
	while (1) {
		if (!setjmp(jmpBuff)) {
			Node * node = parse();
			Node * ret = eval(node, &topEnv);
			if (ret) {
				printNode(ret);
				printf("\n");
			}
		}
	}
	return 0;
}
