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

#ifndef __PARSE_H__
#define __PARSE_H__

#include <gcrypt.h>

typedef struct s_KeyExchMsg {
    unsigned char *raw;         /* The base64-decoded data; must be free()d */
    unsigned char reply;
    gcry_mpi_t p, q, g, e;
    unsigned int keyid;
    gcry_mpi_t y;
    gcry_mpi_t r, s;
    unsigned char *sigstart;    /* Pointers into the "raw" array.  Don't */
    unsigned char *sigend;      /*   free() these. */
} * KeyExchMsg;

typedef struct s_DataMsg {
    unsigned char *raw;         /* The base64-decoded data; must be free()d */
    size_t rawlen;
    int flags;
    unsigned int sender_keyid;
    unsigned int rcpt_keyid;
    gcry_mpi_t y;
    unsigned char ctr[8];
    unsigned char *encmsg;      /* A copy; must be free()d */
    size_t encmsglen;
    unsigned char mac[20];
    unsigned char *mackeys;     /* A copy; must be free()d */
    size_t mackeyslen;
    unsigned char *macstart;    /* Pointers into the "raw" array.  Don't */
    unsigned char *macend;      /*   free() these. */
} * DataMsg;

typedef struct s_CommitMsg {
    unsigned char *raw;         /* The base64-decoded data; must be free()d */
    unsigned char *enckey;
    size_t enckeylen;
    unsigned char *hashkey;
    size_t hashkeylen;
} * CommitMsg;

typedef struct s_KeyMsg {
    unsigned char *raw;         /* The base64-decoded data; must be free()d */
    gcry_mpi_t y;
} * KeyMsg;

typedef struct s_RevealSigMsg {
    unsigned char *raw;         /* The base64-decoded data; must be free()d */
    unsigned char *key;
    size_t keylen;
    unsigned char *encsig;
    size_t encsiglen;
    unsigned char mac[20];
} * RevealSigMsg;

typedef struct s_SignatureMsg {
    unsigned char *raw;         /* The base64-decoded data; must be free()d */
    unsigned char *encsig;
    size_t encsiglen;
    unsigned char mac[20];
} * SignatureMsg;

/* Dump an unsigned int to a FILE * */
void dump_int(FILE *stream, const char *title, unsigned int val);

/* Dump an mpi to a FILE * */
void dump_mpi(FILE *stream, const char *title, gcry_mpi_t val);

/* Dump data to a FILE * */
void dump_data(FILE *stream, const char *title, const unsigned char *data,
	size_t datalen);

/* Parse a Key Exchange Message into a newly-allocated KeyExchMsg structure */
KeyExchMsg parse_keyexch(const char *msg);

/* Deallocate a KeyExchMsg and all of the data it points to */
void free_keyexch(KeyExchMsg keyexch);

/* Parse a D-H Commit Message into a newly-allocated CommitMsg structure */
CommitMsg parse_commit(const char *msg);

/* Parse a Data Message into a newly-allocated DataMsg structure */
DataMsg parse_datamsg(const char *msg);

/* Deallocate a CommitMsg and all of the data it points to */
void free_commit(CommitMsg cmsg);

/* Parse a Reveal Signature Message into a newly-allocated RevealSigMsg
 * structure */
RevealSigMsg parse_revealsig(const char *msg);

/* Deallocate a RevealSigMsg and all of the data it points to */
void free_revealsig(RevealSigMsg rmsg);

/* Parse a Signature Message into a newly-allocated SignatureMsg structure */
SignatureMsg parse_signature(const char *msg);

/* Deallocate a SignatureMsg and all of the data it points to */
void free_signature(SignatureMsg smsg);

/* Parse a D-H Key Message into a newly-allocated KeyMsg structure */
KeyMsg parse_key(const char *msg);

/* Deallocate a KeyMsg and all of the data it points to */
void free_key(KeyMsg cmsg);

/* Recalculate the MAC on the message, base64-encode the resulting MAC'd
 * message, and put on the appropriate header and footer.  Return a
 * newly-allocated pointer to the result, which the caller will have to
 * free(). */
char *remac_datamsg(DataMsg datamsg, unsigned char mackey[20]);

/* Assemble a new Data Message from its pieces.  Return a
 * newly-allocated string containing the base64 representation. */
char *assemble_datamsg(unsigned char mackey[20], int flags,
	unsigned int sender_keyid, unsigned int rcpt_keyid, gcry_mpi_t y,
	unsigned char ctr[8], unsigned char *encmsg, size_t encmsglen,
	unsigned char *mackeys, size_t mackeyslen);

/* Deallocate a DataMsg and all of the data it points to */
void free_datamsg(DataMsg datamsg);

/* Convert a string of hex chars to a buffer of unsigned chars. */
void argv_to_buf(unsigned char **bufp, size_t *lenp, char *arg);

#endif
