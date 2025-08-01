#ifndef HEADER_CURL_GTLS_H
#define HEADER_CURL_GTLS_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#include "../curl_setup.h"
#include <curl/curl.h>

#ifdef USE_GNUTLS

#include <gnutls/gnutls.h>
#include "../curlx/timeval.h"

#ifdef HAVE_GNUTLS_SRP
/* the function exists */
#ifdef USE_TLS_SRP
/* the functionality is not disabled */
#define USE_GNUTLS_SRP
#endif
#endif

struct Curl_easy;
struct Curl_cfilter;
struct alpn_spec;
struct ssl_primary_config;
struct ssl_config_data;
struct ssl_peer;
struct ssl_connect_data;
struct Curl_ssl_session;

int Curl_glts_get_ietf_proto(gnutls_session_t session);

struct gtls_shared_creds {
  gnutls_certificate_credentials_t creds;
  char *CAfile; /* CAfile path used to generate X509 store */
  struct curltime time; /* when the shared creds was created */
  size_t refcount;
  BIT(trust_setup); /* x509 anchors + CRLs have been set up */
};

CURLcode Curl_gtls_shared_creds_create(struct Curl_easy *data,
                                       struct gtls_shared_creds **pcreds);
CURLcode Curl_gtls_shared_creds_up_ref(struct gtls_shared_creds *creds);
void Curl_gtls_shared_creds_free(struct gtls_shared_creds **pcreds);

struct gtls_ctx {
  gnutls_session_t session;
  struct gtls_shared_creds *shared_creds;
#ifdef USE_GNUTLS_SRP
  gnutls_srp_client_credentials_t srp_client_cred;
#endif
  CURLcode io_result; /* result of last IO cfilter operation */
  BIT(sent_shutdown);
};

size_t Curl_gtls_version(char *buffer, size_t size);

typedef CURLcode Curl_gtls_ctx_setup_cb(struct Curl_cfilter *cf,
                                        struct Curl_easy *data,
                                        void *user_data);

typedef CURLcode Curl_gtls_init_session_reuse_cb(struct Curl_cfilter *cf,
                                                 struct Curl_easy *data,
                                                 struct alpn_spec *alpns,
                                                 struct Curl_ssl_session *scs,
                                                 bool *do_early_data);

CURLcode Curl_gtls_ctx_init(struct gtls_ctx *gctx,
                            struct Curl_cfilter *cf,
                            struct Curl_easy *data,
                            struct ssl_peer *peer,
                            const struct alpn_spec *alpns,
                            Curl_gtls_ctx_setup_cb *cb_setup,
                            void *cb_user_data,
                            void *ssl_user_data,
                            Curl_gtls_init_session_reuse_cb *sess_reuse_cb);

CURLcode Curl_gtls_client_trust_setup(struct Curl_cfilter *cf,
                                      struct Curl_easy *data,
                                      struct gtls_ctx *gtls);

CURLcode Curl_gtls_verifyserver(struct Curl_easy *data,
                                gnutls_session_t session,
                                struct ssl_primary_config *config,
                                struct ssl_config_data *ssl_config,
                                struct ssl_peer *peer,
                                const char *pinned_key);

/* Extract TLS session and place in cache, if configured. */
CURLcode Curl_gtls_cache_session(struct Curl_cfilter *cf,
                                 struct Curl_easy *data,
                                 const char *ssl_peer_key,
                                 gnutls_session_t session,
                                 curl_off_t valid_until,
                                 const char *alpn,
                                 unsigned char *quic_tp,
                                 size_t quic_tp_len);

/* Report properties of a successful handshake */
void Curl_gtls_report_handshake(struct Curl_easy *data,
                                struct gtls_ctx *gctx);

extern const struct Curl_ssl Curl_ssl_gnutls;

#endif /* USE_GNUTLS */
#endif /* HEADER_CURL_GTLS_H */
