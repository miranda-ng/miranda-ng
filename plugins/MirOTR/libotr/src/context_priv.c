/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2012  Ian Goldberg, Chris Alexander, Willy Lew,
 *			     Nikita Borisov
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
#include "context_priv.h"

/* Create a new private connection context */
ConnContextPriv *otrl_context_priv_new()
{
	ConnContextPriv *context_priv;
	context_priv = malloc(sizeof(*context_priv));
	assert(context_priv != NULL);

	context_priv->fragment = NULL;
	context_priv->fragment_len = 0;
	context_priv->fragment_n = 0;
	context_priv->fragment_k = 0;
	context_priv->numsavedkeys = 0;
	context_priv->saved_mac_keys = NULL;
	context_priv->generation = 0;
	context_priv->lastsent = 0;
	context_priv->lastmessage = NULL;
	context_priv->lastrecv = 0;
	context_priv->may_retransmit = 0;
	context_priv->their_keyid = 0;
	context_priv->their_y = NULL;
	context_priv->their_old_y = NULL;
	context_priv->our_keyid = 0;
	context_priv->our_dh_key.groupid = 0;
	context_priv->our_dh_key.priv = NULL;
	context_priv->our_dh_key.pub = NULL;
	context_priv->our_old_dh_key.groupid = 0;
	context_priv->our_old_dh_key.priv = NULL;
	context_priv->our_old_dh_key.pub = NULL;
	otrl_dh_session_blank(&(context_priv->sesskeys[0][0]));
	otrl_dh_session_blank(&(context_priv->sesskeys[0][1]));
	otrl_dh_session_blank(&(context_priv->sesskeys[1][0]));
	otrl_dh_session_blank(&(context_priv->sesskeys[1][1]));

	return context_priv;
}

/* Resets the appropriate variables when a context
 * is being force finished
 */
void otrl_context_priv_force_finished(ConnContextPriv *context_priv)
{
	free(context_priv->fragment);
	context_priv->fragment = NULL;
	context_priv->fragment_len = 0;
	context_priv->fragment_n = 0;
	context_priv->fragment_k = 0;
	context_priv->numsavedkeys = 0;
	free(context_priv->saved_mac_keys);
	context_priv->saved_mac_keys = NULL;
	gcry_free(context_priv->lastmessage);
	context_priv->lastmessage = NULL;
	context_priv->may_retransmit = 0;
	context_priv->their_keyid = 0;
	gcry_mpi_release(context_priv->their_y);
	context_priv->their_y = NULL;
	gcry_mpi_release(context_priv->their_old_y);
	context_priv->their_old_y = NULL;
	context_priv->our_keyid = 0;
	otrl_dh_keypair_free(&(context_priv->our_dh_key));
	otrl_dh_keypair_free(&(context_priv->our_old_dh_key));
	otrl_dh_session_free(&(context_priv->sesskeys[0][0]));
	otrl_dh_session_free(&(context_priv->sesskeys[0][1]));
	otrl_dh_session_free(&(context_priv->sesskeys[1][0]));
	otrl_dh_session_free(&(context_priv->sesskeys[1][1]));
}
