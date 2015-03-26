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

/* OTR Protocol implementation.  This file should be independent of
 * gaim, so that it can be used to make other clients. */

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "b64.h"
#include "privkey.h"
#include "proto.h"
#include "mem.h"
#include "version.h"
#include "tlv.h"
#include "serial.h"

#define snprintf _snprintf /* Miranda NG modification */

#if OTRL_DEBUGGING
extern const char *OTRL_DEBUGGING_DEBUGSTR;
#endif

/* For now, we need to know the API version the client is using so that
 * we don't use any UI callbacks it hasn't set. */
unsigned int otrl_api_version = 0;

/* Initialize the OTR library.  Pass the version of the API you are
 * using. */
gcry_error_t otrl_init(unsigned int ver_major, unsigned int ver_minor,
	unsigned int ver_sub)
{
    unsigned int api_version;

    /* The major versions have to match, and you can't be using a newer
     * minor version than we expect. */
    if (ver_major != OTRL_VERSION_MAJOR || ver_minor > OTRL_VERSION_MINOR) {
	fprintf(stderr, "Expected libotr API version %u.%u.%u incompatible "
		"with actual version %u.%u.%u.  Aborting.\n",
		ver_major, ver_minor, ver_sub,
		OTRL_VERSION_MAJOR, OTRL_VERSION_MINOR, OTRL_VERSION_SUB);
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Set the API version.  If we get called multiple times for some
     * reason, take the smallest value. */
    api_version = (ver_major << 16) | (ver_minor << 8) | (ver_sub);
    if (otrl_api_version == 0 || otrl_api_version > api_version) {
	otrl_api_version = api_version;
    }

    /* Initialize the memory module */
    otrl_mem_init();

    /* Initialize the DH module */
    otrl_dh_init();

    /* Initialize the SM module */
    otrl_sm_init();

#if OTRL_DEBUGGING
    /* Inform the user that debugging is available */
    fprintf(stderr, "\nlibotr debugging is available.  Type %s in a message\n"
	    "  to see debug info.\n\n", OTRL_DEBUGGING_DEBUGSTR);
#endif

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Return a pointer to a static string containing the version number of
 * the OTR library. */
const char *otrl_version(void)
{
    return OTRL_VERSION;
}

/* Store some MAC keys to be revealed later */
static gcry_error_t reveal_macs(ConnContext *context,
	DH_sesskeys *sess1, DH_sesskeys *sess2)
{
    unsigned int numnew = sess1->rcvmacused + sess1->sendmacused +
	sess2->rcvmacused + sess2->sendmacused;
    unsigned int newnumsaved;
    unsigned char *newmacs;

    /* Is there anything to do? */
    if (numnew == 0) return gcry_error(GPG_ERR_NO_ERROR);

    newnumsaved = context->context_priv->numsavedkeys + numnew;
    newmacs = realloc(context->context_priv->saved_mac_keys,
	    newnumsaved * 20);
    if (!newmacs) {
	return gcry_error(GPG_ERR_ENOMEM);
    }
    if (sess1->rcvmacused) {
	memmove(newmacs + context->context_priv->numsavedkeys * 20,
		sess1->rcvmackey, 20);
	context->context_priv->numsavedkeys++;
    }
    if (sess1->sendmacused) {
	memmove(newmacs + context->context_priv->numsavedkeys * 20,
		sess1->sendmackey, 20);
	context->context_priv->numsavedkeys++;
    }
    if (sess2->rcvmacused) {
	memmove(newmacs + context->context_priv->numsavedkeys * 20,
		sess2->rcvmackey, 20);
	context->context_priv->numsavedkeys++;
    }
    if (sess2->sendmacused) {
	memmove(newmacs + context->context_priv->numsavedkeys * 20,
		sess2->sendmackey, 20);
	context->context_priv->numsavedkeys++;
    }
    context->context_priv->saved_mac_keys = newmacs;

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Make a new DH key for us, and rotate old old ones.  Be sure to keep
 * the sesskeys array in sync. */
static gcry_error_t rotate_dh_keys(ConnContext *context)
{
    gcry_error_t err;

    /* Rotate the keypair */
    otrl_dh_keypair_free(&(context->context_priv->our_old_dh_key));
    memmove(&(context->context_priv->our_old_dh_key),
	    &(context->context_priv->our_dh_key),
	    sizeof(DH_keypair));

    /* Rotate the session keys */
    err = reveal_macs(context, &(context->context_priv->sesskeys[1][0]),
	    &(context->context_priv->sesskeys[1][1]));
    if (err) return err;
    otrl_dh_session_free(&(context->context_priv->sesskeys[1][0]));
    otrl_dh_session_free(&(context->context_priv->sesskeys[1][1]));
    memmove(&(context->context_priv->sesskeys[1][0]),
	    &(context->context_priv->sesskeys[0][0]),
	    sizeof(DH_sesskeys));
    memmove(&(context->context_priv->sesskeys[1][1]),
	    &(context->context_priv->sesskeys[0][1]),
	    sizeof(DH_sesskeys));

    /* Create a new DH key */
    otrl_dh_gen_keypair(DH1536_GROUP_ID, &(context->context_priv->our_dh_key));
    context->context_priv->our_keyid++;

    /* Make the session keys */
    if (context->context_priv->their_y) {
	err = otrl_dh_session(&(context->context_priv->sesskeys[0][0]),
		&(context->context_priv->our_dh_key),
		context->context_priv->their_y);
	if (err) return err;
    } else {
	otrl_dh_session_blank(&(context->context_priv->sesskeys[0][0]));
    }
    if (context->context_priv->their_old_y) {
	err = otrl_dh_session(&(context->context_priv->sesskeys[0][1]),
		&(context->context_priv->our_dh_key),
		context->context_priv->their_old_y);
	if (err) return err;
    } else {
	otrl_dh_session_blank(&(context->context_priv->sesskeys[0][1]));
    }
    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Rotate in a new DH public key for our correspondent.  Be sure to keep
 * the sesskeys array in sync. */
static gcry_error_t rotate_y_keys(ConnContext *context, gcry_mpi_t new_y)
{
    gcry_error_t err;

    /* Rotate the public key */
    gcry_mpi_release(context->context_priv->their_old_y);
    context->context_priv->their_old_y = context->context_priv->their_y;

    /* Rotate the session keys */
    err = reveal_macs(context, &(context->context_priv->sesskeys[0][1]),
	    &(context->context_priv->sesskeys[1][1]));
    if (err) return err;
    otrl_dh_session_free(&(context->context_priv->sesskeys[0][1]));
    otrl_dh_session_free(&(context->context_priv->sesskeys[1][1]));
    memmove(&(context->context_priv->sesskeys[0][1]),
	    &(context->context_priv->sesskeys[0][0]),
	    sizeof(DH_sesskeys));
    memmove(&(context->context_priv->sesskeys[1][1]),
	    &(context->context_priv->sesskeys[1][0]),
	    sizeof(DH_sesskeys));

    /* Copy in the new public key */
    context->context_priv->their_y = gcry_mpi_copy(new_y);
    context->context_priv->their_keyid++;

    /* Make the session keys */
    err = otrl_dh_session(&(context->context_priv->sesskeys[0][0]),
	    &(context->context_priv->our_dh_key),
	    context->context_priv->their_y);
    if (err) return err;
    err = otrl_dh_session(&(context->context_priv->sesskeys[1][0]),
	    &(context->context_priv->our_old_dh_key),
	    context->context_priv->their_y);
    if (err) return err;

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Return a pointer to a newly-allocated OTR query message, customized
 * with our name.  The caller should free() the result when he's done
 * with it. */
char *otrl_proto_default_query_msg(const char *ourname, OtrlPolicy policy)
{
    char *msg;
    int v1_supported, v2_supported, v3_supported;
    char *version_tag;
    char *bufp;
    /* Don't use g_strdup_printf here, because someone (not us) is going
     * to free() the *message pointer, not g_free() it.  We can't
     * require that they g_free() it, because this pointer will probably
     * get passed to the main IM application for processing (and
     * free()ing). */
    const char *format = "?OTR%s\n<b>%s</b> has requested an "
	    "<a href=\"https://otr.cypherpunks.ca/\">Off-the-Record "
	    "private conversation</a>.  However, you do not have a plugin "
	    "to support that.\nSee <a href=\"https://otr.cypherpunks.ca/\">"
	    "https://otr.cypherpunks.ca/</a> for more information.";

    /* Figure out the version tag */
    v1_supported = (policy & OTRL_POLICY_ALLOW_V1);
    v2_supported = (policy & OTRL_POLICY_ALLOW_V2);
    v3_supported = (policy & OTRL_POLICY_ALLOW_V3);
    version_tag = malloc(8);
    bufp = version_tag;
    if (v1_supported) {
	*bufp = '?';
	bufp++;
    }
    if (v2_supported || v3_supported) {
	*bufp = 'v';
	bufp++;
	if (v2_supported) {
	    *bufp = '2';
	    bufp++;
	}
	if (v3_supported) {
	    *bufp = '3';
	    bufp++;
	}
	*bufp = '?';
	bufp++;
    }
    *bufp = '\0';

    /* Remove two "%s", add '\0' */
    msg = malloc(strlen(format) + strlen(version_tag) + strlen(ourname) - 3);
    if (!msg) {
	free(version_tag);
	return NULL;
    }
    sprintf(msg, format, version_tag, ourname);
    free(version_tag);
    return msg;
}

/* Return the best version of OTR support by both sides, given an OTR
 * Query Message and the local policy. */
unsigned int otrl_proto_query_bestversion(const char *otrquerymsg,
	OtrlPolicy policy)
{
    char *otrtag;
    unsigned int query_versions = 0;


    otrtag = strstr(otrquerymsg, "?OTR");
    if (!otrtag) {
	return 0;
    }
    otrtag += 4;

    if (*otrtag == '?') {
	query_versions = (1<<0);
	++otrtag;
    }
    if (*otrtag == 'v') {
	for(++otrtag; *otrtag && *otrtag != '?'; ++otrtag) {
	    switch(*otrtag) {
		case '2':
		    query_versions |= (1<<1);
		    break;
		case '3':
		    query_versions |= (1<<2);
		    break;
	    }
	}
    }

    if ((policy & OTRL_POLICY_ALLOW_V3) && (query_versions & (1<<2))) {
	return 3;
    }
    if ((policy & OTRL_POLICY_ALLOW_V2) && (query_versions & (1<<1))) {
	return 2;
    }
    if ((policy & OTRL_POLICY_ALLOW_V1) && (query_versions & (1<<0))) {
	return 1;
    }
    return 0;
}

/* Locate any whitespace tag in this message, and return the best
 * version of OTR support on both sides.  Set *starttagp and *endtagp to
 * the start and end of the located tag, so that it can be snipped out. */
unsigned int otrl_proto_whitespace_bestversion(const char *msg,
	const char **starttagp, const char **endtagp, OtrlPolicy policy)
{
    const char *starttag, *endtag;
    unsigned int query_versions = 0;

    *starttagp = NULL;
    *endtagp = NULL;

    starttag = strstr(msg, OTRL_MESSAGE_TAG_BASE);
    if (!starttag) return 0;

    endtag = starttag + strlen(OTRL_MESSAGE_TAG_BASE);

    /* Look for groups of 8 spaces and/or tabs */
    while(1) {
	int i;
	int allwhite = 1;
	for(i=0;i<8;++i) {
	    if (endtag[i] != ' ' && endtag[i] != '\t') {
		allwhite = 0;
		break;
	    }
	}
	if (allwhite) {
	    if (!strncmp(endtag, OTRL_MESSAGE_TAG_V1, 8)) {
		query_versions |= (1<<0);
	    }
	    if (!strncmp(endtag, OTRL_MESSAGE_TAG_V2, 8)) {
		query_versions |= (1<<1);
	    }
	    if (!strncmp(endtag, OTRL_MESSAGE_TAG_V3, 8)) {
		query_versions |= (1<<2);
	    }
	    endtag += 8;
	} else {
	    break;
	}
    }

    *starttagp = starttag;
    *endtagp = endtag;

    if ((policy & OTRL_POLICY_ALLOW_V3) && (query_versions & (1<<2))) {
	return 3;
    }
    if ((policy & OTRL_POLICY_ALLOW_V2) && (query_versions & (1<<1))) {
	return 2;
    }
    if ((policy & OTRL_POLICY_ALLOW_V1) && (query_versions & (1<<0))) {
	return 1;
    }
    return 0;
}

/* Find the message type. */
OtrlMessageType otrl_proto_message_type(const char *message)
{
    char *otrtag;

    otrtag = strstr(message, "?OTR");

    if (!otrtag) {
	if (strstr(message, OTRL_MESSAGE_TAG_BASE)) {
	    return OTRL_MSGTYPE_TAGGEDPLAINTEXT;
	} else {
	    return OTRL_MSGTYPE_NOTOTR;
	}
    }

    if (!strncmp(otrtag, "?OTR:AAM", 8) || !strncmp(otrtag, "?OTR:AAI", 8)) {
	switch(*(otrtag + 8)) {
	    case 'C': return OTRL_MSGTYPE_DH_COMMIT;
	    case 'K': return OTRL_MSGTYPE_DH_KEY;
	    case 'R': return OTRL_MSGTYPE_REVEALSIG;
	    case 'S': return OTRL_MSGTYPE_SIGNATURE;
	    case 'D': return OTRL_MSGTYPE_DATA;
	}
    } else {
	if (!strncmp(otrtag, "?OTR?", 5)) return OTRL_MSGTYPE_QUERY;
	if (!strncmp(otrtag, "?OTRv", 5)) return OTRL_MSGTYPE_QUERY;
	if (!strncmp(otrtag, "?OTR:AAEK", 9)) return OTRL_MSGTYPE_V1_KEYEXCH;
	if (!strncmp(otrtag, "?OTR:AAED", 9)) return OTRL_MSGTYPE_DATA;
	if (!strncmp(otrtag, "?OTR Error:", 11)) return OTRL_MSGTYPE_ERROR;
    }
    return OTRL_MSGTYPE_UNKNOWN;
}

/* Find the message version. */
int otrl_proto_message_version(const char *message)
{
    char *otrtag;

    otrtag = strstr(message, "?OTR");

    if (!otrtag) {
	return 0;
    }

    if (!strncmp(otrtag, "?OTR:AAM", 8))
	return 3;
    if (!strncmp(otrtag, "?OTR:AAI", 8))
	return 2;
    if (!strncmp(otrtag, "?OTR:AAE", 8))
	return 1;

    return 0;
}

/* Find the instance tags in this message */
gcry_error_t otrl_proto_instance(const char *otrmsg,
	unsigned int *instance_from, unsigned int *instance_to)
{
    gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);

    const char *otrtag = otrmsg;
    unsigned char *bufp = NULL;
    unsigned char *bufp_head = NULL;
    size_t lenp;

    if (!otrtag || strncmp(otrtag, "?OTR:AAM", 8)) {
	goto invval;
    }

    if (strlen(otrtag) < 21 ) goto invval;

    /* Decode and extract instance tag */
    bufp = malloc(OTRL_B64_MAX_DECODED_SIZE(12));
    bufp_head = bufp;
    lenp = otrl_base64_decode(bufp, otrtag+9, 12);
    read_int(*instance_from);
    read_int(*instance_to);
    free(bufp_head);
    return gcry_error(GPG_ERR_NO_ERROR);
invval:
    free(bufp_head);
    err = gcry_error(GPG_ERR_INV_VALUE);
    return err;
}

/* Create an OTR Data message.  Pass the plaintext as msg, and an
 * optional chain of TLVs.  A newly-allocated string will be returned in
 * *encmessagep. Put the current extra symmetric key into extrakey
 * (if non-NULL). */
gcry_error_t otrl_proto_create_data(char **encmessagep, ConnContext *context,
	const char *msg, const OtrlTLV *tlvs, unsigned char flags,
	unsigned char *extrakey)
{
    size_t justmsglen = strlen(msg);
    size_t msglen = justmsglen + 1 + otrl_tlv_seriallen(tlvs);
    size_t buflen;
    size_t pubkeylen;
    unsigned char *buf = NULL;
    unsigned char *bufp;
    size_t lenp;
    DH_sesskeys *sess = &(context->context_priv->sesskeys[1][0]);
    gcry_error_t err;
    size_t reveallen = 20 * context->context_priv->numsavedkeys;
    char *base64buf = NULL;
    unsigned char *msgbuf = NULL;
    enum gcry_mpi_format format = GCRYMPI_FMT_USG;
    char *msgdup;
    int version = context->protocol_version;

    /* Make sure we're actually supposed to be able to encrypt */
    if (context->msgstate != OTRL_MSGSTATE_ENCRYPTED ||
	    context->context_priv->their_keyid == 0) {
	return gcry_error(GPG_ERR_CONFLICT);
    }

    /* We need to copy the incoming msg, since it might be an alias for
     * context->lastmessage, which we'll be freeing soon. */
    msgdup = gcry_malloc_secure(justmsglen + 1);
    if (msgdup == NULL) {
	return gcry_error(GPG_ERR_ENOMEM);
    }
    strcpy(msgdup, msg);

    *encmessagep = NULL;

    /* Header, msg flags, send keyid, recv keyid, counter, msg len, msg
     * len of revealed mac keys, revealed mac keys, MAC */
    buflen = OTRL_HEADER_LEN + (version == 3 ? 8 : 0)
	+ (version == 2 || version == 3 ? 1 : 0) + 4 + 4
	+ 8 + 4 + msglen + 4 + reveallen + 20;
    gcry_mpi_print(format, NULL, 0, &pubkeylen,
	    context->context_priv->our_dh_key.pub);
    buflen += pubkeylen + 4;
    buf = malloc(buflen);
    msgbuf = gcry_malloc_secure(msglen);
    if (buf == NULL || msgbuf == NULL) {
	free(buf);
	gcry_free(msgbuf);
	gcry_free(msgdup);
	return gcry_error(GPG_ERR_ENOMEM);
    }
    memmove(msgbuf, msgdup, justmsglen);
    msgbuf[justmsglen] = '\0';
    otrl_tlv_serialize(msgbuf + justmsglen + 1, tlvs);
    bufp = buf;
    lenp = buflen;
    if (version == 1) {
	memmove(bufp, "\x00\x01\x03", 3);  /* header */
    } else if (version == 2) {
	memmove(bufp, "\x00\x02\x03", 3);  /* header */
    } else {
	memmove(bufp, "\x00\x03\x03", 3);  /* header */
    }

    debug_data("Header", bufp, 3);
    bufp += 3; lenp -= 3;

    if (version == 3) {
	/* v3 instance tags */
	write_int(context->our_instance);
	debug_int("Sender instag", bufp-4);
	write_int(context->their_instance);
	debug_int("Recipient instag", bufp-4);
    }

    if (version == 2 || version == 3) {
	bufp[0] = flags;
	bufp += 1; lenp -= 1;
    }

    write_int(context->context_priv->our_keyid-1); /* sender keyid */
    debug_int("Sender keyid", bufp-4);
    write_int(context->context_priv->their_keyid); /* recipient keyid */
    debug_int("Recipient keyid", bufp-4);

    write_mpi(context->context_priv->our_dh_key.pub, pubkeylen, "Y");  /* Y */

    otrl_dh_incctr(sess->sendctr);
    memmove(bufp, sess->sendctr, 8);      /* Counter (top 8 bytes only) */
    debug_data("Counter", bufp, 8);
    bufp += 8; lenp -= 8;

    write_int(msglen);                        /* length of encrypted data */
    debug_int("Msg len", bufp-4);

    err = gcry_cipher_reset(sess->sendenc);
    if (err) goto err;
    err = gcry_cipher_setctr(sess->sendenc, sess->sendctr, 16);
    if (err) goto err;
    err = gcry_cipher_encrypt(sess->sendenc, bufp, msglen, msgbuf, msglen);
    if (err) goto err;                              /* encrypted data */
    debug_data("Enc data", bufp, msglen);
    bufp += msglen;
    lenp -= msglen;

    gcry_md_reset(sess->sendmac);
    gcry_md_write(sess->sendmac, buf, bufp-buf);
    memmove(bufp, gcry_md_read(sess->sendmac, GCRY_MD_SHA1), 20);
    debug_data("MAC", bufp, 20);
    bufp += 20;                                         /* MAC */
    lenp -= 20;

    write_int(reveallen);                     /* length of revealed MAC keys */
    debug_int("Revealed MAC length", bufp-4);

    if (reveallen > 0) {
	memmove(bufp, context->context_priv->saved_mac_keys, reveallen);
	debug_data("Revealed MAC data", bufp, reveallen);
	bufp += reveallen; lenp -= reveallen;
	free(context->context_priv->saved_mac_keys);
	context->context_priv->saved_mac_keys = NULL;
	context->context_priv->numsavedkeys = 0;
    }

    assert(lenp == 0);

    /* Make the base64-encoding. */
    base64buf = otrl_base64_otr_encode(buf, buflen);
    if (base64buf == NULL) {
	err = gcry_error(GPG_ERR_ENOMEM);
	goto err;
    }

    free(buf);
    gcry_free(msgbuf);
    *encmessagep = base64buf;
    gcry_free(context->context_priv->lastmessage);
    context->context_priv->lastmessage = NULL;
    context->context_priv->may_retransmit = 0;
    if (msglen > 0) {
	context->context_priv->lastmessage = gcry_malloc_secure(justmsglen + 1);
	if (context->context_priv->lastmessage) {
	    strcpy(context->context_priv->lastmessage, msgdup);
	}
    }
    gcry_free(msgdup);

    /* Save a copy of the current extra key */
    if (extrakey) {
	memmove(extrakey, sess->extrakey, OTRL_EXTRAKEY_BYTES);
    }

    return gcry_error(GPG_ERR_NO_ERROR);
err:
    free(buf);
    gcry_free(msgbuf);
    gcry_free(msgdup);
    *encmessagep = NULL;
    return err;
}

/* Extract the flags from an otherwise unreadable Data Message. */
gcry_error_t otrl_proto_data_read_flags(const char *datamsg,
	unsigned char *flagsp)
{
    char *otrtag, *endtag;
    unsigned char *rawmsg = NULL;
    unsigned char *bufp;
    size_t msglen, rawlen, lenp;
    unsigned char version;

    if (flagsp) *flagsp = 0;
    otrtag = strstr(datamsg, "?OTR:");
    if (!otrtag) {
	goto invval;
    }
    endtag = strchr(otrtag, '.');
    if (endtag) {
	msglen = endtag-otrtag;
    } else {
	msglen = strlen(otrtag);
    }

    /* Skip over the "?OTR:" */
    otrtag += 5;
    msglen -= 5;

    /* Base64-decode the message */
    rawlen = OTRL_B64_MAX_DECODED_SIZE(msglen);   /* maximum possible */
    rawmsg = malloc(rawlen);
    if (!rawmsg && rawlen > 0) {
	return gcry_error(GPG_ERR_ENOMEM);
    }
    rawlen = otrl_base64_decode(rawmsg, otrtag, msglen);  /* actual size */

    bufp = rawmsg;
    lenp = rawlen;

    require_len(3);
    version = bufp[1];
    skip_header('\x03');

    if (version == 3) {
	require_len(8);
	bufp += 8; lenp -= 8;
    }

    if (version == 2 || version == 3) {
	require_len(1);
	if (flagsp) *flagsp = bufp[0];
	bufp += 1; lenp -= 1;
    }

    free(rawmsg);
    return gcry_error(GPG_ERR_NO_ERROR);

invval:
    free(rawmsg);
    return gcry_error(GPG_ERR_INV_VALUE);
}

/* Accept an OTR Data Message in datamsg.  Decrypt it and put the
 * plaintext into *plaintextp, and any TLVs into tlvsp.  Put any
 * received flags into *flagsp (if non-NULL).  Put the current extra
 * symmetric key into extrakey (if non-NULL). */
gcry_error_t otrl_proto_accept_data(char **plaintextp, OtrlTLV **tlvsp,
	ConnContext *context, const char *datamsg, unsigned char *flagsp,
	unsigned char *extrakey)
{
    char *otrtag, *endtag;
    gcry_error_t err;
    unsigned char *rawmsg = NULL;
    size_t msglen, rawlen, lenp;
    unsigned char *macstart, *macend;
    unsigned char *bufp;
    unsigned int sender_keyid, recipient_keyid;
    gcry_mpi_t sender_next_y = NULL;
    unsigned char ctr[8];
    unsigned int datalen, reveallen;
    unsigned char *data = NULL;
    unsigned char *nul = NULL;
    unsigned char givenmac[20];
    DH_sesskeys *sess;
    unsigned char version;

    *plaintextp = NULL;
    *tlvsp = NULL;
    if (flagsp) *flagsp = 0;
    otrtag = strstr(datamsg, "?OTR:");
    if (!otrtag) {
	goto invval;
    }
    endtag = strchr(otrtag, '.');
    if (endtag) {
	msglen = endtag-otrtag;
    } else {
	msglen = strlen(otrtag);
    }

    /* Skip over the "?OTR:" */
    otrtag += 5;
    msglen -= 5;

    /* Base64-decode the message */
    rawlen = OTRL_B64_MAX_DECODED_SIZE(msglen);   /* maximum possible */
    rawmsg = malloc(rawlen);
    if (!rawmsg && rawlen > 0) {
	err = gcry_error(GPG_ERR_ENOMEM);
	goto err;
    }
    rawlen = otrl_base64_decode(rawmsg, otrtag, msglen);  /* actual size */

    bufp = rawmsg;
    lenp = rawlen;

    macstart = bufp;
    require_len(3);
    version = bufp[1];

    skip_header('\x03');

    if (version == 3) {
	require_len(8);
	bufp += 8; lenp -= 8;
    }

    if (version == 2 || version == 3) {
	require_len(1);
	if (flagsp) *flagsp = bufp[0];
	bufp += 1; lenp -= 1;
    }

    read_int(sender_keyid);
    read_int(recipient_keyid);
    read_mpi(sender_next_y);
    require_len(8);
    memmove(ctr, bufp, 8);
    bufp += 8; lenp -= 8;
    read_int(datalen);
    require_len(datalen);
    data = malloc(datalen+1);
    if (!data) {
	err = gcry_error(GPG_ERR_ENOMEM);
	goto err;
    }
    memmove(data, bufp, datalen);
    data[datalen] = '\0';
    bufp += datalen; lenp -= datalen;
    macend = bufp;
    require_len(20);
    memmove(givenmac, bufp, 20);
    bufp += 20; lenp -= 20;
    read_int(reveallen);
    require_len(reveallen);
    /* Just skip over the revealed MAC keys, which we don't need.  They
     * were published for deniability of transcripts. */
    bufp += reveallen; lenp -= reveallen;

    /* That should be everything */
    if (lenp != 0) goto invval;

    /* We don't take any action on this message (especially rotating
     * keys) until we've verified the MAC on this message.  To that end,
     * we need to know which keys this message is claiming to use. */
    if (context->context_priv->their_keyid == 0 ||
	    (sender_keyid != context->context_priv->their_keyid &&
		sender_keyid != context->context_priv->their_keyid - 1) ||
	    (recipient_keyid != context->context_priv->our_keyid &&
	     recipient_keyid != context->context_priv->our_keyid - 1) ||
	    sender_keyid == 0 || recipient_keyid == 0) {
	goto conflict;
    }

    if (sender_keyid == context->context_priv->their_keyid - 1 &&
	    context->context_priv->their_old_y == NULL) {
	goto conflict;
    }

    /* These are the session keys this message is claiming to use. */
    sess = &(context->context_priv->sesskeys
	    [context->context_priv->our_keyid - recipient_keyid]
	    [context->context_priv->their_keyid - sender_keyid]);

    gcry_md_reset(sess->rcvmac);
    gcry_md_write(sess->rcvmac, macstart, macend-macstart);
    if (otrl_mem_differ(givenmac, gcry_md_read(sess->rcvmac, GCRY_MD_SHA1),
	    20)) {
	/* The MACs didn't match! */
	goto conflict;
    }
    sess->rcvmacused = 1;

    /* Check to see that the counter is increasing; i.e. that this isn't
     * a replay. */
    if (otrl_dh_cmpctr(ctr, sess->rcvctr) <= 0) {
	goto conflict;
    }

    /* Decrypt the message */
    memmove(sess->rcvctr, ctr, 8);
    err = gcry_cipher_reset(sess->rcvenc);
    if (err) goto err;
    err = gcry_cipher_setctr(sess->rcvenc, sess->rcvctr, 16);
    if (err) goto err;
    err = gcry_cipher_decrypt(sess->rcvenc, data, datalen, NULL, 0);
    if (err) goto err;

    /* Save a copy of the current extra key */
    if (extrakey) {
	memmove(extrakey, sess->extrakey, OTRL_EXTRAKEY_BYTES);
    }

    /* See if either set of keys needs rotating */

    if (recipient_keyid == context->context_priv->our_keyid) {
	/* They're using our most recent key, so generate a new one */
	err = rotate_dh_keys(context);
	if (err) goto err;
    }

    if (sender_keyid == context->context_priv->their_keyid) {
	/* They've sent us a new public key */
	err = rotate_y_keys(context, sender_next_y);
	if (err) goto err;
    }

    gcry_mpi_release(sender_next_y);
    *plaintextp = (char *)data;

    /* See if there are TLVs */
    nul = data;
    while (nul < data+datalen && *nul) ++nul;
    /* If we stopped before the end, skip the NUL we stopped at */
    if (nul < data+datalen) ++nul;
    *tlvsp = otrl_tlv_parse(nul, (data+datalen)-nul);

    free(rawmsg);
    return gcry_error(GPG_ERR_NO_ERROR);

invval:
    err = gcry_error(GPG_ERR_INV_VALUE);
    goto err;
conflict:
    err = gcry_error(GPG_ERR_CONFLICT);
    goto err;
err:
    gcry_mpi_release(sender_next_y);
    free(data);
    free(rawmsg);
    return err;
}

/* Accumulate a potential fragment into the current context. */
OtrlFragmentResult otrl_proto_fragment_accumulate(char **unfragmessagep,
	ConnContext *context, const char *msg)
{
    OtrlFragmentResult res = OTRL_FRAGMENT_INCOMPLETE;
    const char *tag;
    unsigned short n = 0, k = 0;
    int start = 0, end = 0;

    tag = strstr(msg, "?OTR|");
    if (tag) {
	sscanf(tag, "?OTR|%*x|%*x,%hu,%hu,%n%*[^,],%n", &k, &n, &start, &end);
    } else if ((tag = strstr(msg, "?OTR,")) != NULL) {
	sscanf(tag, "?OTR,%hu,%hu,%n%*[^,],%n", &k, &n, &start, &end);
    } else {
	/* Unfragmented message, so discard any fragment we may have */
	free(context->context_priv->fragment);
	context->context_priv->fragment = NULL;
	context->context_priv->fragment_len = 0;
	context->context_priv->fragment_n = 0;
	context->context_priv->fragment_k = 0;
	res = OTRL_FRAGMENT_UNFRAGMENTED;
	return res;
    }

    if (k > 0 && n > 0 && k <= n && start > 0 && end > 0 && start < end) {
	if (k == 1) {
	    int fraglen = end - start - 1;
	    size_t newsize = fraglen + 1;
	    free(context->context_priv->fragment);
	    context->context_priv->fragment = NULL;
	    if (newsize >= 1) {  /* Check for overflow */
		context->context_priv->fragment = malloc(newsize);
	    }
	    if (context->context_priv->fragment) {
		memmove(context->context_priv->fragment, tag + start, fraglen);
		context->context_priv->fragment_len = fraglen;
		context->context_priv->fragment[
			context->context_priv->fragment_len] = '\0';
		context->context_priv->fragment_n = n;
		context->context_priv->fragment_k = k;
	    } else {
		context->context_priv->fragment_len = 0;
		context->context_priv->fragment_n = 0;
		context->context_priv->fragment_k = 0;
	    }
	} else if (n == context->context_priv->fragment_n &&
		k == context->context_priv->fragment_k + 1) {
	    int fraglen = end - start - 1;
	    char *newfrag = NULL;
	    size_t newsize = context->context_priv->fragment_len + fraglen + 1;
	    /* Check for overflow */
	    if (newsize > context->context_priv->fragment_len) {
		newfrag = realloc(context->context_priv->fragment, newsize);
	    }
	    if (newfrag) {
		context->context_priv->fragment = newfrag;
		memmove(context->context_priv->fragment +
			context->context_priv->fragment_len,
			tag + start, fraglen);
		context->context_priv->fragment_len += fraglen;
		context->context_priv->fragment[
			context->context_priv->fragment_len] = '\0';
		context->context_priv->fragment_k = k;
	    } else {
		free(context->context_priv->fragment);
		context->context_priv->fragment = NULL;
		context->context_priv->fragment_len = 0;
		context->context_priv->fragment_n = 0;
		context->context_priv->fragment_k = 0;
	    }
	} else {
	    free(context->context_priv->fragment);
	    context->context_priv->fragment = NULL;
	    context->context_priv->fragment_len = 0;
	    context->context_priv->fragment_n = 0;
	    context->context_priv->fragment_k = 0;
	}
    }

    if (context->context_priv->fragment_n > 0 &&
	    context->context_priv->fragment_n ==
	    context->context_priv->fragment_k) {
	/* We've got a complete message */
	*unfragmessagep = context->context_priv->fragment;
	context->context_priv->fragment = NULL;
	context->context_priv->fragment_len = 0;
	context->context_priv->fragment_n = 0;
	context->context_priv->fragment_k = 0;
	res = OTRL_FRAGMENT_COMPLETE;
    }

    return res;
}

/* Create a fragmented message. */
gcry_error_t otrl_proto_fragment_create(int mms, int fragment_count,
	char ***fragments, ConnContext *context, const char *message)
{
    char *fragdata;
    int fragdatalen = 0;
    int curfrag = 0;
    int index = 0;
    int msglen = strlen(message);
    /* Should vary by number of msgs */
    int headerlen = context->protocol_version == 3 ? 37 : 19;

    char **fragmentarray;

    if (fragment_count < 1 || fragment_count > 65535) {
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    fragmentarray = malloc(fragment_count * sizeof(char*));
    if(!fragmentarray) return gcry_error(GPG_ERR_ENOMEM);

    /*
     * Find the next message fragment and store it in the array.
     */
    for(curfrag = 1; curfrag <= fragment_count; curfrag++) {
	int i;
	char *fragmentmsg;

	if (msglen - index < mms - headerlen) {
	    fragdatalen = msglen - index;
	} else {
	    fragdatalen = mms - headerlen;
	}

	fragdata = malloc(fragdatalen + 1);
	if(!fragdata) {
		for (i=0; i<curfrag-1; free(fragmentarray[i++])) {}
		free(fragmentarray);
		return gcry_error(GPG_ERR_ENOMEM);
	}
	strncpy(fragdata, message, fragdatalen);
	fragdata[fragdatalen] = 0;

	fragmentmsg = malloc(fragdatalen+headerlen+1);
	if(!fragmentmsg) {
	    for (i=0; i<curfrag-1; free(fragmentarray[i++])) {}
	    free(fragmentarray);
	    free(fragdata);
	    return gcry_error(GPG_ERR_ENOMEM);
	}

	/*
	 * Create the actual fragment and store it in the array
	 */
	if (context->auth.protocol_version != 3) {
	    snprintf(fragmentmsg, fragdatalen + headerlen,
		    "?OTR,%05hu,%05hu,%s,", (unsigned short)curfrag,
			    (unsigned short)fragment_count, fragdata);
	} else {
	    /* V3 messages require instance tags in the header */
	    snprintf(fragmentmsg, fragdatalen + headerlen,
		    "?OTR|%08x|%08x,%05hu,%05hu,%s,",
		    context->our_instance, context->their_instance,
		    (unsigned short)curfrag, (unsigned short)fragment_count,
		    fragdata);
	}
	fragmentmsg[fragdatalen + headerlen] = 0;

	fragmentarray[curfrag-1] = fragmentmsg;

	free(fragdata);
	index += fragdatalen;
	message += fragdatalen;
    }

    *fragments = fragmentarray;
    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Free a string array containing fragment messages. */
void otrl_proto_fragment_free(char ***fragments, unsigned short arraylen)
{
    int i;
    char **fragmentarray = *fragments;
    if(fragmentarray) {
	for(i = 0; i < arraylen; i++)
	{
	    if(fragmentarray[i]) {
		free(fragmentarray[i]);
	    }
	}
	free(fragmentarray);
    }
}

