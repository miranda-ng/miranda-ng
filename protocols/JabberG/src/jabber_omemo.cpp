/*

Jabber Protocol Plugin for Miranda NG

Copyright (�) 2017 Miranda NG project

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

//TODO: further improovement requirements folllows in priority sequence
/*
 * 1. per-contact encryption settings (enable/disable for one contact)
 * 2. fingerprints/keys management ui
 */

#include "stdafx.h"
#include "jabber_omemo.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <signal_protocol.h>
#include <signal_protocol_types.h>
#include <key_helper.h>
#include <session_builder.h>
#include <session_cipher.h>
#include <protocol.h>

//c++
#include <cstddef>
#include <map>
#include <list>

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
			mir_free(out_buf);
		}
		return result;
	}

	void lock(void *user_data)
	{
		omemo_impl *omi = (omemo_impl*)user_data;
		omi->signal_mutex->lock();
	}

	void unlock(void *user_data)
	{
		omemo_impl *omi = (omemo_impl*)user_data;
		omi->signal_mutex->unlock();
	}


	signal_context *global_context = nullptr;

	struct incomming_message
	{
		incomming_message(HXML x, wchar_t *j, time_t t)
		{
			node = x;
			jid = j;
			msgTime = t;
		}
		HXML node;
		wchar_t *jid;
		time_t msgTime;
	};

	struct outgoing_message
	{
		outgoing_message(MCONTACT h, int u, char* p)
		{
			hContact = h;
			unused_unknown = u;
			pszSrc = p;
		}
		MCONTACT hContact;
		int unused_unknown;
		char* pszSrc;
	};

	struct message_queue
	{
		std::list<incomming_message> incomming_messages;
		std::list<outgoing_message> outgoing_messages;
	};

	struct omemo_session_jabber_internal_ptrs
	{
		session_builder *builder;
		session_cipher *cipher;
		signal_protocol_store_context *store_context;
	};


	omemo_impl::omemo_impl(CJabberProto *p) : proto(p), signal_mutex(nullptr), provider(nullptr)
	{
		if (proto->m_options.UseOMEMO)
			init();
	}
	void omemo_impl::init()
	{
		if (provider && signal_mutex)
			return;
		if (!global_context)
			signal_context_create(&global_context, this);
		signal_mutex = new mir_cslockfull(_signal_cs);
		signal_mutex->unlock(); //fuck...
		provider = new signal_crypto_provider;
		provider->random_func = &random_func;
		provider->hmac_sha256_init_func = &hmac_sha256_init_func;
		provider->hmac_sha256_update_func = &hmac_sha256_update_func;
		provider->hmac_sha256_final_func = &hmac_sha256_final_func;
		provider->hmac_sha256_cleanup_func = &hmac_sha256_cleanup_func;
		provider->sha512_digest_init_func = &sha512_digest_init_func;
		provider->sha512_digest_update_func = &sha512_digest_update_func;
		provider->sha512_digest_final_func = &sha512_digest_final_func;
		provider->sha512_digest_cleanup_func = &sha512_digest_cleanup_func;
		provider->encrypt_func = &encrypt_func;
		provider->decrypt_func = &decrypt_func;

		if (signal_context_set_crypto_provider(global_context, provider))
		{
			proto->debugLogA("Jabber OMEMO: signal_context_set_crypto_provider failed");
			//TODO: handle error
		}

		if (signal_context_set_locking_functions(global_context, &lock, &unlock))
		{
			proto->debugLogA("Jabber OMEMO: signal_context_set_crypto_provider failed");
			//TODO: handle error
		}
		sessions_internal = new std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >;
		message_queue_internal = new message_queue;
		session_checked = new std::map<MCONTACT, bool>;
	}
	omemo_impl::~omemo_impl()
	{
		if (proto->m_options.UseOMEMO)
		{
			deinit();
		}
	}
	void omemo_impl::deinit()
	{
		if (provider && signal_mutex)
		{
			for (std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >::iterator i = ((std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)->begin(),
				end = ((std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)->end(); i != end; ++i)
			{
				for (std::map<unsigned int, omemo_session_jabber_internal_ptrs>::iterator i2 = i->second.begin(), end2 = i->second.end(); i2 != end2; ++i2)
				{
					if (i2->second.cipher)
						session_cipher_free(i2->second.cipher);
					if (i2->second.builder)
						session_builder_free(i2->second.builder);
					if (i2->second.store_context)
						signal_protocol_store_context_destroy(i2->second.store_context);
				}
			}
			((std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)->clear();
			delete (std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal;
			delete signal_mutex;
			delete provider;
			delete message_queue_internal;
			delete session_checked;
			provider = nullptr;
			signal_mutex = nullptr;
		}

	}


	struct omemo_device
	{
		unsigned int id;
		ratchet_identity_key_pair *device_key;
	};

	omemo_device* omemo_impl::create_device()
	{
		omemo_device *dev = (omemo_device*)mir_alloc(sizeof(omemo_device));
		for (dev->id = 0; dev->id == 0;)
		{
			Utils_GetRandom((void*)&(dev->id), 4);
		}
		
		if (signal_protocol_key_helper_generate_identity_key_pair(&(dev->device_key), global_context))
		{
			proto->debugLogA("Jabber OMEMO: signal_protocol_key_helper_generate_identity_key_pair failed");
			//TODO: handle error
		}

		return dev;
	}
	bool omemo_impl::IsFirstRun()
	{
		//TODO: more sanity checks
		//TODO: check and if necessary refresh prekeys
		unsigned int id = proto->getDword("OmemoDeviceId", 0);
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

	unsigned long omemo_impl::GetOwnDeviceId()
	{
		unsigned long own_id = proto->getDword("OmemoDeviceId", 0);
		if (own_id == 0)
		{
			proto->OmemoInitDevice();
			own_id = proto->getDword("OmemoDeviceId", 0);
		}
		return own_id;
	}

	void omemo_impl::RefreshDevice()
	{
		//generate and save device id
		omemo_device *new_dev = create_device();
		proto->setDword("OmemoDeviceId", new_dev->id);
		//generate and save device key
		ec_public_key *public_key = ratchet_identity_key_pair_get_public(new_dev->device_key);
		signal_buffer *key_buf;
		ec_public_key_serialize(&key_buf, public_key);
//		SIGNAL_UNREF(public_key);
		char *key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		char *fingerprint = (char*)mir_alloc((signal_buffer_len(key_buf) * 2) + 1);
		bin2hex(signal_buffer_data(key_buf), signal_buffer_len(key_buf), fingerprint);
		proto->setString("OmemoFingerprintOwn", fingerprint);
		mir_free(fingerprint);
		proto->setString("OmemoDevicePublicKey", key);
		mir_free(key);
		signal_buffer_free(key_buf);
		ec_private_key *private_key = ratchet_identity_key_pair_get_private(new_dev->device_key);
		ec_private_key_serialize(&key_buf, private_key);
//		SIGNAL_UNREF(private_key);
		key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		proto->setString("OmemoDevicePrivateKey", key);
		mir_free(key);
		signal_buffer_free(key_buf);


		//generate and save signed pre key

		session_signed_pre_key* signed_pre_key;
		{
			const unsigned int signed_pre_key_id = 1;
			signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, new_dev->device_key, signed_pre_key_id, time(0), global_context);
			SIGNAL_UNREF(new_dev->device_key);
			signal_buffer *serialized_signed_pre_key;
			session_signed_pre_key_serialize(&serialized_signed_pre_key, signed_pre_key);
			char *setting_name = (char*)mir_alloc(strlen("OmemoSignalSignedPreKey_") + 32);
			mir_snprintf(setting_name, strlen("OmemoSignalSignedPreKey_") + 31, "%s%u%d", "OmemoSignalSignedPreKey_", proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
			db_set_blob(0, proto->m_szModuleName, setting_name, signal_buffer_data(serialized_signed_pre_key), (unsigned int)signal_buffer_len(serialized_signed_pre_key));
//			SIGNAL_UNREF(serialized_signed_pre_key);
			mir_free(setting_name);
		}
		//TODO: store signed_pre_key for libsignal data backend too
		//TODO: dynamic signed pre_key id and setting name ?
		ec_key_pair *signed_pre_key_pair =  session_signed_pre_key_get_key_pair(signed_pre_key);
		public_key = ec_key_pair_get_public(signed_pre_key_pair);
		ec_public_key_serialize(&key_buf, public_key);
//		SIGNAL_UNREF(public_key);
		key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
		proto->setString("OmemoSignedPreKeyPublic", key);
		mir_free(key);
		signal_buffer_free(key_buf);
		char *signature = mir_base64_encode(session_signed_pre_key_get_signature(signed_pre_key), (unsigned int)session_signed_pre_key_get_signature_len(signed_pre_key));
		proto->setString("OmemoSignedPreKeySignature", signature);
		mir_free(signature);

		//generate and save pre keys set

		signal_protocol_key_helper_pre_key_list_node *keys_root, *it;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, 0, 100, global_context);
		it = keys_root;
		char setting_name[64], setting_name2[64];
		for (; it; it = signal_protocol_key_helper_key_list_next(it))
		{
			session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
			uint32_t pre_key_id = session_pre_key_get_id(pre_key);
			{
				signal_buffer *serialized_pre_key;
				session_pre_key_serialize(&serialized_pre_key, pre_key);
				mir_snprintf(setting_name2, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", GetOwnDeviceId(), pre_key_id);
				db_set_blob(0, proto->m_szModuleName, setting_name2, signal_buffer_data(serialized_pre_key), (unsigned int)signal_buffer_len(serialized_pre_key));
				SIGNAL_UNREF(serialized_pre_key);
			}

			ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(pre_key);
			public_key = ec_key_pair_get_public(pre_key_pair);
			ec_public_key_serialize(&key_buf, public_key);
			SIGNAL_UNREF(public_key);
			key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
			mir_snprintf(setting_name, "OmemoPreKey%uPublic", pre_key_id);
			proto->setString(setting_name, key);
			mir_free(key);
			signal_buffer_free(key_buf);
		}
		signal_protocol_key_helper_key_list_free(keys_root);

	}

	//signal_protocol_session_store callbacks follow
	
	struct signal_store_backend_user_data
	{
		MCONTACT hContact;
		unsigned int device_id;
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
		//some sanity checks
		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		if ((unsigned long)address->device_id == 0)
		{
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (invalid device id)");
			return -1;
		}
		if (address->name_len > JABBER_MAX_JID_LEN)
		{
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (invalid address length)");
			return -1;
		}



		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, (unsigned int)(address->name_len + sizeof(int32_t)));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (session does not exist)");
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
	private:		
		db_enum_settings_sub_cb_data(); //we always need array size
	};

	int db_enum_settings_sub_cb(const char *szSetting, LPARAM lParam)
	{
		db_enum_settings_sub_cb_data* data = (db_enum_settings_sub_cb_data*)lParam;
		if (strstr(szSetting, "OmemoSignalSession_"))
		{
			char *ptr = (char*)szSetting;
			ptr += mir_strlen("OmemoSignalSession_");
			char *current_name = mir_base64_encode((BYTE*)data->name, (unsigned int)data->name_len);
			if (strstr(ptr, current_name))
			{
				char *dev_encoded = ptr;
				dev_encoded += strlen(current_name);
				unsigned int len;
				void *dev_tmp = mir_base64_decode(dev_encoded, &len);
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
		char *id_str = mir_base64_encode((BYTE*)id_buf, (unsigned int)(address->name_len + sizeof(int32_t)));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		db_set_blob(data->hContact, data->proto->m_szModuleName, setting_name, record, (unsigned int)record_len); //TODO: check return value
		mir_free(setting_name);
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
		char *id_str = mir_base64_encode((BYTE*)id_buf, (unsigned int)(address->name_len + sizeof(int32_t)));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
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
		char *id_str = mir_base64_encode((BYTE*)id_buf, (unsigned int)(address->name_len + sizeof(int32_t)));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		db_unset(data->hContact, data->proto->m_szModuleName, setting_name);
		mir_free(setting_name);
		return 1;
	}

	struct db_enum_settings_del_all_cb_data
	{
		signal_store_backend_user_data* user_data;
		std::list<char*> settings;
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
			char *current_name = mir_base64_encode((BYTE*)data->name, (unsigned int)data->name_len);
			if (strstr(ptr, current_name))
				data->settings.push_back(mir_strdup(szSetting));
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		db_enum_settings_del_all_cb_data *ud = new db_enum_settings_del_all_cb_data;
		ud->user_data = data;
		ud->name = name;
		ud->name_len = name_len;
		db_enum_settings(data->hContact, &db_enum_settings_del_all_cb, data->proto->m_szModuleName, (void*)ud);
		int count = 0;
		for (std::list<char*>::iterator i = ud->settings.begin(), end = ud->settings.end(); i != end; i++)
		{
			db_unset(data->hContact, data->proto->m_szModuleName, *i);
			mir_free(*i);
			count++;
		}
		delete ud;
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load prekey SG_ERR_INVALID_KEY_ID");
			return SG_ERR_INVALID_KEY_ID;
		}
		*record = signal_buffer_create(dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		return SG_SUCCESS; //key exist and succesfully loaded
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		db_set_blob(0, data->proto->m_szModuleName, setting_name, record, (unsigned int)record_len); //TODO: check return value
		{ //store base64 encoded keys for bundle (private key does not required ?)
			session_pre_key *prekey = nullptr;
			session_pre_key_deserialize(&prekey, record, record_len, global_context); //TODO: handle error
			if (prekey)
			{
				ec_public_key *public_key = nullptr;
				//ec_private_key *private_key = nullptr;
				ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(prekey);
				signal_buffer *key_buf = nullptr;
				char *key = nullptr;
				public_key = ec_key_pair_get_public(pre_key_pair);
				ec_public_key_serialize(&key_buf, public_key);
				SIGNAL_UNREF(public_key);
				key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
				mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "OmemoPreKey%uPublic", pre_key_id);
				data->proto->setString(setting_name, key);
				mir_free(key);
				signal_buffer_free(key_buf);
/*				private_key = ec_key_pair_get_private(pre_key_pair);
				ec_private_key_serialize(&key_buf, private_key);
				SIGNAL_UNREF(private_key);
				key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
				mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "OmemoPreKey%uPrivate", pre_key_id);
				data->proto->setString(setting_name, key);
				mir_free(key);
				signal_buffer_free(key_buf); */

			}
		}
		mir_free(setting_name);
		//TODO: resend bundle ?

		return 0;
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			return 0;
		}
		db_free(&dbv);

		return 1;
	}

	int remove_pre_key(uint32_t pre_key_id, void *user_data)
	{
		/**
		* Delete a PreKey record from local storage.
		*
		* @param pre_key_id The ID of the PreKey record to remove.
		* @return 0 on success, negative on failure
		*/

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "OmemoPreKey%uPublic", pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "OmemoPreKey%uPrivate", pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		mir_free(setting_name);

		//TODO: resend bundle ?


		return 0;
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalSignedPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalSignedPreKey_") + 31, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load signed prekey SG_ERR_INVALID_KEY_ID");
			return SG_ERR_INVALID_KEY_ID;
		}
		*record = signal_buffer_create(dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		return SG_SUCCESS; //key exist and succesfully loaded

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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalSignedPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalSignedPreKey_") + 31, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		db_set_blob(0, data->proto->m_szModuleName, setting_name, record, (unsigned int)record_len); //TODO: check return value
		mir_free(setting_name);
		//TODO: additionally store base64encoded public key for bundle

		return 0;
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalSignedPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalSignedPreKey_") + 31, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal)
		{
			db_free(&dbv);
			return 0;
		}
		db_free(&dbv);
		return 1;

	}

	int remove_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
	{
		/**
		* Delete a SignedPreKeyRecord from local storage.
		*
		* @param signed_pre_key_id The ID of the signed PreKey record to remove.
		* @return 0 on success, negative on failure
		*/

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalSignedPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalSignedPreKey_") + 31, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		mir_free(setting_name);
		//TODO: additionally remove base64encoded public key for bundle

		return 0;
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		char *pub_key = data->proto->getStringA("OmemoDevicePublicKey");
		char *priv_key = data->proto->getStringA("OmemoDevicePrivateKey");
		unsigned int pub_key_len = 0, priv_key_len = 0;
		char *pub_key_buf = (char*)mir_base64_decode(pub_key, &pub_key_len);
		mir_free(pub_key);
		char *priv_key_buf = (char*)mir_base64_decode(priv_key, &priv_key_len);
		mir_free(priv_key);
		*public_data = signal_buffer_create((uint8_t*)pub_key_buf, pub_key_len);
		*private_data = signal_buffer_create((uint8_t*)priv_key_buf, priv_key_len);
		mir_free(priv_key_buf);
		mir_free(pub_key_buf);

		return 0;
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
//		uint32_t *id = (uint32_t*)mir_alloc(sizeof(uint32_t));
		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
/*		*id = data->proto->m_omemo.GetOwnDeviceId();
		registration_id = id; */
		*registration_id = data->proto->m_omemo.GetOwnDeviceId();
		return 0;
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

		signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, (unsigned int)(address->name_len + sizeof(int32_t)));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalIdentity_", id_str);
		mir_free(id_str);
		if (key_data != 0)
			db_set_blob(data->hContact, data->proto->m_szModuleName, setting_name, key_data, (unsigned int)key_len); //TODO: check return value
		else
			db_unset(data->hContact, data->proto->m_szModuleName, setting_name);
		mir_free(setting_name);

		return 0;
	}

	int is_trusted_identity(const signal_protocol_address * /*address*/, uint8_t * /*key_data*/, size_t /*key_len*/, void * /*user_data*/)
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

		return 1;


	/*	signal_store_backend_user_data* data = (signal_store_backend_user_data*)user_data;
		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode((BYTE*)id_buf, (unsigned int)(address->name_len + sizeof(int32_t)));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalIdentity_", id_str);
		mir_free(id_str);


		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (key_len > 0 && !dbv.cpbVal)
		{
			db_free(&dbv);
			return 1;
		}
		if (dbv.cpbVal != key_len)
		{
			db_free(&dbv);
			return 0;
		}
		if (memcmp(key_data, dbv.pbVal, key_len))
		{
			db_free(&dbv);
			return 0;
		}

		db_free(&dbv);

		return 1; */
	}
	//void(*destroy_func)(void *user_data); //use first one as we have nothing special to destroy

	bool omemo_impl::create_session_store(MCONTACT hContact, LPCTSTR device_id)
	{
		signal_store_backend_user_data *data[4];
		char *device_id_a = mir_u2a(device_id);
		DWORD device_id_int = strtoul(device_id_a, nullptr, 10);
		mir_free(device_id_a);
		for (int i = 0; i < 4; i++)
		{
			data[i] = (signal_store_backend_user_data*)mir_alloc(sizeof(signal_store_backend_user_data));
			data[i]->hContact = hContact;
			data[i]->proto = proto;
			data[i]->device_id = device_id_int;
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

		(*(std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)[hContact][device_id_int].store_context = store_context; //oh shit ....

		return true; //success
	}
	bool omemo_impl::build_session(MCONTACT hContact, LPCTSTR jid, LPCTSTR dev_id, LPCTSTR key_id, LPCTSTR pre_key_public, LPCTSTR signed_pre_key_id,
		LPCTSTR signed_pre_key_public, LPCTSTR signed_pre_key_signature, LPCTSTR identity_key)
	{
		/* Instantiate a session_builder for a recipient address. */
		char *jid_str = mir_u2a(jid);
		char *dev_id_a = mir_u2a(dev_id);
		DWORD dev_id_int = strtoul(dev_id_a, nullptr, 10);
		mir_free(dev_id_a);
		signal_protocol_address *address = (signal_protocol_address*)mir_alloc(sizeof(signal_protocol_address)); //libsignal does not copy structure, so we must allocate one manually, does it free it on exit ?
		//rotten compillers support
		address->name = jid_str; //will libsignal free arrav for us on exit ?
		address->name_len = mir_strlen(jid_str);
		address->device_id = dev_id_int;

		session_builder *builder;
		if (session_builder_create(&builder, (*(std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)[hContact][dev_id_int].store_context,
			address, global_context) < 0)
		{
			proto->debugLogA("Jabber OMEMO: error: session_builder_create failed");
			return false; //failure
		}

		(*(std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)[hContact][dev_id_int].builder = builder;

		unsigned int key_id_int = _wtoi(key_id);

		char *pre_key_a = mir_u2a(pre_key_public);
		unsigned int key_buf_len;
		uint8_t *key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		ec_public_key *prekey;
		if (curve_decode_point(&prekey, key_buf, key_buf_len, global_context))
		{
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse prekey");
			return false; //TODO: cleanup
		}
		mir_free(pre_key_a);
		mir_free(key_buf);
		unsigned int signed_pre_key_id_int = _wtoi(signed_pre_key_id);
		pre_key_a = mir_u2a(signed_pre_key_public);
		key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		ec_public_key *signed_prekey;
		if(curve_decode_point(&signed_prekey, key_buf, key_buf_len, global_context))
		{
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse signed prekey");
			return false; //TODO: cleanup
		}
		mir_free(pre_key_a);
		mir_free(key_buf); //TODO: check this
		//load  identity key
		ec_public_key *identity_key_p;
		pre_key_a = mir_u2a(identity_key);
		key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		if(curve_decode_point(&identity_key_p, key_buf, key_buf_len, global_context))
		{
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse identity key");
			return false; //TODO: cleanup
		}
		mir_free(pre_key_a);
		mir_free(key_buf); //TODO: check this
		bool fp_trusted = false;
		{ //check fingerprint
			signal_buffer *key_buf2;
			ec_public_key_serialize(&key_buf2, identity_key_p);
			char *fingerprint = (char*)mir_alloc((signal_buffer_len(key_buf2) * 2) + 1);
			bin2hex(signal_buffer_data(key_buf2), signal_buffer_len(key_buf2), fingerprint);
	  
			const size_t setting_name_len = strlen("OmemoFingerprintTrusted_") + strlen(fingerprint) + 1;
			char *fp_setting_name = (char*)mir_alloc(setting_name_len);
			mir_snprintf(fp_setting_name, setting_name_len, "%s%s", "OmemoFingerprintTrusted_", fingerprint);
			char val = proto->getByte(hContact, fp_setting_name, -1);
			if (val == 1)
				fp_trusted = true;
			if (val == -1)
			{
				const size_t msg_len = strlen(Translate("Do you want to create OMEMO session with new device:")) + strlen("\n\n\t") + strlen(fingerprint) + 1;
				char *msg = (char*)mir_alloc(msg_len);
				mir_snprintf(msg, msg_len, "%s%s%s", Translate("Do you want to create OMEMO session with new device:"), "\n\n\t", fingerprint);

				int ret = MessageBoxA(nullptr, msg, Translate("OMEMO: New session"), MB_YESNO);
				if (ret == IDYES)
				{
					proto->setByte(hContact, fp_setting_name, 1);
					fp_trusted = true;
				}
				else if(ret == IDNO)
					proto->setByte(hContact, fp_setting_name, 0);
				mir_free(msg);
			}
			mir_free(fp_setting_name);
		}
		if (!fp_trusted)
		{
			proto->debugLogA("Jabber OMEMO: untrusted key, session build failure");
			return false; //TODO: cleanup here
		}
		pre_key_a = mir_u2a(signed_pre_key_signature);
		key_buf = (uint8_t*)mir_base64_decode(pre_key_a, &key_buf_len);
		mir_free(pre_key_a);
		session_pre_key_bundle *retrieved_pre_key;
		uint32_t *registration_id = (uint32_t*)mir_alloc(sizeof(uint32_t)); //let's create some momory leak...
		*registration_id = 0;
		signal_protocol_identity_get_local_registration_id((*(std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)[hContact][dev_id_int].store_context, registration_id);
		session_pre_key_bundle_create(&retrieved_pre_key, *registration_id, dev_id_int, key_id_int, prekey, signed_pre_key_id_int, signed_prekey, key_buf, key_buf_len, identity_key_p);
		mir_free(key_buf);

		


		/* Build a session with a pre key retrieved from the server. */
		int ret = session_builder_process_pre_key_bundle(builder, retrieved_pre_key);
		switch (ret)
		{
		case SG_SUCCESS:
			break;
		case SG_ERR_UNTRUSTED_IDENTITY:
		//TODO: do necessary actions for untrusted identity
		break;
		case SG_ERR_INVALID_KEY:
			proto->debugLogA("Jabber OMEMO: session_builder_process_pre_key_bundle failure SG_ERR_INVALID_KEY");
			return false; //failure
			break;
		default:
			proto->debugLogA("Jabber OMEMO: session_builder_process_pre_key_bundle failed with unknown error");
			return false; //failure
			break;

		}

		/* Create the session cipher and encrypt the message */
		session_cipher *cipher;
		if (session_cipher_create(&cipher,
			(*(std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)[hContact][dev_id_int].store_context, address, global_context) < 0)
		{
			proto->debugLogA("Jabber OMEMO: session_cipher_create failure");
			return false; //failure
		}
		(*(std::map<MCONTACT, std::map<unsigned int, omemo_session_jabber_internal_ptrs> >*)sessions_internal)[hContact][dev_id_int].cipher = cipher;

		return true; //success

	}
	void OmemoRefreshUsedPreKey(CJabberProto *proto, pre_key_signal_message *psm)
	{
		uint32_t id = pre_key_signal_message_get_pre_key_id(psm);
		//generate and save pre keys set

		ec_public_key *public_key = nullptr;
		signal_buffer *key_buf = nullptr;
		char *key = nullptr;
		signal_protocol_key_helper_pre_key_list_node *keys_root, *it;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, id, 1, global_context);
		it = keys_root;
		char setting_name[64], setting_name2[64];
		for (; it; it = signal_protocol_key_helper_key_list_next(it))
		{
			session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
			uint32_t pre_key_id = session_pre_key_get_id(pre_key);
			{
				signal_buffer *serialized_pre_key;
				session_pre_key_serialize(&serialized_pre_key, pre_key);
				mir_snprintf(setting_name2, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", proto->m_omemo.GetOwnDeviceId(), pre_key_id);
				db_set_blob(0, proto->m_szModuleName, setting_name2, signal_buffer_data(serialized_pre_key), (unsigned int)signal_buffer_len(serialized_pre_key));
				SIGNAL_UNREF(serialized_pre_key);
			}

			ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(pre_key);
			public_key = ec_key_pair_get_public(pre_key_pair);
			ec_public_key_serialize(&key_buf, public_key);
			SIGNAL_UNREF(public_key);
			key = mir_base64_encode(signal_buffer_data(key_buf), (unsigned int)signal_buffer_len(key_buf));
			mir_snprintf(setting_name, "OmemoPreKey%uPublic", pre_key_id);
			proto->setString(setting_name, key);
			mir_free(key);
			signal_buffer_free(key_buf);
		}
		signal_protocol_key_helper_key_list_free(keys_root);

//		proto->OmemoAnnounceDevice();
		proto->OmemoSendBundle();


	}
};


void CJabberProto::OmemoInitDevice()
{
	if (m_omemo.IsFirstRun())
		m_omemo.RefreshDevice();
}

void CJabberProto::OmemoPutMessageToOutgoingQueue(MCONTACT hContact, int unused_unknown, const char* pszSrc)
{
	char *msg = mir_strdup(pszSrc);
	((omemo::message_queue*)m_omemo.message_queue_internal)->outgoing_messages.push_back(omemo::outgoing_message(hContact, unused_unknown, msg));
}

void CJabberProto::OmemoPutMessageToIncommingQueue(HXML node, LPCTSTR jid, time_t msgTime)
{
	wchar_t *jid_ = mir_wstrdup(jid);
	HXML node_ = xmlCopyNode(node);
	((omemo::message_queue*)m_omemo.message_queue_internal)->incomming_messages.push_back(omemo::incomming_message(node_, jid_, msgTime));

}

void CJabberProto::OmemoHandleMessageQueue()
{
	for (std::list<omemo::outgoing_message>::iterator i = ((omemo::message_queue*)m_omemo.message_queue_internal)->outgoing_messages.begin(), 
		end = ((omemo::message_queue*)m_omemo.message_queue_internal)->outgoing_messages.end();	i != end; ++i)
	{
		SendMsg(i->hContact, i->unused_unknown, i->pszSrc);
		mir_free(i->pszSrc);
	}
	((omemo::message_queue*)m_omemo.message_queue_internal)->outgoing_messages.clear();
	for (std::list<omemo::incomming_message>::iterator i = ((omemo::message_queue*)m_omemo.message_queue_internal)->incomming_messages.begin(),
		end = ((omemo::message_queue*)m_omemo.message_queue_internal)->incomming_messages.end(); i != end; ++i)
	{
		OmemoHandleMessage(i->node, i->jid, i->msgTime);
		xmlFree(i->node);
		mir_free(i->jid);
	}
	((omemo::message_queue*)m_omemo.message_queue_internal)->incomming_messages.clear();
}

DWORD JabberGetLastContactMessageTime(MCONTACT hContact);

void CJabberProto::OmemoHandleMessage(HXML node, wchar_t *jid, time_t msgTime)
{
	MCONTACT hContact = HContactFromJID(jid);
	if (!OmemoCheckSession(hContact))
	{
		OmemoPutMessageToIncommingQueue(node, jid, msgTime);
		debugLogA("Jabber OMEMO: sessions not yet created, session creation launched");
		return;
	}
	HXML header_node = XmlGetChild(node, L"header");
	if (!header_node)
	{
		debugLogA("Jabber OMEMO: error: omemo message does not contain header");
		return;
	}
	HXML payload_node = XmlGetChild(node, L"payload");
	if (!payload_node)
	{
		debugLogA("Jabber OMEMO: omemo message does not contain payload, it's may be \"KeyTransportElement\" which is currently unused by our implementation");
		return; //this is "KeyTransportElement" which is currently unused
	}
	LPCTSTR payload_base64w = XmlGetText(payload_node);
	if (!payload_base64w)
	{
		debugLogA("Jabber OMEMO: error: failed to get payload data");
		return;
	}
	LPCTSTR iv_base64 = XmlGetText(XmlGetChild(header_node, L"iv"));
	if (!iv_base64)
	{
		Netlib_Log(nullptr, "Jabber OMEMO: error: failed to get iv data");
		return;
	}
	LPCTSTR sender_dev_id = XmlGetAttrValue(header_node, L"sid");
	if (!sender_dev_id)
	{
		debugLogA("Jabber OMEMO: error: failed to get sender device id");
		return;
	}
	char *sender_device_id_a = mir_u2a(sender_dev_id);
	DWORD sender_dev_id_int = strtoul(sender_device_id_a, nullptr, 10);
	mir_free(sender_device_id_a);
	if (!(*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact][sender_dev_id_int].cipher
		|| !(*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact][sender_dev_id_int].builder
		|| !(*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact][sender_dev_id_int].store_context)
	{
		OmemoCheckSession(hContact); //this should not normally happened
		OmemoPutMessageToIncommingQueue(node, jid, msgTime);
		debugLogA("Jabber OMEMO: bug: omemo session does not exist or broken");
		return;
	}
	HXML key_node;
	DWORD own_id = m_omemo.GetOwnDeviceId();
	LPCTSTR encrypted_key_base64 = nullptr;
	for (int p = 1; (key_node = XmlGetNthChild(header_node, L"key", p)) != nullptr; p++)
	{
		LPCTSTR dev_id = xmlGetAttrValue(key_node, L"rid");
		char *dev_id_a = mir_u2a(dev_id);
		DWORD dev_id_int = strtoul(dev_id_a, nullptr, 10);
		mir_free(dev_id_a);
		if (dev_id_int == own_id)
		{
			encrypted_key_base64 = XmlGetText(key_node);
			break;
		}
	}
	if (!encrypted_key_base64)
	{
		debugLogA("Jabber OMEMO: message does not have decryption key for our device");
		return; //node does not contain key for our device
	}
	unsigned int encrypted_key_len;
	unsigned char *encrypted_key;
	{
		char *key_buf = mir_u2a(encrypted_key_base64);
		encrypted_key = (unsigned char*)mir_base64_decode(key_buf, &encrypted_key_len);
		mir_free(key_buf);
	}
	unsigned int iv_len;
	unsigned char *iv;
	{
		char *iv_buf = mir_u2a(iv_base64);
		iv = (unsigned char *)mir_base64_decode(iv_buf, &iv_len);
		mir_free(iv_buf);
	}
	signal_buffer *decrypted_key = nullptr;
	bool decrypted = false;
	{ //try to decrypt as  pre_key_signal_message


		pre_key_signal_message *pm = nullptr;
		bool deserialized = false;
		//TODO: cleanup before return on error
		{
			int ret = pre_key_signal_message_deserialize(&pm, encrypted_key, encrypted_key_len, omemo::global_context);
			switch (ret)
			{
			case SG_SUCCESS:
				deserialized = true;
				break;
			case SG_ERR_INVALID_PROTO_BUF:
				debugLogA("Jabber OMEMO: error: pre_key_signal_message_deserialize failed SG_ERR_INVALID_PROTO_BUF\nTODO: use prekey tag in incomming message key element to avoid this");
//				return;
				break;
			default:
				debugLogA("Jabber OMEMO: error: pre_key_signal_message_deserialize failed with unknown error");
//				return;
				break;
			}
		}
		if (deserialized && pm)
		{
			int ret = session_cipher_decrypt_pre_key_signal_message((*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact][sender_dev_id_int].cipher, pm, 0, &decrypted_key);
			switch (ret)
			{
			case SG_SUCCESS:
				decrypted = true;
				omemo::OmemoRefreshUsedPreKey(this, pm);
				break;
			case SG_ERR_INVALID_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_INVALID_MESSAGE\nTODO: use prekey tag in incomming message key element to avoid this");
//				return;
				break;
			case SG_ERR_DUPLICATE_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_DUPLICATE_MESSAGE");
//				return;
				break;
			case SG_ERR_LEGACY_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_LEGACY_MESSAGE");
//				return;
				break;
			case SG_ERR_INVALID_KEY_ID:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_INVALID_KEY_ID");
//				return;
				break;
			case SG_ERR_INVALID_KEY:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_INVALID_KEY");
//				return;
				break;
			case SG_ERR_UNTRUSTED_IDENTITY:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_UNTRUSTED_IDENTITY");
//				return;
				break;
			default:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed with unknown error");
//				return;
				break;
			}
		}
	}
	if (!decrypted)
	{ //try to decrypt as signal message
		signal_message *sm = nullptr;

		bool deserialized = false;
		int ret = signal_message_deserialize(&sm, encrypted_key, encrypted_key_len, omemo::global_context);
		switch (ret)
		{
		case SG_SUCCESS:
			deserialized = true;
			break;
		default:
			debugLogA("Jabber OMEMO: error: signal_message_deserialize failed with unknown error");
			break;
		}
		if (deserialized && sm)
		{
			ret = session_cipher_decrypt_signal_message((*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact][sender_dev_id_int].cipher, sm, 0, &decrypted_key);
			switch (ret)
			{
			case SG_SUCCESS:
				decrypted = true;
				break;
			case SG_ERR_INVALID_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_signal_message failed SG_ERR_INVALID_MESSAGE");
				break;
			case SG_ERR_DUPLICATE_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_signal_message failed SG_ERR_DUPLICATE_MESSAGE");
				break;
			case SG_ERR_LEGACY_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_signal_message failed SG_ERR_LEGACY_MESSAGE");
				break;
			case SG_ERR_NO_SESSION:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_signal_message failed SG_ERR_NO_SESSION");
				break;
			default:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_signal_message failed with unknown error");
				break;

			}
		}
	}
	if(!decrypted)
	{
		debugLogA("Jabber OMEMO: error: failed to decrypt incomming message");
		return; //TODO: cleanup
	}
	char *out = nullptr;
	{
		int dec_success = 0;
		unsigned int payload_len = 0;
		int outl = 0, round_len = 0;
		char *payload_base64 = mir_u2a(payload_base64w);
		unsigned char *payload = (unsigned char*)mir_base64_decode(payload_base64, &payload_len);
		mir_free(payload_base64);
		unsigned char key[16], tag[16];
		{
			unsigned char tmp[32];
			memcpy(tmp, signal_buffer_data(decrypted_key), 32);
			memcpy(key, tmp, 16);
			unsigned char *ptr = tmp + 16;
			memcpy(tag, ptr, 16);
		}
		out = (char*)mir_alloc(payload_len + 1); //TODO: check this
		const EVP_CIPHER *cipher = EVP_aes_128_gcm();
		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit(ctx, cipher, key, iv);
		EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
		EVP_DecryptInit(ctx, nullptr, key, iv);
		//EVP_DecryptUpdate(ctx, nullptr, &howmany, AAD, aad_len);

		for (;;)
		{
			EVP_DecryptUpdate(ctx, (unsigned char*)out + outl, &round_len, payload + outl, (payload_len >= 128) ? 128 : payload_len);
			outl += round_len;
			if (outl >= (int)payload_len - 128)
				break;
		}
		EVP_DecryptUpdate(ctx, (unsigned char*)out + outl, &round_len, payload + outl, payload_len - outl);
		outl += round_len;
		out[outl] = 0;
		mir_free(payload);
		dec_success = EVP_DecryptFinal(ctx, tag, &round_len);
		EVP_CIPHER_CTX_free(ctx);
		if (dec_success <= 0) //TODO: check this... omemo xep have no info about tag
		{
			debugLogA("Jabber OMEMO: error: aes_128_gcm verification failed");
			return;
		}

	}

	time_t now = time(nullptr);
	if (!msgTime)
		msgTime = now;

	if (m_options.FixIncorrectTimestamps && (msgTime > now || (msgTime < (time_t)JabberGetLastContactMessageTime(hContact))))
		msgTime = now;

	pResourceStatus pFromResource(ResourceInfoFromJID(jid));
	PROTORECVEVENT recv = { 0 };
	recv.timestamp = (DWORD)msgTime;
	recv.szMessage = mir_utf8encode(out);
	recv.lParam = (LPARAM)((pFromResource != nullptr && m_options.EnableRemoteControl) ? pFromResource->m_tszResourceName : 0);
	ProtoChainRecvMsg(hContact, &recv);
	mir_free(out);
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
	{
		debugLogA("Jabber OMEMO: error: omemo devicelist does not have <item> node");
		return;
	}
	node = XmlGetChildByTag(node, L"list", L"xmlns", JABBER_FEAT_OMEMO); //<list xmlns = 'urn:xmpp:omemo:0'>
	if (!node)
	{
		debugLogA("Jabber OMEMO: error: omemo devicelist does not have <list> node");
		return;
	}
	bool own_jid = false;
	if (wcsstr(m_ThreadInfo->fullJID, jid))
		own_jid = true;
	DWORD current_id;
	LPCTSTR current_id_str;
	if (own_jid)
	{
		//check if our device exist
		bool own_device_listed = false;
		unsigned int own_id = m_omemo.GetOwnDeviceId();
		char setting_name[64];
		HXML list_item;
		int i = 0;
		for (int p = 1; (list_item = XmlGetNthChild(node, L"device", p)) != nullptr; p++, i++)
		{
			current_id_str = xmlGetAttrValue(list_item, L"id");
			char *current_id_str_a = mir_u2a(current_id_str);
			current_id = strtoul(current_id_str_a, nullptr, 10);
			mir_free(current_id_str_a);
			if (current_id == own_id)
				own_device_listed = true;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			setDword(setting_name, current_id);
		}
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
		for (int p = 1; (list_item = XmlGetNthChild(node, L"device", p)) != nullptr; p++, i++)
		{
			current_id_str = xmlGetAttrValue(list_item, L"id");
			char *current_id_str_a = mir_u2a(current_id_str);
			current_id = strtoul(current_id_str_a, nullptr, 10);
			mir_free(current_id_str_a);
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			setDword(hContact, setting_name, current_id);
		}
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
	unsigned int own_id = m_omemo.GetOwnDeviceId();

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
	HXML publish_node = iq << XCHILDNS(L"pubsub", L"http://jabber.org/protocol/pubsub") << XCHILD(L"publish") << XATTR(L"node", JABBER_FEAT_OMEMO L":devicelist");
	HXML list_node = publish_node << XCHILDNS(L"item") << XCHILDNS(L"list", JABBER_FEAT_OMEMO);

	for (int i = 0; ; ++i) {
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		DWORD val = getDword(setting_name);
		if (val == 0)
			break;

		list_node << XCHILD(L"device") << XATTRI64(L"id", val);
	}
	list_node << XCHILD(L"device") << XATTRI64(L"id", own_id);

	// send device list back
	//TODOL handle response
	m_ThreadInfo->send(iq);
}

struct db_enum_settings_prekeys_cb_data
{
	std::list<char*> settings; //TODO: check this
};


int db_enum_settings_prekeys_cb(const char *szSetting, LPARAM lParam)
{
	db_enum_settings_prekeys_cb_data* data = (db_enum_settings_prekeys_cb_data*)lParam;
	if (strstr(szSetting, "OmemoPreKey") && strstr(szSetting, "Public")) //TODO: suboptimal code, use different names for simple searching
		data->settings.push_back(mir_strdup(szSetting));

	return 0;//?

}

void CJabberProto::OmemoSendBundle()
{
	// get own device id
	DWORD own_id = m_omemo.GetOwnDeviceId();

	// construct bundle node
	wchar_t szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq(L"set", SerialNext());
	iq << XATTR(L"from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));

	HXML publish_node = iq << XCHILDNS(L"pubsub", L"http://jabber.org/protocol/pubsub") << XCHILD(L"publish");
	{
		wchar_t attr_val[128];
		mir_snwprintf(attr_val, L"%s:bundles:%u", JABBER_FEAT_OMEMO, own_id);
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

	db_enum_settings_prekeys_cb_data *ud = new db_enum_settings_prekeys_cb_data;
	db_enum_settings(0, &db_enum_settings_prekeys_cb, m_szModuleName, (void*)ud);
	for (std::list<char*>::iterator i = ud->settings.begin(), end = ud->settings.end(); i != end; i++)
	{
		ptrW val(getWStringA(*i));
		if (val)
		{
			unsigned int key_id = 0;
			char *p = *i, buf[5] = {0};
			p += strlen("OmemoPreKey");
			int i2 = 0;
			for (char c = 0; c != 'P'; i2++, c = p[i2])
				;
			memcpy(buf, p, i2);
			buf[i2 + 1] = 0;
			key_id = atoi(buf);
			prekeys_node << XCHILD(L"preKeyPublic", val) << XATTRI(L"preKeyId", key_id);
		}
		mir_free(*i);
	}
	ud->settings.clear();
	delete ud;
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
	if ((*(std::map<MCONTACT, bool>*)m_omemo.session_checked)[hContact])
		return true;
	bool pending_check = false;

	char setting_name[64], setting_name2[64];
	unsigned int id = 0;
	bool checked = false;
	int i = 0;
	
	mir_snprintf(setting_name, "OmemoDeviceId%d", i);
	mir_snprintf(setting_name2, "%sChecked", setting_name);
	db_set_resident(m_szModuleName, setting_name2);
	id = getDword(hContact, setting_name, 0);
	checked = getBool(hContact, setting_name2);
	while (id)
	{
		if (!checked)
		{
			pending_check = true;
			wchar_t szBareJid[JABBER_MAX_JID_LEN];
			unsigned int *dev_id = (unsigned int*)mir_alloc(sizeof(unsigned int));
			*dev_id = id;
			XmlNodeIq iq(AddIQ(&CJabberProto::OmemoOnIqResultGetBundle, JABBER_IQ_TYPE_GET, 0, 0UL, -1, dev_id));
			iq << XATTR(L"from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));
			wchar_t *jid = ContactToJID(hContact);
			iq << XATTR(L"to", jid);
			HXML items = iq << XCHILDNS(L"pubsub", L"http://jabber.org/protocol/pubsub") << XCHILD(L"items");
			wchar_t bundle[64];
			mir_snwprintf(bundle, 63, L"%s%s%u", JABBER_FEAT_OMEMO, L":bundles:", id);
			XmlAddAttr(items, L"node", bundle);
			m_ThreadInfo->send(iq);
			mir_free(jid);
			break;
		}
		i++;
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		mir_snprintf(setting_name2, "%sChecked", setting_name);
		id = getDword(hContact, setting_name, 0);
		checked = getBool(hContact, setting_name2);
	}

	if (!pending_check)
	{
		(*(std::map<MCONTACT, bool>*)m_omemo.session_checked)[hContact] = true;
		OmemoHandleMessageQueue();
		return true;
	}
	else
		debugLogA("Jabber OMEMO: info: OmemoCheckSession: pending session creation");
	return false;
}

void CJabberProto::OmemoOnIqResultGetBundle(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode == nullptr)
		return;
	
	LPCTSTR jid = XmlGetAttrValue(iqNode, L"from");
	MCONTACT hContact = HContactFromJID(jid);

	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (mir_wstrcmp(type, L"result"))
	{
		//failed to get bundle, do not try to build session
		unsigned int *dev_id = (unsigned int*)pInfo->GetUserData();
		char setting_name[64], setting_name2[64];
		DWORD id = 0;
		int i = 0;

		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		mir_snprintf(setting_name2, "%sChecked", setting_name);
		db_set_resident(m_szModuleName, setting_name2);
		id = getDword(hContact, setting_name, 0);
		while (id)
		{
			if (id == *dev_id)
			{
				setByte(hContact, setting_name2, 1);
				break;
			}
			i++;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			mir_snprintf(setting_name2, "%sChecked", setting_name);
			db_set_resident(m_szModuleName, setting_name2);
			id = getDword(hContact, setting_name, 0);
		}
		debugLogA("Jabber OMEMO: error: failed to get bundle for device, this may be due to absent data on server or due to our bug (incorrect device id in request)");

		return;
	}

	

	HXML pubsub = XmlGetChildByTag(iqNode, L"pubsub", L"xmlns", L"http://jabber.org/protocol/pubsub");
	if (!pubsub)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain pubsub node");
		return;
	}
	HXML items = XmlGetChild(pubsub, L"items");
	LPCTSTR items_node_val = XmlGetAttrValue(items, L"node");
	LPCTSTR device_id = items_node_val;
	device_id += mir_wstrlen(JABBER_FEAT_OMEMO L":bundles:");
	HXML bundle = XmlGetChild(XmlGetChild(items, L"item"), L"bundle");
	if (!bundle)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain bundle node");
		return;
	}
	LPCTSTR signedPreKeyPublic = XmlGetText(XmlGetChild(bundle, L"signedPreKeyPublic"));
	if (!signedPreKeyPublic)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain signedPreKeyPublic node");
		return;
	}
	LPCTSTR signedPreKeyId = XmlGetAttrValue(XmlGetChild(bundle, L"signedPreKeyPublic"), L"signedPreKeyId");
	if (!signedPreKeyId)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain signedPreKeyPublic node");
		return;
	}
	LPCTSTR signedPreKeySignature = XmlGetText(XmlGetChild(bundle, L"signedPreKeySignature"));
	if (!signedPreKeySignature)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain signedPreKeySignature node");
		return;
	}
	LPCTSTR identityKey = XmlGetText(XmlGetChild(bundle, L"identityKey"));
	if (!identityKey)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain identityKey node");
		return;
	}
	HXML prekeys = XmlGetChild(bundle, L"prekeys");
	if (!prekeys)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain prekeys node");
		return;
	}

	unsigned char key_num = 0;
	while(key_num == 0)
		Utils_GetRandom(&key_num, 1);
	key_num = (key_num % (XmlGetChildCount(prekeys))) + 1;

	wchar_t key_num_str[4];
	mir_snwprintf(key_num_str, 3, L"%d", key_num);
	HXML prekey_node;
	for (int p = 1; (prekey_node = XmlGetNthChild(prekeys, L"preKeyPublic", p)) != nullptr && p <= key_num; p++)
		;
	if (!prekey_node)
	{
		debugLogA("Jabber OMEMO: error: device bundle does not contain preKeyPublic node");
		return;
	}

	LPCTSTR preKeyPublic = XmlGetText(prekey_node);
	if (!preKeyPublic)
	{
		debugLogA("Jabber OMEMO: error: failed to get preKeyPublic data");
		return;
	}
	LPCTSTR preKeyId = XmlGetAttrValue(prekey_node, L"preKeyId");
	if (!preKeyId)
	{
		debugLogA("Jabber OMEMO: error: failed to get preKeyId data");
		return;
	}

	
	
	if (!m_omemo.create_session_store(hContact, device_id))
	{
		debugLogA("Jabber OMEMO: error: omemo::create_session_store failed");
		return; //failed to create session store
	}

	if (!m_omemo.build_session(hContact, jid, device_id, preKeyId, preKeyPublic, signedPreKeyId, signedPreKeyPublic, signedPreKeySignature, identityKey))
	{
		debugLogA("Jabber OMEMO: error: omemo::build_session failed");
		return; //failed to build signal(omemo) session
	}

	{
		unsigned int *dev_id = (unsigned int*)pInfo->GetUserData();
		char setting_name[64], setting_name2[64];
		DWORD id = 0;
		int i = 0;

		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		mir_snprintf(setting_name2, "%sChecked", setting_name);
		db_set_resident(m_szModuleName, setting_name2);
		id = getDword(hContact, setting_name, 0);
		while (id)
		{
			if (id == *dev_id)
			{
				setByte(hContact, setting_name2, 1);
				break;
			}
			i++;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			mir_snprintf(setting_name2, "%sChecked", setting_name);
			db_set_resident(m_szModuleName, setting_name2);
			id = getDword(hContact, setting_name, 0);
		}
	}
	OmemoCheckSession(hContact);

}

unsigned int CJabberProto::OmemoEncryptMessage(XmlNode &msg, const wchar_t *msg_text, MCONTACT hContact)
{
	const EVP_CIPHER *cipher = EVP_aes_128_gcm();
	unsigned char key[16], iv[12],  tag[16] /*, aad[48]*/;
	Utils_GetRandom(key, _countof_portable(key));
	Utils_GetRandom(iv, _countof_portable(iv));
	Utils_GetRandom(tag, _countof_portable(tag));
	//Utils_GetRandom(aad, _countof_portable(aad));
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, _countof_portable(iv), nullptr);
	EVP_EncryptInit(ctx, cipher, key, iv);
	char *in = mir_u2a(msg_text), *out;
	const size_t inl = strlen(in);
	int tmp_len = 0, outl;
	//EVP_EncryptUpdate(ctx, nullptr, &outl, aad, _countof_portable(aad));
	out = (char*)mir_alloc(inl + _countof_portable(key) - 1);
	for (;;)
	{
		EVP_EncryptUpdate(ctx, (unsigned char*)(out + tmp_len), &outl, (unsigned char*)(in + tmp_len), (int)(inl - tmp_len));
		tmp_len += outl;
		if (tmp_len >= (int)inl - 16 + 1) //cast to int is required here
			break;
	}
	EVP_EncryptFinal(ctx, (unsigned char*)(in + tmp_len), &outl);
	tmp_len += outl;
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, _countof_portable(tag), tag);
	EVP_CIPHER_CTX_free(ctx);
	mir_free(in);
	HXML encrypted = msg << XCHILDNS(L"encrypted", JABBER_FEAT_OMEMO);
	HXML payload = encrypted << XCHILD(L"payload");
	char *payload_base64 = mir_base64_encode((BYTE*)out, tmp_len);
	wchar_t *payload_base64w = mir_a2u(payload_base64);
	mir_free(payload_base64);
	xmlSetText(payload, payload_base64w);
	mir_free(payload_base64w);
	HXML header = encrypted << XCHILD(L"header");
	header << XATTRI64(L"sid", m_omemo.GetOwnDeviceId());
	unsigned int session_count = 0;
	char key_plus_tag[32] = {0};
	memcpy(key_plus_tag, key, 16);
	{
		char *ptr = key_plus_tag + 16;
		memcpy(ptr, tag, 16);
	}
	for (std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs>::iterator i = (*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact].begin(),
		end = (*(std::map<MCONTACT, std::map<unsigned int, omemo::omemo_session_jabber_internal_ptrs> >*)m_omemo.sessions_internal)[hContact].end(); i != end; i++)
	{
		if (!i->second.cipher)
		{
			debugLogA("Jabber OMEMO: bug: omemo session does not have valid cipher");
			continue;
		}
		unsigned int intdev_id = i->first;
		ciphertext_message *encrypted_key;
		if (session_cipher_encrypt(i->second.cipher, (uint8_t*)key_plus_tag, 32, &encrypted_key) != SG_SUCCESS)
		{
			//TODO: handle encryption error
			debugLogA("Jabber OMEMO: bug: session_cipher_encrypt failed");
			continue;
		}
		else
		{
			HXML key_node = header << XCHILD(L"key");
			key_node << XATTRI64(L"rid", intdev_id);
			int msg_type = ciphertext_message_get_type(encrypted_key);
			if(msg_type == CIPHERTEXT_PREKEY_TYPE)
			{
				key_node << XATTR(L"prekey", L"true");
			}
			signal_buffer *serialized_encrypted_key = ciphertext_message_get_serialized(encrypted_key);
			char *key_base64 = mir_base64_encode(signal_buffer_data(serialized_encrypted_key), (unsigned int)signal_buffer_len(serialized_encrypted_key));
			wchar_t *key_base64w = mir_a2u(key_base64);
			mir_free(key_base64);
			xmlSetText(key_node, key_base64w);
			mir_free(key_base64w);
			SIGNAL_UNREF(encrypted_key);
			session_count++;
		}
	}
	HXML iv_node = header << XCHILD(L"iv");
	char *iv_base64 = mir_base64_encode((BYTE*)iv, _countof_portable(iv));
	wchar_t *iv_base64w = mir_a2u(iv_base64);
	mir_free(iv_base64);
	xmlSetText(iv_node, iv_base64w);
	mir_free(iv_base64w);
	msg << XCHILDNS(L"store", L"urn:xmpp:hints");
	if (!session_count)
	{
		debugLogA("Jabber OMEMO: error: message does not encrypted for any sessions");
	}
	return session_count;
}
bool CJabberProto::OmemoIsEnabled(MCONTACT /*hContact*/)
{
	//TODO:
	return true;
}
