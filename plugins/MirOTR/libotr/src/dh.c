/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2014  Ian Goldberg, David Goulet, Rob Smits,
 *                           Chris Alexander, Willy Lew, Lisa Du,
 *                           Nikita Borisov
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

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "dh.h"


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
static const int DH1536_MOD_LEN_BITS = 1536;
static const int DH1536_MOD_LEN_BYTES = 192;

static gcry_mpi_t DH1536_MODULUS = NULL;
static gcry_mpi_t DH1536_MODULUS_MINUS_2 = NULL;
static gcry_mpi_t DH1536_GENERATOR = NULL;

/*
 * Call this once, at plugin load time.  It sets up the modulus and
 * generator MPIs.
 */
void otrl_dh_init(void)
{
    gcry_mpi_scan(&DH1536_MODULUS, GCRYMPI_FMT_HEX,
	(const unsigned char *)DH1536_MODULUS_S, 0, NULL);
    gcry_mpi_scan(&DH1536_GENERATOR, GCRYMPI_FMT_HEX,
	(const unsigned char *)DH1536_GENERATOR_S, 0, NULL);
    DH1536_MODULUS_MINUS_2 = gcry_mpi_new(DH1536_MOD_LEN_BITS);
    gcry_mpi_sub_ui(DH1536_MODULUS_MINUS_2, DH1536_MODULUS, 2);
}

/*
 * Initialize the fields of a DH keypair.
 */
void otrl_dh_keypair_init(DH_keypair *kp)
{
    kp->groupid = 0;
    kp->priv = NULL;
    kp->pub = NULL;
}

/*
 * Copy a DH_keypair.
 */
void otrl_dh_keypair_copy(DH_keypair *dst, const DH_keypair *src)
{
    dst->groupid = src->groupid;
    dst->priv = gcry_mpi_copy(src->priv);
    dst->pub = gcry_mpi_copy(src->pub);
}

/*
 * Deallocate the contents of a DH_keypair (but not the DH_keypair
 * itself)
 */
void otrl_dh_keypair_free(DH_keypair *kp)
{
    gcry_mpi_release(kp->priv);
    gcry_mpi_release(kp->pub);
    kp->priv = NULL;
    kp->pub = NULL;
}

/*
 * Generate a DH keypair for a specified group.
 */
gcry_error_t otrl_dh_gen_keypair(unsigned int groupid, DH_keypair *kp)
{
    unsigned char *secbuf = NULL;
    gcry_mpi_t privkey = NULL;

    if (groupid != DH1536_GROUP_ID) {
	/* Invalid group id */
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Generate the secret key: a random 320-bit value */
    secbuf = gcry_random_bytes_secure(40, GCRY_STRONG_RANDOM);
    gcry_mpi_scan(&privkey, GCRYMPI_FMT_USG, secbuf, 40, NULL);
    gcry_free(secbuf);

    kp->groupid = groupid;
    kp->priv = privkey;
    kp->pub = gcry_mpi_new(DH1536_MOD_LEN_BITS);
    gcry_mpi_powm(kp->pub, DH1536_GENERATOR, privkey, DH1536_MODULUS);
    return gcry_error(GPG_ERR_NO_ERROR);
}

/*
 * Construct session keys from a DH keypair and someone else's public
 * key.
 */
gcry_error_t otrl_dh_session(DH_sesskeys *sess, const DH_keypair *kp,
	gcry_mpi_t y)
{
    gcry_mpi_t gab;
    size_t gablen;
    unsigned char *gabdata;
    unsigned char *hashdata;
    unsigned char sendbyte, rcvbyte;
    gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);

    otrl_dh_session_blank(sess);

    if (kp->groupid != DH1536_GROUP_ID) {
	/* Invalid group id */
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Calculate the shared secret MPI */
    gab = gcry_mpi_snew(DH1536_MOD_LEN_BITS);
    gcry_mpi_powm(gab, y, kp->priv, DH1536_MODULUS);

    /* Output it in the right format */
    gcry_mpi_print(GCRYMPI_FMT_USG, NULL, 0, &gablen, gab);
    gabdata = gcry_malloc_secure(gablen + 5);
    if (!gabdata) {
	gcry_mpi_release(gab);
	return gcry_error(GPG_ERR_ENOMEM);
    }
    gabdata[1] = (gablen >> 24) & 0xff;
    gabdata[2] = (gablen >> 16) & 0xff;
    gabdata[3] = (gablen >> 8) & 0xff;
    gabdata[4] = gablen & 0xff;
    gcry_mpi_print(GCRYMPI_FMT_USG, gabdata+5, gablen, NULL, gab);
    gcry_mpi_release(gab);

    hashdata = gcry_malloc_secure(20);
    if (!hashdata) {
	gcry_free(gabdata);
	return gcry_error(GPG_ERR_ENOMEM);
    }

    /* Are we the "high" or "low" end of the connection? */
    if ( gcry_mpi_cmp(kp->pub, y) > 0 ) {
	sendbyte = 0x01;
	rcvbyte = 0x02;
    } else {
	sendbyte = 0x02;
	rcvbyte = 0x01;
    }

    /* Calculate the sending encryption key */
    gabdata[0] = sendbyte;
    gcry_md_hash_buffer(GCRY_MD_SHA1, hashdata, gabdata, gablen+5);
    err = gcry_cipher_open(&(sess->sendenc), GCRY_CIPHER_AES,
	    GCRY_CIPHER_MODE_CTR, GCRY_CIPHER_SECURE);
    if (err) goto err;
    err = gcry_cipher_setkey(sess->sendenc, hashdata, 16);
    if (err) goto err;

    /* Calculate the sending MAC key */
    gcry_md_hash_buffer(GCRY_MD_SHA1, sess->sendmackey, hashdata, 16);
    err = gcry_md_open(&(sess->sendmac), GCRY_MD_SHA1, GCRY_MD_FLAG_HMAC);
    if (err) goto err;
    err = gcry_md_setkey(sess->sendmac, sess->sendmackey, 20);
    if (err) goto err;

    /* Calculate the receiving encryption key */
    gabdata[0] = rcvbyte;
    gcry_md_hash_buffer(GCRY_MD_SHA1, hashdata, gabdata, gablen+5);
    err = gcry_cipher_open(&(sess->rcvenc), GCRY_CIPHER_AES,
	    GCRY_CIPHER_MODE_CTR, GCRY_CIPHER_SECURE);
    if (err) goto err;
    err = gcry_cipher_setkey(sess->rcvenc, hashdata, 16);
    if (err) goto err;

    /* Calculate the receiving MAC key (and save it in the DH_sesskeys
     * struct, so we can reveal it later) */
    gcry_md_hash_buffer(GCRY_MD_SHA1, sess->rcvmackey, hashdata, 16);
    err = gcry_md_open(&(sess->rcvmac), GCRY_MD_SHA1, GCRY_MD_FLAG_HMAC);
    if (err) goto err;
    err = gcry_md_setkey(sess->rcvmac, sess->rcvmackey, 20);
    if (err) goto err;

    /* Calculate the extra key (used if applications wish to extract a
     * symmetric key for transferring files, or something like that) */
    gabdata[0] = 0xff;
    gcry_md_hash_buffer(GCRY_MD_SHA256, sess->extrakey, gabdata, gablen+5);

    gcry_free(gabdata);
    gcry_free(hashdata);
    return gcry_error(GPG_ERR_NO_ERROR);
err:
    otrl_dh_session_free(sess);
    gcry_free(gabdata);
    gcry_free(hashdata);
    return err;
}

/*
 * Compute the secure session id, two encryption keys, and four MAC keys
 * given our DH key and their DH public key.
 */
gcry_error_t otrl_dh_compute_v2_auth_keys(const DH_keypair *our_dh,
	gcry_mpi_t their_pub, unsigned char *sessionid, size_t *sessionidlenp,
	gcry_cipher_hd_t *enc_c, gcry_cipher_hd_t *enc_cp,
	gcry_md_hd_t *mac_m1, gcry_md_hd_t *mac_m1p,
	gcry_md_hd_t *mac_m2, gcry_md_hd_t *mac_m2p)
{
    gcry_mpi_t s;
    size_t slen;
    unsigned char *sdata;
    unsigned char *hashdata;
    unsigned char ctr[16];
    gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);

    *enc_c = NULL;
    *enc_cp = NULL;
    *mac_m1 = NULL;
    *mac_m1p = NULL;
    *mac_m2 = NULL;
    *mac_m2p = NULL;
    memset(ctr, 0, 16);

    if (our_dh->groupid != DH1536_GROUP_ID) {
	/* Invalid group id */
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Check that their_pub is in range */
    if (gcry_mpi_cmp_ui(their_pub, 2) < 0 ||
	    gcry_mpi_cmp(their_pub, DH1536_MODULUS_MINUS_2) > 0) {
	/* Invalid pubkey */
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Calculate the shared secret MPI */
    s = gcry_mpi_snew(DH1536_MOD_LEN_BITS);
    gcry_mpi_powm(s, their_pub, our_dh->priv, DH1536_MODULUS);

    /* Output it in the right format */
    gcry_mpi_print(GCRYMPI_FMT_USG, NULL, 0, &slen, s);
    sdata = gcry_malloc_secure(slen + 5);
    if (!sdata) {
	gcry_mpi_release(s);
	return gcry_error(GPG_ERR_ENOMEM);
    }
    sdata[1] = (slen >> 24) & 0xff;
    sdata[2] = (slen >> 16) & 0xff;
    sdata[3] = (slen >> 8) & 0xff;
    sdata[4] = slen & 0xff;
    gcry_mpi_print(GCRYMPI_FMT_USG, sdata+5, slen, NULL, s);
    gcry_mpi_release(s);

    /* Calculate the session id */
    hashdata = gcry_malloc_secure(32);
    if (!hashdata) {
	gcry_free(sdata);
	return gcry_error(GPG_ERR_ENOMEM);
    }
    sdata[0] = 0x00;
    gcry_md_hash_buffer(GCRY_MD_SHA256, hashdata, sdata, slen+5);
    memmove(sessionid, hashdata, 8);
    *sessionidlenp = 8;

    /* Calculate the encryption keys */
    sdata[0] = 0x01;
    gcry_md_hash_buffer(GCRY_MD_SHA256, hashdata, sdata, slen+5);

    err = gcry_cipher_open(enc_c, GCRY_CIPHER_AES,
	    GCRY_CIPHER_MODE_CTR, GCRY_CIPHER_SECURE);
    if (err) goto err;
    err = gcry_cipher_setkey(*enc_c, hashdata, 16);
    if (err) goto err;
    err = gcry_cipher_setctr(*enc_c, ctr, 16);
    if (err) goto err;

    err = gcry_cipher_open(enc_cp, GCRY_CIPHER_AES,
	    GCRY_CIPHER_MODE_CTR, GCRY_CIPHER_SECURE);
    if (err) goto err;
    err = gcry_cipher_setkey(*enc_cp, hashdata+16, 16);
    if (err) goto err;
    err = gcry_cipher_setctr(*enc_cp, ctr, 16);
    if (err) goto err;

    /* Calculate the MAC keys */
    sdata[0] = 0x02;
    gcry_md_hash_buffer(GCRY_MD_SHA256, hashdata, sdata, slen+5);
    err = gcry_md_open(mac_m1, GCRY_MD_SHA256, GCRY_MD_FLAG_HMAC);
    if (err) goto err;
    err = gcry_md_setkey(*mac_m1, hashdata, 32);
    if (err) goto err;

    sdata[0] = 0x03;
    gcry_md_hash_buffer(GCRY_MD_SHA256, hashdata, sdata, slen+5);
    err = gcry_md_open(mac_m2, GCRY_MD_SHA256, GCRY_MD_FLAG_HMAC);
    if (err) goto err;
    err = gcry_md_setkey(*mac_m2, hashdata, 32);
    if (err) goto err;

    sdata[0] = 0x04;
    gcry_md_hash_buffer(GCRY_MD_SHA256, hashdata, sdata, slen+5);
    err = gcry_md_open(mac_m1p, GCRY_MD_SHA256, GCRY_MD_FLAG_HMAC);
    if (err) goto err;
    err = gcry_md_setkey(*mac_m1p, hashdata, 32);
    if (err) goto err;

    sdata[0] = 0x05;
    gcry_md_hash_buffer(GCRY_MD_SHA256, hashdata, sdata, slen+5);
    err = gcry_md_open(mac_m2p, GCRY_MD_SHA256, GCRY_MD_FLAG_HMAC);
    if (err) goto err;
    err = gcry_md_setkey(*mac_m2p, hashdata, 32);
    if (err) goto err;

    gcry_free(sdata);
    gcry_free(hashdata);
    return gcry_error(GPG_ERR_NO_ERROR);

err:
    gcry_cipher_close(*enc_c);
    gcry_cipher_close(*enc_cp);
    gcry_md_close(*mac_m1);
    gcry_md_close(*mac_m1p);
    gcry_md_close(*mac_m2);
    gcry_md_close(*mac_m2p);
    *enc_c = NULL;
    *enc_cp = NULL;
    *mac_m1 = NULL;
    *mac_m1p = NULL;
    *mac_m2 = NULL;
    *mac_m2p = NULL;
    gcry_free(sdata);
    gcry_free(hashdata);
    return err;
}

/*
 * Compute the secure session id, given our DH key and their DH public
 * key.
 */
gcry_error_t otrl_dh_compute_v1_session_id(const DH_keypair *our_dh,
	gcry_mpi_t their_pub, unsigned char *sessionid, size_t *sessionidlenp,
	OtrlSessionIdHalf *halfp)
{
    gcry_mpi_t s;
    size_t slen;
    unsigned char *sdata;
    unsigned char *hashdata;

    if (our_dh->groupid != DH1536_GROUP_ID) {
	/* Invalid group id */
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Check that their_pub is in range */
    if (gcry_mpi_cmp_ui(their_pub, 2) < 0 ||
	    gcry_mpi_cmp(their_pub, DH1536_MODULUS_MINUS_2) > 0) {
	/* Invalid pubkey */
	return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Calculate the shared secret MPI */
    s = gcry_mpi_snew(DH1536_MOD_LEN_BITS);
    gcry_mpi_powm(s, their_pub, our_dh->priv, DH1536_MODULUS);

    /* Output it in the right format */
    gcry_mpi_print(GCRYMPI_FMT_USG, NULL, 0, &slen, s);
    sdata = gcry_malloc_secure(slen + 5);
    if (!sdata) {
	gcry_mpi_release(s);
	return gcry_error(GPG_ERR_ENOMEM);
    }
    sdata[1] = (slen >> 24) & 0xff;
    sdata[2] = (slen >> 16) & 0xff;
    sdata[3] = (slen >> 8) & 0xff;
    sdata[4] = slen & 0xff;
    gcry_mpi_print(GCRYMPI_FMT_USG, sdata+5, slen, NULL, s);
    gcry_mpi_release(s);

    /* Calculate the session id */
    hashdata = gcry_malloc_secure(20);
    if (!hashdata) {
	gcry_free(sdata);
	return gcry_error(GPG_ERR_ENOMEM);
    }
    sdata[0] = 0x00;
    gcry_md_hash_buffer(GCRY_MD_SHA1, hashdata, sdata, slen+5);
    memmove(sessionid, hashdata, 20);
    *sessionidlenp = 20;

    /* Which half should be bold? */
    if (gcry_mpi_cmp(our_dh->pub, their_pub) > 0) {
	*halfp = OTRL_SESSIONID_SECOND_HALF_BOLD;
    } else {
	*halfp = OTRL_SESSIONID_FIRST_HALF_BOLD;
    }

    gcry_free(hashdata);
    gcry_free(sdata);
    return gcry_error(GPG_ERR_NO_ERROR);
}

/*
 * Deallocate the contents of a DH_sesskeys (but not the DH_sesskeys
 * itself)
 */
void otrl_dh_session_free(DH_sesskeys *sess)
{
    gcry_cipher_close(sess->sendenc);
    gcry_cipher_close(sess->rcvenc);
    gcry_md_close(sess->sendmac);
    gcry_md_close(sess->rcvmac);

    otrl_dh_session_blank(sess);
}

/*
 * Blank out the contents of a DH_sesskeys (without releasing it)
 */
void otrl_dh_session_blank(DH_sesskeys *sess)
{
    sess->sendenc = NULL;
    sess->sendmac = NULL;
    sess->rcvenc = NULL;
    sess->rcvmac = NULL;
    memset(sess->sendctr, 0, 16);
    memset(sess->rcvctr, 0, 16);
    memset(sess->sendmackey, 0, 20);
    memset(sess->rcvmackey, 0, 20);
    sess->sendmacused = 0;
    sess->rcvmacused = 0;
    memset(sess->extrakey, 0, OTRL_EXTRAKEY_BYTES);
}

/* Increment the top half of a counter block */
void otrl_dh_incctr(unsigned char *ctr)
{
    int i;
    for (i=8;i;--i) {
	if (++ctr[i-1]) break;
    }
}

/* Compare two counter values (8 bytes each).  Return 0 if ctr1 == ctr2,
 * < 0 if ctr1 < ctr2 (as unsigned 64-bit values), > 0 if ctr1 > ctr2. */
int otrl_dh_cmpctr(const unsigned char *ctr1, const unsigned char *ctr2)
{
    int i;
    for (i=0;i<8;++i) {
	int c = ctr1[i] - ctr2[i];
	if (c) return c;
    }
    return 0;
}
