#include <stdio.h>

#include "9cc.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません。\n");
		return 1;
	}

	// トークナイズしてパースする
	tokenize(argv[1]);
	Node *node = expr();

	// コード生成
	generate(node);
	return 0;
}
