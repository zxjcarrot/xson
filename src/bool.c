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

static int xson_bool_initialize(struct xson_element * e,
                                struct xson_lex_element * lex) {
    struct xson_bool *xbool;

    if ((e->internal = xson_malloc(&e->ctx->pool,
                                   sizeof(struct xson_bool))) == NULL) {
        return XSON_RESULT_OOM;
    }

    xbool = (struct xson_bool *)e->internal;

    if (strncmp(lex->start, "true", 4) == 0) {
        xbool->bool_val = 1;
    } else {
        xbool->bool_val = 0;
    }
    return XSON_RESULT_SUCCESS;
}

static void xson_bool_destroy(struct xson_element * ele) {
    ;
}

static struct xson_element *
xson_bool_get_child(struct xson_element * ele, const char * expr) {
    return XSON_EXPR_OP_NOTSUPPORTED;
}

static int xson_bool_add_child(struct xson_element * parent,
                               struct xson_element * child) {
    printf("Adding children to bool element is NOT supported.\n");
    return XSON_RESULT_INVALID_JSON;
}

static void xson_bool_print(struct xson_element * ele, int level, int indent,
                     int dont_pad_on_first_line) {
    struct xson_bool    *xbool = ele->internal;

    XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), 
                       "%s", xbool->bool_val ? "true" : "false");
}

struct xson_ele_operations bool_ops =  {
    xson_bool_initialize,
    xson_bool_destroy,
    xson_bool_get_child,
    xson_bool_add_child,
    xson_bool_print
};

int xson_bool_to_int(struct xson_bool * xbool, int *out){
    assert(xbool != NULL);

    if(xbool == NULL)
        return XSON_RESULT_ERROR;

    *out = xbool->bool_val;

    return XSON_RESULT_SUCCESS;
}