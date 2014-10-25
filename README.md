##Overview
**xson** is a json parser written in C.
#Installation
    git clone https://github.com/zxjcarrot/xson
    cd xson
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

#Example
[Here](https://github.com/zxjcarrot/xson/tree/master/example) is a small example I've written.