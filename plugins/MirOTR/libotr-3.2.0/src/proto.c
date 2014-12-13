/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

/* For now, we need to know the API version the client is using so that
 * we don't use any UI callbacks it hasn't set. */
unsigned int otrl_api_version = 0;

/* Initialize the OTR library.  Pass the version of the API you are
 * using. */
void otrl_init(unsigned int ver_major, unsigned int ver_minor,
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
	exit(1);
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

    newnumsaved = context->numsavedkeys + numnew;
    newmacs = realloc(context->saved_mac_keys,
	    newnumsaved * 20);
    if (!newmacs) {
	return gcry_error(GPG_ERR_ENOMEM);
    }
    if (sess1->rcvmacused) {
	memmove(newmacs + context->numsavedkeys * 20, sess1->rcvmackey, 20);
	context->numsavedkeys++;
    }
    if (sess1->sendmacused) {
	memmove(newmacs + context->numsavedkeys * 20, sess1->sendmackey, 20);
	context->numsavedkeys++;
    }
    if (sess2->rcvmacused) {
	memmove(newmacs + context->numsavedkeys * 20, sess2->rcvmackey, 20);
	context->numsavedkeys++;
    }
    if (sess2->sendmacused) {
	memmove(newmacs + context->numsavedkeys * 20, sess2->sendmackey, 20);
	context->numsavedkeys++;
    }
    context->saved_mac_keys = newmacs;

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Make a new DH key for us, and rotate old old ones.  Be sure to keep
 * the sesskeys array in sync. */
static gcry_error_t rotate_dh_keys(ConnContext *context)
{
    gcry_error_t err;

    /* Rotate the keypair */
    otrl_dh_keypair_free(&(context->our_old_dh_key));
    memmove(&(context->our_old_dh_key), &(context->our_dh_key),
	    sizeof(DH_keypair));

    /* Rotate the session keys */
    err = reveal_macs(context, &(context->sesskeys[1][0]),
	    &(context->sesskeys[1][1]));
    if (err) return err;
    otrl_dh_session_free(&(context->sesskeys[1][0]));
    otrl_dh_session_free(&(context->sesskeys[1][1]));
    memmove(&(context->sesskeys[1][0]), &(context->sesskeys[0][0]),
	    sizeof(DH_sesskeys));
    memmove(&(context->sesskeys[1][1]), &(context->sesskeys[0][1]),
	    sizeof(DH_sesskeys));

    /* Create a new DH key */
    otrl_dh_gen_keypair(DH1536_GROUP_ID, &(context->our_dh_key));
    context->our_keyid++;

    /* Make the session keys */
    if (context->their_y) {
	err = otrl_dh_session(&(context->sesskeys[0][0]),
		&(context->our_dh_key), context->their_y);
	if (err) return err;
    } else {
	otrl_dh_session_blank(&(context->sesskeys[0][0]));
    }
    if (context->their_old_y) {
	err = otrl_dh_session(&(context->sesskeys[0][1]),
		&(context->our_dh_key), context->their_old_y);
	if (err) return err;
    } else {
	otrl_dh_session_blank(&(context->sesskeys[0][1]));
    }
    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Rotate in a new DH public key for our correspondent.  Be sure to keep
 * the sesskeys array in sync. */
static gcry_error_t rotate_y_keys(ConnContext *context, gcry_mpi_t new_y)
{
    gcry_error_t err;

    /* Rotate the public key */
    gcry_mpi_release(context->their_old_y);
    context->their_old_y = context->their_y;

    /* Rotate the session keys */
    err = reveal_macs(context, &(context->sesskeys[0][1]),
	    &(context->sesskeys[1][1]));
    if (err) return err;
    otrl_dh_session_free(&(context->sesskeys[0][1]));
    otrl_dh_session_free(&(context->sesskeys[1][1]));
    memmove(&(context->sesskeys[0][1]), &(context->sesskeys[0][0]),
	    sizeof(DH_sesskeys));
    memmove(&(context->sesskeys[1][1]), &(context->sesskeys[1][0]),
	    sizeof(DH_sesskeys));

    /* Copy in the new public key */
    context->their_y = gcry_mpi_copy(new_y);
    context->their_keyid++;

    /* Make the session keys */
    err = otrl_dh_session(&(context->sesskeys[0][0]),
	    &(context->our_dh_key), context->their_y);
    if (err) return err;
    err = otrl_dh_session(&(context->sesskeys[1][0]),
	    &(context->our_old_dh_key), context->their_y);
    if (err) return err;

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Return a pointer to a newly-allocated OTR query message, customized
 * with our name.  The caller should free() the result when he's done
 * with it. */
char *otrl_proto_default_query_msg(const char *ourname, OtrlPolicy policy)
{
    char *msg;
    int v1_supported, v2_supported;
    const char *version_tag;
    /* Don't use g_strdup_printf here, because someone (not us) is going
     * to free() the *message pointer, not g_free() it.  We can't
     * require that they g_free() it, because this pointer will probably
     * get passed to the main IM application for processing (and
     * free()ing). */
    const char *format = "?OTR%s\n<b>%s</b> has requested an "
	    "<a href=\"http://otr.cypherpunks.ca/\">Off-the-Record "
	    "private conversation</a>.  However, you do not have a plugin "
	    "to support that.\nSee <a href=\"http://otr.cypherpunks.ca/\">"
	    "http://otr.cypherpunks.ca/</a> for more information.";

    /* Figure out the version tag */
    v1_supported = (policy & OTRL_POLICY_ALLOW_V1);
    v2_supported = (policy & OTRL_POLICY_ALLOW_V2);
    if (v1_supported) {
	if (v2_supported) {
	    version_tag = "?v2?";
	} else {
	    version_tag = "?";
	}
    } else {
	if (v2_supported) {
	    version_tag = "v2?";
	} else {
	    version_tag = "v?";
	}
    }

    /* Remove two "%s", add '\0' */
    msg = malloc(strlen(format) + strlen(version_tag) + strlen(ourname) - 3);
    if (!msg) return NULL;
    sprintf(msg, format, version_tag, ourname);
    return msg;
}

/* Return the best version of OTR support by both sides, given an OTR
 * Query Message and the local policy. */
unsigned int otrl_proto_query_bestversion(const char *querymsg,
	OtrlPolicy policy)
{
    char *otrtag;
    unsigned int query_versions = 0;

    otrtag = strstr(querymsg, "?OTR");
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
	    }
	}
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
	    endtag += 8;
	} else {
	    break;
	}
    }

    *starttagp = starttag;
    *endtagp = endtag;

    if ((policy & OTRL_POLICY_ALLOW_V2) && (query_versions & (1<<1))) {
	return 2;
    }
    if ((policy & OTRL_POLICY_ALLOW_V1) && (query_versions & (1<<0))) {
	return 1;
    }
    return 0;
}

/* Return the Message type of the given message. */
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

    if (!strncmp(otrtag, "?OTR?", 5)) return OTRL_MSGTYPE_QUERY;
    if (!strncmp(otrtag, "?OTRv", 5)) return OTRL_MSGTYPE_QUERY;
    if (!strncmp(otrtag, "?OTR:AAIC", 9)) return OTRL_MSGTYPE_DH_COMMIT;
    if (!strncmp(otrtag, "?OTR:AAIK", 9)) return OTRL_MSGTYPE_DH_KEY;
    if (!strncmp(otrtag, "?OTR:AAIR", 9)) return OTRL_MSGTYPE_REVEALSIG;
    if (!strncmp(otrtag, "?OTR:AAIS", 9)) return OTRL_MSGTYPE_SIGNATURE;
    if (!strncmp(otrtag, "?OTR:AAEK", 9)) return OTRL_MSGTYPE_V1_KEYEXCH;
    if (!strncmp(otrtag, "?OTR:AAED", 9)) return OTRL_MSGTYPE_DATA;
    if (!strncmp(otrtag, "?OTR:AAID", 9)) return OTRL_MSGTYPE_DATA;
    if (!strncmp(otrtag, "?OTR Error:", 11)) return OTRL_MSGTYPE_ERROR;

    return OTRL_MSGTYPE_UNKNOWN;
}

/* Create an OTR Data message.  Pass the plaintext as msg, and an
 * optional chain of TLVs.  A newly-allocated string will be returned in
 * *encmessagep. */
gcry_error_t otrl_proto_create_data(char **encmessagep, ConnContext *context,
	const char *msg, const OtrlTLV *tlvs, unsigned char flags)
{
    size_t justmsglen = strlen(msg);
    size_t msglen = justmsglen + 1 + otrl_tlv_seriallen(tlvs);
    size_t buflen;
    size_t pubkeylen;
    unsigned char *buf = NULL;
    unsigned char *bufp;
    size_t lenp;
    DH_sesskeys *sess = &(context->sesskeys[1][0]);
    gcry_error_t err;
    size_t reveallen = 20 * context->numsavedkeys;
    size_t base64len;
    char *base64buf = NULL;
    unsigned char *msgbuf = NULL;
    enum gcry_mpi_format format = GCRYMPI_FMT_USG;
    char *msgdup;
    int version = context->protocol_version;

    /* Make sure we're actually supposed to be able to encrypt */
    if (context->msgstate != OTRL_MSGSTATE_ENCRYPTED ||
	    context->their_keyid == 0) {
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

    /* Header, send keyid, recv keyid, counter, msg len, msg
     * len of revealed mac keys, revealed mac keys, MAC */
    buflen = 3 + (version == 2 ? 1 : 0) + 4 + 4 + 8 + 4 + msglen +
	4 + reveallen + 20;
    gcry_mpi_print(format, NULL, 0, &pubkeylen, context->our_dh_key.pub);
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
	memcpy(bufp, "\x00\x01\x03", 3);  /* header */
    } else {
	memcpy(bufp, "\x00\x02\x03", 3);  /* header */
    }
    debug_data("Header", bufp, 3);
    bufp += 3; lenp -= 3;
    if (version == 2) {
	bufp[0] = flags;
	bufp += 1; lenp -= 1;
    }
    write_int(context->our_keyid-1);                    /* sender keyid */
    debug_int("Sender keyid", bufp-4);
    write_int(context->their_keyid);                    /* recipient keyid */
    debug_int("Recipient keyid", bufp-4);

    write_mpi(context->our_dh_key.pub, pubkeylen, "Y");      /* Y */

    otrl_dh_incctr(sess->sendctr);
    memcpy(bufp, sess->sendctr, 8);      /* Counter (top 8 bytes only) */
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
    memcpy(bufp, gcry_md_read(sess->sendmac, GCRY_MD_SHA1), 20);
    debug_data("MAC", bufp, 20);
    bufp += 20;                                         /* MAC */
    lenp -= 20;

    write_int(reveallen);                     /* length of revealed MAC keys */
    debug_int("Revealed MAC length", bufp-4);

    if (reveallen > 0) {
	memcpy(bufp, context->saved_mac_keys, reveallen);
	debug_data("Revealed MAC data", bufp, reveallen);
	bufp += reveallen; lenp -= reveallen;
	free(context->saved_mac_keys);
	context->saved_mac_keys = NULL;
	context->numsavedkeys = 0;
    }

    assert(lenp == 0);

    /* Make the base64-encoding. */
    base64len = ((buflen + 2) / 3) * 4;
    base64buf = malloc(5 + base64len + 1 + 1);
    if (base64buf == NULL) {
	err = gcry_error(GPG_ERR_ENOMEM);
	goto err;
    }
    memcpy(base64buf, "?OTR:", 5);
    otrl_base64_encode(base64buf+5, buf, buflen);
    base64buf[5 + base64len] = '.';
    base64buf[5 + base64len + 1] = '\0';

    free(buf);
    gcry_free(msgbuf);
    *encmessagep = base64buf;
    gcry_free(context->lastmessage);
    context->lastmessage = NULL;
    context->may_retransmit = 0;
    if (msglen > 0) {
	const char *prefix = "[resent] ";
	size_t prefixlen = strlen(prefix);
	if (!strncmp(prefix, msgdup, prefixlen)) {
	    /* The prefix is already there.  Don't add it again. */
	    prefix = "";
	    prefixlen = 0;
	}
	context->lastmessage = gcry_malloc_secure(prefixlen + justmsglen + 1);
	if (context->lastmessage) {
	    strcpy(context->lastmessage, prefix);
	    strcat(context->lastmessage, msgdup);
	}
    }
    gcry_free(msgdup);
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

    /* Base64-decode the message */
    rawlen = ((msglen-5) / 4) * 3;   /* maximum possible */
    rawmsg = malloc(rawlen);
    if (!rawmsg && rawlen > 0) {
	return gcry_error(GPG_ERR_ENOMEM);
    }
    rawlen = otrl_base64_decode(rawmsg, otrtag+5, msglen-5);  /* actual size */

    bufp = rawmsg;
    lenp = rawlen;

    require_len(3);
    if (memcmp(bufp, "\x00\x01\x03", 3) && memcmp(bufp, "\x00\x02\x03", 3)) {
	/* Invalid header */
	goto invval;
    }
    version = bufp[1];
    bufp += 3; lenp -= 3;

    if (version == 2) {
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
 * received flags into *flagsp (if non-NULL). */
gcry_error_t otrl_proto_accept_data(char **plaintextp, OtrlTLV **tlvsp,
	ConnContext *context, const char *datamsg, unsigned char *flagsp)
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

    /* Base64-decode the message */
    rawlen = ((msglen-5) / 4) * 3;   /* maximum possible */
    rawmsg = malloc(rawlen);
    if (!rawmsg && rawlen > 0) {
	err = gcry_error(GPG_ERR_ENOMEM);
	goto err;
    }
    rawlen = otrl_base64_decode(rawmsg, otrtag+5, msglen-5);  /* actual size */

    bufp = rawmsg;
    lenp = rawlen;

    macstart = bufp;
    require_len(3);
    if (memcmp(bufp, "\x00\x01\x03", 3) && memcmp(bufp, "\x00\x02\x03", 3)) {
	/* Invalid header */
	goto invval;
    }
    version = bufp[1];
    bufp += 3; lenp -= 3;

    if (version == 2) {
	require_len(1);
	if (flagsp) *flagsp = bufp[0];
	bufp += 1; lenp -= 1;
    }
    read_int(sender_keyid);
    read_int(recipient_keyid);
    read_mpi(sender_next_y);
    require_len(8);
    memcpy(ctr, bufp, 8);
    bufp += 8; lenp -= 8;
    read_int(datalen);
    require_len(datalen);
    data = malloc(datalen+1);
    if (!data) {
	err = gcry_error(GPG_ERR_ENOMEM);
	goto err;
    }
    memcpy(data, bufp, datalen);
    data[datalen] = '\0';
    bufp += datalen; lenp -= datalen;
    macend = bufp;
    require_len(20);
    memcpy(givenmac, bufp, 20);
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
    if (context->their_keyid == 0 ||
	    (sender_keyid != context->their_keyid &&
		sender_keyid != context->their_keyid - 1) ||
	    (recipient_keyid != context->our_keyid &&
	     recipient_keyid != context->our_keyid - 1) ||
	    sender_keyid == 0 || recipient_keyid == 0) {
	goto conflict;
    }

    if (sender_keyid == context->their_keyid - 1 &&
	    context->their_old_y == NULL) {
	goto conflict;
    }

    /* These are the session keys this message is claiming to use. */
    sess = &(context->sesskeys
	    [context->our_keyid - recipient_keyid]
	    [context->their_keyid - sender_keyid]);

    gcry_md_reset(sess->rcvmac);
    gcry_md_write(sess->rcvmac, macstart, macend-macstart);
    if (memcmp(givenmac, gcry_md_read(sess->rcvmac, GCRY_MD_SHA1), 20)) {
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
    memcpy(sess->rcvctr, ctr, 8);
    err = gcry_cipher_reset(sess->rcvenc);
    if (err) goto err;
    err = gcry_cipher_setctr(sess->rcvenc, sess->rcvctr, 16);
    if (err) goto err;
    err = gcry_cipher_decrypt(sess->rcvenc, data, datalen, NULL, 0);
    if (err) goto err;

    /* See if either set of keys needs rotating */

    if (recipient_keyid == context->our_keyid) {
	/* They're using our most recent key, so generate a new one */
	err = rotate_dh_keys(context);
	if (err) goto err;
    }

    if (sender_keyid == context->their_keyid) {
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

    tag = strstr(msg, "?OTR,");
    if (tag) {
	unsigned short n = 0, k = 0;
	int start = 0, end = 0;

	sscanf(tag, "?OTR,%hu,%hu,%n%*[^,],%n", &k, &n, &start, &end);
	if (k > 0 && n > 0 && k <= n && start > 0 && end > 0 && start < end) {
	    if (k == 1) {
		int fraglen = end - start - 1;
		free(context->fragment);
		context->fragment = malloc(fraglen + 1);
		if (fraglen + 1 > fraglen && context->fragment) {
		    memmove(context->fragment, tag + start, fraglen);
		    context->fragment_len = fraglen;
		    context->fragment[context->fragment_len] = '\0';
		    context->fragment_n = n;
		    context->fragment_k = k;
		} else {
		    free(context->fragment);
		    context->fragment = NULL;
		    context->fragment_len = 0;
		    context->fragment_n = 0;
		    context->fragment_k = 0;
		}
	    } else if (n == context->fragment_n &&
		    k == context->fragment_k + 1) {
		int fraglen = end - start - 1;
		char *newfrag = realloc(context->fragment,
			context->fragment_len + fraglen + 1);
		if (context->fragment_len + fraglen + 1 > fraglen && newfrag) {
		    context->fragment = newfrag;
		    memmove(context->fragment + context->fragment_len,
			    tag + start, fraglen);
		    context->fragment_len += fraglen;
		    context->fragment[context->fragment_len] = '\0';
		    context->fragment_k = k;
		} else {
		    free(context->fragment);
		    context->fragment = NULL;
		    context->fragment_len = 0;
		    context->fragment_n = 0;
		    context->fragment_k = 0;
		}
	    } else {
		free(context->fragment);
		context->fragment = NULL;
		context->fragment_len = 0;
		context->fragment_n = 0;
		context->fragment_k = 0;
	    }
	}

	if (context->fragment_n > 0 &&
		context->fragment_n == context->fragment_k) {
	    /* We've got a complete message */
	    *unfragmessagep = context->fragment;
	    context->fragment = NULL;
	    context->fragment_len = 0;
	    context->fragment_n = 0;
	    context->fragment_k = 0;
	    res = OTRL_FRAGMENT_COMPLETE;
	}
    } else {
	/* Unfragmented message, so discard any fragment we may have */
	free(context->fragment);
	context->fragment = NULL;
	context->fragment_len = 0;
	context->fragment_n = 0;
	context->fragment_k = 0;
	res = OTRL_FRAGMENT_UNFRAGMENTED;
    }

    return res;
}

/* Create a fragmented message. */
gcry_error_t otrl_proto_fragment_create(int mms, int fragment_count,
	char ***fragments, const char *message)
{
    char *fragdata;
    int fragdatalen = 0;
    unsigned short curfrag = 0;
    int index = 0;
    int msglen = strlen(message);
    int headerlen = 19; /* Should vary by number of msgs */

    char **fragmentarray = malloc(fragment_count * sizeof(char*));
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
    	_snprintf(fragmentmsg, fragdatalen + headerlen, "?OTR,%05hu,%05hu,%s,", curfrag, fragment_count, fragdata);
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

