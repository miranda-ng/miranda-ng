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
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <signal_protocol.h>
#include <signal_protocol_types.h>
#include <key_helper.h>
#include <session_builder.h>
#include <session_cipher.h>

//c++
#include <cstddef>
#include <map>

namespace utils {
	// code from http://stackoverflow.com/questions/3368883/how-does-this-size-of-array-template-function-work
	template <std::size_t N>
	struct type_of_size
	{
		typedef char type[N];
	};

	template <typename T, std::size_t Size>
	typename type_of_size<Size>::type& sizeof_array_helper(T(&)[Size]);

#define _countof_portable(pArray) sizeof(sizeof_array_helper(pArray))
}

using namespace utils;

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
		ratchet_identity_key_pair *device_key;
	};

	omemo_device* create_device()
	{
		omemo_device *dev = (omemo_device*)mir_alloc(sizeof(omemo_device));
		for (dev->id = 0; dev->id == 0;)
		{
			Utils_GetRandom((void*)&(dev->id), 4);
		}
		
		if (signal_protocol_key_helper_generate_identity_key_pair(&(dev->device_key), global_context))
		{
			//TODO: handle error
		}

		return dev;
	}
	bool IsFirstRun(CJabberProto *proto)
	{
		//TODO: more sanity checks
		//TODO: check and if necessary refresh prekeys
		int id = proto->getDword("OmemoDeviceId", 0);
		if (id == 0)
			return true;
		ptrA buf(proto->getStringA("OmemoDevicePublicKey"));
		if (!buf || !buf[0])
			return true;
		ptrA buf2(proto->getStringA("OmemoDevicePrivateKey")); //ptrA reinitialization always return "" or random trash
		if (!buf2 || !buf2[0])
			return true;

		return false;
	}
	void RefreshDevice(CJabberProto *proto)
	{
		//generate and save device id
		omemo_device *new_dev = create_device();
		proto->setDword("OmemoDeviceId", new_dev->id);
		//generate and save device key
		ec_public_key *public_key = ratchet_identity_key_pair_get_public(new_dev->device_key);
		signal_buffer *key_buf;
		ec_public_key_serialize(&key_buf, public_key);
		char *key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		proto->setString("OmemoDevicePublicKey", key);
		mir_free(key);
		signal_buffer_free(key_buf);
		ec_private_key *private_key = ratchet_identity_key_pair_get_private(new_dev->device_key);
		ec_private_key_serialize(&key_buf, private_key);
		key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		proto->setString("OmemoDevicePrivateKey", key);
		mir_free(key);
		signal_buffer_free(key_buf);

		//TODO: is it required to resend bundle everytime with device ?

		//generate and save signed pre key

		session_signed_pre_key* signed_pre_key;
		signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, new_dev->device_key, 1, time(0), global_context);
		SIGNAL_UNREF(new_dev->device_key);
		ec_key_pair *signed_pre_key_pair =  session_signed_pre_key_get_key_pair(signed_pre_key);
		public_key = ec_key_pair_get_public(signed_pre_key_pair);
		ec_public_key_serialize(&key_buf, public_key);
		key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		proto->setString("OmemoSignedPreKeyPublic", key);
		mir_free(key);
		signal_buffer_free(key_buf);
		private_key = ec_key_pair_get_private(signed_pre_key_pair);
		ec_private_key_serialize(&key_buf, private_key);
		key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		proto->setString("OmemoSignedPreKeyPrivate", key);
		mir_free(key);
		signal_buffer_free(key_buf);
		char *signature = mir_base64_encode(session_signed_pre_key_get_signature(signed_pre_key), (unsigned int)session_signed_pre_key_get_signature_len(signed_pre_key));
		proto->setString("OmemoSignedPreKeySignature", signature);
		mir_free(signature);

		//generate and save pre keys set

		signal_protocol_key_helper_pre_key_list_node *keys_root, *it;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, 0, 100, global_context);
		it = keys_root;
		char setting_name[64];
		for (int i = 0; it; it = signal_protocol_key_helper_key_list_next(it), i++)
		{
			session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
			ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(pre_key);
			public_key = ec_key_pair_get_public(pre_key_pair);
			ec_public_key_serialize(&key_buf, public_key);
			key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
			mir_snprintf(setting_name, "OmemoPreKey%dPublic", i);
			proto->setString(setting_name, key);
			mir_free(key);
			signal_buffer_free(key_buf);
			private_key = ec_key_pair_get_private(pre_key_pair);
			ec_private_key_serialize(&key_buf, private_key);
			key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
			mir_snprintf(setting_name, "OmemoPreKey%dPrivate", i);
			proto->setString(setting_name, key);
			mir_free(key);
			signal_buffer_free(key_buf);

		}
		signal_protocol_key_helper_key_list_free(keys_root);

	}
	DWORD GetOwnDeviceId(CJabberProto *proto)
	{
		DWORD own_id = proto->getDword("OmemoDeviceId", 0);
		if (own_id == 0)
		{
			proto->OmemoInitDevice();
			own_id = proto->getDword("OmemoDeviceId", 0);
		}
		return own_id;
	}

	//session related libsignal api
	struct omemo_session_jabber_internal_ptrs
	{
		session_builder *builder;
		session_cipher *cipher;
		signal_protocol_store_context *store_context;
	};
	std::map<MCONTACT, omemo_session_jabber_internal_ptrs> sessions_internal;
	void clean_sessions()
	{
		for (std::map<MCONTACT, omemo_session_jabber_internal_ptrs>::iterator i = sessions_internal.begin(), end = sessions_internal.end(); i != end; ++i)
		{
			session_cipher_free(i->second.cipher);
			session_builder_free(i->second.builder);
			signal_protocol_store_context_destroy(i->second.store_context);
		}
	}

	//signal_protocol_session_store callbacks follow
	/*
	
ghazan
[20:35:26]
db_set_blob

db_get_blob
*/

	int load_session_func(signal_buffer **record, const signal_protocol_address *address, void *user_data)
	{
		//TODO:
		return 0; //failure
	}

	int get_sub_device_sessions_func(signal_int_list **sessions, const char *name, size_t name_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int store_session_func(const signal_protocol_address *address, uint8_t *record, size_t record_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int contains_session_func(const signal_protocol_address *address, void *user_data)
	{
		//TODO:
		return 0; //not exist
	}

	int delete_session_func(const signal_protocol_address *address, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int delete_all_sessions_func(const char *name, size_t name_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	void destroy_func(void *user_data)
	{

	}

	//signal_protocol_pre_key_store callback follow
	int load_pre_key(signal_buffer **record, uint32_t pre_key_id, void *user_data)
	{
		//TODO:
		return SG_ERR_INVALID_KEY_ID; //failure
	}

	int store_pre_key(uint32_t pre_key_id, uint8_t *record, size_t record_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int contains_pre_key(uint32_t pre_key_id, void *user_data)
	{
		//TODO:
		return 0; //not found
	}

	int remove_pre_key(uint32_t pre_key_id, void *user_data)
	{
		//TODO:
		return -1; //failure
	}
	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	//signal_protocol_signed_pre_key_store callbacks follow

	int load_signed_pre_key(signal_buffer **record, uint32_t signed_pre_key_id, void *user_data)
	{
		//TODO:
		return SG_ERR_INVALID_KEY_ID; //failure
	}

	int store_signed_pre_key(uint32_t signed_pre_key_id, uint8_t *record, size_t record_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int contains_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
	{
		//TODO:
		return 0; //not found
	}

	int remove_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	//signal_protocol_identity_key_store callbacks follow

	int get_identity_key_pair(signal_buffer **public_data, signal_buffer **private_data, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int get_local_registration_id(void *user_data, uint32_t *registration_id)
	{
		//TODO:
		return -1; //failure
	}

	int save_identity(const signal_protocol_address *address, uint8_t *key_data, size_t key_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}

	int is_trusted_identity(const signal_protocol_address *address, uint8_t *key_data, size_t key_len, void *user_data)
	{
		//TODO:
		return -1; //failure
	}
	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	bool create_session_store(MCONTACT hContact)
	{
		/* Create the data store context, and add all the callbacks to it */
		//TODO: validation of functions return codes
		signal_protocol_store_context *store_context;
		signal_protocol_store_context_create(&store_context, global_context);
		signal_protocol_session_store ss;
		ss.contains_session_func = &contains_session_func;
		ss.delete_all_sessions_func = &delete_all_sessions_func;
		ss.delete_session_func = &delete_session_func;
		ss.destroy_func = &destroy_func;
		ss.get_sub_device_sessions_func = &get_sub_device_sessions_func;
		ss.load_session_func = &load_session_func;
		ss.store_session_func = &store_session_func;
		ss.user_data = (void*)hContact;
		signal_protocol_store_context_set_session_store(store_context, &ss);
		signal_protocol_pre_key_store sp;
		sp.contains_pre_key = &contains_pre_key;
		sp.destroy_func = &destroy_func;
		sp.load_pre_key = &load_pre_key;
		sp.remove_pre_key = &remove_pre_key;
		sp.store_pre_key = &store_pre_key;
		sp.user_data = (void*)hContact;
		signal_protocol_store_context_set_pre_key_store(store_context, &sp);
		signal_protocol_signed_pre_key_store ssp;
		ssp.contains_signed_pre_key = &contains_signed_pre_key;
		ssp.destroy_func = &destroy_func;
		ssp.load_signed_pre_key = &load_signed_pre_key;
		ssp.remove_signed_pre_key = &remove_signed_pre_key;
		ssp.store_signed_pre_key = &store_signed_pre_key;
		ssp.user_data = (void*)hContact;
		signal_protocol_store_context_set_signed_pre_key_store(store_context, &ssp);
		signal_protocol_identity_key_store sip;
		sip.destroy_func = &destroy_func;
		sip.get_identity_key_pair = &get_identity_key_pair;
		sip.get_local_registration_id = &get_local_registration_id;
		sip.is_trusted_identity = &is_trusted_identity;
		sip.save_identity = &save_identity;
		sip.user_data = (void*)hContact;
		signal_protocol_store_context_set_identity_key_store(store_context, &sip);

		sessions_internal[hContact].store_context = store_context;

		return true; //success
	}
	bool build_session(MCONTACT hContact, LPCTSTR jid, LPCTSTR id)
	{
		/* Instantiate a session_builder for a recipient address. */
		char *jid_str = mir_u2a(jid);
		int dev_id = _wtoi(id);
		signal_protocol_address address = {
		jid_str, mir_strlen(jid_str), dev_id
		};

		session_builder *builder;
		session_builder_create(&builder, sessions_internal[hContact].store_context, &address, global_context);

		sessions_internal[hContact].builder = builder;

		mir_free(jid_str);

		return false; //not finished yet
		
		/*
		int session_pre_key_bundle_create(session_pre_key_bundle **bundle,
        uint32_t registration_id, int device_id, uint32_t pre_key_id,
        ec_public_key *pre_key_public,
        uint32_t signed_pre_key_id, ec_public_key *signed_pre_key_public,
        const uint8_t *signed_pre_key_signature_data, size_t signed_pre_key_signature_len,
        ec_public_key *identity_key);
	*/

		/* Build a session with a pre key retrieved from the server. */
	//	session_builder_process_pre_key_bundle(builder, retrieved_pre_key);

		/* Create the session cipher and encrypt the message */
		session_cipher *cipher;
		session_cipher_create(&cipher, sessions_internal[hContact].store_context, &address, global_context);
		sessions_internal[hContact].cipher = cipher;

		return true; //success

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

/*	PROTORECVEVENT recv = { 0 };
	recv.timestamp = (DWORD)msgTime;
	recv.szMessage = buf;
	recv.lParam = (LPARAM)((pFromResource != NULL && m_options.EnableRemoteControl) ? pFromResource->m_tszResourceName : 0);
	ProtoChainRecvMsg(hContact, &recv); */

}

void CJabberProto::OmemoHandleDeviceList(HXML node)
{
	if (!node)
		return;
	HXML message = xmlGetParent(node);
	message = xmlGetParent(message);
	LPCTSTR jid = XmlGetAttrValue(message, L"from");
	MCONTACT hContact = HContactFromJID(jid);
	node = XmlGetChild(node, "item"); //get <item> node
	if (!node)
		return;
	node = XmlGetChildByTag(node, L"list", L"xmlns", JABBER_FEAT_OMEMO); //<list xmlns = 'urn:xmpp:omemo:0'>
	if (!node)
		return;
	bool own_jid = false;
	if (wcsstr(m_ThreadInfo->fullJID, jid))
		own_jid = true;
	DWORD current_id;
	LPCTSTR current_id_str;
	if (own_jid)
	{
		//check if our device exist
		bool own_device_listed = false;
		DWORD own_id = omemo::GetOwnDeviceId(this);
		char setting_name[64];
		HXML list_item;
		int i = 0;
		for (int p = 1; (list_item = XmlGetNthChild(node, L"device", p)) != NULL; p++, i++)
		{
			current_id_str = xmlGetAttrValue(list_item, L"id");
			current_id = _wtoi(current_id_str);
			if (current_id == own_id)
				own_device_listed = true;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			setDword(setting_name, current_id);
		}
		i++;
		DWORD val = 0;
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		val = getDword(setting_name, 0);
		while (val)
		{
			delSetting(setting_name);
			i++;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			val = getDword(setting_name, 0);
		}
		if (!own_device_listed)
			OmemoAnnounceDevice();
	}
	else
	{
		//store device id's
		char setting_name[64];
		HXML list_item;
		int i = 0;
		for (int p = 1; (list_item = XmlGetNthChild(node, L"device", p)) != NULL; p++, i++)
		{
			current_id_str = xmlGetAttrValue(list_item, L"id");
			current_id = _wtoi(current_id_str);
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			setDword(hContact, setting_name, current_id);
		}
		i++;
		DWORD val = 0;
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		val = getDword(hContact, setting_name, 0);
		while (val)
		{
			delSetting(hContact, setting_name);
			i++;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			val = getDword(hContact, setting_name, 0);
		}

	}
}



void CJabberProto::OmemoAnnounceDevice()
{
	// check "OmemoDeviceId%d" for own id and send  updated list if not exist
	DWORD own_id = omemo::GetOwnDeviceId(this);

	char setting_name[64];
	for (int i = 0;; ++i) {
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		DWORD val = getDword(setting_name);
		if (val == 0)
			break;
		if (val == own_id)
			return; // nothing to do, list is fresh enough
	}

	// add own device id
	// construct node
	wchar_t szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq(L"set", SerialNext()); 
	iq << XATTR(L"from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));
	HXML publish_node = iq << XCHILDNS(L"pubsub", L"http://jabber.org/protocol/pubsub") << XCHILD(L"publish") << XATTR(L"node", JABBER_FEAT_OMEMO L".devicelist");
	HXML list_node = publish_node << XCHILDNS(L"item") << XCHILDNS(L"list", JABBER_FEAT_OMEMO);

	for (int i = 0; ; ++i) {
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		DWORD val = getDword(setting_name);
		if (val == 0)
			break;

		list_node << XCHILD(L"device") << XATTRI(L"id", val);
	}
	list_node << XCHILD(L"device") << XATTRI(L"id", own_id);

	// send device list back
	//TODOL handle response
	m_ThreadInfo->send(iq);
}

void CJabberProto::OmemoSendBundle()
{
	// get own device id
	DWORD own_id = omemo::GetOwnDeviceId(this);

	// construct bundle node
	wchar_t szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq(L"set", SerialNext());
	iq << XATTR(L"from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));

	HXML publish_node = iq << XCHILDNS(L"pubsub", L"http://jabber.org/protocol/pubsub") << XCHILD(L"publish");
	{
		wchar_t attr_val[128];
		mir_snwprintf(attr_val, L"%s.bundles:%d", JABBER_FEAT_OMEMO, own_id);
		publish_node << XATTR(L"node", attr_val);
	}
	HXML bundle_node = publish_node << XCHILD(L"item") << XCHILDNS(L"bundle", JABBER_FEAT_OMEMO);

	// add signed pre key public
	bundle_node << XCHILD(L"signedPreKeyPublic", ptrW(getWStringA("OmemoSignedPreKeyPublic"))) << XATTR(L"signedPreKeyId", L"1");

	//add pre key signature
	bundle_node << XCHILD(L"signedPreKeySignature", ptrW(getWStringA("OmemoSignedPreKeySignature")));

	//add identity key
	//it is must be a public key right ?, standart is a bit confusing...
	bundle_node << XCHILD(L"identityKey", ptrW(getWStringA("OmemoDevicePublicKey")));

	//add prekeys
	HXML prekeys_node = XmlAddChild(bundle_node, L"prekeys");

	char setting_name[64];
	for (int i = 0;; i++) {
		mir_snprintf(setting_name, "OmemoPreKey%dPublic", i);
		ptrW val(getWStringA(setting_name));
		if (val == nullptr)
			break;

		prekeys_node << XCHILD(L"preKeyPublic", val) << XATTRI(L"preKeyId", i + 1);
	}

	// send bundle
	//TODOL handle response
	m_ThreadInfo->send(iq);
}

void CJabberProto::OmemoPublishNodes()
{
	OmemoAnnounceDevice();
	OmemoSendBundle();
}

bool CJabberProto::OmemoCheckSession(MCONTACT hContact)
{
	if (getBool(hContact, "OmemoSessionChecked"))
		return true;
	bool pending_check = false;

	char setting_name[64], setting_name2[64];
	DWORD id = 0;
	bool checked = false;
	int i = 0;
	
	mir_snprintf(setting_name, "OmemoDeviceId%d", i);
	mir_snprintf(setting_name2, "%sChecked", setting_name);
	id = getDword(hContact, setting_name, 0);
	checked = getBool(hContact, setting_name2);
	while (id)
	{
		if (!checked)
		{
			pending_check = true;
			wchar_t szBareJid[JABBER_MAX_JID_LEN];
			XmlNodeIq iq(AddIQ(&CJabberProto::OmemoOnIqResultGetBundle, JABBER_IQ_TYPE_GET));
			iq << XATTR(L"from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));
			wchar_t *jid = ContactToJID(hContact);
			iq << XATTR(L"to", jid);
			HXML items = iq << XCHILDNS(L"pubsub", L"http://jabber.org/protocol/pubsub") << XCHILD(L"items");
			wchar_t bundle[64];
			mir_snwprintf(bundle, 63, L"%s%s%d", JABBER_FEAT_OMEMO, L".bundles:", id);
			XmlAddAttr(items, L"node", bundle);
			m_ThreadInfo->send(iq);
			mir_free(jid);
		}
		i++;
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		mir_snprintf(setting_name2, "%sChecked", setting_name);
		id = getDword(hContact, setting_name, 0);
		checked = getBool(hContact, setting_name2);
	}

	if (!pending_check)
		return true;
	return false;
}

void CJabberProto::OmemoOnIqResultGetBundle(HXML iqNode, CJabberIqInfo * /*pInfo*/)
{
	if (iqNode == NULL)
		return;
	
	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (mir_wstrcmp(type, L"result"))
		return;

	LPCTSTR jid = XmlGetAttrValue(iqNode, L"from");

	HXML pubsub = XmlGetChildByTag(iqNode, L"pubsub", L"xmlns", L"http://jabber.org/protocol/pubsub");
	if (!pubsub)
		return;
	HXML items = XmlGetChild(pubsub, L"items");
	LPCTSTR items_node_val = XmlGetAttrValue(items, L"node");
	LPCTSTR device_id = items_node_val;
	device_id += mir_wstrlen(JABBER_FEAT_OMEMO L".bundles:");
	HXML bundle = XmlGetChild(XmlGetChild(items, L"item"), L"bundle");
	if (!bundle)
		return;
	LPCTSTR signedPreKeyPublic = XmlGetText(XmlGetChild(bundle, L"signedPreKeyPublic"));
	LPCTSTR signedPreKeySignature = XmlGetText(XmlGetChild(bundle, L"signedPreKeySignature"));
	LPCTSTR identityKey = XmlGetText(XmlGetChild(bundle, L"identityKey"));
	HXML prekeys = XmlGetChild(bundle, L"prekeys");
	if (!prekeys)
		return;

	HXML list_item;
	unsigned char key_num = 0;
	while(key_num == 0)
		Utils_GetRandom(&key_num, 1);
	key_num = key_num % 101;

	wchar_t key_num_str[4];
	mir_snwprintf(key_num_str, 3, L"%d", key_num);
	HXML prekey_node;
	for (int p = 1; (prekey_node = XmlGetNthChild(prekeys, L"preKeyPublic", p)) != NULL && p <= key_num; p++)
		;
	if (!prekey_node)
		return;

	LPCTSTR preKeyPublic = XmlGetText(prekey_node);
	if (!preKeyPublic)
		return;

	
	//TODO: we have all required data, we need to create session with device here
	if (!omemo::create_session_store(HContactFromJID(jid)))
		return; //failed to create session store

	if (!omemo::build_session(HContactFromJID(jid), jid, device_id))
		return; //failed to build signal(omemo) session
	
/*
	ciphertext_message *encrypted_message;
	session_cipher_encrypt(cipher, message, message_len, &encrypted_message);
	*/

	/* Get the serialized content and deliver it */
/*	signal_buffer *serialized = ciphertext_message_get_serialized(encrypted_message);

	deliver(signal_buffer_data(serialized), signal_buffer_len(serialized));
	*/

	/* Cleanup */
/*	SIGNAL_UNREF(encrypted_message);
	session_cipher_free(cipher);
	session_builder_free(builder);
	signal_protocol_store_context_destroy(store_context);
	*/



}

void CJabberProto::OmemoEncryptMessage(XmlNode &msg, const wchar_t *msg_text)
{
	//TODO:
}
bool CJabberProto::OmemoIsEnabled(MCONTACT hContact)
{
	//TODO:
	return false;
}
