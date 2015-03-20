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
 * ekhtml: The El-Kabong HTML parser
 *         by Jon Travis (jtravis@p00p.org)
 *
 * El-Kabong: A speedy, yet forgiving, SAX-stylee HTML parser.  
 *
 * The idea behind this parser is for it to use very little memory, and still 
 * be very speedy, while forgiving poorly written HTML.

 * The internals of the parser consist of a small memory buffer which is able
 * to grow when not enough information is known to correctly parse a tag.
 * Given the typical layout of HTML, 4k should be plenty.  
 *
 * The main state engine loops through this internal buffer, determining what 
 * the next state should be.  Once this is known, it passes off a segment to 
 * the state handlers (starttag, endtag, etc.) to process.  The segment
 * handlers and the main state engine communicate via a few variables.  These
 * variables indicate whether or not the main engine should switch state,
 * or successfully remove some data, etc.  The segment handlers are 
 * guaranteed the same starting data (though not the same pointer) on each
 * invocation until the state is changed.  Thus, the segment handlers cannot
 * use pointers into the main buffer -- they must use offsets.
 *
 * Some of the speed is gained from using character map data found in
 * ekhtml_tables.h.  I don't have any empirical data for this yet --
 * it only sounds like it would be faster.. ;-)  
 *
 * I'm always looking for ways to clean && speed up this code.  Feel free
 * to give feedback -- JMT
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#define EKHTML_USE_TABLES
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif


/*
 * ekhtml_buffer_grow:  Grow the parser's internal buffer by a blocksize.  
 *                      NOTE:  Calling the function has the potential to
 *                             change the data buffer location.  Do
 *                             not rely on it's location!
 *
 * Arguments:           Parser = Parser to grow
 */

static void ekhtml_buffer_grow(ekhtml_parser_t *parser){
    size_t newsize;
    char *newbuf;
    
    newsize = parser->nalloced + EKHTML_BLOCKSIZE;
    
    if((newbuf = realloc(parser->buf, newsize)) == NULL){
        fprintf(stderr, "BAD! Can't allocate %d bytes in ekhtml_buffer_grow\n",
                newsize);
        fflush(stderr); /* Just in case someone changes the buffering scheme */
    }

    parser->buf      = newbuf;
    parser->nalloced = newsize;
}

/*
 * parser_state_determine:  Determine the next state that the main parser 
 *                          should have, by investigating up to the first
 *                          4 characters in the buffer.
 *
 * Arguments:      startp = Starting data pointer
 *                 endp   = Pointer to first byte of 'out of range' data
 *
 * Return values:  Returns one of EKHTML_STATE_* indicating the state that
 *                 was found.
 *
 */

static inline
int parser_state_determine(const char *startp, const char *endp){
    const char *firstchar;
    int newstate;
    
    assert(startp != endp);
    
    if(*startp != '<')
        return EKHTML_STATE_INDATA;
    
    firstchar = startp + 1;
    if(firstchar == endp)
        return EKHTML_STATE_NONE;

    newstate = EKCMap_EKState[(unsigned char)*firstchar];
    if(newstate == EKHTML_STATE_NONE){
        if(firstchar + 2 >= endp) /* Not enough data to evaluate */
            return EKHTML_STATE_NONE;
        if(*(firstchar + 1) == '-' && *(firstchar + 2) == '-')
            return EKHTML_STATE_COMMENT;
        else
            return EKHTML_STATE_SPECIAL;
    } else 
        return newstate;
}


int ekhtml_parser_flush(ekhtml_parser_t *parser, int flushall){
    void **state_data = &parser->state.state_data;
    char *buf = parser->buf, *curp = buf, *endp = buf + parser->nbuf;
    int badp = -1, tmpstate = parser->state.state, didsomething = 0;
    
    while(curp != endp){
        char *workp = curp;
        
        if(tmpstate == EKHTML_STATE_NONE){
            tmpstate = parser_state_determine(workp, endp);
            if(tmpstate == EKHTML_STATE_NONE)  /* Not enough data yet */
                break;
        }
        
        if(tmpstate == EKHTML_STATE_INDATA || tmpstate == EKHTML_STATE_BADDATA)
            curp = ekhtml_parse_data(parser, workp, endp, tmpstate);
        else if(endp - workp > 2){  /* All tags fall under this catagory */
            switch(tmpstate){
            case EKHTML_STATE_ENDTAG:
                curp = ekhtml_parse_endtag(parser, state_data, 
                                           workp, endp, &badp);
                break;
            case EKHTML_STATE_STARTTAG:
                curp = ekhtml_parse_starttag(parser, state_data, 
                                             workp, endp, &badp);
                break;
            case EKHTML_STATE_COMMENT:
                curp = ekhtml_parse_comment(parser, state_data, 
                                            workp, endp, &badp);
                break;
            case EKHTML_STATE_SPECIAL:
                curp = ekhtml_parse_special(parser, state_data, 
                                            workp, endp, &badp);
                break;
            default:
                assert(!"Unimplemented state");
            }
        } else {
            curp = NULL; /* Not enough data, keep going */
        }
        
        /* If one of the parsers said the data was bad, reset the state */
        if(badp != -1){
            tmpstate = badp;
            badp = -1;
        }

        if(curp == NULL){ /* State needed more data, so break out */
            curp = workp;
            break;
        }

        if(workp != curp){  /* state backend cleared up some data */
            didsomething = 1;
            tmpstate = EKHTML_STATE_NONE;
            assert(*state_data == NULL);
        }
    }

    if(flushall){
        /* Flush whatever we didn't use */
        if(parser->datacb){
            ekhtml_string_t str;

            str.str = curp;
            str.len = endp - curp;
            parser->datacb(parser->cbdata, &str);
        }
        curp = endp;
        didsomething = 1;
        tmpstate = EKHTML_STATE_NONE;   /* Clean up to an unknown state */
        *state_data = NULL;
    }

    parser->state.state = tmpstate;

    if(didsomething){
        /* Shuffle the data back, based on where we ended up */
        parser->nbuf -= curp - buf;
        if(endp - curp){  /* If there's still any data to move */
            memmove(buf, curp, endp - curp);
        }
    }
    return didsomething;
}

void ekhtml_parser_feed(ekhtml_parser_t *parser, ekhtml_string_t *str){
    size_t nfed = 0;
    
    while(nfed != str->len){
        size_t tocopy;
        
        /* First see how much we can fill up our internal buffer */
        tocopy = MIN(parser->nalloced - parser->nbuf, str->len - nfed);
        memcpy(parser->buf + parser->nbuf, str->str + nfed, tocopy);
        nfed         += tocopy;
        parser->nbuf += tocopy;
        if(parser->nalloced == parser->nbuf){
            /* Process the buffer */
            if(!ekhtml_parser_flush(parser, 0)){
                /* If we didn't actually process anything, grow our buffer */
                ekhtml_buffer_grow(parser);
            }
        }
    }
}

void ekhtml_parser_datacb_set(ekhtml_parser_t *parser, ekhtml_data_cb_t cb){
    parser->datacb = cb;
}

void ekhtml_parser_commentcb_set(ekhtml_parser_t *parser, ekhtml_data_cb_t cb){
    parser->commentcb = cb;
}

void ekhtml_parser_cbdata_set(ekhtml_parser_t *parser, void *cbdata){
    parser->cbdata = cbdata;
}

static void 
ekhtml_parser_startendcb_add(ekhtml_parser_t *parser, const char *tag,
                             ekhtml_starttag_cb_t startcb, 
                             ekhtml_endtag_cb_t endcb,
                             int isStart)
{
    ekhtml_tag_container *cont;
    ekhtml_string_t lookup_str;
    char *newtag, *cp;
    unsigned int taglen;
    hnode_t *hn;

    if(!tag){
        if(isStart)
            parser->startcb_unk = startcb;
        else
            parser->endcb_unk = endcb;
        return;
    }


    newtag = strdup(tag);
    for(cp=newtag; *cp; cp++)
        *cp = toupper(*cp);
    
    taglen = cp - newtag;

    /* First see if the container already exists */
    lookup_str.str = newtag;
    lookup_str.len = taglen;

    if((hn = hash_lookup(parser->startendcb, &lookup_str))){
        cont = hnode_get(hn);
        free(newtag);
        if(isStart)
            cont->startfunc = startcb;
        else
            cont->endfunc = endcb;
    } else {
        ekhtml_string_t *set_str;

        cont = malloc(sizeof(*cont));
        if(isStart){
            cont->startfunc = startcb;
            cont->endfunc   = NULL;
        } else {
            cont->startfunc = NULL;
            cont->endfunc   = endcb;
        }
        set_str = malloc(sizeof(*set_str));
        *set_str = lookup_str;
        hash_alloc_insert(parser->startendcb, set_str, cont);
    }
}

void ekhtml_parser_startcb_add(ekhtml_parser_t *parser, const char *tag,
			       ekhtml_starttag_cb_t cback)
{
    ekhtml_parser_startendcb_add(parser, tag, cback, NULL, 1);
}

void ekhtml_parser_endcb_add(ekhtml_parser_t *parser, const char *tag,
			     ekhtml_endtag_cb_t cback)
{
    ekhtml_parser_startendcb_add(parser, tag, NULL, cback, 0);
}


static hash_val_t ekhtml_string_hash(const void *key){
    const ekhtml_string_t *s = key;
    hash_val_t res = 5381;
    const char *str = s->str;
    size_t len = s->len;
    int c;

    while(len--){
        c = str[len];
        res = ((res << 5) + res) + c; /* res * 33 + c */
    }
    return res;
}

static int ekhtml_string_comp(const void *key1, const void *key2){
    const ekhtml_string_t *s1 = key1, *s2 = key2;

    if(s1->len == s2->len)
        return memcmp(s1->str, s2->str, s1->len);
    return 1;
}

void ekhtml_parser_destroy(ekhtml_parser_t *ekparser){
    hnode_t *hn;
    hscan_t hs;

    hash_scan_begin(&hs, ekparser->startendcb);
    while((hn = hash_scan_next(&hs))){
        ekhtml_string_t *key = (ekhtml_string_t *)hnode_getkey(hn);
        ekhtml_tag_container *cont = hnode_get(hn);

        hash_scan_delfree(ekparser->startendcb, hn);
        free((char *)key->str);
        free(key);
        free(cont);
    }
    
    hash_destroy(ekparser->startendcb);
    ekhtml_parser_starttag_cleanup(ekparser);
    free(ekparser->buf);
    free(ekparser);
}

ekhtml_parser_t *ekhtml_parser_new(void *cbdata){
    ekhtml_parser_t *res;
    
    res = malloc(sizeof(*res));
    res->datacb             = NULL;
    res->startendcb         = hash_create(HASHCOUNT_T_MAX, ekhtml_string_comp,
                                          ekhtml_string_hash);
    res->cbdata             = cbdata;
    res->startcb_unk        = NULL;
    res->endcb_unk          = NULL;
    res->commentcb          = NULL;
    res->buf                = NULL;
    res->nalloced           = 0;
    res->nbuf               = 0;
    res->freeattrs          = NULL;
    res->state.state        = EKHTML_STATE_NONE;
    res->state.state_data   = NULL;
    
    /* Start out with a buffer of 1 block size */
    ekhtml_buffer_grow(res);
    return res;
}

