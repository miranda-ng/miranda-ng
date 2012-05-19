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

/* libotr headers */
#include "b64.h"

/* toolkit headers */
#include "sha1hmac.h"
#include "parse.h"

/* Dump an unsigned int to a FILE * */
void dump_int(FILE *stream, const char *title, unsigned int val)
{
    fprintf(stream, "%s: %u\n", title, val);
}

/* Dump an mpi to a FILE * */
void dump_mpi(FILE *stream, const char *title, gcry_mpi_t val)
{
    size_t plen;
    unsigned char *d;
    
    gcry_mpi_print(GCRYMPI_FMT_USG, NULL, 0, &plen, val);
    d = malloc(plen);
    gcry_mpi_print(GCRYMPI_FMT_USG, d, plen, NULL, val);
    dump_data(stream, title, d, plen);
    free(d);
}

/* Dump data to a FILE * */
void dump_data(FILE *stream, const char *title, const unsigned char *data,
	size_t datalen)
{
    size_t i;
    fprintf(stream, "%s: ", title);
    for(i=0;i<datalen;++i) {
	fprintf(stream, "%02x", data[i]);
    }
    fprintf(stream, "\n");
}

/* base64 decode the message, and put the resulting size into *lenp */
static unsigned char *decode(const char *msg, size_t *lenp)
{
    const char *header, *footer;
    unsigned char *raw;
	
    /* Find the header */
    header = strstr(msg, "?OTR:");
    if (!header) return NULL;
    /* Skip the header */
    header += 5;

    /* Find the trailing '.' */
    footer = strchr(header, '.');
    if (!footer) footer = header + strlen(header);

    raw = malloc((footer-header) / 4 * 3);
    if (raw == NULL && (footer-header >= 4)) return NULL;
    *lenp = otrl_base64_decode(raw, header, footer-header);

    return raw;
}

#define require_len(l) do { if (lenp < (l)) goto inv; } while(0)
#define read_int(x) do { \
	require_len(4); \
	(x) = (bufp[0] << 24) | (bufp[1] << 16) | (bufp[2] << 8 ) | bufp[3]; \
	bufp += 4; lenp -= 4; \
    } while(0)
#define read_mpi(x) do { \
	size_t mpilen; \
	read_int(mpilen); \
	require_len(mpilen); \
	gcry_mpi_scan(&(x), GCRYMPI_FMT_USG, bufp, mpilen, NULL); \
	bufp += mpilen; lenp -= mpilen; \
    } while(0)
#define read_raw(b, l) do { \
	require_len(l); \
	memmove((b), bufp, (l)); \
	bufp += (l); lenp -= (l); \
    } while(0)
#define write_int(x) do { \
	bufp[0] = ((x) >> 24) & 0xff; \
	bufp[1] = ((x) >> 16) & 0xff; \
	bufp[2] = ((x) >> 8) & 0xff; \
	bufp[3] = (x) & 0xff; \
	bufp += 4; lenp -= 4; \
    } while(0)
#define write_mpi(x,l) do { \
	write_int(l); \
	gcry_mpi_print(GCRYMPI_FMT_USG, bufp, lenp, NULL, (x)); \
	bufp += (l); lenp -= (l); \
    } while(0)
#define write_raw(x,l) do { \
	memmove(bufp, (x), (l)); \
	bufp += (l); lenp -= (l); \
    } while(0)

/* Parse a Key Exchange Message into a newly-allocated KeyExchMsg structure */
KeyExchMsg parse_keyexch(const char *msg)
{
    KeyExchMsg kem = NULL;
    size_t lenp;
    unsigned char *raw = decode(msg, &lenp);
    unsigned char *bufp = raw;
    if (!raw) goto inv;

    kem = calloc(1, sizeof(struct s_KeyExchMsg));
    if (!kem) {
	free(raw);
	goto inv;
    }

    kem->raw = raw;
    kem->sigstart = bufp;

    require_len(3);
    if (memcmp(bufp, "\x00\x01\x0a", 3)) goto inv;
    bufp += 3; lenp -= 3;

    require_len(1);
    kem->reply = *bufp;
    bufp += 1; lenp -= 1;

    read_mpi(kem->p);
    read_mpi(kem->q);
    read_mpi(kem->g);
    read_mpi(kem->e);

    read_int(kem->keyid);

    read_mpi(kem->y);

    kem->sigend = bufp;

    require_len(40);
    gcry_mpi_scan(&kem->r, GCRYMPI_FMT_USG, bufp, 20, NULL);
    gcry_mpi_scan(&kem->s, GCRYMPI_FMT_USG, bufp+20, 20, NULL);
    bufp += 40; lenp -= 40;

    if (lenp != 0) goto inv;

    return kem;
inv:
    free_keyexch(kem);
    return NULL;
}

/* Deallocate a KeyExchMsg and all of the data it points to */
void free_keyexch(KeyExchMsg keyexch)
{
    if (!keyexch) return;
    free(keyexch->raw);
    gcry_mpi_release(keyexch->p);
    gcry_mpi_release(keyexch->q);
    gcry_mpi_release(keyexch->g);
    gcry_mpi_release(keyexch->e);
    gcry_mpi_release(keyexch->y);
    gcry_mpi_release(keyexch->r);
    gcry_mpi_release(keyexch->s);
    free(keyexch);
}

/* Parse a D-H Commit Message into a newly-allocated CommitMsg structure */
CommitMsg parse_commit(const char *msg)
{
    CommitMsg cmsg = NULL;
    size_t lenp;
    unsigned char *raw = decode(msg, &lenp);
    unsigned char *bufp = raw;
    if (!raw) goto inv;

    cmsg = calloc(1, sizeof(struct s_CommitMsg));
    if (!cmsg) {
	free(raw);
	goto inv;
    }

    cmsg->raw = raw;

    require_len(3);
    if (memcmp(bufp, "\x00\x02\x02", 3)) goto inv;
    bufp += 3; lenp -= 3;

    read_int(cmsg->enckeylen);
    cmsg->enckey = malloc(cmsg->enckeylen);
    if (!cmsg->enckey && cmsg->enckeylen > 0) goto inv;
    read_raw(cmsg->enckey, cmsg->enckeylen);

    read_int(cmsg->hashkeylen);
    cmsg->hashkey = malloc(cmsg->hashkeylen);
    if (!cmsg->hashkey && cmsg->hashkeylen > 0) goto inv;
    read_raw(cmsg->hashkey, cmsg->hashkeylen);

    if (lenp != 0) goto inv;

    return cmsg;
inv:
    free_commit(cmsg);
    return NULL;
}

/* Deallocate a CommitMsg and all of the data it points to */
void free_commit(CommitMsg cmsg)
{
    if (!cmsg) return;
    free(cmsg->raw);
    free(cmsg->enckey);
    free(cmsg->hashkey);
    free(cmsg);
}

/* Parse a D-H Key Message into a newly-allocated KeyMsg structure */
KeyMsg parse_key(const char *msg)
{
    KeyMsg kmsg = NULL;
    size_t lenp;
    unsigned char *raw = decode(msg, &lenp);
    unsigned char *bufp = raw;
    if (!raw) goto inv;

    kmsg = calloc(1, sizeof(struct s_KeyMsg));
    if (!kmsg) {
	free(raw);
	goto inv;
    }

    kmsg->raw = raw;

    require_len(3);
    if (memcmp(bufp, "\x00\x02\x0a", 3)) goto inv;
    bufp += 3; lenp -= 3;

    read_mpi(kmsg->y);

    if (lenp != 0) goto inv;

    return kmsg;
inv:
    free_key(kmsg);
    return NULL;
}

/* Deallocate a KeyMsg and all of the data it points to */
void free_key(KeyMsg kmsg)
{
    if (!kmsg) return;
    free(kmsg->raw);
    gcry_mpi_release(kmsg->y);
    free(kmsg);
}

/* Parse a Reveal Signature Message into a newly-allocated RevealSigMsg
 * structure */
RevealSigMsg parse_revealsig(const char *msg)
{
    RevealSigMsg rmsg = NULL;
    size_t lenp;
    unsigned char *raw = decode(msg, &lenp);
    unsigned char *bufp = raw;
    if (!raw) goto inv;

    rmsg = calloc(1, sizeof(struct s_RevealSigMsg));
    if (!rmsg) {
	free(raw);
	goto inv;
    }

    rmsg->raw = raw;

    require_len(3);
    if (memcmp(bufp, "\x00\x02\x11", 3)) goto inv;
    bufp += 3; lenp -= 3;

    read_int(rmsg->keylen);
    rmsg->key = malloc(rmsg->keylen);
    if (!rmsg->key && rmsg->keylen > 0) goto inv;
    read_raw(rmsg->key, rmsg->keylen);

    read_int(rmsg->encsiglen);
    rmsg->encsig = malloc(rmsg->encsiglen);
    if (!rmsg->encsig && rmsg->encsiglen > 0) goto inv;
    read_raw(rmsg->encsig, rmsg->encsiglen);

    read_raw(rmsg->mac, 20);

    if (lenp != 0) goto inv;

    return rmsg;
inv:
    free_revealsig(rmsg);
    return NULL;
}

/* Deallocate a RevealSigMsg and all of the data it points to */
void free_revealsig(RevealSigMsg rmsg)
{
    if (!rmsg) return;
    free(rmsg->raw);
    free(rmsg->key);
    free(rmsg->encsig);
    free(rmsg);
}

/* Parse a Signature Message into a newly-allocated SignatureMsg structure */
SignatureMsg parse_signature(const char *msg)
{
    SignatureMsg smsg = NULL;
    size_t lenp;
    unsigned char *raw = decode(msg, &lenp);
    unsigned char *bufp = raw;
    if (!raw) goto inv;

    smsg = calloc(1, sizeof(struct s_SignatureMsg));
    if (!smsg) {
	free(raw);
	goto inv;
    }

    smsg->raw = raw;

    require_len(3);
    if (memcmp(bufp, "\x00\x02\x12", 3)) goto inv;
    bufp += 3; lenp -= 3;

    read_int(smsg->encsiglen);
    smsg->encsig = malloc(smsg->encsiglen);
    if (!smsg->encsig && smsg->encsiglen > 0) goto inv;
    read_raw(smsg->encsig, smsg->encsiglen);

    read_raw(smsg->mac, 20);

    if (lenp != 0) goto inv;

    return smsg;
inv:
    free_signature(smsg);
    return NULL;
}

/* Deallocate a SignatureMsg and all of the data it points to */
void free_signature(SignatureMsg smsg)
{
    if (!smsg) return;
    free(smsg->raw);
    free(smsg->encsig);
    free(smsg);
}

/* Parse a Data Message into a newly-allocated DataMsg structure */
DataMsg parse_datamsg(const char *msg)
{
    DataMsg datam = NULL;
    size_t lenp;
    unsigned char *raw = decode(msg, &lenp);
    unsigned char *bufp = raw;
    unsigned char version;
    if (!raw) goto inv;

    datam = calloc(1, sizeof(struct s_DataMsg));
    if (!datam) {
	free(raw);
	goto inv;
    }

    datam->raw = raw;
    datam->rawlen = lenp;
    datam->macstart = bufp;

    require_len(3);
    if (memcmp(bufp, "\x00\x01\x03", 3) && memcmp(bufp, "\x00\x02\x03", 3))
	goto inv;
    version = bufp[1];
    bufp += 3; lenp -= 3;

    if (version == 2) {
	require_len(1);
	datam->flags = bufp[0];
	bufp += 1; lenp -= 1;
    } else {
	datam->flags = -1;
    }
    read_int(datam->sender_keyid);
    read_int(datam->rcpt_keyid);
    read_mpi(datam->y);
    read_raw(datam->ctr, 8);
    read_int(datam->encmsglen);
    datam->encmsg = malloc(datam->encmsglen);
    if (!datam->encmsg && datam->encmsglen > 0) goto inv;
    read_raw(datam->encmsg, datam->encmsglen);
    datam->macend = bufp;
    read_raw(datam->mac, 20);
    read_int(datam->mackeyslen);
    datam->mackeys = malloc(datam->mackeyslen);
    if (!datam->mackeys && datam->mackeyslen > 0) goto inv;
    read_raw(datam->mackeys, datam->mackeyslen);

    if (lenp != 0) goto inv;

    return datam;
inv:
    free_datamsg(datam);
    return NULL;
}

/* Recalculate the MAC on the message, base64-encode the resulting MAC'd
 * message, and put on the appropriate header and footer.  Return a
 * newly-allocated pointer to the result, which the caller will have to
 * free(). */
char *remac_datamsg(DataMsg datamsg, unsigned char mackey[20])
{
    size_t rawlen, lenp;
    size_t ylen;
    size_t base64len;
    char *outmsg;
    unsigned char *raw, *bufp;
    unsigned char version = (datamsg->flags >= 0 ? 2 : 1);
    
    /* Calculate the size of the message that will result */
    gcry_mpi_print(GCRYMPI_FMT_USG, NULL, 0, &ylen, datamsg->y);
    rawlen = 3 + (version == 2 ? 1 : 0) + 4 + 4 + 4 + ylen + 8 + 4 +
	datamsg->encmsglen + 20 + 4 + datamsg->mackeyslen;

    /* Construct the new raw message (note that some of the pieces may
     * have been altered, so we construct it from scratch). */
    raw = malloc(rawlen);
    if (!raw) {
	fprintf(stderr, "Out of memory!\n");
	exit(1);
    }
    bufp = raw;
    lenp = rawlen;
    datamsg->macstart = raw;
    datamsg->macend = NULL;
    free(datamsg->raw);
    datamsg->raw = raw;
    datamsg->rawlen = rawlen;

    if (version == 1) {
	memmove(bufp, "\x00\x01\x03", 3);
    } else {
	memmove(bufp, "\x00\x02\x03", 3);
    }
    bufp += 3; lenp -= 3;
    if (version == 2) {
	bufp[0] = datamsg->flags;
	bufp += 1; lenp -= 1;
    }
    write_int(datamsg->sender_keyid);
    write_int(datamsg->rcpt_keyid);
    write_mpi(datamsg->y, ylen);
    write_raw(datamsg->ctr, 8);
    write_int(datamsg->encmsglen);
    write_raw(datamsg->encmsg, datamsg->encmsglen);
    datamsg->macend = bufp;

    /* Recalculate the MAC */
    sha1hmac(datamsg->mac, mackey, datamsg->macstart,
	    datamsg->macend - datamsg->macstart);

    write_raw(datamsg->mac, 20);
    write_int(datamsg->mackeyslen);
    write_raw(datamsg->mackeys, datamsg->mackeyslen);
    
    if (lenp != 0) {
	fprintf(stderr, "Error creating OTR Data Message.\n");
	exit(1);
    }

    base64len = 5 + ((datamsg->rawlen + 2) / 3) * 4 + 1 + 1;
    outmsg = malloc(base64len);
    if (!outmsg) return NULL;

    memmove(outmsg, "?OTR:", 5);
    otrl_base64_encode(outmsg + 5, datamsg->raw, datamsg->rawlen);
    strcpy(outmsg + base64len - 2, ".");
    return outmsg;
}

/* Assemble a new Data Message from its pieces.  Return a
 * newly-allocated string containing the base64 representation. */
char *assemble_datamsg(unsigned char mackey[20], int flags,
	unsigned int sender_keyid, unsigned int rcpt_keyid, gcry_mpi_t y,
	unsigned char ctr[8], unsigned char *encmsg, size_t encmsglen,
	unsigned char *mackeys, size_t mackeyslen)
{
    DataMsg datam = calloc(1, sizeof(struct s_DataMsg));
    char *newmsg = NULL;
    if (!datam) goto inv;
    datam->flags = flags;
    datam->sender_keyid = sender_keyid;
    datam->rcpt_keyid = rcpt_keyid;
    datam->y = gcry_mpi_copy(y);
    memmove(datam->ctr, ctr, 8);
    datam->encmsg = malloc(encmsglen);
    if (!datam->encmsg && encmsglen > 0) goto inv;
    memmove(datam->encmsg, encmsg, encmsglen);
    datam->encmsglen = encmsglen;
    datam->mackeys = malloc(mackeyslen);
    if (!datam->mackeys && mackeyslen > 0) goto inv;
    memmove(datam->mackeys, mackeys, mackeyslen);
    datam->mackeyslen = mackeyslen;

    /* Recalculate the MAC and base64-encode the result */
    newmsg = remac_datamsg(datam, mackey);
    free_datamsg(datam);
    return newmsg;
inv:
    free_datamsg(datam);
    return NULL;
}

/* Deallocate a DataMsg and all of the data it points to */
void free_datamsg(DataMsg datamsg)
{
    if (!datamsg) return;
    free(datamsg->raw);
    gcry_mpi_release(datamsg->y);
    free(datamsg->encmsg);
    free(datamsg->mackeys);
    free(datamsg);
}

static int ctoh(char c)
{
    if (c >= '0' && c <= '9') return (c-'0');
    if (c >= 'a' && c <= 'f') return (c-'a'+10);
    if (c >= 'A' && c <= 'F') return (c-'A'+10);
    return -1;
}

/* Convert a string of hex chars to a buffer of unsigned chars. */
void argv_to_buf(unsigned char **bufp, size_t *lenp, char *arg)
{
    unsigned char *buf;
    size_t len, i;

    *bufp = NULL;
    *lenp = 0;

    len = strlen(arg);
    if (len % 2) {
	fprintf(stderr, "Argument ``%s'' must have even length.\n", arg);
	return;
    }
    buf = malloc(len/2);
    if (buf == NULL && len > 0) {
	fprintf(stderr, "Out of memory!\n");
	return;
    }

    for(i=0;i<len/2;++i) {
	int hi = ctoh(arg[2*i]);
	int lo = ctoh(arg[2*i+1]);
	if (hi < 0 || lo < 0) {
	    free(buf);
	    fprintf(stderr, "Illegal hex char in argument ``%s''.\n", arg);
	    return;
	}
	buf[i] = (hi << 4) + lo;
    }
    *bufp = buf;
    *lenp = len/2;
}
