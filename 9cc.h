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
	// <=
	ND_LE,
	// Assign
	ND_ASSIGN,
	// Local variable
	ND_LVAR,
	// Number
	ND_NUM,
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	// ノードの型
	NodeKind kind;
	// 左辺
	Node *lhs;
	// 右辺
	Node *rhs;
	// kindがND_NUMの場合: 数値
	int val;
	// kindがND_LVARの場合: メモリ位置
	int offset;
};

typedef struct NodeList NodeList;

// Node列
struct NodeList {
	Node *node;
	NodeList *next;
};

// 入力文字列をトークナイズして内部に保持する。
void *tokenize(char *p);

// Parse program
NodeList *parse_program(void);

// 構文木からコードを生成する。
void generate(NodeList *nodes);

#endif
