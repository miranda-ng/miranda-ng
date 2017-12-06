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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tlv.h"

/* Make a single TLV, copying the supplied data */
OtrlTLV *otrl_tlv_new(unsigned short type, unsigned short len,
	const unsigned char *data)
{
    OtrlTLV *tlv = malloc(sizeof(OtrlTLV));
    assert(tlv != NULL);
    tlv->type = type;
    tlv->len = len;
    tlv->data = malloc(len + 1);
    assert(tlv->data != NULL);
    memmove(tlv->data, data, len);
    tlv->data[tlv->len] = '\0';
    tlv->next = NULL;
    return tlv;
}

/* Construct a chain of TLVs from the given data */
OtrlTLV *otrl_tlv_parse(const unsigned char *serialized, size_t seriallen)
{
    OtrlTLV *tlv = NULL;
    OtrlTLV **tlvp = &tlv;
    while (seriallen >= 4) {
	unsigned short type = (serialized[0] << 8) + serialized[1];
	unsigned short len = (serialized[2] << 8) + serialized[3];
	serialized += 4; seriallen -=4;
	if (seriallen < len) break;
	*tlvp = otrl_tlv_new(type, len, serialized);
	serialized += len;
	seriallen -= len;
	tlvp = &((*tlvp)->next);
    }
    return tlv;
}

/* Deallocate a chain of TLVs */
void otrl_tlv_free(OtrlTLV *tlv)
{
    while (tlv) {
	OtrlTLV *next = tlv->next;
	free(tlv->data);
	free(tlv);
	tlv = next;
    }
}

/* Find the serialized length of a chain of TLVs */
size_t otrl_tlv_seriallen(const OtrlTLV *tlv)
{
    size_t totlen = 0;
    while (tlv) {
	totlen += tlv->len + 4;
	tlv = tlv->next;
    }
    return totlen;
}

/* Serialize a chain of TLVs.  The supplied buffer must already be large
 * enough. */
void otrl_tlv_serialize(unsigned char *buf, const OtrlTLV *tlv)
{
    while (tlv) {
	buf[0] = (tlv->type >> 8) & 0xff;
	buf[1] = tlv->type & 0xff;
	buf[2] = (tlv->len >> 8) & 0xff;
	buf[3] = tlv->len & 0xff;
	buf += 4;
	memmove(buf, tlv->data, tlv->len);
	buf += tlv->len;
	tlv = tlv->next;
    }
}

/* Return the first TLV with the given type in the chain, or NULL if one
 * isn't found.  (The tlvs argument isn't const because the return type
 * needs to be non-const.) */
OtrlTLV *otrl_tlv_find(OtrlTLV *tlvs, unsigned short type)
{
    while (tlvs) {
	if (tlvs->type == type) return tlvs;
	tlvs = tlvs->next;
    }
    return NULL;
}
