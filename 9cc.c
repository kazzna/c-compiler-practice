#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;
typedef struct Token Token;

Node *expr(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);

typedef enum {
	// +
	ND_ADD,
	// -
	ND_SUB,
	// *
	ND_MUL,
	// /
	ND_DIV,
	// ==
	ND_EQ,
	// !=
	ND_NE,
	// <
	ND_LT,
	// <
	ND_LE,
	// 整数
	ND_NUM,
} NodeKind;

// 中小構文木のノードの型
struct Node {
	// ノードの型
	NodeKind kind;
	// 左辺
	Node *lhs;
	// 右辺
	Node *rhs;
	// kindがND_NUMの場合: 数値
	int val;
};

// トークンの種類
typedef enum {
	// 記号
	TK_RESERVED,
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
	if (token->kind != TK_RESERVED)
		return false;
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
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		// 複数文字記号
		if (starts_with(p, "==") || starts_with(p, "!=") ||
				starts_with(p, "<=") || starts_with(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		// 単文字記号
		if (strchr("+-*/()<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

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
	return head.next;
}

// Parse expr
Node *expr() {
	Node *node = equality();
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
		Node *node = expr();
		expect(")");
		return node;
	}

	return new_node_num(expect_number());
}

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind) {
		case ND_ADD:
			printf("  add rax, rdi\n");
			break;
		case ND_SUB:
			printf("  sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("  imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("  cqo\n");
			printf("  idiv rdi\n");
			break;
		case ND_EQ:
			printf("  cmp rax, rdi\n");
			printf("  sete al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_NE:
			printf("  cmp rax, rdi\n");
			printf("  setne al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LT:
			printf("  cmp rax, rdi\n");
			printf("  setl al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LE:
			printf("  cmp rax, rdi\n");
			printf("  setle al\n");
			printf("  movzb rax, al\n");
			break;
	}

	printf("  push rax\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません。\n");
		return 1;
	}

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize(user_input);
	Node *node = expr();

	// アセンブリの勢半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("\n");
	printf("main:\n");

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので、
	// それをRAXにロードして関数からの戻り値とする。
	printf("  pop rax\n");

	printf("  ret\n");
	return 0;
}
