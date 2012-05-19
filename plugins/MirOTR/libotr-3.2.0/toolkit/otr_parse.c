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

static void parse(const char *msg)
{
    OtrlMessageType mtype = otrl_proto_message_type(msg);
    CommitMsg cmsg;
    KeyMsg kmsg;
    RevealSigMsg rmsg;
    SignatureMsg smsg;
    KeyExchMsg keyexch;
    DataMsg datamsg;

    switch(mtype) {
	case OTRL_MSGTYPE_QUERY:
	    printf("OTR Query:\n\t%s\n\n", msg);
	    break;
	case OTRL_MSGTYPE_DH_COMMIT:
	    cmsg = parse_commit(msg);
	    if (!cmsg) {
		printf("Invalid D-H Commit Message\n\n");
		break;
	    }
	    printf("D-H Commit Message:\n");
	    dump_data(stdout, "\tEncrypted Key", cmsg->enckey,
		    cmsg->enckeylen);
	    dump_data(stdout, "\tHashed Key", cmsg->hashkey,
		    cmsg->hashkeylen);
	    printf("\n");
	    free_commit(cmsg);
	    break;
	case OTRL_MSGTYPE_DH_KEY:
	    kmsg = parse_key(msg);
	    if (!kmsg) {
		printf("Invalid D-H Key Message\n\n");
		break;
	    }
	    printf("D-H Key Message:\n");
	    dump_mpi(stdout, "\tD-H Key", kmsg->y);
	    printf("\n");
	    free_key(kmsg);
	    break;
	case OTRL_MSGTYPE_REVEALSIG:
	    rmsg = parse_revealsig(msg);
	    if (!rmsg) {
		printf("Invalid Reveal Signature Message\n\n");
		break;
	    }
	    printf("Reveal Signature Message:\n");
	    dump_data(stdout, "\tKey", rmsg->key, rmsg->keylen);
	    dump_data(stdout, "\tEncrypted Signature",
		    rmsg->encsig, rmsg->encsiglen);
	    dump_data(stdout, "\tMAC", rmsg->mac, 20);
	    printf("\n");
	    free_revealsig(rmsg);
	    break;
	case OTRL_MSGTYPE_SIGNATURE:
	    smsg = parse_signature(msg);
	    if (!smsg) {
		printf("Invalid Signature Message\n\n");
		break;
	    }
	    printf("Signature Message:\n");
	    dump_data(stdout, "\tEncrypted Signature",
		    smsg->encsig, smsg->encsiglen);
	    dump_data(stdout, "\tMAC", smsg->mac, 20);
	    printf("\n");
	    free_signature(smsg);
	    break;
	case OTRL_MSGTYPE_V1_KEYEXCH:
	    keyexch = parse_keyexch(msg);
	    if (!keyexch) {
		printf("Invalid Key Exchange Message\n\n");
		break;
	    }
	    printf("Key Exchange Message:\n");
	    dump_int(stdout, "\tReply", keyexch->reply);
	    dump_mpi(stdout, "\tDSA p", keyexch->p);
	    dump_mpi(stdout, "\tDSA q", keyexch->q);
	    dump_mpi(stdout, "\tDSA g", keyexch->g);
	    dump_mpi(stdout, "\tDSA e", keyexch->e);
	    dump_int(stdout, "\tKeyID", keyexch->keyid);
	    dump_mpi(stdout, "\tDH y", keyexch->y);
	    dump_mpi(stdout, "\tSIG r", keyexch->r);
	    dump_mpi(stdout, "\tSIG s", keyexch->s);
	    printf("\n");
	    free_keyexch(keyexch);
	    break;
	case OTRL_MSGTYPE_DATA:
	    datamsg = parse_datamsg(msg);
	    if (!datamsg) {
		printf("Invalid Data Message\n\n");
		break;
	    }
	    printf("Data Message:\n");
	    if (datamsg->flags >= 0) {
		dump_int(stdout, "\tFlags", datamsg->flags);
	    }
	    dump_int(stdout, "\tSender keyid", datamsg->sender_keyid);
	    dump_int(stdout, "\tRcpt keyid", datamsg->rcpt_keyid);
	    dump_mpi(stdout, "\tDH y", datamsg->y);
	    dump_data(stdout, "\tCounter", datamsg->ctr, 8);
	    dump_data(stdout, "\tEncrypted message", datamsg->encmsg,
		    datamsg->encmsglen);
	    dump_data(stdout, "\tMAC", datamsg->mac, 20);
	    if (datamsg->mackeyslen > 0) {
		size_t len = datamsg->mackeyslen;
		unsigned char *mks = datamsg->mackeys;
		unsigned int i = 0;
		printf("\tRevealed MAC keys:\n");

		while(len > 19) {
		    char title[20];
		    sprintf(title, "\t\tKey %u", ++i);
		    dump_data(stdout, title, mks, 20);
		    mks += 20; len -= 20;
		}
	    }

	    printf("\n");
	    free_datamsg(datamsg);
	    break;
	case OTRL_MSGTYPE_ERROR:
	    printf("OTR Error:\n\t%s\n\n", msg);
	    break;
	case OTRL_MSGTYPE_TAGGEDPLAINTEXT:
	    printf("Tagged plaintext message:\n\t%s\n\n", msg);
	    break;
	case OTRL_MSGTYPE_NOTOTR:
	    printf("Not an OTR message:\n\t%s\n\n", msg);
	    break;
	case OTRL_MSGTYPE_UNKNOWN:
	    printf("Unrecognized OTR message:\n\t%s\n\n", msg);
	    break;
    }
    fflush(stdout);
}

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s\n"
"Read Off-the-Record (OTR) Key Exchange and/or Data messages from stdin\n"
"and display their contents in a more readable format.\n", progname);
    exit(1);
}

int main(int argc, char **argv)
{
    char *otrmsg = NULL;

    if (argc != 1) {
	usage(argv[0]);
    }

    while ((otrmsg = readotr(stdin)) != NULL) {
	parse(otrmsg);
	free(otrmsg);
    }

    return 0;
}
