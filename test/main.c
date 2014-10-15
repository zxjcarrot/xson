#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "parser.h"
#include "xmalloc.h"

#define LOOPS 2024*4024
char buf[4024000];

int main(int argc, char const *argv[]){

	int ret, n, idx = 0;
	struct xson_context ctx;
	freopen("test.input", "r", stdin);
	freopen("result.txt", "w", stdout);
	while((n = read(fileno(stdin), buf + idx, sizeof(buf))) > 0){
		idx += n;
	}
	buf[idx] = 0;
	xson_init(&ctx, buf);
	ret = xson_parse(&ctx);
	if(ret == XSON_RESULT_SUCCESS){
		printf("xson parser: success.\n");
		xson_print(&ctx, 4);
	}else if(ret == XSON_RESULT_INVALID_JSON){
		printf("xson parser: invalid json string.\n");
	}else{
		printf("xson parser: unknown error.\n");
	}
	xson_destroy(&ctx);

	return 0;
}