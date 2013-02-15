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
		printf("%s", symToStr(node->sym));
		break;
	case PAIR: {
		DUMP("temp");
			Node * now = node;
			if (now != NULL) {
				DUMP("temp");
				if (now->a == NULL) {
					printf("()");
				} else {
					if (now->a->sym == getSymbol("quote")) {
						printf("'");
						printNode(now->b->a);
					} else if (now->a->sym == getSymbol("unquote")) {
						printf(",");
						if (now->b->a->type == SYMBOL && symToStr(now->b->a->sym)[0] == '@') {
							printf(" ");
						}
						printNode(now->b->a);
					} else if (now->a->sym == getSymbol("quasiquote")) {
						printf("`");
						printNode(now->b->a);
					} else if (now->a->sym == getSymbol("unquote-splicing")) {
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
			for (i = 0; i < node->len; i++) {
				printNode(((Node **)(node + 1))[i]);
				if (i + 1 < node->len) {
					printf(" ");
				}
			}
			printf(")");
		}
	case BOOLLIT:
		DUMP("temp");
		printf("%s", (node->lit) ? "#t" : "#f");
		break;
	case NUMLIT:
		DUMP("temp");
		printf("{NUMLIT}");
		break;
	case CHARLIT:
		DUMP("temp");
		printf("%c", (char)node->lit);
		break;
	case STRLIT: {
		DUMP("temp");
			printf("\"");
			int i;
			for (i = 0; i < node->len; i++) {
				printf("%c", ((char *)(node + 1))[i]);
			}
			printf("\"");
		}
		break;
	}
}

static int isExpo(char ch) {
	return ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l';
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

static const char * getInteger(const char * s) {
	if (!isdigit(*s)) {
		return s;
	}
	while (isdigit(*s)) {
		s++;
	}
	while (*s == '#') {
		s++;
	}
	return s;
}

static const char * getReal(char * ret, const char * s, int dec) {
	const char * cur = s;
	const char * t;
	ret[0] = '\0';
	if (*cur == '+' || *cur == '-') {
		cur++;
	}
	t = cur;
	if ((cur = getInteger(cur)) == t) {
		goto decimal;
	}
	if (*cur == '/') {
		cur++;
		t = cur;
		if ((cur = getInteger(cur)) == t) {
			goto decimal;
		}
	}
	goto out;
decimal:
	if (!dec) {
		cur = s;
		goto out;
	}
	cur = s;
	t = cur;
	cur = getInteger(cur);
	if (cur == t) {
		if (*cur != '.') {
			cur = s;
			goto out;
		}
		cur++;
		t = cur;
		if ((cur = getInteger(cur)) == t) {
			cur = s;
			goto out;
		}
	} else {
		while (isdigit(*cur)) {
			cur++;
		}
		if (*cur == '.') {
			cur = getInteger(cur);
		} else if (*cur == '#') {
			while (*cur == '#') {
				cur++;
			}
			if (*cur != '.') {
				cur = s;
				goto out;
			}
			while (*cur == '#') {
				cur++;
			}
		}
	}
	if (isExpo(*cur)) {
		cur++;
	} else {
		goto out;
	}
	if (*cur == '+' || *cur == '-') {
		cur++;
	}
	t = cur;
	if ((cur = getInteger(cur)) != t) {
		cur = s;
		goto out;
	}
out:
	memcpy(ret, s, cur - s);
	ret[cur - s] = '\0';
	return cur;
}

static Node * nextNum(const char * s, int radix, int exact) {
	if (radix != 2 && radix != 8 && radix != 10 && radix != 16) {
		radix = 10;
	}
	if (strcmp(s, "+i") == 0) {
		return newComplex("0", "1", 2);
	} else if (strcmp(s, "-i") == 0) {
		return newComplex("0", "-1", 2);
	}
	char a[4096], b[4096];
	a[0] = 0;
	b[0] = 0;
	const char * t = s;
	s = getReal(a, s, radix == 10);
	if (t == s) {
		return NULL;
	}
	if (*t == '+' || *t == '-') {
		if (*s == 'i') {
			return newComplex("0", a, radix);
		}
	}
	if (*s == '@') {
		s++;
		t = s;
		s = getReal(b, s, radix == 10);
		if (t == s) {
			return NULL;
		}
		return polar2Cart(newComplex(a, b, radix));
	} else if (*s == '+' || *s == '-') {
		t = s;
		s = getReal(b, s, radix == 10);
		return newComplex(a, b, radix);
	}
	if (*s != '\0') {
		return NULL;
	}
	return newComplex(a, "0", radix);
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
		if ((num = nextNum(buff, -1, -1)) == NULL) {
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
		ch = getchar();
		ret.type = LIT;
		if (ch == 't') {
			ret.lit = newBool(1);
			ch = getchar();
		} else if (ch == 'f') {
			ret.lit = newBool(0);
			ch = getchar();
		} else if (ch == 'e' || ch == 'i' || ch == 'b' || ch == 'o' || ch == 'd' || ch == 'x') {
			int radix = -1;
			int exact = -1;
#define switch_ch \
			switch (ch) {\
			case 'b':\
				radix = 2;\
				break;\
			case 'o':\
				radix = 8;\
				break;\
			case 'd':\
				radix = 10;\
				break;\
			case 'x':\
				radix = 16;\
				break;\
			case 'e':\
				exact = 1;\
				break;\
			case 'i':\
				exact = 0;\
				break;\
			}
			switch_ch;
			ch = getchar();
			if (ch == '#') {
				ch = getchar();
				switch (ch) {
				case 'e':
				case 'i':
					if (exact != -1) {
						error("duplicated exactness requirement");
						err = LEX;
						return ret;
					}
					break;
				case 'b':
				case 'o':
				case 'd':
				case 'x':
					if (radix != -1) {
						error("duplicated exactness requirement");
						err = LEX;
						return ret;
					}
					break;
				default:
					error("unknown exactness or radix");
					err = LEX;
					return ret;
				}
				switch_ch;
				ch = getchar();
			}
			while (isdigit(ch) || isalpha(ch) || isSpec(ch)) {
				buff[len++] = ch;
				ch = getchar();
			}
			buff[len] = '\0';
			if ((ret.lit = nextNum(buff, radix, exact)) == NULL) {
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
