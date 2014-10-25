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
#include "xson/common.h"

int xson_buffer_grow(void ** buffer, int *old_len, int len, int ele_size) {
    void    *new_buffer;
    int     new_len = *old_len;
    

    while(new_len <= len) {
        new_len <<= 1;
    }
    //printf("Expanding old buffer %p with size %d to new size %d.\n", *buffer, *old_len * ele_size, new_len * ele_size);
    if((new_buffer = realloc(*buffer, new_len * ele_size)) == NULL) {
        return XSON_RESULT_OOM;
    }
    //printf("new buffer address: %p.\n", new_buffer);
    *buffer = new_buffer;
    *old_len = new_len;

    return XSON_RESULT_SUCCESS;
}

int xson_is_number_start(char ch) {
    return ch == '-' || (ch >= '0' && ch <= '9');
}

int xson_is_blanks(char ch) {
    return ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ';
}

inline void * xson_malloc(struct xmpool_t * pool, size_t size) {
    if(size > pool->chunk_size) {
        assert(0);
        return NULL;
    }
    return xmpool_alloc(pool, size);
}