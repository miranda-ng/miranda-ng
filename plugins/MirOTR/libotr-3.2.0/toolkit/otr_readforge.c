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

/* libotr headers */
#include "proto.h"

/* toolkit headers */
#include "readotr.h"
#include "parse.h"
#include "sesskeys.h"
#include "sha1hmac.h"
#include "ctrmode.h"

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s aeskey [new_message]\n"
"Read an OTR Data Message from stdin.  Use the given AES key to\n"
"verify its MAC and decrypt the message to stdout.  If new_message\n"
"is given, output a new OTR Data Message with the same fields as the\n"
"original, but with the message replaced by new_message\n", progname);
    exit(1);
}

int main(int argc, char **argv)
{
    unsigned char *aeskey;
    unsigned char mackey[20];
    unsigned char macval[20];
    size_t aeskeylen;
    unsigned char *plaintext, *ciphertext;
    char *otrmsg = NULL;
    DataMsg datamsg;

    if (argc != 2 && argc != 3) {
	usage(argv[0]);
    }

    argv_to_buf(&aeskey, &aeskeylen, argv[1]);
    if (!aeskey) {
	usage(argv[0]);
    }
    
    if (aeskeylen != 16) {
	fprintf(stderr, "The AES key must be 32 hex chars long.\n");
	usage(argv[0]);
    }

    otrmsg = readotr(stdin);
    if (otrmsg == NULL) {
	fprintf(stderr, "No OTR Data Message found on stdin.\n");
	exit(1);
    }
    
    if (otrl_proto_message_type(otrmsg) != OTRL_MSGTYPE_DATA) {
	fprintf(stderr, "OTR Non-Data Message found on stdin.\n");
	exit(1);
    }

    datamsg = parse_datamsg(otrmsg);
    free(otrmsg);
    if (datamsg == NULL) {
	fprintf(stderr, "Invalid OTR Data Message found on stdin.\n");
	exit(1);
    }

    /* Create the MAC key */
    sesskeys_make_mac(mackey, aeskey);

    /* Check the MAC */
    sha1hmac(macval, mackey, datamsg->macstart,
	    datamsg->macend - datamsg->macstart);
    if (memcmp(macval, datamsg->mac, 20)) {
	fprintf(stderr, "MAC does not verify: wrong AES key?\n");
    } else {
	/* Decrypt the message */
	plaintext = malloc(datamsg->encmsglen+1);
	if (!plaintext) {
	    fprintf(stderr, "Out of memory!\n");
	    exit(1);
	}
	aes_ctr_crypt(plaintext, datamsg->encmsg, datamsg->encmsglen,
		aeskey, datamsg->ctr);
	plaintext[datamsg->encmsglen] = '\0';
	printf("Plaintext: ``%s''\n", plaintext);
	free(plaintext);
    }

    /* Do we want to forge a message? */
    if (argv[2] != NULL) {
	char *newdatamsg;
	size_t newlen = strlen(argv[2]);
	ciphertext = malloc(newlen);
	if (!ciphertext && newlen > 0) {
	    fprintf(stderr, "Out of memory!\n");
	    exit(1);
	}
	aes_ctr_crypt(ciphertext, (const unsigned char *)argv[2], newlen,
		aeskey, datamsg->ctr);
	free(datamsg->encmsg);
	datamsg->encmsg = ciphertext;
	datamsg->encmsglen = newlen;
	
	newdatamsg = remac_datamsg(datamsg, mackey);

	printf("%s\n", newdatamsg);
	free(newdatamsg);
    }

    free_datamsg(datamsg);
    free(aeskey);
    fflush(stdout);
    return 0;
}
