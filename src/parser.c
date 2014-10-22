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
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "xson/parser.h"
#include "xson/fsm_string.h"
#include "xson/fsm_number.h"

extern struct xson_ele_operations root_ops;
extern struct xson_ele_operations object_ops;
extern struct xson_ele_operations array_ops;
extern struct xson_ele_operations string_ops;
extern struct xson_ele_operations number_ops;;
extern struct xson_ele_operations pair_ops;

static int xson_element_initialize(struct xson_context * ctx, 
							struct xson_element * ele,
							enum xson_ele_type type) {
	memset(ele, 0, sizeof(struct xson_element));
	ele->ctx = ctx;
	ele->type = type;
	
	switch (type) {
		case ELE_TYPE_ROOT:
			ele->ops = &root_ops;
			break;
		case ELE_TYPE_OBJECT:
			ele->ops = &object_ops;
			break;
		case ELE_TYPE_ARRAY:
			ele->ops = &array_ops;
			break;
		case ELE_TYPE_STRING:
			ele->ops = &string_ops;
			break;
		case ELE_TYPE_NUMBER:
			ele->ops = &number_ops;
			break;
		case ELE_TYPE_PAIR:
			ele->ops = &pair_ops;
			break;
		default:
			printf("Unknown element type %d.\n", type);
			return (-1);
	}

	return 0;
}

int xson_init(struct xson_context * ctx, const char * str) {
	int len;
	assert(str != NULL);
	assert(ctx != NULL);

	len = strlen(str);
	if ((ctx->str_buf = malloc((len + 1) * sizeof(char))) == NULL) {
		return (-1);
	}
	if (xmpool_init(&ctx->pool, XM_CHUNK_SIZE) == -1) {
		free(ctx->str_buf);
		return (-1);
	}
	strcpy(ctx->str_buf, str);
	ctx->str_len = len;

	if ((ctx->root = xson_malloc(&ctx->pool, sizeof(struct xson_element))) == NULL) {
		free(ctx->str_buf);
		xmpool_destroy(&ctx->pool);
		return (-1);
	}
	xson_element_initialize(ctx, ctx->root, ELE_TYPE_ROOT);
	ctx->root->parent = ctx->root;
	if (ctx->root->ops->initialize(ctx->root, NULL) == XSON_RESULT_OOM) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	if ((ctx->stack = malloc(XSON_CTX_INIT_STK_LEN * sizeof(struct xson_lex_element))) == NULL) {
		free(ctx->str_buf);
		free(ctx->root);
		xmpool_destroy(&ctx->pool);
		return (-1);
	}

	ctx->stk_len = XSON_CTX_INIT_STK_LEN;
	ctx->stk_top = 0;
	ctx->stack[0].state = LEX_STATE_EMPTY;
	ctx->stack[0].element = ctx->root;
	ctx->stk_top = 1;

	return 0;
}

/*
* Push a lex element into the lex stack,
* expand the stack if necessary.
* Return value: the lex element just pushed into the stack, NULL if out of memory.
* @ctx: the context.
* @state: the state of the lex element.
* @start: starting pointer to contents of this lex element.
* @end: ending pointer to contents of this lex element.
* @e: the xson_element associated with the lex element.
*/
static struct xson_lex_element * xson_stack_push(struct xson_context * ctx,
					 enum xson_lex_state state,
					  char * start, char * end,
					  struct xson_element * e) {
	if (ctx->stk_top >= ctx->stk_len
		&& xson_buffer_grow((void **)&ctx->stack, &ctx->stk_len, ctx->stk_top, sizeof(struct xson_lex_element))
			== XSON_RESULT_OOM) {
		assert(0);
		return NULL;
	}
	struct xson_lex_element * ret = &ctx->stack[ctx->stk_top];

	ctx->stack[ctx->stk_top].state = state;
	ctx->stack[ctx->stk_top].start = start;
	ctx->stack[ctx->stk_top].end = end;
	ctx->stack[ctx->stk_top].element = e;
	++ctx->stk_top;

	return ret;
}

/*
* Pops off the top element in the stack.
* Return: top element in the stack, NULL if the stack is empty.
* @ctx: the context.
*/
inline static struct xson_lex_element * xson_stack_pop(struct xson_context * ctx) {
	if (ctx->stk_top <= 0) {
		return NULL;
	}
	return &ctx->stack[--ctx->stk_top];
}

/*
* Get the @nth lex element from the top.
* Return: the @nth element from the top, NULL if the stack top pointer < @n.
* @ctx: the context.
*/
inline static struct xson_lex_element * xson_stack_get_top_nr(struct xson_context * ctx, int n) {
	if (ctx->stk_top < n) {
		return NULL;
	}
	return &ctx->stack[ctx->stk_top - n];
}

/*
* Get the top lex element in the stack.
* Return: top element in the stack, NULL if the stack is empty.
* @ctx: the context.
*/
inline static struct xson_lex_element * xson_stack_get_top(struct xson_context * ctx) {
	return xson_stack_get_top_nr(ctx, 1);
}

/*
* Pops off elements in the stack until a lex element with the state equals to @state.
* Return: the element with the specific state, NULL if not exist.
* @ctx: the context.
* @state: the given state.
*/
inline static struct xson_lex_element * xson_stack_pop_until(struct xson_context * ctx,
											   int state) {
	while (ctx->stk_top > 0 && xson_stack_get_top(ctx)->state != state)
		xson_stack_pop(ctx);

	return xson_stack_get_top(ctx);
}

/*
* Get the state of the top lex element in the stack.
* Return: top element's state.
* @ctx: the context.
*/
inline static int xson_stack_top_state(struct xson_context * ctx) {
	return ctx->stack[ctx->stk_top - 1].state;
}



/* defs make code slightly more pretty */

/*
*  {
* [ {
* : {
* , {
*/
#define XSON_LEFT_BRACE_COND (top_state != LEX_STATE_EMPTY &&\
							  top_state != LEX_STATE_LEFT_SQBRACKT &&\
							  top_state != LEX_STATE_COLON &&\
							  top_state != LEX_STATE_COMMA)
/*
* [
* [[
* :[
* ,[
*/
#define XSON_LEFT_SQBRACKT_COND (top_state != LEX_STATE_EMPTY &&\
								 top_state != LEX_STATE_LEFT_SQBRACKT &&\
								 top_state != LEX_STATE_COLON &&\
								 top_state != LEX_STATE_COMMA)
/*
* :"
* ,"
*  {"
* ["
*/
#define XSON_LEFT_DQUOTE_COND (top_state != LEX_STATE_COLON &&\
							   top_state != LEX_STATE_COMMA &&\
							   top_state != LEX_STATE_LEFT_BRACE &&\
							   top_state != LEX_STATE_LEFT_SQBRACKT)
/*
* key:
*/
#define XSON_COLON_COND (top_state != LEX_STATE_STRING)					

/*
*  {...},
* [...],
* "",
* number,
* key:value,
*/
#define XSON_COMMA_COND (top_state != LEX_STATE_OBJECT &&\
						 top_state != LEX_STATE_ARRAY &&\
						 top_state != LEX_STATE_STRING &&\
						 top_state != LEX_STATE_NUMBER &&\
						 top_state != LEX_STATE_PAIR)
/*
* ,number
* :numbuer
* [number
*/
#define XSON_NUMBER_COND (top_state != LEX_STATE_COMMA &&\
						  top_state != LEX_STATE_COLON &&\
						  top_state != LEX_STATE_LEFT_SQBRACKT)
/*
* ,key:value
*  {key:value
*/
#define XSON_PAIR_COND (top_state != LEX_STATE_COMMA &&\
						top_state != LEX_STATE_LEFT_BRACE)		  

/*
*  {}
* key:value}
*/
#define XSON_RIGHT_BRACE (top_state != LEX_STATE_LEFT_BRACE &&\
						  top_state != LEX_STATE_PAIR)

/*
* []
* number]
*  {...}]
* "..."]
* [...]]
*/
#define XSON_RIGHT_SQBRACKT (top_state != LEX_STATE_LEFT_SQBRACKT &&\
						  	 top_state != LEX_STATE_NUMBER &&\
							 top_state != LEX_STATE_OBJECT &&\
							 top_state != LEX_STATE_STRING &&\
							 top_state != LEX_STATE_ARRAY)

inline int xson_right_sqbrackt_chk(int top_state) {
	return top_state != LEX_STATE_LEFT_SQBRACKT && 	// []
		   top_state != LEX_STATE_NUMBER &&			// ,number]
		   top_state != LEX_STATE_OBJECT &&			// , {...}]
		   top_state != LEX_STATE_STRING;			// ,"..."]
}
#define INVALID_JSON_CHK(char_name) if (XSON_##char_name##_COND) return XSON_RESULT_INVALID_JSON;


static int xson_handle_open_object(struct xson_context * ctx, struct xson_element ** parent, char **cp) {
	int 					top_state;
	struct xson_element 	*e = NULL;
	struct xson_lex_element *lex = NULL;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_LEFT_BRACE_COND) return XSON_RESULT_INVALID_JSON;
	e = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (e == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	xson_element_initialize(ctx, e, ELE_TYPE_OBJECT);
	lex = xson_stack_push(ctx, LEX_STATE_LEFT_BRACE, *cp, *cp, e);
	if (lex == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	if (e->ops->initialize(e, lex) == XSON_RESULT_OOM) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	
	/*  {"...": {}} situation. */
	if ((*parent)->type != ELE_TYPE_OBJECT) {
		e->parent = *parent;
		(*parent)->ops->add_child(*parent, e);
	}
		
	*parent = e;

	return XSON_RESULT_SUCCESS;
}

static int xson_handle_open_array(struct xson_context * ctx, struct xson_element ** parent, char **cp) {
	int 					top_state;
	struct xson_element 	*e = NULL;
	struct xson_lex_element *lex = NULL;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_LEFT_SQBRACKT_COND) return XSON_RESULT_INVALID_JSON;
	e = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (e == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	xson_element_initialize(ctx, e, ELE_TYPE_ARRAY);
	lex = xson_stack_push(ctx, LEX_STATE_LEFT_SQBRACKT, *cp, *cp, e);
	if (lex == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	if (e->ops->initialize(e, lex) == XSON_RESULT_OOM) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	e->parent = *parent;
	/*  {"...":[]} situation. */
	if ((*parent)->type != ELE_TYPE_OBJECT)
		(*parent)->ops->add_child(*parent, e);
	*parent = e;

	return XSON_RESULT_SUCCESS;
}

static int xson_handle_pair(struct xson_context * ctx, struct xson_element ** parent) {
	int 					top_state;
	struct xson_element 	*key = NULL;
	struct xson_element 	*value = NULL;
	struct xson_element 	*pair = NULL;
	struct xson_lex_element *lex = NULL;
	struct xson_lex_element *key_lex = NULL;
	struct xson_lex_element *value_lex = NULL;
	struct xson_pair 		*pair_internal = NULL;
	

	value_lex = xson_stack_pop(ctx);
	xson_stack_pop(ctx);// :
	key_lex = xson_stack_pop(ctx);

	top_state = xson_stack_top_state(ctx);
	if (XSON_PAIR_COND || (*parent)->type != ELE_TYPE_OBJECT)
		return XSON_RESULT_INVALID_JSON;

	key = key_lex->element;
	value = value_lex->element;
	if (key->type != ELE_TYPE_STRING) {
		return XSON_RESULT_INVALID_JSON;
	}
	pair = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (pair == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	xson_element_initialize(ctx, pair, ELE_TYPE_PAIR);
	lex = xson_stack_push(ctx, LEX_STATE_PAIR, key_lex->start, value_lex->end, pair);
	if (lex == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	if (pair->ops->initialize(pair, lex) == XSON_RESULT_OOM) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	pair_internal = pair->internal;
	pair_internal->key = key;
	pair_internal->value = value;
	key->parent = pair;
	value->parent = pair;
	pair->parent = *parent;

	return (*parent)->ops->add_child(*parent, pair);
}

static int xson_handle_string(struct xson_context * ctx, struct xson_element ** parent, char **cp) {
	int 					top_state;
	char 					*start, *end;
	struct xson_element 	*e = NULL;
	struct xson_lex_element *lex = NULL;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_LEFT_DQUOTE_COND) return XSON_RESULT_INVALID_JSON;
	e = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (e == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	struct fsm_string fsms;
	start = *cp + 1;
	if (fsm_string_run(&fsms, cp) == -1) {
		return XSON_RESULT_INVALID_JSON;
	}
	end = *cp - 1;

	xson_element_initialize(ctx, e, ELE_TYPE_STRING);
	lex = xson_stack_push(ctx, LEX_STATE_STRING, start, end, e);
	if (lex == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	if (e->ops->initialize(e, lex) == XSON_RESULT_OOM) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	//got a pair
	if (top_state == LEX_STATE_COLON) {
		return xson_handle_pair(ctx, parent);
	}else {
		/*  {"..." situation */
		if ((*parent)->type == ELE_TYPE_OBJECT)return XSON_RESULT_SUCCESS;

		return (*parent)->ops->add_child(*parent, e);
	}
}

static int xson_handle_number(struct xson_context * ctx, struct xson_element ** parent, char **cp) {
	int 					top_state;
	char 					*start, *end;
	struct xson_element 	*e = NULL;
	struct xson_lex_element *lex = NULL;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_NUMBER_COND) return XSON_RESULT_INVALID_JSON;
	e = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (e == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	struct fsm_number fsmn;
	start = *cp;
	if (fsm_number_run(&fsmn, cp) == XSON_RESULT_INVALID_JSON) {
		return XSON_RESULT_INVALID_JSON;
	}
	end = *cp;

	xson_element_initialize(ctx, e, ELE_TYPE_NUMBER);
	lex = xson_stack_push(ctx, LEX_STATE_NUMBER, start, end, e);
	if (lex == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	if (e->ops->initialize(e, lex) == XSON_RESULT_OOM) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	//got a pair
	if (top_state == LEX_STATE_COLON) {
		return xson_handle_pair(ctx, parent);
	}else {
		return (*parent)->ops->add_child(*parent, e);
	}
}

static int xson_handle_comma(struct xson_context * ctx, struct xson_element ** parent, char **cp) {
	int 				top_state;
	struct xson_element *e;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_COMMA_COND) return XSON_RESULT_INVALID_JSON;
	e = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (e == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}

	if (xson_stack_push(ctx, LEX_STATE_COMMA, *cp, *cp, e) == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	e->parent = *parent;
	return XSON_RESULT_SUCCESS;
}

static int xson_handle_colon(struct xson_context * ctx, struct xson_element ** parent, char **cp) {
	int 				top_state;
	struct xson_element *e;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_COMMA_COND) return XSON_RESULT_INVALID_JSON;
	e = xson_malloc(&ctx->pool, sizeof(struct xson_element));
	if (e == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	
	if (xson_stack_push(ctx, LEX_STATE_COLON, *cp, *cp, e) == NULL) {
		assert(0);
		return XSON_RESULT_OOM;
	}
	e->parent = *parent;

	return XSON_RESULT_SUCCESS;
}


static int xson_handle_closed_object(struct xson_context * ctx,
							 struct xson_element ** parent,
							 char **cp) {
	int 					top_state;
	struct xson_lex_element *lex = NULL, *lex_under = NULL;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_RIGHT_BRACE || (*parent)->type != ELE_TYPE_OBJECT)
		return XSON_RESULT_INVALID_JSON;
	lex = xson_stack_pop_until(ctx, LEX_STATE_LEFT_BRACE);
	if (lex == NULL)return XSON_RESULT_INVALID_JSON;
	lex_under = xson_stack_get_top_nr(ctx, 2);
	//turn it into a object
	lex->state = LEX_STATE_OBJECT;

	//we got a pair forming up
	if (lex_under && lex_under->state == LEX_STATE_COLON) {
		*parent = lex_under->element->parent;
		return xson_handle_pair(ctx, parent);
	}else {
		*parent = (*parent)->parent;
		return XSON_RESULT_SUCCESS;
	}
}

static int xson_handle_closed_array(struct xson_context * ctx,
							 struct xson_element ** parent,
							 char **cp) {
	int 					top_state;
	struct xson_lex_element *lex = NULL, *lex_under = NULL;
	

	top_state = xson_stack_top_state(ctx);
	if (XSON_RIGHT_SQBRACKT || (*parent)->type != ELE_TYPE_ARRAY)
		  return XSON_RESULT_INVALID_JSON;
	lex = xson_stack_pop_until(ctx, LEX_STATE_LEFT_SQBRACKT);
	if (lex == NULL)return XSON_RESULT_INVALID_JSON;
	lex_under = xson_stack_get_top_nr(ctx, 2);
	//turn it into a array
	lex->state = LEX_STATE_ARRAY;

	//we got a pair forming up
	if (lex_under && lex_under->state == LEX_STATE_COLON) {
		*parent = lex_under->element->parent;
		return xson_handle_pair(ctx, parent);
	}else {
		*parent = (*parent)->parent;
		return XSON_RESULT_SUCCESS;
	}
}

int xson_parse(struct xson_context * ctx, struct xson_element ** out) {
	char * cp = ctx->str_buf;
	int ret;
	struct xson_element * parent = NULL;
	assert(ctx != NULL);

	parent = ctx->root;

	while (*cp) {
		if (*cp == '{') {/* open object */
			ret = xson_handle_open_object(ctx, &parent, &cp);
		}else if (*cp == '[') {/* open array */
			ret = xson_handle_open_array(ctx, &parent, &cp);
		}else if (*cp == '\"') {/* string */
			ret = xson_handle_string(ctx, &parent, &cp);
		}else if (xson_is_number_start(*cp)) {/* number */
			ret = xson_handle_number(ctx, &parent, &cp);
		}else if (*cp == ':') {/* colon */
			ret = xson_handle_colon(ctx, &parent, &cp);
		}else if (*cp == ',') {/* comma */
			ret = xson_handle_comma(ctx, &parent, &cp);
		}else if (*cp == '}') {/* closed object */
			ret = xson_handle_closed_object(ctx, &parent, &cp);
		}else if (*cp == ']') {/* closed array */
			ret = xson_handle_closed_array(ctx, &parent, &cp);
		}else if (xson_is_blanks(*cp)) {
			ret = XSON_RESULT_SUCCESS;
		}else {
			ret = XSON_RESULT_INVALID_JSON;
		}

		if (ret == XSON_RESULT_INVALID_JSON)goto invalid_json;
		else if (ret == XSON_RESULT_OOM)goto oom;
		else if (ret == XSON_RESULT_ERROR)goto error;
		++cp;
	}

	if (parent != ctx->root || ctx->stk_top > 2 ||
	   (xson_stack_top_state(ctx) != LEX_STATE_OBJECT &&
	   xson_stack_top_state(ctx) != LEX_STATE_ARRAY))goto error;

	*out = ctx->root;

	return 0;

invalid_json:
	if (cp + 10 < cp + ctx->str_len) {
		cp[10] = '\0';
	}
	if (cp - 10 >= ctx->str_buf) {
		cp -= 10;
	}else {
		cp = ctx->str_buf;
	}
	printf("xson parser: eek, invalid json string near '%s' !\n", cp);
	return XSON_RESULT_INVALID_JSON;
oom:
	printf("xson parser: out of memory!\n");
	return XSON_RESULT_OOM;
error:
	printf("xson parser: error!\n");
	return XSON_RESULT_ERROR;

}

/*
* Clean and free up the context.
* @ctx: the context being destroyed.
*/
void xson_destroy(struct xson_context * ctx) {
	assert(ctx != NULL);

	if (ctx->str_buf) {
		free(ctx->str_buf);
		ctx->str_buf = NULL;
		ctx->str_len = 0;
	}
	if (ctx->stack) {
		free(ctx->stack);
		ctx->stack = NULL;
		ctx->stk_len = 0;
	}
	ctx->root->ops->destroy(ctx->root);
	xmpool_destroy(&ctx->pool);
}

void xson_print(struct xson_context * ctx, int indent) {
	assert(ctx != NULL);
	assert(ctx->root != NULL && ctx->root->type == ELE_TYPE_ROOT);
	ctx->root->ops->print(ctx->root, 0, indent, 0);
}