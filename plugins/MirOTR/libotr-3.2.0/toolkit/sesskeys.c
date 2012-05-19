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
#include <stdlib.h>

/* libgcrypt headers */
#include <gcrypt.h>

static const char* DH1536_MODULUS_S = "0x"
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D"
    "670C354E4ABC9804F1746C08CA237327FFFFFFFFFFFFFFFF";
static const char *DH1536_GENERATOR_S = "0x02";

/* Generate the session id and the two encryption keys from our private
 * DH key and their public DH key.  Also indicate in *high_endp if we
 * are the "high" end of the key exchange (set to 1) or the "low" end
 * (set to 0) */
void sesskeys_gen(unsigned char sessionid[20], unsigned char sendenc[16],
	unsigned char rcvenc[16], int *high_endp, gcry_mpi_t *our_yp,
	gcry_mpi_t our_x, gcry_mpi_t their_y)
{
    gcry_mpi_t modulus, generator, secretv;
    unsigned char *secret;
    size_t secretlen;
    unsigned char hash[20];
    int is_high;

    gcry_mpi_scan(&modulus, GCRYMPI_FMT_HEX, DH1536_MODULUS_S, 0, NULL);
    gcry_mpi_scan(&generator, GCRYMPI_FMT_HEX, DH1536_GENERATOR_S, 0, NULL);
    *our_yp = gcry_mpi_new(0);
    gcry_mpi_powm(*our_yp, generator, our_x, modulus);
    secretv = gcry_mpi_new(0);
    gcry_mpi_powm(secretv, their_y, our_x, modulus);
    gcry_mpi_release(generator);
    gcry_mpi_release(modulus);
    gcry_mpi_print(GCRYMPI_FMT_USG, NULL, 0, &secretlen, secretv);
    secret = malloc(secretlen + 5);

    secret[1] = (secretlen >> 24) & 0xff;
    secret[2] = (secretlen >> 16) & 0xff;
    secret[3] = (secretlen >> 8) & 0xff;
    secret[4] = (secretlen) & 0xff;
    gcry_mpi_print(GCRYMPI_FMT_USG, secret+5, secretlen, NULL, secretv);
    gcry_mpi_release(secretv);

    is_high = (gcry_mpi_cmp(*our_yp, their_y) > 0);

    /* Calculate the session id */
    secret[0] = 0x00;
    gcry_md_hash_buffer(GCRY_MD_SHA1, hash, secret, secretlen+5);
    memmove(sessionid, hash, 20);

    /* Calculate the sending enc key */
    secret[0] = is_high ? 0x01 : 0x02;
    gcry_md_hash_buffer(GCRY_MD_SHA1, hash, secret, secretlen+5);
    memmove(sendenc, hash, 16);

    /* Calculate the receiving enc key */
    secret[0] = is_high ? 0x02 : 0x01;
    gcry_md_hash_buffer(GCRY_MD_SHA1, hash, secret, secretlen+5);
    memmove(rcvenc, hash, 16);

    *high_endp = is_high;
    free(secret);
}

/* Generate a MAC key from the corresponding encryption key */
void sesskeys_make_mac(unsigned char mackey[20], unsigned char enckey[16])
{
    gcry_md_hash_buffer(GCRY_MD_SHA1, mackey, enckey, 16);
}
