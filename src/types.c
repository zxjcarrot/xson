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
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include <errno.h>

#include "xson/common.h"
#include "xson/types.h"
#include "xson/parser.h"

/*
* Tests whether @expr is in a form of array accessing expression.
*/
static int xson_is_array_expression(const char * expr) {
	int         idx, ret;
	static char trailing[1];

	trailing[0] = 0;

	ret = sscanf(expr, "%*[^[][%d]%1s", &idx, trailing);

	return ret == 1;
}

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

/*
* if root element holds a array, access it like "[n]".
* Otherwise root element holds a object element, access
* it as normal object element.
*/
struct xson_element *
xson_root_get_child(struct xson_element * ele, const char * expr) {
	assert(ele != NULL);
	assert(ele->internal != NULL);
	struct xson_value *val;
	struct xson_element *child;

	if (ele == NULL)
		return XSON_EXPR_NULL;
	
	val = ele->internal;

	if (val == NULL)
		return XSON_EXPR_NULL;
	
	child = xson_value_get_elt(val);

	if(child == NULL)
		return XSON_EXPR_NULL;

	return child->ops->get_child(child, expr);
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
	xson_root_get_child,
	xson_root_add_child,
	xson_root_print
};

struct xson_element* xson_value_get_elt(struct xson_value * val){
	assert(val != NULL);

	if (val == NULL)
		return NULL;

	return (struct xson_element *)val->child;
}

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

struct xson_element *
xson_object_get_child(struct xson_element * ele, const char * expr) {
	assert(ele != NULL);
	assert(ele->internal != NULL);
	struct xson_object  *obj;

	if(ele == NULL || ele->internal == NULL)
		return XSON_EXPR_NULL;
	obj = ele->internal;

	return xson_object_get_pairval(obj, expr);
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
		if (xson_buffer_grow((void **)&obj->pairs,
		    &obj->size, obj->idx,
		    sizeof(struct xson_element *)) == -1) {
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
	XSON_PADDING_PRINT((dont_pad_on_first_line ? 0 : level * indent), "{\n");
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
	xson_object_get_child,
	xson_object_add_child,
	xson_object_print
};

/*
* Get the pair to which @key is mapped.
* Return: a pointer to that value element, NULL if the object
*		  contains no mapping for @key.
*/
struct xson_pair* xson_object_get_pair(struct xson_object * obj, const char * key) {
	assert(obj != NULL);
	assert(key != NULL);

	if (obj == NULL || key == NULL)
		return NULL;

	return xson_pair_ht_retrieve(&obj->ht, key);
}

/*
* Get the value element of the pair to which @key is mapped.
* Return: a pointer to that value element, NULL if the object
*		  contains no mapping for @key.
*/
struct xson_element* xson_object_get_pairval(struct xson_object * obj, const char * key) {
	struct xson_pair * pair = xson_object_get_pair(obj, key);

	if (pair == NULL)
		return NULL;

	return pair->value;
}

inline int xson_object_get_size(struct xson_object *obj){
	assert(obj != NULL);

	if(obj == NULL)
		return XSON_RESULT_ERROR;

	return obj->idx;
}

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

struct xson_element *
xson_array_get_child(struct xson_element * ele, const char * expr) {
	assert(ele != NULL);
	assert(ele->internal != NULL);
	struct xson_array   *array;
	int                  is_array, idx;

	if(ele == NULL || ele->internal == NULL)
		return XSON_EXPR_NULL;
	array = ele->internal;

	is_array = xson_is_array_expression(expr);

	if (!is_array)
		return XSON_EXPR_INVALID_EXPR;

	sscanf(expr, "%*[^[][%d]", &idx);

	if (idx < 0 || idx >= array->idx)
		return XSON_EXPR_INDEX_OOR;

	return xson_array_get_elt(array, idx);
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
		xson_buffer_grow((void **)&array->array,
						 &array->size, array->idx,
						 sizeof(struct xson_element *)) == -1) {
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
	xson_array_get_child,
	xson_array_add_child,
	xson_array_print
};

struct xson_element* xson_array_get_elt(struct xson_array * array, int idx) {
	assert(array != NULL);

	if(array == NULL || idx >= array->idx || idx < 0)
		return NULL;

	return array->array[idx];
}

inline int xson_array_get_size(struct xson_array *array) {
	assert(array != NULL);

	if(array == NULL)
		return XSON_RESULT_ERROR;

	return array->idx;
}

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

struct xson_element *
xson_string_get_child(struct xson_element * ele, const char * expr) {
	return XSON_EXPR_OP_NOTSUPPORTED;
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
	xson_string_get_child,
	xson_string_add_child,
	xson_string_print
};

int xson_string_to_buf(struct xson_string * string, char * buf, size_t len){
	assert(string != NULL);
	assert(buf != NULL);

	if(string == NULL || buf == NULL)
		return XSON_RESULT_ERROR;

	memcpy(buf, string->start, (size_t)(string->end - string->start + 1));

	return XSON_RESULT_SUCCESS;
}
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

struct xson_element *
xson_number_get_child(struct xson_element * ele, const char * expr) {
	return XSON_EXPR_OP_NOTSUPPORTED;
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
	xson_number_get_child,
	xson_number_add_child,
	xson_number_print
};


int xson_number_to_ullong(struct xson_number * number, unsigned long long *out) {
	unsigned long long t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtoull(number->start, NULL, 10);

	if (t == ULLONG_MAX && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}

int xson_number_to_llong(struct xson_number * number, long long *out){
	long long t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtoll(number->start, NULL, 10);

	if (t == LLONG_MAX && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}


int xson_number_to_ulong(struct xson_number * number, unsigned long int *out){
	unsigned long int t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtoul(number->start, NULL, 10);

	if (t == ULONG_MAX && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}

int xson_number_to_long(struct xson_number * number, long int *out){
	long int t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtol(number->start, NULL, 10);

	if (t == LONG_MAX && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}

int xson_number_to_uint(struct xson_number * number, unsigned int *out){
	unsigned long int t;
	int 	 		  res;

	res = xson_number_to_ulong(number, &t);

	if(res != XSON_RESULT_SUCCESS)
		return res;

	if(t > (unsigned long)UINT_MAX)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}

int xson_number_to_int(struct xson_number * number, int *out){
	long int t;
	int 	 res;

	res = xson_number_to_long(number, &t);

	if(res != XSON_RESULT_SUCCESS)
		return res;

	if(t > (long)INT_MAX || t < (long)INT_MIN)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}

int xson_number_to_ldouble(struct xson_number * number, long double *out){
	long double t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtold(number->start, NULL);

	if ((t == -HUGE_VALL || t == HUGE_VALL) && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}
int xson_number_to_double(struct xson_number * number, double *out){
	double t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtod(number->start, NULL);

	if ((t == -HUGE_VAL || t == HUGE_VAL) && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;

	return XSON_RESULT_SUCCESS;
}

int xson_number_to_float(struct xson_number * number, float *out){
	float t;

	assert(number != NULL);
	assert(out != NULL);

	if (number == NULL || out == NULL)
		return XSON_RESULT_ERROR;

	errno = 0;
	t = strtof(number->start, NULL);

	if ((t == -HUGE_VALF || t == HUGE_VALF) && errno == ERANGE)
		return XSON_RESULT_OOR;

	*out = t;
	
	return XSON_RESULT_SUCCESS;
}

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

struct xson_element *
xson_pair_get_child(struct xson_element * ele, const char * expr) {
	return XSON_EXPR_OP_NOTSUPPORTED;
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
	xson_pair_get_child,
	xson_pair_add_child,
	xson_pair_print
};


struct xson_string* xson_pair_get_key(struct xson_pair * pair) {
	assert(pair != NULL);
	assert(pair->key != NULL);

	if (pair == NULL || pair->key == NULL)
		return NULL;

	return (struct xson_string *)pair->key->internal;
}

struct xson_element* xson_pair_get_value(struct xson_pair * pair) {
	assert(pair != NULL);
	assert(pair->value != NULL);

	if (pair == NULL)
		return NULL;

	return pair->value;
}


/*
* Convert the element to a specific type.
* Return: a pointer to that specific type of element,
*         NULL if actual type of the lement does not
*         match the desired type(implied by the fucntion name).
* @elt: the element to convert.
*/
struct xson_string * xson_elt_to_string(struct xson_element * elt) {
	assert(elt != NULL);
	assert(elt->internal != NULL);

	if (elt == NULL || elt->type != ELE_TYPE_STRING) 
		return NULL;

	return (struct xson_string *)elt->internal;
}

struct xson_number * xson_elt_to_number(struct xson_element * elt) {
	assert(elt != NULL);
	assert(elt->internal != NULL);
	
	if (elt == NULL || elt->type != ELE_TYPE_NUMBER) 
		return NULL;

	return (struct xson_number *)elt->internal;
}

struct xson_value * xson_elt_to_value(struct xson_element * elt) {
	assert(elt != NULL);
	assert(elt->internal != NULL);

	if (elt == NULL || (elt->type != ELE_TYPE_VALUE && elt->type != ELE_TYPE_ROOT))
		return NULL;

	return (struct xson_value *)elt->internal;
}

struct xson_array * xson_elt_to_array(struct xson_element * elt) {
	assert(elt != NULL);
	assert(elt->internal != NULL);

	if (elt == NULL || elt->type != ELE_TYPE_ARRAY) 
		return NULL;

	return (struct xson_array *)elt->internal;
}

struct xson_object * xson_elt_to_object(struct xson_element * elt) {
	assert(elt != NULL);
	assert(elt->internal != NULL);

	if (elt == NULL || elt->type != ELE_TYPE_OBJECT)
		return NULL;

	return (struct xson_object *)elt->internal;
}

struct xson_pair * xson_elt_to_pair(struct xson_element * elt) {
	assert(elt != NULL);
	assert(elt->internal != NULL);

	if (elt == NULL || elt->type != ELE_TYPE_PAIR)
		return NULL;

	return (struct xson_pair *)elt->internal;
}


struct xson_element * xson_get_by_expr(struct xson_element * elt, const char * key) {
	assert(elt != NULL);
	assert(key != NULL);
	char *p;
	char *buf;

	if (elt == NULL || key == NULL)
		return XSON_EXPR_NULL;

	buf = malloc(strlen(key) + 1);

	if (buf == NULL)
		return XSON_EXPR_OOM;

	strcpy(buf, key);

	p = strtok(buf, ".");

	while (p) {
		elt = elt->ops->get_child(elt, p);
		if (!XSON_GOOD_ELEMENT(elt))
			break;
	}

	free(buf);
	
	return elt;
}

static int xson_convert_expr_res_to_res(struct xson_element * elt) {
	if (elt == XSON_EXPR_NULL)
		return XSON_RESULT_ERROR;
	else if(elt == XSON_EXPR_OOM)
		return XSON_RESULT_OOM;
	else if(elt == XSON_EXPR_INDEX_OOR)
		return XSON_RESULT_OOR;
	else if(elt == XSON_EXPR_KEY_NOT_EXIST)
		return XSON_RESULT_KEY_NOT_EXIST;
	else if(elt == XSON_EXPR_OP_NOTSUPPORTED)
		return XSON_RESULT_OP_NOTSUPPORTED;
	else if(elt == XSON_EXPR_INVALID_EXPR)
		return XSON_RESULT_INVALID_EXPR;

	return XSON_RESULT_SUCCESS;
}

int xson_get_ull_by_expr(struct xson_element * elt, const char * expr,
                         unsigned long long *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_ullong(number, out);
}

int xson_get_llong_by_expr(struct xson_element * elt, const char * expr,
                        long long *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_llong(number, out);
}

int xson_get_ulong_by_expr(struct xson_element * elt, const char * expr,
                           unsigned long int *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_ulong(number, out);
}

int xson_get_long_by_expr(struct xson_element * elt, const char * expr,
                          long int *out) {
		int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_long(number, out);
}

int xson_get_uint_by_expr(struct xson_element * elt, const char * expr, 
                          unsigned int *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_uint(number, out);
}

int xson_get_int_by_expr(struct xson_element * elt, const char * expr,
                         int *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_int(number, out);
}

int xson_get_double_by_expr(struct xson_element * elt, const char * expr,
                            double *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_double(number, out);
}

int xson_get_float_expr(struct xson_element * elt, const char * expr,
                        float *out) {
	int                  rc;
	struct xson_number * number;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	number = elt->internal;

	return xson_number_to_float(number, out);
}

int xson_get_string_by_expr(struct xson_element * elt, const char * expr,
                            char * buf, size_t size) {
	int                  rc;
	struct xson_string * string;

	elt = xson_get_by_expr(elt, expr);

	if ((rc = xson_convert_expr_res_to_res(elt)) != XSON_RESULT_SUCCESS)
		return rc;

	string = elt->internal;

	return xson_string_to_buf(string, buf, size);
}