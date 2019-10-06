#include <stdio.h>

#include "9cc.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません。\n");
		return 1;
	}

	// トークナイズしてパースする
	tokenize(argv[1]);
	NodeList *nodes = parse_program();

	// コード生成
	generate(nodes);
	return 0;
}
