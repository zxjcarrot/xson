#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <unistd.h>

#include <xson/parser.h>

#define LOOPS 2024*4024
char buf[4024000];

int main(int argc, char const *argv[]){
	int                  ret, n, idx = 0, i;
	struct xson_context  ctx;
	struct xson_element *root = NULL;
	int                  bool_val = -1;
	int                  term;
	int                  array_elt;
	char                 ip[30] = {0}, expr_buf[30];
	int                  array_size;
	double               double_val;

	freopen("test.input", "r", stdin);

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
	

	ret = xson_get_bool_by_expr(root, "bool", &bool_val);
	if (ret == XSON_RESULT_SUCCESS)
		printf("bool: %d\n", bool_val);
	else
		printf("bool: error occured");


	ret = xson_get_bool_by_expr(root, "term", &term);
	if (ret == XSON_RESULT_SUCCESS)
		printf("term: %d\n", bool_val);
	else
		printf("term: error occured");
	

	ret = xson_get_string_by_expr(root, "candidate_id.ip", ip, sizeof(ip));
	if (ret == XSON_RESULT_SUCCESS)
		printf("ip: %s\n", ip);
	else
		printf("term: error occured");


	ret = xson_get_int_by_expr(root, "candidate_id.array[1]", &array_elt);
	if (ret == XSON_RESULT_SUCCESS)
		printf("candidate_id.array[1]: %d\n", array_elt);
	else
		printf("candidate_id.array[1]: error occured");


	ret = xson_get_double_by_expr(root, "double", &double_val);
	if (ret == XSON_RESULT_SUCCESS)
		printf("double: %lf\n", double_val);
	else
		printf("double: error occured");


	array_size = xson_get_arraysize_by_expr(root, "array");
	if (array_size >= XSON_RESULT_SUCCESS)
		printf("array_size: %d\n", array_size);
	else
		printf("array_size: error occured");


	for (i = 0; i < array_size; ++i) {
		sprintf(expr_buf, "array[%d]", i);
		ret = xson_get_int_by_expr(root, expr_buf, &array_elt);
		if (ret == XSON_RESULT_SUCCESS)
			printf("%s: %d\n", expr_buf, array_elt);
		else
			printf("%s: error occured", expr_buf);
	}

	xson_destroy(&ctx);

	return 0;
}