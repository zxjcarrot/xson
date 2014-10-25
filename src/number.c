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
#include <math.h>
#include <errno.h>
#include <limits.h>

#include "xson/types.h"
#include "xson/parser.h"

static int xson_number_initialize(struct xson_element * e,
                                  struct xson_lex_element * lex) {
    struct xson_number * number;
    if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_number))) == NULL) {
        return XSON_RESULT_OOM;
    }
    number = (struct xson_number *)e->internal;
    number->start = lex->start;
    number->end = lex->end;
    return XSON_RESULT_SUCCESS;
}

static void xson_number_destroy(struct xson_element * ele) {
    ;
}

static struct xson_element *
xson_number_get_child(struct xson_element * ele, const char * expr) {
    return XSON_EXPR_OP_NOTSUPPORTED;
}

static int xson_number_add_child(struct xson_element * parent,
                                 struct xson_element * child) {
    printf("Adding children to number element is NOT supported.\n");
    return XSON_RESULT_INVALID_JSON;
}

static void xson_number_print(struct xson_element * ele, int level, int indent,
                              int dont_pad_on_first_line) {
    struct xson_number  *number = ele->internal;
    char                t = *(number->end + 1);
    
    *(number->end + 1) = 0;
    XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent),
                       "%s", number->start);
    *(number->end + 1) = t;
}

struct xson_ele_operations number_ops =  {
    xson_number_initialize,
    xson_number_destroy,
    xson_number_get_child,
    xson_number_add_child,
    xson_number_print
};


int xson_number_to_ullong(struct xson_number * number, unsigned long long *out) {
    unsigned long long t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtoull(number->start, NULL, 10);

    if (t == ULLONG_MAX && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}

int xson_number_to_llong(struct xson_number * number, long long *out){
    long long t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtoll(number->start, NULL, 10);

    if (t == LLONG_MAX && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}


int xson_number_to_ulong(struct xson_number * number, unsigned long int *out){
    unsigned long int t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtoul(number->start, NULL, 10);

    if (t == ULONG_MAX && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}

int xson_number_to_long(struct xson_number * number, long int *out){
    long int t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtol(number->start, NULL, 10);

    if (t == LONG_MAX && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}

int xson_number_to_uint(struct xson_number * number, unsigned int *out){
    unsigned long int t;
    int               res;

    res = xson_number_to_ulong(number, &t);

    if(res != XSON_RESULT_SUCCESS)
        return res;

    if(t > (unsigned long)UINT_MAX)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}

int xson_number_to_int(struct xson_number * number, int *out){
    long int t;
    int      res;

    res = xson_number_to_long(number, &t);

    if(res != XSON_RESULT_SUCCESS)
        return res;

    if(t > (long)INT_MAX || t < (long)INT_MIN)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}

int xson_number_to_ldouble(struct xson_number * number, long double *out){
    long double t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtold(number->start, NULL);

    if ((t == -HUGE_VALL || t == HUGE_VALL) && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}
int xson_number_to_double(struct xson_number * number, double *out){
    double t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtod(number->start, NULL);

    if ((t == -HUGE_VAL || t == HUGE_VAL) && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;

    return XSON_RESULT_SUCCESS;
}

int xson_number_to_float(struct xson_number * number, float *out){
    float t;

    assert(number != NULL);
    assert(out != NULL);

    if (number == NULL || out == NULL)
        return XSON_RESULT_ERROR;

    errno = 0;
    t = strtof(number->start, NULL);

    if ((t == -HUGE_VALF || t == HUGE_VALF) && errno == ERANGE)
        return XSON_RESULT_OOR;

    *out = t;
    
    return XSON_RESULT_SUCCESS;
}
