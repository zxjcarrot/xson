#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <unistd.h>

#include <xson/parser.h>

#define LOOPS 2024*4024
char buf[4024000];

int main(int argc, char const *argv[]){
	int                  ret, n, idx = 0;
	struct xson_context  ctx;
	struct xson_element *root = NULL;
	//int                  bool_val = -1;

	freopen("test2.input", "r", stdin);
	freopen("result.txt", "w", stdout);

	while((n = read(fileno(stdin), buf + idx, sizeof(buf))) > 0){
		idx += n;
	}
	buf[idx] = 0;

	xson_init(&ctx, buf);

	ret = xson_parse(&ctx, &root);

	if(ret == XSON_RESULT_SUCCESS){
		printf("xson parser: success.\n");
		xson_print(&ctx, 4);
	}else if(ret == XSON_RESULT_INVALID_JSON){
		printf("xson parser: invalid json string.\n");
	}else{
		printf("xson parser: unknown error.\n");
	}
	
	//xson_get_bool_by_expr(root, "bool", &bool_val);

	//printf("bool: %d\n", bool_val);
	xson_destroy(&ctx);

	return 0;
}