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
#ifndef XSON_LEXICAL_H_
#define XSON_LEXICAL_H_

#include "common.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XSON_CTX_INIT_STK_LEN 32

typedef struct xson_context {
	char * str_buf;
	int str_len;

	struct xson_lex_element * stack;
	int stk_top;
	int stk_len;

	/* root of the json elements. */
	struct xson_element * root;
	struct xmpool_t pool;
}xson_context;

/*
* Initialize a context for parsing.
* Return: 0 on success, -1 on failure(out of memory).
* @ctx: the context being initialized.
* @str: the json string.
*/
int xson_init(struct xson_context * ctx, const char * str);

/*
* Do the real parsing for the ctx.
* Return: 0 on success, -1 on failure.
* @ctx: the context being parsed.
* @out: holds the root element if successfully parsed
*/
int xson_parse(struct xson_context * ctx, struct xson_element ** out);

/*
* Clean and free up the context.
* @ctx: the context being destroyed.
*/
void xson_destroy(struct xson_context * ctx);

/*
* Format the json string to tree-like structure.
*/
void xson_print(struct xson_context * ctx, int indent);

#ifdef __cplusplus
}
#endif
#endif