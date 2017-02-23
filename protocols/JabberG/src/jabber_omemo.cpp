/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include <signal_protocol.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <signal_protocol_internal.h>

namespace omemo {

	int random_func(uint8_t *data, size_t len, void * /*user_data*/)
	{
		Utils_GetRandom(data, len);
		return 0;
	}

	struct hmac_sha256_ctx {
		uint8_t *key, *data;
		size_t key_len, data_len;
	};

	int hmac_sha256_init_func(void **hmac_context, const uint8_t *key, size_t key_len, void * /*user_data*/)
	{
		hmac_sha256_ctx *ctx = (hmac_sha256_ctx*)mir_alloc(sizeof(hmac_sha256_ctx));
		ctx->key = (uint8_t*)mir_alloc(key_len);
		memcpy(ctx->key, key, key_len);
		ctx->key_len = key_len;
		*hmac_context = ctx;
		return 0;
	}
	int hmac_sha256_update_func(void *hmac_context, const uint8_t *data, size_t data_len, void * /*user_data*/)
	{
		hmac_sha256_ctx *ctx = (hmac_sha256_ctx*)hmac_context;
		ctx->data = (uint8_t*)mir_alloc(data_len);
		memcpy(ctx->data, data, data_len);
		ctx->data_len = data_len;
		return 0;
	}
	int hmac_sha256_final_func(void *hmac_context, signal_buffer **output, void * /*user_data*/)
	{
		hmac_sha256_ctx *ctx = (hmac_sha256_ctx*)hmac_context;
		BYTE hashout[MIR_SHA256_HASH_SIZE];
		mir_hmac_sha256(hashout, ctx->key, ctx->key_len, ctx->data, ctx->data_len);
		signal_buffer *output_buffer = signal_buffer_create(hashout, MIR_SHA256_HASH_SIZE);
		*output = output_buffer;
		return 0;
	}
	void hmac_sha256_cleanup_func(void * hmac_context, void * /*user_data*/)
	{
		hmac_sha256_ctx *ctx = (hmac_sha256_ctx*)hmac_context;
		mir_free(ctx->key);
		mir_free(ctx->data);
	}

	int sha512_digest_init_func(void **digest_context, void * /*user_data*/)
	{

		int result = 0;
		EVP_MD_CTX *ctx;

		ctx = EVP_MD_CTX_create();
		if (!ctx) {
			result = SG_ERR_NOMEM;
			goto complete;
		}

		result = EVP_DigestInit_ex(ctx, EVP_sha512(), 0);
		if (result == 1) {
			result = SG_SUCCESS;
		}
		else {
			result = SG_ERR_UNKNOWN;
		}

	complete:
		if (result < 0) {
			if (ctx) {
				EVP_MD_CTX_destroy(ctx);
			}
		}
		else {
			*digest_context = ctx;
		}
		return result;
	}

	int sha512_digest_update_func(void *digest_context, const uint8_t *data, size_t data_len, void * /*user_data*/)
	{
		EVP_MD_CTX *ctx = (EVP_MD_CTX*)digest_context;

		int result = EVP_DigestUpdate(ctx, data, data_len);

		return (result == 1) ? SG_SUCCESS : SG_ERR_UNKNOWN;
	}

	int sha512_digest_final_func(void *digest_context, signal_buffer **output, void * /*user_data*/)
	{
		int result = 0;
		unsigned char md[EVP_MAX_MD_SIZE];
		unsigned int len = 0;
		EVP_MD_CTX *ctx = (EVP_MD_CTX*)digest_context;

		result = EVP_DigestFinal_ex(ctx, md, &len);
		if (result == 1) {
			result = SG_SUCCESS;
		}
		else {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		result = EVP_DigestInit_ex(ctx, EVP_sha512(), 0);
		if (result == 1) {
			result = SG_SUCCESS;
		}
		else {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		signal_buffer *output_buffer = signal_buffer_create(md, len);
		if (!output_buffer) {
			result = SG_ERR_NOMEM;
			goto complete;
		}

		*output = output_buffer;

	complete:
		return result;
	}

	void sha512_digest_cleanup_func(void *digest_context, void * /*user_data*/)
	{
		EVP_MD_CTX *ctx = (EVP_MD_CTX *)digest_context;
		EVP_MD_CTX_destroy(ctx);
	}

	const EVP_CIPHER *aes_cipher(int cipher, size_t key_len)
	{
		if (cipher == SG_CIPHER_AES_CBC_PKCS5) {
			if (key_len == 16) {
				return EVP_aes_128_cbc();
			}
			else if (key_len == 24) {
				return EVP_aes_192_cbc();
			}
			else if (key_len == 32) {
				return EVP_aes_256_cbc();
			}
		}
		else if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			if (key_len == 16) {
				return EVP_aes_128_ctr();
			}
			else if (key_len == 24) {
				return EVP_aes_192_ctr();
			}
			else if (key_len == 32) {
				return EVP_aes_256_ctr();
			}
		}
		return 0;
	}

	int encrypt_func(signal_buffer **output,
		int cipher,
		const uint8_t *key, size_t key_len,
		const uint8_t *iv, size_t iv_len,
		const uint8_t *plaintext, size_t plaintext_len,
		void * /*user_data*/)
	{
		//TODO: use netlib for log
		int result = 0;
		uint8_t *out_buf = 0;

		const EVP_CIPHER *evp_cipher = aes_cipher(cipher, key_len);
		if (!evp_cipher) {
			//fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
			return SG_ERR_UNKNOWN;
		}

		if (iv_len != 16) {
			//fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
			return SG_ERR_UNKNOWN;
		}

		if (plaintext_len > INT_MAX - EVP_CIPHER_block_size(evp_cipher)) {
			//fprintf(stderr, "invalid plaintext length: %zu\n", plaintext_len);
			return SG_ERR_UNKNOWN;
		}

		EVP_CIPHER_CTX ctx;
		EVP_CIPHER_CTX_init(&ctx);

		result = EVP_EncryptInit_ex(&ctx, evp_cipher, 0, key, iv);
		if (!result) {
			//fprintf(stderr, "cannot initialize cipher\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			result = EVP_CIPHER_CTX_set_padding(&ctx, 0);
			if (!result) {
				//fprintf(stderr, "cannot set padding\n");
				result = SG_ERR_UNKNOWN;
				goto complete;
			}
		}

		out_buf = (uint8_t*)mir_alloc(sizeof(uint8_t) * (plaintext_len + EVP_CIPHER_block_size(evp_cipher)));
		if (!out_buf) {
			//fprintf(stderr, "cannot allocate output buffer\n");
			result = SG_ERR_NOMEM;
			goto complete;
		}

		int out_len = 0;
		result = EVP_EncryptUpdate(&ctx,
			out_buf, &out_len, plaintext, (int)plaintext_len);
		if (!result) {
			//fprintf(stderr, "cannot encrypt plaintext\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		int final_len = 0;
		result = EVP_EncryptFinal_ex(&ctx, out_buf + out_len, &final_len);
		if (!result) {
			//fprintf(stderr, "cannot finish encrypting plaintext\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		*output = signal_buffer_create(out_buf, out_len + final_len);

	complete:
		EVP_CIPHER_CTX_cleanup(&ctx);
		if (out_buf) {
			mir_free(out_buf);
		}
		return result;
	}

	int decrypt_func(signal_buffer **output,
		int cipher,
		const uint8_t *key, size_t key_len,
		const uint8_t *iv, size_t iv_len,
		const uint8_t *ciphertext, size_t ciphertext_len,
		void * /*user_data*/)
	{
		//TODO: use netlib for log
		int result = 0;
		uint8_t *out_buf = 0;

		const EVP_CIPHER *evp_cipher = aes_cipher(cipher, key_len);
		if (!evp_cipher) {
			//fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
			return SG_ERR_INVAL;
		}

		if (iv_len != 16) {
			//fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
			return SG_ERR_INVAL;
		}

		if (ciphertext_len > INT_MAX - EVP_CIPHER_block_size(evp_cipher)) {
			//fprintf(stderr, "invalid ciphertext length: %zu\n", ciphertext_len);
			return SG_ERR_UNKNOWN;
		}

		EVP_CIPHER_CTX ctx;
		EVP_CIPHER_CTX_init(&ctx);

		result = EVP_DecryptInit_ex(&ctx, evp_cipher, 0, key, iv);
		if (!result) {
			///fprintf(stderr, "cannot initialize cipher\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			result = EVP_CIPHER_CTX_set_padding(&ctx, 0);
			if (!result) {
				//fprintf(stderr, "cannot set padding\n");
				result = SG_ERR_UNKNOWN;
				goto complete;
			}
		}

		out_buf = (uint8_t*)mir_alloc(sizeof(uint8_t) * (ciphertext_len + EVP_CIPHER_block_size(evp_cipher)));
		if (!out_buf) {
			//fprintf(stderr, "cannot allocate output buffer\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		int out_len = 0;
		result = EVP_DecryptUpdate(&ctx,
			out_buf, &out_len, ciphertext, (int)ciphertext_len);
		if (!result) {
			//fprintf(stderr, "cannot decrypt ciphertext\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		int final_len = 0;
		result = EVP_DecryptFinal_ex(&ctx, out_buf + out_len, &final_len);
		if (!result) {
			//fprintf(stderr, "cannot finish decrypting ciphertext\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		*output = signal_buffer_create(out_buf, out_len + final_len);

	complete:
		EVP_CIPHER_CTX_cleanup(&ctx);
		if (out_buf) {
			free(out_buf);
		}
		return result;
	}



	mir_cs _signal_cs;
	mir_cslockfull signal_mutex(_signal_cs);

	void lock(void * /*user_data*/)
	{
		signal_mutex.lock();
	}

	void unlock(void * /*user_data*/)
	{
		signal_mutex.unlock();
	}
	signal_context *global_context;

	int init_omemo()
	{
		signal_mutex.unlock(); //fuck...
		signal_context_create(&global_context, NULL);
		signal_crypto_provider provider;
		provider.random_func = &random_func;
		provider.hmac_sha256_init_func = &hmac_sha256_init_func;
		provider.hmac_sha256_update_func = &hmac_sha256_update_func;
		provider.hmac_sha256_final_func = &hmac_sha256_final_func;
		provider.hmac_sha256_cleanup_func = &hmac_sha256_cleanup_func;
		provider.sha512_digest_init_func = &sha512_digest_init_func;
		provider.sha512_digest_update_func = &sha512_digest_update_func;
		provider.sha512_digest_final_func = &sha512_digest_final_func;
		provider.sha512_digest_cleanup_func = &sha512_digest_cleanup_func;
		provider.encrypt_func = &encrypt_func;
		provider.decrypt_func = &decrypt_func;

		if (signal_context_set_crypto_provider(global_context, &provider))
		{
			//TODO: handle error
		}

		if (signal_context_set_locking_functions(global_context, &lock, &unlock))
		{
			//TODO: handle error
		}

		return 0;
	}

	struct omemo_device
	{
		int id;
		ec_key_pair *device_key;
	};

	omemo_device* create_device()
	{
		omemo_device *dev = (omemo_device*)mir_alloc(sizeof(omemo_device));
		for (dev->id = 0; dev->id == 0;)
		{
			Utils_GetRandom((void*)&(dev->id), 4);
		}
		
		if (curve_generate_key_pair(global_context, &(dev->device_key)))
		{
			//TODO: handle error
		}

		return dev;
	}
	bool IsFirstRun(CJabberProto *proto)
	{
		int id = proto->getDword("OmemoDeviceId", 0);
		if (id == 0)
			return true;
		DBVARIANT dbv = { 0 };
		proto->getString("OmemoDevicePublicKey", &dbv);
		if (!dbv.pszVal[0])
		{
			//does it need to free something in DBVARIANT?
			return true;
		}
		proto->getString("OmemoDevicePrivateKey", &dbv);
		if (!dbv.pszVal[0])
		{
			//does it need to free something in DBVARIANT?
			return true;
		}

		return false;
	}
	void RefreshDevice(CJabberProto *proto)
	{
		omemo_device *new_dev = create_device();
		proto->setDword("OmemoDeviceId", new_dev->id);
		ec_public_key *public_key = ec_key_pair_get_public(new_dev->device_key);
		SIGNAL_REF(public_key);
		signal_buffer *key_buf;
		ec_public_key_serialize(&key_buf, public_key);
		char *key = mir_base64_encode(key_buf->data, (unsigned int)key_buf->len);
		proto->setString("OmemoDevicePublicKey", key);
		mir_free(key);
		signal_buffer_free(key_buf);
		ec_private_key *private_key = ec_key_pair_get_private(new_dev->device_key);
		SIGNAL_REF(private_key);
		ec_private_key_serialize(&key_buf, private_key);
		key = mir_base64_encode(key_buf->data, (unsigned int)key_buf->len);
		proto->setString("OmemoDevicePrivateKey", key);
		mir_free(key);
		signal_buffer_free(key_buf);

		SIGNAL_UNREF(new_dev->device_key);

	}

};

void CJabberProto::OmemoInitDevice()
{
	if (omemo::IsFirstRun(this))
		omemo::RefreshDevice(this);

}


void CJabberProto::OmemoHandleMessage(HXML /*node*/)
{
	//TODO: handle "encrypted" node here
}

void CJabberProto::OmemoHandleDeviceList(HXML /*node*/)
{
	//TODO: handle omemo device list event node here
}