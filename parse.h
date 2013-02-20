#ifndef __PARSE_H__
#define __PARSE_H__

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
};

typedef struct Node Node;
typedef struct Token {
	enum TokenType type;
	Node * lit;
} Token;

Node * parse();

#endif
