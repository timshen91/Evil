#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "symbol.h"
#include "parser.h"
#include "structure.h"

//#define DUMP(s) printf("%s : %d !!!\n", s, __LINE__)
#define DUMP(s) 

#define error(s) { fprintf(stderr, "%s\n", s); }

static enum {
	OK,
	EOLIST,
	PRD,
	END_OF_FILE,
	LEX,
} err;

static void printNode(Node * node) {
	DUMP("temp");
	if (node == NULL) {
		DUMP("temp");
		printf("()");
		return;
	}
	DUMP("temp");
	switch (node->type) {
	case SYMBOL:
		DUMP("temp");
		printf("%s", symToStr(((SymNode *)node)->sym));
		break;
	case PAIR: {
		DUMP("temp");
			PairNode * now = (PairNode *)node;
			if (now != NULL) {
				DUMP("temp");
				if (now->a == NULL) {
					printf("()");
				} else {
					if (now->a->type == SYMBOL && now->b != NULL && now->b->type == PAIR) {
						SymNode * a = (SymNode *)now->a;
						PairNode * b = (PairNode *)now->b;
						if (a->sym == getSymbol("quote")) {
							printf("'");
							printNode(b->a);
						} else if (a->sym == getSymbol("unquote")) {
							printf(",");
							if (b->a->type == SYMBOL && b->a->type == SYMBOL && symToStr(((SymNode *)b->a)->sym)[0] == '@') {
								printf(" ");
							}
							printNode(b->a);
						} else if (a->sym == getSymbol("quasiquote")) {
							printf("`");
							printNode(b->a);
						} else if (a->sym == getSymbol("unquote-splicing")) {
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
							if (now->b->type != PAIR) {
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
	}
}

static int isSpec(char ch) {
	const char * a;
	for (a = "!$%&*+-./:<=>?@^_~"; *a != '\0'; a++) {
		if (*a == ch) {
			return 1;
		}
	}
	return 0;
}

Token nextToken() {
	static int ch = ' ';
	Token ret;
	while (isspace(ch)) {
		ch = getchar();
	}
	char buff[4096]; // FIXME
	int len = 0;
	if (ch == '(') {
		DUMP("lparen");
		ret.type = LPAREN;
		ch = getchar();
	} else if (ch == ')') {
		DUMP("rparen");
		ret.type = RPAREN;
		ch = getchar();
	} else if (isdigit(ch) || isalpha(ch) || isSpec(ch)) {
		DUMP("sym");
		while (isdigit(ch) || isalpha(ch) || isSpec(ch)) {
			buff[len++] = ch;
			ch = getchar();
		}
		buff[len] = '\0';
		Node * num;
		if ((num = newComplex(buff)) == NULL) {
			if (strcmp(buff, ".") == 0) {
				ret.type = PERIOD;
			} else {
				ret.type = LIT;
				ret.lit = newSymbol(buff);
			}
		} else {
			ret.type = LIT;
			ret.lit = num;
		}
	} else if (ch == ';') {
		DUMP("comment");
		while (getchar() != '\n');
		ch = getchar();
		return nextToken();
	} else if (ch == '\"') {
		DUMP("strlit");
		ch = getchar();
		while (ch != '\"') {
			if (ch == '\\') {
				buff[len++] = getchar();
				ch = getchar();
				continue;
			}
			buff[len++] = ch;
			ch = getchar();
		}
		buff[len] = '\0';
		ret.type = LIT;
		ret.lit = newStrLit(buff, len);
		ch = getchar();
	} else if (ch == '\'') {
		ret.type = QUOTE;
		ch = getchar();
	} else if (ch == '`') {
		ret.type = QUASIQUOTE;
		ch = getchar();
	} else if (ch == ',') {
		ch = getchar();
		if (ch == '@') {
			ret.type = COMMAAT;
			ch = getchar();
		} else {
			ret.type = COMMA;
		}
	} else if (ch == '#') {
		DUMP("#");
		buff[len++] = ch;
		ch = getchar();
		ret.type = LIT;
		if (ch == 't') {
			ret.lit = newBool(1);
			ch = getchar();
		} else if (ch == 'f') {
			ret.lit = newBool(0);
			ch = getchar();
		} else if (ch == 'e' || ch == 'i' || ch == 'b' || ch == 'o' || ch == 'd' || ch == 'x') {
			while (!isspace(ch)) {
				buff[len++] = ch;
				ch = getchar();
			}
			buff[len] = '\0';
			if ((ret.lit = newComplex(buff)) == NULL) {
				error("invalid number");
				err = LEX;
				return ret;
			}
		} else if (ch == '(') {
			ret.type = HASHLPAREN;
			ch = getchar();
		} else if (ch == '\\') {
			ret.type = LIT;
			ch = getchar();
			buff[len++] = ch;
			while (!isspace(ch = getchar())) {
				buff[len++] = ch;
			}
			buff[len] = '\0';
			if (strcmp(buff, "space") == 0) {
				ret.lit = newChar(' ');
			} else if (strcmp(buff, "newline") == 0) {
				ret.lit = newChar('\n');
			} else {
				if (len != 1 && isalpha(buff[0])) {
					error("unknown character name");
					err = LEX;
					return ret;
				} else {
					ret.lit = newChar(ch);
				}
			}
		} else {
			error("unknown object");
			err = LEX;
			return ret;
		}
	} else if (ch == EOF) {
		ret.type = END;
	}
	err = OK;
	return ret;
}

Node * parse() {
	Token tok = nextToken();
	if (err != OK) {
		return NULL;
	}
	err = OK;
	switch (tok.type) {
	case LPAREN: {
			PairNode * ret;
			Node ** last = (Node **)&ret; 
			while (1) {
				Node * now = parse();
				if (err != OK) {
					if (err == EOLIST) {
						err = OK;
						*last = NULL;
						break;
					}
					if (err == PRD) {
						err = OK;
						*last = parse();
					}
					return NULL;
				}
				*last = cons(now, NULL);
				last = &(*(PairNode **)last)->b;
			}
			return (Node *)ret;
		}
	case RPAREN:
		err = EOLIST;
		return NULL;
	case LIT:
		return tok.lit;
	case PERIOD:
		err = PRD;
		return NULL;
	case QUOTE:
		return cons(newSymbol("quote"), cons(parse(), NULL));
	case QUASIQUOTE:
		return cons(newSymbol("quasiquote"), cons(parse(), NULL));
	case COMMA:
		return cons(newSymbol("unquote"), cons(parse(), NULL));
	case COMMAAT:
		return cons(newSymbol("unquote-splicing"), cons(parse(), NULL));
	case HASHLPAREN: {
			Node * v[4096]; // FIXME
			int len = 0;
			while (1) {
				Node * now = parse();
				if (err != OK) {
					if (err == EOLIST) {
						err = OK;
						break;
					}
				}
				assert(len < 4096);
				v[len++] = now;
			}
			Node * ret = newVector(len);
			memcpy(ret + 1, v, sizeof(Node *) * len);
			return ret;
		}
	case END:
		err = END_OF_FILE;
		break;
	}
	return NULL;
}

int main(int argc, char * argv[]) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin);
	}
	while (1) {
		Node * ret = parse();
		DUMP("temp");
		if (err != OK) {
			if (err == END_OF_FILE) {
				break;
			}
			continue;
		}
		DUMP("temp");
		printNode(ret);
		printf("\n");
	}
	return 0;
}
