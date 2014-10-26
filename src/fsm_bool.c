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
#include "xson/fsm_bool.h"


int fsm_bool_run(struct fsm_bool * fsms, char ** cp) {
	fsms->state = BOOL_STATE_START;
again:
	switch (fsms->state) {
		case BOOL_STATE_START:
			if (**cp == 't')fsms->state = BOOL_STATE_T;
			else if (**cp == 'f' )fsms->state = BOOL_STATE_F;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case BOOL_STATE_T:
			if (**cp == 'r')fsms->state = BOOL_STATE_TR;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case BOOL_STATE_TR:
			if (**cp == 'u')fsms->state = BOOL_STATE_TRU;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case BOOL_STATE_TRU:
			if (**cp == 'e')fsms->state = BOOL_STATE_END;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case BOOL_STATE_F:
			if (**cp == 'a')fsms->state = BOOL_STATE_FA;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case BOOL_STATE_FA:
			if (**cp == '')fsms->state = NUMBER_STATE_DIGITS_FRAC;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case NUMBER_STATE_DIGITS_FRAC:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_FRAC;
			else if (is_e(**cp))fsms->state = NUMBER_STATE_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		case NUMBER_STATE_E:
			if (is_sign(**cp))fsms->state = NUMBER_STATE_SIGN_AFTER_E;
			else if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_AFTER_E;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case NUMBER_STATE_SIGN_AFTER_E:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_AFTER_E;
			else fsms->state = BOOL_STATE_INVALID;
			break;
		case NUMBER_STATE_DIGITS_AFTER_E:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_AFTER_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		default:
			fsms->state = BOOL_STATE_INVALID;
	}
	if (fsms->state != BOOL_STATE_INVALID &&
	    fsms->state != NUMBER_STATE_END) {
	   	++*cp;
		goto again;
	}
	--*cp;
	if (fsms->state != NUMBER_STATE_END)return XSON_RESULT_INVALID_JSON;
	else return XSON_RESULT_SUCCESS;
}