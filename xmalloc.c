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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"
static struct xmpool_chunk_t * xmpool_chunk_alloc_init(size_t chunk_size){
	struct xmpool_chunk_t * chunk = malloc(sizeof(struct xmpool_chunk_t));

	if(chunk == NULL){
		assert(0);
		return NULL;
	}
	
	memset(chunk, 0, sizeof(struct xmpool_chunk_t));
	
	chunk->smem = malloc(chunk_size);
	if(chunk->smem == NULL){
		assert(0);
		free(chunk);
		return NULL;
	}
	assert(chunk_size > 0);
	chunk->size = chunk_size;
	chunk->first = chunk->smem;

	return chunk;
}

int32_t xmpool_init(struct xmpool_t * pool, size_t chunk_size){
	struct xmpool_chunk_t * chunk;
	assert(pool != NULL);
	INIT_LIST_HEAD(&pool->chunk_list);
	pool->chunk_size = chunk_size;
	pool->chunks = XM_INIT_CHUNKS;
	chunk = xmpool_chunk_alloc_init(chunk_size);
	if(chunk == NULL)return -1;
	list_add(&chunk->chunk_link, &pool->chunk_list);
	return 0;
}



inline static void * xmpool_chunk_do_alloc(struct list_head * head, struct xmpool_chunk_t * chunk, size_t size){
	char * res = chunk->first;
	chunk->first += size;
	if(res == NULL){
		assert(res);
	}
	/* move the chunk to the head of the list. */
	list_del(&chunk->chunk_link);
	list_add(&chunk->chunk_link, head);
	
	
	return res;
}

#define XM_TO_NEXT_POWER_OF_2(n) ({	\
	int32_t highest_one = 0;		\
	int32_t tmp = (n);				\
	int32_t i = 0;					\
	while(tmp){						\
		if(tmp & 1){				\
			highest_one = i;		\
		}							\
		++i;						\
		tmp >>= 1;					\
	}								\
	(1 << (highest_one + 1));		\
})

static void * xmpool_chunklist_grow_alloc(struct xmpool_t * pool, size_t size){
	assert(size > 0);
	struct xmpool_chunk_t * first_chunk = NULL, *chunk = NULL;
	int32_t chunks_to_add = 0, i = 0;
	chunks_to_add = XM_TO_NEXT_POWER_OF_2(pool->chunks) - pool->chunks;
	assert(chunks_to_add);
	for(; i < chunks_to_add; ++i){
		chunk = xmpool_chunk_alloc_init(pool->chunk_size);
		if(chunk == NULL)break;
		list_add(&chunk->chunk_link, &pool->chunk_list);
		first_chunk = chunk;
	}
	pool->chunks += i;
	if(first_chunk == NULL){
		assert(0);
		return NULL;
	}
	return xmpool_chunk_do_alloc(&pool->chunk_list, first_chunk, size);
}

/*
* Allocate
*/
void * xmpool_alloc_large(struct xmpool_t * pool, size_t size){
	assert(size > 0);
	struct list_head * p, * head = &pool->chunk_list;
	struct xmpool_chunk_t * chunk;
	assert(pool != NULL);

	if(size > pool->chunk_size){
		return 
	}

	size = XM_ALIGN(size, XM_ALIGNMENT);

	list_for_each(p, head){
		chunk = list_entry(p, struct xmpool_chunk_t, chunk_link);
		if(XM_CHUNK_FREE_SIZE(chunk) >= size){
			return xmpool_chunk_do_alloc(head, chunk, size);
		}
	}

	return xmpool_chunklist_grow_alloc(pool, size);
}

#define XM_CHUNK_FREE_SIZE(c) ((c)->size - ((size_t)((c)->first - (c)->smem)))

void * xmpool_alloc(struct xmpool_t * pool, size_t size){
	assert(size > 0);
	struct list_head * p, * head = &pool->chunk_list;
	struct xmpool_chunk_t * chunk;
	assert(pool != NULL);

	if(size > pool->chunk_size){
		return 
	}

	size = XM_ALIGN(size, XM_ALIGNMENT);

	list_for_each(p, head){
		chunk = list_entry(p, struct xmpool_chunk_t, chunk_link);
		if(XM_CHUNK_FREE_SIZE(chunk) >= size){
			return xmpool_chunk_do_alloc(head, chunk, size);
		}
	}

	return xmpool_chunklist_grow_alloc(pool, size);
}

void xmpool_destroy(struct xmpool_t * pool){
	assert(pool != NULL);
	struct list_head * p, *q;
	struct xmpool_chunk_t * chunk;
	
	for(p = pool->chunk_list.next; p != &pool->chunk_list;p = q){
		q = p->next;
		chunk = list_entry(p, struct xmpool_chunk_t, chunk_link);
		free(chunk->smem);
		free(chunk);
	}
	pool->chunks = 0;
}