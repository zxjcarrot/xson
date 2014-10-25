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

static int xson_array_initialize(struct xson_element * e, struct xson_lex_element * lex) {
    struct xson_array *array;
    
    if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_array))) == NULL) {
        return XSON_RESULT_OOM;
    }
    
    array = (struct xson_array *)e->internal;
    
    if ((array->array = malloc(sizeof(struct xson_element *) * XSON_OBJECT_INIT_ARRAY_SIZE)) == NULL) {
        return XSON_RESULT_OOM;
    }
    array->idx = 0;
    array->size = XSON_OBJECT_INIT_ARRAY_SIZE;
    return XSON_RESULT_SUCCESS;
}

static void xson_array_destroy(struct xson_element * ele) {
    int                 i;
    struct xson_array   *array = ele->internal;
    for (i = 0; i < array->idx; ++i) {
        array->array[i]->ops->destroy(array->array[i]);
    }
    free(array->array);
}

static struct xson_element *
xson_array_get_child(struct xson_element * ele, const char * expr) {
    assert(ele != NULL);
    assert(ele->internal != NULL);
    struct xson_array   *array;
    int                  is_array, idx;

    if(ele == NULL || ele->internal == NULL)
        return XSON_EXPR_NULL;
    array = ele->internal;

    is_array = xson_is_array_expression(expr);

    if (!is_array)
        return XSON_EXPR_INVALID_EXPR;

    /* discard key */
    sscanf(expr, "%*[^[][%d]", &idx);

    if (idx < 0 || idx >= array->idx)
        return XSON_EXPR_INDEX_OOR;

    return xson_array_get_elt(array, idx);
}

static int xson_array_add_child(struct xson_element * parent,
                                struct xson_element * child) {
    struct xson_array *array = (struct xson_array *)parent->internal;
    if (child->type != ELE_TYPE_NUMBER &&
       child->type != ELE_TYPE_STRING &&
       child->type != ELE_TYPE_OBJECT &&
       child->type != ELE_TYPE_ARRAY) {
        printf("Child type for array element must be one of:"
               "number, string, object, array.\n");
        return XSON_RESULT_INVALID_JSON;
    }
    
    if (array->idx >= array->size && 
        xson_buffer_grow((void **)&array->array,
                         &array->size, array->idx,
                         sizeof(struct xson_element *)) == -1) {
        return XSON_RESULT_OOM;
    }
    array->array[array->idx++] = child;
    child->parent = parent;
    return XSON_RESULT_SUCCESS;
}

static void xson_array_print(struct xson_element * ele, int level, int indent,
                             int dont_pad_on_first_line) {
    int                 i;
    struct xson_array   *array = ele->internal;
    XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "[\n");
    for (i = 0; i < array->idx; ++i) {
        if (i)XSON_PADDING_PRINT(0, ",\n");
        assert(array->array[i]);
        assert(array->array[i]->ops);
        array->array[i]->ops->print(array->array[i], level + 1, indent, 0);
        if (i == array->idx - 1)XSON_PADDING_PRINT(0, "\n");
    }
    XSON_PADDING_PRINT(level * indent, "]");
}

struct xson_ele_operations array_ops =  {
    xson_array_initialize,
    xson_array_destroy,
    xson_array_get_child,
    xson_array_add_child,
    xson_array_print
};

struct xson_element*
xson_array_get_elt(struct xson_array * array, int idx) {
    assert(array != NULL);

    if(array == NULL || idx >= array->idx || idx < 0)
        return NULL;

    return array->array[idx];
}

inline int xson_array_get_size(struct xson_array *array) {
    assert(array != NULL);

    if(array == NULL)
        return XSON_RESULT_ERROR;

    return array->idx;
}