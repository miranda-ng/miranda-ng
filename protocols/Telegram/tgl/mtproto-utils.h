#ifndef __MTPROTO_UTILS_H__
#define __MTPROTO_UTILS_H__
#include <openssl/bn.h>
int tglmp_check_DH_params (struct tgl_state *TLS, BIGNUM *p, int g);
int tglmp_check_g_a (struct tgl_state *TLS, BIGNUM *p, BIGNUM *g_a);
int bn_factorize (BIGNUM *pq, BIGNUM *p, BIGNUM *q);
#endif
