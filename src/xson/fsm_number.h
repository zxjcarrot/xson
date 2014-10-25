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
#ifndef FSM_NUMBER_H_
#define FSM_NUMBER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum number_state {
    NUMBER_STATE_START,
    NUMBER_STATE_NEG,               /* - sign */
    NUMBER_STATE_ZERO,              /* 0.XXX */
    NUMBER_STATE_FIRST_DIGIT,       /* [+-][1-9]... */
    NUMBER_STATE_DIGITS_INT,        /* integral part of the number */ 
    NUMBER_STATE_DIGITS_FRAC,       /* fractional part of the number */
    NUMBER_STATE_DOT,               /* the dot */
    NUMBER_STATE_E,                 /* [eE] */
    NUMBER_STATE_SIGN_AFTER_E,      /* [eE][+-] */
    NUMBER_STATE_DIGITS_AFTER_E,    /* [eE][+-]([0-9])* */
    NUMBER_STATE_INVALID,
    NUMBER_STATE_END
}number_state;

typedef struct fsm_number {
    enum number_state state;
}fsm_number;

/*
* Run the number in the state machine until succeeded or a invalid state has been reached.
* After calling this function, @cp points to wherever the state machine stops.
* Return: XSON_RESULT_SUCCESS on success, XSON_RESULT_INVALID_JSON on failure.
* @fsms: the state machine to be run upon.
* @cp: the input pointer.
*/
int fsm_number_run(struct fsm_number * fsms, char ** cp);

#ifdef __cplusplus
}
#endif
#endif