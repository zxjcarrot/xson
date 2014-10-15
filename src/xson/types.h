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
#ifndef XSON_TYPES_H_
#define XSON_TYPES_H_
#include "pair_ht.h"
#include "list.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* forward declarations */
struct xson_pair;
struct xson_object;
struct xson_array;
struct xson_value;
struct xson_number;
struct xson_string;
struct xson_element;
struct xson_context;

typedef struct xson_ele_operations {
	int (*initialize)(struct xson_element * ele, struct xson_lex_element * lex);
	void (*destroy)(struct xson_element * ele);
	int (*read)(struct xson_element * ele, void * buf, int bufsize);
	int (*add_child)(struct xson_element * parent, struct xson_element * child);
	void (*print)(struct xson_element * ele, int level, int indent, int dont_break_on_first_line);
}xson_ele_operations;

typedef struct xson_pair {
	struct xson_element * key;
	struct xson_element * value;
	struct list_head hash_link;
}xson_pair;

struct xson_string* xson_pair_get_key(struct xson_pair * pair);

struct xson_element* xson_pair_get_value(struct xson_pair * pair);

typedef struct xson_object {
#define XSON_OBJECT_INIT_PAIRS_SIZE 8
	struct xson_pair_ht ht;
	struct xson_element ** pairs;
	int idx;
	int size;
}xson_object;
struct xson_pair* xson_object_get_pair(struct xson_object * obj, const char * key);

struct xson_element* xson_object_get_pairval(struct xson_object * obj, const char * key);

inine int xson_object_get_size(struct xson_object *obj);

typedef struct xson_array {
#define XSON_OBJECT_INIT_ARRAY_SIZE 16
	struct xson_element ** array;
	int idx;
	int size;
}xson_array;

struct xson_element* xson_array_get_elt(struct xson_array * array, int idx);

typedef struct xson_value {
	struct xson_element * child;
}xson_value;

struct xson_element* xson_value_get_elt(struct xson_value * val);

typedef struct xson_number {
	char *start, *end;
}xson_integer;

int xson_number_to_ullong(struct xson_number * number, unsigned long long *out);
int xson_number_to_llong(struct xson_number * number, long long *out);
int xson_number_to_ulong(struct xson_number * number, unsigned long int *out);
int xson_number_to_long(struct xson_number * number, long int *out);
int xson_number_to_uint(struct xson_number * number, unsigned int *out);
int xson_number_to_int(struct xson_number * number, int *out);
int xson_number_to_double(struct xson_number * number, double *out);
int xson_number_to_float(struct xson_number * number, double *out);

typedef struct xson_string {
	char *start, *end;
}xson_string;

typedef struct xson_element {
	enum xson_ele_type type;
	struct xson_context * ctx;
	struct xson_element * parent;
	void * internal;
	struct xson_ele_operations * ops;
}xson_element;

#ifdef __cplusplus
}
#endif
#endif