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

#ifndef FSM_BOOL_H_
#define FSM_BOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bool_state {
	BOOL_STATE_START,
	BOOL_STATE_T,					/* t      */
	BOOL_STATE_TR,					/* tr     */
	BOOL_STATE_TRU,					/* tru    */
	BOOL_STATE_F,					/* f      */
	BOOL_STATE_FA,					/* fa     */
	BOOL_STATE_FAL,					/* fal    */
	BOOL_STATE_FALS,				/* fals   */
	BOOL_STATE_INVALID,
	BOOL_STATE_END
}bool_state;

typedef struct fsm_bool {
	enum bool_state state;
} fsm_bool;

/*
* Run the string in the state machine until succeeded or a invalid state has been reached.
* After calling this function, @cp points to wherever the state machine stops.
* Return: XSON_RESULT_SUCCESS on success, XSON_RESULT_INVALID_JSON on failure.
* @fsms: the state machine to be run upon.
* @cp: the input pointer.
*/
int fsm_bool_run(struct fsm_bool * fsms, char ** cp);

#ifdef __cplusplus
}
#endif
#endif