#ifndef INCLUDE_9CC_HEADER
#define INCLUDE_9CC_HEADER

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

typedef struct Node Node;

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

// 入力文字列をトークナイズして内部に保持する。
void *tokenize(char *p);

// Parse expr
Node *expr(void);

// 構文木からコードを生成する。
void generate(Node *node);

#endif
