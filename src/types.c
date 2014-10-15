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
#include "xson/types.h"
#include "xson/parser.h"

int xson_root_initialize(struct xson_element * e, struct xson_lex_element * lex) {
	struct xson_value * val = xson_malloc(&e->ctx->pool, sizeof(struct xson_value));
	if (val == NULL) {
		return XSON_RESULT_OOM;
	}
	val->child = NULL;
	e->internal = val;

	return XSON_RESULT_SUCCESS;
}
void xson_root_destroy(struct xson_element * ele) {
	struct xson_value * val = ele->internal;
	val->child->ops->destroy(val->child);
}

int xson_root_read(struct xson_element * e, void * buf, int bufsize) {
	printf("Read operation on root element is NOT supported.\n");
	return XSON_RESULT_INVALID_JSON;
}

int xson_root_add_child(struct xson_element * parent, struct xson_element * child) {
	struct xson_value * val = (struct xson_value *)parent->internal;
	if (child->type != ELE_TYPE_OBJECT &&
	   child->type != ELE_TYPE_ARRAY) {
		printf("Child type for root element must be one of: object, pair.\n");
		return XSON_RESULT_INVALID_JSON;
	}else if (val->child) {
		printf("Only one child can be added on root element\n");
		return XSON_RESULT_INVALID_JSON;
	}
	val->child = child;
	child->parent = parent;
	return XSON_RESULT_SUCCESS;
}
void xson_root_print(struct xson_element * ele, int level, int indent, int dont_pad_on_first_line) {
	struct xson_value * val = ele->internal;
	val->child->ops->print(val->child, level, indent, 0);
	XSON_PADDING_PRINT(level * indent, "\n");
}
struct xson_ele_operations root_ops =  {
	xson_root_initialize,
	xson_root_destroy,
	xson_root_read,
	xson_root_add_child,
	xson_root_print
};

int xson_object_initialize(struct xson_element * e, struct xson_lex_element * lex) {
	struct xson_object * obj;
	
	if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_object))) == NULL) {
		return XSON_RESULT_OOM;
	}
	
	obj = (struct xson_object *)e->internal;
	
	if ((obj->pairs = malloc(sizeof(struct xson_element *) * XSON_OBJECT_INIT_PAIRS_SIZE)) == NULL) {
		return XSON_RESULT_OOM;
	}
	if (xson_pair_ht_init(&obj->ht) == XSON_RESULT_OOM) {
		free(obj->pairs);
		return XSON_RESULT_OOM;
	}
	obj->idx = 0;
	obj->size = XSON_OBJECT_INIT_PAIRS_SIZE;
	return XSON_RESULT_SUCCESS;
}

void xson_object_destroy(struct xson_element * ele) {
	int 				i;
	struct xson_object 	*obj = ele->internal;

	for (i = 0; i < obj->idx; ++i) {
		obj->pairs[i]->ops->destroy(obj->pairs[i]);
	}
	xson_pair_ht_free(&obj->ht);
	free(obj->pairs);
}

int xson_object_read(struct xson_element * e, void * buf, int bufsize) {
	printf("Read operation on object element is NOT supported.\n");
	return XSON_RESULT_INVALID_JSON;
}

int xson_object_add_child(struct xson_element * parent, struct xson_element * child) {
	int 				ret;
	struct xson_object 	*obj = (struct xson_object *)parent->internal;
	if (child->type != ELE_TYPE_PAIR) {
		printf("Child type for object element must be pair.\n");
		return XSON_RESULT_INVALID_JSON;
	}
	
	ret = xson_pair_ht_insert_replace(&obj->ht, (struct xson_pair *)child->internal);
	if (ret != XSON_RESULT_SUCCESS) {
		assert(0);
		return ret;
	}
	if (obj->idx >= obj->size) {
		if (xson_buffer_grow((void **)&obj->pairs, &obj->size, obj->idx, sizeof(struct xson_element *)) == -1) {
			assert(0);
			return XSON_RESULT_OOM;
		}
	}
	obj->pairs[obj->idx++] = child;
	child->parent = parent;
	return XSON_RESULT_SUCCESS;;
}

void xson_object_print(struct xson_element * ele, int level, int indent, int dont_pad_on_first_line) {
	int 				i;
	struct xson_object 	*obj = ele->internal;
	XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), " {\n");
	for (i = 0; i < obj->idx; ++i) {
		if (i)XSON_PADDING_PRINT(0, ",\n");
		obj->pairs[i]->ops->print(obj->pairs[i], level + 1, indent, 0);
		if (i == obj->idx - 1)XSON_PADDING_PRINT(0, "\n");
	}
	XSON_PADDING_PRINT(level * indent, "}");
}
struct xson_ele_operations object_ops = {
	xson_object_initialize,
	xson_object_destroy,
	xson_object_read,
	xson_object_add_child,
	xson_object_print
};

int xson_array_initialize(struct xson_element * e, struct xson_lex_element * lex) {
	struct xson_array *array;
	
	if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_array))) == NULL) {
		return XSON_RESULT_OOM;
	}
	
	array = (struct xson_array *)e->internal;
	
	if ((array->array = malloc(sizeof(struct xson_element *) * XSON_OBJECT_INIT_ARRAY_SIZE)) == NULL) {
		return XSON_RESULT_OOM;
	}
	array->idx = 0;
	array->size = XSON_OBJECT_INIT_ARRAY_SIZE;
	return XSON_RESULT_SUCCESS;
}

void xson_array_destroy(struct xson_element * ele) {
	int 				i;
	struct xson_array 	*array = ele->internal;
	for (i = 0; i < array->idx; ++i) {
		array->array[i]->ops->destroy(array->array[i]);
	}
	free(array->array);
}

int xson_array_read(struct xson_element * e, void * buf, int bufsize) {
	printf("Read operation on array element is NOT supported.\n");
	return XSON_RESULT_INVALID_JSON;
}

int xson_array_add_child(struct xson_element * parent, struct xson_element * child) {
	struct xson_array *array = (struct xson_array *)parent->internal;
	if (child->type != ELE_TYPE_NUMBER &&
	   child->type != ELE_TYPE_STRING &&
	   child->type != ELE_TYPE_OBJECT &&
	   child->type != ELE_TYPE_ARRAY) {
		printf("Child type for array element must be one of: number, string, object, array.\n");
		return XSON_RESULT_INVALID_JSON;
	}
	
	if (array->idx >= array->size && 
		xson_buffer_grow((void **)&array->array, &array->size, array->idx, sizeof(struct xson_element *)) == -1) {
		return XSON_RESULT_OOM;
	}
	array->array[array->idx++] = child;
	child->parent = parent;
	return XSON_RESULT_SUCCESS;
}
void xson_array_print(struct xson_element * ele, int level, int indent, int dont_pad_on_first_line) {
	int 				i;
	struct xson_array 	*array = ele->internal;
	XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "[\n");
	for (i = 0; i < array->idx; ++i) {
		if (i)XSON_PADDING_PRINT(0, ",\n");
		assert(array->array[i]);
		assert(array->array[i]->ops);
		array->array[i]->ops->print(array->array[i], level + 1, indent, 0);
		if (i == array->idx - 1)XSON_PADDING_PRINT(0, "\n");
	}
	XSON_PADDING_PRINT(level * indent, "]");
}
struct xson_ele_operations array_ops =  {
	xson_array_initialize,
	xson_array_destroy,
	xson_array_read,
	xson_array_add_child,
	xson_array_print
};

int xson_string_initialize(struct xson_element * e, struct xson_lex_element * lex) {
	struct xson_string *string;
	if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_string))) == NULL) {
		return XSON_RESULT_OOM;
	}
	string = (struct xson_string *)e->internal;
	string->start = lex->start;
	string->end = lex->end;
	return XSON_RESULT_SUCCESS;
}
void xson_string_destroy(struct xson_element * ele) {
	;
}
int xson_string_read(struct xson_element * e, void * buf, int bufsize) {
	return XSON_RESULT_SUCCESS;
}

int xson_string_add_child(struct xson_element * parent, struct xson_element * child) {
	printf("Adding children to string element is NOT supported.\n");
	return XSON_RESULT_INVALID_JSON;
}
void xson_string_print(struct xson_element * ele, int level, int indent, int dont_pad_on_first_line) {
	struct xson_string 	*string = ele->internal;
	char 				t = *(string->end + 1);

	*(string->end + 1) = 0;
	XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "\"%s\"", string->start);
	*(string->end + 1) = t;
}
struct xson_ele_operations string_ops =  {
	xson_string_initialize,
	xson_string_destroy,
	xson_string_read,
	xson_string_add_child,
	xson_string_print
};

int xson_number_initialize(struct xson_element * e, struct xson_lex_element * lex) {
	struct xson_number * number;
	if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_number))) == NULL) {
		return XSON_RESULT_OOM;
	}
	number = (struct xson_number *)e->internal;
	number->start = lex->start;
	number->end = lex->end;
	return XSON_RESULT_SUCCESS;
}
void xson_number_destroy(struct xson_element * ele) {
	;
}
int xson_number_read(struct xson_element * e, void * buf, int bufsize) {
	return XSON_RESULT_SUCCESS;
}

int xson_number_add_child(struct xson_element * parent, struct xson_element * child) {
	printf("Adding children to number element is NOT supported.\n");
	return XSON_RESULT_INVALID_JSON;
}
void xson_number_print(struct xson_element * ele, int level, int indent, int dont_pad_on_first_line) {
	struct xson_number 	*number = ele->internal;
	char 				t = *(number->end + 1);
	
	
	*(number->end + 1) = 0;
	XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "%s", number->start);
	*(number->end + 1) = t;
}
struct xson_ele_operations number_ops =  {
	xson_number_initialize,
	xson_number_destroy,
	xson_number_read,
	xson_number_add_child,
	xson_number_print
};

int xson_pair_initialize(struct xson_element * e, struct xson_lex_element * lex) {
	struct xson_pair *pair;
	if ((e->internal = xson_malloc(&e->ctx->pool, sizeof(struct xson_pair))) == NULL) {
		return XSON_RESULT_OOM;
	}
	pair = (struct xson_pair *)e->internal;
	memset(pair, 0, sizeof(struct xson_pair));
	return XSON_RESULT_SUCCESS;
}
void xson_pair_destroy(struct xson_element * ele) {
	struct xson_pair *pair = ele->internal;
	pair->key->ops->destroy(pair->key);
	pair->value->ops->destroy(pair->value);
}
int xson_pair_read(struct xson_element * e, void * buf, int bufsize) {
	return XSON_RESULT_INVALID_JSON;
}

int xson_pair_add_child(struct xson_element * parent, struct xson_element * child) {
	printf("Adding children to pair element is NOT supported.\n");
	return XSON_RESULT_INVALID_JSON;
}
void xson_pair_print(struct xson_element * ele, int level, int indent, int dont_pad_on_first_line) {
	struct xson_pair *pair = ele->internal;
	pair->key->ops->print(pair->key, level, indent, 0);
	XSON_PADDING_PRINT(0, ":");
	pair->value->ops->print(pair->value, level, indent, 1);
}
struct xson_ele_operations pair_ops =  {
	xson_pair_initialize,
	xson_pair_destroy,
	xson_pair_read,
	xson_pair_add_child,
	xson_pair_print
};
