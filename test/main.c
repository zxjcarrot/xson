#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <unistd.h>

#include <xson/parser.h>

#define LOOPS 2024*4024
char buf[4024000];

int main(int argc, char const *argv[]){
	int ret, n, idx = 0;
	struct xson_context ctx;
	struct xson_element *root = NULL, *elt = NULL;
	struct xson_value   *val = NULL;
	struct xson_object  *obj = NULL, *ci_obj = NULL;
	struct xson_string  *ip_s = NULL;
	char 				ip_buf[1024] = {0};
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
	val = xson_elt_to_value(root);
	elt = xson_value_get_elt(val);
	obj = xson_elt_to_object(elt);
	ci_obj = xson_elt_to_object(xson_object_get_pairval(obj, "candidate_id"));
	ip_s = xson_elt_to_string(xson_object_get_pairval(ci_obj, "ip"));
	xson_string_to_buf(ip_s, ip_buf, 1024);
	printf("%s", ip_buf);

	xson_destroy(&ctx);

	return 0;
}