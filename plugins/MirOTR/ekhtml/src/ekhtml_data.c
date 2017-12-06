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

#include <string.h>
#include <assert.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"

char *ekhtml_parse_data(ekhtml_parser_t *parser, const char *curp, 
			const char *endp, int curstate)
{
    const char *resp, *startp = curp;
    
    /* If we are in the data state, we can absorb everything up to a 
       '<' sign */
    
    if(curstate == EKHTML_STATE_BADDATA){
        /* This state signifies that there was some bad-data involved, 
           skip over the first '<' sign, and take it as raw data       */
        assert(*curp == '<');
        startp++;
    }
    resp = memchr(startp, '<', endp - startp);
    resp = resp ? resp : endp;
    if(parser->datacb){
        ekhtml_string_t str;

        str.str = curp;
        str.len = resp - curp;
        parser->datacb(parser->cbdata, &str);
    }
    return (char *)resp;
}

