#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

typedef struct Token Token;

// トークンの種類
typedef enum {
	// 記号
	TK_RESERVED,
	// 識別子
	TK_IDENT,
	// 整数トークン
	TK_NUM,
	// 入力の終わりを表すトークン
	TK_EOF,
} TokenKind;

// トークン型
struct Token {
	// トークン型
	TokenKind kind;
	// 次の入力トークン
	Token *next;
	// kindがTK_NUMの場合: その数値
	int val; 
	// トークン文字列
	char *str;
	// トークンの長さ
	int len;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	// pos個の空白を出力
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool expect_token(char *op) {
	switch (token->kind) {
		case TK_RESERVED:
			break;
		case TK_IDENT:
			break;
		default:
			return false;
	}
	if (strlen(op) != token->len)
		return false;
	if (memcmp(token->str, op, token->len))
		return false;
	return true;
}

// 次のトークンが期待している記号の時には、
// トークンを1つ読み進めて真を返す。
// それ以外の場合には偽を返す。
bool consume(char *op) {
	bool result = expect_token(op);
	if (result)
		token = token->next;
	return result;
}

// 現在のトークンが変数の場合、
// トークンを読み進めたうえで変数のトークンを返却します。
Token *consume_ident() {
	Token *tok = NULL;
	bool result = expect_token("a") || expect_token("b") || expect_token("c");
	if (result) {
		tok = token;
		token = token->next;
	}
	return tok;
}

// 次のトークンが期待している記号のときには、
// トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
	if (!expect_token(op))
		error_at(token->str, "'%s'ではありません。", op);
	token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません。");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる。
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool starts_with(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズしてそれを返す。
void *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;
	user_input = p;

	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		// 2文字記号
		if (starts_with(p, "==") || starts_with(p, "!=") ||
				starts_with(p, "<=") || starts_with(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		// 1文字記号
		if (strchr("+-*/()<>;=", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		// 1文字アルファベット
		if ('a' <= *p && *p <= 'c') {
			cur = new_token(TK_IDENT, cur, p++, 1);
			continue;
		}

		// 数値
		if (isdigit(*p)) {
			char *q = p;
			cur = new_token(TK_NUM, cur, p, 0);
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(p, "不正なトークンです。");
	}

	new_token(TK_EOF, cur, p, 0);
	token = head.next;
}

Node *statement(void);
Node *expression(void);
Node *assign(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);

NodeList *new_node_list(NodeList *cur, Node *node) {
	NodeList *nodes = calloc(1, sizeof(NodeList));
	nodes->node = node;
	cur->next = nodes;
	return nodes;
}

NodeList *parse_program() {
	NodeList head;
	NodeList *cur = &head;
	int i = 0;
	while (!at_eof()) {
		Node *node = statement();
		cur = new_node_list(cur, node);
	}
	return head.next;
}

Node *statement() {
	Node *node = expression();
	expect(";");
	return node;
}

// Parse expr
Node *expression() {
	return assign();
}

Node *assign() {
	Node *node = equality();
	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return node;
}

// Parse equality
Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}

// Parse relational
Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

// Parse add
Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

// Parse mul
Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

// Parse unary
Node *unary() {
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

// Parse primary
Node *primary() {
	// 次のトークンが"("なら、"(" expr ")"のはず。
	if (consume("(")) {
		Node *node = expression();
		expect(")");
		return node;
	}

	Token *tok = consume_ident();
	if (tok) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;
		node->offset = (tok->str[0] - 'a' + 1) * 8;
		return node;
	}

	return new_node_num(expect_number());
}
