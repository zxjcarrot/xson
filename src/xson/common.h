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
#ifndef XSON_COMMON_H_
#define XSON_COMMON_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "xmalloc.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum xson_lex_state {
	LEX_STATE_EMPTY, 			/* empty state  */
	LEX_STATE_LEFT_BRACE, 		/*  {  */
	LEX_STATE_RIGHT_BRACE,		/* }  */
	LEX_STATE_LEFT_SQBRACKT, 	/* [  */
	LEX_STATE_RIGHT_SQBRACKT, 	/* ]  */
	LEX_STATE_LEFT_DQUOTE,		/* "  */
	LEX_STATE_RIGHT_DQUOTE,		/* "  */
	LEX_STATE_COMMA,			/* ,  */
	LEX_STATE_COLON,			/* :  */
	LEX_STATE_OBJECT,			/* json object  {...} */
	LEX_STATE_ARRAY,			/* json array  [...] */
	LEX_STATE_STRING, 			/* string "..." */
	LEX_STATE_NUMBER,			/* number value */
	LEX_STATE_PAIR				/* pair string:value */
}xson_lex_state;

typedef enum xson_ele_type {
	ELE_TYPE_ROOT,
	ELE_TYPE_VALUE,
	ELE_TYPE_OBJECT,
	ELE_TYPE_ARRAY,
	ELE_TYPE_STRING,
	ELE_TYPE_NUMBER,
	ELE_TYPE_PAIR
}xson_ele_type;

struct xson_element;

typedef struct xson_lex_element {
	xson_lex_state state;
	/*
	* @start, @end: pinpoint(inclusive) the contents of this element inside the json string.
	*/
	char *start, *end;
	struct xson_element * element;
}xson_lex_element;


/*
* Expand the buffer to given size.
* After succeed expanding the buffer,
* *@buffer holds the new buffer address,
* *@old_len holds the new buffer size.
* Return value: XSON_RESULT_SUCCESS on success, XSON_RESULT_OOM if failed to reallocate memory for the buffer.
* @buffer: the buffer address to expand and holds the result buffer address.
* @old_len: the size of the old buffer and holds the new buffer size.
* @len: the expected number of elements in the buffer.
* @ele_size: the size of each element in the buffer.
*/
int xson_buffer_grow(void ** buffer, int *old_len, int len, int ele_size);

/*
* Allocate @size memory from the memory pool.
* @size must be less or equal to @pool->chunk_size which is set during initalization of the memory pool;
* Return: the memory address newly allocated, NULL if out of memory.
* @pool: the memory pool from which the memory is allocated.
* @size: the desired size.
*/
void * xson_malloc(struct xmpool_t * pool, size_t size);
/* helper functions */ 
int xson_is_number_start(char ch);
int xson_is_blanks(char ch);

#define XSON_RESULT_INVALID_EXPR -7		/* invalid expression for accessing child element */
#define XSON_RESULT_OOG -6				/* result out of range */
#define XSON_RESULT_TYPE_MISMATCH -5	/* result type mismatch */
#define XSON_RESULT_OP_NOTSUPPORTED -4  /* operation not supported by a specific type */
#define XSON_RESULT_ERROR -3			/* some errors occured */
#define XSON_RESULT_OOM -2 				/* out of memory */
#define XSON_RESULT_INVALID_JSON -1		/* the input json string is invalid */
#define XSON_RESULT_SUCCESS 0			/* success */

#define XSON_PADDING_PRINT(N, format, ...)do {\
	int n = (N);\
	while(n--)putchar(' ');\
	printf(format, ##__VA_ARGS__);\
}while(0)

#ifdef __cplusplus
}
#endif
#endif