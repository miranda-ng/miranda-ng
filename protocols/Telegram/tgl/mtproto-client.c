/*
    This file is part of tgl-library

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Copyright Nikolay Durov, Andrey Lopatin 2012-2013
              Vitaly Valtman 2013-2015
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define        _FILE_OFFSET_BITS        64

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#endif
#include <fcntl.h>
#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/endian.h>
#endif
#include <sys/types.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

//#include "telegram.h"
#include "queries.h"
//#include "loop.h"
#include "tgl-structures.h"
#include "tgl-binlog.h"
#include "auto.h"
#include "auto/auto-types.h"
#include "auto/auto-skip.h"
#include "tgl.h"
#include "mtproto-client.h"
#include "tools.h"
#include "tree.h"
#include "updates.h"
#include "mtproto-utils.h"
#include "auto.h"
#include "tgl-methods-in.h"

#if defined(__FreeBSD__)
#define __builtin_bswap32(x) bswap32(x)
#endif

#if defined(__OpenBSD__)
#define __builtin_bswap32(x) __swap32gen(x)
#endif

#define sha1 SHA1

#include "mtproto-common.h"

#define MAX_NET_RES        (1L << 16)
//extern int log_level;

static long long generate_next_msg_id (struct tgl_state *TLS, struct tgl_dc *DC, struct tgl_session *S);
static double get_server_time (struct tgl_dc *DC);

#if !defined(HAVE___BUILTIN_BSWAP32) && !defined(__FreeBSD__) && !defined(__OpenBSD__)
static inline unsigned __builtin_bswap32(unsigned x) {
  return ((x << 24) & 0xff000000 ) |
  ((x << 8) & 0x00ff0000 ) |
  ((x >> 8) & 0x0000ff00 ) |
  ((x >> 24) & 0x000000ff );
}
#endif

// for statistic only
static int total_packets_sent;
static long long total_data_sent;


static int rpc_execute (struct tgl_state *TLS, struct connection *c, int op, int len);
static int rpc_becomes_ready (struct tgl_state *TLS, struct connection *c);
static int rpc_close (struct tgl_state *TLS, struct connection *c);

static double get_utime (int clock_id) {
  struct timespec T;
  tgl_my_clock_gettime (clock_id, &T);
  return T.tv_sec + (double) T.tv_nsec * 1e-9;
}


#define MAX_RESPONSE_SIZE        (1L << 24)

static RSA *rsa_load_public_key (struct tgl_state *TLS, const char *public_key_name) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
  FILE * f = NULL;
  errno_t err = fopen_s(&f, public_key_name, "r");
  if (err != 0) {
#else
  FILE *f = fopen (public_key_name, "r");
  if (f == NULL) {
#endif
    vlogprintf (E_WARNING, "Couldn't open public key file: %s\n", public_key_name);
    return NULL;
  }
  RSA *res = PEM_read_RSAPublicKey (f, NULL, NULL, NULL);
  fclose (f);
  if (res == NULL) {
    vlogprintf (E_WARNING, "PEM_read_RSAPublicKey returns NULL.\n");
    return NULL;
  }

  vlogprintf (E_NOTICE, "public key '%s' loaded successfully\n", public_key_name);

  return res;
}




/*
 *
 *        UNAUTHORIZED (DH KEY EXCHANGE) PROTOCOL PART
 *
 */

#define ENCRYPT_BUFFER_INTS        16384
static int encrypt_buffer[ENCRYPT_BUFFER_INTS];

#define DECRYPT_BUFFER_INTS        16384
static int decrypt_buffer[ENCRYPT_BUFFER_INTS];

static int encrypt_packet_buffer (struct tgl_state *TLS, struct tgl_dc *DC) {
  RSA *key = TLS->rsa_key_loaded[DC->rsa_key_idx];
  return tgl_pad_rsa_encrypt (TLS, (char *) packet_buffer, (packet_ptr - packet_buffer) * 4, (char *) encrypt_buffer, ENCRYPT_BUFFER_INTS * 4, key->n, key->e);
}

static int encrypt_packet_buffer_aes_unauth (const char server_nonce[16], const char hidden_client_nonce[32]) {
  tgl_init_aes_unauth (server_nonce, hidden_client_nonce, AES_ENCRYPT);
  return tgl_pad_aes_encrypt ((char *) packet_buffer, (packet_ptr - packet_buffer) * 4, (char *) encrypt_buffer, ENCRYPT_BUFFER_INTS * 4);
}

//
// Used in unauthorized part of protocol
//
static int rpc_send_packet (struct tgl_state *TLS, struct connection *c) {
  static struct {
    long long auth_key_id;
    long long out_msg_id;
    int msg_len;
  } unenc_msg_header;

  int len = (packet_ptr - packet_buffer) * 4;
  TLS->net_methods->incr_out_packet_num (c);

  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  struct tgl_session *S = TLS->net_methods->get_session (c);

  unenc_msg_header.out_msg_id = generate_next_msg_id (TLS, DC, S);
  unenc_msg_header.msg_len = len;

  int total_len = len + 20;
  assert (total_len > 0 && !(total_len & 0xfc000003));
  total_len >>= 2;
  vlogprintf (E_DEBUG, "writing packet: total_len = %d, len = %d\n", total_len, len);
  if (total_len < 0x7f) {
    assert (TLS->net_methods->write_out (c, &total_len, 1) == 1);
  } else {
    total_len = (total_len << 8) | 0x7f;
    assert (TLS->net_methods->write_out (c, &total_len, 4) == 4);
  }
  TLS->net_methods->write_out (c, &unenc_msg_header, 20);
  TLS->net_methods->write_out (c, packet_buffer, len);
  TLS->net_methods->flush_out (c);

  total_packets_sent ++;
  total_data_sent += total_len;
  return 1;
}

static int rpc_send_message (struct tgl_state *TLS, struct connection *c, void *data, int len) {
  assert (len > 0 && !(len & 0xfc000003));

  int total_len = len >> 2;
  if (total_len < 0x7f) {
    assert (TLS->net_methods->write_out (c, &total_len, 1) == 1);
  } else {
    total_len = (total_len << 8) | 0x7f;
    assert (TLS->net_methods->write_out (c, &total_len, 4) == 4);
  }

  TLS->net_methods->incr_out_packet_num (c);
  assert (TLS->net_methods->write_out (c, data, len) == len);
  TLS->net_methods->flush_out (c);

  total_packets_sent ++;
  total_data_sent += total_len;
  return 1;
}

//
// State machine. See description at
// https://core.telegram.org/mtproto/auth_key
//


static int check_unauthorized_header (struct tgl_state *TLS) {
  long long auth_key_id = fetch_long ();
  if (auth_key_id) {
    vlogprintf (E_ERROR, "ERROR: auth_key_id should be NULL\n");
    return -1;
  }
  fetch_long (); // msg_id
  int len = fetch_int ();
  if (len != 4 * (in_end - in_ptr)) {
    vlogprintf (E_ERROR, "ERROR: length mismatch\n");
    return -1;
  }
  return 0;
}

/* {{{ REQ_PQ */
// req_pq#60469778 nonce:int128 = ResPQ
static int send_req_pq_packet (struct tgl_state *TLS, struct connection *c) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  assert (DC->state == st_init);

  tglt_secure_random (DC->nonce, 16);
  clear_packet ();
  out_int (CODE_req_pq);
  out_ints ((int *)DC->nonce, 4);
  rpc_send_packet (TLS, c);

  DC->state = st_reqpq_sent;
  return 1;
}

// req_pq#60469778 nonce:int128 = ResPQ
static int send_req_pq_temp_packet (struct tgl_state *TLS, struct connection *c) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  assert (DC->state == st_authorized);

  tglt_secure_random (DC->nonce, 16);
  clear_packet ();
  out_int (CODE_req_pq);
  out_ints ((int *)DC->nonce, 4);
  rpc_send_packet (TLS, c);

  DC->state = st_reqpq_sent_temp;
  return 1;
}
/* }}} */

/* {{{ REQ DH */
// req_DH_params#d712e4be nonce:int128 server_nonce:int128 p:string q:string public_key_fingerprint:long encrypted_data:string = Server_DH_Params;
// p_q_inner_data#83c95aec pq:string p:string q:string nonce:int128 server_nonce:int128 new_nonce:int256 = P_Q_inner_data;
// p_q_inner_data_temp#3c6a84d4 pq:string p:string q:string nonce:int128 server_nonce:int128 new_nonce:int256 expires_in:int = P_Q_inner_data;
static void send_req_dh_packet (struct tgl_state *TLS, struct connection *c, BIGNUM *pq, int temp_key) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);

  BIGNUM *p = BN_new ();
  BIGNUM *q = BN_new ();
  assert (bn_factorize (pq, p, q) >= 0);

  clear_packet ();
  packet_ptr += 5;
  out_int (temp_key ? CODE_p_q_inner_data_temp : CODE_p_q_inner_data);

  out_bignum (pq);
  out_bignum (p);
  out_bignum (q);

  out_ints ((int *) DC->nonce, 4);
  out_ints ((int *) DC->server_nonce, 4);
  tglt_secure_random (DC->new_nonce, 32);
  out_ints ((int *) DC->new_nonce, 8);
  if (temp_key) {
    out_int (TLS->temp_key_expire_time);
  }
  sha1 ((unsigned char *) (packet_buffer + 5), (packet_ptr - packet_buffer - 5) * 4, (unsigned char *) packet_buffer);

  int l = encrypt_packet_buffer (TLS, DC);

  clear_packet ();
  out_int (CODE_req_DH_params);
  out_ints ((int *) DC->nonce, 4);
  out_ints ((int *) DC->server_nonce, 4);
  out_bignum (p);
  out_bignum (q);

  out_long (TLS->rsa_key_fingerprint[DC->rsa_key_idx]);
  out_cstring ((char *) encrypt_buffer, l);

  BN_free (p);
  BN_free (q);
  DC->state = temp_key ? st_reqdh_sent_temp : st_reqdh_sent;
  rpc_send_packet (TLS, c);
}
/* }}} */

/* {{{ SEND DH PARAMS */
// set_client_DH_params#f5045f1f nonce:int128 server_nonce:int128 encrypted_data:string = Set_client_DH_params_answer;
// client_DH_inner_data#6643b654 nonce:int128 server_nonce:int128 retry_id:long g_b:string = Client_DH_Inner_Data
static void send_dh_params (struct tgl_state *TLS, struct connection *c, BIGNUM *dh_prime, BIGNUM *g_a, int g, int temp_key) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);

  clear_packet ();
  packet_ptr += 5;
  out_int (CODE_client_DH_inner_data);
  out_ints ((int *) DC->nonce, 4);
  out_ints ((int *) DC->server_nonce, 4);
  out_long (0);

  BIGNUM *dh_g = BN_new ();
  ensure (BN_set_word (dh_g, g));

  static unsigned char s_power[256];
  tglt_secure_random (s_power, 256);
  BIGNUM *dh_power = BN_bin2bn ((unsigned char *)s_power, 256, 0);
  ensure_ptr (dh_power);

  BIGNUM *y = BN_new ();
  ensure_ptr (y);
  ensure (BN_mod_exp (y, dh_g, dh_power, dh_prime, TLS->BN_ctx));
  out_bignum (y);
  BN_free (y);

  BIGNUM *auth_key_num = BN_new ();
  ensure (BN_mod_exp (auth_key_num, g_a, dh_power, dh_prime, TLS->BN_ctx));
  int l = BN_num_bytes (auth_key_num);
  assert (l >= 250 && l <= 256);
  assert (BN_bn2bin (auth_key_num, (unsigned char *)(temp_key ? DC->temp_auth_key : DC->auth_key)));
  if (l < 256) {
    char *key = temp_key ? DC->temp_auth_key : DC->auth_key;
    memmove (key + 256 - l, key, l);
    memset (key, 0, 256 - l);
  }

  BN_free (dh_power);
  BN_free (auth_key_num);
  BN_free (dh_g);

  sha1 ((unsigned char *) (packet_buffer + 5), (packet_ptr - packet_buffer - 5) * 4, (unsigned char *) packet_buffer);

  l = encrypt_packet_buffer_aes_unauth (DC->server_nonce, DC->new_nonce);

  clear_packet ();
  out_int (CODE_set_client_DH_params);
  out_ints ((int *) DC->nonce, 4);
  out_ints ((int *) DC->server_nonce, 4);
  out_cstring ((char *) encrypt_buffer, l);

  DC->state = temp_key ? st_client_dh_sent_temp : st_client_dh_sent;;
  rpc_send_packet (TLS, c);
}
/* }}} */

/* {{{ RECV RESPQ */
// resPQ#05162463 nonce:int128 server_nonce:int128 pq:string server_public_key_fingerprints:Vector long = ResPQ
static int process_respq_answer (struct tgl_state *TLS, struct connection *c, char *packet, int len, int temp_key) {
  assert (!(len & 3));
  in_ptr = (int *)packet;
  in_end = in_ptr + (len / 4);
  if (check_unauthorized_header (TLS) < 0) {
    return -1;
  }

  int *in_save = in_ptr;
  if (skip_type_any (TYPE_TO_PARAM (res_p_q)) < 0 || in_ptr != in_end) {
    vlogprintf (E_ERROR, "can not parse req_p_q answer\n");
    return -1;
  }
  in_ptr = in_save;

  struct tgl_dc *DC = TLS->net_methods->get_dc (c);

  assert (fetch_int() == CODE_res_p_q);

  static int tmp[4];
  fetch_ints (tmp, 4);
  if (memcmp (tmp, DC->nonce, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  fetch_ints (DC->server_nonce, 4);

  BIGNUM *pq = BN_new ();
  assert (fetch_bignum (pq) >= 0);

  assert (fetch_int ()  == CODE_vector);
  int fingerprints_num = fetch_int ();
  assert (fingerprints_num >= 0);
  DC->rsa_key_idx = -1;

  int i;
  for (i = 0; i < fingerprints_num; i++) {
    int j;
    long long fprint = fetch_long ();
    for (j = 0; j < TLS->rsa_key_num; j++) {
      if (TLS->rsa_key_loaded[j]) {
        if (fprint == TLS->rsa_key_fingerprint[j]) {
          DC->rsa_key_idx = j;
          break;
        }
      }
    }
  }
  assert (in_ptr == in_end);
  if (DC->rsa_key_idx == -1) {
    vlogprintf (E_ERROR, "fatal: don't have any matching keys\n");
    return -1;
  }

  send_req_dh_packet (TLS, c, pq, temp_key);

  BN_free (pq);
  return 1;
}
/* }}} */

/* {{{ RECV DH */
// server_DH_params_fail#79cb045d nonce:int128 server_nonce:int128 new_nonce_hash:int128 = Server_DH_Params;
// server_DH_params_ok#d0e8075c nonce:int128 server_nonce:int128 encrypted_answer:string = Server_DH_Params;
// server_DH_inner_data#b5890dba nonce:int128 server_nonce:int128 g:int dh_prime:string g_a:string server_time:int = Server_DH_inner_data;
static int process_dh_answer (struct tgl_state *TLS, struct connection *c, char *packet, int len, int temp_key) {
  assert (!(len & 3));
  in_ptr = (int *)packet;
  in_end = in_ptr + (len / 4);
  if (check_unauthorized_header (TLS) < 0) {
    return -1;
  }

  int *in_save = in_ptr;
  if (skip_type_any (TYPE_TO_PARAM (server_d_h_params)) < 0 || in_ptr != in_end) {
    vlogprintf (E_ERROR, "can not parse server_DH_params answer\n");
    return -1;
  }
  in_ptr = in_save;

  struct tgl_dc *DC = TLS->net_methods->get_dc (c);

  unsigned op = fetch_int ();
  assert (op == CODE_server__d_h_params_ok || op == CODE_server__d_h_params_fail);

  int tmp[4];
  fetch_ints (tmp, 4);
  if (memcmp (tmp, DC->nonce, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  assert (!memcmp (tmp, DC->nonce, 16));
  fetch_ints (tmp, 4);
  if (memcmp (tmp, DC->server_nonce, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  assert (!memcmp (tmp, DC->server_nonce, 16));

  if (op == CODE_server__d_h_params_fail) {
    vlogprintf (E_ERROR, "DH params fail\n");
    return -1;
  }

  tgl_init_aes_unauth (DC->server_nonce, DC->new_nonce, AES_DECRYPT);

  int l = prefetch_strlen ();
  assert (l >= 0);
  if (!l) {
    vlogprintf (E_ERROR, "non-empty encrypted part expected\n");
    return -1;
  }
  l = tgl_pad_aes_decrypt (fetch_str (l), l, (char *) decrypt_buffer, DECRYPT_BUFFER_INTS * 4 - 16);
  assert (in_ptr == in_end);

  in_ptr = decrypt_buffer + 5;
  in_end = decrypt_buffer + (l >> 2);
  if (skip_type_any (TYPE_TO_PARAM (server_d_h_inner_data)) < 0) {
    vlogprintf (E_ERROR, "can not parse server_DH_inner_data answer\n");
    return -1;
  }
  in_ptr = decrypt_buffer + 5;

  assert (fetch_int ()  == (int)CODE_server_DH_inner_data);
  fetch_ints (tmp, 4);
  if (memcmp (tmp, DC->nonce, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  assert (!memcmp (tmp, DC->nonce, 16));
  fetch_ints (tmp, 4);
  if (memcmp (tmp, DC->server_nonce, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  assert (!memcmp (tmp, DC->server_nonce, 16));
  int g = fetch_int ();

  BIGNUM *dh_prime = BN_new ();
  BIGNUM *g_a = BN_new ();
  assert (fetch_bignum (dh_prime) > 0);
  assert (fetch_bignum (g_a) > 0);

  if (tglmp_check_DH_params (TLS, dh_prime, g) < 0) {
    vlogprintf (E_ERROR, "bad DH params\n");
    return -1;
  }
  if (tglmp_check_g_a (TLS, dh_prime, g_a) < 0) {
    vlogprintf (E_ERROR, "bad dh_prime\n");
    return -1;
  }

  int server_time = fetch_int ();
  assert (in_ptr <= in_end);

  static char sha1_buffer[20];
  sha1 ((unsigned char *) decrypt_buffer + 20, (in_ptr - decrypt_buffer - 5) * 4, (unsigned char *) sha1_buffer);
  if (memcmp (decrypt_buffer, sha1_buffer, 20)) {
    vlogprintf (E_ERROR, "bad encrypted message SHA1\n");
    return -1;
  }
  if ((char *) in_end - (char *) in_ptr >= 16) {
    vlogprintf (E_ERROR, "too much padding\n");
    return -1;
  }

  DC->server_time_delta = server_time - get_utime (CLOCK_REALTIME);
  DC->server_time_udelta = server_time - get_utime (CLOCK_MONOTONIC);

  send_dh_params (TLS, c, dh_prime, g_a, g, temp_key);

  BN_free (dh_prime);
  BN_free (g_a);

  return 1;
}
/* }}} */

static void create_temp_auth_key (struct tgl_state *TLS, struct connection *c) {
  assert (TLS->enable_pfs);
  send_req_pq_temp_packet (TLS, c);
}

int tglmp_encrypt_inner_temp (struct tgl_state *TLS, struct connection *c, int *msg, int msg_ints, int useful, void *data, long long msg_id);
static long long msg_id_override;
static void mpc_on_get_config (struct tgl_state *TLS, void *extra, int success);
static void bind_temp_auth_key (struct tgl_state *TLS, struct connection *c);

/* {{{ RECV AUTH COMPLETE */

// dh_gen_ok#3bcbf734 nonce:int128 server_nonce:int128 new_nonce_hash1:int128 = Set_client_DH_params_answer;
// dh_gen_retry#46dc1fb9 nonce:int128 server_nonce:int128 new_nonce_hash2:int128 = Set_client_DH_params_answer;
// dh_gen_fail#a69dae02 nonce:int128 server_nonce:int128 new_nonce_hash3:int128 = Set_client_DH_params_answer;
static int process_auth_complete (struct tgl_state *TLS, struct connection *c, char *packet, int len, int temp_key) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);

  assert (!(len & 3));
  in_ptr = (int *)packet;
  in_end = in_ptr + (len / 4);
  if (check_unauthorized_header (TLS) < 0) {
    return -1;
  }

  int *in_save = in_ptr;
  if (skip_type_any (TYPE_TO_PARAM (set_client_d_h_params_answer)) < 0 || in_ptr != in_end) {
    vlogprintf (E_ERROR, "can not parse server_DH_params answer\n");
    return -1;
  }
  in_ptr = in_save;

  unsigned op = fetch_int ();
  assert (op == CODE_dh_gen_ok || op == CODE_dh_gen_retry || op == CODE_dh_gen_fail);

  int tmp[4];
  fetch_ints (tmp, 4);
  if (memcmp (DC->nonce, tmp, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  fetch_ints (tmp, 4);
  if (memcmp (DC->server_nonce, tmp, 16)) {
    vlogprintf (E_ERROR, "nonce mismatch\n");
    return -1;
  }
  if (op != CODE_dh_gen_ok) {
    vlogprintf (E_ERROR, "something bad. Retry regen\n");
    return -1;
  }

  fetch_ints (tmp, 4);

  static unsigned char th[44], sha1_buffer[20];
  memcpy (th, DC->new_nonce, 32);
  th[32] = 1;
  if (!temp_key) {
    sha1 ((unsigned char *)DC->auth_key, 256, sha1_buffer);
  } else {
    sha1 ((unsigned char *)DC->temp_auth_key, 256, sha1_buffer);
  }
  memcpy (th + 33, sha1_buffer, 8);
  sha1 (th, 41, sha1_buffer);
  if (memcmp (tmp, sha1_buffer + 4, 16)) {
    vlogprintf (E_ERROR, "hash mismatch\n");
    return -1;
  }

  if (!temp_key) {
    bl_do_set_auth_key (TLS, DC->id, (unsigned char *)DC->auth_key);
    sha1 ((unsigned char *)DC->auth_key, 256, sha1_buffer);
  } else {
    sha1 ((unsigned char *)DC->temp_auth_key, 256, sha1_buffer);
    DC->temp_auth_key_id = *(long long *)(sha1_buffer + 12);
  }

  DC->server_salt = *(long long *)DC->server_nonce ^ *(long long *)DC->new_nonce;

  DC->state = st_authorized;

  vlogprintf (E_DEBUG, "Auth success\n");
  if (temp_key) {
    bind_temp_auth_key (TLS, c);
  } else {
    DC->flags |= 1;
    if (TLS->enable_pfs) {
      create_temp_auth_key (TLS, c);
    } else {
      DC->temp_auth_key_id = DC->auth_key_id;
      memcpy (DC->temp_auth_key, DC->auth_key, 256);
      DC->flags |= 2;
      if (!(DC->flags & 4)) {
        tgl_do_help_get_config_dc (TLS, DC, mpc_on_get_config, DC);
      }
    }
  }

  return 1;
}
/* }}} */

static void bind_temp_auth_key (struct tgl_state *TLS, struct connection *c) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  if (DC->temp_auth_key_bind_query_id) {
    tglq_query_delete (TLS, DC->temp_auth_key_bind_query_id);
  }
  struct tgl_session *S = TLS->net_methods->get_session (c);
  long long msg_id = generate_next_msg_id (TLS, DC, S);

  clear_packet ();
  out_int (CODE_bind_auth_key_inner);
  long long rand;
  tglt_secure_random (&rand, 8);
  out_long (rand);
  out_long (DC->temp_auth_key_id);
  out_long (DC->auth_key_id);

  if (!S->session_id) {
    tglt_secure_random (&S->session_id, 8);
  }
  out_long (S->session_id);
  int expires = (int)time (0) + DC->server_time_delta + TLS->temp_key_expire_time;
  out_int (expires);

  static int data[1000];
  int len = tglmp_encrypt_inner_temp (TLS, c, packet_buffer, packet_ptr - packet_buffer, 0, data, msg_id);
  msg_id_override = msg_id;
  DC->temp_auth_key_bind_query_id = msg_id;
  tgl_do_send_bind_temp_key (TLS, DC, rand, expires, (void *)data, len, msg_id);
  msg_id_override = 0;
}

/*
 *
 *                AUTHORIZED (MAIN) PROTOCOL PART
 *
 */

static struct encrypted_message enc_msg;

static double get_server_time (struct tgl_dc *DC) {
  //if (!DC->server_time_udelta) {
  //  DC->server_time_udelta = get_utime (CLOCK_REALTIME) - get_utime (CLOCK_MONOTONIC);
  //}
  return get_utime (CLOCK_MONOTONIC) + DC->server_time_udelta;
}

static long long generate_next_msg_id (struct tgl_state *TLS, struct tgl_dc *DC, struct tgl_session *S) {
  long long next_id = (long long) (get_server_time (DC) * (1LL << 32)) & -4;
  if (next_id <= S->last_msg_id) {
    next_id = S->last_msg_id  += 4;
  } else {
    S->last_msg_id = next_id;
  }
  return next_id;
}

static void init_enc_msg (struct tgl_state *TLS, struct tgl_session *S, int useful) {
  struct tgl_dc *DC = S->dc;
  assert (DC->state == st_authorized);
  assert (DC->temp_auth_key_id);
  vlogprintf (E_DEBUG, "temp_auth_key_id = 0x%016llx, auth_key_id = 0x%016llx\n", DC->temp_auth_key_id, DC->auth_key_id);
  enc_msg.auth_key_id = DC->temp_auth_key_id;
  enc_msg.server_salt = DC->server_salt;
  if (!S->session_id) {
    tglt_secure_random (&S->session_id, 8);
  }
  enc_msg.session_id = S->session_id;
  enc_msg.msg_id = msg_id_override ? msg_id_override : generate_next_msg_id (TLS, DC, S);
  enc_msg.seq_no = S->seq_no;
  if (useful) {
    enc_msg.seq_no |= 1;
  }
  S->seq_no += 2;
};

static void init_enc_msg_inner_temp (struct tgl_dc *DC, long long msg_id) {
  enc_msg.auth_key_id = DC->auth_key_id;
  tglt_secure_random (&enc_msg.server_salt, 8);
  tglt_secure_random (&enc_msg.session_id, 8);
  enc_msg.msg_id = msg_id;
  enc_msg.seq_no = 0;
};


static int aes_encrypt_message (struct tgl_state *TLS, char *key, struct encrypted_message *enc) {
  unsigned char sha1_buffer[20];
  const int MINSZ = offsetof (struct encrypted_message, message);
  const int UNENCSZ = offsetof (struct encrypted_message, server_salt);

  int enc_len = (MINSZ - UNENCSZ) + enc->msg_len;
  assert (enc->msg_len >= 0 && enc->msg_len <= MAX_MESSAGE_INTS * 4 - 16 && !(enc->msg_len & 3));
  sha1 ((unsigned char *) &enc->server_salt, enc_len, sha1_buffer);
  vlogprintf (E_DEBUG, "sending message with sha1 %08x\n", *(int *)sha1_buffer);
  memcpy (enc->msg_key, sha1_buffer + 4, 16);
  tgl_init_aes_auth (key, enc->msg_key, AES_ENCRYPT);
  return tgl_pad_aes_encrypt ((char *) &enc->server_salt, enc_len, (char *) &enc->server_salt, MAX_MESSAGE_INTS * 4 + (MINSZ - UNENCSZ));
}

long long tglmp_encrypt_send_message (struct tgl_state *TLS, struct connection *c, int *msg, int msg_ints, int flags) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  struct tgl_session *S = TLS->net_methods->get_session (c);
  assert (S);
  if (!(DC->flags & 4) && !(flags & 2)) {
    return generate_next_msg_id (TLS, DC, S);
  }

  const int UNENCSZ = offsetof (struct encrypted_message, server_salt);
  if (msg_ints <= 0 || msg_ints > MAX_MESSAGE_INTS - 4) {
    return -1;
  }
  if (msg) {
    memcpy (enc_msg.message, msg, msg_ints * 4);
    enc_msg.msg_len = msg_ints * 4;
  } else {
    if ((enc_msg.msg_len & 0x80000003) || enc_msg.msg_len > MAX_MESSAGE_INTS * 4 - 16) {
      return -1;
    }
  }
  init_enc_msg (TLS, S, flags & 1);

  int l = aes_encrypt_message (TLS, DC->temp_auth_key, &enc_msg);
  assert (l > 0);
  rpc_send_message (TLS, c, &enc_msg, l + UNENCSZ);

  return S->last_msg_id;
}

int tglmp_encrypt_inner_temp (struct tgl_state *TLS, struct connection *c, int *msg, int msg_ints, int useful, void *data, long long msg_id) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  struct tgl_session *S = TLS->net_methods->get_session (c);
  assert (S);

  const int UNENCSZ = offsetof (struct encrypted_message, server_salt);
  if (msg_ints <= 0 || msg_ints > MAX_MESSAGE_INTS - 4) {
    return -1;
  }
  memcpy (enc_msg.message, msg, msg_ints * 4);
  enc_msg.msg_len = msg_ints * 4;

  init_enc_msg_inner_temp (DC, msg_id);

  int l = aes_encrypt_message (TLS, DC->auth_key, &enc_msg);
  assert (l > 0);
  //rpc_send_message (c, &enc_msg, l + UNENCSZ);
  memcpy (data, &enc_msg, l + UNENCSZ);

  return l + UNENCSZ;
}

static int rpc_execute_answer (struct tgl_state *TLS, struct connection *c, long long msg_id);

static int work_container (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  vlogprintf (E_DEBUG, "work_container: msg_id = %"_PRINTF_INT64_"d\n", msg_id);
  assert (fetch_int () == CODE_msg_container);
  int n = fetch_int ();
  int i;
  for (i = 0; i < n; i++) {
    long long id = fetch_long ();
    //int seqno = fetch_int ();
    fetch_int (); // seq_no
    if (id & 1) {
      tgln_insert_msg_id (TLS, TLS->net_methods->get_session (c), id);
    }
    int bytes = fetch_int ();
    int *t = in_end;
    in_end = in_ptr + (bytes / 4);
    int r = rpc_execute_answer (TLS, c, id);
    if (r < 0) { return -1; }
    assert (in_ptr == in_end);
    in_end = t;
  }
  return 0;
}

static int work_new_session_created (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  vlogprintf (E_DEBUG, "work_new_session_created: msg_id = %"_PRINTF_INT64_"d\n", msg_id);
  assert (fetch_int () == (int)CODE_new_session_created);
  fetch_long (); // first message id
  fetch_long (); // unique_id
  TLS->net_methods->get_dc (c)->server_salt = fetch_long ();
  if (TLS->started && !(TLS->locks & TGL_LOCK_DIFF) && (TLS->DC_working->flags & TGLDCF_LOGGED_IN)) {
    tgl_do_get_difference (TLS, 0, 0, 0);
  }
  return 0;
}

static int work_msgs_ack (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  vlogprintf (E_DEBUG, "work_msgs_ack: msg_id = %"_PRINTF_INT64_"d\n", msg_id);
  assert (fetch_int () == CODE_msgs_ack);
  assert (fetch_int () == CODE_vector);
  int n = fetch_int ();
  int i;
  for (i = 0; i < n; i++) {
    long long id = fetch_long ();
    vlogprintf (E_DEBUG + 1, "ack for %"_PRINTF_INT64_"d\n", id);
    tglq_query_ack (TLS, id);
  }
  return 0;
}

static int work_rpc_result (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  vlogprintf (E_DEBUG, "work_rpc_result: msg_id = %"_PRINTF_INT64_"d\n", msg_id);
  assert (fetch_int () == (int)CODE_rpc_result);
  long long id = fetch_long ();
  int op = prefetch_int ();
  if (op == CODE_rpc_error) {
    return tglq_query_error (TLS, id);
  } else {
    return tglq_query_result (TLS, id);
  }
}

#define MAX_PACKED_SIZE (1 << 24)
static int work_packed (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  assert (fetch_int () == CODE_gzip_packed);
  static int in_gzip;
  static int buf[MAX_PACKED_SIZE >> 2];
  assert (!in_gzip);
  in_gzip = 1;

  int l = prefetch_strlen ();
  char *s = fetch_str (l);

  int total_out = tgl_inflate (s, l, buf, MAX_PACKED_SIZE);
  int *end = in_ptr;
  int *eend = in_end;
  //assert (total_out % 4 == 0);
  in_ptr = buf;
  in_end = in_ptr + total_out / 4;
  int r = rpc_execute_answer (TLS, c, msg_id);
  in_ptr = end;
  in_end = eend;
  in_gzip = 0;
  return r;
}

static int work_bad_server_salt (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  assert (fetch_int () == (int)CODE_bad_server_salt);
  long long id = fetch_long ();
  tglq_query_restart (TLS, id);
  fetch_int (); // seq_no
  fetch_int (); // error_code
  long long new_server_salt = fetch_long ();
  TLS->net_methods->get_dc (c)->server_salt = new_server_salt;
  return 0;
}

static int work_pong (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  assert (fetch_int () == CODE_pong);
  fetch_long (); // msg_id
  fetch_long (); // ping_id
  return 0;
}

static int work_detailed_info (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  assert (fetch_int () == CODE_msg_detailed_info);
  fetch_long (); // msg_id
  fetch_long (); // answer_msg_id
  fetch_int (); // bytes
  fetch_int (); // status
  return 0;
}

static int work_new_detailed_info (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  assert (fetch_int () == (int)CODE_msg_new_detailed_info);
  fetch_long (); // answer_msg_id
  fetch_int (); // bytes
  fetch_int (); // status
  return 0;
}

static int work_bad_msg_notification (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  assert (fetch_int () == (int)CODE_bad_msg_notification);
  long long m1 = fetch_long ();
  int s = fetch_int ();
  int e = fetch_int ();
  vlogprintf (E_NOTICE, "bad_msg_notification: msg_id = %"_PRINTF_INT64_"d, seq = %d, error = %d\n", m1, s, e);
  switch (e) {
  // Too low msg id
  case 16:
    tglq_regen_query (TLS, m1);
    break;
  // Too high msg id
  case 17:
    tglq_regen_query (TLS, m1);
    break;
  default:
    vlogprintf (E_NOTICE, "bad_msg_notification: msg_id = %"_PRINTF_INT64_"d, seq = %d, error = %d\n", m1, s, e);
    break;
  }

  return -1;
}

static int rpc_execute_answer (struct tgl_state *TLS, struct connection *c, long long msg_id) {
  int op = prefetch_int ();
  switch (op) {
  case CODE_msg_container:
    return work_container (TLS, c, msg_id);
  case CODE_new_session_created:
    return work_new_session_created (TLS, c, msg_id);
  case CODE_msgs_ack:
    return work_msgs_ack (TLS, c, msg_id);
  case CODE_rpc_result:
    return work_rpc_result (TLS, c, msg_id);
  case CODE_update_short:
  case CODE_updates:
  case CODE_update_short_message:
  case CODE_update_short_chat_message:
  case CODE_updates_too_long:
    tglu_work_any_updates (TLS);
    return 0;
  case CODE_gzip_packed:
    return work_packed (TLS, c, msg_id);
  case CODE_bad_server_salt:
    return work_bad_server_salt (TLS, c, msg_id);
  case CODE_pong:
    return work_pong (TLS, c, msg_id);
  case CODE_msg_detailed_info:
    return work_detailed_info (TLS, c, msg_id);
  case CODE_msg_new_detailed_info:
    return work_new_detailed_info (TLS, c, msg_id);
  case CODE_bad_msg_notification:
    return work_bad_msg_notification (TLS, c, msg_id);
  }
  vlogprintf (E_WARNING, "Unknown message: %08x\n", op);
  in_ptr = in_end; // Will not fail due to assertion in_ptr == in_end
  return 0;
}

static struct mtproto_methods mtproto_methods;
void tgls_free_session (struct tgl_state *TLS, struct tgl_session *S);
/*
static char *get_ipv6 (struct tgl_state *TLS, int num) {
  static char res[1<< 10];
  if (TLS->test_mode) {
    switch (num) {
      case 1:
        strcpy (res, TG_SERVER_TEST_IPV6_1);
        break;
      case 2:
        strcpy (res, TG_SERVER_TEST_IPV6_2);
        break;
      case 3:
        strcpy (res, TG_SERVER_TEST_IPV6_3);
        break;
      default:
        assert (0);
    }
  } else {
    switch (num) {
      case 1:
        strcpy (res, TG_SERVER_IPV6_1);
        break;
      case 2:
        strcpy (res, TG_SERVER_IPV6_2);
        break;
      case 3:
        strcpy (res, TG_SERVER_IPV6_3);
        break;
      case 4:
        strcpy (res, TG_SERVER_IPV6_4);
        break;
      case 5:
        strcpy (res, TG_SERVER_IPV6_5);
        break;
      default:
        assert (0);
    }
  }
  return res;
}
*/

static void create_session_connect (struct tgl_state *TLS, struct tgl_session *S) {
  struct tgl_dc *DC = S->dc;

  if (TLS->ipv6_enabled) {
    S->c = TLS->net_methods->create_connection (TLS, DC->options[1]->ip, DC->options[1]->port, S, DC, &mtproto_methods);
  } else {
    S->c = TLS->net_methods->create_connection (TLS, DC->options[0]->ip, DC->options[0]->port, S, DC, &mtproto_methods);
  }
}

static void fail_connection (struct tgl_state *TLS, struct connection *c) {
  struct tgl_session *S = TLS->net_methods->get_session (c);
  TLS->net_methods->free (c);
  create_session_connect (TLS, S);
}

static void fail_session (struct tgl_state *TLS, struct tgl_session *S) {
  vlogprintf (E_NOTICE, "failing session %"_PRINTF_INT64_"d\n", S->session_id);
  struct tgl_dc *DC = S->dc;
  tgls_free_session (TLS, S);
  DC->sessions[0] = NULL;
  tglmp_dc_create_session (TLS, DC);
}

static int process_rpc_message (struct tgl_state *TLS, struct connection *c, struct encrypted_message *enc, int len) {
  const int MINSZ = offsetof (struct encrypted_message, message);
  const int UNENCSZ = offsetof (struct encrypted_message, server_salt);
  vlogprintf (E_DEBUG, "process_rpc_message(), len=%d\n", len);
  if (len < MINSZ || (len & 15) != (UNENCSZ & 15)) {
    vlogprintf (E_WARNING, "Incorrect packet from server. Closing connection\n");
    fail_connection (TLS, c);
    return -1;
  }
  assert (len >= MINSZ && (len & 15) == (UNENCSZ & 15));
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  if (enc->auth_key_id != DC->temp_auth_key_id && enc->auth_key_id != DC->auth_key_id) {
    vlogprintf (E_WARNING, "received msg from dc %d with auth_key_id %"_PRINTF_INT64_"d (perm_auth_key_id %"_PRINTF_INT64_"d temp_auth_key_id %"_PRINTF_INT64_"d). Dropping\n",
    DC->id, enc->auth_key_id, DC->auth_key_id, DC->temp_auth_key_id);
    return 0;
  }
  if (enc->auth_key_id == DC->temp_auth_key_id) {
    assert (enc->auth_key_id == DC->temp_auth_key_id);
    assert (DC->temp_auth_key_id);
    tgl_init_aes_auth (DC->temp_auth_key + 8, enc->msg_key, AES_DECRYPT);
  } else {
    assert (enc->auth_key_id == DC->auth_key_id);
    assert (DC->auth_key_id);
    tgl_init_aes_auth (DC->auth_key + 8, enc->msg_key, AES_DECRYPT);
  }

  int l = tgl_pad_aes_decrypt ((char *)&enc->server_salt, len - UNENCSZ, (char *)&enc->server_salt, len - UNENCSZ);
  assert (l == len - UNENCSZ);

  if (!(!(enc->msg_len & 3) && enc->msg_len > 0 && enc->msg_len <= len - MINSZ && len - MINSZ - enc->msg_len <= 12)) {
    vlogprintf (E_WARNING, "Incorrect packet from server. Closing connection\n");
    fail_connection (TLS, c);
    return -1;
  }
  assert (!(enc->msg_len & 3) && enc->msg_len > 0 && enc->msg_len <= len - MINSZ && len - MINSZ - enc->msg_len <= 12);

  struct tgl_session *S = TLS->net_methods->get_session (c);
  if (!S || S->session_id != enc->session_id) {
    vlogprintf (E_WARNING, "Message to bad session. Drop.\n");
    return 0;
  }

  static unsigned char sha1_buffer[20];
  sha1 ((void *)&enc->server_salt, enc->msg_len + (MINSZ - UNENCSZ), sha1_buffer);
  if (memcmp (&enc->msg_key, sha1_buffer + 4, 16)) {
    vlogprintf (E_WARNING, "Incorrect packet from server. Closing connection\n");
    fail_connection (TLS, c);
    return -1;
  }
  assert (!memcmp (&enc->msg_key, sha1_buffer + 4, 16));

  int this_server_time = enc->msg_id >> 32LL;
  if (!S->received_messages) {
    DC->server_time_delta = this_server_time - get_utime (CLOCK_REALTIME);
    if (DC->server_time_udelta) {
      vlogprintf (E_WARNING, "adjusting CLOCK_MONOTONIC delta to %lf\n",
          DC->server_time_udelta - this_server_time + get_utime (CLOCK_MONOTONIC));
    }
    DC->server_time_udelta = this_server_time - get_utime (CLOCK_MONOTONIC);
  }

  double st = get_server_time (DC);
  if (this_server_time < st - 300 || this_server_time > st + 30) {
    vlogprintf (E_WARNING, "bad msg time: salt = %"_PRINTF_INT64_"d, session_id = %"_PRINTF_INT64_"d, msg_id = %"_PRINTF_INT64_"d, seq_no = %d, st = %lf, now = %lf\n", enc->server_salt, enc->session_id, enc->msg_id, enc->seq_no, st, get_utime (CLOCK_REALTIME));
    fail_session (TLS, S);
    return -1;
  }
  S->received_messages ++;

  if (DC->server_salt != enc->server_salt) {
    DC->server_salt = enc->server_salt;
  }

  assert (this_server_time >= st - 300 && this_server_time <= st + 30);
  //assert (enc->msg_id > server_last_msg_id && (enc->msg_id & 3) == 1);
  vlogprintf (E_DEBUG, "received mesage id %016llx\n", enc->msg_id);
  //server_last_msg_id = enc->msg_id;

  //*(long long *)(longpoll_query + 3) = *(long long *)((char *)(&enc->msg_id) + 0x3c);
  //*(long long *)(longpoll_query + 5) = *(long long *)((char *)(&enc->msg_id) + 0x3c);

  assert (l >= (MINSZ - UNENCSZ) + 8);
  //assert (enc->message[0] == CODE_rpc_result && *(long long *)(enc->message + 1) == client_last_msg_id);

  in_ptr = enc->message;
  in_end = in_ptr + (enc->msg_len / 4);

  if (enc->msg_id & 1) {
    tgln_insert_msg_id (TLS, S, enc->msg_id);
  }
  assert (S->session_id == enc->session_id);

  if (rpc_execute_answer (TLS, c, enc->msg_id) < 0) {
    fail_session (TLS, S);
    return -1;
  }
  assert (in_ptr == in_end);
  return 0;
}


static int rpc_execute (struct tgl_state *TLS, struct connection *c, int op, int len) {
  struct tgl_dc *DC = TLS->net_methods->get_dc (c);

  if (len >= MAX_RESPONSE_SIZE/* - 12*/ || len < 0/*12*/) {
    vlogprintf (E_WARNING, "answer too long (%d bytes), skipping\n", len);
    return 0;
  }

  int Response_len = len;

  static char Response[MAX_RESPONSE_SIZE];
  vlogprintf (E_DEBUG, "Response_len = %d\n", Response_len);
  assert (TLS->net_methods->read_in (c, Response, Response_len) == Response_len);

#if !defined(WIN32) || !defined(_WIN32)
  setsockopt(c->fd, IPPROTO_TCP, TCP_QUICKACK, (int[]) { 0 }, 4);
#endif
#if !defined(__MACH__) && !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined (__CYGWIN__)
//  setsockopt (c->fd, IPPROTO_TCP, TCP_QUICKACK, (int[]){0}, 4);
#endif
  int o = DC->state;
  //if (DC->flags & 1) { o = st_authorized;}
  if (o != st_authorized) {
    vlogprintf (E_DEBUG, "%s: state = %d\n", __func__, o);
  }
  switch (o) {
  case st_reqpq_sent:
    process_respq_answer (TLS, c, Response/* + 8*/, Response_len/* - 12*/, 0);
    return 0;
  case st_reqdh_sent:
    process_dh_answer (TLS, c, Response/* + 8*/, Response_len/* - 12*/, 0);
    return 0;
  case st_client_dh_sent:
    process_auth_complete (TLS, c, Response/* + 8*/, Response_len/* - 12*/, 0);
    return 0;
  case st_reqpq_sent_temp:
    process_respq_answer (TLS, c, Response/* + 8*/, Response_len/* - 12*/, 1);
    return 0;
  case st_reqdh_sent_temp:
    process_dh_answer (TLS, c, Response/* + 8*/, Response_len/* - 12*/, 1);
    return 0;
  case st_client_dh_sent_temp:
    process_auth_complete (TLS, c, Response/* + 8*/, Response_len/* - 12*/, 1);
    return 0;
  case st_authorized:
    if (op < 0 && op >= -999) {
      vlogprintf (E_WARNING, "Server error %d\n", op);
    } else {
      return process_rpc_message (TLS, c, (void *)(Response/* + 8*/), Response_len/* - 12*/);
    }
    return 0;
  default:
    vlogprintf (E_ERROR, "fatal: cannot receive answer in state %d\n", DC->state);
    exit (2);
  }

  return 0;
}


static int tc_close (struct tgl_state *TLS, struct connection *c, int who) {
  vlogprintf (E_DEBUG, "outbound rpc connection from dc #%d : closing by %d\n", TLS->net_methods->get_dc(c)->id, who);
  return 0;
}

static void mpc_on_get_config (struct tgl_state *TLS, void *extra, int success) {
  assert (success);
  struct tgl_dc *DC = extra;
  DC->flags |= 4;
}

static int tc_becomes_ready (struct tgl_state *TLS, struct connection *c) {
  vlogprintf (E_NOTICE, "outbound rpc connection from dc #%d becomed ready\n", TLS->net_methods->get_dc(c)->id);
  //char byte = 0xef;
  //assert (TLS->net_methods->write_out (c, &byte, 1) == 1);
  //TLS->net_methods->flush_out (c);

  struct tgl_dc *DC = TLS->net_methods->get_dc (c);
  if (DC->flags & 1) { DC->state = st_authorized; }
  int o = DC->state;
  if (o == st_authorized && !TLS->enable_pfs) {
    DC->temp_auth_key_id = DC->auth_key_id;
    memcpy (DC->temp_auth_key, DC->auth_key, 256);
    DC->flags |= 2;
  }
  switch (o) {
  case st_init:
    send_req_pq_packet (TLS, c);
    break;
  case st_authorized:
    if (!(DC->flags & 2)) {
      assert (TLS->enable_pfs);
      if (!DC->temp_auth_key_id) {
        assert (!DC->temp_auth_key_id);
        create_temp_auth_key (TLS, c);
      } else {
        bind_temp_auth_key (TLS, c);
      }
    } else if (!(DC->flags & 4)) {
      tgl_do_help_get_config_dc (TLS, DC, mpc_on_get_config, DC);
    }
    break;
  default:
    vlogprintf (E_DEBUG, "c_state = %d\n", DC->state);
    DC->state = st_init; // previous connection was reset
    send_req_pq_packet (TLS, c);
    break;
  }
  return 0;
}

static int rpc_becomes_ready (struct tgl_state *TLS, struct connection *c) {
  return tc_becomes_ready (TLS, c);
}

static int rpc_close (struct tgl_state *TLS, struct connection *c) {
  return tc_close (TLS, c, 0);
}


#define RANDSEED_PASSWORD_FILENAME     NULL
#define RANDSEED_PASSWORD_LENGTH       0
void tglmp_on_start (struct tgl_state *TLS) {
  tgl_prng_seed (TLS, RANDSEED_PASSWORD_FILENAME, RANDSEED_PASSWORD_LENGTH);

  int i;
  int ok = 0;
  for (i = 0; i < TLS->rsa_key_num; i++) {
    char *key = TLS->rsa_key_list[i];
    RSA *res = rsa_load_public_key (TLS, key);
    if (!res) {
      vlogprintf (E_WARNING, "Can not load key %s\n", key);
      TLS->rsa_key_loaded[i] = NULL;
    } else {
      ok = 1;
      TLS->rsa_key_loaded[i] = res;
      TLS->rsa_key_fingerprint[i] = tgl_do_compute_rsa_key_fingerprint (res);
    }
  }

  if (!ok) {
    vlogprintf (E_ERROR, "No public keys found\n");
    exit (1);
  }
}

void tgl_dc_authorize (struct tgl_state *TLS, struct tgl_dc *DC) {
  //c_state = 0;
  if (!DC->sessions[0]) {
    tglmp_dc_create_session (TLS, DC);
  }
  vlogprintf (E_DEBUG, "Starting authorization for DC #%d\n", DC->id);
  //net_loop (0, auth_ok);
}

#define long_cmp(a,b) ((a) > (b) ? 1 : (a) == (b) ? 0 : -1)
DEFINE_TREE(long,long long,long_cmp,0)

static int send_all_acks (struct tgl_state *TLS, struct tgl_session *S) {
  clear_packet ();
  out_int (CODE_msgs_ack);
  out_int (CODE_vector);
  out_int (tree_count_long (S->ack_tree));
  while (S->ack_tree) {
    long long x = tree_get_min_long (S->ack_tree);
    out_long (x);
    S->ack_tree = tree_delete_long (S->ack_tree, x);
  }
  tglmp_encrypt_send_message (TLS, S->c, packet_buffer, packet_ptr - packet_buffer, 0);
  return 0;
}

static void send_all_acks_gateway (struct tgl_state *TLS, void *arg) {
  send_all_acks (TLS, arg);
}


void tgln_insert_msg_id (struct tgl_state *TLS, struct tgl_session *S, long long id) {
  if (!S->ack_tree) {
    TLS->timer_methods->insert (S->ev, ACK_TIMEOUT);
  }
  if (!tree_lookup_long (S->ack_tree, id)) {
    S->ack_tree = tree_insert_long (S->ack_tree, id, lrand48 ());
  }
}

//extern struct tgl_dc *DC_list[];


static void regen_temp_key_gw (struct tgl_state *TLS, void *arg) {
  tglmp_regenerate_temp_auth_key (TLS, arg);
}

struct tgl_dc *tglmp_alloc_dc (struct tgl_state *TLS, int flags, int id, char *ip, int port) {
  //assert (!TLS->DC_list[id]);

  if (!TLS->DC_list[id]) {
    struct tgl_dc *DC = talloc0 (sizeof (*DC));
    DC->id = id;
    TLS->DC_list[id] = DC;
    if (id > TLS->max_dc_num) {
      TLS->max_dc_num = id;
    }
    DC->ev = TLS->timer_methods->alloc (TLS, regen_temp_key_gw, DC);
    TLS->timer_methods->insert (DC->ev, 0);
  }

  struct tgl_dc *DC = TLS->DC_list[id];

  struct tgl_dc_option *O = DC->options[flags & 3];

  struct tgl_dc_option *O2 = O;
  while (O2) {
    if (!strcmp (O2->ip, ip)) {
      tfree_str (ip);
      return DC;
    }
    O2 = O2->next;
  }

  struct tgl_dc_option *T = talloc (sizeof (*T));
  T->ip = ip;
  T->port = port;
  T->next = O;
  DC->options[flags & 3] = T;


  return DC;
}

static struct mtproto_methods mtproto_methods = {
  .execute = rpc_execute,
  .ready = rpc_becomes_ready,
  .close = rpc_close
};

void tglmp_dc_create_session (struct tgl_state *TLS, struct tgl_dc *DC) {
  struct tgl_session *S = talloc0 (sizeof (*S));
  assert (RAND_pseudo_bytes ((unsigned char *) &S->session_id, 8) >= 0);
  S->dc = DC;
  //S->c = TLS->net_methods->create_connection (TLS, DC->ip, DC->port, S, DC, &mtproto_methods);

  create_session_connect (TLS, S);
  S->ev = TLS->timer_methods->alloc (TLS, send_all_acks_gateway, S);
  assert (!DC->sessions[0]);
  DC->sessions[0] = S;
}

void tgl_do_send_ping (struct tgl_state *TLS, struct connection *c) {
  int x[3];
  x[0] = CODE_ping;
  *(long long *)(x + 1) = lrand48 () * (1ll << 32) + lrand48 ();
  tglmp_encrypt_send_message (TLS, c, x, 3, 0);
}

void tgl_dc_iterator (struct tgl_state *TLS, void (*iterator)(struct tgl_dc *DC)) {
  int i;
  for (i = 0; i <= TLS->max_dc_num; i++) {
    iterator (TLS->DC_list[i]);
  }
}

void tgl_dc_iterator_ex (struct tgl_state *TLS, void (*iterator)(struct tgl_dc *DC, void *extra), void *extra) {
  int i;
  for (i = 0; i <= TLS->max_dc_num; i++) {
    iterator (TLS->DC_list[i], extra);
  }
}


void tglmp_regenerate_temp_auth_key (struct tgl_state *TLS, struct tgl_dc *DC) {
  DC->flags &= ~6;
  DC->temp_auth_key_id = 0;
  memset (DC->temp_auth_key, 0, 256);

  if (!DC->sessions[0]) {
    tgl_dc_authorize (TLS, DC);
    return;
  }


  struct tgl_session *S = DC->sessions[0];
  tglt_secure_random (&S->session_id, 8);
  S->seq_no = 0;

  TLS->timer_methods->remove (S->ev);
  S->ack_tree = tree_clear_long (S->ack_tree);

  if (DC->state != st_authorized) {
    return;
  }

  if (S->c) {
    create_temp_auth_key (TLS, S->c);
  }
}

void tgls_free_session (struct tgl_state *TLS, struct tgl_session *S) {
  S->ack_tree = tree_clear_long (S->ack_tree);
  if (S->ev) { TLS->timer_methods->free (S->ev); }
  if (S->c) {
    TLS->net_methods->free (S->c);
  }
  tfree (S, sizeof (*S));
}

void tgls_free_dc (struct tgl_state *TLS, struct tgl_dc *DC) {
  //if (DC->ip) { tfree_str (DC->ip); }

  struct tgl_session *S = DC->sessions[0];
  if (S) { tgls_free_session (TLS, S); }

  if (DC->ev) { TLS->timer_methods->free (DC->ev); }
  tfree (DC, sizeof (*DC));
}

void tgls_free_pubkey (struct tgl_state *TLS) {
  int i;
  for (i = 0; i < TLS->rsa_key_num; i++) {
    if (TLS->rsa_key_loaded[i]) {
      RSA_free (TLS->rsa_key_loaded[i]);
      TLS->rsa_key_loaded[i] = NULL;
    }
  }
}
