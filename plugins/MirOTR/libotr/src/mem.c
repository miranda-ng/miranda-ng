/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2014  Ian Goldberg, David Goulet, Rob Smits,
 *                           Chris Alexander, Willy Lew, Lisa Du,
 *                           Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of version 2.1 of the GNU Lesser General
 *  Public License as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Memory allocation routines for libgcrypt.  All of the session key
 * information gets allocated through here, so we can wipe it out when
 * it's free()d.  We don't use the built-in secmem functions of
 * libgcrypt because you need to declare a fixed amount of it when you
 * start up.
 *
 * Because "secure" and "insecure" allocations from libgcrypt will get
 * handled the same way (since we're not going to be running as root,
 * and so won't actually have pinned memory), pretend all allocated
 * memory (but just from libgcrypt) is requested secure, and wipe it on
 * free(). */

/* Uncomment the following to add a check that our free() and realloc() only
 * get called on things returned from our malloc(). */
/* #define OTRL_MEM_MAGIC 0x31415926 */

/* system headers */
#ifdef OTRL_MEM_MAGIC
#include <stdio.h>
#endif
#include <stdlib.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "mem.h"

static size_t header_size;

static void *otrl_mem_malloc(size_t n)
{
    void *p;
    size_t new_n = n;
    new_n += header_size;

    /* Check for overflow attack */
    if (new_n < n) return NULL;
    p = malloc(new_n);
    if (p == NULL) return NULL;

    ((size_t *)p)[0] = new_n;  /* Includes header size */
#ifdef OTRL_MEM_MAGIC
    ((size_t *)p)[1] = OTRL_MEM_MAGIC;
#endif

    return (void *)((char *)p + header_size);
}

static int otrl_mem_is_secure(const void *p)
{
    return 1;
}

static void otrl_mem_free(void *p)
{
    void *real_p = (void *)((char *)p - header_size);
    size_t n = ((size_t *)real_p)[0];
#ifdef OTRL_MEM_MAGIC
    if (((size_t *)real_p)[1] != OTRL_MEM_MAGIC) {
	fprintf(stderr, "Illegal free!\n");
	return;
    }
#endif

    /* Wipe the memory (in the same way the built-in deallocator in
     * libgcrypt would) */
    memset(real_p, 0xff, n);
    memset(real_p, 0xaa, n);
    memset(real_p, 0x55, n);
    memset(real_p, 0x00, n);

    free(real_p);
}

static void *otrl_mem_realloc(void *p, size_t n)
{
    if (p == NULL) {
	return otrl_mem_malloc(n);
    } else if (n == 0) {
	otrl_mem_free(p);
	return NULL;
    } else {
	void *real_p = (void *)((char *)p - header_size);
	void *new_p;
	size_t old_n = ((size_t *)real_p)[0];
#ifdef OTRL_MEM_MAGIC
	size_t magic = ((size_t *)real_p)[1];
#endif
	size_t new_n = n;
	new_n += header_size;

	/* Check for overflow attack */
	if (new_n < n) return NULL;

#ifdef OTRL_MEM_MAGIC
	if (magic != OTRL_MEM_MAGIC) {
	    fprintf(stderr, "Illegal realloc!\n");
	    return NULL;
	}
#endif

	if (new_n < old_n) {
	    /* Overwrite the space we're about to stop using */
	    void *p = (void *)((char *)real_p + new_n);
	    size_t excess = old_n - new_n;
	    memset(p, 0xff, excess);
	    memset(p, 0xaa, excess);
	    memset(p, 0x55, excess);
	    memset(p, 0x00, excess);

	    /* We don't actually need to realloc() */
	    new_p = real_p;
	} else {
	    new_p = realloc(real_p, new_n);
	    if (new_p == NULL) return NULL;
	}

	((size_t *)new_p)[0] = new_n;  /* Includes header size */
	return (void *)((char *)new_p + header_size);
    }
}

void otrl_mem_init(void)
{
    header_size = 8;
#ifdef OTRL_MEM_MAGIC
    if (header_size < 2*sizeof(size_t)) {
	header_size = 2*sizeof(size_t);
    }
#else
    if (header_size < sizeof(size_t)) {
	header_size = sizeof(size_t);
    }
#endif

    gcry_set_allocation_handler(
	    otrl_mem_malloc,
	    otrl_mem_malloc,
	    otrl_mem_is_secure,
	    otrl_mem_realloc,
	    otrl_mem_free
	);
}

/* Compare two memory blocks in time dependent on the length of the
 * blocks, but not their contents.  Returns 1 if they differ, 0 if they
 * are the same. */
int otrl_mem_differ(const unsigned char *buf1, const unsigned char *buf2,
    size_t len)
{
    volatile unsigned char diff = 0;
    size_t i;

    for (i = 0; i < len; ++i) {
        diff |= (buf1[i] ^ buf2[i]);
    }
    return (diff != 0);
}
