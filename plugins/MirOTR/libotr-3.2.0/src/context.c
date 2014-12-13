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
#include <stdlib.h>
#include <assert.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "context.h"

/* Create a new connection context. */
static ConnContext * new_context(const char * user, const char * accountname,
	const char * protocol)
{
	ConnContext * context;
	OtrlSMState *smstate;
	context = malloc(sizeof(*context));
	assert(context != NULL);
	context->username = _strdup(user);
	context->accountname = _strdup(accountname);
	context->protocol = _strdup(protocol);
	context->fragment = NULL;
	context->fragment_len = 0;
	context->fragment_n = 0;
	context->fragment_k = 0;
	context->msgstate = OTRL_MSGSTATE_PLAINTEXT;
	otrl_auth_new(&(context->auth));

	smstate = malloc(sizeof(OtrlSMState));
	assert(smstate != NULL);
	otrl_sm_state_new(smstate);
	context->smstate = smstate;

	context->fingerprint_root.fingerprint = NULL;
	context->fingerprint_root.context = context;
	context->fingerprint_root.next = NULL;
	context->fingerprint_root.tous = NULL;
	context->active_fingerprint = NULL;
	context->their_keyid = 0;
	context->their_y = NULL;
	context->their_old_y = NULL;
	context->our_keyid = 0;
	context->our_dh_key.groupid = 0;
	context->our_dh_key.priv = NULL;
	context->our_dh_key.pub = NULL;
	context->our_old_dh_key.groupid = 0;
	context->our_old_dh_key.priv = NULL;
	context->our_old_dh_key.pub = NULL;
	otrl_dh_session_blank(&(context->sesskeys[0][0]));
	otrl_dh_session_blank(&(context->sesskeys[0][1]));
	otrl_dh_session_blank(&(context->sesskeys[1][0]));
	otrl_dh_session_blank(&(context->sesskeys[1][1]));
	memset(context->sessionid, 0, 20);
	context->sessionid_len = 0;
	context->protocol_version = 0;
	context->numsavedkeys = 0;
	context->preshared_secret = NULL;
	context->preshared_secret_len = 0;
	context->saved_mac_keys = NULL;
	context->generation = 0;
	context->lastsent = 0;
	context->lastmessage = NULL;
	context->may_retransmit = 0;
	context->otr_offer = OFFER_NOT;
	context->app_data = NULL;
	context->app_data_free = NULL;
	context->next = NULL;
	return context;
}

ConnContext * otrl_context_new(const char * user, const char * accountname,
	const char * protocol)
{
	return new_context(user, accountname, protocol);
}

/* Look up a connection context by name/account/protocol from the given
 * OtrlUserState.  If add_if_missing is true, allocate and return a new
 * context if one does not currently exist.  In that event, call
 * add_app_data(data, context) so that app_data and app_data_free can be
 * filled in by the application, and set *addedp to 1. */
ConnContext * otrl_context_find(OtrlUserState us, const char *user,
	const char *accountname, const char *protocol, int add_if_missing,
	int *addedp,
	void (*add_app_data)(void *data, ConnContext *context), void *data)
{
	ConnContext ** curp;
	int usercmp = 1, acctcmp = 1, protocmp = 1;
	if (addedp) *addedp = 0;
	if (!user || !accountname || !protocol) return NULL;
	for (curp = &(us->context_root); *curp; curp = &((*curp)->next)) {
		if ((usercmp = strcmp((*curp)->username, user)) > 0 ||
		(usercmp == 0 &&
		  (acctcmp = strcmp((*curp)->accountname, accountname)) > 0) ||
		(usercmp == 0 && acctcmp == 0 &&
		  (protocmp = strcmp((*curp)->protocol, protocol)) >= 0))
		/* We're at the right place in the list.  We've either found
		 * it, or gone too far. */
		break;
	}
	if (usercmp == 0 && acctcmp == 0 && protocmp == 0) {
	/* Found it! */
	return *curp;
	}
	if (add_if_missing) {
	ConnContext *newctx;
	if (addedp) *addedp = 1;
	newctx = new_context(user, accountname, protocol);
	newctx->next = *curp;
	if (*curp) {
		(*curp)->tous = &(newctx->next);
	}
	*curp = newctx;
	newctx->tous = curp;
	if (add_app_data) {
		add_app_data(data, *curp);
	}
	return *curp;
	}
	return NULL;
}

/* Find a fingerprint in a given context, perhaps adding it if not
 * present. */
Fingerprint *otrl_context_find_fingerprint(ConnContext *context,
	unsigned char fingerprint[20], int add_if_missing, int *addedp)
{
	Fingerprint *f = context->fingerprint_root.next;
	if (addedp) *addedp = 0;
	while(f) {
	if (!memcmp(f->fingerprint, fingerprint, 20)) return f;
	f = f->next;
	}
	/* Didn't find it. */
	if (add_if_missing) {
	if (addedp) *addedp = 1;
	f = malloc(sizeof(*f));
	assert(f != NULL);
	f->fingerprint = malloc(20);
	assert(f->fingerprint != NULL);
	memcpy(f->fingerprint, fingerprint, 20);
	f->context = context;
	f->trust = NULL;
	f->next = context->fingerprint_root.next;
	if (f->next) {
		f->next->tous = &(f->next);
	}
	context->fingerprint_root.next = f;
	f->tous = &(context->fingerprint_root.next);
	return f;
	}
	return NULL;
}

/* Set the trust level for a given fingerprint */
void otrl_context_set_trust(Fingerprint *fprint, const char *trust)
{
	if (fprint == NULL) return;

	free(fprint->trust);
	fprint->trust = trust ? _strdup(trust) : NULL;
}

/* Set the preshared secret for a given fingerprint.  Note that this
 * currently only stores the secret in the ConnContext structure, but
 * doesn't yet do anything with it. */
void otrl_context_set_preshared_secret(ConnContext *context,
	const unsigned char *secret, size_t secret_len)
{
	free(context->preshared_secret);
	context->preshared_secret = NULL;
	context->preshared_secret_len = 0;

	if (secret_len) {
	context->preshared_secret = malloc(secret_len);
	if (context->preshared_secret) {
		memcpy(context->preshared_secret, secret, secret_len);
		context->preshared_secret_len = secret_len;
	}
	}
}

/* Force a context into the OTRL_MSGSTATE_FINISHED state. */
void otrl_context_force_finished(ConnContext *context)
{
	context->msgstate = OTRL_MSGSTATE_FINISHED;
	otrl_auth_clear(&(context->auth));
	free(context->fragment);
	context->fragment = NULL;
	context->fragment_len = 0;
	context->fragment_n = 0;
	context->fragment_k = 0;
	context->active_fingerprint = NULL;
	context->their_keyid = 0;
	gcry_mpi_release(context->their_y);
	context->their_y = NULL;
	gcry_mpi_release(context->their_old_y);
	context->their_old_y = NULL;
	context->our_keyid = 0;
	otrl_dh_keypair_free(&(context->our_dh_key));
	otrl_dh_keypair_free(&(context->our_old_dh_key));
	otrl_dh_session_free(&(context->sesskeys[0][0]));
	otrl_dh_session_free(&(context->sesskeys[0][1]));
	otrl_dh_session_free(&(context->sesskeys[1][0]));
	otrl_dh_session_free(&(context->sesskeys[1][1]));
	memset(context->sessionid, 0, 20);
	context->sessionid_len = 0;
	free(context->preshared_secret);
	context->preshared_secret = NULL;
	context->preshared_secret_len = 0;
	context->protocol_version = 0;
	context->numsavedkeys = 0;
	free(context->saved_mac_keys);
	context->saved_mac_keys = NULL;
	gcry_free(context->lastmessage);
	context->lastmessage = NULL;
	context->may_retransmit = 0;
	otrl_sm_state_free(context->smstate);
}

/* Force a context into the OTRL_MSGSTATE_PLAINTEXT state. */
void otrl_context_force_plaintext(ConnContext *context)
{
	/* First clean up everything we'd need to do for the FINISHED state */
	otrl_context_force_finished(context);

	/* And just set the state properly */
	context->msgstate = OTRL_MSGSTATE_PLAINTEXT;
}

/* Forget a fingerprint (so long as it's not the active one.  If it's a
 * fingerprint_root, forget the whole context (as long as
 * and_maybe_context is set, and it's PLAINTEXT).  Also, if it's not
 * the fingerprint_root, but it's the only fingerprint, and we're
 * PLAINTEXT, forget the whole context if and_maybe_context is set. */
void otrl_context_forget_fingerprint(Fingerprint *fprint,
	int and_maybe_context)
{
	ConnContext *context = fprint->context;
	if (fprint == &(context->fingerprint_root)) {
	if (context->msgstate == OTRL_MSGSTATE_PLAINTEXT &&
		and_maybe_context) {
		otrl_context_forget(context);
	}
	} else {
	if (context->msgstate != OTRL_MSGSTATE_PLAINTEXT ||
		context->active_fingerprint != fprint) {
		free(fprint->fingerprint);
		free(fprint->trust);
		*(fprint->tous) = fprint->next;
		if (fprint->next) {
		fprint->next->tous = fprint->tous;
		}
		free(fprint);
		if (context->msgstate == OTRL_MSGSTATE_PLAINTEXT &&
			context->fingerprint_root.next == NULL &&
			and_maybe_context) {
		/* We just deleted the only fingerprint.  Forget the
		 * whole thing. */
		otrl_context_forget(context);
		}
	}
	}
}

/* Forget a whole context, so long as it's PLAINTEXT. */
void otrl_context_forget(ConnContext *context)
{
	if (context->msgstate != OTRL_MSGSTATE_PLAINTEXT) return;

	/* Just to be safe, force to plaintext.  This also frees any
	 * extraneous data lying around. */
	otrl_context_force_plaintext(context);

	/* First free all the Fingerprints */
	while(context->fingerprint_root.next) {
	otrl_context_forget_fingerprint(context->fingerprint_root.next, 0);
	}
	/* Now free all the dynamic info here */
	free(context->username);
	free(context->accountname);
	free(context->protocol);
	free(context->smstate);
	context->username = NULL;
	context->accountname = NULL;
	context->protocol = NULL;
	context->smstate = NULL;

	/* Free the application data, if it exists */
	if (context->app_data && context->app_data_free) {
	(context->app_data_free)(context->app_data);
	context->app_data = NULL;
	}

	/* Fix the list linkages */
	*(context->tous) = context->next;
	if (context->next) {
	context->next->tous = context->tous;
	}

	free(context);
}

/* Forget all the contexts in a given OtrlUserState. */
void otrl_context_forget_all(OtrlUserState us)
{
	while (us->context_root) {
	otrl_context_force_plaintext(us->context_root);
	otrl_context_forget(us->context_root);
	}
}
