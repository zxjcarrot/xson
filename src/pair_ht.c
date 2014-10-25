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
#include <stdlib.h>
#include <string.h>

#include "xson/list.h"
#include "xson/types.h"
#include "xson/common.h"
#include "xson/pair_ht.h"

static const int xson_pair_ht_primes[] =  {
            11, 17, 23, 29, 37, 47, 
            53, 97, 193, 389, 769, 1543, 3079, 6151,
            12289, 24593, 49157, 98317, 196613, 393241,
            786433, 1572869, 3145739, 3145739, 12582917,
            25165843, 50331653, 100663319, 201326611,
            402653189, 805306457, 1610612741
            };
static const int xson_pair_ht_nprimes = sizeof(xson_pair_ht_primes) / sizeof(int);

static int xson_pair_ht_cmp(struct xson_pair * p1, const char * key2) {
    int                 ret;
    struct xson_string  *string = p1->key->internal;
    char                *key1 = string->start;
    int                 len = string->end - string->start + 1;

    ret = strncmp(key1, key2, len);

    return ret;
}

inline unsigned xson_pair_ht_hash_by_pair(struct xson_pair * p) {
    char                *cp, *end;
    unsigned            hash = 0;
    struct xson_string  *string = p->key->internal;

    for (cp = string->start, end = string->end + 1;
        cp < end;
        ++cp) {
        hash = hash * 131 + *cp;
    }

    return hash; 
}

inline unsigned xson_pair_ht_hash_by_key(const char * key) {
    char        ch;
    unsigned    hash = 0;

    while ((ch = *key++)) {
        hash = hash * 131 + ch;
    }

    return hash; 
}
/*
* Expand the size of the hash table to @size.
* @ht: the hash table to expand
* @size: the size we expand to
*/
static int xson_pair_ht_expand(struct xson_pair_ht * ht, int size) {
    int                 new_len, new_idx, i;
    struct xson_pair    *entry;
    struct list_head    *new_table, *p, *q, *head;
    
    unsigned h;
    new_len = ht->len;
    new_idx = ht->p_index;
    while ((new_len * LOAD_FACTOR) < size && new_idx < xson_pair_ht_nprimes) {
        new_len = xson_pair_ht_primes[++new_idx];
    }

    if ((new_table = malloc(new_len * sizeof(struct list_head))) == NULL) {
        printf("xson parser: failed to malloc.");
        return XSON_RESULT_OOM;
    }

    for (i = 0; i < new_len; ++i) {
        INIT_LIST_HEAD(&new_table[i]);
    }

    /*
    * Rehash and move all xson_pair to new_table.
    */
    for (i = 0; i < ht->len; ++i) {
        head = &(ht->table[i]);
        if (!list_empty(head)) {
            p = head->next;
            while (p != head) {
                q = p->next;
                entry = list_entry(p, struct xson_pair, hash_link);
                list_del(p);
                h = xson_pair_ht_hash_by_pair(entry) % new_len;
                list_add_tail(&entry->hash_link, &new_table[h]);
                p = q;
            }
        }
    }

    free(ht->table);

    ht->p_index = new_idx;
    ht->table = new_table;
    ht->len = new_len;

    return XSON_RESULT_SUCCESS;
}

inline int xson_pair_ht_init(struct xson_pair_ht * ht) {
    int i, idx;

    idx = 0;
    ht->p_index = 0;
    ht->n_entries = 0;
    ht->len = xson_pair_ht_primes[idx];

    ht->table = malloc(ht->len * sizeof(struct list_head));
    if (ht->table == NULL) {
        return XSON_RESULT_OOM;
    }
    for (i = 0; i < ht->len; ++i) {
        INIT_LIST_HEAD(&ht->table[i]);
    }
    return XSON_RESULT_SUCCESS;
}

inline int
xson_pair_ht_insert(struct xson_pair_ht * ht, struct xson_pair * new) {
    unsigned h;

    if (new->hash_link.prev || new->hash_link.next) {
        /*
        * This xson_pair is already in the hash table.
        * Assume every xson_pair only can be in one reactor.
        */
        return XSON_RESULT_ERROR;
    }

    /* expand the hash table if nessesary */
    if (ht->n_entries >= (ht->len * LOAD_FACTOR) &&
        xson_pair_ht_expand(ht, ht->n_entries + 1) == XSON_RESULT_OOM)
        return XSON_RESULT_OOM;

    h = xson_pair_ht_hash_by_pair(new) % ht->len;
    list_add_tail(&new->hash_link, &ht->table[h]);
    ++(ht->n_entries);
    return XSON_RESULT_SUCCESS;
}

inline int
xson_pair_ht_insert_replace(struct xson_pair_ht * ht, struct xson_pair * new) {
    unsigned h;

    if (new->hash_link.prev || new->hash_link.next) {
        /*
        * This xson_pair is not in the hash table.
        * Assume every xson_pair only can be in one reactor.
        */
        return XSON_RESULT_ERROR;
    }

    /* expand the hash table if nessesary */
    if (ht->n_entries >= (ht->len * LOAD_FACTOR) &&
        xson_pair_ht_expand(ht, ht->n_entries + 1) == XSON_RESULT_OOM)
        return XSON_RESULT_OOM;

    /* rehash the key */
    h = xson_pair_ht_hash_by_pair(new) % ht->len;
    list_add_tail(&new->hash_link, &ht->table[h]);
    ++(ht->n_entries);
    return XSON_RESULT_SUCCESS;
}

inline void
xson_pair_ht_delete_by_key(struct xson_pair_ht * ht, const char * key) {
    unsigned            h;
    struct list_head    *p;
    
    h = xson_pair_ht_hash_by_key(key) % ht->len;
    

    list_for_each(p, &ht->table[h]) {
        struct xson_pair * entry = list_entry(p, struct xson_pair, hash_link);
        if (xson_pair_ht_cmp(entry,key) == 0) {
            list_del(p);
            --(ht->n_entries);
            return;
        }
    }
}

inline int
xson_pair_ht_delete(struct xson_pair_ht * ht, struct xson_pair * pair) {
    if (pair->hash_link.prev == NULL || pair->hash_link.next == NULL) {
        /*
        * This xson_pair is not in the hash table.
        * Assume every xson_pair only can be in one reactor.
        */
        return XSON_RESULT_SUCCESS;
    }

    list_del(&pair->hash_link);
    --(ht->n_entries);
    return XSON_RESULT_SUCCESS;
}

inline struct xson_pair *
xson_pair_ht_retrieve(struct xson_pair_ht * ht, const char * key) {
    unsigned            h;
    struct list_head    *p;

    h =  xson_pair_ht_hash_by_key(key) % ht->len;

    list_for_each(p, &ht->table[h]) {
        struct xson_pair * entry = list_entry(p, struct xson_pair, hash_link);
        if (xson_pair_ht_cmp(entry, key) == 0) {
            return entry;
        }
    }
    
    return NULL;
}

inline void xson_pair_ht_free(struct xson_pair_ht * ht) {
    free(ht->table);
}
