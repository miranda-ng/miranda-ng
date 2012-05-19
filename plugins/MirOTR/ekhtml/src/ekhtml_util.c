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

#include <ctype.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"


/*
 * ekhtml_make_upperstr:  Make a new bytestring based on the old one .. 
 *                        only uppercase.  
 *
 * Arguments:             buf  = Buffer containing bytes to 'upper'
 *                        len  = Length of bytes in buf
 *
 * Return values:  Capitalizes the string pointed at by 'buf', and returns
 *                 'buf'
 */

char *ekhtml_make_upperstr(char *buf, int len){
    char *endp = buf + len, *cp;
    
    for(cp = buf; cp < endp; cp++)
        *cp = toupper(*cp);
    return buf;
}


