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

/*
 * ekhtml_endtag.c:  Processor for a closing tag '</tag>
 *
 * The endtag processor is a very simple processor with a very small
 * state machine.  Like the other tags, it accomodates for malformated
 * HTML, exchanging a '<' for a '>'.  Whitespace trailing the tagname is
 * ignored, and it might be interesting to note that a tag of </FOO BAR>
 * will make callbacks with a tag of 'FOO BAR'
 */

#include <assert.h>
#include <ctype.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"

static void handle_endtag(ekhtml_parser_t *parser, ekhtml_string_t *str){
    ekhtml_tag_container *container;
    hnode_t *hn;
    
    if((hn = hash_lookup(parser->startendcb, str)) &&
       (container = hnode_get(hn)) && 
       container->endfunc)
    {
        container->endfunc(parser->cbdata, str);
    } else if(parser->endcb_unk)
        parser->endcb_unk(parser->cbdata, str);
}

char *ekhtml_parse_endtag(ekhtml_parser_t *parser, void **state_data,
			  char *curp, char *endp, int *baddata)
{
    const char *workp, *arrowp, *upper_tag;
    ekhtml_endtag_state *endstate = *state_data;
    int taglen, *offset = &parser->state.offset;
    ekhtml_string_t str;
    
    /* Prerequisites for this function are that the first chars are </' 
       and that there are at least 3 bytes of data to work with         */
    assert(*curp == '<' && *(curp + 1) == '/');
    assert(endp - curp >= 3);
    
    if(endstate == NULL){  /* Only called the first time a tag is started */
        const char *secondchar = curp + 2;

        /* Initial check to make sure this isn't some bad tag */
        if(!isalpha(*secondchar)){
            if(*secondchar != '>' && *secondchar != '<'){
                /* Bogus tag */
                *baddata = EKHTML_STATE_BADDATA;
                return (char *)curp;
            } else { /* Might as well handle this case while we are here */
                str.str = "";
                str.len = 0;
                handle_endtag(parser, &str);
                return (char *)(*secondchar == '>' ? secondchar + 1 : 
                                secondchar);
            }
        }
        
        /* Store state, since this is the first time we are state-ifying. */
        endstate = &parser->endstate;
        endstate->lastchar  = 2; 
        *state_data         = endstate;
        *offset             = 2;
    }
    
    workp = curp + *offset;
    /* Search for the close tag, or even malformed HTML */
    for(arrowp=workp; 
        arrowp != endp && *arrowp != '<' && *arrowp != '>';
        arrowp++)
    {
        if(!(EKCMap_CharMap[(unsigned char)*arrowp] & EKHTML_CHAR_WHITESPACE))
            endstate->lastchar = arrowp - curp;
    }
    
    if(arrowp == endp){
        /* Didn't find the end, so return NULL */
        *offset = endp - curp;
        return NULL;
    }

    /* Found the end.  Clear up our state and return the next char that
     * the parser should process
     */
    taglen = endstate->lastchar + 1 - 2;
    upper_tag = ekhtml_make_upperstr(curp + 2, taglen);
    str.str = upper_tag;
    str.len = taglen;
    handle_endtag(parser, &str);
    *state_data = NULL;
    assert(arrowp < endp);
    if(*arrowp == '<'){ /* Malformed HTML */
        return (char *)(arrowp);
    } else {
        return (char *)(arrowp + 1);
    }
}

