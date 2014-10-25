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

/* definitions of some utility functions in types.h */
#include <assert.h>

#include "xson/types.h"
#include "xson/parser.h"

int xson_is_array_expression(const char * expr) {
    int         idx, ret;
    static char trailing[1];

    trailing[0] = 0;

    ret = sscanf(expr, "%*[^[][%d]%1s", &idx, trailing);

    return ret == 1;
}

struct xson_string * xson_elt_to_string(struct xson_element * elt) {
    assert(elt != NULL);
    assert(elt->internal != NULL);

    if (elt == NULL || elt->type != ELE_TYPE_STRING) 
        return NULL;

    return (struct xson_string *)elt->internal;
}

struct xson_number * xson_elt_to_number(struct xson_element * elt) {
    assert(elt != NULL);
    assert(elt->internal != NULL);
    
    if (elt == NULL || elt->type != ELE_TYPE_NUMBER) 
        return NULL;

    return (struct xson_number *)elt->internal;
}

struct xson_value * xson_elt_to_value(struct xson_element * elt) {
    assert(elt != NULL);
    assert(elt->internal != NULL);

    if (elt == NULL ||
        (elt->type != ELE_TYPE_VALUE && elt->type != ELE_TYPE_ROOT))
        return NULL;

    return (struct xson_value *)elt->internal;
}

struct xson_array * xson_elt_to_array(struct xson_element * elt) {
    assert(elt != NULL);
    assert(elt->internal != NULL);

    if (elt == NULL || elt->type != ELE_TYPE_ARRAY) 
        return NULL;

    return (struct xson_array *)elt->internal;
}

struct xson_object * xson_elt_to_object(struct xson_element * elt) {
    assert(elt != NULL);
    assert(elt->internal != NULL);

    if (elt == NULL || elt->type != ELE_TYPE_OBJECT)
        return NULL;

    return (struct xson_object *)elt->internal;
}

struct xson_pair * xson_elt_to_pair(struct xson_element * elt) {
    assert(elt != NULL);
    assert(elt->internal != NULL);

    if (elt == NULL || elt->type != ELE_TYPE_PAIR)
        return NULL;

    return (struct xson_pair *)elt->internal;
}


struct xson_element *
xson_get_by_expr(struct xson_element * elt, const char * key) {
    assert(elt != NULL);
    assert(key != NULL);
    char *p;
    char *buf;

    if (elt == NULL || key == NULL)
        return XSON_EXPR_NULL;

    buf = malloc(strlen(key) + 1);

    if (buf == NULL)
        return XSON_EXPR_OOM;

    strcpy(buf, key);

    p = strtok(buf, ".");

    while (p) {
        elt = elt->ops->get_child(elt, p);
        if (!XSON_GOOD_ELEMENT(elt))
            break;
        p = strtok(NULL, ".");
    }

    free(buf);
    
    return elt;
}

static int xson_convert_expr_res_to_res(struct xson_element * elt) {
    if (elt == XSON_EXPR_NULL)
        return XSON_RESULT_ERROR;
    else if(elt == XSON_EXPR_OOM)
        return XSON_RESULT_OOM;
    else if(elt == XSON_EXPR_INDEX_OOR)
        return XSON_RESULT_OOR;
    else if(elt == XSON_EXPR_KEY_NOT_EXIST)
        return XSON_RESULT_KEY_NOT_EXIST;
    else if(elt == XSON_EXPR_OP_NOTSUPPORTED)
        return XSON_RESULT_OP_NOTSUPPORTED;
    else if(elt == XSON_EXPR_INVALID_EXPR)
        return XSON_RESULT_INVALID_EXPR;

    return XSON_RESULT_SUCCESS;
}

int xson_get_ull_by_expr(struct xson_element * elt, const char * expr,
                         unsigned long long *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_ullong(number, out);
}

int xson_get_llong_by_expr(struct xson_element * elt, const char * expr,
                        long long *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_llong(number, out);
}

int xson_get_ulong_by_expr(struct xson_element * elt, const char * expr,
                           unsigned long int *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_ulong(number, out);
}

int xson_get_long_by_expr(struct xson_element * elt, const char * expr,
                          long int *out) {
        int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_long(number, out);
}

int xson_get_uint_by_expr(struct xson_element * elt, const char * expr, 
                          unsigned int *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_uint(number, out);
}

int xson_get_int_by_expr(struct xson_element * elt, const char * expr,
                         int *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_int(number, out);
}

int xson_get_bool_by_expr(struct xson_element * elt, const char * expr,
                         int *out) {
    int                  rc;
    struct xson_bool * xbool;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    xbool = elt->internal;

    return xson_bool_to_int(xbool, out);
}
int xson_get_double_by_expr(struct xson_element * elt, const char * expr,
                            double *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_double(number, out);
}

int xson_get_float_expr(struct xson_element * elt, const char * expr,
                        float *out) {
    int                  rc;
    struct xson_number * number;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    number = elt->internal;

    return xson_number_to_float(number, out);
}


int xson_get_string_by_expr(struct xson_element * elt, const char * expr,
                            char * buf, size_t size) {
    int                  rc;
    struct xson_string * string;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    string = elt->internal;

    return xson_string_to_buf(string, buf, size);
}

struct xson_array * xson_get_array_by_expr(struct xson_element * elt,
                                           const char * expr) {
    int rc;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return NULL;

    if (elt->type != ELE_TYPE_ARRAY)
        return NULL;

    return (struct xson_array *)elt->internal;
}

struct xson_object * xson_get_object_by_expr(struct xson_element * elt,
                                             const char * expr) {
    int rc;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return NULL;

    if (elt->type != ELE_TYPE_OBJECT)
        return NULL;

    return (struct xson_object *)elt->internal;
}

int xson_get_arraysize_by_expr(struct xson_element * elt,
                               const char * expr) {
    int rc;

    elt = xson_get_by_expr(elt, expr);

    if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
        return rc;

    if (elt->type != ELE_TYPE_ARRAY)
        return XSON_RESULT_TYPE_MISMATCH;

    return xson_array_get_size((struct xson_array *)elt->internal);
}