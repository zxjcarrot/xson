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
#include <stdint.h>
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
    /*
    * Initialize the element with the given lexical element.
    * Return: XSON_RESULT_SUCCESS on success, XSON_RESULT_OOM if
    *         there is a memory shortage during the initialization.
    */
    int (*initialize)(struct xson_element * ele, struct xson_lex_element * lex);
    /*
    * Free up resources held by the element recursively.
    */ 
    void (*destroy)(struct xson_element * ele);
    

    /*
    * Get child element by an expression(key1.key2[i].key3....).
    * Return: a pointer to the child element,
    *         XSON_RESULT_OP_NOTSUPPORTED if this type of element 
    *         does not support this operation(xson_string, xson_number),
    *         XSON_RESULT_INVALID_JSON if @expr is invalid for
    *         this type of element.
    */
    struct xson_element * (*get_child)(struct xson_element * ele, const char * expr);

    /*
    * Add a element to another element as a child.
    * Return: XSON_RESULT_SUCCESS on success,
    *         XSON_RESULT_INVALID_JSON if the parent element 
    *         does not support adding a child element which 
    *         implies the input json string contains grammatical errors,
    *         XSON_RESULT_OOM if there is a memory shortage
    *         during the adding operation.
    */
    int (*add_child)(struct xson_element * parent, struct xson_element * child);
    /*
    * Print out the element contents recursively.
    */
    void (*print)(struct xson_element * ele, int level, int indent, int dont_break_on_first_line);
}xson_ele_operations;

typedef struct xson_pair {
    struct xson_element * key;
    struct xson_element * value;
    struct list_head hash_link;
}xson_pair;

/*
* Get the key of a pair.
* Return: a pointer that key element, NULL if pair is null or
*         there is no key cotained in the pair.
*/
struct xson_string* xson_pair_get_key(struct xson_pair * pair);

/*
* Get the value of a pair.
* Return: a pointer that value element, NULL if pair is null or
*         there is no value element cotained in the pair.
*/
struct xson_element* xson_pair_get_value(struct xson_pair * pair);

typedef struct xson_object {
#define XSON_OBJECT_INIT_PAIRS_SIZE 8
    struct xson_pair_ht ht;
    struct xson_element ** pairs;
    int idx;
    int size;
}xson_object;

/*
* Get the pair to which @key is mapped.
* Return: a pointer to that value element, NULL if the object
*         contains no mapping for @key.
*/
struct xson_pair* xson_object_get_pair(struct xson_object * obj, const char * key);

/*
* Get the value element of the pair to which @key is mapped.
* Return: a pointer to that value element, NULL if the object
*         contains no mapping for @key.
*/
struct xson_element* xson_object_get_pairval(struct xson_object * obj, const char * key);

/*
* Get the number of pairs contained in the object.
* Return: the number of element contained in the object,
*         XSON_RESULT_ERROR if @obj is null.
*/
inline int xson_object_get_size(struct xson_object *obj);

typedef struct xson_array {
#define XSON_OBJECT_INIT_ARRAY_SIZE 16
    struct xson_element ** array;
    int idx;
    int size;
}xson_array;

/*
* Get the the @idxth element in the array.
* Return: a pointer to that element, NULL if the idx is out of range.
*/
struct xson_element* xson_array_get_elt(struct xson_array * array, int idx);

/*
* Get the number of elements in the array.
*/
inline int xson_array_get_size(struct xson_array *array);

typedef struct xson_value {
    struct xson_element * child;
}xson_value;

/*
* Get the element of the given value object.
* Return: a pointer to that element,
*         NULL of @val is null or @val does not contain
*         any element at all.
*/
struct xson_element* xson_value_get_elt(struct xson_value * val);

typedef struct xson_number {
    char *start, *end;
}xson_integer;

typedef struct xson_bool {
    int bool_val;
}xson_bool;

/*
* Tests whether @expr is in a form of array accessing expression.
*/
int xson_is_array_expression(const char * expr);


/*
* Convert the number to a specific type.
* Return: XSON_RESULT_SUCCESS if the conversion is successful,
*         XSON_RESULT_OOG if the value overflows or underflows,
*         XSON_RESULT_ERROR if the @number or @out is null.
*/
int xson_number_to_ullong(struct xson_number * number, unsigned long long *out);
int xson_number_to_llong(struct xson_number * number, long long *out);
int xson_number_to_ulong(struct xson_number * number, unsigned long int *out);
int xson_number_to_long(struct xson_number * number, long int *out);
int xson_number_to_uint(struct xson_number * number, unsigned int *out);
int xson_number_to_int(struct xson_number * number, int *out);
int xson_number_to_intptr(struct xson_number * number, intptr_t *out);
int xson_bool_to_int(struct xson_bool * xbool, int *out);
int xson_number_to_double(struct xson_number * number, double *out);
int xson_number_to_float(struct xson_number * number, float *out);

typedef struct xson_string {
    char *start, *end;
}xson_string;

/*
* Copy the string to a buffer specified by @buf with buffer size of @len.
* Return: XSON_RESULT_SUCCESS if the copying is successful, 
*         XSON_RESULT_ERROR if @string or @buf is null.
*/
int xson_string_to_buf(struct xson_string * string, char * buf, size_t len);

typedef struct xson_element {
    enum xson_ele_type          type;
    struct xson_context        *ctx;
    struct xson_element        *parent;
    void                       *internal;
    struct xson_ele_operations *ops;   
}xson_element;

/*
* Convert the element to a specific type.
* Return: a pointer to that element of the specific type,
*         NULL if actual type of the lement does not
*         match the desired type(implied by the fucntion name).
* @elt: the element to convert.
*/
struct xson_string * xson_elt_to_string(struct xson_element * elt);
struct xson_number * xson_elt_to_number(struct xson_element * elt);
struct xson_value  * xson_elt_to_value(struct xson_element * elt);
struct xson_array  * xson_elt_to_array(struct xson_element * elt);
struct xson_object * xson_elt_to_object(struct xson_element * elt);
struct xson_pair   * xson_elt_to_pair(struct xson_element * elt);


struct xson_element * xson_get_by_expr(struct xson_element * elt, const char * key);

/*
* Accessing a field of number type by expression.
* Return: XSON_RESULT_SUCCESS if the conversion is successful,
*         XSON_RESULT_OOG if the value overflows or underflows,
*         XSON_RESULT_ERROR if the @elt or @expr or @out is null,
*         XSON_RESULT_TYPE_MISMATCH if the type of element
*         indicated by the last key in @expr is not compatible
*         with the type implied by the function name.
* @elt: the root element to start with.
* @expr: the dot-separated keys(key1.key2.key3[n].key4 etc...).
* @out: this field holds the value if successfully converted.
*/
int xson_get_ullong_by_expr(struct xson_element * elt, const char * expr, unsigned long long *out);
int xson_get_llong_by_expr(struct xson_element * elt, const char * expr, long long *out);
int xson_get_ulong_by_expr(struct xson_element * elt, const char * expr, unsigned long int *out);
int xson_get_long_by_expr(struct xson_element * elt, const char * expr, long int *out);
int xson_get_uint_by_expr(struct xson_element * elt, const char * expr, unsigned int *out);
int xson_get_int_by_expr(struct xson_element * elt, const char * expr, int *out);
int xson_get_intptr_by_expr(struct xson_element * elt, const char * expr, intptr_t *out);
int xson_get_bool_by_expr(struct xson_element * elt, const char * expr, int *out);
int xson_get_double_by_expr(struct xson_element * elt, const char * expr, double *out);
int xson_get_float_expr(struct xson_element * elt, const char * expr, float *out);


/*
* Accessing a field of string type by expression.
* Return: XSON_RESULT_SUCCESS if the conversion is successful,
*         XSON_RESULT_ERROR if the @elt or @expr or @out is null,
*         XSON_RESULT_TYPE_MISMATCH if the type of element
*         indicated by the last key in @expr is not compatible
*         with the type implied by the function name.
* @elt: the root element to start with.
* @expr: the dot-separated keys(key1.key2.key3[n].key4 etc...).
* @buf: the buffer holds the string.
* @size: size of @buf in byte.
*/
int xson_get_string_by_expr(struct xson_element * elt, const char * expr,char * buf, size_t size);

/*
* Accessing a field of specific type by expression.
* Return: a pointer to that element,
*         NULL if any of the follwing is true:
*           1. @elt is null.
*           2. @expr is null.
*           3. the type of element indicated by the last key in @expr does
*              not match the type implied by the function name(array, object).
* @elt: the root element to start with.
* @expr: the dot-separated keys(key1.key2.key3[n].key4 etc...).
*/
struct xson_array * xson_get_array_by_expr(struct xson_element * elt, const char * expr);
struct xson_object * xson_get_object_by_expr(struct xson_element * elt, const char * expr);

/*
* Get the size of the array indicated by the last key in @expr.
* Return: size of the array,
*         XSON_RESULT_ERROR if the @elt or @expr or @out is null,
*         XSON_RESULT_TYPE_MISMATCH if the type of element
*         indicated by the last key in @expr is not compatible
*         with the type implied by the function name.
* @elt: the root element to start with.
* @expr: the dot-separated keys(key1.key2.key3[n].key4 etc...).
*/
int xson_get_arraysize_by_expr(struct xson_element * elt, const char * expr);
int xson_get_stringsize_by_expr(struct xson_element * elt, const char * expr);
#ifdef __cplusplus
}
#endif
#endif