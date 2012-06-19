/*
 * Copyright (c) 2002, Jon Travis
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

/*
 * ekhtml_comment.c:  Comment tag processor for El-Kabong. 
 *
 * The comment processor is a pretty simple piece of machinery.  It 
 * relies that the first 4 characters are '<!--'.  It then searches
 * for two adjacent dashes '--' followed by optional whitespace,
 * followed by a '>'.  
 */

#include <string.h>
#include <assert.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"

char *ekhtml_parse_comment(ekhtml_parser_t *parser, void **state_data,
			   const char *curp, const char *endp, 
			   int *baddata)
{
    ekhtml_comment_state *comstate = *state_data;
    int *offset = &parser->state.offset;
    const char *workp;
    
    assert(*curp == '<' && *(curp + 1) == '!' && *(curp + 2) == '-');
    assert(*(curp + 3) == '-' && endp - curp >= 4);

    if(comstate == NULL){  /* Only called the first time the tag is started */
        comstate           = &parser->commentstate;
        comstate->dashes   = 0;
        comstate->lastdash = 0;
        *state_data        = comstate;
        *offset            = sizeof("<!--") - 1;
    }
    
    workp = curp + *offset;
    while(workp != endp){
        if(comstate->dashes == 0){  /* Still on the quest for the double dash*/
            /* XXX -- Searching for '--' could be faster, doing 
               multibyte searching, or something similar */
            for(; workp < endp - 1; workp += 2){
                if(*workp == '-')
                    break;
            }

            if(!(workp < endp - 1)){
                *offset = endp - 1 - curp;
                return NULL;
            }

            if((*(workp - 1) == '-') && 
               (workp - curp) > (sizeof("<!--") - 1))
            {
                comstate->lastdash = workp - 1 - curp;
                comstate->dashes = 1;
            } else if(*(workp + 1) == '-'){
                comstate->lastdash = workp - curp;
                comstate->dashes = 1;
            } else {
                /* Else, a lone dash -- how sad, lonely dash.. ;-) */
            }
            workp++;
            continue;
        }
    
        /* At this point we have the double dash.  Search through whitespace */
        workp = ekhtml_find_notcharsmap(workp, endp - workp, EKCMap_CharMap,
                                        EKHTML_CHAR_WHITESPACE);
        if(workp == endp){/* Reached end of the data without finding the '>' */
            *offset = workp - curp;
            return NULL;  
        }
        
        if(*workp == '>'){
            if(parser->commentcb){
                ekhtml_string_t str;

                str.str = curp + 4;
                str.len = comstate->lastdash - 4;
                parser->commentcb(parser->cbdata, &str);
            }
            *state_data = NULL;
            return (char *)workp + 1;
        } else {
            comstate->dashes = 0;
        }
    }
    
    *offset = workp - curp;
    return NULL;  /* Not done yet */
}
