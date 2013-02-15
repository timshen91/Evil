#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "memory.h"
#include "symbol.h"

//#define DUMP(s) printf("%s : %d !!!\n", s, __LINE__)
#define DUMP(s) 

enum TokenType {
	LPAREN,
	RPAREN,
	LIT,
	HASHLPAREN,
	QUOTE,
	QUASIQUOTE,
	COMMA,
	COMMAAT,
	PERIOD,
	END,
};

enum NodeType {
	SYMBOL,
	PAIR,
	VECTOR,
	BOOLLIT,
	NUMLIT,
	CHARLIT,
	STRLIT,
};

typedef struct Node {
	enum NodeType type;
	union {
		struct {
			struct Node * a;
			struct Node * b;
		};
		unsigned long dscr;
	};
} Node;

typedef struct Token {
	enum TokenType type;
	Node * lit;
} Token;

static int isSpec(char ch) {
	const char * a;
	for (a = "!$%&*+-./:<=>?@^_~"; *a != '\0'; a++) {
		if (*a == ch) {
			return 1;
		}
	}
	return 0;
}

Node * newSymbol(const char * s) {
	Node * ret = alloc(sizeof(Node));
	ret->type = SYMBOL;
	ret->dscr = getSymbol(s);
	return ret;
}

//const char * getUnreal(const char * s, int dec) {
//}
//
//Node * newNum(const char * s, int radix, int exact) {
//	if (radix != 2 && radix != 8 && radix != 10 && radix != 16) {
//		radix = 10;
//	}
//	if (strcmp(s, "+i") == 0) {
//		return string2num(1, "0", 1, "1", 1);
//	} else if (strcmp(s, "-i")) {
//		return string2num(1, "0", -1, "1", 1);
//	}
//	int sign = 1;
//	int i = 0;
//	if (s[0] == '+') {
//		sign = 1;
//		i++;
//	} else if (s[0] == '-') {
//		sign = -1;
//		i++;
//	}
//	const char * firstEnd;
//	Node * first = getUnreal(&firstEnd, s + i, radix == 10);
//	const char * secondEnd;
//	if (*firstEnd == '@') {
//		Node * second = getUnreal(&secondEnd, firstEnd + 1, radix == 10);
//	} else if (*firstEnd == '+' || *firstEnd == '-') {
//		Node * second = getUnreal(&secondEnd, firstEnd + 1, radix == 10);
//		if (*secondEnd != 'i') {
//			return NULL;
//		}
//	} else if (*firstEnd == 'i') {
//		if (i == 0) {
//			return NULL;
//		}
//	}
//	return NULL;
//}

Node * newVector(int len) {
	Node * ret = alloc(sizeof(Node) + len * sizeof(Node *));
	ret->type = VECTOR;
	ret->dscr = len;
	return ret;
}

Node * newStrLit(const char * s, int len) {
	Node * ret = alloc(sizeof(Node) + len);
	memcpy(ret + 1, s, len);
	ret->type = STRLIT;
	ret->dscr = len;
	return ret;
}

Node * newBool(int b) {
	Node * ret = alloc(sizeof(Node));
	ret->type = BOOLLIT;
	ret->dscr = b;
	return ret;
}

Node * newChar(char ch) {
	Node * ret = alloc(sizeof(Node));
	ret->type = CHARLIT;
	ret->dscr = ch;
	return ret;
}

const char * nextToken(Token * ret) {
	static int ch = ' ';
	while (isspace(ch)) {
		ch = getchar();
	}
	char buff[4096]; // FIXME
	int len = 0;
	if (ch == '(') {
		DUMP("lparen");
		ret->type = LPAREN;
		ch = getchar();
	} else if (ch == ')') {
		DUMP("rparen");
		ret->type = RPAREN;
		ch = getchar();
	} else if (isdigit(ch) || isalpha(ch) || isSpec(ch)) {
		DUMP("sym");
		while (isdigit(ch) || isalpha(ch) || isSpec(ch)) {
			buff[len++] = ch;
			ch = getchar();
		}
		buff[len] = '\0';
		//Node * num;
		//if ((num = newNum(buff, -1, -1)) == NULL) {
			if (strcmp(buff, ".") == 0) {
				ret->type = PERIOD;
			} else {
				ret->type = LIT;
				ret->lit = newSymbol(buff);
			}
		//} else {
		//	ret->type = LIT;
		//	ret->lit = num;
		//}
	} else if (ch == ';') {
		DUMP("comment");
		while (getchar() != '\n');
		ch = getchar();
		return nextToken(ret);
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
		ret->type = LIT;
		ret->lit = newStrLit(buff, len);
		ch = getchar();
	} else if (ch == '\'') {
		ret->type = QUOTE;
		ch = getchar();
	} else if (ch == '`') {
		ret->type = QUASIQUOTE;
		ch = getchar();
	} else if (ch == ',') {
		ch = getchar();
		if (ch == '@') {
			ret->type = COMMAAT;
			ch = getchar();
		} else {
			ret->type = COMMA;
		}
	} else if (ch == '#') {
		DUMP("#");
		ch = getchar();
		ret->type = LIT;
		if (ch == 't') {
			ret->lit = newBool(1);
			ch = getchar();
		} else if (ch == 'f') {
			ret->lit = newBool(0);
			ch = getchar();
		//} else if (ch == 'e' || ch == 'i' || ch == 'b' || ch == 'o' || ch == 'd' || ch == 'x') {
		//	int radix = -1;
		//	int exact = -1;
		//	switch (ch) {
		//	case 'b':
		//		radix = 2;
		//		break;
		//	case 'o':
		//		radix = 8;
		//		break;
		//	case 'd':
		//		radix = 10;
		//		break;
		//	case 'x':
		//		radix = 16;
		//		break;
		//	case 'e':
		//		exact = 1;
		//		break;
		//	case 'i':
		//		exact = 0;
		//		break;
		//	}
		//	ch = getchar();
		//	if (ch == '#') {
		//		ch = getchar();
		//		switch (ch) {
		//		case 'e':
		//		case 'i':
		//			if (exact != -1) {
		//				return "duplicated exactness require";
		//			}
		//			break;
		//		case 'b':
		//		case 'o':
		//		case 'd':
		//		case 'x':
		//			if (radix != -1) {
		//				return "duplicated exactness require";
		//			}
		//			break;
		//		default:
		//			return "unknown exactness or radix";
		//		}
		//		switch (ch) {
		//		case 'b':
		//			radix = 2;
		//			break;
		//		case 'o':
		//			radix = 8;
		//			break;
		//		case 'd':
		//			radix = 10;
		//			break;
		//		case 'x':
		//			radix = 16;
		//			break;
		//		case 'e':
		//			exact = 1;
		//			break;
		//		case 'i':
		//			exact = 0;
		//			break;
		//		}
		//		ch = getchar();
		//	}
		//	while (isdigit(ch) || isalpha(ch) || isSpec(ch)) {
		//		buff[len++] = ch;
		//		ch = getchar();
		//	}
		//	buff[len] = '\0';
		//	if ((ret->lit = newNum(buff, radix, exact)) == NULL) {
		//		return "invalid number";
		//	}
		} else if (ch == '(') {
			ret->type = HASHLPAREN;
			ch = getchar();
		} else if (ch == '\\') {
			ret->type = LIT;
			ch = getchar();
			buff[len++] = ch;
			while (!isspace(ch = getchar())) {
				buff[len++] = ch;
			}
			buff[len] = '\0';
			if (strcmp(buff, "space") == 0) {
				ret->lit = newChar(' ');
			} else if (strcmp(buff, "newline") == 0) {
				ret->lit = newChar('\n');
			} else {
				if (len != 1 && isalpha(buff[0])) {
					return "unknown character name";
				} else {
					ret->lit = newChar(ch);
				}
			}
		} else {
			return "unknown object";
		}
	} else if (ch == EOF) {
		ret->type = END;
	}
	return NULL;
}

Node * cons(Node * a, Node * b) {
	Node * ret = alloc(sizeof(Node));
	ret->type = PAIR;
	ret->a = a;
	ret->b = b;
	return ret;
}

enum {
	OK,
	EOLIST,
	PRD,
	END_OF_FILE,
	LEX,
} err;

Node * parse() {
	Token tok;
	const char * errs;
	if ((errs = nextToken(&tok)) != NULL) {
		err = LEX;
		printf("%s", errs);
		return NULL;
	}
	err = OK;
	switch (tok.type) {
	case LPAREN: {
			Node * ret;
			Node ** last = &ret; 
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
				last = &(*last)->b;
			}
			return ret;
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
			Node * v[4096];
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
		if (err == END_OF_FILE) {
			break;
		}
		if (err != OK) {
			continue;
		}
		DUMP("temp");
		void printNode(Node *);
		printNode(ret);
		printf("\n");
	}
	return 0;
}

void printNode(Node * node) {
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
		printf("%s", symToStr(node->dscr));
		break;
	case PAIR: {
		DUMP("temp");
			Node * now = node;
			if (now != NULL) {
				DUMP("temp");
				if (now->a == NULL) {
					printf("()");
				} else {
					if (now->a->dscr == getSymbol("quote")) {
						printf("'");
						printNode(now->b->a);
					} else if (now->a->dscr == getSymbol("unquote")) {
						printf(",");
						if (now->b->a->type == SYMBOL && symToStr(now->b->a->dscr)[0] == '@') {
							printf(" ");
						}
						printNode(now->b->a);
					} else if (now->a->dscr == getSymbol("quasiquote")) {
						printf("`");
						printNode(now->b->a);
					} else if (now->a->dscr == getSymbol("unquote-splicing")) {
						printf(",@");
						printNode(now->b->a);
					} else {
						printf("(");
						for (now = node; now != NULL && now->type == PAIR; now = now->b) {
							printNode(now->a);
							if (now->b != NULL) {
								printf(" ");
							}
						}
						if (now != NULL) {
							printf(". ");
							printNode(now->b);
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
			printf("#(");
			for (i = 0; i < node->dscr; i++) {
				printNode(((Node **)(node + 1))[i]);
				if (i + 1 < node->dscr) {
					printf(" ");
				}
			}
			printf(")");
		}
	case BOOLLIT:
		DUMP("temp");
		printf("%s", (node->dscr) ? "#t" : "#f");
		break;
	case NUMLIT:
		DUMP("temp");
		printf("{NUMLIT}");
		break;
	case CHARLIT:
		DUMP("temp");
		printf("%c", (char)node->dscr);
		break;
	case STRLIT: {
		DUMP("temp");
			printf("\"");
			int i;
			for (i = 0; i < node->dscr; i++) {
				printf("%c", ((char *)(node + 1))[i]);
			}
			printf("\"");
		}
		break;
	}
}
