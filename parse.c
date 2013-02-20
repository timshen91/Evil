#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "parse.h"
#include "structure.h"
#include "symbol.h"
#include "error.h"

int depth = 0;

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
				abort();
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
					abort();
				} else {
					ret.lit = newChar(ch);
				}
			}
		} else {
			error("unknown object");
			abort();
		}
	} else if (ch == EOF) {
		if (depth != 0) {
			error("unexpected end of file");
		}
		exit(0);
	}
	return ret;
}

Node * parse();
static int endOfList = 0;
Node * parseList() {
	Node * a = parse();
	if (endOfList) {
		endOfList = 0;
		return a;
	}
	Node * b = parseList();
	return cons(a, b);
}

Node * parse() {
	Token tok = nextToken();
	switch (tok.type) {
		case LPAREN:
			depth++;
			return parseList();
		case RPAREN:
			if (depth <= 0) {
				error("unmatched RPAREN");
				abort();
			}
			depth--;
			endOfList = 1;
			return &empty;
		case LIT:
			return tok.lit;
		case PERIOD: {
			Node * ret = parse();
			parse();
			if (endOfList != 1) {
				error("unexpected end of pair");
				abort();
			}
			return ret;
		}
		case QUOTE:
		case QUASIQUOTE:
		case COMMA:
		case COMMAAT: {
			Node * now = parse();
			Node * sym;
			switch (tok.type) {
				case QUOTE:
					sym = newSymbol("quote");
					break;
				case QUASIQUOTE:
					sym = newSymbol("quasiquote");
					break;
				case COMMA:
					sym = newSymbol("unquote");
					break;
				case COMMAAT:
					sym = newSymbol("unquote-splicing");
					break;
				default:
					assert(0);
			}
			return LIST2(sym, now);
		}
		case HASHLPAREN: {
			Node * v[4096]; // FIXME
			int len = 0;
			while (1) {
				Node * now = parse();
				assert(len < 4096);
				v[len++] = now;
			}
			Node * ret = newVector(len);
			memcpy(toVec(ret)->vec, v, sizeof(Node *) * len);
			return ret;
		}
	}
	assert(0);
}
