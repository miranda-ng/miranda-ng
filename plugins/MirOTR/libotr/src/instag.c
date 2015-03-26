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
#include <stdio.h>
#include <stdlib.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "instag.h"
#include "userstate.h"

/* Forget the given instag. */
void otrl_instag_forget(OtrlInsTag* instag) {
    if (!instag) return;

    if (instag->accountname) free(instag->accountname);
    if (instag->protocol) free(instag->protocol);

    /* Re-link the list */
    *(instag->tous) = instag->next;
    if (instag->next) {
	instag->next->tous = instag->tous;
    }

    free(instag);
}

/* Forget all instags in a given OtrlUserState. */
void otrl_instag_forget_all(OtrlUserState us) {
    while(us->instag_root) {
	otrl_instag_forget(us->instag_root);
    }
}

/* Fetch the instance tag from the given OtrlUserState associated with
 * the given account */
OtrlInsTag * otrl_instag_find(OtrlUserState us, const char *accountname,
	const char *protocol)
{
    OtrlInsTag *p;

    for(p=us->instag_root; p; p=p->next) {
	if (!strcmp(p->accountname, accountname) &&
		!strcmp(p->protocol, protocol)) {
	    return p;
	}
    }
    return NULL;
}

/* Read our instance tag from a file on disk into the given
 * OtrlUserState. */
gcry_error_t otrl_instag_read(OtrlUserState us, const char *filename)
{
    gcry_error_t err;
    FILE *instf;

    /* Open the instance tag file. */
    instf = fopen(filename, "rb");
    if (!instf) {
	return gcry_error_from_errno(errno);
    }

    err = otrl_instag_read_FILEp(us, instf);
    fclose(instf);
    return err;
}

/* Read our instance tag from a file on disk into the given
 * OtrlUserState. The FILE* must be open for reading. */
gcry_error_t otrl_instag_read_FILEp(OtrlUserState us, FILE *instf)
{

    OtrlInsTag *p;
    char storeline[1000];
    size_t maxsize = sizeof(storeline);
    
    if (!instf) return gcry_error(GPG_ERR_NO_ERROR);

    while(fgets(storeline, maxsize, instf)) {
	char *prevpos;
	char *pos;
	unsigned int instag = 0;

	p = malloc(sizeof(*p));
	if (!p) {
	    return gcry_error(GPG_ERR_ENOMEM);
	}

	/* Parse the line, which should be of the form:
	 * accountname\tprotocol\t40_hex_nybbles\n          */
	prevpos = storeline;
	pos = strchr(prevpos, '\t');
	if (!pos) {
	    free(p);
	    continue;
	}
	*pos = '\0';
	pos++;
	p->accountname = malloc(pos - prevpos);
	memmove(p->accountname, prevpos, pos - prevpos);

	prevpos = pos;
	pos = strchr(prevpos, '\t');
	if (!pos) {
	    free(p);
	    continue;
	}
	*pos = '\0';
	pos++;
	p->protocol = malloc(pos - prevpos);
	memmove(p->protocol, prevpos, pos - prevpos);

	prevpos = pos;
	pos = strchr(prevpos, '\r');
	if (!pos) pos = strchr(prevpos, '\n');
	if (!pos) {
	    free(p);
	    continue;
	}
	*pos = '\0';
	pos++;
	/* hex str of length 8 */
	if (strlen(prevpos) != 8) {
	    free(p);
	    continue;
	}

	sscanf(prevpos, "%08x", &instag);

	if (instag < OTRL_MIN_VALID_INSTAG) {
	    free(p);
	    continue;
	}
	p->instag = instag;

	/* Link it up */
	p->next = us->instag_root;
	if (p->next) {
	    p->next->tous = &(p->next);
	}
	p->tous = &(us->instag_root);
	us->instag_root = p;
    }

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Generate a new instance tag for the given account and write to file */
gcry_error_t otrl_instag_generate(OtrlUserState us, const char *filename,
	const char *accountname, const char *protocol)
{
    gcry_error_t err;
    FILE *instf;

    /* Open the instance tag file. */
    instf = fopen(filename, "wb");
    if (!instf) {
	return gcry_error_from_errno(errno);
    }

    err = otrl_instag_generate_FILEp(us, instf, accountname, protocol);
    fclose(instf);
    return err;
}

/* Return a new valid instance tag */
otrl_instag_t otrl_instag_get_new()
{
    otrl_instag_t result = 0;

    while(result < OTRL_MIN_VALID_INSTAG) {
	otrl_instag_t * instag = (otrl_instag_t *)gcry_random_bytes(
		sizeof(otrl_instag_t), GCRY_STRONG_RANDOM);
	result = *instag;
	gcry_free(instag);
    }

    return result;
}

/* Generate a new instance tag for the given account and write to file
 * The FILE* must be open for writing. */
gcry_error_t otrl_instag_generate_FILEp(OtrlUserState us, FILE *instf,
	const char *accountname, const char *protocol)
{
    OtrlInsTag *p;
    if (!accountname || !protocol) return gcry_error(GPG_ERR_NO_ERROR);

    p = (OtrlInsTag *)malloc(sizeof(OtrlInsTag));
    p->accountname = strdup(accountname);
    p->protocol = strdup(protocol);

    p->instag = otrl_instag_get_new();

    /* Add to our list in OtrlUserState */
    p->next = us->instag_root;
    if (p->next) {
	p->next->tous = &(p->next);
    }
    p->tous = &(us->instag_root);
    us->instag_root = p;

    otrl_instag_write_FILEp(us, instf);

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Write our instance tags to a file on disk. */
gcry_error_t otrl_instag_write(OtrlUserState us, const char *filename)
{
    gcry_error_t err;
    FILE *instf;

    /* Open the instance tag file. */
    instf = fopen(filename, "wb");
    if (!instf) {
	return gcry_error_from_errno(errno);
    }

    err = otrl_instag_write_FILEp(us, instf);
    fclose(instf);
    return err;
}

/* Write our instance tags to a file on disk.
 * The FILE* must be open for writing. */
gcry_error_t otrl_instag_write_FILEp(OtrlUserState us, FILE *instf)
{
    OtrlInsTag *p;
    /* This line should be ignored when read back in, since there are no
    tabs. */
    fprintf(instf, "# WARNING! You shouldn't copy this file to another"
    " computer. It is unnecessary and can cause problems.\n");
    for(p=us->instag_root; p; p=p->next) {
	fprintf(instf, "%s\t%s\t%08x\n", p->accountname, p->protocol,
		p->instag);
    }

    return gcry_error(GPG_ERR_NO_ERROR);
}

