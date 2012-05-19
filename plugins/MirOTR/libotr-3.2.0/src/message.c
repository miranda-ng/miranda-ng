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

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "privkey.h"
#include "proto.h"
#include "auth.h"
#include "message.h"
#include "sm.h"

/* The API version */
extern unsigned int otrl_api_version;

/* How long after sending a packet should we wait to send a heartbeat? */
#define HEARTBEAT_INTERVAL 60

/* How old are messages allowed to be in order to be candidates for
 * resending in response to a rekey? */
#define RESEND_INTERVAL 60

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
 * If this routine returns non-zero, then the library tried to encrypt
 * the message, but for some reason failed.  DO NOT send the message in
 * the clear in that case.
 * 
 * If *messagep gets set by the call to something non-NULL, then you
 * should replace your message with the contents of *messagep, and
 * send that instead.  Call otrl_message_free(*messagep) when you're
 * done with it. */
gcry_error_t otrl_message_sending(OtrlUserState us,
	const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *recipient, const char *message, OtrlTLV *tlvs,
	char **messagep,
	void (*add_appdata)(void *data, ConnContext *context),
	void *data)
{
	struct context * context;
	char * msgtosend;
	gcry_error_t err;
	OtrlPolicy policy = OTRL_POLICY_DEFAULT;
	int context_added = 0;

	*messagep = NULL;

	if (!accountname || !protocol || !recipient || !message || !messagep)
		return gcry_error(GPG_ERR_NO_ERROR);

	/* See if we have a fingerprint for this user */
	context = otrl_context_find(us, recipient, accountname, protocol,
		1, &context_added, add_appdata, data);

	/* Update the context list if we added one */
	if (context_added && ops->update_context_list) {
	ops->update_context_list(opdata);
	}

	/* Check the policy */
	if (ops->policy) {
	policy = ops->policy(opdata, context);
	}

	/* Should we go on at all? */
	if ((policy & OTRL_POLICY_VERSION_MASK) == 0) {
		return gcry_error(GPG_ERR_NO_ERROR);
	}

	/* If this is an OTR Query message, don't encrypt it. */
	if (otrl_proto_message_type(message) == OTRL_MSGTYPE_QUERY) {
	/* Replace the "?OTR?" with a custom message */
	char *bettermsg = otrl_proto_default_query_msg(accountname, policy);
	if (bettermsg) {
		*messagep = bettermsg;
	}
	return gcry_error(GPG_ERR_NO_ERROR);
	}

	/* What is the current message disposition? */
	switch(context->msgstate) {
	case OTRL_MSGSTATE_PLAINTEXT:
		if ((policy & OTRL_POLICY_REQUIRE_ENCRYPTION)) {
		/* We're trying to send an unencrypted message with a policy
		 * that disallows that.  Don't do that, but try to start
		 * up OTR instead. */
		if ((!(ops->display_otr_message) ||
			ops->display_otr_message(opdata, accountname,
				protocol, recipient, "Attempting to start a "
				"private conversation...")) && ops->notify) {
			const char *format = "You attempted to send an "
			"unencrypted message to %s";
			char *primary = malloc(strlen(format) +
				strlen(recipient) - 1);
			if (primary) {
			sprintf(primary, format, recipient);
			ops->notify(opdata, OTRL_NOTIFY_WARNING, accountname,
				protocol, recipient, "OTR Policy Violation",
				primary,
				"Unencrypted messages to this recipient are "
				"not allowed.  Attempting to start a private "
				"conversation.\n\nYour message will be "
				"retransmitted when the private conversation "
				"starts.");
			free(primary);
			}
		}
		context->lastmessage = gcry_malloc_secure(strlen(message) + 1);
		if (context->lastmessage) {
			char *bettermsg = otrl_proto_default_query_msg(accountname,
				policy);
			strcpy(context->lastmessage, message);
			context->lastsent = time(NULL);
			context->may_retransmit = 2;
			if (bettermsg) {
			*messagep = bettermsg;
			} else {
			return gcry_error(GPG_ERR_ENOMEM);
			}
		}
		} else {
		if ((policy & OTRL_POLICY_SEND_WHITESPACE_TAG) &&
			context->otr_offer != OFFER_REJECTED) {
			/* See if this user can speak OTR.  Append the
			 * OTR_MESSAGE_TAG to the plaintext message, and see
			 * if he responds. */
			size_t msglen = strlen(message);
			size_t basetaglen = strlen(OTRL_MESSAGE_TAG_BASE);
			size_t v1taglen = (policy & OTRL_POLICY_ALLOW_V1) ?
			strlen(OTRL_MESSAGE_TAG_V1) : 0;
			size_t v2taglen = (policy & OTRL_POLICY_ALLOW_V2) ?
			strlen(OTRL_MESSAGE_TAG_V2) : 0;
			char *taggedmsg = malloc(msglen + basetaglen + v1taglen
				+v2taglen + 1);
			if (taggedmsg) {
			strcpy(taggedmsg, message);
			strcpy(taggedmsg + msglen, OTRL_MESSAGE_TAG_BASE);
			if (v1taglen) {
				strcpy(taggedmsg + msglen + basetaglen,
					OTRL_MESSAGE_TAG_V1);
			}
			if (v2taglen) {
				strcpy(taggedmsg + msglen + basetaglen + v1taglen,
					OTRL_MESSAGE_TAG_V2);
			}
			*messagep = taggedmsg;
			if (context) {
				context->otr_offer = OFFER_SENT;
			}
			}
		}
		}
		break;
	case OTRL_MSGSTATE_ENCRYPTED:
		/* Create the new, encrypted message */
		err = otrl_proto_create_data(&msgtosend, context, message, tlvs,
			0);
		if (!err) {
		context->lastsent = time(NULL);
		*messagep = msgtosend;
		} else {
		/* Uh, oh.  Whatever we do, *don't* send the message in the
		 * clear. */
		*messagep = _strdup("?OTR Error: Error occurred encrypting "
			"message");
		if ((!(ops->display_otr_message) ||
			ops->display_otr_message(opdata, accountname,
				protocol, recipient, "An error occurred when "
				"encrypting your message.  The message was not "
				"sent.")) && ops->notify) {
			ops->notify(opdata, OTRL_NOTIFY_ERROR, 
				accountname, protocol, recipient,
				"Error encrypting message",
				"An error occurred when encrypting your message",
				"The message was not sent.");
		}
		if (!(*messagep)) {
			return gcry_error(GPG_ERR_ENOMEM);
		}
		}
		break;
	case OTRL_MSGSTATE_FINISHED:
		*messagep = _strdup("");
		if ((!(ops->display_otr_message) ||
			ops->display_otr_message(opdata, accountname,
			protocol, recipient, "Your message was not sent.  "
			"Either end your private conversation, or restart "
			"it.")) && ops->notify) {
		const char *fmt = "%s has already closed his/her private "
			"connection to you";
		char *primary = malloc(strlen(fmt) + strlen(recipient) - 1);
		if (primary) {
			sprintf(primary, fmt, recipient);
			ops->notify(opdata, OTRL_NOTIFY_ERROR, 
				accountname, protocol, recipient,
				"Private connection closed", primary,
				"Your message was not sent.  Either close your "
				"private connection to him, or refresh it.");
		}
		}
		if (!(*messagep)) {
		return gcry_error(GPG_ERR_ENOMEM);
		}
		break;
	}

	return gcry_error(GPG_ERR_NO_ERROR);
}

/* If err == 0, send the last auth message for the given context to the
 * appropriate user.  Otherwise, display an appripriate error dialog.
 * Return the value of err that was passed. */
static gcry_error_t send_or_error_auth(const OtrlMessageAppOps *ops,
	void *opdata, gcry_error_t err, ConnContext *context)
{
	if (!err) {
	const char *msg = context->auth.lastauthmsg;
	if (msg && *msg) {
		otrl_message_fragment_and_send(ops, opdata, context, msg, OTRL_FRAGMENT_SEND_ALL, NULL);
		/*if (ops->inject_message) {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, msg);
		}*/
	}
	} else {
	const char *buf_format = "Error setting up private conversation: %s";
	const char *strerr;
	char *buf;
	
	switch(gcry_err_code(err)) {
		case GPG_ERR_INV_VALUE:
		strerr = "Malformed message received";
		break;
		default:
		strerr = gcry_strerror(err);
		break;
	}
	buf = malloc(strlen(buf_format) + strlen(strerr) - 1);
	if (buf) {
		sprintf(buf, buf_format, strerr);
	}
	if ((!(ops->display_otr_message) ||
		ops->display_otr_message(opdata, context->accountname,
			context->protocol, context->username, buf))
		&& ops->notify) {
		ops->notify(opdata, OTRL_NOTIFY_ERROR, context->accountname,
			context->protocol, context->username, "OTR error",
			buf, NULL);
	}
	free(buf);
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
	if ((!(edata->ops->display_otr_message) ||
		edata->ops->display_otr_message(edata->opdata,
			edata->context->accountname, edata->context->protocol,
			edata->context->username,
			"We are receiving our own OTR messages.  "
			"You are either trying to talk to yourself, "
			"or someone is reflecting your messages back "
			"at you.")) && edata->ops->notify) {
		edata->ops->notify(edata->opdata, OTRL_NOTIFY_ERROR,
		edata->context->accountname, edata->context->protocol,
		edata->context->username, "OTR Error",
		"We are receiving our own OTR messages.",
		"You are either trying to talk to yourself, "
		"or someone is reflecting your messages back "
		"at you.");
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
		edata->context->our_keyid - 1 == edata->context->auth.our_keyid &&
		!gcry_mpi_cmp(edata->context->our_old_dh_key.pub,
		edata->context->auth.our_dh.pub) &&
		((edata->context->their_keyid > 0 &&
		  edata->context->their_keyid ==
			edata->context->auth.their_keyid &&
		  !gcry_mpi_cmp(edata->context->their_y,
		  edata->context->auth.their_pub)) ||
		(edata->context->their_keyid > 1 &&
		 edata->context->their_keyid - 1 ==
			edata->context->auth.their_keyid &&
		 edata->context->their_old_y != NULL &&
		 !gcry_mpi_cmp(edata->context->their_old_y,
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

	edata->context->their_keyid = edata->context->auth.their_keyid;
	gcry_mpi_release(edata->context->their_y);
	gcry_mpi_release(edata->context->their_old_y);
	edata->context->their_y = gcry_mpi_copy(edata->context->auth.their_pub);
	edata->context->their_old_y = NULL;

	if (edata->context->our_keyid - 1 != edata->context->auth.our_keyid ||
	gcry_mpi_cmp(edata->context->our_old_dh_key.pub,
		edata->context->auth.our_dh.pub)) {
	otrl_dh_keypair_free(&(edata->context->our_dh_key));
	otrl_dh_keypair_free(&(edata->context->our_old_dh_key));
	otrl_dh_keypair_copy(&(edata->context->our_old_dh_key),
		&(edata->context->auth.our_dh));
	otrl_dh_gen_keypair(edata->context->our_old_dh_key.groupid,
		&(edata->context->our_dh_key));
	edata->context->our_keyid = edata->context->auth.our_keyid + 1;
	}

	/* Create the session keys from the DH keys */
	otrl_dh_session_free(&(edata->context->sesskeys[0][0]));
	err = otrl_dh_session(&(edata->context->sesskeys[0][0]),
	&(edata->context->our_dh_key), edata->context->their_y);
	if (err) return err;
	otrl_dh_session_free(&(edata->context->sesskeys[1][0]));
	err = otrl_dh_session(&(edata->context->sesskeys[1][0]),
	&(edata->context->our_old_dh_key), edata->context->their_y);
	if (err) return err;

	edata->context->generation++;
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
	if (edata->context->lastmessage != NULL &&
		edata->context->may_retransmit &&
		edata->context->lastsent >= (now - RESEND_INTERVAL)) {
	char *resendmsg;
	int resending = (edata->context->may_retransmit == 1);

	/* Re-encrypt the message with the new keys */
	err = otrl_proto_create_data(&resendmsg,
		edata->context, edata->context->lastmessage, NULL, 0);
	if (!err) {
		const char *format = "<b>The last message "
		"to %s was resent.</b>";
		char *buf;

		/* Resend the message */
		otrl_message_fragment_and_send(edata->ops, edata->opdata, edata->context, resendmsg, OTRL_FRAGMENT_SEND_ALL, NULL);
		free(resendmsg);
		edata->context->lastsent = now;

		if (!resending) {
		/* We're actually just sending it
		 * for the first time. */
		edata->ignore_message = 1;
		} else {
		/* Let the user know we resent it */
		buf = malloc(strlen(format) +
			strlen(edata->context->username) - 1);
		if (buf) {
			sprintf(buf, format, edata->context->username);
			if (edata->ops->display_otr_message) {
			if (!edata->ops->display_otr_message(
					edata->opdata, edata->context->accountname,
					edata->context->protocol,
					edata->context->username, buf)) {
				edata->ignore_message = 1;
			}
			}
			if (edata->ignore_message != 1) {
			*(edata->messagep) = buf;
			edata->ignore_message = 0;
			} else {
			free(buf);
			}
		}
		}
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
			OTRL_MSGFLAGS_IGNORE_UNREADABLE);
	if (!err) {
		/*  Send it, and set the next expected message to the
	 *  logical response */
		err = otrl_message_fragment_and_send(ops, opdata, context,
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
		OTRL_MSGFLAGS_IGNORE_UNREADABLE);
	if (!err) {
	/* Send the abort signal so our buddy knows we've stopped */
	err = otrl_message_fragment_and_send(ops, opdata, context,
		sendsmp, OTRL_FRAGMENT_SEND_ALL, NULL);
	}
	free(sendsmp);
	otrl_tlv_free(sendtlv);
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
	OtrlTLV **tlvsp,
	void (*add_appdata)(void *data, ConnContext *context),
	void *data)
{
	ConnContext *context;
	OtrlMessageType msgtype;
	int context_added = 0;
	OtrlMessageState msgstate;
	OtrlPolicy policy = OTRL_POLICY_DEFAULT;
	int fragment_assembled = 0;
	char *unfragmessage = NULL;
	EncrData edata;

	if (!accountname || !protocol || !sender || !message || !newmessagep)
		return 0;

	*newmessagep = NULL;
	if (tlvsp) *tlvsp = NULL;

	/* Find our context and state with this correspondent */
	context = otrl_context_find(us, sender, accountname,
		protocol, 1, &context_added, add_appdata, data);

	/* Update the context list if we added one */
	if (context_added && ops->update_context_list) {
	ops->update_context_list(opdata);
	}

	/* Check the policy */
	if (ops->policy) {
	policy = ops->policy(opdata, context);
	}

	/* Should we go on at all? */
	if ((policy & OTRL_POLICY_VERSION_MASK) == 0) {
		return 0;
	}

	/* See if we have a fragment */
	switch(otrl_proto_fragment_accumulate(&unfragmessage, context, message)) {
	case OTRL_FRAGMENT_UNFRAGMENTED:
		/* Do nothing */
		break;
	case OTRL_FRAGMENT_INCOMPLETE:
		/* We've accumulated this fragment, but we don't have a
		 * complete message yet */
		return 1;
	case OTRL_FRAGMENT_COMPLETE:
		/* We've got a new complete message, in unfragmessage. */
		fragment_assembled = 1;
		message = unfragmessage;
		break;
	}

	/* What type of message is it?  Note that this just checks the
	 * header; it's not necessarily a _valid_ message of this type. */
	msgtype = otrl_proto_message_type(message);
	msgstate = context->msgstate;

	/* See if they responded to our OTR offer */
	if ((policy & OTRL_POLICY_SEND_WHITESPACE_TAG)) {
	if (msgtype != OTRL_MSGTYPE_NOTOTR) {
		context->otr_offer = OFFER_ACCEPTED;
	} else if (context->otr_offer == OFFER_SENT) {
		context->otr_offer = OFFER_REJECTED;
	}
	}

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
	gcry_error_t err;
	int haveauthmsg;
	case OTRL_MSGTYPE_QUERY:
		/* See if we should use an existing DH keypair, or generate
		 * a fresh one. */
		if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		our_dh = &(context->our_old_dh_key);
		our_keyid = context->our_keyid - 1;
		} else {
		our_dh = NULL;
		our_keyid = 0;
		}

		/* Find the best version of OTR that we both speak */
		switch(otrl_proto_query_bestversion(message, policy)) {
		case 2:
			err = otrl_auth_start_v2(&(context->auth));
			send_or_error_auth(ops, opdata, err, context);
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
			send_or_error_auth(ops, opdata, err, context);
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
		if ((policy & OTRL_POLICY_ALLOW_V2)) {
		err = otrl_auth_handle_commit(&(context->auth), message);
		send_or_error_auth(ops, opdata, err, context);
		}

		if (edata.ignore_message == -1) edata.ignore_message = 1;
		break;

	case OTRL_MSGTYPE_DH_KEY:
		if ((policy & OTRL_POLICY_ALLOW_V2)) {
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
			err = otrl_auth_handle_key(&(context->auth), message,
				&haveauthmsg, privkey);
			if (err || haveauthmsg) {
			send_or_error_auth(ops, opdata, err, context);
			}
		}
		}

		if (edata.ignore_message == -1) edata.ignore_message = 1;
		break;

	case OTRL_MSGTYPE_REVEALSIG:
		if ((policy & OTRL_POLICY_ALLOW_V2)) {
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
				message, &haveauthmsg, privkey, go_encrypted,
				&edata);
			if (err || haveauthmsg) {
			send_or_error_auth(ops, opdata, err, context);
			maybe_resend(&edata);
			}
		}
		}

		if (edata.ignore_message == -1) edata.ignore_message = 1;
		break;

	case OTRL_MSGTYPE_SIGNATURE:
		if ((policy & OTRL_POLICY_ALLOW_V2)) {
		err = otrl_auth_handle_signature(&(context->auth),
			message, &haveauthmsg, go_encrypted, &edata);
		if (err || haveauthmsg) {
			send_or_error_auth(ops, opdata, err, context);
			maybe_resend(&edata);
		}
		}
		
		if (edata.ignore_message == -1) edata.ignore_message = 1;
		break;

	case OTRL_MSGTYPE_V1_KEYEXCH:
		if ((policy & OTRL_POLICY_ALLOW_V1)) {
		/* See if we should use an existing DH keypair, or generate
		 * a fresh one. */
		if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
			our_dh = &(context->our_old_dh_key);
			our_keyid = context->our_keyid - 1;
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
			privkey = otrl_privkey_find(us,
				context->accountname, context->protocol);
			}
		}
		if (privkey) {
			err = otrl_auth_handle_v1_key_exchange(&(context->auth),
				message, &haveauthmsg, privkey, our_dh, our_keyid,
				go_encrypted, &edata);
			if (err || haveauthmsg) {
			send_or_error_auth(ops, opdata, err, context);
			maybe_resend(&edata);
			}
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
		const char *format;
		const char *displayaccountname;
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

			/* Don't use g_strdup_printf here, because someone
			 * (not us) is going to free() the *newmessagep pointer,
			 * not g_free() it. */
			format = "<b>The encrypted message received from %s is "
			"unreadable, as you are not currently communicating "
			"privately.</b>";
			buf = malloc(strlen(format) + strlen(context->username)
				- 1);  /* Remove "%s", add username + '\0' */
			if (buf) {
			sprintf(buf, format, context->username);
			if (ops->display_otr_message) {
				if (!ops->display_otr_message(opdata, accountname,
					protocol, sender, buf)) {
				edata.ignore_message = 1;
				}
			}
			if (edata.ignore_message != 1) {
				*newmessagep = buf;
				edata.ignore_message = 0;
			} else {
				free(buf);
			}
			}
			format = "?OTR Error: You sent encrypted "
				"data to %s, who wasn't expecting it.";
			if (otrl_api_version >= 0x00030100 &&
				ops->account_name) {
			displayaccountname = ops->account_name(opdata,
				context->accountname, protocol);
			} else {
			displayaccountname = NULL;
			}
			buf = malloc(strlen(format) + strlen(displayaccountname ?
				displayaccountname : context->accountname)
				- 1);
			if (buf) {
			sprintf(buf, format, displayaccountname ?
				displayaccountname : context->accountname);
			if (ops->inject_message) {
				ops->inject_message(opdata, accountname, protocol,
					sender, buf);
			}
			free(buf);
			}
			if (displayaccountname && otrl_api_version >= 0x00030100 &&
				ops->account_name_free) {
			ops->account_name_free(opdata, displayaccountname);
			}

			break;

		case OTRL_MSGSTATE_ENCRYPTED:
			err = otrl_proto_accept_data(&plaintext, &tlvs, context,
				message, &flags);
			if (err) {
			int is_conflict =
				(gpg_err_code(err) == GPG_ERR_CONFLICT);
			if ((flags & OTRL_MSGFLAGS_IGNORE_UNREADABLE)) {
				edata.ignore_message = 1;
				break;
			}
			format = is_conflict ? "We received an unreadable "
				"encrypted message from %s." :
				"We received a malformed data message from %s.";
			buf = malloc(strlen(format) + strlen(sender) - 1);
			if (buf) {
				sprintf(buf, format, sender);
				if ((!(ops->display_otr_message) ||
					ops->display_otr_message(opdata,
					accountname, protocol, sender,
					buf)) && ops->notify) {
				ops->notify(opdata, OTRL_NOTIFY_ERROR,
					accountname, protocol, sender,
					"OTR Error", buf, NULL);
				}
				free(buf);
			}
			if (ops->inject_message) {
				ops->inject_message(opdata, accountname, protocol,
					sender, is_conflict ? "?OTR Error: "
						"You transmitted an unreadable "
						"encrypted message." :
						"?OTR Error: You transmitted "
						"a malformed data message");
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

					/* If TLVs contain SMP data, process it */
			nextMsg = context->smstate->nextExpected;
					tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q);
			if (tlv && nextMsg == OTRL_SMP_EXPECT1) {
			/* We can only do the verification half now.
			 * We must wait for the secret to be entered
			 * to continue. */
			char *question = (char *)tlv->data;
			char *qend = memchr(question, '\0', tlv->len - 1);
			size_t qlen = qend ? (qend - question + 1) : tlv->len;
			otrl_sm_step2a(context->smstate, tlv->data + qlen,
				tlv->len - qlen, 1);
					}
					tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1);
			if (tlv && nextMsg == OTRL_SMP_EXPECT1) {
			/* We can only do the verification half now.
			 * We must wait for the secret to be entered
			 * to continue. */
			otrl_sm_step2a(context->smstate, tlv->data, tlv->len,
				0);
					}
					tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP2);
			if (tlv && nextMsg == OTRL_SMP_EXPECT2) {
			unsigned char* nextmsg;
			int nextmsglen;
			OtrlTLV *sendtlv;
			char *sendsmp;
			otrl_sm_step3(context->smstate, tlv->data, tlv->len,
					&nextmsg, &nextmsglen);
			
			if (context->smstate->sm_prog_state !=
				OTRL_SMP_PROG_CHEATED) {
				/* Send msg with next smp msg content */
				sendtlv = otrl_tlv_new(OTRL_TLV_SMP3, nextmsglen,
					nextmsg);
				err = otrl_proto_create_data(&sendsmp,
					context, "", sendtlv,
					OTRL_MSGFLAGS_IGNORE_UNREADABLE);
				if (!err) {
				err = otrl_message_fragment_and_send(ops,
					opdata, context, sendsmp,
					OTRL_FRAGMENT_SEND_ALL, NULL);
				}
				free(sendsmp);
				otrl_tlv_free(sendtlv);
			}
			free(nextmsg);
					}
					tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP3);
			if (tlv && nextMsg == OTRL_SMP_EXPECT3) {
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
				sendtlv = otrl_tlv_new(OTRL_TLV_SMP4, nextmsglen,
					nextmsg);
				err = otrl_proto_create_data(&sendsmp,
					context, "", sendtlv,
					OTRL_MSGFLAGS_IGNORE_UNREADABLE);
				if (!err) {
				err = otrl_message_fragment_and_send(ops,
					opdata, context, sendsmp,
					OTRL_FRAGMENT_SEND_ALL, NULL);
				}
				free(sendsmp);
				otrl_tlv_free(sendtlv);
			}
			free(nextmsg);
					}
					tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP4);
			if (tlv && nextMsg == OTRL_SMP_EXPECT4) {
			err = otrl_sm_step5(context->smstate, tlv->data,
				tlv->len);
			/* Set trust level based on result */
			set_smp_trust(ops, opdata, context,
				(err == gcry_error(GPG_ERR_NO_ERROR)));
					}
					tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP_ABORT);
			if (tlv) {
			context->smstate->nextExpected = OTRL_SMP_EXPECT1;
			}
			if (plaintext[0] == '\0') {
			/* If it's a heartbeat (an empty message), don't
			 * display it to the user, but log a debug message. */
			format = "Heartbeat received from %s.\n";
			buf = malloc(strlen(format) + strlen(sender) - 1);
			if (buf) {
				sprintf(buf, format, sender);
				if (ops->log_message) {
				ops->log_message(opdata, buf);
				}
				free(buf);
			}
			edata.ignore_message = 1;
			} else if (edata.ignore_message == 0 &&
				context->their_keyid > 0) {
			/* If it's *not* a heartbeat, and we haven't
			 * sent anything in a while, also send a
			 * heartbeat. */
			time_t now = time(NULL);
			if (context->lastsent < (now - HEARTBEAT_INTERVAL)) {
				char *heartbeat;

				/* Create the heartbeat message */
				err = otrl_proto_create_data(&heartbeat,
					context, "", NULL,
					OTRL_MSGFLAGS_IGNORE_UNREADABLE);
				if (!err) {
				/* Send it, and log a debug message */
				if (ops->inject_message) {
					ops->inject_message(opdata, accountname,
						protocol, sender, heartbeat);
				}
				free(heartbeat);

				context->lastsent = now;

				/* Log a debug message */
				format = "Heartbeat sent to %s.\n";
				buf = malloc(strlen(format) + strlen(sender)
					- 1);
				if (buf) {
					sprintf(buf, format, sender);
					if (ops->log_message) {
					ops->log_message(opdata, buf);
					}
					free(buf);
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
			*newmessagep = plaintext;
			edata.ignore_message = 0;
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
		context->may_retransmit = 1;
		}

		/* In any event, display the error message, with the
		 * display_otr_message callback, if possible */
		if (ops->display_otr_message) {
		const char *otrerror = strstr(message, "?OTR Error:");
		if (otrerror) {
			/* Skip the leading '?' */
			++otrerror;
		} else {
			otrerror = message;
		}
		if (!ops->display_otr_message(opdata, accountname, protocol,
				sender, otrerror)) {
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
		char *strippedmsg = _strdup(message);

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
			case 2:
			err = otrl_auth_start_v2(&(context->auth));
			send_or_error_auth(ops, opdata, err, context);
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
				send_or_error_auth(ops, opdata, err, context);
			}
			break;
			default:
			/* Don't start the AKE */
			break;
		}
		}

		/* FALLTHROUGH */
	case OTRL_MSGTYPE_NOTOTR:
		if (context->msgstate != OTRL_MSGSTATE_PLAINTEXT ||
			(policy & OTRL_POLICY_REQUIRE_ENCRYPTION)) {
		/* Not fine.  Let the user know. */

		/* Don't use g_strdup_printf here, because someone
		 * (not us) is going to free() the *message pointer,
		 * not g_free() it. */
		const char *plainmsg = (*newmessagep) ? *newmessagep : message;
		const char *format = "The following message received "
			"from %s was not encrypted: [</b>%s<b>]";
		char *buf = malloc(strlen(format) + strlen(context->username)
			+ strlen(plainmsg) - 3);
			/* Remove "%s%s", add username + message + '\0' */
		if (buf) {
			sprintf(buf, format, context->username, plainmsg);
			if (ops->display_otr_message) {
			if (!ops->display_otr_message(opdata, accountname,
					protocol, sender, buf)) {
				free(*newmessagep);
				*newmessagep = NULL;
				edata.ignore_message = 1;
			}
			}
			if (edata.ignore_message != 1) {
			free(*newmessagep);
			*newmessagep = buf;
			edata.ignore_message = 0;
			} else {
			free(buf);
			}
		}
		}
		break;

	case OTRL_MSGTYPE_UNKNOWN:
		/* We received an OTR message we didn't recognize.  Ignore
		 * it, but make a log entry. */
		if (ops->log_message) {
		const char *format = "Unrecognized OTR message received "
			"from %s.\n";
		char *buf = malloc(strlen(format) + strlen(sender) - 1);
		if (buf) {
			sprintf(buf, format, sender);
			ops->log_message(opdata, buf);
			free(buf);
		}
		}
		if (edata.ignore_message == -1) edata.ignore_message = 1;
		break;
	}

	/* If we reassembled a fragmented message, we need to free the
	 * allocated memory now. */
	if (fragment_assembled) {
	free(unfragmessage);
	}

	if (edata.ignore_message == -1) edata.ignore_message = 0;
	return edata.ignore_message;
}

/* Send a message to the network, fragmenting first if necessary.
 * All messages to be sent to the network should go through this
 * method immediately before they are sent, ie after encryption. */
gcry_error_t otrl_message_fragment_and_send(const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const char *message,
	OtrlFragmentPolicy fragPolicy, char **returnFragment)
{
	int mms = 0;
	if (message && ops->inject_message) {
		int msglen;

		if (otrl_api_version >= 0x030100 && ops->max_message_size) {
		mms = ops->max_message_size(opdata, context);
		}
		msglen = strlen(message);

	/* Don't incur overhead of fragmentation unless necessary */
		if(mms != 0 && msglen > mms) {
		char **fragments;
		gcry_error_t err;
		int i;
		int fragment_count = ((msglen - 1) / (mms -19)) + 1;
		/* like ceil(msglen/(mms - 19)) */

		err = otrl_proto_fragment_create(mms, fragment_count, &fragments,
			message);
		if (err) {
		return err;
		}

		/* Determine which fragments to send and which to return
		 * based on given Fragment Policy.  If the first fragment
		 * should be returned instead of sent, store it. */
		if (fragPolicy == OTRL_FRAGMENT_SEND_ALL_BUT_FIRST) {
		*returnFragment = _strdup(fragments[0]);
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
		*returnFragment = _strdup(fragments[fragment_count-1]);
		} else {
		ops->inject_message(opdata, context->accountname,
			context->protocol, context->username, fragments[fragment_count-1]);
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
		int l = strlen(message) + 1;
		*returnFragment = malloc(sizeof(char)*l);
		strcpy(*returnFragment, message);
		}
	}
	}
	return gcry_error(GPG_ERR_NO_ERROR);
}

/* Put a connection into the PLAINTEXT state, first sending the
 * other side a notice that we're doing so if we're currently ENCRYPTED,
 * and we think he's logged in. */
void otrl_message_disconnect(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *username)
{
	ConnContext *context = otrl_context_find(us, username, accountname,
		protocol, 0, NULL, NULL, NULL);

	if (!context) return;

	if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED &&
		context->their_keyid > 0 &&
		ops->is_logged_in &&
		ops->is_logged_in(opdata, accountname, protocol, username) == 1) {
	if (ops->inject_message) {
		char *encmsg = NULL;
		gcry_error_t err;
		OtrlTLV *tlv = otrl_tlv_new(OTRL_TLV_DISCONNECTED, 0, NULL);

		err = otrl_proto_create_data(&encmsg, context, "", tlv,
			OTRL_MSGFLAGS_IGNORE_UNREADABLE);
		if (!err) {
		ops->inject_message(opdata, accountname, protocol,
			username, encmsg);
		}
		free(encmsg);
	}
	}

	otrl_context_force_plaintext(context);
	if (ops->update_context_list) {
	ops->update_context_list(opdata);
	}
}
