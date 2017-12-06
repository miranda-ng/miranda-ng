/*
 * otrlextensions.c - Off-the-Record Messaging library extensions
 *
 * Strongly based on parts of the Off-the-Record Messaging library,
 * Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                          <otr@cypherpunks.ca>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include <privkey.h>

#include "otrlextensions.h"

static gcry_error_t sexp_write(FILE* privf, gcry_sexp_t sexp)
{
    size_t buflen;
    char* buf;

    buflen = gcry_sexp_sprint(sexp, GCRYSEXP_FMT_ADVANCED, NULL, 0);
    buf = malloc(buflen);
    if (buf == NULL && buflen > 0) {
        return gcry_error(GPG_ERR_ENOMEM);
    }
    gcry_sexp_sprint(sexp, GCRYSEXP_FMT_ADVANCED, buf, buflen);

    fprintf(privf, "%s", buf);
    free(buf);

    return gcry_error(GPG_ERR_NO_ERROR);
}

static gcry_error_t account_write(FILE* privf, const char* accountname,
    const char* protocol, gcry_sexp_t privkey)
{
    gcry_error_t err;
    gcry_sexp_t names, protos;

    fprintf(privf, " (account\n");

    err = gcry_sexp_build(&names, NULL, "(name %s)", accountname);
    if (!err) {
        err = sexp_write(privf, names);
        gcry_sexp_release(names);
    }
    if (!err) err = gcry_sexp_build(&protos, NULL, "(protocol %s)", protocol);
    if (!err) {
        err = sexp_write(privf, protos);
        gcry_sexp_release(protos);
    }
    if (!err) err = sexp_write(privf, privkey);

    fprintf(privf, " )\n");

    return err;
}

/* Store all keys of an OtrlUserState.
 * The FILE* must be open for reading and writing. */
gcry_error_t otrl_privkey_write_FILEp(OtrlUserState us, FILE* privf)
{
    OtrlPrivKey* p;

    /* Output the other keys we know */
    fprintf(privf, "(privkeys\n");

    for (p=us->privkey_root; p; p=p->next) {
        account_write(privf, p->accountname, p->protocol, p->privkey);
    }

    fprintf(privf, ")\n");

    fseek(privf, 0, SEEK_SET);

    return otrl_privkey_read_FILEp(us, privf);
}

/* Store all keys of an OtrlUserState. */
gcry_error_t otrl_privkey_write(OtrlUserState us, const char* filename)
{
    gcry_error_t err;
    FILE* privf;
#ifndef WIN32
    mode_t oldmask;
#endif

#ifndef WIN32
    oldmask = umask(077);
#endif
    privf = fopen(filename, "w+b");
    if (!privf) {
#ifndef WIN32
        umask(oldmask);
#endif
        err = gcry_error_from_errno(errno);
        return err;
    }

    err = otrl_privkey_write_FILEp(us, privf);

    fclose(privf);
#ifndef WIN32
    umask(oldmask);
#endif
    return err;
}
