##Overview
**xson** is a json parser written in C.
#Installation
    git clone https://github.com/zxjcarrot/xson
    cd xson/src
    make all
    sudo make install

#Usage
you can use DOMTree-like expression to access a json field, i.e. "foo.bar.array[1].element".

    struct xson_context  ctx;
	struct xson_element *root = NULL;

    xson_init(&ctx, json_string);
    
    xson_parse(&ctx, &root);

    xson_get_int_by_expr(root, "foo.bar.array[1].element", &val);
    xson_destroy(&ctx);

Don't forget to call xson_destroy after done with it, since xson used a memory pool to manage memory allocations. 

In addition, xson can be easily used as a json prettifier, simply call xson_print which print out the whole json string in a tree-like form to the stdout.
	
	struct xson_context  ctx;
	int                  ret;

    xson_init(&ctx, json_string);
    
    ret = xson_parse(&ctx, &root);

    if (ret == XSON_RESULT_SUCCESS)
    	xson_print(&ctx, 4);/* 4 for indentation */

    xson_destroy(&ctx);
#Example
[Here](https://github.com/zxjcarrot/xson/tree/master/example) is a small example I've written.
