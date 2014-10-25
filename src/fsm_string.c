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
#include "xson/fsm_string.h"

inline static int fsm_string_is_hex_char(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int fsm_string_run(struct fsm_string * fsms, char ** cp) {
    fsms->state = STRING_STATE_START;
    /*
    * Ugly switch/cases implementation of finite state machine.
    */
again:
    if (*(++*cp) == 0) {
        fsms->state = STRING_STATE_INVALID;
        goto out;
    }
    switch (fsms->state) {
        case STRING_STATE_START:
            if (**cp == '\"')fsms->state = STRING_STATE_END;
            else if (**cp == '\\')fsms->state = STRING_STATE_RSOLIDUS;
            else fsms->state = STRING_STATE_UNICODE_CHAR;
            break;
        case STRING_STATE_UNICODE_CHAR:
            if (**cp == '\"')fsms->state = STRING_STATE_END;
            else if (**cp == '\\')fsms->state = STRING_STATE_RSOLIDUS;
            else fsms->state = STRING_STATE_UNICODE_CHAR;
            break;
        case STRING_STATE_RSOLIDUS:
            if (**cp == '\"' || **cp == '\\' || **cp == '/' || **cp == 'b' || 
               **cp == 'f' || **cp == 'n' || **cp == 'r' || **cp == 't')
                fsms->state = STRING_STATE_RS_ESCAPE_CHARS;
            else if (**cp == 'u')
                fsms->state = STRING_STATE_RS_U;
            else
                fsms->state = STRING_STATE_INVALID;
            break;
        case STRING_STATE_RS_ESCAPE_CHARS:
            if (**cp == '\"')fsms->state = STRING_STATE_END;
            else if (**cp == '\\')fsms->state = STRING_STATE_RSOLIDUS;
            else fsms->state = STRING_STATE_UNICODE_CHAR;
            break;
        case STRING_STATE_RS_U:
            if (fsm_string_is_hex_char(**cp))fsms->state = STRING_STATE_RS_U_D1;
            else fsms->state = STRING_STATE_INVALID;
            break;
        case STRING_STATE_RS_U_D1:
            if (fsm_string_is_hex_char(**cp))fsms->state = STRING_STATE_RS_U_D2;
            else fsms->state = STRING_STATE_INVALID;
            break;
        case STRING_STATE_RS_U_D2:
            if (fsm_string_is_hex_char(**cp))fsms->state = STRING_STATE_RS_U_D3;
            else fsms->state = STRING_STATE_INVALID;
            break;
        case STRING_STATE_RS_U_D3:
            if (fsm_string_is_hex_char(**cp))fsms->state = STRING_STATE_RS_U_D4;
            else fsms->state = STRING_STATE_INVALID;
            break;
        case STRING_STATE_RS_U_D4:
            if (**cp == '"')fsms->state = STRING_STATE_END;
            else if (**cp == '\\')fsms->state = STRING_STATE_RSOLIDUS;
            else fsms->state = STRING_STATE_UNICODE_CHAR;
            break;
        default:
            fsms->state = STRING_STATE_INVALID;
    }
    if (fsms->state != STRING_STATE_INVALID &&
        fsms->state != STRING_STATE_END)
        goto again;
out:
    return fsms->state == STRING_STATE_END ? XSON_RESULT_SUCCESS : XSON_RESULT_INVALID_JSON;
}