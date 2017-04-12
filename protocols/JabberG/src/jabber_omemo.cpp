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
		/*
		as this is called for every jabber account it may cause problemsif one of multiple jabber accounts is deleted in runtime,
		to avoid this kind of problems this map must be defained in CJabberProto,
		but as our silent rules agains stl and heavy constructions i will leave it here for now.
		*/
		for (std::map<MCONTACT, omemo_session_jabber_internal_ptrs>::iterator i = sessions_internal.begin(), end = sessions_internal.end(); i != end; ++i)
		{
			if(i->second.cipher)
				session_cipher_free(i->second.cipher);
			if(i->second.builder) 
				session_builder_free(i->second.builder);
			if(i->second.store_context)
				signal_protocol_store_context_destroy(i->second.store_context);
		}
		sessions_internal.clear();
	}

	//signal_protocol_session_store callbacks follow
	
	struct signal_store_backend_user_data
	{
		MCONTACT hContact;
		CJabberProto *proto;
	};
	int load_session_func(signal_buffer **record, const signal_protocol_address *address, void *user_data)
	{
		/**
		* Returns a copy of the serialized session record corresponding to the
		* provided recipient ID + device ID tuple.
		*
		* @param record pointer to a freshly allocated buffer containing the
		*     serialized session record. Unset if no record was found.
		*     The Signal Protocol library is responsible for freeing this buffer.
		* @param address the address of the remote client
		* @return 1 if the session was loaded, 0 if the session was not found, negative on failure
		*/

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			return 0;
		}
		*record = signal_buffer_create(dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		return 1; //session exist and succesfully loaded
	}

	struct db_enum_settings_sub_cb_data
	{
		signal_store_backend_user_data* user_data;
		unsigned int &arr_size;
		signal_int_list *sessions;
		const char *name;
		size_t name_len;
		db_enum_settings_sub_cb_data(unsigned int &arr_size_) : arr_size(arr_size_)
		{}
		db_enum_settings_sub_cb_data() = delete; //we always need array size
	};

	int db_enum_settings_sub_cb(const char *szSetting, LPARAM lParam)
	{
		db_enum_settings_sub_cb_data* data = (db_enum_settings_sub_cb_data*)lParam;
		if (strstr(szSetting, "OmemoSignalSession_"))
		{
			char *ptr = (char*)szSetting;
			ptr += mir_strlen("OmemoSignalSession_");
			char *current_name = mir_base64_encode((BYTE*)data->name, data->name_len);
			if (strstr(ptr, current_name))
			{
				char *dev_encoded = ptr;
				ptr += strlen(current_name);
				unsigned int len;
				void *dev_tmp = mir_base64_decode(ptr, &len);
				signal_int_list_push_back(data->sessions, *((int *)dev_tmp));
				data->arr_size++;
				mir_free(dev_tmp);
			}
			mir_free(current_name);
		}
		return 0; //?

	}
	int get_sub_device_sessions_func(signal_int_list **sessions, const char *name, size_t name_len, void *user_data)
	{
		/**
		* Returns all known devices with active sessions for a recipient
		*
		* @param pointer to an array that will be allocated and populated with the result
		* @param name the name of the remote client
		* @param name_len the length of the name
		* @return size of the sessions array, or negative on failure
		*/

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		signal_int_list *l = signal_int_list_alloc();
		unsigned int array_size = 0;

		db_enum_settings_sub_cb_data *ud = new db_enum_settings_sub_cb_data(array_size);
		ud->user_data = data;
		ud->sessions = l;
		ud->name = name;
		ud->name_len = name_len;
		db_enum_settings(data->hContact, &db_enum_settings_sub_cb, data->proto->m_szModuleName, (void*)ud);
		*sessions = l;
		delete ud;
		return array_size;
	}

	int store_session_func(const signal_protocol_address *address, uint8_t *record, size_t record_len, void *user_data)
	{
		/**
		* Commit to storage the session record for a given
		* recipient ID + device ID tuple.
		*
		* @param address the address of the remote client
		* @param record pointer to a buffer containing the serialized session
		*     record for the remote client
		* @param record_len length of the serialized session record
		* @return 0 on success, negative on failure
		*/
		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		db_set_blob(data->hContact, data->proto->m_szModuleName, setting_name, record, record_len);
		return 0;

	}

	int contains_session_func(const signal_protocol_address *address, void *user_data)
	{
		/**
		* Determine whether there is a committed session record for a
		* recipient ID + device ID tuple.
		*
		* @param address the address of the remote client
		* @return 1 if a session record exists, 0 otherwise.
		*/
		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			return 0;
		}
		db_free(&dbv);
		return 1;

	}

	int delete_session_func(const signal_protocol_address *address, void *user_data)
	{
		/**
		* Remove a session record for a recipient ID + device ID tuple.
		*
		* @param address the address of the remote client
		* @return 1 if a session was deleted, 0 if a session was not deleted, negative on error
		*/

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		db_unset(data->hContact, data->proto->m_szModuleName, setting_name);
		return 1;
	}

	struct db_enum_settings_del_all_cb_data
	{
		signal_store_backend_user_data* user_data;
		LIST<char> settings; //TODO: check this
		const char *name;
		size_t name_len;
	};

	int db_enum_settings_del_all_cb(const char *szSetting, LPARAM lParam)
	{
		db_enum_settings_del_all_cb_data* data = (db_enum_settings_del_all_cb_data*)lParam;
		if (strstr(szSetting, "OmemoSignalSession_"))
		{
			char *ptr = (char*)szSetting;
			ptr += mir_strlen("OmemoSignalSession_");
			char *current_name = mir_base64_encode((BYTE*)data->name, data->name_len);
			if (strstr(ptr, current_name))
				data->settings.insert(mir_strdup(szSetting));
			mir_free(current_name);
		}

		return 0;//?
	}
	int delete_all_sessions_func(const char *name, size_t name_len, void *user_data)
	{
		/**
		* Remove the session records corresponding to all devices of a recipient ID.
		*
		* @param name the name of the remote client
		* @param name_len the length of the name
		* @return the number of deleted sessions on success, negative on failure
		*/

		//TODO:
		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		db_enum_settings_del_all_cb_data *ud = (db_enum_settings_del_all_cb_data*)mir_alloc(sizeof(db_enum_settings_del_all_cb_data));
		ud->user_data = data;
		ud->name = name;
		ud->name_len = name_len;
		db_enum_settings(data->hContact, &db_enum_settings_del_all_cb, data->proto->m_szModuleName, (void*)ud);
		int count = 0;
		for (int i = 0; i < ud->settings.getCount(); i++)
		{
			db_unset(data->hContact, data->proto->m_szModuleName, ud->settings[i]);
			mir_free(ud->settings[i]);
			count++;
		}
		mir_free(ud);
		return count;
	}

	void destroy_func(void *user_data)
	{
		if (user_data)
		{
			signal_store_backend_user_data* d = (signal_store_backend_user_data*)user_data;
			mir_free(d);
			user_data = nullptr;
		}
	}

	//signal_protocol_pre_key_store callback follow
	int load_pre_key(signal_buffer **record, uint32_t pre_key_id, void *user_data)
	{
		/**
		* Load a local serialized PreKey record.
		*
		* @param record pointer to a newly allocated buffer containing the record,
		*     if found. Unset if no record was found.
		*     The Signal Protocol library is responsible for freeing this buffer.
		* @param pre_key_id the ID of the local serialized PreKey record
		* @retval SG_SUCCESS if the key was found
		* @retval SG_ERR_INVALID_KEY_ID if the key could not be found
		*/

		//TODO:
		return SG_ERR_INVALID_KEY_ID; //failure
	}

	int store_pre_key(uint32_t pre_key_id, uint8_t *record, size_t record_len, void *user_data)
	{
		/**
		* Store a local serialized PreKey record.
		*
		* @param pre_key_id the ID of the PreKey record to store.
		* @param record pointer to a buffer containing the serialized record
		* @param record_len length of the serialized record
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}

	int contains_pre_key(uint32_t pre_key_id, void *user_data)
	{
		/**
		* Determine whether there is a committed PreKey record matching the
		* provided ID.
		*
		* @param pre_key_id A PreKey record ID.
		* @return 1 if the store has a record for the PreKey ID, 0 otherwise
		*/

		//TODO:
		return 0; //not found
	}

	int remove_pre_key(uint32_t pre_key_id, void *user_data)
	{
		/**
		* Delete a PreKey record from local storage.
		*
		* @param pre_key_id The ID of the PreKey record to remove.
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}
	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	//signal_protocol_signed_pre_key_store callbacks follow

	int load_signed_pre_key(signal_buffer **record, uint32_t signed_pre_key_id, void *user_data)
	{
		/**
		* Load a local serialized signed PreKey record.
		*
		* @param record pointer to a newly allocated buffer containing the record,
		*     if found. Unset if no record was found.
		*     The Signal Protocol library is responsible for freeing this buffer.
		* @param signed_pre_key_id the ID of the local signed PreKey record
		* @retval SG_SUCCESS if the key was found
		* @retval SG_ERR_INVALID_KEY_ID if the key could not be found
		*/

		//TODO:
		return SG_ERR_INVALID_KEY_ID; //failure
	}

	int store_signed_pre_key(uint32_t signed_pre_key_id, uint8_t *record, size_t record_len, void *user_data)
	{
		/**
		* Store a local serialized signed PreKey record.
		*
		* @param signed_pre_key_id the ID of the signed PreKey record to store
		* @param record pointer to a buffer containing the serialized record
		* @param record_len length of the serialized record
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}

	int contains_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
	{
		/**
		* Determine whether there is a committed signed PreKey record matching
		* the provided ID.
		*
		* @param signed_pre_key_id A signed PreKey record ID.
		* @return 1 if the store has a record for the signed PreKey ID, 0 otherwise
		*/

		//TODO:
		return 0; //not found
	}

	int remove_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
	{
		/**
		* Delete a SignedPreKeyRecord from local storage.
		*
		* @param signed_pre_key_id The ID of the signed PreKey record to remove.
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}

	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	//signal_protocol_identity_key_store callbacks follow

	int get_identity_key_pair(signal_buffer **public_data, signal_buffer **private_data, void *user_data)
	{
		/**
		* Get the local client's identity key pair.
		*
		* @param public_data pointer to a newly allocated buffer containing the
		*     public key, if found. Unset if no record was found.
		*     The Signal Protocol library is responsible for freeing this buffer.
		* @param private_data pointer to a newly allocated buffer containing the
		*     private key, if found. Unset if no record was found.
		*     The Signal Protocol library is responsible for freeing this buffer.
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}

	int get_local_registration_id(void *user_data, uint32_t *registration_id)
	{
		/**
		* Return the local client's registration ID.
		*
		* Clients should maintain a registration ID, a random number
		* between 1 and 16380 that's generated once at install time.
		*
		* @param registration_id pointer to be set to the local client's
		*     registration ID, if it was successfully retrieved.
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}

	int save_identity(const signal_protocol_address *address, uint8_t *key_data, size_t key_len, void *user_data)
	{
		/**
		* Save a remote client's identity key
		* <p>
		* Store a remote client's identity key as trusted.
		* The value of key_data may be null. In this case remove the key data
		* from the identity store, but retain any metadata that may be kept
		* alongside it.
		*
		* @param address the address of the remote client
		* @param key_data Pointer to the remote client's identity key, may be null
		* @param key_len Length of the remote client's identity key
		* @return 0 on success, negative on failure
		*/

		//TODO:
		return -1; //failure
	}

	int is_trusted_identity(const signal_protocol_address *address, uint8_t *key_data, size_t key_len, void *user_data)
	{
		/**
		* Verify a remote client's identity key.
		*
		* Determine whether a remote client's identity is trusted.  Convention is
		* that the TextSecure protocol is 'trust on first use.'  This means that
		* an identity key is considered 'trusted' if there is no entry for the recipient
		* in the local store, or if it matches the saved key for a recipient in the local
		* store.  Only if it mismatches an entry in the local store is it considered
		* 'untrusted.'
		*
		* @param address the address of the remote client
		* @param identityKey The identity key to verify.
		* @param key_data Pointer to the identity key to verify
		* @param key_len Length of the identity key to verify
		* @return 1 if trusted, 0 if untrusted, negative on failure
		*/

		//TODO:
		return -1; //failure
	}
	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	bool create_session_store(MCONTACT hContact, CJabberProto *proto)
	{
		sessions_internal[hContact].builder = nullptr;
		sessions_internal[hContact].cipher = nullptr;
		sessions_internal[hContact].store_context = nullptr;
		signal_store_backend_user_data *data[4];
		for (int i = 0; i < 4; i++)
		{
			data[i] = (signal_store_backend_user_data*)mir_alloc(sizeof(signal_store_backend_user_data));
			data[i]->hContact = hContact;
			data[i]->proto = proto;
		}
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
		ss.user_data = (void*)data[0];
		signal_protocol_store_context_set_session_store(store_context, &ss);
		signal_protocol_pre_key_store sp;
		sp.contains_pre_key = &contains_pre_key;
		sp.destroy_func = &destroy_func;
		sp.load_pre_key = &load_pre_key;
		sp.remove_pre_key = &remove_pre_key;
		sp.store_pre_key = &store_pre_key;
		sp.user_data = (void*)data[1];
		signal_protocol_store_context_set_pre_key_store(store_context, &sp);
		signal_protocol_signed_pre_key_store ssp;
		ssp.contains_signed_pre_key = &contains_signed_pre_key;
		ssp.destroy_func = &destroy_func;
		ssp.load_signed_pre_key = &load_signed_pre_key;
		ssp.remove_signed_pre_key = &remove_signed_pre_key;
		ssp.store_signed_pre_key = &store_signed_pre_key;
		ssp.user_data = (void*)data[2];
		signal_protocol_store_context_set_signed_pre_key_store(store_context, &ssp);
		signal_protocol_identity_key_store sip;
		sip.destroy_func = &destroy_func;
		sip.get_identity_key_pair = &get_identity_key_pair;
		sip.get_local_registration_id = &get_local_registration_id;
		sip.is_trusted_identity = &is_trusted_identity;
		sip.save_identity = &save_identity;
		sip.user_data = (void*)data[3];
		signal_protocol_store_context_set_identity_key_store(store_context, &sip);

		sessions_internal[hContact].store_context = store_context;

		return true; //success
	}
	bool build_session(MCONTACT hContact, LPCTSTR jid, LPCTSTR dev_id, LPCTSTR key_id, LPCTSTR pre_key_public, LPCTSTR signed_pre_key_id,
		LPCTSTR signed_pre_key_public, LPCTSTR signed_pre_key_signature, LPCTSTR identity_key)
	{
		/* Instantiate a session_builder for a recipient address. */
		char *jid_str = mir_u2a(jid);
		int dev_id_int = _wtoi(dev_id);
		signal_protocol_address address = 
		{
			jid_str,
			mir_strlen(jid_str),
			dev_id_int
		};

		session_builder *builder;
		if (session_builder_create(&builder, sessions_internal[hContact].store_context, &address, global_context) < 0)
		{
			mir_free(jid_str);
			return false; //failure
		}

		sessions_internal[hContact].builder = builder;

		mir_free(jid_str);

		int key_id_int = _wtoi(key_id);

		char *pre_key_a = mir_u2a(pre_key_public);
		unsigned int key_buf_len;
		uint8_t *key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		ec_public_key *prekey;
		curve_decode_point(&prekey, key_buf, key_buf_len, global_context);
		mir_free(pre_key_a);
		mir_free(key_buf); //TODO: check this
		int signed_pre_key_id_int = _wtoi(signed_pre_key_id);
		pre_key_a = mir_u2a(signed_pre_key_public);
		key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		ec_public_key *signed_prekey;
		curve_decode_point(&signed_prekey, key_buf, key_buf_len, global_context);
		mir_free(pre_key_a);
		mir_free(key_buf); //TODO: check this
		//load  identity key
		ec_public_key *identity_key_p;
		pre_key_a = mir_u2a(identity_key);
		key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		curve_decode_point(&identity_key_p, key_buf, key_buf_len, global_context);
		mir_free(pre_key_a);
		mir_free(key_buf); //TODO: check this
		pre_key_a = mir_u2a(signed_pre_key_signature);
		key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		mir_free(pre_key_a);
		session_pre_key_bundle *retrieved_pre_key;
		//what is "registration_id" ?
		session_pre_key_bundle_create(&retrieved_pre_key, 1, dev_id_int, key_id_int, prekey, signed_pre_key_id_int, signed_prekey, key_buf, key_buf_len, identity_key_p);
		mir_free(key_buf); //TODO: check this


		/* Build a session with a pre key retrieved from the server. */
		int ret = session_builder_process_pre_key_bundle(builder, retrieved_pre_key);
		switch (ret)
		{
		case SG_ERR_UNTRUSTED_IDENTITY:
		//TODO: do necessary actions for untrusted identity
		break;
		case SG_ERR_INVALID_KEY:
			return false; //failure
			break;
		default:
			break;

		}

		/* Create the session cipher and encrypt the message */
		session_cipher *cipher;
		if (session_cipher_create(&cipher, sessions_internal[hContact].store_context, &address, global_context) < 0)
			return false; //failure
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
	if (!signedPreKeyPublic)
		return;
	LPCTSTR signedPreKeyId = XmlGetAttrValue(XmlGetChild(bundle, L"signedPreKeyPublic"), L"signedPreKeyId");
	if (!signedPreKeyId)
		return;
	LPCTSTR signedPreKeySignature = XmlGetText(XmlGetChild(bundle, L"signedPreKeySignature"));
	if (!signedPreKeySignature)
		return;
	LPCTSTR identityKey = XmlGetText(XmlGetChild(bundle, L"identityKey"));
	if (!identityKey)
		return;
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
	LPCTSTR preKeyId = XmlGetAttrValue(prekey_node, L"preKeyId");
	if (!preKeyId)
		return;

	
	//TODO: we have all required data, we need to create session with device here
	if (!omemo::create_session_store(HContactFromJID(jid), this))
		return; //failed to create session store

	if (!omemo::build_session(HContactFromJID(jid), jid, device_id, preKeyId, preKeyPublic, signedPreKeyId, signedPreKeyPublic, signedPreKeySignature, identityKey))
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
