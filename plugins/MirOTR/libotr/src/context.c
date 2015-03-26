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
#include <stdlib.h>
#include <assert.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "context.h"
#include "instag.h"

#if OTRL_DEBUGGING
#include <stdio.h>

void otrl_auth_dump(FILE *f, const OtrlAuthInfo *auth);
void otrl_sm_dump(FILE *f, const OtrlSMState *sm);

/* Dump the contents of a context to the FILE *f. */
void otrl_context_dump(FILE *f, const ConnContext *context)
{
    const Fingerprint *fing;

    fprintf(f, "Context %p:\n\n", context);

    fprintf(f, "  Username: %s\n", context->username);
    fprintf(f, "  Accountname: %s\n", context->accountname);
    fprintf(f, "  Protocol: %s\n\n", context->protocol);
    fprintf(f, "  Master context: %p%s\n", context->m_context,
	    context->m_context == context ? " IS MASTER" : "");
    fprintf(f, "  Recent recv child: %p\n", context->recent_rcvd_child);
    fprintf(f, "  Recent sent child: %p\n", context->recent_sent_child);
    fprintf(f, "  Recent child: %p\n\n", context->recent_child);
    fprintf(f, "  Our instance:   %08x\n", context->our_instance);
    fprintf(f, "  Their instance: %08x\n\n", context->their_instance);
    fprintf(f, "  Msgstate: %d (%s)\n\n", context->msgstate,
	context->msgstate == OTRL_MSGSTATE_PLAINTEXT ? "PLAINTEXT" :
	context->msgstate == OTRL_MSGSTATE_ENCRYPTED ? "ENCRYPTED" :
	context->msgstate == OTRL_MSGSTATE_FINISHED ? "FINISHED" :
	"INVALID");
    otrl_auth_dump(f, &context->auth);
    fprintf(f, "\n  Fingerprints:\n");
    for (fing = context->fingerprint_root.next; fing; fing = fing->next) {
	fprintf(f, "    %p ", fing);
	if (fing->fingerprint == NULL) {
	    fprintf(f, "(null)");
	} else {
	    int i;
	    for (i=0;i<20;++i) {
		fprintf(f, "%02x", fing->fingerprint[i]);
	    }
	}
	fprintf(f, " %p", fing->context);
	if (fing->trust && fing->trust[0]) {
	    fprintf(f, " %s", fing->trust);
	}
	fprintf(f, "\n");
    }
    fprintf(f, "\n  Active fingerprint: %p\n\n", context->active_fingerprint);
    fprintf(f, "  Protocol version: %d\n", context->protocol_version);
    fprintf(f, "  OTR offer: %d (%s)\n\n", context->otr_offer,
	context->otr_offer == OFFER_NOT ? "NOT" :
	context->otr_offer == OFFER_SENT ? "SENT" :
	context->otr_offer == OFFER_REJECTED ? "REJECTED" :
	context->otr_offer == OFFER_ACCEPTED ? "ACCEPTED" :
	"INVALID");

    fprintf(f, "  Application data: %p\n", context->app_data);
    if (context->smstate == NULL) {
	fprintf(f, "  SM state: NULL\n");
    } else {
	otrl_sm_dump(f, context->smstate);
    }
    fprintf(f, "\n");
}

/* Dump the master context of this context, and all of its children. */
void otrl_context_siblings_dump(FILE *f, const ConnContext *context)
{
    const ConnContext *citer;
    for (citer = context->m_context;
	    citer && citer->m_context == context->m_context;
	    citer = citer->next) {
	if (citer == context) {
	    fprintf(f, "*** ");
	}
	otrl_context_dump(f, citer);
    }
}

/* Dump all contexts. */
void otrl_context_all_dump(FILE *f, OtrlUserState us)
{
    const ConnContext *citer;
    unsigned int ctxnum = 1;
    for (citer = us->context_root; citer; citer = citer->next, ++ctxnum) {
	fprintf(f, "%u. ", ctxnum);
	otrl_context_dump(f, citer);
    }
}
#endif

/* Create a new connection context. */
static ConnContext * new_context(const char * user, const char * accountname,
	const char * protocol)
{
    ConnContext * context;
    OtrlSMState *smstate;

    context = malloc(sizeof(ConnContext));
    assert(context != NULL);

    context->username = strdup(user);
    context->accountname = strdup(accountname);
    context->protocol = strdup(protocol);

    context->msgstate = OTRL_MSGSTATE_PLAINTEXT;
    otrl_auth_new(context);

    smstate = malloc(sizeof(OtrlSMState));
    assert(smstate != NULL);
    otrl_sm_state_new(smstate);
    context->smstate = smstate;

    context->our_instance = 0;
    context->their_instance = OTRL_INSTAG_MASTER;
    context->fingerprint_root.fingerprint = NULL;
    context->fingerprint_root.context = context;
    context->fingerprint_root.next = NULL;
    context->fingerprint_root.tous = NULL;
    context->active_fingerprint = NULL;
    memset(context->sessionid, 0, 20);
    context->sessionid_len = 0;
    context->protocol_version = 0;
    context->otr_offer = OFFER_NOT;
    context->app_data = NULL;
    context->app_data_free = NULL;
    context->context_priv = otrl_context_priv_new();
    assert(context->context_priv != NULL);
    context->next = NULL;
    context->m_context = context;
    context->recent_rcvd_child = NULL;
    context->recent_sent_child = NULL;
    context->recent_child = NULL;

    return context;
}

ConnContext * otrl_context_find_recent_instance(ConnContext * context,
	otrl_instag_t recent_instag) {
    ConnContext * m_context;

    if (!context) return NULL;

    m_context = context->m_context;

    if (!m_context) return NULL;

    switch(recent_instag) {
	case OTRL_INSTAG_RECENT:
	    return m_context->recent_child;
	case OTRL_INSTAG_RECENT_RECEIVED:
	    return m_context->recent_rcvd_child;
	case OTRL_INSTAG_RECENT_SENT:
	    return m_context->recent_sent_child;
	default:
	    return NULL;
    }
}

/* Find the instance of this context that has the best security level,
   and for which we have most recently received a message from. Note that most
   recent in this case is limited to a one-second resolution. */
ConnContext * otrl_context_find_recent_secure_instance(ConnContext * context)
{
    ConnContext *curp; /* for iteration */
    ConnContext *m_context; /* master */
    ConnContext *cresult = context;  /* best so far */

    if (!context) {
	return cresult;
    }

    m_context = context->m_context;

    for (curp = m_context; curp && curp->m_context == m_context;
	    curp = curp->next) {
	int msgstate_improved = 0; /* 0 == same, 1 == improved   */
	int trust_improved = 0;    /* (will immediately 'continue' if worse
				    * than) */

	if (cresult->msgstate == curp->msgstate) {
	    msgstate_improved = 0;
	} else if (curp->msgstate == OTRL_MSGSTATE_ENCRYPTED ||
		(cresult->msgstate == OTRL_MSGSTATE_PLAINTEXT &&
		curp->msgstate == OTRL_MSGSTATE_FINISHED)) {
	    msgstate_improved = 1;
	} else {
	    continue;
	}


	if (otrl_context_is_fingerprint_trusted(cresult->active_fingerprint) ==
		otrl_context_is_fingerprint_trusted(curp->active_fingerprint)) {

	    trust_improved = 0;
	} else if
		(otrl_context_is_fingerprint_trusted(curp->active_fingerprint)){

	    trust_improved = 1;
	} else {
	    continue;
	}

	if (msgstate_improved || trust_improved ||
		(!msgstate_improved && !trust_improved &&
		curp->context_priv->lastrecv >=
		cresult->context_priv->lastrecv)) {
	    cresult = curp;
	}
    }

    return cresult;
}

/* Look up a connection context by name/account/protocol/instag from the given
 * OtrlUserState.  If add_if_missing is true, allocate and return a new
 * context if one does not currently exist.  In that event, call
 * add_app_data(data, context) so that app_data and app_data_free can be
 * filled in by the application, and set *addedp to 1.
 * In the 'their_instance' field note that you can also specify a 'meta-
 * instance' value such as OTRL_INSTAG_MASTER, OTRL_INSTAG_RECENT,
 * OTRL_INSTAG_RECENT_RECEIVED and OTRL_INSTAG_RECENT_SENT. */
ConnContext * otrl_context_find(OtrlUserState us, const char *user,
	const char *accountname, const char *protocol,
	otrl_instag_t their_instance, int add_if_missing, int *addedp,
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
		(protocmp = strcmp((*curp)->protocol, protocol)) > 0) ||
		(usercmp == 0 && acctcmp == 0 && protocmp == 0
		&& (their_instance < OTRL_MIN_VALID_INSTAG ||
		    ((*curp)->their_instance >= their_instance))))
	    /* We're at the right place in the list.  We've either found
	     * it, or gone too far. */
	    break;
    }

    if (usercmp == 0 && acctcmp == 0 && protocmp == 0 && *curp &&
	    (their_instance < OTRL_MIN_VALID_INSTAG ||
	    (their_instance == (*curp)->their_instance))) {
	/* Found one! */
	if (their_instance >= OTRL_MIN_VALID_INSTAG ||
		their_instance == OTRL_INSTAG_MASTER) {
	    return *curp;
	}

	/* We need to go back and check more values in the context */
	switch(their_instance) {
	    case OTRL_INSTAG_BEST:
		return otrl_context_find_recent_secure_instance(*curp);
	    case OTRL_INSTAG_RECENT:
	    case OTRL_INSTAG_RECENT_RECEIVED:
	    case OTRL_INSTAG_RECENT_SENT:
		return otrl_context_find_recent_instance(*curp, their_instance);
	    default:
		return NULL;
	}
    }

    if (add_if_missing) {
	ConnContext *newctx;
	OtrlInsTag *our_instag = (OtrlInsTag *)otrl_instag_find(us, accountname,
		protocol);

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

	/* Initialize specified instance tags */
	if (our_instag) {
	    newctx->our_instance = our_instag->instag;
	}

	if (their_instance >= OTRL_MIN_VALID_INSTAG ||
		their_instance == OTRL_INSTAG_MASTER) {
	    newctx->their_instance = their_instance;
	}

	if (their_instance >= OTRL_MIN_VALID_INSTAG) {
	    newctx->m_context = otrl_context_find(us, user, accountname,
		protocol, OTRL_INSTAG_MASTER, 1, NULL, add_app_data, data);
	}

	if (their_instance == OTRL_INSTAG_MASTER) {
	    /* if we're adding a master, there are no children, so the most
	     * recent context is the one we add. */
	    newctx->recent_child = newctx;
	    newctx->recent_rcvd_child = newctx;
	    newctx->recent_sent_child = newctx;
	}

	return *curp;
    }
    return NULL;
}

/* Return true iff the given fingerprint is marked as trusted. */
int otrl_context_is_fingerprint_trusted(Fingerprint *fprint) {
    return fprint && fprint->trust && fprint->trust[0] != '\0';
}

/* This method gets called after sending or receiving a message, to
 * update the master context's "recent context" pointers. */
void otrl_context_update_recent_child(ConnContext *context,
	unsigned int sent_msg) {
    ConnContext *m_context = context->m_context;

    if (sent_msg) {
	m_context->recent_sent_child = context;
    } else {
	m_context->recent_rcvd_child = context;
    }

    m_context->recent_child = context;

}

/* Find a fingerprint in a given context, perhaps adding it if not
 * present. */
Fingerprint *otrl_context_find_fingerprint(ConnContext *context,
	unsigned char fingerprint[20], int add_if_missing, int *addedp)
{
    Fingerprint *f;
    if (addedp) *addedp = 0;

    if (!context || !context->m_context) return NULL;

    context = context->m_context;

    f = context->fingerprint_root.next;
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
	memmove(f->fingerprint, fingerprint, 20);
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
    fprint->trust = trust ? strdup(trust) : NULL;
}

/* Force a context into the OTRL_MSGSTATE_FINISHED state. */
void otrl_context_force_finished(ConnContext *context)
{
    context->msgstate = OTRL_MSGSTATE_FINISHED;
    otrl_auth_clear(&(context->auth));
    context->active_fingerprint = NULL;
    memset(context->sessionid, 0, 20);
    context->sessionid_len = 0;
    context->protocol_version = 0;
    otrl_sm_state_free(context->smstate);
    otrl_context_priv_force_finished(context->context_priv);
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

/* Forget a whole context, so long as it's PLAINTEXT. If a context has child
 * instances, don't remove this instance unless children are also all in
 * PLAINTEXT state. In this case, the children will also be removed.
 * Returns 0 on success, 1 on failure. */
int otrl_context_forget(ConnContext *context)
{
    if (context->msgstate != OTRL_MSGSTATE_PLAINTEXT) return 1;

    if (context->their_instance == OTRL_INSTAG_MASTER) {
	ConnContext *c_iter;

	for (c_iter = context; c_iter &&
		c_iter->m_context == context->m_context;
		c_iter = c_iter->next) {
	    if (c_iter->msgstate != OTRL_MSGSTATE_PLAINTEXT) return 1;
	}

	c_iter = context->next;
	while (c_iter && c_iter->m_context == context->m_context) {
	    if (!otrl_context_forget(c_iter)) {
		c_iter = context->next;
	    } else {
		return 1;
	    }
	}

    }

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
    return 0;
}

/* Forget all the contexts in a given OtrlUserState. */
void otrl_context_forget_all(OtrlUserState us)
{
    ConnContext *c_iter;

    for (c_iter = us->context_root; c_iter; c_iter = c_iter->next) {
	otrl_context_force_plaintext(c_iter);
    }

    while (us->context_root) {
	otrl_context_forget(us->context_root);
    }
}
