/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2017-19 Miranda NG team

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

#include "..\..\libs\libsignal\src\signal_protocol.h"
#include "..\..\libs\libsignal\src\signal_protocol_types.h"
#include "..\..\libs\libsignal\src\key_helper.h"
#include "..\..\libs\libsignal\src\session_builder.h"
#include "..\..\libs\libsignal\src\session_cipher.h"
#include "..\..\libs\libsignal\src\protocol.h"

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

namespace omemo
{
	int random_func(uint8_t *data, size_t len, void * /*user_data*/)
	{
		Utils_GetRandom(data, len);
		return 0;
	}

	int hmac_sha256_init_func(void **hmac_context, const uint8_t *key, size_t key_len, void * /*user_data*/)
	{
		HMAC_CTX *ctx = (HMAC_CTX*)mir_alloc(sizeof(HMAC_CTX));
		HMAC_CTX_init(ctx);
		HMAC_Init_ex(ctx, key, (int)key_len, EVP_sha256(), NULL);
		*hmac_context = ctx;
		return 0;
	}

	int hmac_sha256_update_func(void *hmac_context, const uint8_t *data, size_t data_len, void * /*user_data*/)
	{
		HMAC_CTX *ctx = (HMAC_CTX*)hmac_context;
		HMAC_Update(ctx, data, data_len);
		return 0;
	}

	int hmac_sha256_final_func(void *hmac_context, signal_buffer **output, void * /*user_data*/)
	{
		HMAC_CTX *ctx = (HMAC_CTX*)hmac_context;
		unsigned char buf[128];
		unsigned int len = 0;
		HMAC_Final(ctx, buf, &len);
		signal_buffer *output_buffer = signal_buffer_create(buf, len);
		*output = output_buffer;
		return 0;
	}

	void hmac_sha256_cleanup_func(void * hmac_context, void * /*user_data*/)
	{
		HMAC_CTX *ctx = (HMAC_CTX*)hmac_context;
		HMAC_CTX_cleanup(ctx);
		mir_free(ctx);
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

		result = (1 == EVP_DigestInit_ex(ctx, EVP_sha512(), nullptr)) ? SG_SUCCESS : SG_ERR_UNKNOWN;

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

		result = EVP_DigestInit_ex(ctx, EVP_sha512(), nullptr);
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
		return nullptr;
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
		uint8_t *out_buf = nullptr;

		const EVP_CIPHER *evp_cipher = aes_cipher(cipher, key_len);
		if (!evp_cipher) {
			//fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
			return SG_ERR_UNKNOWN;
		}

		if (iv_len != 16) {
			//fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
			return SG_ERR_UNKNOWN;
		}

		if ((int)plaintext_len > INT_MAX - EVP_CIPHER_block_size(evp_cipher)) {
			//fprintf(stderr, "invalid plaintext length: %zu\n", plaintext_len);
			return SG_ERR_UNKNOWN;
		}

		EVP_CIPHER_CTX ctx;
		EVP_CIPHER_CTX_init(&ctx);

		result = EVP_EncryptInit_ex(&ctx, evp_cipher, nullptr, key, iv);
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
		uint8_t *out_buf = nullptr;

		const EVP_CIPHER *evp_cipher = aes_cipher(cipher, key_len);
		if (!evp_cipher) {
			//fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
			return SG_ERR_INVAL;
		}

		if (iv_len != 16) {
			//fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
			return SG_ERR_INVAL;
		}

		if ((int)ciphertext_len > INT_MAX - EVP_CIPHER_block_size(evp_cipher)) {
			//fprintf(stderr, "invalid ciphertext length: %zu\n", ciphertext_len);
			return SG_ERR_UNKNOWN;
		}

		EVP_CIPHER_CTX ctx;
		EVP_CIPHER_CTX_init(&ctx);

		result = EVP_DecryptInit_ex(&ctx, evp_cipher, nullptr, key, iv);
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

	struct incoming_message
	{
		incoming_message(TiXmlElement *x, char *j, time_t t)
		{
			node = x;
			jid = j;
			msgTime = t;
		}
		TiXmlElement *node;
		char *jid;
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

	struct omemo_session_jabber_internal_ptrs
	{
		session_builder *builder;
		session_cipher *cipher;
		signal_protocol_store_context *store_context;
	};
	
	omemo_impl::omemo_impl(CJabberProto *p) : proto(p), signal_mutex(nullptr), provider(nullptr)
	{
		if (proto->m_bUseOMEMO)
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

		if (signal_context_set_crypto_provider(global_context, provider)) {
			proto->debugLogA("Jabber OMEMO: signal_context_set_crypto_provider failed");
			//TODO: handle error
		}

		if (signal_context_set_locking_functions(global_context, &lock, &unlock)) {
			proto->debugLogA("Jabber OMEMO: signal_context_set_crypto_provider failed");
			//TODO: handle error
		}
	}

	omemo_impl::~omemo_impl()
	{
		if (proto->m_bUseOMEMO)
			deinit();
	}

	void omemo_impl::deinit()
	{
		if (provider && signal_mutex) {
			for (auto &i : sessions) {
				for (auto &i2 : i.second) {
					if (i2.second.cipher)
						session_cipher_free(i2.second.cipher);
					if (i2.second.builder)
						session_builder_free(i2.second.builder);
					if (i2.second.store_context)
						signal_protocol_store_context_destroy(i2.second.store_context);
				}
			}
			sessions.clear();

			delete signal_mutex; signal_mutex = nullptr;
			delete provider; provider = nullptr;			
		}
	}

	struct omemo_device
	{
		uint32_t id;
		ratchet_identity_key_pair *device_key;
	};

	omemo_device* omemo_impl::create_device()
	{
		omemo_device *dev = (omemo_device*)mir_alloc(sizeof(omemo_device));
		for (dev->id = 0; dev->id == 0;)
			Utils_GetRandom((void*)&(dev->id), 4);

		dev->id &= ~0x80000000;

		if (signal_protocol_key_helper_generate_identity_key_pair(&(dev->device_key), global_context)) {
			proto->debugLogA("Jabber OMEMO: signal_protocol_key_helper_generate_identity_key_pair failed");
			//TODO: handle error
		}

		return dev;
	}

	bool omemo_impl::IsFirstRun()
	{
		// TODO: more sanity checks
		// TODO: check and if necessary refresh prekeys
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
		if (own_id == 0) {
			proto->OmemoInitDevice();
			own_id = proto->getDword("OmemoDeviceId", 0);
		}
		return own_id;
	}

	void omemo_impl::RefreshDevice()
	{
		// generate and save device id
		omemo_device *new_dev = create_device();
		proto->setDword("OmemoDeviceId", new_dev->id);

		// generate and save device key
		ec_public_key *public_key = ratchet_identity_key_pair_get_public(new_dev->device_key);
		signal_buffer *key_buf;
		ec_public_key_serialize(&key_buf, public_key);
		{
			ptrA key(mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf)));
			ptrA fingerprint((char*)mir_alloc((signal_buffer_len(key_buf) * 2) + 1));
			bin2hex(signal_buffer_data(key_buf), signal_buffer_len(key_buf), fingerprint);
			proto->setString("OmemoFingerprintOwn", fingerprint);
			proto->setString("OmemoDevicePublicKey", key);
		}
		signal_buffer_free(key_buf);

		ec_private_key *private_key = ratchet_identity_key_pair_get_private(new_dev->device_key);
		ec_private_key_serialize(&key_buf, private_key);
		proto->setString("OmemoDevicePrivateKey", ptrA(mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf))));
		signal_buffer_free(key_buf);

		// generate and save signed pre key
		session_signed_pre_key* signed_pre_key;
		{
			const unsigned int signed_pre_key_id = 1;
			signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, new_dev->device_key, signed_pre_key_id, time(0), global_context);
			SIGNAL_UNREF(new_dev->device_key);

			signal_buffer *serialized_signed_pre_key;
			session_signed_pre_key_serialize(&serialized_signed_pre_key, signed_pre_key);
			CMStringA setting_name(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
			db_set_blob(0, proto->m_szModuleName, setting_name, signal_buffer_data(serialized_signed_pre_key), (unsigned int)signal_buffer_len(serialized_signed_pre_key));
		}

		// TODO: store signed_pre_key for libsignal data backend too
		// TODO: dynamic signed pre_key id and setting name ?
		ec_key_pair *signed_pre_key_pair = session_signed_pre_key_get_key_pair(signed_pre_key);
		public_key = ec_key_pair_get_public(signed_pre_key_pair);
		ec_public_key_serialize(&key_buf, public_key);
		proto->setString("OmemoSignedPreKeyPublic", ptrA(mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf))));
		signal_buffer_free(key_buf);

		ptrA signature(mir_base64_encode(session_signed_pre_key_get_signature(signed_pre_key), session_signed_pre_key_get_signature_len(signed_pre_key)));
		proto->setString("OmemoSignedPreKeySignature", signature);

		// generate and save pre keys set
		signal_protocol_key_helper_pre_key_list_node *keys_root, *it;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, 0, 100, global_context);
		it = keys_root;
		char setting_name[64], setting_name2[64];
		for (; it; it = signal_protocol_key_helper_key_list_next(it)) {
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

			mir_snprintf(setting_name, "OmemoPreKey%uPublic", pre_key_id);
			proto->setString(setting_name, ptrA(mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf))));
			signal_buffer_free(key_buf);
		}
		signal_protocol_key_helper_key_list_free(keys_root);

	}

	// signal_protocol_session_store callbacks follow

	struct signal_store_backend_user_data
	{
		MCONTACT hContact;
		unsigned int device_id;
		CJabberProto *proto;
	};

	int load_session_func(signal_buffer **record, signal_buffer ** /*user_data_storage*/, const signal_protocol_address *address, void *user_data)
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
		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		if ((unsigned long)address->device_id == 0) {
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (invalid device id)");
			return -1;
		}

		if (address->name_len > JABBER_MAX_JID_LEN) {
			data->proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (invalid address length)");
			return -1;
		}

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode(id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal) {
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
		{
		}
	private:
		db_enum_settings_sub_cb_data(); //we always need array size
	};

	int db_enum_settings_sub_cb(const char *szSetting, void *lParam)
	{
		db_enum_settings_sub_cb_data *data = (db_enum_settings_sub_cb_data*)lParam;
		if (strstr(szSetting, "OmemoSignalSession_")) {
			char *ptr = (char*)szSetting;
			ptr += mir_strlen("OmemoSignalSession_");
			char *current_name = mir_base64_encode(data->name, data->name_len);
			if (strstr(ptr, current_name)) {
				char *dev_encoded = ptr;
				dev_encoded += strlen(current_name);
				size_t len;
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
		signal_int_list *l = signal_int_list_alloc();
		unsigned int array_size = 0;

		db_enum_settings_sub_cb_data *ud = new db_enum_settings_sub_cb_data(array_size);
		ud->user_data = data;
		ud->sessions = l;
		ud->name = name;
		ud->name_len = name_len;
		db_enum_settings(data->hContact, &db_enum_settings_sub_cb, data->proto->m_szModuleName, ud);
		*sessions = l;
		delete ud;
		return array_size;
	}

	int store_session_func(const signal_protocol_address *address, uint8_t *record, size_t record_len, uint8_t * /*user_record*/, size_t /*user_record_len*/, void *user_data)
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
		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode(id_buf, address->name_len + sizeof(int32_t));
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
		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode(id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalSession_", id_str);
		mir_free(id_str);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(data->hContact, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal) {
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode(id_buf, address->name_len + sizeof(int32_t));
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

	int db_enum_settings_del_all_cb(const char *szSetting, void *lParam)
	{
		db_enum_settings_del_all_cb_data *data = (db_enum_settings_del_all_cb_data*)lParam;
		if (strstr(szSetting, "OmemoSignalSession_")) {
			char *ptr = (char*)szSetting;
			ptr += mir_strlen("OmemoSignalSession_");
			char *current_name = mir_base64_encode(data->name, data->name_len);
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
		db_enum_settings_del_all_cb_data *ud = new db_enum_settings_del_all_cb_data;
		ud->user_data = data;
		ud->name = name;
		ud->name_len = name_len;
		db_enum_settings(data->hContact, &db_enum_settings_del_all_cb, data->proto->m_szModuleName, (void*)ud);
		int count = 0;
		for (std::list<char*>::iterator i = ud->settings.begin(), end = ud->settings.end(); i != end; i++) {
			db_unset(data->hContact, data->proto->m_szModuleName, *i);
			mir_free(*i);
			count++;
		}
		delete ud;
		return count;
	}

	void destroy_func(void *user_data)
	{
		if (user_data) {
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal) {
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		db_set_blob(0, data->proto->m_szModuleName, setting_name, record, (unsigned int)record_len); //TODO: check return value
		{ //store base64 encoded keys for bundle (private key does not required ?)
			session_pre_key *prekey = nullptr;
			session_pre_key_deserialize(&prekey, record, record_len, global_context); //TODO: handle error
			if (prekey) {
				ec_public_key *public_key = nullptr;
				//ec_private_key *private_key = nullptr;
				ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(prekey);
				signal_buffer *key_buf = nullptr;
				char *key = nullptr;
				public_key = ec_key_pair_get_public(pre_key_pair);
				ec_public_key_serialize(&key_buf, public_key);
				SIGNAL_UNREF(public_key);
				key = mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf));
				mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "OmemoPreKey%uPublic", pre_key_id);
				data->proto->setString(setting_name, key);
				mir_free(key);
				signal_buffer_free(key_buf);
/*				private_key = ec_key_pair_get_private(pre_key_pair);
				ec_private_key_serialize(&key_buf, private_key);
				SIGNAL_UNREF(private_key);
				key = mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf));
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		char *setting_name = (char*)mir_alloc(strlen("OmemoSignalPreKey_") + 32);
		mir_snprintf(setting_name, strlen("OmemoSignalPreKey_") + 31, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		mir_free(setting_name);
		if (!dbv.cpbVal) {
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		CMStringA setting_name(FORMAT, "%s%u%d", "OmemoSignalPreKey_", data->proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);

		setting_name.Format("OmemoPreKey%uPublic", pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		setting_name.Format("OmemoPreKey%uPrivate", pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		return 0;
	}

	// signal_protocol_signed_pre_key_store callbacks follow

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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
		CMStringA setting_name(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		if (!dbv.cpbVal) {
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		CMStringA setting_name(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		db_set_blob(0, data->proto->m_szModuleName, setting_name, record, (unsigned int)record_len); //TODO: check return value
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;

		CMStringA setting_name(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, data->proto->m_szModuleName, setting_name, &dbv);
		if (!dbv.cpbVal) {
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
		CMStringA setting_name(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", data->proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		db_unset(0, data->proto->m_szModuleName, setting_name);
		return 0;
	}

	// signal_protocol_identity_key_store callbacks follow

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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
		char *pub_key = data->proto->getStringA("OmemoDevicePublicKey");
		char *priv_key = data->proto->getStringA("OmemoDevicePrivateKey");
		size_t pub_key_len = 0, priv_key_len = 0;
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
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

		signal_store_backend_user_data *data = (signal_store_backend_user_data*)user_data;
		char *id_buf = (char*)mir_alloc(address->name_len + sizeof(int32_t));
		memcpy(id_buf, address->name, address->name_len);
		char *id_buf_ptr = id_buf;
		id_buf_ptr += address->name_len;
		memcpy(id_buf_ptr, &address->device_id, sizeof(int32_t));
		char *id_str = mir_base64_encode(id_buf, address->name_len + sizeof(int32_t));
		mir_free(id_buf);
		char *setting_name = (char*)mir_alloc(strlen(id_str) + 65);
		mir_snprintf(setting_name, strlen(id_str) + 64, "%s%s", "OmemoSignalIdentity_", id_str);
		mir_free(id_str);
		if (key_data != nullptr)
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
		char *id_str = mir_base64_encode(id_buf, address->name_len + sizeof(int32_t));
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

	bool omemo_impl::create_session_store(MCONTACT hContact, const char *device_id)
	{
		signal_store_backend_user_data *data[4];
		DWORD device_id_int = strtoul(device_id, nullptr, 10);
		for (int i = 0; i < 4; i++) {
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

		sessions[hContact][device_id_int].store_context = store_context; //oh shit ....
		return true; //success
	}

	bool omemo_impl::build_session(MCONTACT hContact, const char *jid, const char *dev_id, const char *key_id, const char *pre_key_public, const char *signed_pre_key_id,
		const char *signed_pre_key_public, const char *signed_pre_key_signature, const char *identity_key)
	{
		// Instantiate a session_builder for a recipient address.
		DWORD dev_id_int = strtoul(dev_id, nullptr, 10);

		// libsignal does not copy structure, so we must allocate one manually, does it free it on exit ?
		signal_protocol_address *address = (signal_protocol_address*)mir_alloc(sizeof(signal_protocol_address));

		// rotten compillers support
		address->name = jid; // will libsignal free arrav for us on exit ?
		address->name_len = mir_strlen(jid);
		address->device_id = dev_id_int;

		session_builder *builder;
		if (session_builder_create(&builder, sessions[hContact][dev_id_int].store_context, address, global_context) < 0) {
			proto->debugLogA("Jabber OMEMO: error: session_builder_create failed");
			return false; //failure
		}

		sessions[hContact][dev_id_int].builder = builder;

		unsigned int key_id_int = atoi(key_id);

		size_t key_buf_len;
		uint8_t *key_buf = (uint8_t*)mir_base64_decode(pre_key_public, &key_buf_len);
		ec_public_key *prekey;
		if (curve_decode_point(&prekey, key_buf, key_buf_len, global_context)) {
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse prekey");
			return false; //TODO: cleanup
		}
		mir_free(key_buf);

		unsigned int signed_pre_key_id_int = atoi(signed_pre_key_id);
		key_buf = (uint8_t*)mir_base64_decode(signed_pre_key_public, &key_buf_len);
		ec_public_key *signed_prekey;
		if (curve_decode_point(&signed_prekey, key_buf, key_buf_len, global_context)) {
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse signed prekey");
			return false; // TODO: cleanup
		}
		mir_free(key_buf); // TODO: check this

		// load  identity key
		ec_public_key *identity_key_p;
		key_buf = (uint8_t*)mir_base64_decode(identity_key, &key_buf_len);
		if (curve_decode_point(&identity_key_p, key_buf, key_buf_len, global_context)) {
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse identity key");
			return false; // TODO: cleanup
		}
		mir_free(key_buf); // TODO: check this
		bool fp_trusted = false;
		{
			// check fingerprint
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
			if (val == -1) {
				const size_t msg_len = strlen(Translate("Do you want to create OMEMO session with new device:")) + strlen("\n\n\t") + strlen(fingerprint) + 1;
				char *msg = (char*)mir_alloc(msg_len);
				mir_snprintf(msg, msg_len, "%s%s%s", Translate("Do you want to create OMEMO session with new device:"), "\n\n\t", fingerprint);

				int ret = MessageBoxA(nullptr, msg, Translate("OMEMO: New session"), MB_YESNO);
				if (ret == IDYES) {
					proto->setByte(hContact, fp_setting_name, 1);
					fp_trusted = true;
				}
				else if (ret == IDNO)
					proto->setByte(hContact, fp_setting_name, 0);
				mir_free(msg);
			}
			mir_free(fp_setting_name);
		}
		if (!fp_trusted) {
			proto->debugLogA("Jabber OMEMO: untrusted key, session build failure");
			return false; //TODO: cleanup here
		}

		key_buf = (uint8_t*)mir_base64_decode(signed_pre_key_signature, &key_buf_len);
		session_pre_key_bundle *retrieved_pre_key;
		uint32_t registration_id = 0;
		signal_protocol_identity_get_local_registration_id(sessions[hContact][dev_id_int].store_context, &registration_id);
		session_pre_key_bundle_create(&retrieved_pre_key, registration_id, dev_id_int, key_id_int, prekey, signed_pre_key_id_int, signed_prekey, key_buf, key_buf_len, identity_key_p);
		mir_free(key_buf);

		/* Build a session with a pre key retrieved from the server. */
		int ret = session_builder_process_pre_key_bundle(builder, retrieved_pre_key);
		switch (ret) {
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
			sessions[hContact][dev_id_int].store_context, address, global_context) < 0) {
			proto->debugLogA("Jabber OMEMO: session_cipher_create failure");
			return false; //failure
		}
		sessions[hContact][dev_id_int].cipher = cipher;
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
		for (; it; it = signal_protocol_key_helper_key_list_next(it)) {
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
			key = mir_base64_encode(signal_buffer_data(key_buf), signal_buffer_len(key_buf));
			mir_snprintf(setting_name, "OmemoPreKey%uPublic", pre_key_id);
			proto->setString(setting_name, key);
			mir_free(key);
			signal_buffer_free(key_buf);
		}
		signal_protocol_key_helper_key_list_free(keys_root);

		//	proto->OmemoAnnounceDevice();
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
	m_omemo.outgoing_messages.push_back(omemo::outgoing_message(hContact, unused_unknown, msg));
}

void CJabberProto::OmemoPutMessageToIncommingQueue(const TiXmlElement *node, const char *jid, time_t msgTime)
{
	TiXmlElement *node_ = node->DeepClone(&m_omemo.doc)->ToElement();
	m_omemo.incoming_messages.push_back(omemo::incoming_message(node_, mir_strdup(jid), msgTime));
}

void CJabberProto::OmemoHandleMessageQueue()
{
	for (auto &i : m_omemo.outgoing_messages) {
		SendMsg(i.hContact, i.unused_unknown, i.pszSrc);
		mir_free(i.pszSrc);
	}
	m_omemo.outgoing_messages.clear();
	std::list<omemo::incoming_message> tmp = m_omemo.incoming_messages;
	m_omemo.incoming_messages.clear();
	for (auto &i : tmp) {
		if (!OmemoHandleMessage(i.node, i.jid, i.msgTime))
			OmemoPutMessageToIncommingQueue(i.node, i.jid, i.msgTime);

		m_omemo.doc.DeleteNode(i.node);
		mir_free(i.jid);
	}
}

DWORD JabberGetLastContactMessageTime(MCONTACT hContact);

bool CJabberProto::OmemoHandleMessage(const TiXmlElement *node, const char *jid, time_t msgTime)
{
	MCONTACT hContact = HContactFromJID(jid);
	if (!OmemoCheckSession(hContact)) {
		debugLogA("Jabber OMEMO: sessions not yet created, session creation launched");
		return false;
	}
	auto *header_node = XmlFirstChild(node, "header");
	if (!header_node) {
		debugLogA("Jabber OMEMO: error: omemo message does not contain header");
		return true; //this should never happen
	}
	auto *payload_node = XmlFirstChild(node, "payload");
	if (!payload_node) {
		debugLogA("Jabber OMEMO: omemo message does not contain payload, it's may be \"KeyTransportElement\" which is currently unused by our implementation");
		return true; //this is "KeyTransportElement" which is currently unused
	}
	const char *payload_base64 = payload_node->GetText();
	if (!payload_base64) {
		debugLogA("Jabber OMEMO: error: failed to get payload data");
		return true; //this should never happen
	}
	const char *iv_base64 = XmlFirstChild(header_node, "iv")->GetText();
	if (!iv_base64) {
		Netlib_Log(nullptr, "Jabber OMEMO: error: failed to get iv data");
		return true;
	}
	const char *sender_dev_id = XmlGetAttr(header_node, "sid");
	if (!sender_dev_id) {
		debugLogA("Jabber OMEMO: error: failed to get sender device id");
		return true;
	}
	DWORD sender_dev_id_int = strtoul(sender_dev_id, nullptr, 10);

	auto &pSession = m_omemo.sessions[hContact][sender_dev_id_int];
	if (!pSession.cipher || !pSession.builder || !pSession.store_context) {
		OmemoCheckSession(hContact); //this should not normally happened
		debugLogA("Jabber OMEMO: bug: omemo session does not exist or broken");
		return false;
	}
	
	DWORD own_id = m_omemo.GetOwnDeviceId();
	const char *encrypted_key_base64 = nullptr;
	for (auto *it : TiXmlFilter(header_node, "key")) {
		DWORD dev_id_int = it->IntAttribute("rid");
		if (dev_id_int == own_id) {
			encrypted_key_base64 = it->GetText();
			break;
		}
	}

	if (!encrypted_key_base64) {
		debugLogA("Jabber OMEMO: message does not have decryption key for our device");
		return true; //node does not contain key for our device
	}

	size_t encrypted_key_len;
	unsigned char *encrypted_key = (unsigned char*)mir_base64_decode(encrypted_key_base64, &encrypted_key_len);

	size_t iv_len;
	unsigned char *iv = (unsigned char *)mir_base64_decode(iv_base64, &iv_len);

	signal_buffer *decrypted_key = nullptr;
	bool decrypted = false;
	{
		//try to decrypt as  pre_key_signal_message
		pre_key_signal_message *pm = nullptr;
		bool deserialized = false;
		//TODO: cleanup before return on error
		{
			int ret = pre_key_signal_message_deserialize(&pm, encrypted_key, encrypted_key_len, omemo::global_context);
			switch (ret) {
			case SG_SUCCESS:
				deserialized = true;
				break;
			case SG_ERR_INVALID_PROTO_BUF:
				debugLogA("Jabber OMEMO: error: pre_key_signal_message_deserialize failed SG_ERR_INVALID_PROTO_BUF\nTODO: use prekey tag in incomming message key element to avoid this");
				break;
			default:
				debugLogA("Jabber OMEMO: error: pre_key_signal_message_deserialize failed with unknown error");
				break;
			}
		}
		if (deserialized && pm) {
			int ret = session_cipher_decrypt_pre_key_signal_message(m_omemo.sessions[hContact][sender_dev_id_int].cipher, pm, nullptr, &decrypted_key);
			switch (ret) {
			case SG_SUCCESS:
				decrypted = true;
				omemo::OmemoRefreshUsedPreKey(this, pm);
				break;
			case SG_ERR_INVALID_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_INVALID_MESSAGE\nTODO: use prekey tag in incomming message key element to avoid this");
				break;
			case SG_ERR_DUPLICATE_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_DUPLICATE_MESSAGE");
				break;
			case SG_ERR_LEGACY_MESSAGE:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_LEGACY_MESSAGE");
				break;
			case SG_ERR_INVALID_KEY_ID:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_INVALID_KEY_ID");
				break;
			case SG_ERR_INVALID_KEY:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_INVALID_KEY");
				break;
			case SG_ERR_UNTRUSTED_IDENTITY:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed SG_ERR_UNTRUSTED_IDENTITY");
				break;
			default:
				debugLogA("Jabber OMEMO: error: session_cipher_decrypt_pre_key_signal_message failed with unknown error");
				break;
			}
		}
	}
	if (!decrypted) { //try to decrypt as signal message
		signal_message *sm = nullptr;

		bool deserialized = false;
		int ret = signal_message_deserialize(&sm, encrypted_key, encrypted_key_len, omemo::global_context);
		switch (ret) {
		case SG_SUCCESS:
			deserialized = true;
			break;
		default:
			debugLogA("Jabber OMEMO: error: signal_message_deserialize failed with unknown error");
			break;
		}

		if (deserialized && sm) {
			ret = session_cipher_decrypt_signal_message(m_omemo.sessions[hContact][sender_dev_id_int].cipher, sm, nullptr, &decrypted_key);
			switch (ret) {
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
	if (!decrypted) {
		debugLogA("Jabber OMEMO: error: failed to decrypt incomming message");
		return true; //TODO: cleanup
	}
	char *out = nullptr;
	{
		int dec_success = 0;
		size_t payload_len = 0;
		int outl = 0, round_len = 0, tag_len = 0;
		unsigned char *payload = (unsigned char*)mir_base64_decode(payload_base64, &payload_len);
		out = (char*)mir_alloc(payload_len + 32); //TODO: check this
		unsigned char key[16], *tag;
		{
			size_t buf_len = signal_buffer_len(decrypted_key);
			unsigned char *tmp = (unsigned char*)mir_alloc(buf_len);
			memcpy(tmp, signal_buffer_data(decrypted_key), buf_len);
			memcpy(key, tmp, 16);
			unsigned char *ptr = tmp + 16;
			tag_len = (int)buf_len - 16;
			tag = (unsigned char*)mir_alloc(tag_len);
			memcpy(tag, ptr, tag_len);
			mir_free(tmp);
			signal_buffer_free(decrypted_key);
		}

		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL);
		if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv_len, NULL)) {
			debugLogA("Jabber OMEMO: error: EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv_len, NULL) failed");
			mir_free(tag);
			return true;
		}
		if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
			debugLogA("Jabber OMEMO: error: EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) failed");
			mir_free(tag);
			return true;
		}
		if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag_len, tag)) {
			debugLogA("Jabber OMEMO: error: EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag_len, tag) failed");
			mir_free(tag);
			return true;
		}
		if (!EVP_DecryptUpdate(ctx, (unsigned char*)out, &outl, payload, int(payload_len))) {
			debugLogA("Jabber OMEMO: error: EVP_DecryptUpdate(ctx, (unsigned char*)out, &outl, payload, int(payload_len)) failed");
			mir_free(tag);
			return true;
		}
/*		for (;;)
		{
			if (!EVP_DecryptUpdate(ctx, (unsigned char*)out + outl, &round_len, payload + outl, int((payload_len >= 128) ? 128 : payload_len)))
			{
				debugLogA("Jabber OMEMO: error: EVP_DecryptUpdate(ctx, (unsigned char*)out + outl, &round_len, payload + outl, int((payload_len >= 128) ? 128 : payload_len)) failed");
				mir_free(tag);
				return true;
			}
			outl += round_len;
			if (outl >= (int)payload_len - 128)
				break;
		}
		EVP_DecryptUpdate(ctx, (unsigned char*)out + outl, &round_len, payload + outl, int(payload_len - outl));
		outl += round_len; */

		dec_success = EVP_DecryptFinal_ex(ctx, (unsigned char*)out + outl, &round_len);
		outl += round_len;
		out[outl] = 0;
		mir_free(payload);
		EVP_CIPHER_CTX_free(ctx);
		mir_free(tag);
		if (dec_success <= 0) {
			debugLogA("Jabber OMEMO: error: aes_128_gcm verification failed");
			return true;
		}
	}

	time_t now = time(0);
	if (!msgTime)
		msgTime = now;

	if (m_bFixIncorrectTimestamps && (msgTime > now || (msgTime < (time_t)JabberGetLastContactMessageTime(hContact))))
		msgTime = now;

	PROTORECVEVENT recv = {};
	recv.timestamp = (DWORD)msgTime;
	recv.szMessage = mir_strdup(out);
	ProtoChainRecvMsg(hContact, &recv);
	mir_free(out);
	return true;
}

void CJabberProto::OmemoHandleDeviceList(const TiXmlElement *node)
{
	if (!node)
		return;
	
	auto *message = node->Parent()->ToElement();
	message = message->Parent()->ToElement();
	
	const char *jid = XmlGetAttr(message, "from");
	MCONTACT hContact = HContactFromJID(jid);
	node = XmlFirstChild(node, "item"); //get <item> node
	if (!node) {
		debugLogA("Jabber OMEMO: error: omemo devicelist does not have <item> node");
		return;
	}
	node = XmlGetChildByTag(node, "list", "xmlns", JABBER_FEAT_OMEMO); //<list xmlns = 'urn:xmpp:omemo:0'>
	if (!node) {
		debugLogA("Jabber OMEMO: error: omemo devicelist does not have <list> node");
		return;
	}
	bool own_jid = false;
	if (strstr(m_ThreadInfo->fullJID, jid))
		own_jid = true;

	if (own_jid) {
		//check if our device exist
		bool own_device_listed = false;
		uint32_t own_id = m_omemo.GetOwnDeviceId();
		char setting_name[64];
		int i = 0;
		for (auto *list_item : TiXmlFilter(node, "device")) {
			uint32_t current_id = list_item->IntAttribute("id");
			if (current_id == own_id)
				own_device_listed = true;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i++);
			setDword(setting_name, current_id);
		}

		DWORD val = 0;
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		val = getDword(setting_name, 0);
		while (val) {
			delSetting(setting_name);
			i++;
			mir_snprintf(setting_name, "OmemoDeviceId%d", i);
			val = getDword(setting_name, 0);
		}
		if (!own_device_listed)
			OmemoAnnounceDevice();
	}
	else {
		// store device id's
		char setting_name[64];
		int i = 0;
		for (auto *list_item : TiXmlFilter(node, "device")) {
			uint32_t current_id = list_item->IntAttribute("id");
			mir_snprintf(setting_name, "OmemoDeviceId%d", i++);
			setDword(hContact, setting_name, current_id);
		}

		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		DWORD val = getDword(hContact, setting_name, 0);
		while (val) {
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
	char szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq("set", SerialNext());
	iq << XATTR("from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));
	TiXmlElement *publish_node = iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub") << XCHILD("publish") << XATTR("node", JABBER_FEAT_OMEMO ".devicelist");
	TiXmlElement *list_node = publish_node << XCHILDNS("item") << XCHILDNS("list", JABBER_FEAT_OMEMO);

	for (int i = 0; ; ++i) {
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		DWORD val = getDword(setting_name);
		if (val == 0)
			break;

		list_node << XCHILD("device") << XATTRI64("id", val);
	}
	list_node << XCHILD("device") << XATTRI64("id", own_id);

	// send device list back
	//TODOL handle response
	m_ThreadInfo->send(iq);
}

struct db_enum_settings_prekeys_cb_data
{
	std::list<char*> settings; //TODO: check this
};

int db_enum_settings_prekeys_cb(const char *szSetting, void *lParam)
{
	db_enum_settings_prekeys_cb_data *data = (db_enum_settings_prekeys_cb_data*)lParam;
	if (strstr(szSetting, "OmemoPreKey") && strstr(szSetting, "Public")) //TODO: suboptimal code, use different names for simple searching
		data->settings.push_back(mir_strdup(szSetting));

	return 0;
}

void CJabberProto::OmemoSendBundle()
{
	// get own device id
	DWORD own_id = m_omemo.GetOwnDeviceId();

	// construct bundle node
	char szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq("set", SerialNext());
	iq << XATTR("from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));

	TiXmlElement *publish_node = iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub") << XCHILD("publish");
	{
		char attr_val[128];
		mir_snprintf(attr_val, "%s.bundles:%u", JABBER_FEAT_OMEMO, own_id);
		publish_node << XATTR("node", attr_val);
	}
	TiXmlElement *bundle_node = publish_node << XCHILD("item") << XCHILDNS("bundle", JABBER_FEAT_OMEMO);

	// add signed pre key public
	bundle_node << XCHILD("signedPreKeyPublic", ptrA(getUStringA("OmemoSignedPreKeyPublic"))) << XATTRI("signedPreKeyId", 1);

	// add pre key signature
	bundle_node << XCHILD("signedPreKeySignature", ptrA(getUStringA("OmemoSignedPreKeySignature")));

	// add identity key
	// it is must be a public key right ?, standart is a bit confusing...
	bundle_node << XCHILD("identityKey", ptrA(getUStringA("OmemoDevicePublicKey")));

	// add prekeys
	TiXmlElement *prekeys_node = XmlAddChild(bundle_node, "prekeys");

	db_enum_settings_prekeys_cb_data *ud = new db_enum_settings_prekeys_cb_data;
	db_enum_settings(0, &db_enum_settings_prekeys_cb, m_szModuleName, ud);
	for (std::list<char*>::iterator i = ud->settings.begin(), end = ud->settings.end(); i != end; i++) {
		ptrA val(getUStringA(*i));
		if (val) {
			unsigned int key_id = 0;
			char *p = *i, buf[5] = { 0 };
			p += strlen("OmemoPreKey");
			int i2 = 0;
			for (char c = 0; c != 'P'; i2++, c = p[i2])
				;
			memcpy(buf, p, i2);
			buf[i2 + 1] = 0;
			key_id = atoi(buf);
			prekeys_node << XCHILD("preKeyPublic", val) << XATTRI("preKeyId", key_id);
		}
		mir_free(*i);
	}
	ud->settings.clear();
	delete ud;
	char setting_name[64];
	for (int i = 0;; i++) {
		mir_snprintf(setting_name, "OmemoPreKey%dPublic", i);
		ptrA val(getUStringA(setting_name));
		if (val == nullptr)
			break;

		prekeys_node << XCHILD("preKeyPublic", val) << XATTRI("preKeyId", i + 1);
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
	if (m_omemo.session_checked[hContact])
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
	while (id) {
		if (!checked) {
			pending_check = true;
			XmlNodeIq iq(AddIQ(&CJabberProto::OmemoOnIqResultGetBundle, JABBER_IQ_TYPE_GET, nullptr, &id));

			char szBareJid[JABBER_MAX_JID_LEN];
			iq << XATTR("from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof_portable(szBareJid)));

			char *jid = ContactToJID(hContact);
			iq << XATTR("to", jid);
			
			TiXmlElement *items = iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub") << XCHILD("items");
			char bundle[64];
			mir_snprintf(bundle, "%s%s%u", JABBER_FEAT_OMEMO, ".bundles:", id);
			XmlAddAttr(items, "node", bundle);
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

	if (!pending_check) {
		m_omemo.session_checked[hContact] = true;
		OmemoHandleMessageQueue();
		return true;
	}

	debugLogA("Jabber OMEMO: info: OmemoCheckSession: pending session creation");
	return false;
}

void CJabberProto::OmemoOnIqResultGetBundle(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode == nullptr)
		return;

	const char *jid = XmlGetAttr(iqNode, "from");
	MCONTACT hContact = HContactFromJID(jid);

	const char *type = XmlGetAttr(iqNode, "type");
	if (mir_strcmp(type, "result")) {
		// failed to get bundle, do not try to build session
		unsigned int *dev_id = (unsigned int*)pInfo->GetUserData();
		char setting_name[64], setting_name2[64];
		DWORD id = 0;
		int i = 0;

		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		mir_snprintf(setting_name2, "%sChecked", setting_name);
		db_set_resident(m_szModuleName, setting_name2);
		id = getDword(hContact, setting_name, 0);
		while (id) {
			if (id == *dev_id) {
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

	auto *pubsub = XmlGetChildByTag(iqNode, "pubsub", "xmlns", "http://jabber.org/protocol/pubsub");
	if (!pubsub) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain pubsub node");
		return;
	}

	auto *items = XmlFirstChild(pubsub, "items");
	const char *items_node_val = XmlGetAttr(items, "node");
	const char *device_id = items_node_val;
	device_id += mir_wstrlen(JABBER_FEAT_OMEMO L".bundles:");
	
	auto *bundle = XmlFirstChild(XmlFirstChild(items, "item"), "bundle");
	if (!bundle) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain bundle node");
		return;
	}
	auto *signedPreKeyPublic = XmlFirstChild(bundle, "signedPreKeyPublic");
	if (!signedPreKeyPublic) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain signedPreKeyPublic node");
		return;
	}
	const char *signedPreKeyId = XmlGetAttr(signedPreKeyPublic, "signedPreKeyId");
	if (!signedPreKeyId) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain signedPreKeyId attr");
		return;
	}
	auto *signedPreKeySignature = XmlFirstChild(bundle, "signedPreKeySignature");
	if (!signedPreKeySignature) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain signedPreKeySignature node");
		return;
	}
	auto *identityKey = XmlFirstChild(bundle, "identityKey");
	if (!identityKey) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain identityKey node");
		return;
	}
	
	auto *prekeys = XmlFirstChild(bundle, "prekeys");
	if (!prekeys) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain prekeys node");
		return;
	}

	auto *prekey_node = XmlFirstChild(prekeys, "preKeyPublic");
	if (!prekey_node) {
		debugLogA("Jabber OMEMO: error: device bundle does not contain preKeyPublic node");
		return;
	}

	const char *preKeyPublic = prekey_node->GetText();
	if (!preKeyPublic) {
		debugLogA("Jabber OMEMO: error: failed to get preKeyPublic data");
		return;
	}

	const char *preKeyId = XmlGetAttr(prekey_node, "preKeyId");
	if (!preKeyId) {
		debugLogA("Jabber OMEMO: error: failed to get preKeyId data");
		return;
	}

	if (!m_omemo.create_session_store(hContact, device_id)) {
		debugLogA("Jabber OMEMO: error: omemo::create_session_store failed");
		return; //failed to create session store
	}

	if (!m_omemo.build_session(hContact, jid, device_id, preKeyId, preKeyPublic, signedPreKeyId, signedPreKeyPublic->GetText(), signedPreKeySignature->GetText(), identityKey->GetText())) {
		debugLogA("Jabber OMEMO: error: omemo::build_session failed");
		return; //failed to build signal(omemo) session
	}

	unsigned int *dev_id = (unsigned int*)pInfo->GetUserData();
	char setting_name[64], setting_name2[64];
	DWORD id = 0;
	int i = 0;

	mir_snprintf(setting_name, "OmemoDeviceId%d", i);
	mir_snprintf(setting_name2, "%sChecked", setting_name);
	db_set_resident(m_szModuleName, setting_name2);
	id = getDword(hContact, setting_name, 0);
	while (id) {
		if (id == *dev_id) {
			setByte(hContact, setting_name2, 1);
			break;
		}
		i++;
		mir_snprintf(setting_name, "OmemoDeviceId%d", i);
		mir_snprintf(setting_name2, "%sChecked", setting_name);
		db_set_resident(m_szModuleName, setting_name2);
		id = getDword(hContact, setting_name, 0);
	}

	OmemoCheckSession(hContact);
}

unsigned int CJabberProto::OmemoEncryptMessage(XmlNode &msg, const char *msg_text, MCONTACT hContact)
{
	const EVP_CIPHER *cipher = EVP_aes_128_gcm();
	unsigned char key[16], iv[12], tag[16] /*, aad[48]*/;
	Utils_GetRandom(key, _countof_portable(key));
	Utils_GetRandom(iv, _countof_portable(iv));
	Utils_GetRandom(tag, _countof_portable(tag));
	//Utils_GetRandom(aad, _countof_portable(aad));
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, _countof_portable(iv), nullptr);
	EVP_EncryptInit(ctx, cipher, key, iv);
	char *out;
	const size_t inl = strlen(msg_text);
	int tmp_len = 0, outl;
	//EVP_EncryptUpdate(ctx, nullptr, &outl, aad, _countof_portable(aad));
	out = (char*)mir_alloc(inl + _countof_portable(key) - 1);
	for (;;) {
		EVP_EncryptUpdate(ctx, (unsigned char*)(out + tmp_len), &outl, (unsigned char*)(msg_text + tmp_len), (int)(inl - tmp_len));
		tmp_len += outl;
		if (tmp_len >= (int)inl - 16 + 1) //cast to int is required here
			break;
	}
	
	EVP_EncryptFinal(ctx, (unsigned char*)(msg_text + tmp_len), &outl);
	tmp_len += outl;
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, _countof_portable(tag), tag);
	EVP_CIPHER_CTX_free(ctx);

	TiXmlElement *encrypted = msg << XCHILDNS("encrypted", JABBER_FEAT_OMEMO);
	TiXmlElement *payload = encrypted << XCHILD("payload");
	payload->SetText(ptrA(mir_base64_encode(out, tmp_len)).get());

	TiXmlElement *header = encrypted << XCHILD("header");
	header << XATTRI64("sid", m_omemo.GetOwnDeviceId());
	unsigned int session_count = 0;
	char key_plus_tag[32] = { 0 };
	memcpy(key_plus_tag, key, 16);
	{
		char *ptr = key_plus_tag + 16;
		memcpy(ptr, tag, 16);
	}

	for (auto &i : m_omemo.sessions[hContact]) {
		if (!i.second.cipher) {
			debugLogA("Jabber OMEMO: bug: omemo session does not have valid cipher");
			continue;
		}
		unsigned int intdev_id = i.first;
		ciphertext_message *encrypted_key;
		if (session_cipher_encrypt(i.second.cipher, (uint8_t*)key_plus_tag, 32, &encrypted_key) != SG_SUCCESS) {
			// TODO: handle encryption error
			debugLogA("Jabber OMEMO: bug: session_cipher_encrypt failed");
			continue;
		}

		TiXmlElement *key_node = header << XCHILD("key");
		key_node << XATTRI64("rid", intdev_id);
		int msg_type = ciphertext_message_get_type(encrypted_key);
		if (msg_type == CIPHERTEXT_PREKEY_TYPE)
			key_node << XATTR("prekey", "true");

		signal_buffer *serialized_encrypted_key = ciphertext_message_get_serialized(encrypted_key);
		ptrA key_base64(mir_base64_encode(signal_buffer_data(serialized_encrypted_key), signal_buffer_len(serialized_encrypted_key)));
		key_node->SetText(key_base64.get());

		SIGNAL_UNREF(encrypted_key);
		session_count++;
	}

	TiXmlElement *iv_node = header << XCHILD("iv");
	iv_node->SetText(ptrA(mir_base64_encode(iv, _countof_portable(iv))).get());

	msg << XCHILDNS("store", "urn:xmpp:hints");
	if (!session_count)
		debugLogA("Jabber OMEMO: error: message does not encrypted for any sessions");

	return session_count;
}

bool CJabberProto::OmemoIsEnabled(MCONTACT /*hContact*/)
{
	//TODO:
	return true;
}
