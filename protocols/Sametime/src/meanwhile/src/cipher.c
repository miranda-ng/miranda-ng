
/*
  Meanwhile - Unofficial Lotus Sametime Community Client Library
  Copyright (C) 2004  Christopher (siege) O'Brien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <time.h>

#include "mpi/mpi.h"

#include "mw_channel.h"
#include "mw_cipher.h"
#include "mw_debug.h"
#include "mw_session.h"


struct mwMpi {
  mw_mp_int i;
};


/** From RFC2268 */
static guchar PT[] = {
  0xD9, 0x78, 0xF9, 0xC4, 0x19, 0xDD, 0xB5, 0xED,
  0x28, 0xE9, 0xFD, 0x79, 0x4A, 0xA0, 0xD8, 0x9D,
  0xC6, 0x7E, 0x37, 0x83, 0x2B, 0x76, 0x53, 0x8E,
  0x62, 0x4C, 0x64, 0x88, 0x44, 0x8B, 0xFB, 0xA2,
  0x17, 0x9A, 0x59, 0xF5, 0x87, 0xB3, 0x4F, 0x13,
  0x61, 0x45, 0x6D, 0x8D, 0x09, 0x81, 0x7D, 0x32,
  0xBD, 0x8F, 0x40, 0xEB, 0x86, 0xB7, 0x7B, 0x0B,
  0xF0, 0x95, 0x21, 0x22, 0x5C, 0x6B, 0x4E, 0x82,
  0x54, 0xD6, 0x65, 0x93, 0xCE, 0x60, 0xB2, 0x1C,
  0x73, 0x56, 0xC0, 0x14, 0xA7, 0x8C, 0xF1, 0xDC,
  0x12, 0x75, 0xCA, 0x1F, 0x3B, 0xBE, 0xE4, 0xD1,
  0x42, 0x3D, 0xD4, 0x30, 0xA3, 0x3C, 0xB6, 0x26,
  0x6F, 0xBF, 0x0E, 0xDA, 0x46, 0x69, 0x07, 0x57,
  0x27, 0xF2, 0x1D, 0x9B, 0xBC, 0x94, 0x43, 0x03,
  0xF8, 0x11, 0xC7, 0xF6, 0x90, 0xEF, 0x3E, 0xE7,
  0x06, 0xC3, 0xD5, 0x2F, 0xC8, 0x66, 0x1E, 0xD7,
  0x08, 0xE8, 0xEA, 0xDE, 0x80, 0x52, 0xEE, 0xF7,
  0x84, 0xAA, 0x72, 0xAC, 0x35, 0x4D, 0x6A, 0x2A,
  0x96, 0x1A, 0xD2, 0x71, 0x5A, 0x15, 0x49, 0x74,
  0x4B, 0x9F, 0xD0, 0x5E, 0x04, 0x18, 0xA4, 0xEC,
  0xC2, 0xE0, 0x41, 0x6E, 0x0F, 0x51, 0xCB, 0xCC,
  0x24, 0x91, 0xAF, 0x50, 0xA1, 0xF4, 0x70, 0x39,
  0x99, 0x7C, 0x3A, 0x85, 0x23, 0xB8, 0xB4, 0x7A,
  0xFC, 0x02, 0x36, 0x5B, 0x25, 0x55, 0x97, 0x31,
  0x2D, 0x5D, 0xFA, 0x98, 0xE3, 0x8A, 0x92, 0xAE,
  0x05, 0xDF, 0x29, 0x10, 0x67, 0x6C, 0xBA, 0xC9,
  0xD3, 0x00, 0xE6, 0xCF, 0xE1, 0x9E, 0xA8, 0x2C,
  0x63, 0x16, 0x01, 0x3F, 0x58, 0xE2, 0x89, 0xA9,
  0x0D, 0x38, 0x34, 0x1B, 0xAB, 0x33, 0xFF, 0xB0,
  0xBB, 0x48, 0x0C, 0x5F, 0xB9, 0xB1, 0xCD, 0x2E,
  0xC5, 0xF3, 0xDB, 0x47, 0xE5, 0xA5, 0x9C, 0x77,
  0x0A, 0xA6, 0x20, 0x68, 0xFE, 0x7F, 0xC1, 0xAD
};


/** prime number used in DH exchange */
static guchar dh_prime[] = {
  0xCF, 0x84, 0xAF, 0xCE, 0x86, 0xDD, 0xFA, 0x52,
  0x7F, 0x13, 0x6D, 0x10, 0x35, 0x75, 0x28, 0xEE,
  0xFB, 0xA0, 0xAF, 0xEF, 0x80, 0x8F, 0x29, 0x17,
  0x4E, 0x3B, 0x6A, 0x9E, 0x97, 0x00, 0x01, 0x71,
  0x7C, 0x8F, 0x10, 0x6C, 0x41, 0xC1, 0x61, 0xA6,
  0xCE, 0x91, 0x05, 0x7B, 0x34, 0xDA, 0x62, 0xCB,
  0xB8, 0x7B, 0xFD, 0xC1, 0xB3, 0x5C, 0x1B, 0x91,
  0x0F, 0xEA, 0x72, 0x24, 0x9D, 0x56, 0x6B, 0x9F
};


/** base used in DH exchange */
#define DH_BASE  3


struct mwMpi *mwMpi_new() {
  struct mwMpi *i;
  i = g_new0(struct mwMpi, 1);
  mw_mp_init(&i->i);
  return i;
}


void mwMpi_free(struct mwMpi *i) {
  if(! i) return;
  mw_mp_clear(&i->i);
  g_free(i);
}


static void mwInitDHPrime(mw_mp_int *i) {
  mw_mp_init(i);
  mw_mp_read_unsigned_bin(i, dh_prime, 64);
}


void mwMpi_setDHPrime(struct mwMpi *i) {
  g_return_if_fail(i != NULL);
  mw_mp_read_unsigned_bin(&i->i, dh_prime, 64);
}


static void mwInitDHBase(mw_mp_int *i) {
  mw_mp_init(i);
  mw_mp_set_int(i, DH_BASE);
}


void mwMpi_setDHBase(struct mwMpi *i) {
  g_return_if_fail(i != NULL);
  mw_mp_set_int(&i->i, DH_BASE);
}


static void mw_mp_set_rand(mw_mp_int *i, guint bits) {
  size_t len, l;
  guchar *buf;

  l = len = (bits / 8) + 1;
  buf = g_malloc(len);

  /// Miranda NG adaptation start - MSVC
  //srand(time(NULL));
  srand((unsigned int)time(NULL));
  /// Miranda NG adaptation end

  while(l--) buf[l] = rand() & 0xff;

  buf[0] &= (0xff >> (8 - (bits % 8)));

  mw_mp_read_unsigned_bin(i, buf, len);
  g_free(buf);
}


static void mwDHRandKeypair(mw_mp_int *private_key, mw_mp_int *public_key) {
  mw_mp_int prime, base;
 
  mwInitDHPrime(&prime);
  mwInitDHBase(&base);

  mw_mp_set_rand(private_key, 512);
  mw_mp_exptmod(&base, private_key, &prime, public_key);

  mw_mp_clear(&prime);
  mw_mp_clear(&base);
}


void mwMpi_randDHKeypair(struct mwMpi *private_key, struct mwMpi *public_key) {
  g_return_if_fail(private_key != NULL);
  g_return_if_fail(public_key != NULL);

  mwDHRandKeypair(&private_key->i, &public_key->i);
}


static void mwDHCalculateShared(mw_mp_int *shared_key, mw_mp_int *remote_key,
				mw_mp_int *private_key) {
  mw_mp_int prime;
 
  mwInitDHPrime(&prime);
  mw_mp_exptmod(remote_key, private_key, &prime, shared_key);
  mw_mp_clear(&prime);
}


void mwMpi_calculateDHShared(struct mwMpi *shared_key, struct mwMpi *remote_key,
			     struct mwMpi *private_key) {

  g_return_if_fail(shared_key != NULL);
  g_return_if_fail(remote_key != NULL);
  g_return_if_fail(private_key != NULL);

  mwDHCalculateShared(&shared_key->i, &remote_key->i, &private_key->i);
}


static void mwDHImportKey(mw_mp_int *key, struct mwOpaque *o) {
  mw_mp_read_unsigned_bin(key, o->data, o->len);
}


void mwMpi_import(struct mwMpi *i, struct mwOpaque *o) {
  g_return_if_fail(i != NULL);
  g_return_if_fail(o != NULL);

  mwDHImportKey(&i->i, o);
}


static void mwDHExportKey(mw_mp_int *key, struct mwOpaque *o) {
  o->len = mw_mp_unsigned_bin_size(key);
  o->data = g_malloc0(o->len);
  mw_mp_to_unsigned_bin(key, o->data);
}


void mwMpi_export(struct mwMpi *i, struct mwOpaque *o) {
  g_return_if_fail(i != NULL);
  g_return_if_fail(o != NULL);
  
  mwDHExportKey(&i->i, o);
}


void mwKeyRandom(guchar *key, gsize keylen) {
  g_return_if_fail(key != NULL);

  /// Miranda NG adaptation start - MSVC
  //srand(time(NULL));
  srand((unsigned int)time(NULL));
  /// Miranda NG adaptation end
  while(keylen--) key[keylen] = rand() & 0xff;
}


void mwIV_init(guchar *iv) {
  iv[0] = 0x01;
  iv[1] = 0x23;
  iv[2] = 0x45;
  iv[3] = 0x67;
  iv[4] = 0x89;
  iv[5] = 0xab;
  iv[6] = 0xcd;
  iv[7] = 0xef;
}


/* This does not seem to produce the same results as normal RC2 key
   expansion would, but it works, so eh. It might be smart to farm
   this out to mozilla or openssl */
void mwKeyExpand(int *ekey, const guchar *key, gsize keylen) {
  guchar tmp[128];
  int i, j;

  g_return_if_fail(keylen > 0);
  g_return_if_fail(key != NULL);

  if(keylen > 128) keylen = 128;

  /* fill the first chunk with what key bytes we have */
  for(i = keylen; i--; tmp[i] = key[i]);
  /* memcpy(tmp, key, keylen); */

  /* build the remaining key from the given data */
  for(i = 0; keylen < 128; i++) {
    tmp[keylen] = PT[ (tmp[keylen - 1] + tmp[i]) & 0xff ];
    keylen++;
  }

  tmp[0] = PT[ tmp[0] & 0xff ];

  for(i = 0, j = 0; i < 64; i++) {
    ekey[i] = (tmp[j] & 0xff) | (tmp[j+1] << 8);
    j += 2;
  }
}


/* normal RC2 encryption given a full 128-byte (as 64 ints) key */
static void mwEncryptBlock(const int *ekey, guchar *out) {

  int a, b, c, d;
  int i, j;

  a = (out[7] << 8) | (out[6] & 0xff);
  b = (out[5] << 8) | (out[4] & 0xff);
  c = (out[3] << 8) | (out[2] & 0xff);
  d = (out[1] << 8) | (out[0] & 0xff);

  for(i = 0; i < 16; i++) {
    j = i * 4;

    d += ((c & (a ^ 0xffff)) + (b & a) + ekey[j++]);
    d = (d << 1) | (d >> 15 & 0x0001);

    c += ((b & (d ^ 0xffff)) + (a & d) + ekey[j++]);
    c = (c << 2) | (c >> 14 & 0x0003);

    b += ((a & (c ^ 0xffff)) + (d & c) + ekey[j++]);
    b = (b << 3) | (b >> 13 & 0x0007);
    
    a += ((d & (b ^ 0xffff)) + (c & b) + ekey[j++]);
    a = (a << 5) | (a >> 11 & 0x001f);

    if(i == 4 || i == 10) {
      d += ekey[a & 0x003f];
      c += ekey[d & 0x003f];
      b += ekey[c & 0x003f];
      a += ekey[b & 0x003f];
    }    
  }

  *out++ = d & 0xff;
  *out++ = (d >> 8) & 0xff;
  *out++ = c & 0xff;
  *out++ = (c >> 8) & 0xff;
  *out++ = b & 0xff;
  *out++ = (b >> 8) & 0xff;
  *out++ = a & 0xff;
  *out++ = (a >> 8) & 0xff;
}


void mwEncryptExpanded(const int *ekey, guchar *iv,
		       struct mwOpaque *in_data,
		       struct mwOpaque *out_data) {

  guchar *i = in_data->data;
  gsize i_len = in_data->len;

  guchar *o;
  gsize o_len;

  /// Miranda NG adaptation start - MSVC
  ///int x, y;
  gsize x;
  int y;
  /// Miranda NG adaptation end

  /* pad upwards to a multiple of 8 */
  /* o_len = (i_len & -8) + 8; */
  o_len = i_len + (8 - (i_len % 8));
  o = g_malloc(o_len);

  out_data->data = o;
  out_data->len = o_len;

  /* figure out the amount of padding */
  y = o_len - i_len;

  /* copy in to out, and write padding bytes */
  for(x = i_len; x--; o[x] = i[x]);
  for(x = i_len; x < o_len; o[x++] = y);
  /* memcpy(o, i, i_len);
     memset(o + i_len, y, y); */

  /* encrypt in blocks */
  for(x = o_len; x > 0; x -= 8) {
    for(y = 8; y--; o[y] ^= iv[y]);
    mwEncryptBlock(ekey, o);
    for(y = 8; y--; iv[y] = o[y]);
    /* memcpy(iv, o, 8); */
    o += 8;
  }
}


void mwEncrypt(const guchar *key, gsize keylen, guchar *iv,
	       struct mwOpaque *in, struct mwOpaque *out) {

  int ekey[64];
  mwKeyExpand(ekey, key, keylen);
  mwEncryptExpanded(ekey, iv, in, out);
}


static void mwDecryptBlock(const int *ekey, guchar *out) {

  int a, b, c, d;
  int i, j;

  a = (out[7] << 8) | (out[6] & 0xff);
  b = (out[5] << 8) | (out[4] & 0xff);
  c = (out[3] << 8) | (out[2] & 0xff);
  d = (out[1] << 8) | (out[0] & 0xff);

  for(i = 16; i--; ) {
    j = i * 4 + 3;

    a = (a << 11) | (a >> 5 & 0x07ff);
    a -= ((d & (b ^ 0xffff)) + (c & b) + ekey[j--]);

    b = (b << 13) | (b >> 3 & 0x1fff);
    b -= ((a & (c ^ 0xffff)) + (d & c) + ekey[j--]);

    c = (c << 14) | (c >> 2 & 0x3fff);
    c -= ((b & (d ^ 0xffff)) + (a & d) + ekey[j--]);

    d = (d << 15) | (d >> 1 & 0x7fff);
    d -= ((c & (a ^ 0xffff)) + (b & a) + ekey[j--]);

    if(i == 5 || i == 11) {
      a -= ekey[b & 0x003f];
      b -= ekey[c & 0x003f];
      c -= ekey[d & 0x003f];
      d -= ekey[a & 0x003f];
    }
  }

  *out++ = d & 0xff;
  *out++ = (d >> 8) & 0xff;
  *out++ = c & 0xff;
  *out++ = (c >> 8) & 0xff;
  *out++ = b & 0xff;
  *out++ = (b >> 8) & 0xff;
  *out++ = a & 0xff;
  *out++ = (a >> 8) & 0xff;
}


void mwDecryptExpanded(const int *ekey, guchar *iv,
		       struct mwOpaque *in_data,
		       struct mwOpaque *out_data) {

  guchar *i = in_data->data;
  gsize i_len = in_data->len;

  guchar *o;
  gsize o_len;

  int x, y;

  if(i_len % 8) {
    /* this doesn't check to ensure that in_data->len is a multiple of
       8, which is damn well ought to be. */
    g_warning("attempting decryption of mis-sized data, %u bytes",
	      (guint) i_len);
  }

  o = g_malloc(i_len);
  o_len = i_len;
  for(x = i_len; x--; o[x] = i[x]);
  /* memcpy(o, i, i_len); */

  out_data->data = o;
  out_data->len = o_len;

  for(x = o_len; x > 0; x -= 8) {
    /* decrypt a block */
    mwDecryptBlock(ekey, o);

    /* modify the initialization vector */
    for(y = 8; y--; o[y] ^= iv[y]);
    for(y = 8; y--; iv[y] = i[y]);
    /* memcpy(iv, i, 8); */
    i += 8;
    o += 8;
  }

  /* shorten the length by the value of the filler in the padding
     bytes */
  out_data->len -= *(o - 1);
}


void mwDecrypt(const guchar *key, gsize keylen, guchar *iv,
	       struct mwOpaque *in, struct mwOpaque *out) {

  int ekey[64];
  mwKeyExpand(ekey, key, keylen);
  mwDecryptExpanded(ekey, iv, in, out);
}



struct mwCipher_RC2_40 {
  struct mwCipher cipher;
  int session_key[64];
  gboolean ready;
};


struct mwCipherInstance_RC2_40 {
  struct mwCipherInstance instance;
  int incoming_key[64];
  guchar outgoing_iv[8];
  guchar incoming_iv[8];
};


static const char *get_name_RC2_40() {
  return "RC2/40 Cipher";
}


static const char *get_desc_RC2_40() {
  return "RC2, 40-bit effective key";
}


static int encrypt_RC2_40(struct mwCipherInstance *ci,
			  struct mwOpaque *data) {

  struct mwCipherInstance_RC2_40 *cir;
  struct mwCipher_RC2_40 *cr;
  struct mwOpaque o = { 0, 0 };

  cir = (struct mwCipherInstance_RC2_40 *) ci;
  cr = (struct mwCipher_RC2_40 *) ci->cipher;

  mwEncryptExpanded(cr->session_key, cir->outgoing_iv, data, &o);

  mwOpaque_clear(data);
  data->data = o.data;
  data->len = o.len;

  return 0;
}


static int decrypt_RC2_40(struct mwCipherInstance *ci,
			  struct mwOpaque *data) {
  
  struct mwCipherInstance_RC2_40 *cir;
  struct mwCipher_RC2_40 *cr;
  struct mwOpaque o = { 0, 0 };

  cir = (struct mwCipherInstance_RC2_40 *) ci;
  cr = (struct mwCipher_RC2_40 *) ci->cipher;

  mwDecryptExpanded(cir->incoming_key, cir->incoming_iv, data, &o);

  mwOpaque_clear(data);
  data->data = o.data;
  data->len = o.len;

  return 0;
}


static struct mwCipherInstance *
new_instance_RC2_40(struct mwCipher *cipher,
		    struct mwChannel *chan) {

  struct mwCipher_RC2_40 *cr;
  struct mwCipherInstance_RC2_40 *cir;
  struct mwCipherInstance *ci;

  cr = (struct mwCipher_RC2_40 *) cipher;

  /* a bit of lazy initialization here */
  if(! cr->ready) {
    struct mwLoginInfo *info = mwSession_getLoginInfo(cipher->session);
    mwKeyExpand(cr->session_key, (guchar *) info->login_id, 5);
    cr->ready = TRUE;
  }

  cir = g_new0(struct mwCipherInstance_RC2_40, 1);
  ci = &cir->instance;

  ci->cipher = cipher;
  ci->channel = chan;

  mwIV_init(cir->incoming_iv);
  mwIV_init(cir->outgoing_iv);

  return ci;
}


static struct mwEncryptItem *new_item_RC2_40(struct mwCipherInstance *ci) {
  struct mwEncryptItem *e;

  e = g_new0(struct mwEncryptItem, 1);
  e->id = mwCipher_RC2_40;
  return e;
}


static struct mwEncryptItem *
offer_RC2_40(struct mwCipherInstance *ci) {
  return new_item_RC2_40(ci);
}


static void accepted_RC2_40(struct mwCipherInstance *ci,
			    struct mwEncryptItem *item) {

  struct mwCipherInstance_RC2_40 *cir;
  struct mwLoginInfo *info;

  cir = (struct mwCipherInstance_RC2_40 *) ci;
  info = mwChannel_getUser(ci->channel);

  if(info->login_id) {
    mwKeyExpand(cir->incoming_key, (guchar *) info->login_id, 5);
  }
}


static struct mwEncryptItem *
accept_RC2_40(struct mwCipherInstance *ci) {

  accepted_RC2_40(ci, NULL);
  return new_item_RC2_40(ci);
}


struct mwCipher *mwCipher_new_RC2_40(struct mwSession *s) {
  struct mwCipher_RC2_40 *cr = g_new0(struct mwCipher_RC2_40, 1);
  struct mwCipher *c = &cr->cipher;

  c->session = s;
  c->type = mwCipher_RC2_40;
  c->get_name = get_name_RC2_40;
  c->get_desc = get_desc_RC2_40;
  c->new_instance = new_instance_RC2_40;

  c->offer = offer_RC2_40;

  c->accepted = accepted_RC2_40;
  c->accept = accept_RC2_40;

  c->encrypt = encrypt_RC2_40;
  c->decrypt = decrypt_RC2_40;

  return c;
}


struct mwCipher_RC2_128 {
  struct mwCipher cipher;
  mw_mp_int private_key;
  struct mwOpaque public_key;
};


struct mwCipherInstance_RC2_128 {
  struct mwCipherInstance instance;
  int shared[64];      /* shared secret determined via DH exchange */
  guchar outgoing_iv[8];
  guchar incoming_iv[8];
};


static const char *get_name_RC2_128() {
  return "RC2/128 Cipher";
}


static const char *get_desc_RC2_128() {
  return "RC2, DH shared secret key";
}


static struct mwCipherInstance *
new_instance_RC2_128(struct mwCipher *cipher,
		     struct mwChannel *chan) {

  struct mwCipher_RC2_128 *cr;
  struct mwCipherInstance_RC2_128 *cir;
  struct mwCipherInstance *ci;

  cr = (struct mwCipher_RC2_128 *) cipher;

  cir = g_new0(struct mwCipherInstance_RC2_128, 1);
  ci = &cir->instance;
  
  ci->cipher = cipher;
  ci->channel = chan;

  mwIV_init(cir->incoming_iv);
  mwIV_init(cir->outgoing_iv);

  return ci;
}


static void offered_RC2_128(struct mwCipherInstance *ci,
			    struct mwEncryptItem *item) {
  
  mw_mp_int remote_key;
  mw_mp_int shared;
  struct mwOpaque sho = { 0, 0 };

  struct mwCipher *c;
  struct mwCipher_RC2_128 *cr;
  struct mwCipherInstance_RC2_128 *cir;

  c = ci->cipher;
  cr = (struct mwCipher_RC2_128 *) c;
  cir = (struct mwCipherInstance_RC2_128 *) ci;

  mw_mp_init(&remote_key);
  mw_mp_init(&shared);

  mwDHImportKey(&remote_key, &item->info);
  mwDHCalculateShared(&shared, &remote_key, &cr->private_key);
  mwDHExportKey(&shared, &sho);

  /* key expanded from the last 16 bytes of the DH shared secret. This
     took me forever to figure out. 16 bytes is 128 bit. */
  /* the sh_len-16 is important, because the key len could
     hypothetically start with 8bits or more unset, meaning the
     exported key might be less than 64 bytes in length */
  mwKeyExpand(cir->shared, sho.data+(sho.len-16), 16);
  
  mw_mp_clear(&remote_key);
  mw_mp_clear(&shared);
  mwOpaque_clear(&sho);
}


static struct mwEncryptItem *
offer_RC2_128(struct mwCipherInstance *ci) {

  struct mwCipher *c;
  struct mwCipher_RC2_128 *cr;
  struct mwEncryptItem *ei;

  c = ci->cipher;
  cr = (struct mwCipher_RC2_128 *) c;

  ei = g_new0(struct mwEncryptItem, 1);
  ei->id = mwCipher_RC2_128;
  mwOpaque_clone(&ei->info, &cr->public_key);

  return ei;
}			  


static void accepted_RC2_128(struct mwCipherInstance *ci,
			     struct mwEncryptItem *item) {

  /// Miranda NG adaptation start - MSVC
  ///return offered_RC2_128(ci, item);
  offered_RC2_128(ci, item);
  /// Miranda NG adaptation end
}


static struct mwEncryptItem *
accept_RC2_128(struct mwCipherInstance *ci) {

  return offer_RC2_128(ci);
}


static int encrypt_RC2_128(struct mwCipherInstance *ci,
			   struct mwOpaque *data) {

  struct mwCipherInstance_RC2_128 *cir;
  struct mwOpaque o = { 0, 0 };

  cir = (struct mwCipherInstance_RC2_128 *) ci;

  mwEncryptExpanded(cir->shared, cir->outgoing_iv, data, &o);

  mwOpaque_clear(data);
  data->data = o.data;
  data->len = o.len;

  return 0;
}


static int decrypt_RC2_128(struct mwCipherInstance *ci,
			   struct mwOpaque *data) {

  struct mwCipherInstance_RC2_128 *cir;
  struct mwOpaque o = { 0, 0 };

  cir = (struct mwCipherInstance_RC2_128 *) ci;

  mwDecryptExpanded(cir->shared, cir->incoming_iv, data, &o);

  mwOpaque_clear(data);
  data->data = o.data;
  data->len = o.len;

  return 0;
}


static void clear_RC2_128(struct mwCipher *c) {
  struct mwCipher_RC2_128 *cr;
  cr = (struct mwCipher_RC2_128 *) c;

  mw_mp_clear(&cr->private_key);
  mwOpaque_clear(&cr->public_key);
}


struct mwCipher *mwCipher_new_RC2_128(struct mwSession *s) {
  struct mwCipher_RC2_128 *cr;
  struct mwCipher *c;

  mw_mp_int pubkey;

  cr = g_new0(struct mwCipher_RC2_128, 1);
  c = &cr->cipher;

  c->session = s;
  c->type = mwCipher_RC2_128;
  c->get_name = get_name_RC2_128;
  c->get_desc = get_desc_RC2_128;
  c->new_instance = new_instance_RC2_128;

  c->offered = offered_RC2_128;
  c->offer = offer_RC2_128;

  c->accepted = accepted_RC2_128;
  c->accept = accept_RC2_128;

  c->encrypt = encrypt_RC2_128;
  c->decrypt = decrypt_RC2_128;

  c->clear = clear_RC2_128;
  
  mw_mp_init(&cr->private_key);
  mw_mp_init(&pubkey);
  mwDHRandKeypair(&cr->private_key, &pubkey);
  mwDHExportKey(&pubkey, &cr->public_key);
  mw_mp_clear(&pubkey);

  return c;
}


struct mwSession *mwCipher_getSession(struct mwCipher *cipher) {
  g_return_val_if_fail(cipher != NULL, NULL);
  return cipher->session;
}


guint16 mwCipher_getType(struct mwCipher *cipher) {
  /* oh man, this is a bad failover... who the hell decided to make
     zero a real cipher id? */
  g_return_val_if_fail(cipher != NULL, 0xffff);
  return cipher->type;
}


const char *mwCipher_getName(struct mwCipher *cipher) {
  g_return_val_if_fail(cipher != NULL, NULL);
  g_return_val_if_fail(cipher->get_name != NULL, NULL);
  return cipher->get_name();
}


const char *mwCipher_getDesc(struct mwCipher *cipher) {
  g_return_val_if_fail(cipher != NULL, NULL);
  g_return_val_if_fail(cipher->get_desc != NULL, NULL);
  return cipher->get_desc();
}


void mwCipher_free(struct mwCipher *cipher) {
  if(! cipher) return;

  if(cipher->clear)
    cipher->clear(cipher);

  g_free(cipher);
}


struct mwCipherInstance *mwCipher_newInstance(struct mwCipher *cipher,
					      struct mwChannel *chan) {
  g_return_val_if_fail(cipher != NULL, NULL);
  g_return_val_if_fail(chan != NULL, NULL);
  g_return_val_if_fail(cipher->new_instance != NULL, NULL);
  return cipher->new_instance(cipher, chan);
}


struct mwCipher *mwCipherInstance_getCipher(struct mwCipherInstance *ci) {
  g_return_val_if_fail(ci != NULL, NULL);
  return ci->cipher;
}


struct mwChannel *mwCipherInstance_getChannel(struct mwCipherInstance *ci) {
  g_return_val_if_fail(ci != NULL, NULL);
  return ci->channel;
}


void mwCipherInstance_offered(struct mwCipherInstance *ci,
			      struct mwEncryptItem *item) {
  struct mwCipher *cipher;

  g_return_if_fail(ci != NULL);

  cipher = ci->cipher;
  g_return_if_fail(cipher != NULL);

  if(cipher->offered) cipher->offered(ci, item);
}


struct mwEncryptItem *
mwCipherInstance_offer(struct mwCipherInstance *ci) {
  struct mwCipher *cipher;

  g_return_val_if_fail(ci != NULL, NULL);

  cipher = ci->cipher;
  g_return_val_if_fail(cipher != NULL, NULL);

  return cipher->offer(ci);
}


void mwCipherInstance_accepted(struct mwCipherInstance *ci,
			       struct mwEncryptItem *item) {
  struct mwCipher *cipher;

  g_return_if_fail(ci != NULL);

  cipher = ci->cipher;
  g_return_if_fail(cipher != NULL);

  if(cipher->accepted) cipher->accepted(ci, item);
}


struct mwEncryptItem *
mwCipherInstance_accept(struct mwCipherInstance *ci) {
  struct mwCipher *cipher;

  g_return_val_if_fail(ci != NULL, NULL);

  cipher = ci->cipher;
  g_return_val_if_fail(cipher != NULL, NULL);

  return cipher->accept(ci);
}


int mwCipherInstance_encrypt(struct mwCipherInstance *ci,
			     struct mwOpaque *data) {
  struct mwCipher *cipher;

  g_return_val_if_fail(data != NULL, 0);

  if(! ci) return 0;
  cipher = ci->cipher;

  g_return_val_if_fail(cipher != NULL, -1);

  return (cipher->encrypt)?
    cipher->encrypt(ci, data): 0;
}


int mwCipherInstance_decrypt(struct mwCipherInstance *ci,
			     struct mwOpaque *data) {
  struct mwCipher *cipher;

  g_return_val_if_fail(data != NULL, 0);

  if(! ci) return 0;
  cipher = ci->cipher;

  g_return_val_if_fail(cipher != NULL, -1);

  return (cipher->decrypt)?
    cipher->decrypt(ci, data): 0;
}


void mwCipherInstance_free(struct mwCipherInstance *ci) {
  struct mwCipher *cipher;

  if(! ci) return;

  cipher = ci->cipher;

  if(cipher && cipher->clear_instance)
    cipher->clear_instance(ci);

  g_free(ci);
}

