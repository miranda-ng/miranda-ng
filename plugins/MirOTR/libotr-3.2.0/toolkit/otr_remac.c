/*
 *  Off-the-Record Messaging Toolkit
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* toolkit headers */
#include "parse.h"
#include "sha1hmac.h"

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s mackey flags snd_keyid rcp_keyid pubkey "
	    "counter encdata revealed_mackeys\n"
"Make a new Data message, with the given pieces (note that the\n"
"data part is already encrypted).  MAC it with the given mackey.\n"
"mackey, pubkey, counter, encdata, and revealed_mackeys are given\n"
"as strings of hex chars.  snd_keyid and rcp_keyid are decimal integers.\n", progname);
    exit(1);
}

int main(int argc, char **argv)
{
    unsigned char *mackey;
    size_t mackeylen;
    unsigned int snd_keyid, rcp_keyid;
    int flags;
    unsigned char *pubkey;
    size_t pubkeylen;
    gcry_mpi_t pubv;
    unsigned char *ctr;
    size_t ctrlen;
    unsigned char *encdata;
    size_t encdatalen;
    unsigned char *mackeys;
    size_t mackeyslen;
    char *newdatamsg;

    if (argc != 9) {
	usage(argv[0]);
    }

    argv_to_buf(&mackey, &mackeylen, argv[1]);
    if (!mackey) {
	usage(argv[0]);
    }
    
    if (mackeylen != 20) {
	fprintf(stderr, "The MAC key must be 40 hex chars long.\n");
	usage(argv[0]);
    }

    if (sscanf(argv[2], "%d", &flags) != 1) {
	fprintf(stderr, "Unparseable flags given.\n");
	usage(argv[0]);
    }

    if (sscanf(argv[3], "%u", &snd_keyid) != 1) {
	fprintf(stderr, "Unparseable snd_keyid given.\n");
	usage(argv[0]);
    }

    if (sscanf(argv[4], "%u", &rcp_keyid) != 1) {
	fprintf(stderr, "Unparseable rcp_keyid given.\n");
	usage(argv[0]);
    }

    argv_to_buf(&pubkey, &pubkeylen, argv[5]);
    if (!pubkey) {
	usage(argv[0]);
    }
    gcry_mpi_scan(&pubv, GCRYMPI_FMT_USG, pubkey, pubkeylen, NULL);
    free(pubkey);
    
    argv_to_buf(&ctr, &ctrlen, argv[6]);
    if (!ctr) {
	usage(argv[0]);
    }

    if (ctrlen != 8) {
	fprintf(stderr, "The counter must be 16 hex chars long.\n");
	usage(argv[0]);
    }

    argv_to_buf(&encdata, &encdatalen, argv[7]);
    if (!encdata) {
	usage(argv[0]);
    }

    argv_to_buf(&mackeys, &mackeyslen, argv[8]);
    if (!mackeys) {
	usage(argv[0]);
    }

    newdatamsg = assemble_datamsg(mackey, flags, snd_keyid, rcp_keyid,
	    pubv, ctr, encdata, encdatalen, mackeys, mackeyslen);
    printf("%s\n", newdatamsg);
    free(newdatamsg);

    free(mackey);
    gcry_mpi_release(pubv);
    free(ctr);
    free(encdata);
    free(mackeys);
    fflush(stdout);
    return 0;
}
