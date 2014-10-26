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
#ifndef XSON_PAIR_HT_H_
#define XSON_PAIR_HT_H_

#ifdef __cplusplus
extern "C" {
#endif
/* The load factor we apply to the hash table */
#define LOAD_FACTOR 0.7

struct list_head;
struct xson_pair;
typedef struct xson_pair_ht {
    struct list_head * table;
    /* The index of xson_pair_ht_primes we are using as the size of the hash table */
    int p_index;
    /* the number of entries this hash table has */
    int n_entries;
    /* The number of slots this hash table has */
    int len;
}xson_pair_ht;

/*
* BKDR Hash Function.
* @p: the pair to be hashed.
*/
inline unsigned xson_pair_ht_hash_by_pair(struct xson_pair * p);

inline unsigned xson_pair_ht_hash_by_key(const char * key);

/*
* Initialize the xson_pair hash table.
* @ht: &struct xson_pair_ht to be initialized.
*/
inline int xson_pair_ht_init(struct xson_pair_ht * ht);

/*
* Insert a xson_pair into the hash table.
* Do nothing if the xson_pair is already in the table,
* @ht: &struct xson_pair_ht into which the xson_pair to be inserted
* @xson_pair: &struct xson_pair entry to be inserted
* @key: hash key
*/
inline int xson_pair_ht_insert(struct xson_pair_ht * ht, struct xson_pair * new_entry);

/*
* Insert a xson_pair into the hash table. 
* Replace old xson_pair by new one if the old xson_pair is already in the table.
* @ht: &struct xson_pair_ht into which the xson_pair to be inserted
* @xson_pair: &struct xson_pair entry to be inserted
* @key: hash key
*/
inline int xson_pair_ht_insert_replace(struct xson_pair_ht * ht, struct xson_pair * new_entry);

/*
* Delete the xson_pair with the key from the hash table.
* Do nothing if there is no matching key.
* @ht: &struct xson_pair_ht from which the xson_pair will be deleted
* @key: hash key
*/
inline void xson_pair_ht_delete_by_key(struct xson_pair_ht * ht, const char * key);

/*
* Delete the xson_pair with the key from the hash table.
* Do nothing if there is no matching key.
* @ht: &struct xson_pair_ht from which the xson_pair will be deleted
* @key: hash key
*/
inline int xson_pair_ht_delete(struct xson_pair_ht * ht, struct xson_pair * pair);


/*
* Retrieve the coressponding xson_pair from the hash table.
* Return null if there is no matching key.
* @ht: &struct xson_pair_ht from which the xson_pair will be retrieved
* @key: hash key
*/
inline struct xson_pair * xson_pair_ht_retrieve(struct xson_pair_ht * ht, const char * key);

/*
* Free up the hash table.
* @ht: the hash table
*/
inline void xson_pair_ht_free(struct xson_pair_ht * ht);

#ifdef __cplusplus
}
#endif
#endif