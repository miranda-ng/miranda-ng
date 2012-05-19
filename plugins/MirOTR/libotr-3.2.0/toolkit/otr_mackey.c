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

/* toolkit headers */
#include "parse.h"
#include "sesskeys.h"

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s aeskey\n"
"Calculate and display the MAC key derived from a given AES key.\n",
	progname);
    exit(1);
}

int main(int argc, char **argv)
{
    unsigned char *argbuf;
    size_t argbuflen;
    unsigned char mackey[20];

    if (argc != 2) {
	usage(argv[0]);
    }

    argv_to_buf(&argbuf, &argbuflen, argv[1]);
    /* AES keys are 128 bits long, so check for that */
    if (!argbuf) {
	usage(argv[0]);
    }
    
    if (argbuflen != 16) {
	fprintf(stderr, "The AES key must be 32 hex chars long.\n");
	usage(argv[0]);
    }

    sesskeys_make_mac(mackey, argbuf);

    dump_data(stdout, "AES key", argbuf, 16);
    dump_data(stdout, "MAC key", mackey, 20);

    free(argbuf);
    fflush(stdout);
    return 0;
}
