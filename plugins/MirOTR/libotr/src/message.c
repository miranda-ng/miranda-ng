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

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "privkey.h"
#include "userstate.h"
#include "proto.h"
#include "auth.h"
#include "message.h"
#include "sm.h"
#include "instag.h"

#if OTRL_DEBUGGING
#include <stdio.h>

/* If OTRL_DEBUGGING is on, and the user types this string, the current
 * context and its siblings will be dumped to stderr. */
const char *OTRL_DEBUGGING_DEBUGSTR = "?OTR!";

void otrl_context_all_dump(FILE *f, OtrlUserState us);
void otrl_context_siblings_dump(FILE *f, const ConnContext *context);
#endif

/* The API version */
extern unsigned int otrl_api_version;

/* How long after sending a packet should we wait to send a heartbeat? */
#define HEARTBEAT_INTERVAL 60

/* How old are messages allowed to be in order to be candidates for
 * resending in response to a rekey? */
#define RESEND_INTERVAL 60

/* How long should we wait for the last of the logged-in instances of
 * our buddy to respond before marking our private key as a candidate
 * for wiping (in seconds)? */
#define MAX_AKE_WAIT_TIME 60

/* How frequently should we check our ConnContexts for wipeable private
 * keys (and wipe them) (in seconds)? */
#define POLL_DEFAULT_INTERVAL 70

/* Send a message to the network, fragmenting first if necessary.
 * All messages to be sent to the network should go through this
 * method immediately before they are sent, ie after encryption. */
static gcry_error_t fragment_and_send(const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const char *message,
	OtrlFragmentPolicy fragPolicy, char **returnFragment)
{
    int mms = 0;

    if (message && ops->inject_message) {
	int msglen;

	if (ops->max_message_size) {
	    mms = ops->max_message_size(opdata, context);
	}
	msglen = strlen(message);

	/* Don't incur overhead of fragmentation unless necessary */
	if(mms != 0 && msglen > mms) {
	    char **fragments;
	    gcry_error_t err;
	    int i;
	    int headerlen = context->protocol_version == 3 ? 37 : 19;
	    /* Like ceil(msglen/(mms - headerlen)) */
	    int fragment_count = ((msglen - 1) / (mms - headerlen)) + 1;

	    err = otrl_proto_fragment_create(mms, fragment_count, &fragments,
		    context, message);
	    if (err) {
		return err;
	    }

	    /* Determine which fragments to send and which to return
	     * based on given Fragment Policy.  If the first fragment
	     * should be returned instead of sent, store it. */
	    if (fragPolicy == OTRL_FRAGMENT_SEND_ALL_BUT_FIRST) {
		*returnFragment = strdup(fragments[0]);
	    } else {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, fragments[0]);
	    }
	    for (i=1; i<fragment_count-1; i++) {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, fragments[i]);
	    }
	    /* If the last fragment should be stored instead of sent,
	     * store it */
	    if (fragPolicy == OTRL_FRAGMENT_SEND_ALL_BUT_LAST) {
		*returnFragment = strdup(fragments[fragment_count-1]);
	    } else {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username,
			fragments[fragment_count-1]);
	    }
	    /* Now free all fragment memory */
	    otrl_proto_fragment_free(&fragments, fragment_count);

	} else {
	    /* No fragmentation necessary */
	    if (fragPolicy == OTRL_FRAGMENT_SEND_ALL) {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, message);
	    } else {
		/* Copy and return the entire given message. */
		*returnFragment = strdup(message);
	    }
	}
    }

    return gcry_error(GPG_ERR_NO_ERROR);
}

static void populate_context_instag(OtrlUserState us, const OtrlMessageAppOps
	*ops, void *opdata, const char *accountname, const char *protocol,
	ConnContext *context) {
    OtrlInsTag *p_instag;

    p_instag = otrl_instag_find(us, accountname, protocol);
    if ((!p_instag) && ops->create_instag) {
	ops->create_instag(opdata, accountname, protocol);
	p_instag = otrl_instag_find(us, accountname, protocol);
    }

    if (p_instag && p_instag->instag >= OTRL_MIN_VALID_INSTAG) {
	context->our_instance = p_instag->instag;
    } else {
	context->our_instance = otrl_instag_get_new();
    }
}

/* Deallocate a message allocated by other otrl_message_* routines. */
void otrl_message_free(char *message)
{
    free(message);
}

/* Handle a message about to be sent to the network.  It is safe to pass
 * all messages about to be sent to this routine.  add_appdata is a
 * function that will be called in the event that a new ConnContext is
 * created.  It will be passed the data that you supplied, as well as a
 * pointer to the new ConnContext.  You can use this to add
 * application-specific information to the ConnContext using the
 * "context->app" field, for example.  If you don't need to do this, you
 * can pass NULL for the last two arguments of otrl_message_sending.
 *
 * tlvs is a chain of OtrlTLVs to append to the private message.  It is
 * usually correct to just pass NULL here.
 *
 * If non-NULL, ops->convert_msg will be called just before encrypting a
 * message.
 *
 * "instag" specifies the instance tag of the buddy (protocol version 3 only).
 * Meta-instances may also be specified (e.g., OTRL_INSTAG_MOST_SECURE).
 * If "contextp" is not NULL, it will be set to the ConnContext used for
 * sending the message.
 *
 * If no fragmentation or msg injection is wanted, use OTRL_FRAGMENT_SEND_SKIP
 * as the OtrlFragmentPolicy. In this case, this function will assign *messagep
 * with the encrypted msg. If the routine returns non-zero, then the library
 * tried to encrypt the message, but for some reason failed. DO NOT send the
 * message in the clear in that case. If *messagep gets set by the call to
 * something non-NULL, then you should replace your message with the contents
 * of *messagep, and send that instead.
 *
 * Other fragmentation policies are OTRL_FRAGMENT_SEND_ALL,
 * OTRL_FRAGMENT_SEND_ALL_BUT_LAST, or OTRL_FRAGMENT_SEND_ALL_BUT_FIRST. In
 * these cases, the appropriate fragments will be automatically sent. For the
 * last two policies, the remaining fragment will be passed in *original_msg.
 *
 * Call otrl_message_free(*messagep) if you don't need *messagep or when you're
 * done with it. */
gcry_error_t otrl_message_sending(OtrlUserState us,
	const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *recipient, otrl_instag_t their_instag,
	const char *original_msg, OtrlTLV *tlvs, char **messagep,
	OtrlFragmentPolicy fragPolicy, ConnContext **contextp,
	void (*add_appdata)(void *data, ConnContext *context),
	void *data)
{
    ConnContext * context = NULL;
    char * msgtosend;
    const char * err_msg;
    gcry_error_t err_code, err;
    OtrlPolicy policy = OTRL_POLICY_DEFAULT;
    int context_added = 0;
    int convert_called = 0;
    char *converted_msg = NULL;

    if (messagep) {
	*messagep = NULL;
    }

    err = gcry_error(GPG_ERR_NO_ERROR);	/* Default to no error */

    if (contextp) {
	*contextp = NULL;
    }

    if (!accountname || !protocol || !recipient ||
		!original_msg || !messagep) {
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto fragment;
    }

    /* See if we have a fingerprint for this user */
    context = otrl_context_find(us, recipient, accountname, protocol,
	    their_instag, 1, &context_added, add_appdata, data);

    /* Update the context list if we added one */
    if (context_added && ops->update_context_list) {
	ops->update_context_list(opdata);
    }

    /* Find or generate the instance tag if needed */
    if (!context->our_instance) {
	populate_context_instag(us, ops, opdata, accountname, protocol,
	    context);
    }

    if (contextp) {
	*contextp = context;
    }

    /* Check the policy */
    if (ops->policy) {
	policy = ops->policy(opdata, context);
    }

    /* Should we go on at all? */
    if ((policy & OTRL_POLICY_VERSION_MASK) == 0) {
	err =  gcry_error(GPG_ERR_NO_ERROR);
	goto fragment;
    }

#if OTRL_DEBUGGING
    /* If the user typed the magic debug string, dump this context and
     * its siblings. */
    {
	const char *debugtag = strstr(original_msg, OTRL_DEBUGGING_DEBUGSTR);

	if (debugtag) {
	    const char *debugargs =
		debugtag + strlen(OTRL_DEBUGGING_DEBUGSTR);
	    if (debugargs[0] == '!') { /* typed ?OTR!! */
		otrl_context_all_dump(stderr, us);
	    } else { /* typed ?OTR! without extra command chars */
		otrl_context_siblings_dump(stderr, context);
	    }

	    /* Don't actually send the message */
	    *messagep = strdup("");
	    if (!(*messagep)) {
		err = gcry_error(GPG_ERR_ENOMEM);
	    }
	    goto fragment;
	}
    }
#endif

    /* If this is an OTR Query message, don't encrypt it. */
    if (otrl_proto_message_type(original_msg) == OTRL_MSGTYPE_QUERY) {
	/* Replace the "?OTR?" with a custom message */
	char *bettermsg = otrl_proto_default_query_msg(accountname, policy);
	if (bettermsg) {
	    *messagep = bettermsg;
	}
	context->otr_offer = OFFER_SENT;
	err = gcry_error(GPG_ERR_NO_ERROR);
	goto fragment;
    }

    /* What is the current message disposition? */
    switch(context->msgstate) {

	case OTRL_MSGSTATE_PLAINTEXT:
	    if ((policy & OTRL_POLICY_REQUIRE_ENCRYPTION)) {
		/* We're trying to send an unencrypted message with a policy
		 * that disallows that.  Don't do that, but try to start
		 * up OTR instead. */
		if (ops->handle_msg_event) {
		    ops->handle_msg_event(opdata,
			    OTRL_MSGEVENT_ENCRYPTION_REQUIRED,
			    context, NULL, gcry_error(GPG_ERR_NO_ERROR));
		}

		context->context_priv->lastmessage =
			gcry_malloc_secure(strlen(original_msg) + 1);
		if (context->context_priv->lastmessage) {
		    char *bettermsg = otrl_proto_default_query_msg(accountname,
			    policy);
		    strcpy(context->context_priv->lastmessage, original_msg);
		    context->context_priv->lastsent = time(NULL);
		    otrl_context_update_recent_child(context, 1);
		    context->context_priv->may_retransmit = 2;
		    if (bettermsg) {
			*messagep = bettermsg;
			context->otr_offer = OFFER_SENT;
		    } else {
			err = gcry_error(GPG_ERR_ENOMEM);
			goto fragment;
		    }
		}
	    } else {
		if ((policy & OTRL_POLICY_SEND_WHITESPACE_TAG) &&
			context->otr_offer != OFFER_REJECTED) {
		    /* See if this user can speak OTR.  Append the
		     * OTR_MESSAGE_TAG to the plaintext message, and see
		     * if he responds. */
		    size_t msglen = strlen(original_msg);
		    size_t basetaglen = strlen(OTRL_MESSAGE_TAG_BASE);
		    size_t v1taglen = (policy & OTRL_POLICY_ALLOW_V1) ?
			strlen(OTRL_MESSAGE_TAG_V1) : 0;
		    size_t v2taglen = (policy & OTRL_POLICY_ALLOW_V2) ?
			strlen(OTRL_MESSAGE_TAG_V2) : 0;
		    size_t v3taglen = (policy & OTRL_POLICY_ALLOW_V3) ?
			strlen(OTRL_MESSAGE_TAG_V3) : 0;
		    char *taggedmsg = malloc(msglen + basetaglen + v1taglen
			    + v2taglen + v3taglen + 1);
		    if (taggedmsg) {
			strcpy(taggedmsg, original_msg);
			strcpy(taggedmsg + msglen, OTRL_MESSAGE_TAG_BASE);
			if (v1taglen) {
			    strcpy(taggedmsg + msglen + basetaglen,
				    OTRL_MESSAGE_TAG_V1);
			}
			if (v2taglen) {
			    strcpy(taggedmsg + msglen + basetaglen + v1taglen,
				    OTRL_MESSAGE_TAG_V2);
			}
			if (v3taglen) {
			    strcpy(taggedmsg + msglen + basetaglen + v1taglen
				    + v2taglen, OTRL_MESSAGE_TAG_V3);
			}
			*messagep = taggedmsg;
			context->otr_offer = OFFER_SENT;
		    }
		}
	    }
	    break;
	case OTRL_MSGSTATE_ENCRYPTED:
	    /* convert the original message if necessary */
	    if (ops->convert_msg) {
		ops->convert_msg(opdata, context, OTRL_CONVERT_SENDING,
			&converted_msg, original_msg);

		if (converted_msg) {
		    convert_called = 1;
		}
	    }

	    /* Create the new, encrypted message */
	    if (convert_called) {
		err_code = otrl_proto_create_data(&msgtosend, context,
			converted_msg, tlvs, 0, NULL);

		if (ops->convert_free) {
		    ops->convert_free(opdata, context, converted_msg);
		    converted_msg = NULL;
		}
	    } else {
		err_code = otrl_proto_create_data(&msgtosend, context,
			original_msg, tlvs, 0, NULL);
	    }
	    if (!err_code) {
		context->context_priv->lastsent = time(NULL);
		otrl_context_update_recent_child(context, 1);
		*messagep = msgtosend;
	    } else {
		/* Uh, oh.  Whatever we do, *don't* send the message in the
		 * clear. */
		if (ops->handle_msg_event) {
		    ops->handle_msg_event(opdata,
			    OTRL_MSGEVENT_ENCRYPTION_ERROR,
			    context, NULL, gcry_error(GPG_ERR_NO_ERROR));
		}
		if (ops->otr_error_message) {
		    err_msg = ops->otr_error_message(opdata, context,
			OTRL_ERRCODE_ENCRYPTION_ERROR);
		    *messagep = malloc(strlen(OTR_ERROR_PREFIX) +
			strlen(err_msg) + 1);
		    if (*messagep) {
			strcpy(*messagep, OTR_ERROR_PREFIX);
			strcat(*messagep, err_msg);
		    }
		    if (ops->otr_error_message_free) {
			ops->otr_error_message_free(opdata, err_msg);
		    }
		    if (!(*messagep)) {
			err = gcry_error(GPG_ERR_ENOMEM);
			goto fragment;
		    }
		}
	    }
	    break;
	case OTRL_MSGSTATE_FINISHED:
	    if (ops->handle_msg_event) {
		ops->handle_msg_event(opdata, OTRL_MSGEVENT_CONNECTION_ENDED,
		    context, NULL, gcry_error(GPG_ERR_NO_ERROR));
	    }
	    *messagep = strdup("");
	    if (!(*messagep)) {
		err = gcry_error(GPG_ERR_ENOMEM);
		goto fragment;
	    }
	    break;
    }

fragment:
    if (fragPolicy == OTRL_FRAGMENT_SEND_SKIP ) {
	/* Do not fragment/inject. Default behaviour of libotr3.2.0 */
	return err;
    } else {
	/* Fragment and send according to policy */
	if (!err && messagep && *messagep) {
	    if (context) {
		char *rmessagep = NULL;
		err = fragment_and_send(ops, opdata, context, *messagep,
					fragPolicy, &rmessagep);
		if (rmessagep) {
		    /* Free the current message pointer and return back the
		     * returned fragmented one. */
		    free(*messagep);
		    *messagep = rmessagep;
		}
	    }
	}
	return err;
    }
}

/* If err == 0, send the last auth message for the given context to the
 * appropriate user.  Otherwise, display an appripriate error dialog.
 * Return the value of err that was passed. */
static gcry_error_t send_or_error_auth(const OtrlMessageAppOps *ops,
	void *opdata, gcry_error_t err, ConnContext *context,
	OtrlUserState us)
{
    if (!err) {
	const char *msg = context->auth.lastauthmsg;
	if (msg && *msg) {
	    time_t now;
	    fragment_and_send(ops, opdata, context, msg,
		    OTRL_FRAGMENT_SEND_ALL, NULL);
	    now = time(NULL);
	    /* Update the "last sent" fields, unless this is a version 3
	     * message typing to update the master context (as happens
	     * when sending a v3 COMMIT message, for example). */
	    if (context != context->m_context ||
		    context->auth.protocol_version != 3) {
		context->context_priv->lastsent = now;
		otrl_context_update_recent_child(context, 1);
	    }

	    /* If this is a master context, and we're sending a v3 COMMIT
	     * message, update the commit_sent_time timestamp, so we can
	     * expire it. */
	    if (context == context->m_context &&
		    context->auth.authstate == OTRL_AUTHSTATE_AWAITING_DHKEY &&
		    context->auth.protocol_version == 3) {
		context->auth.commit_sent_time = now;
		/* If there's not already a timer running to clean up
		 * this private key, try to start one. */
		if (us->timer_running == 0 && ops && ops->timer_control) {
		    ops->timer_control(opdata, POLL_DEFAULT_INTERVAL);
		    us->timer_running = 1;
		}
	    }
	}
    } else {
	if (ops->handle_msg_event) {
	    ops->handle_msg_event(opdata, OTRL_MSGEVENT_SETUP_ERROR,
		    context, NULL, err);
	}
    }
    return err;
}

typedef struct {
    int gone_encrypted;
    OtrlUserState us;
    const OtrlMessageAppOps *ops;
    void *opdata;
    ConnContext *context;
    int ignore_message;
    char **messagep;
} EncrData;

static gcry_error_t go_encrypted(const OtrlAuthInfo *auth, void *asdata)
{
    EncrData *edata = asdata;
    gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
    Fingerprint *found_print = NULL;
    int fprint_added = 0;
    OtrlMessageState oldstate = edata->context->msgstate;
    Fingerprint *oldprint = edata->context->active_fingerprint;

    /* See if we're talking to ourselves */
    if (!gcry_mpi_cmp(auth->their_pub, auth->our_dh.pub)) {
	/* Yes, we are. */
	if (edata->ops->handle_msg_event) {
	    edata->ops->handle_msg_event(edata->opdata,
		    OTRL_MSGEVENT_MSG_REFLECTED, edata->context,
		    NULL, gcry_error(GPG_ERR_NO_ERROR));
	}
	edata->ignore_message = 1;
	return gcry_error(GPG_ERR_NO_ERROR);
    }

    found_print = otrl_context_find_fingerprint(edata->context,
	    edata->context->auth.their_fingerprint, 1, &fprint_added);

    if (fprint_added) {
	/* Inform the user of the new fingerprint */
	if (edata->ops->new_fingerprint) {
	    edata->ops->new_fingerprint(edata->opdata, edata->us,
		    edata->context->accountname, edata->context->protocol,
		    edata->context->username,
		    edata->context->auth.their_fingerprint);
	}
	/* Arrange that the new fingerprint be written to disk */
	if (edata->ops->write_fingerprints) {
	    edata->ops->write_fingerprints(edata->opdata);
	}
    }

    /* Is this a new session or just a refresh of an existing one? */
    if (edata->context->msgstate == OTRL_MSGSTATE_ENCRYPTED &&
	    oldprint == found_print &&
	    edata->context->context_priv->our_keyid - 1 ==
	    edata->context->auth.our_keyid &&
	    !gcry_mpi_cmp(edata->context->context_priv->our_old_dh_key.pub,
		edata->context->auth.our_dh.pub) &&
	    ((edata->context->context_priv->their_keyid > 0 &&
	      edata->context->context_priv->their_keyid ==
		    edata->context->auth.their_keyid &&
	      !gcry_mpi_cmp(edata->context->context_priv->their_y,
		  edata->context->auth.their_pub)) ||
	    (edata->context->context_priv->their_keyid > 1 &&
	     edata->context->context_priv->their_keyid - 1 ==
		    edata->context->auth.their_keyid &&
	     edata->context->context_priv->their_old_y != NULL &&
	     !gcry_mpi_cmp(edata->context->context_priv->their_old_y,
		 edata->context->auth.their_pub)))) {
	/* This is just a refresh of the existing session. */
	if (edata->ops->still_secure) {
	    edata->ops->still_secure(edata->opdata, edata->context,
		    edata->context->auth.initiated);
	}
	edata->ignore_message = 1;
	return gcry_error(GPG_ERR_NO_ERROR);
    }

    /* Copy the information from the auth into the context */
    memmove(edata->context->sessionid,
	    edata->context->auth.secure_session_id, 20);
    edata->context->sessionid_len =
	    edata->context->auth.secure_session_id_len;
    edata->context->sessionid_half =
	    edata->context->auth.session_id_half;
    edata->context->protocol_version =
	    edata->context->auth.protocol_version;

    edata->context->context_priv->their_keyid =
	    edata->context->auth.their_keyid;
    gcry_mpi_release(edata->context->context_priv->their_y);
    gcry_mpi_release(edata->context->context_priv->their_old_y);
    edata->context->context_priv->their_y =
	    gcry_mpi_copy(edata->context->auth.their_pub);
    edata->context->context_priv->their_old_y = NULL;

    if (edata->context->context_priv->our_keyid - 1 !=
	edata->context->auth.our_keyid ||
	gcry_mpi_cmp(edata->context->context_priv->our_old_dh_key.pub,
		edata->context->auth.our_dh.pub)) {
	otrl_dh_keypair_free(&(edata->context->context_priv->our_dh_key));
	otrl_dh_keypair_free(&(edata->context->context_priv->our_old_dh_key));
	otrl_dh_keypair_copy(&(edata->context->context_priv->our_old_dh_key),
		&(edata->context->auth.our_dh));
	otrl_dh_gen_keypair(
		edata->context->context_priv->our_old_dh_key.groupid,
		&(edata->context->context_priv->our_dh_key));
	edata->context->context_priv->our_keyid = edata->context->auth.our_keyid
		+ 1;
    }

    /* Create the session keys from the DH keys */
    otrl_dh_session_free(&(edata->context->context_priv->sesskeys[0][0]));
    err = otrl_dh_session(&(edata->context->context_priv->sesskeys[0][0]),
	    &(edata->context->context_priv->our_dh_key),
	    edata->context->context_priv->their_y);
    if (err) return err;
    otrl_dh_session_free(&(edata->context->context_priv->sesskeys[1][0]));
    err = otrl_dh_session(&(edata->context->context_priv->sesskeys[1][0]),
	    &(edata->context->context_priv->our_old_dh_key),
	    edata->context->context_priv->their_y);
    if (err) return err;

    edata->context->context_priv->generation++;
    edata->context->active_fingerprint = found_print;
    edata->context->msgstate = OTRL_MSGSTATE_ENCRYPTED;

    if (edata->ops->update_context_list) {
	edata->ops->update_context_list(edata->opdata);
    }
    if (oldstate == OTRL_MSGSTATE_ENCRYPTED && oldprint == found_print) {
	if (edata->ops->still_secure) {
	    edata->ops->still_secure(edata->opdata, edata->context,
		    edata->context->auth.initiated);
	}
    } else {
	if (edata->ops->gone_secure) {
	    edata->ops->gone_secure(edata->opdata, edata->context);
	}
    }

    edata->gone_encrypted = 1;

    return gpg_error(GPG_ERR_NO_ERROR);
}

static void maybe_resend(EncrData *edata)
{
    gcry_error_t err;
    time_t now;

    if (!edata->gone_encrypted) return;

    /* See if there's a message we sent recently that should be resent. */
    now = time(NULL);
    if (edata->context->context_priv->lastmessage != NULL &&
	    edata->context->context_priv->may_retransmit &&
	    edata->context->context_priv->lastsent >= (now - RESEND_INTERVAL)) {
	char *resendmsg;
	char *msg_to_send;
	int resending = (edata->context->context_priv->may_retransmit == 1);

	/* Initialize msg_to_send */
	if (resending) {
	    const char *resent_prefix;
	    int used_ops_resentmp = 1;
	    resent_prefix = edata->ops->resent_msg_prefix ?
				    edata->ops->resent_msg_prefix(edata->opdata,
				    edata->context) : NULL;
	    if (!resent_prefix) {
		resent_prefix = "[resent]"; /* Assign default prefix */
		used_ops_resentmp = 0;
	    }
	    msg_to_send = malloc(
		    strlen(edata->context->context_priv->lastmessage) +
		    strlen(resent_prefix) + 2);
	    if (msg_to_send) {
		strcpy(msg_to_send, resent_prefix);
		strcat(msg_to_send, " ");
		strcat(msg_to_send, edata->context->context_priv->lastmessage);
	    } else {
		return;  /* Out of memory; don't try to resend */
	    }
	    if (used_ops_resentmp) {
		edata->ops->resent_msg_prefix_free(edata->opdata,
			resent_prefix);
	    }
	} else {
	    msg_to_send = edata->context->context_priv->lastmessage;
	}

	/* Re-encrypt the message with the new keys */
	err = otrl_proto_create_data(&resendmsg,
		edata->context, msg_to_send, NULL, 0, NULL);
	if (resending) {
		free(msg_to_send);
	}
	if (!err) {
	    /* Resend the message */
	    fragment_and_send(edata->ops, edata->opdata, edata->context,
		    resendmsg, OTRL_FRAGMENT_SEND_ALL, NULL);
	    free(resendmsg);
	    edata->context->context_priv->lastsent = now;
	    otrl_context_update_recent_child(edata->context, 1);
	    if (resending) {
		/* We're not sending it for the first time; let the user
		 * know we resent it */
		if (edata->ops->handle_msg_event) {
		    edata->ops->handle_msg_event(edata->opdata,
			    OTRL_MSGEVENT_MSG_RESENT, edata->context,
			    NULL, gcry_error(GPG_ERR_NO_ERROR));
		}
	    }
	    edata->ignore_message = 1;
	}
    }
}

/* Set the trust level based on the result of the SMP */
static void set_smp_trust(const OtrlMessageAppOps *ops, void *opdata,
	ConnContext *context, int trusted)
{
    otrl_context_set_trust(context->active_fingerprint, trusted ? "smp" : "");

    /* Write the new info to disk, redraw the ui, and redraw the
     * OTR buttons. */
    if (ops->write_fingerprints) {
	ops->write_fingerprints(opdata);
    }
}

static void init_respond_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const char *question,
	const unsigned char *secret, size_t secretlen, int initiating)
{
    unsigned char *smpmsg = NULL;
    int smpmsglen;
    unsigned char combined_secret[SM_DIGEST_SIZE];
    gcry_error_t err;
    unsigned char our_fp[20];
    unsigned char *combined_buf;
    size_t combined_buf_len;
    OtrlTLV *sendtlv;
    char *sendsmp = NULL;

    if (!context || context->msgstate != OTRL_MSGSTATE_ENCRYPTED) return;

    /*
     * Construct the combined secret as a SHA256 hash of:
     * Version byte (0x01), Initiator fingerprint (20 bytes),
     * responder fingerprint (20 bytes), secure session id, input secret
     */
    otrl_privkey_fingerprint_raw(us, our_fp, context->accountname,
	    context->protocol);

    combined_buf_len = 41 + context->sessionid_len + secretlen;
    combined_buf = malloc(combined_buf_len);
    combined_buf[0] = 0x01;
    if (initiating) {
	memmove(combined_buf + 1, our_fp, 20);
	memmove(combined_buf + 21,
		context->active_fingerprint->fingerprint, 20);
    } else {
	memmove(combined_buf + 1,
		context->active_fingerprint->fingerprint, 20);
	memmove(combined_buf + 21, our_fp, 20);
    }
    memmove(combined_buf + 41, context->sessionid,
	    context->sessionid_len);
    memmove(combined_buf + 41 + context->sessionid_len,
	    secret, secretlen);
    gcry_md_hash_buffer(SM_HASH_ALGORITHM, combined_secret, combined_buf,
	    combined_buf_len);
    free(combined_buf);

    if (initiating) {
	otrl_sm_step1(context->smstate, combined_secret, SM_DIGEST_SIZE,
		&smpmsg, &smpmsglen);
    } else {
	otrl_sm_step2b(context->smstate, combined_secret, SM_DIGEST_SIZE,
		&smpmsg, &smpmsglen);
    }

    /* If we've got a question, attach it to the smpmsg */
    if (question != NULL) {
	size_t qlen = strlen(question);
	unsigned char *qsmpmsg = malloc(qlen + 1 + smpmsglen);
	if (!qsmpmsg) {
	    free(smpmsg);
	    return;
	}
	strcpy((char *)qsmpmsg, question);
	memmove(qsmpmsg + qlen + 1, smpmsg, smpmsglen);
	free(smpmsg);
	smpmsg = qsmpmsg;
	smpmsglen += qlen + 1;
    }

    /* Send msg with next smp msg content */
    sendtlv = otrl_tlv_new(initiating ?
	    (question != NULL ? OTRL_TLV_SMP1Q : OTRL_TLV_SMP1)
	    : OTRL_TLV_SMP2,
	    smpmsglen, smpmsg);
    err = otrl_proto_create_data(&sendsmp, context, "", sendtlv,
	    OTRL_MSGFLAGS_IGNORE_UNREADABLE, NULL);
    if (!err) {
	/*  Send it, and set the next expected message to the
	 *  logical response */
	err = fragment_and_send(ops, opdata, context,
		sendsmp, OTRL_FRAGMENT_SEND_ALL, NULL);
	context->smstate->nextExpected =
		initiating ? OTRL_SMP_EXPECT2 : OTRL_SMP_EXPECT3;
    }
    free(sendsmp);
    otrl_tlv_free(sendtlv);
    free(smpmsg);
}

/* Initiate the Socialist Millionaires' Protocol */
void otrl_message_initiate_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const unsigned char *secret,
	size_t secretlen)
{
    init_respond_smp(us, ops, opdata, context, NULL, secret, secretlen, 1);
}

/* Initiate the Socialist Millionaires' Protocol and send a prompt
 * question to the buddy */
void otrl_message_initiate_smp_q(OtrlUserState us,
	const OtrlMessageAppOps *ops, void *opdata, ConnContext *context,
	const char *question, const unsigned char *secret, size_t secretlen)
{
    init_respond_smp(us, ops, opdata, context, question, secret, secretlen, 1);
}

/* Respond to a buddy initiating the Socialist Millionaires' Protocol */
void otrl_message_respond_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const unsigned char *secret,
	size_t secretlen)
{
    init_respond_smp(us, ops, opdata, context, NULL, secret, secretlen, 0);
}

/* Abort the SMP.  Called when an unexpected SMP message breaks the
 * normal flow. */
void otrl_message_abort_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context)
{
    OtrlTLV *sendtlv = otrl_tlv_new(OTRL_TLV_SMP_ABORT, 0,
	    (const unsigned char *)"");
    char *sendsmp = NULL;
    gcry_error_t err;

    context->smstate->nextExpected = OTRL_SMP_EXPECT1;

    err = otrl_proto_create_data(&sendsmp,
	    context, "", sendtlv,
	    OTRL_MSGFLAGS_IGNORE_UNREADABLE, NULL);
    if (!err) {
	/* Send the abort signal so our buddy knows we've stopped */
	err = fragment_and_send(ops, opdata, context,
		sendsmp, OTRL_FRAGMENT_SEND_ALL, NULL);
    }
    free(sendsmp);
    otrl_tlv_free(sendtlv);
}

static void message_malformed(const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context) {
    if (ops->handle_msg_event) {
	ops->handle_msg_event(opdata, OTRL_MSGEVENT_RCVDMSG_MALFORMED, context,
	    NULL, gcry_error(GPG_ERR_NO_ERROR));
    }

    if (ops->inject_message && ops->otr_error_message) {
	const char *err_msg = ops->otr_error_message(opdata, context,
		OTRL_ERRCODE_MSG_MALFORMED);

	if (err_msg) {
	    char *buf = malloc(strlen(OTR_ERROR_PREFIX) + strlen(err_msg) + 1);

	    if (buf) {
		strcpy(buf, OTR_ERROR_PREFIX);
		strcat(buf, err_msg);
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, buf);
		free(buf);
	    }

	    if (ops->otr_error_message_free) {
		ops->otr_error_message_free(opdata, err_msg);
	    }
	}
    }
}


/* Handle a message just received from the network.  It is safe to pass
 * all received messages to this routine.  add_appdata is a function
 * that will be called in the event that a new ConnContext is created.
 * It will be passed the data that you supplied, as well as
 * a pointer to the new ConnContext.  You can use this to add
 * application-specific information to the ConnContext using the
 * "context->app" field, for example.  If you don't need to do this, you
 * can pass NULL for the last two arguments of otrl_message_receiving.
 *
 * If non-NULL, ops->convert_msg will be called after a data message is
 * decrypted.
 *
 * If "contextp" is not NULL, it will be set to the ConnContext used for
 * receiving the message.
 *
 * If otrl_message_receiving returns 1, then the message you received
 * was an internal protocol message, and no message should be delivered
 * to the user.
 *
 * If it returns 0, then check if *messagep was set to non-NULL.  If
 * so, replace the received message with the contents of *messagep, and
 * deliver that to the user instead.  You must call
 * otrl_message_free(*messagep) when you're done with it.  If tlvsp is
 * non-NULL, *tlvsp will be set to a chain of any TLVs that were
 * transmitted along with this message.  You must call
 * otrl_tlv_free(*tlvsp) when you're done with those.
 *
 * If otrl_message_receiving returns 0 and *messagep is NULL, then this
 * was an ordinary, non-OTR message, which should just be delivered to
 * the user without modification. */
int otrl_message_receiving(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *sender, const char *message, char **newmessagep,
	OtrlTLV **tlvsp, ConnContext **contextp,
	void (*add_appdata)(void *data, ConnContext *context),
	void *data)
{
    ConnContext *context, *m_context, *best_context;
    OtrlMessageType msgtype;
    int context_added = 0;
    OtrlPolicy policy = OTRL_POLICY_DEFAULT;
    char *unfragmessage = NULL, *otrtag = NULL;
    EncrData edata;
    otrl_instag_t our_instance = 0, their_instance = 0;
    int version;
    gcry_error_t err;

    if (!accountname || !protocol || !sender || !message || !newmessagep)
	return 0;

    *newmessagep = NULL;
    if (tlvsp) *tlvsp = NULL;

    if (contextp) {
	*contextp = NULL;
    }

    /* Find the master context and state with this correspondent */
    m_context = otrl_context_find(us, sender, accountname,
	    protocol, OTRL_INSTAG_MASTER, 1, &context_added, add_appdata, data);
    context = m_context;

    /* Update the context list if we added one */
    if (context_added && ops->update_context_list) {
	ops->update_context_list(opdata);
    }

    best_context = otrl_context_find(us, sender, accountname,
	    protocol, OTRL_INSTAG_BEST, 0, NULL, add_appdata, data);

    /* Find or generate the instance tag if needed */
    if (!context->our_instance) {
	populate_context_instag(us, ops, opdata, accountname, protocol,
		context);
    }


    /* Check the policy */
    if (ops->policy) {
	policy = ops->policy(opdata, context);
    }

    /* Should we go on at all? */
    if ((policy & OTRL_POLICY_VERSION_MASK) == 0) {
	return 0;
    }

    otrtag = strstr(message, "?OTR");
    if (otrtag) {
	/* See if we have a V3 fragment.  The '4' in the next line is
	 * strlen("?OTR").  otrtag[4] is the character immediately after
	 * the "?OTR", and is guaranteed to exist, because in the worst
	 * case, it is the NUL terminating 'message'. */
	if (otrtag[4] == '|') {
	    /* Get the instance tag from fragment header*/
	    sscanf(otrtag, "?OTR|%x|%x,", &their_instance, &our_instance);
	    /* Ignore message if it is intended for a different instance */
	    if (our_instance && context->our_instance != our_instance) {

		    if (ops->handle_msg_event) {
			ops->handle_msg_event(opdata,
				OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE,
				m_context, NULL, gcry_error(GPG_ERR_NO_ERROR));
		    }
		    return 1;
	    }
	    /* Get the context for this instance */
	    if (their_instance >= OTRL_MIN_VALID_INSTAG) {
		context = otrl_context_find(us, sender, accountname,
			protocol, their_instance, 1, &context_added,
			add_appdata, data);
	    } else {
		message_malformed(ops, opdata, context);
		return 1;
	    }
	}
	switch(otrl_proto_fragment_accumulate(&unfragmessage,
		context, message)) {
	    case OTRL_FRAGMENT_UNFRAGMENTED:
		/* Do nothing */
		break;
	    case OTRL_FRAGMENT_INCOMPLETE:
		/* We've accumulated this fragment, but we don't have a
		 * complete message yet */
		return 1;
	    case OTRL_FRAGMENT_COMPLETE:
		/* We've got a new complete message, in unfragmessage. */
		message = unfragmessage;
		otrtag = strstr(message, "?OTR");
		break;
	}
    }

    /* What type of message is it?  Note that this just checks the
     * header; it's not necessarily a _valid_ message of this type. */
    msgtype = otrl_proto_message_type(message);
    version = otrl_proto_message_version(message);

    /* See if they responded to our OTR offer */
    if ((policy & OTRL_POLICY_SEND_WHITESPACE_TAG)) {
	if (msgtype != OTRL_MSGTYPE_NOTOTR) {
	    context->otr_offer = OFFER_ACCEPTED;
	} else if (context->otr_offer == OFFER_SENT) {
	    context->otr_offer = OFFER_REJECTED;
	}
    }

    /* Check that this version is allowed by the policy */
    if (((version == 3) && !(policy & OTRL_POLICY_ALLOW_V3))
	|| ((version == 2) && !(policy & OTRL_POLICY_ALLOW_V2))
	|| ((version == 1) && !(policy & OTRL_POLICY_ALLOW_V1))) {
	    edata.ignore_message = 1;
	    goto end;
    }
    /* Check the to and from instance tags */
    if (version == 3) {
	err = gcry_error(GPG_ERR_INV_VALUE);
	if (otrtag) {
	    err = otrl_proto_instance(otrtag, &their_instance, &our_instance);
	}
	if (!err) {
	    if ((msgtype == OTRL_MSGTYPE_DH_COMMIT && our_instance &&
		    context->our_instance != our_instance) ||
		    (msgtype != OTRL_MSGTYPE_DH_COMMIT &&
		    context->our_instance != our_instance)) {
		if (ops->handle_msg_event) {
		    ops->handle_msg_event(opdata,
			    OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE,
			    m_context, NULL, gcry_error(GPG_ERR_NO_ERROR));
		}
		/* ignore message intended for a different instance */
		edata.ignore_message = 1;
		goto end;
	    }

	    if (their_instance >= OTRL_MIN_VALID_INSTAG) {
		context = otrl_context_find(us, sender, accountname,
			protocol, their_instance, 1, &context_added,
			add_appdata, data);
	    }
	}

	if (err || their_instance < OTRL_MIN_VALID_INSTAG) {
	    message_malformed(ops, opdata, context);
	    edata.ignore_message = 1;
	    goto end;
	}

	if (context_added) {
	    /* Context added because of new instance (either here or when
	     * accumulating fragments */
	    /* Copy information from m_context to the new instance context */
	    context->auth.protocol_version = 3;
	    context->protocol_version = 3;
	    context->msgstate = m_context->msgstate;

	    if (m_context->context_priv->may_retransmit) {
		gcry_free(context->context_priv->lastmessage);
		context->context_priv->lastmessage = m_context->context_priv->lastmessage;
		m_context->context_priv->lastmessage = NULL;
		context->context_priv->may_retransmit = m_context->context_priv->may_retransmit;
		m_context->context_priv->may_retransmit = 0;
	    }

	    if (msgtype == OTRL_MSGTYPE_DH_KEY) {
		otrl_auth_copy_on_key(&(m_context->auth), &(context->auth));
	    } else if (msgtype != OTRL_MSGTYPE_DH_COMMIT) {
		edata.ignore_message = 1;
		goto end;
	    }

	    /* Update the context list */
	    if (ops->update_context_list) {
		ops->update_context_list(opdata);
	    }
	} else if (m_context != context) {
	    /* Switching from m_context to existing instance context */
	    if (msgtype == OTRL_MSGTYPE_DH_KEY && m_context->auth.authstate
		    == OTRL_AUTHSTATE_AWAITING_DHKEY &&
		    !(context->auth.authstate ==
		    OTRL_AUTHSTATE_AWAITING_DHKEY)) {
		context->msgstate = m_context->msgstate;
		context->auth.protocol_version = 3;
		context->protocol_version = 3;
		otrl_auth_copy_on_key(&(m_context->auth), &(context->auth));
	    }
	}
    }

    if (contextp) {
	*contextp = context;
    }

    /* update time of last received message */
    context->context_priv->lastrecv = time(NULL);
    otrl_context_update_recent_child(context, 0);

    edata.gone_encrypted = 0;
    edata.us = us;
    edata.context = context;
    edata.ops = ops;
    edata.opdata = opdata;
    edata.ignore_message = -1;
    edata.messagep = newmessagep;

    switch(msgtype) {
	unsigned int bestversion;
	const char *startwhite, *endwhite;
	DH_keypair *our_dh;
	unsigned int our_keyid;
	OtrlPrivKey *privkey;
	int haveauthmsg;

	case OTRL_MSGTYPE_QUERY:
	    /* See if we should use an existing DH keypair, or generate
	     * a fresh one. */
	    if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		our_dh = &(context->context_priv->our_old_dh_key);
		our_keyid = context->context_priv->our_keyid - 1;
	    } else {
		our_dh = NULL;
		our_keyid = 0;
	    }

	    /* Find the best version of OTR that we both speak */
	    switch(otrl_proto_query_bestversion(message, policy)) {
		case 3:
		    err = otrl_auth_start_v23(&(context->auth), 3);
		    send_or_error_auth(ops, opdata, err, context, us);
		    break;
		case 2:
		    err = otrl_auth_start_v23(&(context->auth), 2);
		    send_or_error_auth(ops, opdata, err, context, us);
		    break;
		case 1:
		    /* Get our private key */
		    privkey = otrl_privkey_find(us, context->accountname,
			    context->protocol);
		    if (privkey == NULL) {
			/* We've got no private key! */
			if (ops->create_privkey) {
			    ops->create_privkey(opdata, context->accountname,
				    context->protocol);
			    privkey = otrl_privkey_find(us,
				    context->accountname, context->protocol);
			}
		    }
		    if (privkey) {
			err = otrl_auth_start_v1(&(context->auth), our_dh,
				our_keyid, privkey);
			send_or_error_auth(ops, opdata, err, context, us);
		    }
		    break;
		default:
		    /* Just ignore this message */
		    break;
	    }
	    /* Don't display the Query message to the user. */
	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;

	case OTRL_MSGTYPE_DH_COMMIT:
	    err = otrl_auth_handle_commit(&(context->auth), otrtag, version);
	    send_or_error_auth(ops, opdata, err, context, us);

	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;

	case OTRL_MSGTYPE_DH_KEY:
	    /* Get our private key */
	    privkey = otrl_privkey_find(us, context->accountname,
		    context->protocol);
	    if (privkey == NULL) {
		/* We've got no private key! */
		if (ops->create_privkey) {
		    ops->create_privkey(opdata, context->accountname,
			    context->protocol);
		    privkey = otrl_privkey_find(us,
			    context->accountname, context->protocol);
		}
	    }
	    if (privkey) {
		err = otrl_auth_handle_key(&(context->auth), otrtag,
			&haveauthmsg, privkey);
		if (err || haveauthmsg) {
		    send_or_error_auth(ops, opdata, err, context, us);
		}
	    }

	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;

	case OTRL_MSGTYPE_REVEALSIG:
	    /* Get our private key */
	    privkey = otrl_privkey_find(us, context->accountname,
		    context->protocol);
	    if (privkey == NULL) {
		/* We've got no private key! */
		if (ops->create_privkey) {
		    ops->create_privkey(opdata, context->accountname,
			    context->protocol);
		    privkey = otrl_privkey_find(us,
			    context->accountname, context->protocol);
		}
	    }
	    if (privkey) {
		err = otrl_auth_handle_revealsig(&(context->auth),
			otrtag, &haveauthmsg, privkey, go_encrypted,
			&edata);
		if (err || haveauthmsg) {
		    send_or_error_auth(ops, opdata, err, context, us);
		    maybe_resend(&edata);
		}
	    }

	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;

	case OTRL_MSGTYPE_SIGNATURE:
	    err = otrl_auth_handle_signature(&(context->auth),
		    otrtag, &haveauthmsg, go_encrypted, &edata);
	    if (err || haveauthmsg) {
		send_or_error_auth(ops, opdata, err, context, us);
		maybe_resend(&edata);
	    }

	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;

	case OTRL_MSGTYPE_V1_KEYEXCH:
	    /* See if we should use an existing DH keypair, or generate
	     * a fresh one. */
	    if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		our_dh = &(context->context_priv->our_old_dh_key);
		our_keyid = context->context_priv->our_keyid - 1;
	    } else {
		our_dh = NULL;
		our_keyid = 0;
	    }

	    /* Get our private key */
	    privkey = otrl_privkey_find(us, context->accountname,
		    context->protocol);
	    if (privkey == NULL) {
		/* We've got no private key! */
		if (ops->create_privkey) {
		    ops->create_privkey(opdata, context->accountname,
			    context->protocol);
		    privkey = otrl_privkey_find(us, context->accountname,
			    context->protocol);
		}
	    }
	    if (privkey) {
		err = otrl_auth_handle_v1_key_exchange(&(context->auth),
			message, &haveauthmsg, privkey, our_dh, our_keyid,
			go_encrypted, &edata);
		if (err || haveauthmsg) {
		    send_or_error_auth(ops, opdata, err, context, us);
		    maybe_resend(&edata);
		}
	    }

	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;

	case OTRL_MSGTYPE_DATA:
	    switch(context->msgstate) {
		gcry_error_t err;
		OtrlTLV *tlvs, *tlv;
		char *plaintext;
		char *buf;
		const char *err_msg;
		unsigned char *extrakey;
		unsigned char flags;
		NextExpectedSMP nextMsg;

		case OTRL_MSGSTATE_PLAINTEXT:
		case OTRL_MSGSTATE_FINISHED:
		    /* See if we're supposed to ignore this message in
		     * the event it's unreadable. */
		    err = otrl_proto_data_read_flags(message, &flags);
		    if ((flags & OTRL_MSGFLAGS_IGNORE_UNREADABLE)) {
			edata.ignore_message = 1;
			break;
		    }

		    if(best_context && best_context != context &&
			best_context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {

			if (ops->handle_msg_event) {
			    ops->handle_msg_event(opdata,
				    OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE,
				    m_context, NULL,
				    gcry_error(GPG_ERR_NO_ERROR));
			}
		    } else if (ops->handle_msg_event) {
			ops->handle_msg_event(opdata,
				    OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE,
				    context, NULL,
				    gcry_error(GPG_ERR_NO_ERROR));
		    }
		    edata.ignore_message = 1;

		    /* We don't actually want to send anything in this case,
		       since this could just be a message intended for another
		       v2 instance.  We still notify the local user though */
		    break;

		case OTRL_MSGSTATE_ENCRYPTED:
		    extrakey = gcry_malloc_secure(OTRL_EXTRAKEY_BYTES);
		    err = otrl_proto_accept_data(&plaintext, &tlvs, context,
				    message, &flags, extrakey);
		    if (err) {
			int is_conflict =
				(gpg_err_code(err) == GPG_ERR_CONFLICT);
			if ((flags & OTRL_MSGFLAGS_IGNORE_UNREADABLE)) {
			    edata.ignore_message = 1;
			    break;
			}
			if (is_conflict) {
			    if (ops->handle_msg_event) {
				ops->handle_msg_event(opdata,
					OTRL_MSGEVENT_RCVDMSG_UNREADABLE,
					context, NULL,
					gcry_error(GPG_ERR_NO_ERROR));
			    }
			} else {
			    if (ops->handle_msg_event) {
				ops->handle_msg_event(opdata,
					OTRL_MSGEVENT_RCVDMSG_MALFORMED,
					context, NULL,
					gcry_error(GPG_ERR_NO_ERROR));
			    }
			}
			if (ops->inject_message && ops->otr_error_message) {
			    err_msg = ops->otr_error_message(opdata,
					context,
					is_conflict ?
					    OTRL_ERRCODE_MSG_UNREADABLE :
					    OTRL_ERRCODE_MSG_MALFORMED);
			    if (err_msg) {
				buf = malloc(strlen(OTR_ERROR_PREFIX) +
						strlen(err_msg) + 1);
				if (buf) {
				    strcpy(buf, OTR_ERROR_PREFIX);
				    strcat(buf, err_msg);
				    ops->inject_message(opdata,
					    accountname, protocol,
					    sender, buf);
				    free(buf);
				}
			    }
			    if (ops->otr_error_message_free) {
				ops->otr_error_message_free(opdata,
					err_msg);
			    }
			}
			edata.ignore_message = 1;
			break;
		    }

		    /* If the other side told us he's disconnected his
		     * private connection, make a note of that so we
		     * don't try sending anything else to him. */
		    if (otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED)) {
			otrl_context_force_finished(context);
		    }

		    /* If the other side told us to use the current
		     * extra symmetric key, let the application know. */
		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SYMKEY);
		    if (tlv && otrl_api_version >= 0x040000) {
			if (ops->received_symkey && tlv->len >= 4) {
			    unsigned char *bufp = tlv->data;
			    unsigned int use =
				(bufp[0] << 24) | (bufp[1] << 16) |
				(bufp[2] << 8) | bufp[3];
			    ops->received_symkey(opdata, context, use,
				    bufp+4, tlv->len - 4, extrakey);
			}
		    }
		    gcry_free(extrakey);
		    extrakey = NULL;

		    /* If TLVs contain SMP data, process it */
		    nextMsg = context->smstate->nextExpected;

		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q);
		    if (tlv) {
			if (nextMsg == OTRL_SMP_EXPECT1 && tlv->len > 0) {
			    /* We can only do the verification half now.
			     * We must wait for the secret to be entered
			     * to continue. */
			    char *question = (char *)tlv->data;
			    char *qend = memchr(question, '\0', tlv->len - 1);
			    size_t qlen = qend ? (qend - question + 1) :
				    tlv->len;
			    otrl_sm_step2a(context->smstate, tlv->data + qlen,
				    tlv->len - qlen, 1);

			    if (context->smstate->sm_prog_state !=
				    OTRL_SMP_PROG_CHEATED) {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_ASK_FOR_ANSWER,
					    context, 25, question);
				}
			    } else {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_CHEATED, context,
					    0, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT1;
				context->smstate->sm_prog_state =
					OTRL_SMP_PROG_OK;
			    }
			} else {
			    if (ops->handle_smp_event) {
				ops->handle_smp_event(opdata,
					OTRL_SMPEVENT_ERROR, context,
					0, NULL);
			    }
			}
		    }

		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1);
		    if (tlv) {
			if (nextMsg == OTRL_SMP_EXPECT1) {
			    /* We can only do the verification half now.
			     * We must wait for the secret to be entered
			     * to continue. */
			    otrl_sm_step2a(context->smstate, tlv->data,
				    tlv->len, 0);
			    if (context->smstate->sm_prog_state !=
				    OTRL_SMP_PROG_CHEATED) {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_ASK_FOR_SECRET,
					    context, 25, NULL);
				}
			    } else {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_CHEATED,
					    context, 0, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT1;
				context->smstate->sm_prog_state =
					OTRL_SMP_PROG_OK;
			    }
			} else {
			    if (ops->handle_smp_event) {
				ops->handle_smp_event(opdata,
					OTRL_SMPEVENT_ERROR, context,
					0, NULL);
			    }
			}
		    }

		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP2);
		    if (tlv) {
			if (nextMsg == OTRL_SMP_EXPECT2) {
			    unsigned char* nextmsg;
			    int nextmsglen;
			    OtrlTLV *sendtlv;
			    char *sendsmp;
			    otrl_sm_step3(context->smstate, tlv->data,
				    tlv->len, &nextmsg, &nextmsglen);

			    if (context->smstate->sm_prog_state !=
				    OTRL_SMP_PROG_CHEATED) {
				/* Send msg with next smp msg content */
				sendtlv = otrl_tlv_new(OTRL_TLV_SMP3,
					nextmsglen, nextmsg);
				err = otrl_proto_create_data(&sendsmp,
					context, "", sendtlv,
					OTRL_MSGFLAGS_IGNORE_UNREADABLE,
					NULL);
				if (!err) {
				err = fragment_and_send(ops,
					opdata, context, sendsmp,
					OTRL_FRAGMENT_SEND_ALL, NULL);
				}
				free(sendsmp);
				otrl_tlv_free(sendtlv);

				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_IN_PROGRESS,
					    context, 60, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT4;
			    } else {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_CHEATED,
					    context, 0, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT1;
				context->smstate->sm_prog_state =
					OTRL_SMP_PROG_OK;
			    }
			    free(nextmsg);
			} else {
			    if (ops->handle_smp_event) {
				ops->handle_smp_event(opdata,
					OTRL_SMPEVENT_ERROR, context,
					0, NULL);
			    }
			}
		    }

		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP3);
		    if (tlv) {
			if (nextMsg == OTRL_SMP_EXPECT3) {
			    unsigned char* nextmsg;
			    int nextmsglen;
			    OtrlTLV *sendtlv;
			    char *sendsmp;
			    err = otrl_sm_step4(context->smstate, tlv->data,
				    tlv->len, &nextmsg, &nextmsglen);
			    /* Set trust level based on result */
			    if (context->smstate->received_question == 0) {
				set_smp_trust(ops, opdata, context,
					(err == gcry_error(GPG_ERR_NO_ERROR)));
			    }

			    if (context->smstate->sm_prog_state !=
				    OTRL_SMP_PROG_CHEATED) {
				/* Send msg with next smp msg content */
				sendtlv = otrl_tlv_new(OTRL_TLV_SMP4,
					nextmsglen, nextmsg);
				err = otrl_proto_create_data(&sendsmp,
					context, "", sendtlv,
					OTRL_MSGFLAGS_IGNORE_UNREADABLE,
					NULL);
				if (!err) {
				err = fragment_and_send(ops,
					opdata, context, sendsmp,
					OTRL_FRAGMENT_SEND_ALL, NULL);
				}
				free(sendsmp);
				otrl_tlv_free(sendtlv);

				if (ops->handle_smp_event) {
				    OtrlSMPEvent succorfail =
					context->smstate->sm_prog_state ==
						OTRL_SMP_PROG_SUCCEEDED ?
					    OTRL_SMPEVENT_SUCCESS :
					    OTRL_SMPEVENT_FAILURE;
				    ops->handle_smp_event(opdata, succorfail,
					    context, 100, NULL);
				}
				context->smstate->nextExpected =
				    OTRL_SMP_EXPECT1;
			    } else {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_CHEATED,
					    context, 0, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT1;
				context->smstate->sm_prog_state =
					OTRL_SMP_PROG_OK;
			    }
			    free(nextmsg);
			} else {
			    if (ops->handle_smp_event) {
				ops->handle_smp_event(opdata,
					OTRL_SMPEVENT_ERROR, context,
					0, NULL);
			    }
			}
		    }

		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP4);
		    if (tlv) {
			if (nextMsg == OTRL_SMP_EXPECT4) {
			    err = otrl_sm_step5(context->smstate, tlv->data,
				    tlv->len);
			    /* Set trust level based on result */
			    set_smp_trust(ops, opdata, context,
				    (err == gcry_error(GPG_ERR_NO_ERROR)));

			    if (context->smstate->sm_prog_state !=
				    OTRL_SMP_PROG_CHEATED) {
				if (ops->handle_smp_event) {
				    OtrlSMPEvent succorfail =
					context->smstate->sm_prog_state ==
						OTRL_SMP_PROG_SUCCEEDED ?
					    OTRL_SMPEVENT_SUCCESS :
					    OTRL_SMPEVENT_FAILURE;
				    ops->handle_smp_event(opdata, succorfail,
					    context, 100, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT1;
			    } else {
				if (ops->handle_smp_event) {
				    ops->handle_smp_event(opdata,
					    OTRL_SMPEVENT_CHEATED,
					    context, 0, NULL);
				}
				context->smstate->nextExpected =
					OTRL_SMP_EXPECT1;
				context->smstate->sm_prog_state =
					OTRL_SMP_PROG_OK;
			    }
			} else {
			    if (ops->handle_smp_event) {
				ops->handle_smp_event(opdata,
					OTRL_SMPEVENT_ERROR, context,
					0, NULL);
			    }
			}
		    }

		    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP_ABORT);
		    if (tlv) {
			context->smstate->nextExpected = OTRL_SMP_EXPECT1;
			if (ops->handle_smp_event) {
			    ops->handle_smp_event(opdata, OTRL_SMPEVENT_ABORT,
				    context, 0, NULL);
			}
		    }

		    if (plaintext[0] == '\0') {
			/* If it's a heartbeat (an empty message), don't
			 * display it to the user, but signal an event. */
			if (ops->handle_msg_event) {
			    ops->handle_msg_event(opdata,
				    OTRL_MSGEVENT_LOG_HEARTBEAT_RCVD,
				    context, NULL,
				    gcry_error(GPG_ERR_NO_ERROR));
			}
			edata.ignore_message = 1;
		    } else if (edata.ignore_message != 1 &&
			    context->context_priv->their_keyid > 0) {
			/* If it's *not* a heartbeat, and we haven't
			 * sent anything in a while, also send a
			 * heartbeat. */
			time_t now = time(NULL);
			if (context->context_priv->lastsent <
				(now - HEARTBEAT_INTERVAL)) {
			    char *heartbeat;

			    /* Create the heartbeat message */
			    err = otrl_proto_create_data(&heartbeat,
				    context, "", NULL,
				    OTRL_MSGFLAGS_IGNORE_UNREADABLE,
				    NULL);
			    if (!err) {
				/* Send it, and inject a debug message */
				if (ops->inject_message) {
				    ops->inject_message(opdata, accountname,
					    protocol, sender, heartbeat);
				}
				free(heartbeat);

				context->context_priv->lastsent = now;
				otrl_context_update_recent_child(context, 1);

				/* Signal an event for the heartbeat message */
				if (ops->handle_msg_event) {
				    ops->handle_msg_event(opdata,
					    OTRL_MSGEVENT_LOG_HEARTBEAT_SENT,
					    context, NULL,
					    gcry_error(GPG_ERR_NO_ERROR));
				}
			    }
			}
		    }

		    /* Return the TLVs even if ignore_message == 1 so
		     * that we can attach TLVs to heartbeats. */
		    if (tlvsp) {
			*tlvsp = tlvs;
		    } else {
			otrl_tlv_free(tlvs);
		    }

		    if (edata.ignore_message != 1) {
			char *converted_msg = NULL;

			*newmessagep = plaintext;
			edata.ignore_message = 0;

			/* convert the plaintext message if necessary */
			if (ops->convert_msg) {
			    ops->convert_msg(opdata, context,
				    OTRL_CONVERT_RECEIVING, &converted_msg,
				    plaintext);

			    if (converted_msg) {
				free(plaintext);
				plaintext = NULL;
				*newmessagep = strdup(converted_msg);

				if (ops->convert_free) {
				    ops->convert_free(opdata, context,
					    converted_msg);
				}
			    }
			}
		    } else {
			free(plaintext);
		    }
		    break;
	    }
	    break;

	case OTRL_MSGTYPE_ERROR:
	    if ((policy & OTRL_POLICY_ERROR_START_AKE)) {
		char *msgtosend = otrl_proto_default_query_msg(
			context->accountname, policy);
		if (msgtosend && ops->inject_message) {
		    ops->inject_message(opdata, context->accountname,
			    context->protocol, context->username,
			    msgtosend);
		}
		free(msgtosend);
	    }

	    if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		/* Mark the last message we sent as eligible for
		 * retransmission */
		context->context_priv->may_retransmit = 1;
	    }

	    /* In any event, display the error message, with the
	     * display_otr_message callback, if possible */
	    if (ops->handle_msg_event) {
		/* Remove the OTR error prefix and pass the msg */
		const char *just_err_msg = strstr(message, OTR_ERROR_PREFIX);
		if (!just_err_msg) {
		    just_err_msg = message;
		} else {
		    just_err_msg += (strlen(OTR_ERROR_PREFIX));
		    if (*just_err_msg == ' ') {
			/* Advance pointer to skip the space character */
			just_err_msg++;
		    }
		    ops->handle_msg_event(opdata,
			    OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR,
			    context, just_err_msg,
			    gcry_error(GPG_ERR_NO_ERROR));
		    edata.ignore_message = 1;
		}
	    }
	    break;

	case OTRL_MSGTYPE_TAGGEDPLAINTEXT:
	    /* Strip the tag from the message */
	    bestversion = otrl_proto_whitespace_bestversion(message,
		    &startwhite, &endwhite, policy);
	    if (startwhite && endwhite) {
		size_t restlen = strlen(endwhite);
		char *strippedmsg = strdup(message);

		if (strippedmsg) {
		    memmove(strippedmsg + (startwhite - message),
			    strippedmsg + (endwhite - message), restlen+1);
		    *newmessagep = strippedmsg;
		    edata.ignore_message = 0;
		}
	    }
	    if (bestversion && context->msgstate != OTRL_MSGSTATE_ENCRYPTED
		    && (policy & OTRL_POLICY_WHITESPACE_START_AKE)) {
		switch(bestversion) {
		    case 3:
			err = otrl_auth_start_v23(&(context->auth), 3);
			send_or_error_auth(ops, opdata, err, context, us);
			break;
		    case 2:
			err = otrl_auth_start_v23(&(context->auth), 2);
			send_or_error_auth(ops, opdata, err, context, us);
			break;
		    case 1:
			/* Get our private key */
			privkey = otrl_privkey_find(us, context->accountname,
				context->protocol);
			if (privkey == NULL) {
			    /* We've got no private key! */
			    if (ops->create_privkey) {
				ops->create_privkey(opdata,
					context->accountname,
					context->protocol);
				privkey = otrl_privkey_find(us,
					context->accountname,
					context->protocol);
			    }
			}
			if (privkey) {
			    err = otrl_auth_start_v1(&(context->auth), NULL, 0,
				    privkey);
			    send_or_error_auth(ops, opdata, err, context, us);
			}
			break;
		    default:
			/* Don't start the AKE */
			break;
		}
	    }

	    /* FALLTHROUGH */
	case OTRL_MSGTYPE_NOTOTR:
	    if (best_context->msgstate != OTRL_MSGSTATE_PLAINTEXT ||
		    (policy & OTRL_POLICY_REQUIRE_ENCRYPTION)) {
		/* Not fine.  Let the user know. */
		const char *plainmsg = (*newmessagep) ? *newmessagep : message;
		if (ops->handle_msg_event) {
		    ops->handle_msg_event(opdata,
			    OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED,
			    context, plainmsg, gcry_error(GPG_ERR_NO_ERROR));
		    free(*newmessagep);
		    *newmessagep = NULL;
		    edata.ignore_message = 1;
		}
	    }
	    break;

	case OTRL_MSGTYPE_UNKNOWN:
	    /* We received an OTR message we didn't recognize.  Ignore
	     * it, and signal an event. */
	    if (ops->handle_msg_event) {
		ops->handle_msg_event(opdata,
			OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED,
			context, NULL, gcry_error(GPG_ERR_NO_ERROR));
	    }
	    if (edata.ignore_message == -1) edata.ignore_message = 1;
	    break;
    }

end:
    /* If we reassembled a fragmented message, we need to free the
     * allocated memory now. */
    free(unfragmessage);

    if (edata.ignore_message == -1) edata.ignore_message = 0;
    return edata.ignore_message;
}

/* Put a connection into the PLAINTEXT state, first sending the
 * other side a notice that we're doing so if we're currently ENCRYPTED,
 * and we think he's logged in. Affects only the specified context. */
static void disconnect_context(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context)
{
    if (!context) return;

    if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED &&
	    context->context_priv->their_keyid > 0 &&
	    ops->is_logged_in &&
	    ops->is_logged_in(opdata, context->accountname, context->protocol,
		    context->username) == 1) {
	if (ops->inject_message) {
	    char *encmsg = NULL;
	    gcry_error_t err;
	    OtrlTLV *tlv = otrl_tlv_new(OTRL_TLV_DISCONNECTED, 0, NULL);

	    err = otrl_proto_create_data(&encmsg, context, "", tlv,
		    OTRL_MSGFLAGS_IGNORE_UNREADABLE, NULL);
	    if (!err) {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, encmsg);
	    }
	    free(encmsg);
	    otrl_tlv_free(tlv);
	}
    }

    otrl_context_force_plaintext(context);
    if (ops->update_context_list) {
	ops->update_context_list(opdata);
    }
}


/* Put a connection into the PLAINTEXT state, first sending the
 * other side a notice that we're doing so if we're currently ENCRYPTED,
 * and we think he's logged in. Affects only the specified instance. */
void otrl_message_disconnect(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *username, otrl_instag_t instance)
{
    ConnContext *context = otrl_context_find(us, username, accountname,
	    protocol, instance, 0, NULL, NULL, NULL);

    if (!context) return;

    disconnect_context(us, ops, opdata, context);
}

/* Put a connection into the PLAINTEXT state, first sending the
 * other side a notice that we're doing so if we're currently ENCRYPTED,
 * and we think he's logged in. Affects all matching instances. */
void otrl_message_disconnect_all_instances(OtrlUserState us,
	const OtrlMessageAppOps *ops, void *opdata, const char *accountname,
	const char *protocol, const char *username)
{
    ConnContext * c_iter;
    ConnContext *context;

    if (!username || !accountname || !protocol) return;

    context = otrl_context_find(us, username, accountname,
	    protocol, OTRL_INSTAG_MASTER, 0, NULL, NULL, NULL);

    if (!context) return;

    for (c_iter = context; c_iter && c_iter->m_context == context->m_context;
	c_iter = c_iter->next) {
	disconnect_context(us, ops, opdata, c_iter);
    }
}

/* Get the current extra symmetric key (of size OTRL_EXTRAKEY_BYTES
 * bytes) and let the other side know what we're going to use it for.
 * The key is stored in symkey, which must already be allocated
 * and OTRL_EXTRAKEY_BYTES bytes long. */
gcry_error_t otrl_message_symkey(OtrlUserState us,
	const OtrlMessageAppOps *ops, void *opdata, ConnContext *context,
	unsigned int use, const unsigned char *usedata, size_t usedatalen,
	unsigned char *symkey)
{
    if (!context || (usedatalen > 0 && !usedata)) {
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED &&
	    context->context_priv->their_keyid > 0) {
	unsigned char *tlvdata = malloc(usedatalen+4);
	char *encmsg = NULL;
	gcry_error_t err;
	OtrlTLV *tlv;

	tlvdata[0] = (use >> 24) & 0xff;
	tlvdata[1] = (use >> 16) & 0xff;
	tlvdata[2] = (use >> 8) & 0xff;
	tlvdata[3] = (use) & 0xff;
	if (usedatalen > 0) {
	    memmove(tlvdata+4, usedata, usedatalen);
	}

	tlv = otrl_tlv_new(OTRL_TLV_SYMKEY, usedatalen+4, tlvdata);
	free(tlvdata);

	err = otrl_proto_create_data(&encmsg, context, "", tlv,
		OTRL_MSGFLAGS_IGNORE_UNREADABLE, symkey);
	if (!err && ops->inject_message) {
	    ops->inject_message(opdata, context->accountname,
		    context->protocol, context->username, encmsg);
	}
	free(encmsg);
	otrl_tlv_free(tlv);

	return err;
    }

    /* We weren't in an encrypted session. */
    return gcry_error(GPG_ERR_INV_VALUE);
}

/* If you do _not_ define a timer_control callback function, set a timer
 * to go off every definterval =
 * otrl_message_poll_get_default_interval(userstate) seconds, and call
 * otrl_message_poll every time the timer goes off. */
unsigned int otrl_message_poll_get_default_interval(OtrlUserState us)
{
    return POLL_DEFAULT_INTERVAL;
}

/* Call this function every so often, either as directed by the
 * timer_control callback, or every definterval =
 * otrl_message_poll_get_default_interval(userstate) seconds if you have
 * no timer_control callback.  This function must be called from the
 * main libotr thread.*/
void otrl_message_poll(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata)
{
    /* Wipe private keys last sent before this time */
    time_t expire_before = time(NULL) - MAX_AKE_WAIT_TIME;

    ConnContext *contextp;

    /* Is there a context still waiting for a DHKEY message, even after
     * we wipe the stale ones? */
    int still_waiting = 0;

    if (us == NULL) return;

    for (contextp = us->context_root; contextp; contextp = contextp->next) {
	/* If this is a master context, and it's still waiting for a
	 * v3 DHKEY message, see if it's waited long enough. */
	if (contextp->m_context == contextp &&
		contextp->auth.authstate == OTRL_AUTHSTATE_AWAITING_DHKEY &&
		contextp->auth.protocol_version == 3 &&
		contextp->auth.commit_sent_time > 0) {
	    if (contextp->auth.commit_sent_time < expire_before) {
		otrl_auth_clear(&contextp->auth);
	    } else {
		/* Not yet expired */
		still_waiting = 1;
	    }
	}
    }

    /* If there's nothing more to wait for, stop the timer, if possible. */
    if (still_waiting == 0 && ops && ops->timer_control) {
	ops->timer_control(opdata, 0);
	us->timer_running = 0;
    }
}
