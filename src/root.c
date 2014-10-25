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

static int xson_root_initialize(struct xson_element * e,
                                struct xson_lex_element * lex) {
    struct xson_value * val = xson_malloc(&e->ctx->pool,
                                          sizeof(struct xson_value));
    if (val == NULL) {
        return XSON_RESULT_OOM;
    }
    val->child = NULL;
    e->internal = val;

    return XSON_RESULT_SUCCESS;
}
static void xson_root_destroy(struct xson_element * ele) {
    struct xson_value * val = ele->internal;
    val->child->ops->destroy(val->child);
}

/*
* if root element holds a array, access it like "[n]".
* Otherwise root element holds a object element, access
* it as normal object element.
*/
static struct xson_element *
xson_root_get_child(struct xson_element * ele, const char * expr) {
    assert(ele != NULL);
    assert(ele->internal != NULL);
    struct xson_value   *val;
    struct xson_element *child, *ret;
    char                *buf;
    if (ele == NULL)
        return XSON_EXPR_NULL;
    
    val = ele->internal;

    if (val == NULL)
        return XSON_EXPR_NULL;
    
    child = xson_value_get_elt(val);

    if (child == NULL)
        return XSON_EXPR_NULL;

    /* handle the case that the epxression start with '[i].key1.key2...' */
    if (child->type == ELE_TYPE_ARRAY && expr[0] == '[') {
        buf = malloc(strlen(expr) + 2); /* 1 for ' ', 1 for '\0' */

        if (buf == NULL)
            return XSON_EXPR_OOM;

        buf[0] = ' ';
        strcpy(buf + 1, expr);

        ret = child->ops->get_child(child, buf);

        free(buf);

        return ret;
    }
    return child->ops->get_child(child, expr);
}

static int xson_root_add_child(struct xson_element * parent,
                               struct xson_element * child) {
    struct xson_value * val = (struct xson_value *)parent->internal;
    if (child->type != ELE_TYPE_OBJECT &&
       child->type != ELE_TYPE_ARRAY) {
        printf("Child type for root element must be one of: object, pair.\n");
        return XSON_RESULT_INVALID_JSON;
    }else if (val->child) {
        printf("Only one child can be added on root element\n");
        return XSON_RESULT_INVALID_JSON;
    }
    val->child = child;
    child->parent = parent;
    return XSON_RESULT_SUCCESS;
}
static void xson_root_print(struct xson_element * ele, int level, int indent,
                            int dont_pad_on_first_line) {
    struct xson_value * val = ele->internal;
    val->child->ops->print(val->child, level, indent, 0);
    XSON_PADDING_PRINT(level * indent, "\n");
}
struct xson_ele_operations root_ops =  {
    xson_root_initialize,
    xson_root_destroy,
    xson_root_get_child,
    xson_root_add_child,
    xson_root_print
};

struct xson_element* xson_value_get_elt(struct xson_value * val){
    assert(val != NULL);

    if (val == NULL)
        return NULL;

    return (struct xson_element *)val->child;
}