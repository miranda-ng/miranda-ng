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

#ifndef EKHTML_PRIVATE_DOT_H
#define EKHTML_PRIVATE_DOT_H

#ifndef EKHTML_USE_PRIVATE
#error This file should only be used by the EKHTML library
#endif

#include <stdio.h>

#include "ekhtml_config.h"
#include "hash.h"

/* 
 *  Container structures -- since a void * is not guaranteed to store 
 *  the function pointer correctly, we make a small container for them 
 */

typedef struct {
    ekhtml_starttag_cb_t startfunc;
    ekhtml_endtag_cb_t   endfunc;
} ekhtml_tag_container;

/*
 * Processing state information
 */

#define EKHTML_STATE_NONE      0  /* No state - figure it out      */
#define EKHTML_STATE_INDATA    1  /* In regular data               */
#define EKHTML_STATE_BADDATA   2  /* In some kind of bad data .. I.e
				     an invalid tag.  The first char
				     in the data is a '<' we must skip */
#define EKHTML_STATE_STARTTAG  3  /* In a start tag                */
#define EKHTML_STATE_ENDTAG    4  /* In an end tag                 */
#define EKHTML_STATE_SPECIAL   5  /* Special tag: <!FOO BAR BAZ    */
#define EKHTML_STATE_COMMENT   6  /* <!--Comment tag, yo! -->      */

#define EKHTML_CHAR_TAGNAME      (1 << 0)  /* AFTER first letter of tag  */
#define EKHTML_CHAR_WHITESPACE   (1 << 1)  /* Whitespace, obviously      */
#define EKHTML_CHAR_BEGATTRNAME  (1 << 2)  /* Valid 1st letter of attr   */
#define EKHTML_CHAR_ATTRNAME     (1 << 3)  /* AFTER first letter of attr */
#define EKHTML_CHAR_ATTRVALUE    (1 << 4)  /* Valid attr value chars     */


#define EKHTML_CHARMAP_TYPE      unsigned int
#define EKHTML_CHARMAP_TYPE_S    "unsigned int"
#define EKHTML_CHARMAP_LEN       (sizeof(EKHTML_CHARMAP_TYPE))

typedef struct {
    int                tagend;  /* Offset to the end of the tag name         */
    int                mode;    /* One of EKHTML_STMODE_*                    */
    ekhtml_attr_t     *attrs;   /* Already processed attributes              */
    ekhtml_attr_t     *curattr; /* Attribute currently being processed       */
    char               quote;   /* If the value is quoted, this is the char  */
} ekhtml_starttag_state;

typedef struct {
    int dashes;                 /* # of dashes in a row found                */
    int lastdash;               /* Offset of last dash                       */
} ekhtml_comment_state;

typedef struct {
    int lastchar;     /* Offset of last non-witespace char (offset from '<') */
} ekhtml_endtag_state;

struct ekhtml_parser_t {
    ekhtml_data_cb_t      datacb;      /* Callback when data is read         */
    hash_t               *startendcb;  /* Hash of start & end tag callbacks  */
    void                 *cbdata;      /* Data to pass into all callbacks    */
    ekhtml_starttag_cb_t  startcb_unk; /* Unknown starttag callback          */
    ekhtml_endtag_cb_t    endcb_unk;   /* Unknown endtag callback            */
    ekhtml_data_cb_t      commentcb;   /* Comment callback                   */
    
    char                 *buf;         /* malloced buffer holding parse data */
    size_t                nalloced;    /* # of bytes alloced in 'buf'        */
    size_t                nbuf;        /* # of bytes used in 'buf'           */
    
    /* The next two vars are only used in the starttag portion               */
    ekhtml_starttag_state startstate;  /* State info in start tags           */
    ekhtml_attr_t         *freeattrs;  /* Attribute structures which callers 
                                          can allocate and release at will   */
    ekhtml_endtag_state   endstate;    /* State info in end tags             */
    ekhtml_comment_state  commentstate;/* Only used in comment state         */
    
    struct {
        int state;          /* One of EKHTML_STATE_*                         */
        void *state_data;   /* State dependent data pointer                  */
        int offset;         /* State dependent offset information            */
    } state;
};

extern char *ekhtml_parse_comment(ekhtml_parser_t *, void **, const char *, 
				  const char *, int *baddata);
extern char *ekhtml_parse_special(ekhtml_parser_t *, void **, const char *, 
				  const char *, int *);
extern char *ekhtml_parse_starttag(ekhtml_parser_t *, void **, char *,
				   char *, int *);
extern char *ekhtml_parse_endtag(ekhtml_parser_t *, void **, char *,
				 char *, int *);
extern char *ekhtml_parse_data(ekhtml_parser_t *, const char *, const char *,
			       int);

extern void ekhtml_parser_starttag_cleanup(ekhtml_parser_t *);
extern char *ekhtml_make_upperstr(char *, int);

/*
 * find_notcharsmap:  Find a character in a buffer which contains a false
 *                    value in the character map 'map'
 *
 * Arguments:         buf     = Buffer to search through
 *                    len     = Length of `buf`
 *                    charmap = Character map (i.e. one from ekhtml_tables.h)
 *                    mask    = Mask to compare against each value of the map.
 *                              If the char masked against 'mask' is zero,
 *                              then the comparison is 'false'.
 *
 * Return values:     Returns buf + len if a character was not found, else
 *                    a pointer to the invalid character.
 */

static inline
char *ekhtml_find_notcharsmap(const char *buf, int len, 
			      const EKHTML_CHARMAP_TYPE *charmap,
			      EKHTML_CHARMAP_TYPE mask)
{
    const char *endp = buf + len;
    
    for(;buf<endp;buf++)
        if(!(charmap[(unsigned char )*buf] & mask))
            break;
    
    return (char *)((buf == endp) ? endp : buf);
}


#endif

