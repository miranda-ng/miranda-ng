/*
 *  Off-the-Record Messaging Toolkit
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
    size_t alloclen;
} Buffer;

static void buf_new(Buffer *bufp)
{
    bufp->data = NULL;
    bufp->len = 0;
    bufp->alloclen = 0;
}

static void buf_put(Buffer *bufp, const char *str, size_t len)
{
    while (bufp->len + len + 1 > bufp->alloclen) {
	char *newdata = realloc(bufp->data, bufp->alloclen + 1024);
	if (!newdata) {
	    fprintf(stderr, "Out of memory!\n");
	    exit(1);
	}
	bufp->data = newdata;
	bufp->alloclen += 1024;
    }
    memmove(bufp->data + bufp->len, str, len);
    bufp->len += len;
    bufp->data[bufp->len] = '\0';
}

static void buf_putc(Buffer *bufp, char c)
{
    buf_put(bufp, &c, 1);
}

/* Read from the given stream until we see a complete OTR Key Exchange
 * or OTR Data message.  Return a newly-allocated pointer to a copy of
 * this message, which the caller should free().  Returns NULL if no
 * such message could be found. */
char *readotr(FILE *stream)
{
    int seen = 0;
    const char header[] = "?OTR:";  /* There are no '?' chars other than
				       the leading one */
    int headerlen = strlen(header);
    Buffer buf;

    while(seen < headerlen) {
	int c = fgetc(stream);
	if (c == EOF) return NULL;
	else if (c == header[seen]) seen++;
	else if (c == header[0]) seen = 1;
	else seen = 0;
    }

    buf_new(&buf);
    buf_put(&buf, header, headerlen);

    /* Look for the trailing '.' */
    while(1) {
	int c = fgetc(stream);
	if (c == EOF) break;
	buf_putc(&buf, c);
	if (c == '.') break;
    }

    return buf.data;
}
