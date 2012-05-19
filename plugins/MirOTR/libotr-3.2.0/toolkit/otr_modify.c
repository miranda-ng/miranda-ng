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
#include "sha1hmac.h"

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s mackey old_text new_text offset\n"
"Read an OTR Data Message from stdin.  Even if we can't read the\n"
"data because we don't know either the AES key or the DH privkey,\n"
"but we can make a good guess that the substring \"old_text\"\n"
"appears at the given offset in the message, replace the old_text\n"
"with the new_text (which must be of the same length), recalculate\n"
"the MAC with the given mackey, and output the resulting Data message.\n",
    progname);
    exit(1);
}

int main(int argc, char **argv)
{
    unsigned char *mackey;
    size_t mackeylen;
    unsigned char macval[20];
    char *otrmsg = NULL;
    DataMsg datamsg;
    size_t textlen;
    unsigned int offset;
    const unsigned char *old_text, *new_text;
    char *newdatamsg;
    size_t i;

    if (argc != 5) {
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

    textlen = strlen(argv[2]);
    if (textlen != strlen(argv[3])) {
	fprintf(stderr, "The old_text and new_text must be of the same "
		"length.\n");
	usage(argv[0]);
    }
    old_text = (const unsigned char *)argv[2];
    new_text = (const unsigned char *)argv[3];

    if (sscanf(argv[4], "%u", &offset) != 1) {
	fprintf(stderr, "Unparseable offset given.\n");
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

    /* Check the MAC */
    sha1hmac(macval, mackey, datamsg->macstart,
	    datamsg->macend - datamsg->macstart);
    if (memcmp(macval, datamsg->mac, 20)) {
	fprintf(stderr, "MAC does not verify: wrong MAC key?\n");
	exit(1);
    }

    /* Modify the ciphertext */
    for(i=0; i<textlen && offset+i < datamsg->encmsglen; ++i) {
	datamsg->encmsg[offset+i] ^= (old_text[i] ^ new_text[i]);
    }

    /* Recalculate the MAC */
    newdatamsg = remac_datamsg(datamsg, mackey);
    printf("%s\n", newdatamsg);
    free(newdatamsg);

    free_datamsg(datamsg);
    free(mackey);
    fflush(stdout);
    return 0;
}
