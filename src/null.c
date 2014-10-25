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

static int xson_null_initialize(struct xson_element * e,
                                struct xson_lex_element * lex) {
    return XSON_RESULT_SUCCESS;
}

static void xson_null_destroy(struct xson_element * ele) {
    ;
}

static struct xson_element *
xson_null_get_child(struct xson_element * ele, const char * expr) {
    return XSON_EXPR_NULL;
}

static int xson_null_add_child(struct xson_element * parent,
                               struct xson_element * child) {
    printf("Adding children to null element is NOT supported.\n");
    return XSON_RESULT_INVALID_JSON;
}

static void xson_null_print(struct xson_element * ele, int level, int indent,
                     int dont_pad_on_first_line) {
    XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "null");
}

struct xson_ele_operations null_ops =  {
    xson_null_initialize,
    xson_null_destroy,
    xson_null_get_child,
    xson_null_add_child,
    xson_null_print
};