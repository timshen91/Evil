#ifndef __PARSER_H__
#define __PARSER_H__

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

typedef struct Node Node;

typedef struct Token {
	enum TokenType type;
	Node * lit;
} Token;

#endif
