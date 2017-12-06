/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2012  Ian Goldberg, Chris Alexander, Willy Lew,
 *  			     Nikita Borisov
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

/* Modified from: */

/*********************************************************************\

MODULE NAME:    b64.c

AUTHOR:         Bob Trower 08/04/01

LICENCE:        Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.

		Permission is hereby granted, free of charge, to any person
		obtaining a copy of this software and associated
		documentation files (the "Software"), to deal in the
		Software without restriction, including without limitation
		the rights to use, copy, modify, merge, publish, distribute,
		sublicense, and/or sell copies of the Software, and to
		permit persons to whom the Software is furnished to do so,
		subject to the following conditions:

		The above copyright notice and this permission notice shall
		be included in all copies or substantial portions of the
		Software.

		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
		KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
		WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
		PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
		OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
		OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
		OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
		SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

VERSION HISTORY:
		Bob Trower 08/04/01 -- Create Version 0.00.00B

\******************************************************************* */

/* system headers */
#include <stdio.h>
#include <string.h>

/* libotr headers */
#include "b64.h"

/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** encodeblock
**
** encode up to 3 8-bit binary bytes as 4 '6-bit' characters.
** len must be 1, 2, or 3.
*/
static void encodeblock( char *out, const unsigned char *in, size_t len )
{
    unsigned char in0, in1, in2;
    in0 = in[0];
    in1 = len > 1 ? in[1] : 0;
    in2 = len > 2 ? in[2] : 0;

    out[0] = cb64[ in0 >> 2 ];
    out[1] = cb64[ ((in0 & 0x03) << 4) | ((in1 & 0xf0) >> 4) ];
    out[2] = len > 1 ? cb64[ ((in1 & 0x0f) << 2) | ((in2 & 0xc0) >> 6) ]
		     : '=';
    out[3] = len > 2 ? cb64[ in2 & 0x3f ]
		     : '=';
}

/*
 * base64 encode data.  Insert no linebreaks or whitespace.
 *
 * The buffer base64data must contain at least ((datalen+2)/3)*4 bytes of
 * space.  This function will return the number of bytes actually used.
 */
size_t otrl_base64_encode(char *base64data, const unsigned char *data,
	size_t datalen)
{
    size_t base64len = 0;

    while(datalen > 2) {
	encodeblock(base64data, data, 3);
	base64data += 4;
	base64len += 4;
	data += 3;
	datalen -= 3;
    }
    if (datalen > 0) {
	encodeblock(base64data, data, datalen);
	base64len += 4;
    }

    return base64len;
}

static size_t decode(unsigned char *out, const char *in, size_t b64len)
{
    size_t written = 0;
    unsigned char c = 0;

    if (b64len > 0) {
	c = in[0] << 2;
    }
    if (b64len > 1) {
	out[0] = c | in[1] >> 4;
	written = 1;
	c = in[1] << 4;
    }
    if (b64len > 2) {
	out[1] = c | in[2] >> 2;
	written = 2;
	c = in[2] << 6;
    }
    if (b64len > 3) {
	out[2] = c | in[3];
	written = 3;
    }
    return written;
}

/*
 * base64 decode data.  Skip non-base64 chars, and terminate at the
 * first '=', or the end of the buffer.
 *
 * The buffer data must contain at least ((base64len+3) / 4) * 3 bytes
 * of space.  This function will return the number of bytes actually
 * used.
 */
size_t otrl_base64_decode(unsigned char *data, const char *base64data,
	size_t base64len)
{
    size_t datalen = 0;
    char b64[4];
    size_t b64accum = 0;

    while(base64len > 0) {
	char b = *base64data;
	unsigned char bdecode;
	++base64data;
	--base64len;
	if (b < '+' || b > 'z') continue;  /* Skip non-base64 chars */
	if (b == '=') {
	    /* Force termination */
	    datalen += decode(data, b64, b64accum);
	    base64len = 0;
	} else {
	    bdecode = cd64[b-'+'];
	    if (bdecode == '$') continue;  /* Skip non-base64 chars */
	    b64[b64accum++] = bdecode-'>';
	    if (b64accum == 4) {
		/* We have a complete block; decode it. */
		size_t written = decode(data, b64, b64accum);
		data += written;
		datalen += written;
		b64accum = 0;
	    }
	}
    }

    /* Just discard any short block at the end. */

    return datalen;
}

/*
 * Base64-encode a block of data, stick "?OTR:" and "." around it, and
 * return the result, or NULL in the event of a memory error.  The
 * caller must free() the return value.
 */
char *otrl_base64_otr_encode(const unsigned char *buf, size_t buflen)
{
    char *base64buf;
    size_t base64len;
    const size_t HALF_MAX_SIZE_T = ((size_t)-1) >> 1;

    if (buflen > HALF_MAX_SIZE_T) {
	/* You somehow have a buffer that's of size more than half of
	 * all addressable memory, and you now want a base64 version in
	 * a new buffer 33% larger?  Not going to happen.  Exit now,
	 * rather in the malloc below, to avoid integer overflowing the
	 * computation of base64len. */
	 return NULL;
    }

    /* Make the base64-encoding. */
    base64len = ((buflen + 2) / 3) * 4;
    base64buf = malloc(5 + base64len + 1 + 1);
    if (base64buf == NULL) {
	return NULL;
    }
    memmove(base64buf, "?OTR:", 5);
    otrl_base64_encode(base64buf+5, buf, buflen);
    base64buf[5 + base64len] = '.';
    base64buf[5 + base64len + 1] = '\0';

    return base64buf;
}

/*
 * Base64-decode the portion of the given message between "?OTR:" and
 * ".".  Set *bufp to the decoded data, and set *lenp to its length.
 * The caller must free() the result.  Return 0 on success, -1 on a
 * memory error, or -2 on invalid input.
 */
int otrl_base64_otr_decode(const char *msg, unsigned char **bufp,
	size_t *lenp)
{
    char *otrtag, *endtag;
    size_t msglen, rawlen;
    unsigned char *rawmsg;

    otrtag = strstr(msg, "?OTR:");
    if (!otrtag) {
	return -2;
    }

    endtag = strchr(otrtag, '.');
    if (endtag) {
	msglen = endtag-otrtag;
    } else {
	return -2;
    }

    /* Skip over the "?OTR:" */
    otrtag += 5;
    msglen -= 5;

    /* Base64-decode the message */
    rawlen = OTRL_B64_MAX_DECODED_SIZE(msglen);   /* maximum possible */
    rawmsg = malloc(rawlen);
    if (!rawmsg && rawlen > 0) {
	return -1;
    }

    rawlen = otrl_base64_decode(rawmsg, otrtag, msglen);  /* actual size */

    *bufp = rawmsg;
    *lenp = rawlen;

    return 0;
}
