/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

MBinBuffer aesDecrypt(
	const EVP_CIPHER *cipher,
	const uint8_t *key, 
	const uint8_t *iv,
	const void *data, size_t dataLen,
	const void *additionalData, size_t additionalLen)
{
	int tag_len = 0, dec_len = 0, final_len = 0;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);

	if (additionalLen)
		EVP_DecryptUpdate(ctx, nullptr, &tag_len, (uint8_t *)additionalData, (int)additionalLen);

	dataLen -= 16;
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (uint8_t *)data + dataLen);

	MBinBuffer ret;
	uint8_t outbuf[2000];
	for (size_t len = 0; len < dataLen; len += 1024) {
		size_t portionSize = dataLen - len;
		EVP_DecryptUpdate(ctx, outbuf, &dec_len, (uint8_t *)data + len, (int)min(portionSize, 1024));
		ret.append(outbuf, dec_len);
	}

	EVP_DecryptFinal_ex(ctx, outbuf, &final_len);
	if (final_len)
		ret.append(outbuf, final_len);

	EVP_CIPHER_CTX_free(ctx);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

static unsigned char *HKDF_Extract(const EVP_MD *evp_md,
	const unsigned char *salt, size_t salt_len,
	const unsigned char *key, size_t key_len,
	unsigned char *prk, size_t *prk_len)
{
	unsigned int tmp_len;

	if (!HMAC(evp_md, salt, (int)salt_len, key, (int)key_len, prk, &tmp_len))
		return NULL;

	*prk_len = tmp_len;
	return prk;
}

static unsigned char *HKDF_Expand(const EVP_MD *evp_md,
	const unsigned char *prk, size_t prk_len,
	const unsigned char *info, size_t info_len,
	unsigned char *okm, size_t okm_len)
{
	HMAC_CTX *hmac;
	unsigned char *ret = NULL;

	unsigned int i;

	unsigned char prev[EVP_MAX_MD_SIZE];

	size_t done_len = 0, dig_len = EVP_MD_size(evp_md);

	size_t n = okm_len / dig_len;
	if (okm_len % dig_len)
		n++;

	if (n > 255 || okm == NULL)
		return NULL;

	if ((hmac = HMAC_CTX_new()) == NULL)
		return NULL;

	if (!HMAC_Init_ex(hmac, prk, (int)prk_len, evp_md, NULL))
		goto err;

	for (i = 1; i <= n; i++) {
		size_t copy_len;
		const unsigned char ctr = i;

		if (i > 1) {
			if (!HMAC_Init_ex(hmac, NULL, 0, NULL, NULL))
				goto err;

			if (!HMAC_Update(hmac, prev, dig_len))
				goto err;
		}

		if (!HMAC_Update(hmac, info, info_len))
			goto err;

		if (!HMAC_Update(hmac, &ctr, 1))
			goto err;

		if (!HMAC_Final(hmac, prev, NULL))
			goto err;

		copy_len = (done_len + dig_len > okm_len) ?
			okm_len - done_len :
			dig_len;

		memcpy(okm + done_len, prev, copy_len);

		done_len += copy_len;
	}
	ret = okm;

err:
	OPENSSL_cleanse(prev, sizeof(prev));
	HMAC_CTX_free(hmac);
	return ret;
}

unsigned char *HKDF(const EVP_MD *evp_md,
	const unsigned char *salt, size_t salt_len,
	const unsigned char *key, size_t key_len,
	const unsigned char *info, size_t info_len,
	unsigned char *okm, size_t okm_len)
{
	unsigned char prk[EVP_MAX_MD_SIZE];
	unsigned char *ret;
	size_t prk_len;

	if (!HKDF_Extract(evp_md, salt, salt_len, key, key_len, prk, &prk_len))
		return NULL;

	ret = HKDF_Expand(evp_md, prk, prk_len, info, info_len, okm, okm_len);
	OPENSSL_cleanse(prk, sizeof(prk));

	return ret;
}
