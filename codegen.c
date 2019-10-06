#include <stdio.h>

#include "9cc.h"

void gen_lval(Node *node) {
	if (node->kind != ND_LVAR)
		fprintf(stderr, "代入の左辺値が変数ではありません。\n");

	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", node->offset);
	printf("  mov rax, rbp\n");
}

void gen(Node *node) {
	switch (node->kind) {
		case ND_NUM:
			printf("  push %d\n", node->val);
			return;
		case ND_LVAR:
			gen_lval(node);
			printf("  pop rax\n");
			printf("  mov rax, [rax]\n");
			printf("  push rax\n");
			return;
		case ND_ASSIGN:
			gen_lval(node->lhs);
			gen(node->rhs);

			printf("  pop rdi\n");
			printf("  pop rax\n");
			printf("  mov [rax], rdi\n");
			printf("  push rdi\n");
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

void generate(NodeList *nodes) {
	// アセンブリの勢半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("\n");
	printf("main:\n");

	// 変数26個(固定)分の領域を確保
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, 208\n");

	// 抽象構文木を下りながらコード生成
	while (nodes) {
		Node *node = nodes->node;
		gen(node);

		// スタックトップに式全体の値が残っているはずなので、
		// それをRAXにロードして関数からの戻り値とする。
		printf("  pop rax\n");
		nodes = nodes->next;
	}


	// 最後の式の結果がRAXに残っているのでそれを戻り値に設定
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
}
