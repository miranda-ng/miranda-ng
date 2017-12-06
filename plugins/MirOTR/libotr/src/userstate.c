/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2012  Ian Goldberg, Rob Smits, Chris Alexander,
 *  			      Willy Lew, Lisa Du, Nikita Borisov
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

/* libotr headers */
#include "context.h"
#include "privkey.h"
#include "userstate.h"

/* Create a new OtrlUserState.  Most clients will only need one of
 * these.  A OtrlUserState encapsulates the list of known fingerprints
 * and the list of private keys; if you have separate files for these
 * things for (say) different users, use different OtrlUserStates.  If
 * you've got only one user, with multiple accounts all stored together
 * in the same fingerprint store and privkey store files, use just one
 * OtrlUserState. */
OtrlUserState otrl_userstate_create(void)
{
    OtrlUserState us = malloc(sizeof(struct s_OtrlUserState));
    if (!us) return NULL;
    us->context_root = NULL;
    us->privkey_root = NULL;
    us->instag_root = NULL;
    us->pending_root = NULL;
    us->timer_running = 0;
    return us;
}

/* Free a OtrlUserState.  If you have a timer running for this userstate,
stop it before freeing the userstate. */
void otrl_userstate_free(OtrlUserState us)
{
    otrl_context_forget_all(us);
    otrl_privkey_forget_all(us);
    otrl_privkey_pending_forget_all(us);
    otrl_instag_forget_all(us);
    free(us);
}
