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
 * ekhtml_mktables:  A small utility for generating tables of valid characters
 *                   for different parts of the HTML parsing, such as tags,
 *                   whitespace, etc.
 *
 *                   Each table is described by a function which returns 1
 *                   if the character should be in the table, else 0 
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include "ekhtml_config.h"
#include "ekhtml.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"


/* valid_tagname:  Character map for a tagname AFTER the first letter */
static EKHTML_CHARMAP_TYPE valid_tagname(char in){
    if(in == '-' || in == '.' || isdigit(in) || isalpha(in))
        return 1;
    return 0;
}

static EKHTML_CHARMAP_TYPE valid_whitespace(char in){
    return isspace(in) ? 1 : 0;
}

/* attribute name AFTER the first character */
static EKHTML_CHARMAP_TYPE valid_attrname(char in){
    if(in == '_' || valid_tagname(in))
        return 1;
    return 0;
}

/* attribute value */
static EKHTML_CHARMAP_TYPE valid_attrvalue(char in){
    if(valid_attrname(in) || 
       in == '/' || in == ':' || in == '+' || in == '*' ||
       in == '%' || in == '?' || in == '!' || in == '&' ||
       in == '(' || in == ')' || in == '#' || in == '=' ||
       in == '~' || in == ']' || in == '*' || in == '@' ||
       in == '$' || in == '_')
        return 1;
    return 0;
}

static EKHTML_CHARMAP_TYPE valid_begattrname(char in){
    return (isalpha(in) || in == '_') ? 1 : 0;
}

static EKHTML_CHARMAP_TYPE ekhtml_state(char in){
    if(in == '/')
        return EKHTML_STATE_ENDTAG;
    if(isalpha(in))
        return EKHTML_STATE_STARTTAG;
    if(in == '!')
        return EKHTML_STATE_NONE;  /* Must be determined by caller */
    return EKHTML_STATE_BADDATA;
}

static EKHTML_CHARMAP_TYPE charmap_values(char in){
    EKHTML_CHARMAP_TYPE res = 0;
    
    if(valid_tagname(in))
	res |= EKHTML_CHAR_TAGNAME;
    if(valid_whitespace(in))
	res |= EKHTML_CHAR_WHITESPACE;
    if(valid_begattrname(in))
	res |= EKHTML_CHAR_BEGATTRNAME;
    if(valid_attrname(in))
	res |= EKHTML_CHAR_ATTRNAME;
    if(valid_attrvalue(in))
	res |= EKHTML_CHAR_ATTRVALUE;
    return res;
}

#define EKHTML_STRINGIFY(x) #x

static void print_charmap(char *name, EKHTML_CHARMAP_TYPE (*cmap_func)(char)){
    int ch;
    char sbuf[256];
    
    sprintf_s(sbuf, 256, "0x%%0%dx ", EKHTML_CHARMAP_LEN * 2);
    printf("#ifdef EKHTML_USE_TABLES\n");
    printf("const %s %s[256] = {\n", EKHTML_CHARMAP_TYPE_S, name);
    for(ch=0; ch < 256; ch++){
        printf(sbuf, cmap_func((char)ch));
        if(isgraph(ch))
            printf("/*  '%c' */", ch);
        else
            printf("/* 0x%.2x */", ch);
        printf(",  ");
        if(!((ch + 1) % 4))
            printf("\n");
    }
    printf("};\n\n");
    printf("#else\n");
    printf("extern const %s %s[256];\n", EKHTML_CHARMAP_TYPE_S, name);
    printf("#endif\n");
}

int main(int argc, char *argv[]){
    printf("#ifndef EKHTML_MKTABLES_DOT_H\n");
    printf("#define EKHTML_MKTABLES_DOT_H\n");
    
    print_charmap("EKCMap_CharMap",     charmap_values);
    print_charmap("EKCMap_EKState",     ekhtml_state);
    
    printf("\n#endif\n");
    return 0;
}
