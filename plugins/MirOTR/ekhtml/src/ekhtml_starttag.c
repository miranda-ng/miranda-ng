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
 * ekhtml_starttag:  Processor for HTML start-tags.
 *
 * This starttag processor is the most complicated of the tag processors.
 * It has it's own small internal state machine which keeps track of what
 * the next thing it is searching for is.
 *
 * SOURCE NOTES:  We do a bit of weird hackery with respect to storing 
 *                attributes.  Since we need to store of OFFSET of the
 *                attribute and values, and cannot store actual pointers
 *                (see ekhtml.c for an explanation as to why), we need
 *                to create a whole new structure to store this info.
 *                BUT, we are just going to allocate a very similar 
 *                structure (ekhtml_attr_t) anyway, and fill it in.  
 *                SO, we just use the ekhtml_attr_t, and assign 'integer'
 *                values to the pointer.  This saves us allocations and
 *                some management issues at the cost of readability.
 */

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "ekhtml_config.h"
#include "ekhtml.h"
#include "ekhtml_tables.h"
#define EKHTML_USE_PRIVATE
#include "ekhtml_private.h"

#define EKHTML_STMODE_TAG      0  /* Finding the tagname                     */
#define EKHTML_STMODE_SUCK     1  /* Suck data until the last '<' or '>'     */
#define EKHTML_STMODE_BEGNAME  2  /* Find the beginning of an attribute name */
#define EKHTML_STMODE_GETNAME  3  /* Get the rest of the attribute name      */
#define EKHTML_STMODE_GETEQUAL 4  /* Find the equals sign                    */
#define EKHTML_STMODE_BEGVALUE 5  /* Get the beginning of a attribute value  */
#define EKHTML_STMODE_GETVALUE 6  /* Get the rest of an attribute value      */

/*
 * ekhtml_parser_starttag_cleanup:  Cleanup allocated memory, as the 
 *                                  parser object is about to be destroyed
 *
 * Arguments:      parser = Parser to cleanup
 *
 */

void ekhtml_parser_starttag_cleanup(ekhtml_parser_t *parser){
    ekhtml_attr_t *attr, *next;

    for(attr=parser->freeattrs; attr; attr=next){
        next = attr->next;
        free(attr);
    }
}

/*
 * ekhtml_parser_newattr:  Get a new unused attribute structure.  
 *
 * Arguments:      parser = Parser to get a new attribute structure for
 *
 * Return values:  Returns a new attribute structure, which should be
 *                 passed to ekhtml_parser_attr_release when the caller
 *                 is done using it.  The values of the returned attribute
 *                 must be initialized by the caller
 */

static inline
ekhtml_attr_t *ekhtml_parser_attr_new(ekhtml_parser_t *parser){
    ekhtml_attr_t *res;
    
    if(parser->freeattrs == NULL){
        res = malloc(sizeof(*res));
    } else {
        res = parser->freeattrs;
        /* Remove it from the list */
        parser->freeattrs = parser->freeattrs->next; 
    }
    return res;
}

/*
 * ekhtml_parser_attr_release:  Release use of an attribute previously fetched
 *                              via the ekhtml_parser_newattr.
 *
 * Arguments:     parser = parser to give the attribute back to
 *                attr   = Attribute to relinquish use of
 *
 */

static inline
void ekhtml_parser_attr_release(ekhtml_parser_t *parser, ekhtml_attr_t *attr){
    attr->next = parser->freeattrs;
    parser->freeattrs = attr;
}

static void handle_starttag(ekhtml_parser_t *parser, char *curp,
			    ekhtml_starttag_state *sstate)
{
    ekhtml_tag_container *container;
    ekhtml_starttag_cb_t cback = NULL;
    int taglen = sstate->tagend - 1;
    ekhtml_string_t str;
    ekhtml_attr_t *attr;
    char *upper_str;
    hnode_t *hn;

    upper_str = ekhtml_make_upperstr(curp + 1, taglen);
    str.str = upper_str;
    str.len = taglen;

    if((hn = hash_lookup(parser->startendcb, &str)) &&
       (container = hnode_get(hn)) &&
       container->startfunc)
    {
        cback = container->startfunc;
    } else if(parser->startcb_unk)
        cback = parser->startcb_unk;
    
    if(!cback)
        return;
    
    /* Formulate real attribute callback data from the 'offset' 
       pointer values */
    for(attr=sstate->attrs;attr;attr=attr->next){
        attr->name.str = curp + (int)attr->name.str;
        if(!attr->isBoolean)
            attr->val.str = curp + (int)attr->val.str;
    }
    
    cback(parser->cbdata, &str, sstate->attrs);
}

static void release_attributes(ekhtml_parser_t *parser,
			       ekhtml_starttag_state *sstate)
{
    ekhtml_attr_t *attr, *next;
    
    if(sstate->curattr)
        ekhtml_parser_attr_release(parser, sstate->curattr);
    
    attr = sstate->attrs;
    while(attr){
        next = attr->next;
        ekhtml_parser_attr_release(parser, attr);
        attr = next;
    }
}

static inline void scroll_attribute(ekhtml_starttag_state *sstate){
    sstate->curattr->next = sstate->attrs;
    sstate->attrs         = sstate->curattr;
    sstate->curattr       = NULL;
}


char *ekhtml_parse_starttag(ekhtml_parser_t *parser, void **state_data,
			    char *curp, char *endp, int *baddata)
{
    ekhtml_starttag_state *startstate = *state_data;
    int *offset = &parser->state.offset;
    char *workp;
    
    assert(*curp == '<' && isalpha(*(curp + 1)));
    assert(endp - curp >= 3);
    
    if(startstate == NULL){  /* First time the tag is called */
        startstate          = &parser->startstate;
        startstate->tagend  = sizeof("<F") - 1;
        startstate->mode    = EKHTML_STMODE_TAG;
        startstate->attrs   = NULL;
        startstate->curattr = NULL;
        startstate->quote   = '\0';
        *state_data         = startstate;
        *offset             = startstate->tagend;
    }
    
    workp = curp + *offset;
    
    if(startstate->mode == EKHTML_STMODE_TAG){
        /* Find that tag! */
        workp = ekhtml_find_notcharsmap(workp, endp - workp, EKCMap_CharMap,
                                        EKHTML_CHAR_TAGNAME);
        *offset = workp - curp;  
        if(workp == endp)
            return NULL;
        
        startstate->tagend  = *offset;
        startstate->mode = EKHTML_STMODE_BEGNAME;
    }
    
    while(workp != endp){  /* Main state processing loop */
        if(startstate->mode == EKHTML_STMODE_BEGNAME){
            ekhtml_attr_t *attr;
            
            workp = ekhtml_find_notcharsmap(workp, endp - workp, 
                                            EKCMap_CharMap,
                                            EKHTML_CHAR_WHITESPACE);
            if(workp == endp)
                break;
            
            if(!(EKCMap_CharMap[(unsigned char)*workp] & 
                 EKHTML_CHAR_BEGATTRNAME))
            {
                /* Bad attrname character */
                startstate->mode = EKHTML_STMODE_SUCK;  
            } else {
                assert(startstate->curattr == NULL);
                /* Valid attribute name, allocate space for it */
                attr = ekhtml_parser_attr_new(parser);
                attr->name.str      = (char *)NULL + (workp - curp);
                attr->name.len      = 0;     /* Will get assigned later */
                attr->val.str       = NULL;
                attr->val.len       = 0;
                attr->isBoolean     = 1;
                attr->quoteChar     = '\0';
                attr->next          = NULL;
                startstate->mode    = EKHTML_STMODE_GETNAME;
                startstate->curattr = attr;
            }
        }

        if(startstate->mode == EKHTML_STMODE_GETNAME){
            workp = ekhtml_find_notcharsmap(workp, endp - workp, 
                                            EKCMap_CharMap,
                                            EKHTML_CHAR_ATTRNAME);
            if(workp == endp)
                break;
            
            /* There be dragons here -- watch out -- see comment @ top 
               of file */
            startstate->curattr->name.len = 
                workp - (curp + (int)startstate->curattr->name.str);
            if(*workp == '='){
                startstate->mode = EKHTML_STMODE_BEGVALUE;
                workp++;  /* Skip the equals sign */
            } else {
                if(!(EKCMap_CharMap[(unsigned char)*workp] & 
                     EKHTML_CHAR_WHITESPACE))
            {
                /* Found something we weren't expecting.  Use the current
                   attribute as a boolean value and suck the rest */
                scroll_attribute(startstate);
                startstate->mode = EKHTML_STMODE_SUCK;
            } else
                startstate->mode = EKHTML_STMODE_GETEQUAL;
            }
        }
        
        if(startstate->mode == EKHTML_STMODE_GETEQUAL){
            workp = ekhtml_find_notcharsmap(workp, endp - workp, 
                                            EKCMap_CharMap,
                                            EKHTML_CHAR_WHITESPACE);
            if(workp == endp)
                break;
      
            if(*workp != '='){ 
                /* Unexpected value.  Could either be time to suck, or this was
                   really only a boolean value */
                scroll_attribute(startstate);
                
                if(EKCMap_CharMap[(unsigned char)*workp] & 
                   EKHTML_CHAR_BEGATTRNAME)
                {
                    startstate->mode = EKHTML_STMODE_BEGNAME;
                    continue;
                } else {
                    startstate->mode = EKHTML_STMODE_SUCK;
                } 
            } else {
                startstate->mode = EKHTML_STMODE_BEGVALUE;
                workp++;  /* Skip the equals sign */
            }
        }
        
        if(startstate->mode == EKHTML_STMODE_BEGVALUE){
            workp = ekhtml_find_notcharsmap(workp, endp - workp, 
                                            EKCMap_CharMap,
                                            EKHTML_CHAR_WHITESPACE);
            if(workp == endp)
                break;
            
            startstate->curattr->isBoolean = 0;
            startstate->curattr->val.str = (char *)NULL + (workp - curp);
            startstate->quote        = '\0';
            if(*workp == '"' || *workp == '\''){
                startstate->curattr->val.str++;  /* Skip the quote */
                startstate->mode   = EKHTML_STMODE_GETVALUE;
                startstate->quote  = *workp;
                startstate->curattr->quoteChar = *workp;
                workp++;
            } else if(!(EKCMap_CharMap[(unsigned char)*workp] & 
                        EKHTML_CHAR_ATTRVALUE))
            {
                /* Bad value .. */
                startstate->curattr->val.len = 0;
                scroll_attribute(startstate);
                startstate->mode = EKHTML_STMODE_SUCK;
            } else {
                /* Valid value */
                startstate->mode = EKHTML_STMODE_GETVALUE;
            }
        }
        
        if(startstate->mode == EKHTML_STMODE_GETVALUE){
            if(startstate->quote){
                for(;workp != endp && *workp != '>' && *workp != '<'; workp++){
                    if(*workp == startstate->quote){
                        startstate->curattr->val.len = 
                            workp - (curp + (int)startstate->curattr->val.str);
                        scroll_attribute(startstate);
                        startstate->mode = EKHTML_STMODE_BEGNAME;
                        workp++;  /* Skip the quote */
                        break;
                    }
                }
                /* In case we broke out in the above loop, we may 
                   need to continue in the main loop -- CONFUSING */
                if(startstate->mode == EKHTML_STMODE_BEGNAME)
                    continue;
            } else
                workp = ekhtml_find_notcharsmap(workp, endp - workp, 
                                                EKCMap_CharMap,
                                                EKHTML_CHAR_ATTRVALUE);
            if(workp == endp)
                break;
            
            startstate->curattr->val.len = 
                workp - (curp + (int)startstate->curattr->val.str);
            scroll_attribute(startstate);
            
            if(*workp == '>' || *workp == '<') {
                *offset = workp - curp;
                handle_starttag(parser, curp, startstate);
                release_attributes(parser, startstate);
                *state_data = NULL;
                if(*workp == '<')
                    return workp;
                else
                    return workp + 1;
            } else {
                startstate->mode          = EKHTML_STMODE_BEGNAME;
                continue;
            }
        }

        if(startstate->mode == EKHTML_STMODE_SUCK){
            /* The sucking mode is here in case someone puts a bad character
               in an attribute name. We suck until what looks like end of tag*/
            for(;workp != endp && *workp != '<' && *workp != '>'; workp++)
                ;
            if(workp == endp)
                break;
            
            *offset = workp - curp;
            handle_starttag(parser, curp, startstate);
            release_attributes(parser, startstate);
            *state_data = NULL;
            if(*workp == '<')
                return workp;
            else
                return workp + 1;
        }
    }
    
    *offset = workp - curp;
    return NULL;
}
