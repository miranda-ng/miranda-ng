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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "privkey.h"
#include "serial.h"

/* Convert a 20-byte hash value to a 45-byte human-readable value */
void otrl_privkey_hash_to_human(char human[45], const unsigned char hash[20])
{
	int word, byte;
	char *p = human;

	for(word=0; word<5; ++word) {
	for(byte=0; byte<4; ++byte) {
		sprintf(p, "%02X", hash[word*4+byte]);
		p += 2;
	}
	*(p++) = ' ';
	}
	/* Change that last ' ' to a '\0' */
	--p;
	*p = '\0';
}

/* Calculate a human-readable hash of our DSA public key.  Return it in
 * the passed fingerprint buffer.  Return NULL on error, or a pointer to
 * the given buffer on success. */
char *otrl_privkey_fingerprint(OtrlUserState us, char fingerprint[45],
	const char *accountname, const char *protocol)
{
	unsigned char hash[20];
	OtrlPrivKey *p = otrl_privkey_find(us, accountname, protocol);

	if (p) {
	/* Calculate the hash */
	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, p->pubkey_data,
		p->pubkey_datalen);

	/* Now convert it to a human-readable format */
	otrl_privkey_hash_to_human(fingerprint, hash);
	} else {
	return NULL;
	}

	return fingerprint;
}

/* Calculate a raw hash of our DSA public key.  Return it in the passed
 * fingerprint buffer.  Return NULL on error, or a pointer to the given
 * buffer on success. */
unsigned char *otrl_privkey_fingerprint_raw(OtrlUserState us,
	unsigned char hash[20], const char *accountname, const char *protocol)
{
	OtrlPrivKey *p = otrl_privkey_find(us, accountname, protocol);

	if (p) {
	/* Calculate the hash */
	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, p->pubkey_data,
		p->pubkey_datalen);
	} else {
	return NULL;
	}

	return hash;
}

/* Create a public key block from a private key */
static gcry_error_t make_pubkey(unsigned char **pubbufp, size_t *publenp,
	gcry_sexp_t privkey)
{
	gcry_mpi_t p,q,g,y;
	gcry_sexp_t dsas,ps,qs,gs,ys;
	size_t np,nq,ng,ny;
	enum gcry_mpi_format format = GCRYMPI_FMT_USG;
	unsigned char *bufp;
	size_t lenp;

	*pubbufp = NULL;
	*publenp = 0;

	/* Extract the public parameters */
	dsas = gcry_sexp_find_token(privkey, "dsa", 0);
	if (dsas == NULL) {
	return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	ps = gcry_sexp_find_token(dsas, "p", 0);
	qs = gcry_sexp_find_token(dsas, "q", 0);
	gs = gcry_sexp_find_token(dsas, "g", 0);
	ys = gcry_sexp_find_token(dsas, "y", 0);
	gcry_sexp_release(dsas);
	if (!ps || !qs || !gs || !ys) {
	gcry_sexp_release(ps);
	gcry_sexp_release(qs);
	gcry_sexp_release(gs);
	gcry_sexp_release(ys);
	return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	p = gcry_sexp_nth_mpi(ps, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(ps);
	q = gcry_sexp_nth_mpi(qs, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(qs);
	g = gcry_sexp_nth_mpi(gs, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(gs);
	y = gcry_sexp_nth_mpi(ys, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(ys);
	if (!p || !q || !g || !y) {
	gcry_mpi_release(p);
	gcry_mpi_release(q);
	gcry_mpi_release(g);
	gcry_mpi_release(y);
	return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}

	*publenp = 0;
	gcry_mpi_print(format, NULL, 0, &np, p);
	*publenp += np + 4;
	gcry_mpi_print(format, NULL, 0, &nq, q);
	*publenp += nq + 4;
	gcry_mpi_print(format, NULL, 0, &ng, g);
	*publenp += ng + 4;
	gcry_mpi_print(format, NULL, 0, &ny, y);
	*publenp += ny + 4;

	*pubbufp = malloc(*publenp);
	if (*pubbufp == NULL) {
	gcry_mpi_release(p);
	gcry_mpi_release(q);
	gcry_mpi_release(g);
	gcry_mpi_release(y);
	return gcry_error(GPG_ERR_ENOMEM);
	}
	bufp = *pubbufp;
	lenp = *publenp;

	write_mpi(p,np,"P");
	write_mpi(q,nq,"Q");
	write_mpi(g,ng,"G");
	write_mpi(y,ny,"Y");

	gcry_mpi_release(p);
	gcry_mpi_release(q);
	gcry_mpi_release(g);
	gcry_mpi_release(y);

	return gcry_error(GPG_ERR_NO_ERROR);
}

/* Read a sets of private DSA keys from a file on disk into the given
 * OtrlUserState. */
gcry_error_t otrl_privkey_read(OtrlUserState us, const char *filename)
{
	FILE *privf;
	gcry_error_t err;

	/* Open the privkey file.  We use rb mode so that on WIN32, fread()
	 * reads the same number of bytes that fstat() indicates are in the
	 * file. */
	privf = fopen(filename, "rb");
	if (!privf) {
	err = gcry_error_from_errno(errno);
	return err;
	}

	err = otrl_privkey_read_FILEp(us, privf);

	fclose(privf);
	return err;
}

/* Read a sets of private DSA keys from a FILE* into the given
 * OtrlUserState.  The FILE* must be open for reading. */
gcry_error_t otrl_privkey_read_FILEp(OtrlUserState us, FILE *privf)
{
	int privfd;
	struct stat st;
	char *buf;
	const char *token;
	size_t tokenlen;
	gcry_error_t err;
	gcry_sexp_t allkeys;
	size_t i;

	if (!privf) return gcry_error(GPG_ERR_NO_ERROR);

	/* Release any old ideas we had about our keys */
	otrl_privkey_forget_all(us);

	/* Load the data into a buffer */
	privfd = _fileno(privf);
	if (fstat(privfd, &st)) {
	err = gcry_error_from_errno(errno);
	return err;
	}
	buf = malloc(st.st_size);
	if (!buf && st.st_size > 0) {
	return gcry_error(GPG_ERR_ENOMEM);
	}
	if (fread(buf, st.st_size, 1, privf) != 1) {
	err = gcry_error_from_errno(errno);
	free(buf);
	return err;
	}

	err = gcry_sexp_new(&allkeys, buf, st.st_size, 0);
	free(buf);
	if (err) {
	return err;
	}

	token = gcry_sexp_nth_data(allkeys, 0, &tokenlen);
	if (tokenlen != 8 || strncmp(token, "privkeys", 8)) {
	gcry_sexp_release(allkeys);
	return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}

	/* Get each account */
	for(i=1; i<gcry_sexp_length(allkeys); ++i) {
	gcry_sexp_t names, protos, privs;
	char *name, *proto;
	gcry_sexp_t accounts;
	OtrlPrivKey *p;
	
	/* Get the ith "account" S-exp */
	accounts = gcry_sexp_nth(allkeys, i);
	
	/* It's really an "account" S-exp? */
	token = gcry_sexp_nth_data(accounts, 0, &tokenlen);
	if (tokenlen != 7 || strncmp(token, "account", 7)) {
		gcry_sexp_release(accounts);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	/* Extract the name, protocol, and privkey S-exps */
	names = gcry_sexp_find_token(accounts, "name", 0);
	protos = gcry_sexp_find_token(accounts, "protocol", 0);
	privs = gcry_sexp_find_token(accounts, "private-key", 0);
	gcry_sexp_release(accounts);
	if (!names || !protos || !privs) {
		gcry_sexp_release(names);
		gcry_sexp_release(protos);
		gcry_sexp_release(privs);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	/* Extract the actual name and protocol */
	token = gcry_sexp_nth_data(names, 1, &tokenlen);
	if (!token) {
		gcry_sexp_release(names);
		gcry_sexp_release(protos);
		gcry_sexp_release(privs);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	name = malloc(tokenlen + 1);
	if (!name) {
		gcry_sexp_release(names);
		gcry_sexp_release(protos);
		gcry_sexp_release(privs);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_ENOMEM);
	}
	memmove(name, token, tokenlen);
	name[tokenlen] = '\0';
	gcry_sexp_release(names);

	token = gcry_sexp_nth_data(protos, 1, &tokenlen);
	if (!token) {
		free(name);
		gcry_sexp_release(protos);
		gcry_sexp_release(privs);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	proto = malloc(tokenlen + 1);
	if (!proto) {
		free(name);
		gcry_sexp_release(protos);
		gcry_sexp_release(privs);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_ENOMEM);
	}
	memmove(proto, token, tokenlen);
	proto[tokenlen] = '\0';
	gcry_sexp_release(protos);

	/* Make a new OtrlPrivKey entry */
	p = malloc(sizeof(*p));
	if (!p) {
		free(name);
		free(proto);
		gcry_sexp_release(privs);
		gcry_sexp_release(allkeys);
		return gcry_error(GPG_ERR_ENOMEM);
	}

	/* Fill it in and link it up */
	p->accountname = name;
	p->protocol = proto;
	p->pubkey_type = OTRL_PUBKEY_TYPE_DSA;
	p->privkey = privs;
	p->next = us->privkey_root;
	if (p->next) {
		p->next->tous = &(p->next);
	}
	p->tous = &(us->privkey_root);
	us->privkey_root = p;
	err = make_pubkey(&(p->pubkey_data), &(p->pubkey_datalen), p->privkey);
	if (err) {
		gcry_sexp_release(allkeys);
		otrl_privkey_forget(p);
		return gcry_error(GPG_ERR_UNUSABLE_SECKEY);
	}
	}
	gcry_sexp_release(allkeys);

	return gcry_error(GPG_ERR_NO_ERROR);
}

static gcry_error_t sexp_write(FILE *privf, gcry_sexp_t sexp)
{
	size_t buflen;
	char *buf;

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

static gcry_error_t account_write(FILE *privf, const char *accountname,
	const char *protocol, gcry_sexp_t privkey)
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

/* Generate a private DSA key for a given account, storing it into a
 * file on disk, and loading it into the given OtrlUserState.  Overwrite any
 * previously generated keys for that account in that OtrlUserState. */
gcry_error_t otrl_privkey_generate(OtrlUserState us, const char *filename,
	const char *accountname, const char *protocol)
{
	gcry_error_t err;
	FILE *privf;
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

	err = otrl_privkey_generate_FILEp(us, privf, accountname, protocol);

	fclose(privf);
#ifndef WIN32
	umask(oldmask);
#endif
	return err;
}

/* Generate a private DSA key for a given account, storing it into a
 * FILE*, and loading it into the given OtrlUserState.  Overwrite any
 * previously generated keys for that account in that OtrlUserState.
 * The FILE* must be open for reading and writing. */
gcry_error_t otrl_privkey_generate_FILEp(OtrlUserState us, FILE *privf,
	const char *accountname, const char *protocol)
{
	gcry_error_t err;
	gcry_sexp_t key, parms, privkey;
	static const char *parmstr = "(genkey (dsa (nbits 4:1024)))";
	OtrlPrivKey *p;

	if (!privf) return gcry_error(GPG_ERR_NO_ERROR);

	/* Create a DSA key */
	err = gcry_sexp_new(&parms, parmstr, strlen(parmstr), 0);
	if (err) {
	return err;
	}
	err = gcry_pk_genkey(&key, parms);
	gcry_sexp_release(parms);
	if (err) {
	return err;
	}

	/* Extract the privkey */
	privkey = gcry_sexp_find_token(key, "private-key", 0);
	gcry_sexp_release(key);

	/* Output the other keys we know */
	fprintf(privf, "(privkeys\n");

	for (p=us->privkey_root; p; p=p->next) {
	/* Skip this one if our new key replaces it */
	if (!strcmp(p->accountname, accountname) &&
		!strcmp(p->protocol, protocol)) {
		continue;
	}

	account_write(privf, p->accountname, p->protocol, p->privkey);
	}
	account_write(privf, accountname, protocol, privkey);
	gcry_sexp_release(privkey);
	fprintf(privf, ")\n");

	fseek(privf, 0, SEEK_SET);

	return otrl_privkey_read_FILEp(us, privf);
}

/* Convert a hex character to a value */
static unsigned int ctoh(char c)
{
	if (c >= '0' && c <= '9') return c-'0';
	if (c >= 'a' && c <= 'f') return c-'a'+10;
	if (c >= 'A' && c <= 'F') return c-'A'+10;
	return 0;  /* Unknown hex char */
}

/* Read the fingerprint store from a file on disk into the given
 * OtrlUserState.  Use add_app_data to add application data to each
 * ConnContext so created. */
gcry_error_t otrl_privkey_read_fingerprints(OtrlUserState us,
	const char *filename,
	void (*add_app_data)(void *data, ConnContext *context),
	void  *data)
{
	gcry_error_t err;
	FILE *storef;

	storef = fopen(filename, "rb");
	if (!storef) {
	err = gcry_error_from_errno(errno);
	return err;
	}

	err = otrl_privkey_read_fingerprints_FILEp(us, storef, add_app_data, data);

	fclose(storef);
	return err;
}

/* Read the fingerprint store from a FILE* into the given
 * OtrlUserState.  Use add_app_data to add application data to each
 * ConnContext so created.  The FILE* must be open for reading. */
gcry_error_t otrl_privkey_read_fingerprints_FILEp(OtrlUserState us,
	FILE *storef,
	void (*add_app_data)(void *data, ConnContext *context),
	void  *data)
{
	ConnContext *context;
	char storeline[1000];
	unsigned char fingerprint[20];
	size_t maxsize = sizeof(storeline);

	if (!storef) return gcry_error(GPG_ERR_NO_ERROR);

	while(fgets(storeline, maxsize, storef)) {
	char *username;
	char *accountname;
	char *protocol;
	char *hex;
	char *trust;
	char *tab;
	char *eol;
	Fingerprint *fng;
	int i, j;
	/* Parse the line, which should be of the form:
	 *    username\taccountname\tprotocol\t40_hex_nybbles\n          */
	username = storeline;
	tab = strchr(username, '\t');
	if (!tab) continue;
	*tab = '\0';

	accountname = tab + 1;
	tab = strchr(accountname, '\t');
	if (!tab) continue;
	*tab = '\0';

	protocol = tab + 1;
	tab = strchr(protocol, '\t');
	if (!tab) continue;
	*tab = '\0';

	hex = tab + 1;
	tab = strchr(hex, '\t');
	if (!tab) {
		eol = strchr(hex, '\r');
		if (!eol) eol = strchr(hex, '\n');
		if (!eol) continue;
		*eol = '\0';
		trust = NULL;
	} else {
		*tab = '\0';
		trust = tab + 1;
		eol = strchr(trust, '\r');
		if (!eol) eol = strchr(trust, '\n');
		if (!eol) continue;
		*eol = '\0';
	}

	if (strlen(hex) != 40) continue;
	for(j=0, i=0; i<40; i+=2) {
		fingerprint[j++] = (ctoh(hex[i]) << 4) + (ctoh(hex[i+1]));
	}
	/* Get the context for this user, adding if not yet present */
	context = otrl_context_find(us, username, accountname, protocol,
		1, NULL, add_app_data, data);
	/* Add the fingerprint if not already there */
	fng = otrl_context_find_fingerprint(context, fingerprint, 1, NULL);
	otrl_context_set_trust(fng, trust);
	}

	return gcry_error(GPG_ERR_NO_ERROR);
}

/* Write the fingerprint store from a given OtrlUserState to a file on disk. */
gcry_error_t otrl_privkey_write_fingerprints(OtrlUserState us,
	const char *filename)
{
	gcry_error_t err;
	FILE *storef;

	storef = fopen(filename, "wb");
	if (!storef) {
	err = gcry_error_from_errno(errno);
	return err;
	}

	err = otrl_privkey_write_fingerprints_FILEp(us, storef);

	fclose(storef);
	return err;
}

/* Write the fingerprint store from a given OtrlUserState to a FILE*.
 * The FILE* must be open for writing. */
gcry_error_t otrl_privkey_write_fingerprints_FILEp(OtrlUserState us,
	FILE *storef)
{
	ConnContext *context;
	Fingerprint *fprint;

	if (!storef) return gcry_error(GPG_ERR_NO_ERROR);

	for(context = us->context_root; context; context = context->next) {
	/* Don't both with the first (fingerprintless) entry. */
	for (fprint = context->fingerprint_root.next; fprint;
		fprint = fprint->next) {
		int i;
		fprintf(storef, "%s\t%s\t%s\t", context->username,
			context->accountname, context->protocol);
		for(i=0;i<20;++i) {
		fprintf(storef, "%02x", fprint->fingerprint[i]);
		}
		fprintf(storef, "\t%s\n", fprint->trust ? fprint->trust : "");
	}
	}

	return gcry_error(GPG_ERR_NO_ERROR);
}

/* Fetch the private key from the given OtrlUserState associated with
 * the given account */
OtrlPrivKey *otrl_privkey_find(OtrlUserState us, const char *accountname,
	const char *protocol)
{
	OtrlPrivKey *p;
	if (!accountname || !protocol) return NULL;

	for(p=us->privkey_root; p; p=p->next) {
	if (!strcmp(p->accountname, accountname) &&
		!strcmp(p->protocol, protocol)) {
		return p;
	}
	}
	return NULL;
}

/* Forget a private key */
void otrl_privkey_forget(OtrlPrivKey *privkey)
{
	free(privkey->accountname);
	free(privkey->protocol);
	gcry_sexp_release(privkey->privkey);
	free(privkey->pubkey_data);

	/* Re-link the list */
	*(privkey->tous) = privkey->next;
	if (privkey->next) {
	privkey->next->tous = privkey->tous;
	}

	/* Free the privkey struct */
	free(privkey);
}

/* Forget all private keys in a given OtrlUserState. */
void otrl_privkey_forget_all(OtrlUserState us)
{
	while (us->privkey_root) {
	otrl_privkey_forget(us->privkey_root);
	}
}

/* Sign data using a private key.  The data must be small enough to be
 * signed (i.e. already hashed, if necessary).  The signature will be
 * returned in *sigp, which the caller must free().  Its length will be
 * returned in *siglenp. */
gcry_error_t otrl_privkey_sign(unsigned char **sigp, size_t *siglenp,
	OtrlPrivKey *privkey, const unsigned char *data, size_t len)
{
	gcry_mpi_t r,s, datampi;
	gcry_sexp_t dsas, rs, ss, sigs, datas;
	size_t nr, ns;
	const enum gcry_mpi_format format = GCRYMPI_FMT_USG;

	if (privkey->pubkey_type != OTRL_PUBKEY_TYPE_DSA)
	return gcry_error(GPG_ERR_INV_VALUE);

	*sigp = malloc(40);
	if (sigp == NULL) return gcry_error(GPG_ERR_ENOMEM);
	*siglenp = 40;

	if (len) {
	gcry_mpi_scan(&datampi, GCRYMPI_FMT_USG, data, len, NULL);
	} else {
	datampi = gcry_mpi_set_ui(NULL, 0);
	}
	gcry_sexp_build(&datas, NULL, "(%m)", datampi);
	gcry_mpi_release(datampi);
	gcry_pk_sign(&sigs, datas, privkey->privkey);
	gcry_sexp_release(datas);
	dsas = gcry_sexp_find_token(sigs, "dsa", 0);
	gcry_sexp_release(sigs);
	rs = gcry_sexp_find_token(dsas, "r", 0);
	ss = gcry_sexp_find_token(dsas, "s", 0);
	gcry_sexp_release(dsas);
	r = gcry_sexp_nth_mpi(rs, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(rs);
	s = gcry_sexp_nth_mpi(ss, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(ss);
	gcry_mpi_print(format, NULL, 0, &nr, r);
	gcry_mpi_print(format, NULL, 0, &ns, s);
	memset(*sigp, 0, 40);
	gcry_mpi_print(format, (*sigp)+(20-nr), nr, NULL, r);
	gcry_mpi_print(format, (*sigp)+20+(20-ns), ns, NULL, s);
	gcry_mpi_release(r);
	gcry_mpi_release(s);

	return gcry_error(GPG_ERR_NO_ERROR);
}

/* Verify a signature on data using a public key.  The data must be
 * small enough to be signed (i.e. already hashed, if necessary). */
gcry_error_t otrl_privkey_verify(const unsigned char *sigbuf, size_t siglen,
	unsigned short pubkey_type, gcry_sexp_t pubs,
	const unsigned char *data, size_t len)
{
	gcry_error_t err;
	gcry_mpi_t datampi,r,s;
	gcry_sexp_t datas, sigs;

	if (pubkey_type != OTRL_PUBKEY_TYPE_DSA || siglen != 40)
	return gcry_error(GPG_ERR_INV_VALUE);

	if (len) {
	gcry_mpi_scan(&datampi, GCRYMPI_FMT_USG, data, len, NULL);
	} else {
	datampi = gcry_mpi_set_ui(NULL, 0);
	}
	gcry_sexp_build(&datas, NULL, "(%m)", datampi);
	gcry_mpi_release(datampi);
	gcry_mpi_scan(&r, GCRYMPI_FMT_USG, sigbuf, 20, NULL);
	gcry_mpi_scan(&s, GCRYMPI_FMT_USG, sigbuf+20, 20, NULL);
	gcry_sexp_build(&sigs, NULL, "(sig-val (dsa (r %m)(s %m)))", r, s);
	gcry_mpi_release(r);
	gcry_mpi_release(s);

	err = gcry_pk_verify(sigs, datas, pubs);
	gcry_sexp_release(datas);
	gcry_sexp_release(sigs);

	return err;
}
gcry_error_t otrl_account_write(FILE *privf, const char *accountname,
									   const char *protocol, gcry_sexp_t privkey) {
	   return account_write(privf, accountname, protocol, privkey);
}