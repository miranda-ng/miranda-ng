/*
 * Copyright (c) 2002-2004, Jon Travis
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"

char *ekhtml_parse_special(ekhtml_parser_t *parser, void **state_data,
			   const char *curp, const char *endp, 
			   int *baddata)
{
    const char *workp;
    int *offset = &parser->state.offset;
    
    assert(*curp == '<' && *(curp + 1) == '!');
    
    if(*state_data == NULL){/* Only called the first time the tag is started */
        *offset = 2;
        *state_data = (void *)1;  /* Assign it any non-NULL value */
    }
    
    for(workp=curp + *offset;workp != endp; workp++)
        if(*workp == '<' || *workp == '>')  
            break;
    
    if(workp == endp){
        /* No end of tag found yet, save state */
        *offset = endp - curp;
        return NULL;  
    }
    
    if(parser->datacb){
        ekhtml_string_t str;

        str.str = curp;
        str.len = workp - curp + 1;
        parser->datacb(parser->cbdata, &str);
    }
    
    *state_data = NULL;
    if(*workp == '<')  /* Malformed HTML */
        return (char *)workp;
    else
        return (char *)workp + 1;
}
