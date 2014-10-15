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
#ifndef FSM_STRING_H_
#define FSM_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum string_state {
	STRING_STATE_START,
	STRING_STATE_UNICODE_CHAR,		/* any unicode character */
	STRING_STATE_RSOLIDUS, 			/* \  */
	STRING_STATE_RS_ESCAPE_CHARS,	/* \x */
	STRING_STATE_RS_U,				/* \u */
	STRING_STATE_RS_U_D1,			/* \ux */
	STRING_STATE_RS_U_D2,			/* \uxx */
	STRING_STATE_RS_U_D3,			/* \uxxx */
	STRING_STATE_RS_U_D4,			/* \uxxxx */
	STRING_STATE_INVALID,
	STRING_STATE_END
}string_state;
typedef struct fsm_string {
	string_state state;
}fsm_string;

/*
* Run the string in the state machine until succeeded or a invalid state has been reached.
* After calling this function, @cp points to wherever the state machine stops.
* Return: 0 on success, -1 on failure.
* @fsms: the state machine to be run upon.
* @cp: the input pointer.
*/
int fsm_string_run(struct fsm_string * fsms, char ** cp);

#ifdef __cplusplus
}
#endif
#endif