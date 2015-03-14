/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* system headers */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* libotr headers */
#include "b64.h"
#include "privkey.h"
#include "auth.h"
#include "serial.h"

/*
 * Initialize the fields of an OtrlAuthInfo (already allocated).
 */
void otrl_auth_new(OtrlAuthInfo *auth)
{
	auth->authstate = OTRL_AUTHSTATE_NONE;
	otrl_dh_keypair_init(&(auth->our_dh));
	auth->our_keyid = 0;
	auth->encgx = NULL;
	auth->encgx_len = 0;
	memset(auth->r, 0, 16);
	memset(auth->hashgx, 0, 32);
	auth->their_pub = NULL;
	auth->their_keyid = 0;
	auth->enc_c = NULL;
	auth->enc_cp = NULL;
	auth->mac_m1 = NULL;
	auth->mac_m1p = NULL;
	auth->mac_m2 = NULL;
	auth->mac_m2p = NULL;
	memset(auth->their_fingerprint, 0, 20);
	auth->initiated = 0;
	auth->protocol_version = 0;
	memset(auth->secure_session_id, 0, 20);
	auth->secure_session_id_len = 0;
	auth->lastauthmsg = NULL;
}

/*
 * Clear the fields of an OtrlAuthInfo (but leave it allocated).
 */
void otrl_auth_clear(OtrlAuthInfo *auth)
{
	auth->authstate = OTRL_AUTHSTATE_NONE;
	otrl_dh_keypair_free(&(auth->our_dh));
	auth->our_keyid = 0;
	free(auth->encgx);
	auth->encgx = NULL;
	auth->encgx_len = 0;
	memset(auth->r, 0, 16);
	memset(auth->hashgx, 0, 32);
	gcry_mpi_release(auth->their_pub);
	auth->their_pub = NULL;
	auth->their_keyid = 0;
	gcry_cipher_close(auth->enc_c);
	gcry_cipher_close(auth->enc_cp);
	gcry_md_close(auth->mac_m1);
	gcry_md_close(auth->mac_m1p);
	gcry_md_close(auth->mac_m2);
	gcry_md_close(auth->mac_m2p);
	auth->enc_c = NULL;
	auth->enc_cp = NULL;
	auth->mac_m1 = NULL;
	auth->mac_m1p = NULL;
	auth->mac_m2 = NULL;
	auth->mac_m2p = NULL;
	memset(auth->their_fingerprint, 0, 20);
	auth->initiated = 0;
	auth->protocol_version = 0;
	memset(auth->secure_session_id, 0, 20);
	auth->secure_session_id_len = 0;
	free(auth->lastauthmsg);
	auth->lastauthmsg = NULL;
}

/*
 * Start a fresh AKE (version 2) using the given OtrlAuthInfo.  Generate
 * a fresh DH keypair to use.  If no error is returned, the message to
 * transmit will be contained in auth->lastauthmsg.
 */
gcry_error_t otrl_auth_start_v2(OtrlAuthInfo *auth)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	const enum gcry_mpi_format format = GCRYMPI_FMT_USG;
	size_t npub;
	gcry_cipher_hd_t enc = NULL;
	unsigned char ctr[16];
	unsigned char *buf, *bufp;
	size_t buflen, lenp;

	/* Clear out this OtrlAuthInfo and start over */
	otrl_auth_clear(auth);
	auth->initiated = 1;

	otrl_dh_gen_keypair(DH1536_GROUP_ID, &(auth->our_dh));
	auth->our_keyid = 1;

	/* Pick an encryption key */
	gcry_randomize(auth->r, 16, GCRY_STRONG_RANDOM);

	/* Allocate space for the encrypted g^x */
	gcry_mpi_print(format, NULL, 0, &npub, auth->our_dh.pub);
	auth->encgx = malloc(4+npub);
	if (auth->encgx == NULL) goto memerr;
	auth->encgx_len = 4+npub;
	bufp = auth->encgx;
	lenp = auth->encgx_len;
	write_mpi(auth->our_dh.pub, npub, "g^x");
	assert(lenp == 0);

	/* Hash g^x */
	gcry_md_hash_buffer(GCRY_MD_SHA256, auth->hashgx, auth->encgx,
		auth->encgx_len);

	/* Encrypt g^x using the key r */
	err = gcry_cipher_open(&enc, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CTR,
		GCRY_CIPHER_SECURE);
	if (err) goto err;

	err = gcry_cipher_setkey(enc, auth->r, 16);
	if (err) goto err;

	memset(ctr, 0, 16);
	err = gcry_cipher_setctr(enc, ctr, 16);
	if (err) goto err;

	err = gcry_cipher_encrypt(enc, auth->encgx, auth->encgx_len, NULL, 0);
	if (err) goto err;

	gcry_cipher_close(enc);
	enc = NULL;

	/* Now serialize the message */
	lenp = 3 + 4 + auth->encgx_len + 4 + 32;
	bufp = malloc(lenp);
	if (bufp == NULL) goto memerr;
	buf = bufp;
	buflen = lenp;

	memcpy(bufp, "\x00\x02\x02", 3); /* header */
	debug_data("Header", bufp, 3);
	bufp += 3; lenp -= 3;

	/* Encrypted g^x */
	write_int(auth->encgx_len);
	debug_int("Enc gx len", bufp-4);
	memcpy(bufp, auth->encgx, auth->encgx_len);
	debug_data("Enc gx", bufp, auth->encgx_len);
	bufp += auth->encgx_len; lenp -= auth->encgx_len;

	/* Hashed g^x */
	write_int(32);
	debug_int("hashgx len", bufp-4);
	memcpy(bufp, auth->hashgx, 32);
	debug_data("hashgx", bufp, 32);
	bufp += 32; lenp -= 32;

	assert(lenp == 0);

	auth->lastauthmsg = otrl_base64_otr_encode(buf, buflen);
	free(buf);
	if (auth->lastauthmsg == NULL) goto memerr;
	auth->authstate = OTRL_AUTHSTATE_AWAITING_DHKEY;

	return err;

memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	otrl_auth_clear(auth);
	gcry_cipher_close(enc);
	return err;
}

/*
 * Create a D-H Key Message using the our_dh value in the given auth,
 * and store it in auth->lastauthmsg.
 */
static gcry_error_t create_key_message(OtrlAuthInfo *auth)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	const enum gcry_mpi_format format = GCRYMPI_FMT_USG;
	unsigned char *buf, *bufp;
	size_t buflen, lenp;
	size_t npub;

	gcry_mpi_print(format, NULL, 0, &npub, auth->our_dh.pub);
	buflen = 3 + 4 + npub;
	buf = malloc(buflen);
	if (buf == NULL) goto memerr;
	bufp = buf;
	lenp = buflen;

	memcpy(bufp, "\x00\x02\x0a", 3); /* header */
	debug_data("Header", bufp, 3);
	bufp += 3; lenp -= 3;

	/* g^y */
	write_mpi(auth->our_dh.pub, npub, "g^y");

	assert(lenp == 0);

	free(auth->lastauthmsg);
	auth->lastauthmsg = otrl_base64_otr_encode(buf, buflen);
	free(buf);
	if (auth->lastauthmsg == NULL) goto memerr;

	return err;

memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
	return err;
}

/*
 * Handle an incoming D-H Commit Message.  If no error is returned, the
 * message to send will be left in auth->lastauthmsg.  Generate a fresh
 * keypair to use.
 */
gcry_error_t otrl_auth_handle_commit(OtrlAuthInfo *auth,
	const char *commitmsg)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp = NULL, *encbuf = NULL;
	unsigned char hashbuf[32];
	size_t buflen, lenp, enclen, hashlen;
	int res;

	res = otrl_base64_otr_decode(commitmsg, &buf, &buflen);
	if (res == -1) goto memerr;
	if (res == -2) goto invval;

	bufp = buf;
	lenp = buflen;

	/* Header */
	require_len(3);
	if (memcmp(bufp, "\x00\x02\x02", 3)) goto invval;
	bufp += 3; lenp -= 3;

	/* Encrypted g^x */
	read_int(enclen);
	require_len(enclen);
	encbuf = malloc(enclen);
	if (encbuf == NULL && enclen > 0) goto memerr;
	memcpy(encbuf, bufp, enclen);
	bufp += enclen; lenp -= enclen;

	/* Hashed g^x */
	read_int(hashlen);
	if (hashlen != 32) goto invval;
	require_len(32);
	memcpy(hashbuf, bufp, 32);
	bufp += 32; lenp -= 32;

	if (lenp != 0) goto invval;
	free(buf);
	buf = NULL;

	switch(auth->authstate) {
	case OTRL_AUTHSTATE_NONE:
	case OTRL_AUTHSTATE_AWAITING_SIG:
	case OTRL_AUTHSTATE_V1_SETUP:

		/* Store the incoming information */
		otrl_auth_clear(auth);
		otrl_dh_gen_keypair(DH1536_GROUP_ID, &(auth->our_dh));
		auth->our_keyid = 1;
		auth->encgx = encbuf;
		encbuf = NULL;
		auth->encgx_len = enclen;
		memcpy(auth->hashgx, hashbuf, 32);

		/* Create a D-H Key Message */
		err = create_key_message(auth);
		if (err) goto err;
		auth->authstate = OTRL_AUTHSTATE_AWAITING_REVEALSIG;

		break;

	case OTRL_AUTHSTATE_AWAITING_DHKEY:
		/* We sent a D-H Commit Message, and we also received one
		 * back.  Compare the hashgx values to see which one wins. */
		if (memcmp(auth->hashgx, hashbuf, 32) > 0) {
		/* Ours wins.  Ignore the message we received, and just
		 * resend the same D-H Commit message again. */
		free(encbuf);
		encbuf = NULL;
		} else {
		/* Ours loses.  Use the incoming parameters instead. */
		otrl_auth_clear(auth);
		otrl_dh_gen_keypair(DH1536_GROUP_ID, &(auth->our_dh));
		auth->our_keyid = 1;
		auth->encgx = encbuf;
		encbuf = NULL;
		auth->encgx_len = enclen;
		memcpy(auth->hashgx, hashbuf, 32);

		/* Create a D-H Key Message */
		err = create_key_message(auth);
		if (err) goto err;
		auth->authstate = OTRL_AUTHSTATE_AWAITING_REVEALSIG;
		}
		break;
	case OTRL_AUTHSTATE_AWAITING_REVEALSIG:
		/* Use the incoming parameters, but just retransmit the old
		 * D-H Key Message. */
		free(auth->encgx);
		auth->encgx = encbuf;
		encbuf = NULL;
		auth->encgx_len = enclen;
		memcpy(auth->hashgx, hashbuf, 32);
		break;
	}

	return err;

invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	free(encbuf);
	return err;
}

/*
 * Calculate the encrypted part of the Reveal Signature and Signature
 * Messages, given a MAC key, an encryption key, two DH public keys, an
 * authentication public key (contained in an OtrlPrivKey structure),
 * and a keyid.  If no error is returned, *authbufp will point to the
 * result, and *authlenp will point to its length.
 */
static gcry_error_t calculate_pubkey_auth(unsigned char **authbufp,
	size_t *authlenp, gcry_md_hd_t mackey, gcry_cipher_hd_t enckey,
	gcry_mpi_t our_dh_pub, gcry_mpi_t their_dh_pub,
	OtrlPrivKey *privkey, unsigned int keyid)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	const enum gcry_mpi_format format = GCRYMPI_FMT_USG;
	size_t ourpublen, theirpublen, totallen, lenp;
	unsigned char *buf = NULL, *bufp = NULL;
	unsigned char macbuf[32];
	unsigned char *sigbuf = NULL;
	size_t siglen;

	/* How big are the DH public keys? */
	gcry_mpi_print(format, NULL, 0, &ourpublen, our_dh_pub);
	gcry_mpi_print(format, NULL, 0, &theirpublen, their_dh_pub);

	/* How big is the total structure to be MAC'd? */
	totallen = 4 + ourpublen + 4 + theirpublen + 2 + privkey->pubkey_datalen
	+ 4;
	buf = malloc(totallen);
	if (buf == NULL) goto memerr;

	bufp = buf;
	lenp = totallen;

	/* Write the data to be MAC'd */
	write_mpi(our_dh_pub, ourpublen, "Our DH pubkey");
	write_mpi(their_dh_pub, theirpublen, "Their DH pubkey");
	bufp[0] = ((privkey->pubkey_type) >> 8) & 0xff;
	bufp[1] = (privkey->pubkey_type) & 0xff;
	bufp += 2; lenp -= 2;
	memcpy(bufp, privkey->pubkey_data, privkey->pubkey_datalen);
	debug_data("Pubkey", bufp, privkey->pubkey_datalen);
	bufp += privkey->pubkey_datalen; lenp -= privkey->pubkey_datalen;
	write_int(keyid);
	debug_int("Keyid", bufp-4);

	assert(lenp == 0);

	/* Do the MAC */
	gcry_md_reset(mackey);
	gcry_md_write(mackey, buf, totallen);
	memcpy(macbuf, gcry_md_read(mackey, GCRY_MD_SHA256), 32);

	free(buf);
	buf = NULL;

	/* Sign the MAC */
	err = otrl_privkey_sign(&sigbuf, &siglen, privkey, macbuf, 32);
	if (err) goto err;

	/* Calculate the total size of the structure to be encrypted */
	totallen = 2 + privkey->pubkey_datalen + 4 + siglen;
	buf = malloc(totallen);
	if (buf == NULL) goto memerr;
	bufp = buf;
	lenp = totallen;

	/* Write the data to be encrypted */
	bufp[0] = ((privkey->pubkey_type) >> 8) & 0xff;
	bufp[1] = (privkey->pubkey_type) & 0xff;
	bufp += 2; lenp -= 2;
	memcpy(bufp, privkey->pubkey_data, privkey->pubkey_datalen);
	debug_data("Pubkey", bufp, privkey->pubkey_datalen);
	bufp += privkey->pubkey_datalen; lenp -= privkey->pubkey_datalen;
	write_int(keyid);
	debug_int("Keyid", bufp-4);
	memcpy(bufp, sigbuf, siglen);
	debug_data("Signature", bufp, siglen);
	bufp += siglen; lenp -= siglen;
	free(sigbuf);
	sigbuf = NULL;

	assert(lenp == 0);

	/* Now do the encryption */
	err = gcry_cipher_encrypt(enckey, buf, totallen, NULL, 0);
	if (err) goto err;

	*authbufp = buf;
	buf = NULL;
	*authlenp = totallen;

	return err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	free(sigbuf);
	return err;
}

/*
 * Decrypt the authenticator in the Reveal Signature and Signature
 * Messages, given a MAC key, and encryption key, and two DH public
 * keys.  The fingerprint of the received public key will get put into
 * fingerprintbufp, and the received keyid will get put in *keyidp.
 * The encrypted data pointed to by authbuf will be decrypted in place.
 */
static gcry_error_t check_pubkey_auth(unsigned char fingerprintbufp[20],
	unsigned int *keyidp, unsigned char *authbuf, size_t authlen,
	gcry_md_hd_t mackey, gcry_cipher_hd_t enckey,
	gcry_mpi_t our_dh_pub, gcry_mpi_t their_dh_pub)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	const enum gcry_mpi_format format = GCRYMPI_FMT_USG;
	size_t ourpublen, theirpublen, totallen, lenp;
	unsigned char *buf = NULL, *bufp = NULL;
	unsigned char macbuf[32];
	unsigned short pubkey_type;
	gcry_mpi_t p,q,g,y;
	gcry_sexp_t pubs = NULL;
	unsigned int received_keyid;
	unsigned char *fingerprintstart, *fingerprintend, *sigbuf;
	size_t siglen;

	/* Start by decrypting it */
	err = gcry_cipher_decrypt(enckey, authbuf, authlen, NULL, 0);
	if (err) goto err;

	bufp = authbuf;
	lenp = authlen;

	/* Get the public key and calculate its fingerprint */
	require_len(2);
	pubkey_type = (bufp[0] << 8) + bufp[1];
	bufp += 2; lenp -= 2;
	if (pubkey_type != OTRL_PUBKEY_TYPE_DSA) goto invval;
	fingerprintstart = bufp;
	read_mpi(p);
	read_mpi(q);
	read_mpi(g);
	read_mpi(y);
	fingerprintend = bufp;
	gcry_md_hash_buffer(GCRY_MD_SHA1, fingerprintbufp,
		fingerprintstart, fingerprintend-fingerprintstart);
	gcry_sexp_build(&pubs, NULL,
	"(public-key (dsa (p %m)(q %m)(g %m)(y %m)))", p, q, g, y);
	gcry_mpi_release(p);
	gcry_mpi_release(q);
	gcry_mpi_release(g);
	gcry_mpi_release(y);

	/* Get the keyid */
	read_int(received_keyid);
	if (received_keyid == 0) goto invval;

	/* Get the signature */
	sigbuf = bufp;
	siglen = lenp;

	/* How big are the DH public keys? */
	gcry_mpi_print(format, NULL, 0, &ourpublen, our_dh_pub);
	gcry_mpi_print(format, NULL, 0, &theirpublen, their_dh_pub);

	/* Now calculate the message to be MAC'd. */
	totallen = 4 + ourpublen + 4 + theirpublen + 2 +
	(fingerprintend - fingerprintstart) + 4;
	buf = malloc(totallen);
	if (buf == NULL) goto memerr;

	bufp = buf;
	lenp = totallen;

	write_mpi(their_dh_pub, theirpublen, "Their DH pubkey");
	write_mpi(our_dh_pub, ourpublen, "Our DH pubkey");
	bufp[0] = (pubkey_type >> 8) & 0xff;
	bufp[1] = pubkey_type & 0xff;
	bufp += 2; lenp -= 2;
	memcpy(bufp, fingerprintstart, fingerprintend - fingerprintstart);
	debug_data("Pubkey", bufp, fingerprintend - fingerprintstart);
	bufp += fingerprintend - fingerprintstart;
	lenp -= fingerprintend - fingerprintstart;
	write_int(received_keyid);
	debug_int("Keyid", bufp-4);

	assert(lenp == 0);

	/* Do the MAC */
	gcry_md_reset(mackey);
	gcry_md_write(mackey, buf, totallen);
	memcpy(macbuf, gcry_md_read(mackey, GCRY_MD_SHA256), 32);

	free(buf);
	buf = NULL;

	/* Verify the signature on the MAC */
	err = otrl_privkey_verify(sigbuf, siglen, pubkey_type, pubs, macbuf, 32);
	if (err) goto err;
	gcry_sexp_release(pubs);
	pubs = NULL;

	/* Everything checked out */
	*keyidp = received_keyid;

	return err;
invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	gcry_sexp_release(pubs);
	return err;
}

/*
 * Create a Reveal Signature Message using the values in the given auth,
 * and store it in auth->lastauthmsg.  Use the given privkey to sign the
 * message.
 */
static gcry_error_t create_revealsig_message(OtrlAuthInfo *auth,
	OtrlPrivKey *privkey)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp, *startmac;
	size_t buflen, lenp;

	unsigned char *authbuf = NULL;
	size_t authlen;

	/* Get the encrypted authenticator */
	err = calculate_pubkey_auth(&authbuf, &authlen, auth->mac_m1, auth->enc_c,
		auth->our_dh.pub, auth->their_pub, privkey, auth->our_keyid);
	if (err) goto err;

	buflen = 3 + 4 + 16 + 4 + authlen + 20;
	buf = malloc(buflen);
	if (buf == NULL) goto memerr;

	bufp = buf;
	lenp = buflen;

	memcpy(bufp, "\x00\x02\x11", 3); /* header */
	debug_data("Header", bufp, 3);
	bufp += 3; lenp -= 3;

	/* r */
	write_int(16);
	memcpy(bufp, auth->r, 16);
	debug_data("r", bufp, 16);
	bufp += 16; lenp -= 16;

	/* Encrypted authenticator */
	startmac = bufp;
	write_int(authlen);
	memcpy(bufp, authbuf, authlen);
	debug_data("auth", bufp, authlen);
	bufp += authlen; lenp -= authlen;
	free(authbuf);
	authbuf = NULL;

	/* MAC it, but only take the first 20 bytes */
	gcry_md_reset(auth->mac_m2);
	gcry_md_write(auth->mac_m2, startmac, bufp - startmac);
	memcpy(bufp, gcry_md_read(auth->mac_m2, GCRY_MD_SHA256), 20);
	debug_data("MAC", bufp, 20);
	bufp += 20; lenp -= 20;

	assert(lenp == 0);

	free(auth->lastauthmsg);
	auth->lastauthmsg = otrl_base64_otr_encode(buf, buflen);
	if (auth->lastauthmsg == NULL) goto memerr;
	free(buf);
	buf = NULL;

	return err;

memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	free(authbuf);
	return err;
}

/*
 * Create a Signature Message using the values in the given auth, and
 * store it in auth->lastauthmsg.  Use the given privkey to sign the
 * message.
 */
static gcry_error_t create_signature_message(OtrlAuthInfo *auth,
	OtrlPrivKey *privkey)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp, *startmac;
	size_t buflen, lenp;

	unsigned char *authbuf = NULL;
	size_t authlen;

	/* Get the encrypted authenticator */
	err = calculate_pubkey_auth(&authbuf, &authlen, auth->mac_m1p,
		auth->enc_cp, auth->our_dh.pub, auth->their_pub, privkey,
		auth->our_keyid);
	if (err) goto err;

	buflen = 3 + 4 + authlen + 20;
	buf = malloc(buflen);
	if (buf == NULL) goto memerr;

	bufp = buf;
	lenp = buflen;

	memcpy(bufp, "\x00\x02\x12", 3); /* header */
	debug_data("Header", bufp, 3);
	bufp += 3; lenp -= 3;

	/* Encrypted authenticator */
	startmac = bufp;
	write_int(authlen);
	memcpy(bufp, authbuf, authlen);
	debug_data("auth", bufp, authlen);
	bufp += authlen; lenp -= authlen;
	free(authbuf);
	authbuf = NULL;

	/* MAC it, but only take the first 20 bytes */
	gcry_md_reset(auth->mac_m2p);
	gcry_md_write(auth->mac_m2p, startmac, bufp - startmac);
	memcpy(bufp, gcry_md_read(auth->mac_m2p, GCRY_MD_SHA256), 20);
	debug_data("MAC", bufp, 20);
	bufp += 20; lenp -= 20;

	assert(lenp == 0);

	free(auth->lastauthmsg);
	auth->lastauthmsg = otrl_base64_otr_encode(buf, buflen);
	if (auth->lastauthmsg == NULL) goto memerr;
	free(buf);
	buf = NULL;

	return err;

memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	free(authbuf);
	return err;
}

/*
 * Handle an incoming D-H Key Message.  If no error is returned, and
 * *havemsgp is 1, the message to sent will be left in auth->lastauthmsg.
 * Use the given private authentication key to sign messages.
 */
gcry_error_t otrl_auth_handle_key(OtrlAuthInfo *auth, const char *keymsg,
	int *havemsgp, OtrlPrivKey *privkey)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp = NULL;
	size_t buflen, lenp;
	gcry_mpi_t incoming_pub = NULL;
	int res;

	*havemsgp = 0;

	res = otrl_base64_otr_decode(keymsg, &buf, &buflen);
	if (res == -1) goto memerr;
	if (res == -2) goto invval;

	bufp = buf;
	lenp = buflen;

	/* Header */
	if (memcmp(bufp, "\x00\x02\x0a", 3)) goto invval;
	bufp += 3; lenp -= 3;

	/* g^y */
	read_mpi(incoming_pub);

	if (lenp != 0) goto invval;
	free(buf);
	buf = NULL;

	switch(auth->authstate) {
	case OTRL_AUTHSTATE_AWAITING_DHKEY:
		/* Store the incoming public key */
		gcry_mpi_release(auth->their_pub);
		auth->their_pub = incoming_pub;
		incoming_pub = NULL;

		/* Compute the encryption and MAC keys */
		err = otrl_dh_compute_v2_auth_keys(&(auth->our_dh),
			auth->their_pub, auth->secure_session_id,
			&(auth->secure_session_id_len),
			&(auth->enc_c), &(auth->enc_cp),
			&(auth->mac_m1), &(auth->mac_m1p),
			&(auth->mac_m2), &(auth->mac_m2p));
		if (err) goto err;

		/* Create the Reveal Signature Message */
		err = create_revealsig_message(auth, privkey);
		if (err) goto err;
		*havemsgp = 1;
		auth->authstate = OTRL_AUTHSTATE_AWAITING_SIG;

		break;

	case OTRL_AUTHSTATE_AWAITING_SIG:
		if (gcry_mpi_cmp(incoming_pub, auth->their_pub) == 0) {
		/* Retransmit the Reveal Signature Message */
		*havemsgp = 1;
		} else {
		/* Ignore this message */
		*havemsgp = 0;
		}
		break;
	case OTRL_AUTHSTATE_NONE:
	case OTRL_AUTHSTATE_AWAITING_REVEALSIG:
	case OTRL_AUTHSTATE_V1_SETUP:
		/* Ignore this message */
		*havemsgp = 0;
		break;
	}

	gcry_mpi_release(incoming_pub);
	return err;

invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	gcry_mpi_release(incoming_pub);
	return err;
}

/*
 * Handle an incoming Reveal Signature Message.  If no error is
 * returned, and *havemsgp is 1, the message to be sent will be left in
 * auth->lastauthmsg.  Use the given private authentication key to sign
 * messages.  Call the auth_succeeded callback if authentication is
 * successful.
 */
gcry_error_t otrl_auth_handle_revealsig(OtrlAuthInfo *auth,
	const char *revealmsg, int *havemsgp, OtrlPrivKey *privkey,
	gcry_error_t (*auth_succeeded)(const OtrlAuthInfo *auth, void *asdata),
	void *asdata)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp = NULL, *gxbuf = NULL;
	unsigned char *authstart, *authend, *macstart;
	size_t buflen, lenp, rlen, authlen;
	gcry_cipher_hd_t enc = NULL;
	gcry_mpi_t incoming_pub = NULL;
	unsigned char ctr[16], hashbuf[32];
	int res;

	*havemsgp = 0;

	res = otrl_base64_otr_decode(revealmsg, &buf, &buflen);
	if (res == -1) goto memerr;
	if (res == -2) goto invval;

	bufp = buf;
	lenp = buflen;

	/* Header */
	if (memcmp(bufp, "\x00\x02\x11", 3)) goto invval;
	bufp += 3; lenp -= 3;

	/* r */
	read_int(rlen);
	if (rlen != 16) goto invval;
	require_len(rlen);
	memcpy(auth->r, bufp, rlen);
	bufp += rlen; lenp -= rlen;

	/* auth */
	authstart = bufp;
	read_int(authlen);
	require_len(authlen);
	bufp += authlen; lenp -= authlen;
	authend = bufp;

	/* MAC */
	require_len(20);
	macstart = bufp;
	bufp += 20; lenp -= 20;

	if (lenp != 0) goto invval;

	switch(auth->authstate) {
	case OTRL_AUTHSTATE_AWAITING_REVEALSIG:
		gxbuf = malloc(auth->encgx_len);
		if (auth->encgx_len && gxbuf == NULL) goto memerr;

		/* Use r to decrypt the value of g^x we received earlier */
		err = gcry_cipher_open(&enc, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CTR,
			GCRY_CIPHER_SECURE);
		if (err) goto err;

		err = gcry_cipher_setkey(enc, auth->r, 16);
		if (err) goto err;

		memset(ctr, 0, 16);
		err = gcry_cipher_setctr(enc, ctr, 16);
		if (err) goto err;

		err = gcry_cipher_decrypt(enc, gxbuf, auth->encgx_len,
			auth->encgx, auth->encgx_len);
		if (err) goto err;

		gcry_cipher_close(enc);
		enc = NULL;

		/* Check the hash */
		gcry_md_hash_buffer(GCRY_MD_SHA256, hashbuf, gxbuf,
			auth->encgx_len);
		if (memcmp(hashbuf, auth->hashgx, 32)) goto decfail;

		/* Extract g^x */
		bufp = gxbuf;
		lenp = auth->encgx_len;

		read_mpi(incoming_pub);
		free(gxbuf);
		gxbuf = NULL;

		if (lenp != 0) goto invval;

		gcry_mpi_release(auth->their_pub);
		auth->their_pub = incoming_pub;
		incoming_pub = NULL;

		/* Compute the encryption and MAC keys */
		err = otrl_dh_compute_v2_auth_keys(&(auth->our_dh),
			auth->their_pub, auth->secure_session_id,
			&(auth->secure_session_id_len),
			&(auth->enc_c), &(auth->enc_cp),
			&(auth->mac_m1), &(auth->mac_m1p),
			&(auth->mac_m2), &(auth->mac_m2p));
		if (err) goto err;

		/* Check the MAC */
		gcry_md_reset(auth->mac_m2);
		gcry_md_write(auth->mac_m2, authstart, authend - authstart);
		if (memcmp(macstart,
			gcry_md_read(auth->mac_m2, GCRY_MD_SHA256),
			20)) goto invval;

		/* Check the auth */
		err = check_pubkey_auth(auth->their_fingerprint,
			&(auth->their_keyid), authstart + 4,
			authend - authstart - 4, auth->mac_m1, auth->enc_c,
			auth->our_dh.pub, auth->their_pub);
		if (err) goto err;

		authstart = NULL;
		authend = NULL;
		macstart = NULL;
		free(buf);
		buf = NULL;

		/* Create the Signature Message */
		err = create_signature_message(auth, privkey);
		if (err) goto err;

		/* No error?  Then we've completed our end of the
		 * authentication. */
		auth->protocol_version = 2;
		auth->session_id_half = OTRL_SESSIONID_SECOND_HALF_BOLD;
		if (auth_succeeded) err = auth_succeeded(auth, asdata);
		*havemsgp = 1;
		auth->our_keyid = 0;
		auth->authstate = OTRL_AUTHSTATE_NONE;

		break;
	case OTRL_AUTHSTATE_NONE:
	case OTRL_AUTHSTATE_AWAITING_DHKEY:
	case OTRL_AUTHSTATE_AWAITING_SIG:
	case OTRL_AUTHSTATE_V1_SETUP:
		/* Ignore this message */
		*havemsgp = 0;
		free(buf);
		buf = NULL;
		break;
	}

	return err;

decfail:
	err = gcry_error(GPG_ERR_NO_ERROR);
	goto err;
invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	free(gxbuf);
	gcry_cipher_close(enc);
	gcry_mpi_release(incoming_pub);
	return err;
}

/*
 * Handle an incoming Signature Message.  If no error is returned, and
 * *havemsgp is 1, the message to be sent will be left in
 * auth->lastauthmsg.  Call the auth_succeeded callback if
 * authentication is successful.
 */
gcry_error_t otrl_auth_handle_signature(OtrlAuthInfo *auth,
	const char *sigmsg, int *havemsgp,
	gcry_error_t (*auth_succeeded)(const OtrlAuthInfo *auth, void *asdata),
	void *asdata)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp = NULL;
	unsigned char *authstart, *authend, *macstart;
	size_t buflen, lenp, authlen;
	int res;

	*havemsgp = 0;

	res = otrl_base64_otr_decode(sigmsg, &buf, &buflen);
	if (res == -1) goto memerr;
	if (res == -2) goto invval;

	bufp = buf;
	lenp = buflen;

	/* Header */
	if (memcmp(bufp, "\x00\x02\x12", 3)) goto invval;
	bufp += 3; lenp -= 3;

	/* auth */
	authstart = bufp;
	read_int(authlen);
	require_len(authlen);
	bufp += authlen; lenp -= authlen;
	authend = bufp;

	/* MAC */
	require_len(20);
	macstart = bufp;
	bufp += 20; lenp -= 20;

	if (lenp != 0) goto invval;

	switch(auth->authstate) {
	case OTRL_AUTHSTATE_AWAITING_SIG:
		/* Check the MAC */
		gcry_md_reset(auth->mac_m2p);
		gcry_md_write(auth->mac_m2p, authstart, authend - authstart);
		if (memcmp(macstart,
			gcry_md_read(auth->mac_m2p, GCRY_MD_SHA256),
			20)) goto invval;

		/* Check the auth */
		err = check_pubkey_auth(auth->their_fingerprint,
			&(auth->their_keyid), authstart + 4,
			authend - authstart - 4, auth->mac_m1p, auth->enc_cp,
			auth->our_dh.pub, auth->their_pub);
		if (err) goto err;

		authstart = NULL;
		authend = NULL;
		macstart = NULL;
		free(buf);
		buf = NULL;

		/* No error?  Then we've completed our end of the
		 * authentication. */
		auth->protocol_version = 2;
		auth->session_id_half = OTRL_SESSIONID_FIRST_HALF_BOLD;
		if (auth_succeeded) err = auth_succeeded(auth, asdata);
		free(auth->lastauthmsg);
		auth->lastauthmsg = NULL;
		*havemsgp = 1;
		auth->our_keyid = 0;
		auth->authstate = OTRL_AUTHSTATE_NONE;

		break;
	case OTRL_AUTHSTATE_NONE:
	case OTRL_AUTHSTATE_AWAITING_DHKEY:
	case OTRL_AUTHSTATE_AWAITING_REVEALSIG:
	case OTRL_AUTHSTATE_V1_SETUP:
		/* Ignore this message */
		*havemsgp = 0;
		break;
	}

	return err;

invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	return err;
}

/* Version 1 routines, for compatibility */

/*
 * Create a verion 1 Key Exchange Message using the values in the given
 * auth, and store it in auth->lastauthmsg.  Set the Reply field to the
 * given value, and use the given privkey to sign the message.
 */
static gcry_error_t create_v1_key_exchange_message(OtrlAuthInfo *auth,
	unsigned char reply, OtrlPrivKey *privkey)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	const enum gcry_mpi_format format = GCRYMPI_FMT_USG;
	unsigned char *buf = NULL, *bufp = NULL, *sigbuf = NULL;
	size_t lenp, ourpublen, totallen, siglen;
	unsigned char hashbuf[20];

	if (privkey->pubkey_type != OTRL_PUBKEY_TYPE_DSA) {
	return gpg_error(GPG_ERR_INV_VALUE);
	}

	/* How big is the DH public key? */
	gcry_mpi_print(format, NULL, 0, &ourpublen, auth->our_dh.pub);

	totallen = 3 + 1 + privkey->pubkey_datalen + 4 + 4 + ourpublen + 40;
	buf = malloc(totallen);
	if (buf == NULL) goto memerr;

	bufp = buf;
	lenp = totallen;

	memcpy(bufp, "\x00\x01\x0a", 3); /* header */
	debug_data("Header", bufp, 3);
	bufp += 3; lenp -= 3;

	bufp[0] = reply;
	debug_data("Reply", bufp, 1);
	bufp += 1; lenp -= 1;

	memcpy(bufp, privkey->pubkey_data, privkey->pubkey_datalen);
	debug_data("Pubkey", bufp, privkey->pubkey_datalen);
	bufp += privkey->pubkey_datalen; lenp -= privkey->pubkey_datalen;

	write_int(auth->our_keyid);
	debug_int("Keyid", bufp-4);

	write_mpi(auth->our_dh.pub, ourpublen, "D-H y");

	/* Hash all the data written so far, and sign the hash */
	gcry_md_hash_buffer(GCRY_MD_SHA1, hashbuf, buf, bufp - buf);

	err = otrl_privkey_sign(&sigbuf, &siglen, privkey, hashbuf, 20);
	if (err) goto err;

	if (siglen != 40) goto invval;
	memcpy(bufp, sigbuf, 40);
	debug_data("Signature", bufp, 40);
	bufp += 40; lenp -= 40;
	free(sigbuf);
	sigbuf = NULL;

	assert(lenp == 0);

	free(auth->lastauthmsg);
	auth->lastauthmsg = otrl_base64_otr_encode(buf, totallen);
	if (auth->lastauthmsg == NULL) goto memerr;
	free(buf);
	buf = NULL;

	return err;

invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	free(sigbuf);
	return err;
}

/*
 * Start a fresh AKE (version 1) using the given OtrlAuthInfo.  If
 * our_dh is NULL, generate a fresh DH keypair to use.  Otherwise, use a
 * copy of the one passed (with the given keyid).  Use the given private
 * key to sign the message.  If no error is returned, the message to
 * transmit will be contained in auth->lastauthmsg.
 */
gcry_error_t otrl_auth_start_v1(OtrlAuthInfo *auth, DH_keypair *our_dh,
	unsigned int our_keyid, OtrlPrivKey *privkey)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);

	/* Clear out this OtrlAuthInfo and start over */
	otrl_auth_clear(auth);
	auth->initiated = 1;

	/* Import the given DH keypair, or else create a fresh one */
	if (our_dh) {
	otrl_dh_keypair_copy(&(auth->our_dh), our_dh);
	auth->our_keyid = our_keyid;
	} else {
	otrl_dh_gen_keypair(DH1536_GROUP_ID, &(auth->our_dh));
	auth->our_keyid = 1;
	}

	err = create_v1_key_exchange_message(auth, 0, privkey);
	if (!err) {
	auth->authstate = OTRL_AUTHSTATE_V1_SETUP;
	}

	return err;
}

/*
 * Handle an incoming v1 Key Exchange Message.  If no error is returned,
 * and *havemsgp is 1, the message to be sent will be left in
 * auth->lastauthmsg.  Use the given private authentication key to sign
 * messages.  Call the auth_secceeded callback if authentication is
 * successful.  If non-NULL, use a copy of the given D-H keypair, with
 * the given keyid.
 */
gcry_error_t otrl_auth_handle_v1_key_exchange(OtrlAuthInfo *auth,
	const char *keyexchmsg, int *havemsgp, OtrlPrivKey *privkey,
	DH_keypair *our_dh, unsigned int our_keyid,
	gcry_error_t (*auth_succeeded)(const OtrlAuthInfo *auth, void *asdata),
	void *asdata)
{
	gcry_error_t err = gcry_error(GPG_ERR_NO_ERROR);
	unsigned char *buf = NULL, *bufp = NULL;
	unsigned char *fingerprintstart, *fingerprintend;
	unsigned char fingerprintbuf[20], hashbuf[20];
	gcry_mpi_t p, q, g, y, received_pub = NULL;
	gcry_sexp_t pubs = NULL;
	size_t buflen, lenp;
	unsigned char received_reply;
	unsigned int received_keyid;
	int res;

	*havemsgp = 0;

	res = otrl_base64_otr_decode(keyexchmsg, &buf, &buflen);
	if (res == -1) goto memerr;
	if (res == -2) goto invval;

	bufp = buf;
	lenp = buflen;

	/* Header */
	require_len(3);
	if (memcmp(bufp, "\x00\x01\x0a", 3)) goto invval;
	bufp += 3; lenp -= 3;

	/* Reply */
	require_len(1);
	received_reply = bufp[0];
	bufp += 1; lenp -= 1;

	/* Public Key */
	fingerprintstart = bufp;
	read_mpi(p);
	read_mpi(q);
	read_mpi(g);
	read_mpi(y);
	fingerprintend = bufp;
	gcry_md_hash_buffer(GCRY_MD_SHA1, fingerprintbuf,
		fingerprintstart, fingerprintend-fingerprintstart);
	gcry_sexp_build(&pubs, NULL,
	"(public-key (dsa (p %m)(q %m)(g %m)(y %m)))", p, q, g, y);
	gcry_mpi_release(p);
	gcry_mpi_release(q);
	gcry_mpi_release(g);
	gcry_mpi_release(y);

	/* keyid */
	read_int(received_keyid);
	if (received_keyid == 0) goto invval;

	/* D-H pubkey */
	read_mpi(received_pub);

	/* Verify the signature */
	if (lenp != 40) goto invval;
	gcry_md_hash_buffer(GCRY_MD_SHA1, hashbuf, buf, bufp - buf);
	err = otrl_privkey_verify(bufp, lenp, OTRL_PUBKEY_TYPE_DSA,
		pubs, hashbuf, 20);
	if (err) goto err;
	gcry_sexp_release(pubs);
	pubs = NULL;
	free(buf);
	buf = NULL;
	
	if (auth->authstate != OTRL_AUTHSTATE_V1_SETUP && received_reply == 0x01) {
	/* They're replying to something we never sent.  We must be
	 * logged in more than once; ignore the message. */
	err = gpg_error(GPG_ERR_NO_ERROR);
	goto err;
	}

	if (auth->authstate != OTRL_AUTHSTATE_V1_SETUP) {
	/* Clear the auth and start over */
	otrl_auth_clear(auth);
	}

	/* Everything checked out */
	auth->their_keyid = received_keyid;
	gcry_mpi_release(auth->their_pub);
	auth->their_pub = received_pub;
	received_pub = NULL;
	memcpy(auth->their_fingerprint, fingerprintbuf, 20);

	if (received_reply == 0x01) {
	/* Don't send a reply to this. */
	*havemsgp = 0;
	} else {
	/* Import the given DH keypair, or else create a fresh one */
	if (our_dh) {
		otrl_dh_keypair_copy(&(auth->our_dh), our_dh);
		auth->our_keyid = our_keyid;
	} else if (auth->our_keyid == 0) {
		otrl_dh_gen_keypair(DH1536_GROUP_ID, &(auth->our_dh));
		auth->our_keyid = 1;
	}

	/* Reply with our own Key Exchange Message */
	err = create_v1_key_exchange_message(auth, 1, privkey);
	if (err) goto err;
	*havemsgp = 1;
	}

	/* Compute the session id */
	err = otrl_dh_compute_v1_session_id(&(auth->our_dh),
		auth->their_pub, auth->secure_session_id,
		&(auth->secure_session_id_len),
		&(auth->session_id_half));
	if (err) goto err;

	/* We've completed our end of the authentication */
	auth->protocol_version = 1;
	if (auth_succeeded) err = auth_succeeded(auth, asdata);
	auth->our_keyid = 0;
	auth->authstate = OTRL_AUTHSTATE_NONE;

	return err;

invval:
	err = gcry_error(GPG_ERR_INV_VALUE);
	goto err;
memerr:
	err = gcry_error(GPG_ERR_ENOMEM);
err:
	free(buf);
	gcry_sexp_release(pubs);
	gcry_mpi_release(received_pub);
	return err;
}

#ifdef OTRL_TESTING_AUTH
#include "mem.h"
#include "privkey.h"

#define CHECK_ERR if (err) { printf("Error: %s\n", gcry_strerror(err)); return 1; }

static gcry_error_t starting(const OtrlAuthInfo *auth, void *asdata)
{
	char *name = asdata;

	fprintf(stderr, "\nStarting ENCRYPTED mode for %s (v%d).\n", name, auth->protocol_version);

	fprintf(stderr, "\nour_dh (%d):", auth->our_keyid);
	gcry_mpi_dump(auth->our_dh.pub);
	fprintf(stderr, "\ntheir_pub (%d):", auth->their_keyid);
	gcry_mpi_dump(auth->their_pub);

	debug_data("\nTheir fingerprint", auth->their_fingerprint, 20);
	debug_data("\nSecure session id", auth->secure_session_id,
		auth->secure_session_id_len);
	fprintf(stderr, "Sessionid half: %d\n\n", auth->session_id_half);

	return gpg_error(GPG_ERR_NO_ERROR);
}

int main(int argc, char **argv)
{
	OtrlAuthInfo alice, bob;
	gcry_error_t err;
	int havemsg;
	OtrlUserState us;
	OtrlPrivKey *alicepriv, *bobpriv;

	otrl_mem_init();
	otrl_dh_init();
	otrl_auth_new(&alice);
	otrl_auth_new(&bob);

	us = otrl_userstate_create();
	otrl_privkey_read(us, "/home/iang/.gaim/otr.private_key");
	alicepriv = otrl_privkey_find(us, "oneeyedian", "prpl-oscar");
	bobpriv = otrl_privkey_find(us, "otr4ian", "prpl-oscar");

	printf("\n\n  ***** V2 *****\n\n");

	err = otrl_auth_start_v2(&bob, NULL, 0);
	CHECK_ERR
	printf("\nBob: %d\n%s\n\n", strlen(bob.lastauthmsg), bob.lastauthmsg);
	err = otrl_auth_handle_commit(&alice, bob.lastauthmsg, NULL, 0);
	CHECK_ERR
	printf("\nAlice: %d\n%s\n\n", strlen(alice.lastauthmsg), alice.lastauthmsg);
	err = otrl_auth_handle_key(&bob, alice.lastauthmsg, &havemsg, bobpriv);
	CHECK_ERR
	if (havemsg) {
	printf("\nBob: %d\n%s\n\n", strlen(bob.lastauthmsg), bob.lastauthmsg);
	} else {
	printf("\nIGNORE\n\n");
	}
	err = otrl_auth_handle_revealsig(&alice, bob.lastauthmsg, &havemsg,
		alicepriv, starting, "Alice");
	CHECK_ERR
	if (havemsg) {
	printf("\nAlice: %d\n%s\n\n", strlen(alice.lastauthmsg), alice.lastauthmsg);
	} else {
	printf("\nIGNORE\n\n");
	}
	err = otrl_auth_handle_signature(&bob, alice.lastauthmsg, &havemsg,
		starting, "Bob");
	CHECK_ERR

	printf("\n\n  ***** V1 *****\n\n");

	err = otrl_auth_start_v1(&bob, NULL, 0, bobpriv);
	CHECK_ERR
	printf("\nBob: %d\n%s\n\n", strlen(bob.lastauthmsg), bob.lastauthmsg);
	err = otrl_auth_handle_v1_key_exchange(&alice, bob.lastauthmsg,
		&havemsg, alicepriv, NULL, 0, starting, "Alice");
	CHECK_ERR
	if (havemsg) {
	printf("\nAlice: %d\n%s\n\n", strlen(alice.lastauthmsg), alice.lastauthmsg);
	} else {
	printf("\nIGNORE\n\n");
	}
	err = otrl_auth_handle_v1_key_exchange(&bob, alice.lastauthmsg,
		&havemsg, bobpriv, NULL, 0, starting, "Bob");
	CHECK_ERR
	if (havemsg) {
	printf("\nBob: %d\n%s\n\n", strlen(bob.lastauthmsg), bob.lastauthmsg);
	} else {
	printf("\nIGNORE\n\n");
	}

	otrl_userstate_free(us);
	otrl_auth_clear(&alice);
	otrl_auth_clear(&bob);
	return 0;
}
#endif
