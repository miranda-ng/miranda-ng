/*
 * hash algorithms used in OAuth 
 *
 * Copyright 2007-2010 Robin Gareus <robin@gareus.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if USE_BUILTIN_HASH // built-in / AVR -- TODO: check license of sha1.c
#include <stdio.h>
#include "oauth.h" // oauth_encode_base64
#include "xmalloc.h"

#include "sha1.c" // TODO: sha1.h ; Makefile.am: add sha1.c

/* API */
char *oauth_sign_hmac_sha1_raw (const char *m, const size_t ml, const char *k, const size_t kl) {
	sha1nfo s;
	sha1_initHmac(&s, (const uint8_t*) k, kl);
	sha1_write(&s, m, ml);
	unsigned char *digest = sha1_resultHmac(&s);
  return oauth_encode_base64(HASH_LENGTH, digest);
}

char *oauth_sign_hmac_sha1 (const char *m, const char *k) {
  return(oauth_sign_hmac_sha1_raw (m, strlen(m), k, strlen(k)));
}

char *oauth_body_hash_file(char *filename) {
  FILE *F= fopen(filename, "r");
  if (!F) return NULL;

  size_t len=0;
  char fb[BUFSIZ];
	sha1nfo s;
	sha1_init(&s);

  while (!feof(F) && (len=fread(fb,sizeof(char),BUFSIZ, F))>0) {
		sha1_write(&s, fb, len);
	}
  fclose(F);

  unsigned char *dgst = xmalloc(HASH_LENGTH*sizeof(char)); // oauth_body_hash_encode frees the digest..
  memcpy(dgst, sha1_result(&s), HASH_LENGTH);
  return oauth_body_hash_encode(HASH_LENGTH, dgst);
}

char *oauth_body_hash_data(size_t length, const char *data) {
	sha1nfo s;
	sha1_init(&s);
	for (;length--;) sha1_writebyte(&s, *data++);

  unsigned char *dgst = xmalloc(HASH_LENGTH*sizeof(char)); // oauth_body_hash_encode frees the digest..
  memcpy(dgst, sha1_result(&s), HASH_LENGTH);
  return oauth_body_hash_encode(HASH_LENGTH, dgst);
}

char *oauth_sign_rsa_sha1 (const char *m, const char *k) {
	/* NOT RSA/PK11 support */
	return xstrdup("---RSA/PK11-is-not-supported-by-this-version-of-liboauth---");
}

int oauth_verify_rsa_sha1 (const char *m, const char *c, const char *sig) {
	/* NOT RSA/PK11 support */
	return -1; // mismatch , error
}

#elif defined (USE_NSS)
/* use http://www.mozilla.org/projects/security/pki/nss/ for hash/sign */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"
#include "oauth.h" // oauth base64 encode fn's.

// NSS includes
#include "pk11pub.h"
#include "nss.h"
#include "base64.h"
#include "keyhi.h"
#include "cryptohi.h"
#include "cert.h"

#if 1 // work-around compiler-warning
      // see http://bugzilla.mozilla.org/show_bug.cgi?id=243245#c3
  extern CERTCertificate *
  __CERT_DecodeDERCertificate (SECItem *derSignedCert, PRBool copyDER, char *nickname);
#endif

static const char NS_CERT_HEADER[]  = "-----BEGIN CERTIFICATE-----";
static const char NS_CERT_TRAILER[] = "-----END CERTIFICATE-----";
static const char NS_PRIV_HEADER[]  = "-----BEGIN PRIVATE KEY-----";
static const char NS_PRIV_TRAILER[] = "-----END PRIVATE KEY-----";

void oauth_init_nss() {
  static short nss_initialized = 0;
  if (!nss_initialized) { NSS_NoDB_Init("."); nss_initialized=1;}
}

/**
 * Removes heading & trailing strings; used only internally.
 * similar to NSS-source/nss/lib/pkcs7/certread.c
 *
 * the returned string (if not NULL) needs to be freed by the caller
 */
char *oauth_strip_pkcs(const char *txt, const char *h, const char *t) {
  char *start, *end, *rv;
  size_t len;
  if ((start=strstr(txt, h))==NULL) return NULL;
  start+=strlen(h);
  while (*start=='\r' || *start=='\n') start++;
  if ((end=strstr(start, t))==NULL) return NULL;
  end--;
  while (*end=='\r' || *end=='\n') end--;
  len = end-start+2;
  rv = xmalloc(len*sizeof(char));
  memcpy(rv,start,len);
  rv[len-1]='\0';
  return rv;
}

char *oauth_sign_hmac_sha1 (const char *m, const char *k) {
  return(oauth_sign_hmac_sha1_raw (m, strlen(m), k, strlen(k)));
}

char *oauth_sign_hmac_sha1_raw (const char *m, const size_t ml, const char *k, const size_t kl) {
  PK11SlotInfo  *slot = NULL;
  PK11SymKey    *pkey = NULL;
  PK11Context   *context = NULL;
  unsigned char  digest[20]; // Is there a way to tell how large the output is?
  unsigned int   len;
  SECStatus      s;
  SECItem        keyItem, noParams;
  char          *rv=NULL;

  keyItem.type = siBuffer;
  keyItem.data = (unsigned char*) k;
  keyItem.len = kl;

  noParams.type = siBuffer;
  noParams.data = NULL;
  noParams.len = 0;

  oauth_init_nss();

  slot = PK11_GetInternalKeySlot();
  if (!slot) goto looser;
  pkey = PK11_ImportSymKey(slot, CKM_SHA_1_HMAC, PK11_OriginUnwrap, CKA_SIGN, &keyItem, NULL);
  if (!pkey)  goto looser;
  context = PK11_CreateContextBySymKey(CKM_SHA_1_HMAC, CKA_SIGN, pkey, &noParams);
  if (!context) goto looser;

  s = PK11_DigestBegin(context);
  if (s != SECSuccess) goto looser;
  s = PK11_DigestOp(context, (unsigned char*) m, ml);
  if (s != SECSuccess) goto looser;
  s = PK11_DigestFinal(context, digest, &len, sizeof digest);
  if (s != SECSuccess) goto looser;

  rv=oauth_encode_base64(len, digest);

looser:
  if (context) PK11_DestroyContext(context, PR_TRUE);
  if (pkey) PK11_FreeSymKey(pkey);
  if (slot) PK11_FreeSlot(slot);
  return rv;
}

char *oauth_sign_rsa_sha1 (const char *m, const char *k) {
  PK11SlotInfo      *slot = NULL;
  SECKEYPrivateKey  *pkey = NULL;
  SECItem            signature;
  SECStatus          s;
  SECItem            der;
  char              *rv=NULL;

  char *key = oauth_strip_pkcs(k, NS_PRIV_HEADER, NS_PRIV_TRAILER); 
  if (!key) return NULL;

  oauth_init_nss();

  slot = PK11_GetInternalKeySlot();
  if (!slot) goto looser;
  s = ATOB_ConvertAsciiToItem(&der, key);
  if (s != SECSuccess) goto looser;
  s = PK11_ImportDERPrivateKeyInfoAndReturnKey(slot, &der, NULL, NULL, PR_FALSE, PR_TRUE, KU_ALL, &pkey, NULL);
  SECITEM_FreeItem(&der, PR_FALSE);
  if (s != SECSuccess) goto looser;
  if (!pkey) goto looser;
  if (pkey->keyType != rsaKey) goto looser;
  s = SEC_SignData(&signature, (unsigned char*) m, strlen(m), pkey, SEC_OID_ISO_SHA1_WITH_RSA_SIGNATURE);
  if (s != SECSuccess) goto looser;

  rv=oauth_encode_base64(signature.len, signature.data);
  SECITEM_FreeItem(&signature, PR_FALSE);

looser:
  if (pkey) SECKEY_DestroyPrivateKey(pkey);
  if (slot) PK11_FreeSlot(slot);
  free(key);
  return rv;
}

int oauth_verify_rsa_sha1 (const char *m, const char *c, const char *sig) {
  PK11SlotInfo      *slot = NULL;
  SECKEYPublicKey   *pkey = NULL;
  CERTCertificate   *cert = NULL;
  SECItem            signature;
  SECStatus          s;
  SECItem            der;
  int                rv=0;

  char *key = oauth_strip_pkcs(c, NS_CERT_HEADER, NS_CERT_TRAILER); 
  if (!key) return 0;

  oauth_init_nss();

  s = ATOB_ConvertAsciiToItem(&signature, (char*) sig); // XXX cast (const char*) -> (char*)
  if (s != SECSuccess) goto looser;
  slot = PK11_GetInternalKeySlot();
  if (!slot) goto looser;
  s = ATOB_ConvertAsciiToItem(&der, key);
  if (s != SECSuccess) goto looser;
  cert = __CERT_DecodeDERCertificate(&der, PR_TRUE, NULL);
  SECITEM_FreeItem(&der, PR_FALSE); 
  if (!cert) goto looser;
  pkey = CERT_ExtractPublicKey(cert);
  if (!pkey) goto looser;
  if (pkey->keyType != rsaKey) goto looser;

  s = VFY_VerifyData((unsigned char*) m, strlen(m), pkey, &signature, SEC_OID_ISO_SHA1_WITH_RSA_SIGNATURE, NULL);
  if (s == SECSuccess) rv=1;
#if 0
  else if (PR_GetError()!= SEC_ERROR_BAD_SIGNATURE) rv=-1;
#endif

looser:
  if (pkey) SECKEY_DestroyPublicKey(pkey);
  if (slot) PK11_FreeSlot(slot);
  free(key);
  return rv;
}

char *oauth_body_hash_file(char *filename) {
  PK11SlotInfo  *slot = NULL;
  PK11Context   *context = NULL;
  unsigned char  digest[20]; // Is there a way to tell how large the output is?
  unsigned int   len;
  SECStatus      s;
  char          *rv=NULL;
  size_t         bl;
  unsigned char  fb[BUFSIZ];

  FILE *F= fopen(filename, "r");
  if (!F) return NULL;

  oauth_init_nss();

  slot = PK11_GetInternalKeySlot();
  if (!slot) goto looser;
  context = PK11_CreateDigestContext(SEC_OID_SHA1);
  if (!context) goto looser;

  s = PK11_DigestBegin(context);
  if (s != SECSuccess) goto looser;
  while (!feof(F) && (bl=fread(fb,sizeof(char),BUFSIZ, F))>0) {
    s = PK11_DigestOp(context, (unsigned char*) fb, bl);
    if (s != SECSuccess) goto looser;
  }
  s = PK11_DigestFinal(context, digest, &len, sizeof digest);
  if (s != SECSuccess) goto looser;

  unsigned char *dgst = xmalloc(len*sizeof(char)); // oauth_body_hash_encode frees the digest..
  memcpy(dgst, digest, len);
  rv=oauth_body_hash_encode(len, dgst);

looser:
  fclose(F);
  if (context) PK11_DestroyContext(context, PR_TRUE);
  if (slot) PK11_FreeSlot(slot);
  return rv;
}

char *oauth_body_hash_data(size_t length, const char *data) {
  PK11SlotInfo  *slot = NULL;
  PK11Context   *context = NULL;
  unsigned char  digest[20]; // Is there a way to tell how large the output is?
  unsigned int   len;
  SECStatus      s;
  char          *rv=NULL;

  oauth_init_nss();

  slot = PK11_GetInternalKeySlot();
  if (!slot) goto looser;
  context = PK11_CreateDigestContext(SEC_OID_SHA1);
  if (!context) goto looser;

  s = PK11_DigestBegin(context);
  if (s != SECSuccess) goto looser;
  s = PK11_DigestOp(context, (unsigned char*) data, length);
  if (s != SECSuccess) goto looser;
  s = PK11_DigestFinal(context, digest, &len, sizeof digest);
  if (s != SECSuccess) goto looser;

  unsigned char *dgst = xmalloc(len*sizeof(char)); // oauth_body_hash_encode frees the digest..
  memcpy(dgst, digest, len);
  rv=oauth_body_hash_encode(len, dgst);

looser:
  if (context) PK11_DestroyContext(context, PR_TRUE);
  if (slot) PK11_FreeSlot(slot);
  return rv;
}

#else 
/* use http://www.openssl.org/ for hash/sign */

#ifdef _GNU_SOURCE
/*
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"
#include "oauth.h" // base64 encode fn's.
#include <openssl/hmac.h>

char *oauth_sign_hmac_sha1 (const char *m, const char *k) {
  return(oauth_sign_hmac_sha1_raw (m, strlen(m), k, strlen(k)));
}

char *oauth_sign_hmac_sha1_raw (const char *m, const size_t ml, const char *k, const size_t kl) {
  unsigned char result[EVP_MAX_MD_SIZE];
  unsigned int resultlen = 0;
  
  HMAC(EVP_sha1(), k, kl, 
      (unsigned char*) m, ml,
      result, &resultlen);

  return(oauth_encode_base64(resultlen, result));
}

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>

char *oauth_sign_rsa_sha1 (const char *m, const char *k) {
  unsigned char *sig = NULL;
  unsigned char *passphrase = NULL;
  unsigned int len=0;
  EVP_MD_CTX md_ctx;

  EVP_PKEY *pkey;
  BIO *in;
  in = BIO_new_mem_buf((unsigned char*) k, strlen(k));
  pkey = PEM_read_bio_PrivateKey(in, NULL, 0, passphrase); // generate sign
  BIO_free(in);

  if (pkey == NULL) {
  //fprintf(stderr, "liboauth/OpenSSL: can not read private key\n");
    return xstrdup("liboauth/OpenSSL: can not read private key");
  }

  len = EVP_PKEY_size(pkey);
  sig = (unsigned char*)xmalloc((len+1)*sizeof(char));

  EVP_SignInit(&md_ctx, EVP_sha1());
  EVP_SignUpdate(&md_ctx, m, strlen(m));
  if (EVP_SignFinal (&md_ctx, sig, &len, pkey)) {
    char *tmp;
    sig[len] = '\0';
    tmp = oauth_encode_base64(len,sig);
    OPENSSL_free(sig);
    EVP_PKEY_free(pkey);
    return tmp;
  }
  return xstrdup("liboauth/OpenSSL: rsa-sha1 signing failed");
}

int oauth_verify_rsa_sha1 (const char *m, const char *c, const char *s) {
  EVP_MD_CTX md_ctx;
  EVP_PKEY *pkey;
  BIO *in;
  X509 *cert = NULL;
  unsigned char *b64d;
  int slen, err;

  in = BIO_new_mem_buf((unsigned char*)c, strlen(c));
  cert = PEM_read_bio_X509(in, NULL, 0, NULL);
  if (cert)  {
    pkey = (EVP_PKEY *) X509_get_pubkey(cert); 
    X509_free(cert);
  } else {
    pkey = PEM_read_bio_PUBKEY(in, NULL, 0, NULL);
  }
  BIO_free(in);
  if (pkey == NULL) {
  //fprintf(stderr, "could not read cert/pubkey.\n");
    return -2;
  }

  b64d= (unsigned char*) xmalloc(sizeof(char)*strlen(s));
  slen = oauth_decode_base64(b64d, s);

  EVP_VerifyInit(&md_ctx, EVP_sha1());
  EVP_VerifyUpdate(&md_ctx, m, strlen(m));
  err = EVP_VerifyFinal(&md_ctx, b64d, slen, pkey);
  EVP_MD_CTX_cleanup(&md_ctx);
  EVP_PKEY_free(pkey);
  free(b64d);
  return (err);
}


/** 
 * http://oauth.googlecode.com/svn/spec/ext/body_hash/1.0/oauth-bodyhash.html
 */
char *oauth_body_hash_file(char *filename) {
  unsigned char fb[BUFSIZ];
  EVP_MD_CTX ctx;
  size_t len=0;
  unsigned char *md;
  FILE *F= fopen(filename, "r");
  if (!F) return NULL;

  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit(&ctx,EVP_sha1());
  while (!feof(F) && (len=fread(fb,sizeof(char),BUFSIZ, F))>0) {
    EVP_DigestUpdate(&ctx, fb, len);
  }
  fclose(F);
  len=0;
  md=(unsigned char*) xcalloc(EVP_MD_size(EVP_sha1()),sizeof(unsigned char));
  EVP_DigestFinal(&ctx, md,(unsigned int*) &len);
  EVP_MD_CTX_cleanup(&ctx);
  return oauth_body_hash_encode(len, md);
}

char *oauth_body_hash_data(size_t length, const char *data) {
  EVP_MD_CTX ctx;
  size_t len=0;
  unsigned char *md;
  md=(unsigned char*) xcalloc(EVP_MD_size(EVP_sha1()),sizeof(unsigned char));
  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit(&ctx,EVP_sha1());
  EVP_DigestUpdate(&ctx, data, length);
  EVP_DigestFinal(&ctx, md,(unsigned int*) &len);
  EVP_MD_CTX_cleanup(&ctx);
  return oauth_body_hash_encode(len, md);
}

#endif

// vi: sts=2 sw=2 ts=2
