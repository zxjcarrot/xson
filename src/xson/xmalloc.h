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
#ifndef XSON_XMALLOC_H_
#define XSON_XMALLOC_H_
#include <stddef.h>
#include <stdlib.h>

#include "list.h"

/**
* A simple memory pool implementation for small object.
*/
#ifdef __cplusplus
extern "C" {
#endif

#define XM_ALIGNMENT 8
#define XM_ALIGN(a, b) (((a) + (b - 1)) & ~(b - 1))
#define XM_NR_OF_LIST 16
#define XM_CHUNK_SIZE 4096

typedef struct xmpool_chunk_t {
    struct list_head chunk_link;
    /* starting address of this memory chunk */
    char * smem;
    /* size of the chunk in bytes */
    size_t size;
    /* first unsed byte */
    char * first;
}xmpool_chunk_t;

typedef struct xmpool_large_chunk_t {
    void                        *data;
    struct xmpool_large_chunk_t *next;
}xmpool_large_chunk_t;

#define XM_INIT_CHUNKS 4

typedef struct xmpool_t {
    struct list_head chunk_list;
    size_t chunks;
    size_t chunk_size;
}xmpool_t;

/*
* Intitalize a memory pool.
* Return: 0 on success, -1 on failure.
* @pool: the memory pool being initialized
*/
int xmpool_init(struct xmpool_t * pool, size_t chunk_size);

/*
* Allocates @size bytes from the pool.
* The @size will be rounded up to the nearest multiple of 8.
* Return: memory address on success, NULL on failure.
* @pool: the memory pool from which the memory is allocated.
* @size: the requested size of memory.
*/
void * xmpool_alloc(struct xmpool_t * pool, size_t size);


/*
* Frees up the memory occupied by the memory pool.
* @pool: the memory pool being destroyed.
*/
void xmpool_destroy(struct xmpool_t * pool);
#ifdef __cplusplus
}
#endif
#endif
