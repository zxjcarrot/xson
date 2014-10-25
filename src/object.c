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

static int xson_object_initialize(struct xson_element * e,
                                  struct xson_lex_element * lex) {
    struct xson_object * obj;
    
    if ((e->internal = xson_malloc(&e->ctx->pool,
                                   sizeof(struct xson_object))) == NULL) {
        return XSON_RESULT_OOM;
    }
    
    obj = (struct xson_object *)e->internal;
    
    if ((obj->pairs = malloc(sizeof(struct xson_element *) *
                             XSON_OBJECT_INIT_PAIRS_SIZE)) == NULL) {
        return XSON_RESULT_OOM;
    }
    if (xson_pair_ht_init(&obj->ht) == XSON_RESULT_OOM) {
        free(obj->pairs);
        return XSON_RESULT_OOM;
    }
    obj->idx = 0;
    obj->size = XSON_OBJECT_INIT_PAIRS_SIZE;
    return XSON_RESULT_SUCCESS;
}

static void xson_object_destroy(struct xson_element * ele) {
    int                 i;
    struct xson_object  *obj = ele->internal;

    for (i = 0; i < obj->idx; ++i) {
        obj->pairs[i]->ops->destroy(obj->pairs[i]);
    }
    xson_pair_ht_free(&obj->ht);
    free(obj->pairs);
}

static struct xson_element *
xson_object_get_child(struct xson_element * ele, const char * expr) {
    assert(ele != NULL);
    assert(ele->internal != NULL);
    struct xson_object  *obj;
    struct xson_element *array_elt;
    int                  is_array;
    char                *array_name;
    
    if (ele == NULL || ele->internal == NULL)
        return XSON_EXPR_NULL;
    obj = ele->internal;
    is_array = xson_is_array_expression(expr);

    if (is_array) {
        /* extract key from things like 'key[idx]' */

        array_name = malloc(strlen(expr));
        
        if (array_name == NULL)
            return XSON_EXPR_OOM;
        
        strcpy(array_name, expr);
        *strchr(array_name, '[') = '\0';
        
        array_elt = xson_object_get_pairval(obj, array_name);

        free(array_name);

        if (!XSON_GOOD_ELEMENT(array_elt))
            return array_elt;
        else if (array_elt->type != ELE_TYPE_ARRAY)
            return XSON_EXPR_TYPE_MISMATCH;
        else
            return array_elt->ops->get_child(array_elt, expr);
    } else {
        return xson_object_get_pairval(obj, expr);
    }
    
}

static int xson_object_add_child(struct xson_element * parent,
                                 struct xson_element * child) {
    int                 ret;
    struct xson_object  *obj = (struct xson_object *)parent->internal;
    if (child->type != ELE_TYPE_PAIR) {
        printf("Child type for object element must be pair.\n");
        return XSON_RESULT_INVALID_JSON;
    }
    
    ret = xson_pair_ht_insert_replace(&obj->ht,
                                      (struct xson_pair *)child->internal);
    if (ret != XSON_RESULT_SUCCESS) {
        assert(0);
        return ret;
    }
    if (obj->idx >= obj->size) {
        if (xson_buffer_grow((void **)&obj->pairs,
            &obj->size, obj->idx,
            sizeof(struct xson_element *)) == -1) {
            assert(0);
            return XSON_RESULT_OOM;
        }
    }
    obj->pairs[obj->idx++] = child;
    child->parent = parent;
    return XSON_RESULT_SUCCESS;;
}

static void xson_object_print(struct xson_element * ele, int level, int indent,
                              int dont_pad_on_first_line) {
    int                 i;
    struct xson_object  *obj = ele->internal;
    XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "{\n");
    for (i = 0; i < obj->idx; ++i) {
        if (i)XSON_PADDING_PRINT(0, ",\n");
        obj->pairs[i]->ops->print(obj->pairs[i], level + 1, indent, 0);
        if (i == obj->idx - 1)XSON_PADDING_PRINT(0, "\n");
    }
    XSON_PADDING_PRINT(level * indent, "}");
}

struct xson_ele_operations object_ops = {
    xson_object_initialize,
    xson_object_destroy,
    xson_object_get_child,
    xson_object_add_child,
    xson_object_print
};

/*
* Get the pair to which @key is mapped.
* Return: a pointer to that value element, NULL if the object
*         contains no mapping for @key.
*/
struct xson_pair*
xson_object_get_pair(struct xson_object * obj, const char * key) {
    assert(obj != NULL);
    assert(key != NULL);

    if (obj == NULL || key == NULL)
        return NULL;

    return xson_pair_ht_retrieve(&obj->ht, key);
}

/*
* Get the value element of the pair to which @key is mapped.
* Return: a pointer to that value element, NULL if the object
*         contains no mapping for @key.
*/
struct xson_element*
xson_object_get_pairval(struct xson_object * obj, const char * key) {
    struct xson_pair * pair = xson_object_get_pair(obj, key);

    if (pair == NULL)
        return NULL;

    return pair->value;
}

inline int xson_object_get_size(struct xson_object *obj){
    assert(obj != NULL);

    if(obj == NULL)
        return XSON_RESULT_ERROR;

    return obj->idx;
}
