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
#include "xson/fsm_number.h"

inline static int is_digit_1_9(char c) {
	return c >= '1' && c <= '9';
}
inline static int is_digit(char c) {
	return c >= '0' && c <= '9';
}
inline static int is_sign(char c) {
	return c == '+' || c == '-';
}
inline static int is_e(char c) {
	return c == 'E' || c == 'e';
}
inline static int is_number_char(char c) {
	return is_digit(c) || is_sign(c) || is_e(c);
}


int fsm_number_run(struct fsm_number * fsms, char ** cp) {
	fsms->state = NUMBER_STATE_START;
again:
	switch (fsms->state) {
		case NUMBER_STATE_START:
			if (**cp == '-')fsms->state = NUMBER_STATE_NEG;
			else if (**cp == '0' )fsms->state = NUMBER_STATE_ZERO;
			else if (is_digit_1_9(**cp))fsms->state = NUMBER_STATE_FIRST_DIGIT;
			else fsms->state = NUMBER_STATE_INVALID;
			break;
		case NUMBER_STATE_NEG:
			if (**cp == '0')fsms->state = NUMBER_STATE_ZERO;
			else if (is_digit_1_9(**cp))fsms->state = NUMBER_STATE_FIRST_DIGIT;
			else fsms->state = NUMBER_STATE_INVALID;
			break;
		case NUMBER_STATE_ZERO:
			if (**cp == '.')fsms->state = NUMBER_STATE_DOT;
			else if (is_e(**cp))fsms->state = NUMBER_STATE_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		case NUMBER_STATE_FIRST_DIGIT:
			if (**cp == '.')fsms->state = NUMBER_STATE_DOT;
			else if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_INT;
			else if (is_e(**cp))fsms->state = NUMBER_STATE_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		case NUMBER_STATE_DIGITS_INT:
			if (**cp == '.')fsms->state = NUMBER_STATE_DOT;
			else if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_INT;
			else if (is_e(**cp))fsms->state = NUMBER_STATE_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		case NUMBER_STATE_DOT:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_FRAC;
			else fsms->state = NUMBER_STATE_INVALID;
			break;
		case NUMBER_STATE_DIGITS_FRAC:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_FRAC;
			else if (is_e(**cp))fsms->state = NUMBER_STATE_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		case NUMBER_STATE_E:
			if (is_sign(**cp))fsms->state = NUMBER_STATE_SIGN_AFTER_E;
			else if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_AFTER_E;
			else fsms->state = NUMBER_STATE_INVALID;
			break;
		case NUMBER_STATE_SIGN_AFTER_E:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_AFTER_E;
			else fsms->state = NUMBER_STATE_INVALID;
			break;
		case NUMBER_STATE_DIGITS_AFTER_E:
			if (is_digit(**cp))fsms->state = NUMBER_STATE_DIGITS_AFTER_E;
			else fsms->state = NUMBER_STATE_END;
			break;
		default:
			fsms->state = NUMBER_STATE_INVALID;
	}
	if (fsms->state != NUMBER_STATE_INVALID &&
	   fsms->state != NUMBER_STATE_END) {
	   	++*cp;
		goto again;
	}
	--*cp;
	if (fsms->state != NUMBER_STATE_END)return XSON_RESULT_INVALID_JSON;
	else return ELE_TYPE_NUMBER;
}