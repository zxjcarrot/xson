/*
* Copyright (c) 2014 Xinjing Cho
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERR
*/

#include "xson/types.h"
#include "xson/parser.h"

static int xson_pair_initialize(struct xson_element * e,
                                struct xson_lex_element * lex) {
    struct xson_pair *pair;
    if ((e->internal = xson_malloc(&e->ctx->pool,
                                   sizeof(struct xson_pair))) == NULL) {
        return XSON_RESULT_OOM;
    }
    pair = (struct xson_pair *)e->internal;
    memset(pair, 0, sizeof(struct xson_pair));
    return XSON_RESULT_SUCCESS;
}

static void xson_pair_destroy(struct xson_element * ele) {
    struct xson_pair *pair = ele->internal;
    pair->key->ops->destroy(pair->key);
    pair->value->ops->destroy(pair->value);
}

static struct xson_element *
xson_pair_get_child(struct xson_element * ele, const char * expr) {
    return XSON_EXPR_OP_NOTSUPPORTED;
}

static int xson_pair_add_child(struct xson_element * parent,
                               struct xson_element * child) {
    printf("Adding children to pair element is NOT supported.\n");
    return XSON_RESULT_INVALID_JSON;
}

static void xson_pair_print(struct xson_element * ele, int level, int indent,
                            int dont_pad_on_first_line) {
    struct xson_pair *pair = ele->internal;
    pair->key->ops->print(pair->key, level, indent, 0);
    XSON_PADDING_PRINT(0, ":");
    pair->value->ops->print(pair->value, level, indent, 1);
}
struct xson_ele_operations pair_ops =  {
    xson_pair_initialize,
    xson_pair_destroy,
    xson_pair_get_child,
    xson_pair_add_child,
    xson_pair_print
};


struct xson_string* xson_pair_get_key(struct xson_pair * pair) {
    assert(pair != NULL);
    assert(pair->key != NULL);

    if (pair == NULL || pair->key == NULL)
        return NULL;

    return (struct xson_string *)pair->key->internal;
}

struct xson_element* xson_pair_get_value(struct xson_pair * pair) {
    assert(pair != NULL);
    assert(pair->value != NULL);

    if (pair == NULL)
        return NULL;

    return pair->value;
}