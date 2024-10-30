/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2017-24 Miranda NG team

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

namespace omemo
{
	int random_func(uint8_t *data, size_t len, void * /*user_data*/)
	{
		Utils_GetRandom(data, len);
		return 0;
	}

	int hmac_sha256_init_func(void **hmac_context, const uint8_t *key, size_t key_len, void * /*user_data*/)
	{
		HMAC_CTX *ctx = HMAC_CTX_new();
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

	void hmac_sha256_cleanup_func(void *hmac_context, void * /*user_data*/)
	{
		HMAC_CTX *ctx = (HMAC_CTX*)hmac_context;
		HMAC_CTX_free(ctx);
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

		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		if (!ctx) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		result = EVP_EncryptInit_ex(ctx, evp_cipher, nullptr, key, iv);
		if (!result) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			result = EVP_CIPHER_CTX_set_padding(ctx, 0);
			if (!result) {
				result = SG_ERR_UNKNOWN;
				goto complete;
			}
		}

		out_buf = (uint8_t*)mir_alloc(sizeof(uint8_t) * (plaintext_len + EVP_CIPHER_block_size(evp_cipher)));
		if (!out_buf) {
			result = SG_ERR_NOMEM;
			goto complete;
		}

		int out_len = 0;
		result = EVP_EncryptUpdate(ctx, out_buf, &out_len, plaintext, (int)plaintext_len);
		if (!result) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		int final_len = 0;
		result = EVP_EncryptFinal_ex(ctx, out_buf + out_len, &final_len);
		if (!result) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		*output = signal_buffer_create(out_buf, out_len + final_len);

complete:
		EVP_CIPHER_CTX_free(ctx);
		mir_free(out_buf);
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

		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		if (!ctx) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		result = EVP_DecryptInit_ex(ctx, evp_cipher, nullptr, key, iv);
		if (!result) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			result = EVP_CIPHER_CTX_set_padding(ctx, 0);
			if (!result) {
				result = SG_ERR_UNKNOWN;
				goto complete;
			}
		}

		out_buf = (uint8_t*)mir_alloc(sizeof(uint8_t) * (ciphertext_len + EVP_CIPHER_block_size(evp_cipher)));
		if (!out_buf) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		int out_len = 0;
		result = EVP_DecryptUpdate(ctx, out_buf, &out_len, ciphertext, (int)ciphertext_len);
		if (!result) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		int final_len = 0;
		result = EVP_DecryptFinal_ex(ctx, out_buf + out_len, &final_len);
		if (!result) {
			result = SG_ERR_UNKNOWN;
			goto complete;
		}

		*output = signal_buffer_create(out_buf, out_len + final_len);

complete:
		EVP_CIPHER_CTX_free(ctx);
		if (out_buf) {
			mir_free(out_buf);
		}
		return result;
	}

	static void myLock(void *user_data)
	{
		omemo_impl *omi = (omemo_impl*)user_data;
		omi->lock();
	}

	static void myUnlock(void *user_data)
	{
		omemo_impl *omi = (omemo_impl*)user_data;
		omi->unlock();
	}

	struct outgoing_message
	{
		outgoing_message(MCONTACT h, char *p)
		{
			hContact = h;
			pszSrc = p;
		}

		MCONTACT hContact;
		char *pszSrc;
	};

	omemo_impl::omemo_impl(CJabberProto *p) :
		proto(p)
	{
		if (proto->m_bUseOMEMO)
			init();
	}

	int db_enum_settings_trash_cb(const char *szSetting, void *lParam)
	{
		if (strstr(szSetting, "OmemoPreKey") == szSetting)
			((CJabberProto *)lParam)->delSetting(szSetting);
		return 0;
	}

	void omemo_impl::init()
	{
		if (global_context)
			return;

		// some db cleanup
		if (!proto->getMStringA("OmemoFingerprintOwn").IsEmpty()) {
			db_enum_settings(0, &db_enum_settings_trash_cb, proto->m_szModuleName, proto);

			proto->delSetting(CMStringA(FORMAT, "OmemoSignalPreKey_%d512", GetOwnDeviceId()));
			proto->delSetting("OmemoSignedPreKeyPublic");
			proto->delSetting("OmemoSignedPreKeySignature");
			proto->delSetting("OmemoFingerprintOwn");
		}

		signal_context_create(&global_context, this);

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

		if (signal_context_set_crypto_provider(global_context, &provider)) {
			proto->debugLogA("Jabber OMEMO: signal_context_set_crypto_provider failed");
			//TODO: handle error
		}

		if (signal_context_set_locking_functions(global_context, &myLock, &myUnlock)) {
			proto->debugLogA("Jabber OMEMO: signal_context_set_crypto_provider failed");
			//TODO: handle error
		}

		create_session_store();
	}

	omemo_impl::~omemo_impl()
	{
		deinit();
	}

	void omemo_impl::deinit()
	{
		signal_protocol_store_context_destroy(store_context);
		signal_context_destroy(global_context);
		global_context = nullptr;
		store_context = nullptr;
	}

	int omemo_impl::GetOwnDeviceId()
	{
		int own_id = proto->getDword("OmemoDeviceId", 0);
		if (own_id == 0) {
			proto->OmemoInitDevice();
			own_id = proto->getDword("OmemoDeviceId", 0);
		}
		return own_id;
	}

	void omemo_impl::RefreshPrekeys(ratchet_identity_key_pair *device_key, int32_t dev_id)
	{
		if (!device_key || !dev_id)
			return;

		// generate and save signed pre key
		session_signed_pre_key *signed_pre_key;
		{
			const unsigned int signed_pre_key_id = 1;
			signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, device_key, signed_pre_key_id, time(0), global_context);

			SignalBuffer buf(signed_pre_key);
			CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", dev_id, signed_pre_key_id);
			db_set_blob(0, proto->m_szModuleName, szSetting, buf.data(), buf.len());
			SIGNAL_UNREF(signed_pre_key);
		}

		// generate and save pre keys set
		signal_protocol_key_helper_pre_key_list_node *keys_root;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, 1, 100, global_context);
		CMStringA szSetting;
		for (auto *it = keys_root; it; it = signal_protocol_key_helper_key_list_next(it)) {
			session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
			uint32_t pre_key_id = session_pre_key_get_id(pre_key);

			SignalBuffer buf(pre_key);
			szSetting.Format("%s%u%d", "OmemoSignalPreKey_", GetOwnDeviceId(), pre_key_id);
			db_set_blob(0, proto->m_szModuleName, szSetting, buf.data(), buf.len());
		}
		signal_protocol_key_helper_key_list_free(keys_root);

		if (proto->m_bJabberOnline)
			proto->OmemoSendBundle();
	}

	static CMStringA getSessionSetting(const signal_protocol_address *address)
	{
		ptrA id_buf((char*)mir_alloc(address->name_len + sizeof(int32_t)));
		memcpy(id_buf, address->name, address->name_len);
		memcpy(id_buf.get() + address->name_len, &address->device_id, sizeof(int32_t));

		ptrA id_str(mir_base64_encode(id_buf, address->name_len + sizeof(int32_t)));
		return CMStringA("OmemoSignalSession_") + id_str;
	}

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
		auto *proto = (CJabberProto *)user_data;

		if ((unsigned long)address->device_id == 0) {
			proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (invalid device id)");
			return -1;
		}

		if (address->name_len > JABBER_MAX_JID_LEN) {
			proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (invalid address length)");
			return -1;
		}

		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(proto->HContactFromJID(address->name), proto->m_szModuleName, getSessionSetting(address), &dbv);
		if (!dbv.cpbVal) {
			db_free(&dbv);
			proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load session (session does not exist)");
			return 0;
		}
		*record = signal_buffer_create(dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		return 1; // session exists and successfully loaded
	}

	struct db_enum_settings_sub_cb_data
	{
		CJabberProto *proto;
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

		auto *proto = (CJabberProto *)user_data;
		signal_int_list *l = signal_int_list_alloc();
		unsigned int array_size = 0;

		db_enum_settings_sub_cb_data *ud = new db_enum_settings_sub_cb_data(array_size);
		ud->proto = proto;
		ud->sessions = l;
		ud->name = name;
		ud->name_len = name_len;
		db_enum_settings(proto->HContactFromJID(name), &db_enum_settings_sub_cb, proto->m_szModuleName, ud);
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

		auto *proto = (CJabberProto *)user_data;
		db_set_blob(proto->HContactFromJID(address->name), proto->m_szModuleName, getSessionSetting(address), record, (unsigned int)record_len); //TODO: check return value
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
		auto *proto = (CJabberProto *)user_data;

		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(proto->HContactFromJID(address->name), proto->m_szModuleName, getSessionSetting(address), &dbv);
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

		auto *proto = (CJabberProto *)user_data;
		db_unset(proto->HContactFromJID(address->name), proto->m_szModuleName, getSessionSetting(address));
		return 1;
	}

	struct db_enum_settings_del_all_cb_data
	{
		std::list<char*> settings;
		CJabberProto *proto;
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

		auto *proto = (CJabberProto *)user_data;
		db_enum_settings_del_all_cb_data *ud = new db_enum_settings_del_all_cb_data;
		ud->proto = proto;
		ud->name = name;
		ud->name_len = name_len;
		MCONTACT hContact = proto->HContactFromJID(name);
		db_enum_settings(hContact, &db_enum_settings_del_all_cb, proto->m_szModuleName, (void*)ud);
		int count = 0;
		for (std::list<char*>::iterator i = ud->settings.begin(), end = ud->settings.end(); i != end; i++) {
			db_unset(hContact, proto->m_szModuleName, *i);
			mir_free(*i);
			count++;
		}
		delete ud;
		return count;
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

		auto *proto = (CJabberProto *)user_data;

		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalPreKey_", proto->m_omemo.GetOwnDeviceId(), pre_key_id);

		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, proto->m_szModuleName, szSetting, &dbv);
		if (!dbv.cpbVal) {
			db_free(&dbv);
			proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load prekey SG_ERR_INVALID_KEY_ID");
			return SG_ERR_INVALID_KEY_ID;
		}
		*record = signal_buffer_create(dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		return SG_SUCCESS; //key exist and successfully loaded
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

		auto *proto = (CJabberProto *)user_data;

		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalPreKey_", proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		db_set_blob(0, proto->m_szModuleName, szSetting, record, (unsigned int)record_len); //TODO: check return value

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

		auto *proto = (CJabberProto *)user_data;
		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalPreKey_", proto->m_omemo.GetOwnDeviceId(), pre_key_id);

		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, proto->m_szModuleName, szSetting, &dbv);
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

		auto *proto = (CJabberProto *)user_data;

		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalPreKey_", proto->m_omemo.GetOwnDeviceId(), pre_key_id);
		db_unset(0, proto->m_szModuleName, szSetting);
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

		auto *proto = (CJabberProto *)user_data;
		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		MBinBuffer blob(proto->getBlob(szSetting));
		if (blob.isEmpty()) {
			proto->debugLogA("Jabber OMEMO: libsignal data backend impl: failed to load signed prekey SG_ERR_INVALID_KEY_ID");
			return SG_ERR_INVALID_KEY_ID;
		}
		*record = signal_buffer_create(blob.data(), blob.length());
		return SG_SUCCESS; //key exist and successfully loaded
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

		auto *proto = (CJabberProto *)user_data;
		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		db_set_blob(0, proto->m_szModuleName, szSetting, record, (unsigned int)record_len); //TODO: check return value
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

		auto *proto = (CJabberProto *)user_data;
		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		DBVARIANT dbv = { 0 };
		dbv.type = DBVT_BLOB;
		db_get(0, proto->m_szModuleName, szSetting, &dbv);
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

		auto *proto = (CJabberProto *)user_data;
		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalSignedPreKey_", proto->m_omemo.GetOwnDeviceId(), signed_pre_key_id);
		db_unset(0, proto->m_szModuleName, szSetting);
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

		auto *proto = (CJabberProto *)user_data;
		ptrA pub_key(proto->getStringA("OmemoDevicePublicKey"));
		ptrA priv_key(proto->getStringA("OmemoDevicePrivateKey"));

		size_t pub_key_len = 0, priv_key_len = 0;
		ptrA pub_key_buf((char *)mir_base64_decode(pub_key, &pub_key_len));
		ptrA priv_key_buf((char *)mir_base64_decode(priv_key, &priv_key_len));

		*public_data = signal_buffer_create((uint8_t*)pub_key_buf.get(), pub_key_len);
		*private_data = signal_buffer_create((uint8_t*)priv_key_buf.get(), priv_key_len);
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

		auto *proto = (CJabberProto *)user_data;
		*registration_id = proto->m_omemo.GetOwnDeviceId();
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

		auto *proto = (CJabberProto *)user_data;
		ptrA id_buf((char *)mir_alloc(address->name_len + sizeof(int32_t)));
		memcpy(id_buf.get(), address->name, address->name_len);
		memcpy(id_buf.get() + address->name_len, &address->device_id, sizeof(int32_t));

		CMStringA szSetting(omemo::IdentityPrefix);
		szSetting.Append(ptrA(mir_base64_encode(id_buf, address->name_len + sizeof(int32_t))));

		if (key_data != nullptr)
			db_set_blob(proto->HContactFromJID(address->name), proto->m_szModuleName, szSetting, key_data, (unsigned int)key_len); //TODO: check return value
		else
			db_unset(proto->HContactFromJID(address->name), proto->m_szModuleName, szSetting);
		return 0;
	}

	int db_enum_settings_fps_cb(const char *szSetting, void *lParam)
	{
		if (strstr(szSetting, "OmemoFingerprintTrusted_") == szSetting)
			(*(uint32_t*)lParam)++;

		return 0;
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

		auto *proto = (CJabberProto *)user_data;
		CMStringA fp_hex(omemo::hex_string(key_data, key_len));
		CMStringA TrustSettingName("OmemoFingerprintTrusted_" + fp_hex);

		MCONTACT hContact = proto->HContactFromJID(address->name);
		char val = proto->getByte(hContact, TrustSettingName, FP_ABSENT);
		if (val == FP_ABSENT) {
			uint32_t count = 0;
			db_enum_settings(hContact, omemo::db_enum_settings_fps_cb, proto->m_szModuleName, &count);
			if (count) {
				proto->MsgPopup(hContact, TranslateT("Unknown OMEMO device added!"),
					hContact ? Clist_GetContactDisplayName(hContact) : TranslateT("(My devices)"));
				proto->setByte(hContact, TrustSettingName, FP_BAD);
			}
		}

		//always return true to decrypt incoming messages from untrusted devices
		//we must check trust manually when send messages
		return true;
	}

	bool omemo_impl::create_session_store()
	{
		/* Create the data store context, and add all the callbacks to it */
		signal_protocol_store_context_create(&store_context, global_context);

		signal_protocol_session_store ss;
		ss.contains_session_func = &contains_session_func;
		ss.delete_all_sessions_func = &delete_all_sessions_func;
		ss.delete_session_func = &delete_session_func;
		ss.destroy_func = nullptr;
		ss.get_sub_device_sessions_func = &get_sub_device_sessions_func;
		ss.load_session_func = &load_session_func;
		ss.store_session_func = &store_session_func;
		ss.user_data = proto;
		signal_protocol_store_context_set_session_store(store_context, &ss);

		signal_protocol_pre_key_store sp;
		sp.contains_pre_key = &contains_pre_key;
		sp.destroy_func = nullptr;
		sp.load_pre_key = &load_pre_key;
		sp.remove_pre_key = &remove_pre_key;
		sp.store_pre_key = &store_pre_key;
		sp.user_data = proto;
		signal_protocol_store_context_set_pre_key_store(store_context, &sp);

		signal_protocol_signed_pre_key_store ssp;
		ssp.contains_signed_pre_key = &contains_signed_pre_key;
		ssp.destroy_func = nullptr;
		ssp.load_signed_pre_key = &load_signed_pre_key;
		ssp.remove_signed_pre_key = &remove_signed_pre_key;
		ssp.store_signed_pre_key = &store_signed_pre_key;
		ssp.user_data = proto;
		signal_protocol_store_context_set_signed_pre_key_store(store_context, &ssp);

		signal_protocol_identity_key_store sip;
		sip.destroy_func = nullptr;
		sip.get_identity_key_pair = &get_identity_key_pair;
		sip.get_local_registration_id = &get_local_registration_id;
		sip.is_trusted_identity = &is_trusted_identity;
		sip.save_identity = &save_identity;
		sip.user_data = proto;
		signal_protocol_store_context_set_identity_key_store(store_context, &sip);
		return true; //success
	}

	bool omemo_impl::build_session(const char *jid, const char *dev_id, const char *key_id, const char *pre_key_public, const char *signed_pre_key_id,
		const char *signed_pre_key_public, const char *signed_pre_key_signature, const char *identity_key)
	{
		// Instantiate a session_builder for a recipient address.
		int32_t dev_id_int = strtol(dev_id, nullptr, 10);
		signal_protocol_address address = { jid, mir_strlen(jid), dev_id_int };

		session_builder *builder;
		if (session_builder_create(&builder, store_context, &address, global_context) < 0) {
			proto->debugLogA("Jabber OMEMO: error: session_builder_create failed");
			return false; //failure
		}

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

		// load the identity key
		ec_public_key *identity_key_p;
		key_buf = (uint8_t*)mir_base64_decode(identity_key, &key_buf_len);
		if (curve_decode_point(&identity_key_p, key_buf, key_buf_len, global_context)) {
			proto->debugLogA("Jabber OMEMO: error: curve_decode_point failed to parse identity key");
			return false; // TODO: cleanup
		}
		mir_free(key_buf); // TODO: check this

		key_buf = (uint8_t*)mir_base64_decode(signed_pre_key_signature, &key_buf_len);
		session_pre_key_bundle *retrieved_pre_key;
		uint32_t registration_id = 0;
		signal_protocol_identity_get_local_registration_id(store_context, &registration_id);
		session_pre_key_bundle_create(&retrieved_pre_key, registration_id, dev_id_int, key_id_int, prekey, signed_pre_key_id_int, signed_prekey, key_buf, key_buf_len, identity_key_p);
		mir_free(key_buf);

		/* Build a session with the pre-key retrieved from the server. */
		int ret = session_builder_process_pre_key_bundle(builder, retrieved_pre_key);
		switch (ret) {
		case SG_SUCCESS:
			break;
		case SG_ERR_UNTRUSTED_IDENTITY:
			signal_protocol_identity_save_identity(store_context, &address, identity_key_p);
			proto->debugLogA("Jabber OMEMO: Refused to build session with untrusted identity");
			break;
		case SG_ERR_INVALID_KEY:
			proto->debugLogA("Jabber OMEMO: session_builder_process_pre_key_bundle failure SG_ERR_INVALID_KEY");
			break;
		default:
			proto->debugLogA("Jabber OMEMO: session_builder_process_pre_key_bundle failed with unknown error");
			break;
		}

		SIGNAL_UNREF(retrieved_pre_key);
		SIGNAL_UNREF(identity_key_p);
		SIGNAL_UNREF(signed_prekey);
		SIGNAL_UNREF(prekey);
		session_builder_free(builder);

		return ret == SG_SUCCESS; //success
	}

	void OmemoRefreshUsedPreKey(CJabberProto *proto, pre_key_signal_message *psm)
	{
		uint32_t id = pre_key_signal_message_get_pre_key_id(psm);

		// generate and save the pre-keys set
		signal_protocol_key_helper_pre_key_list_node *keys_root;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, id, 1, proto->m_omemo.global_context);

		SignalBuffer buf(signal_protocol_key_helper_key_list_element(keys_root));
		CMStringA szSetting(FORMAT, "%s%u%d", "OmemoSignalPreKey_", proto->m_omemo.GetOwnDeviceId(), id);
		db_set_blob(0, proto->m_szModuleName, szSetting, buf.data(), buf.len());

		signal_protocol_key_helper_key_list_free(keys_root);

		//	proto->OmemoAnnounceDevice();
		proto->OmemoSendBundle();
	}

	CMStringW FormatFingerprint(const char *pszHexString)
	{
		CMStringW buf;
		if (pszHexString) {
			int i = 0;
			const char *p = pszHexString;
			if (*p && *(p + 1)) p += 2;
			for (; *p; p++) {
				buf.AppendChar(toupper(*p));
				i++;
				if (i % 8 == 0)
					buf.AppendChar(' ');
				if (i % 32 == 0)
					buf.AppendChar('\n');
			}
		}

		return buf;
	}

	// Some DB helpers
	int omemo_impl::dbGetDeviceId(MCONTACT hContact, uint32_t number)
	{
		CMStringA szSetting(FORMAT, "%s%u", DevicePrefix, number);
		return proto->getDword(hContact, szSetting, 0);
	}

	CMStringA omemo_impl::dbGetSuffix(MCONTACT hContact, int device_id)
	{
		ptrA jid(proto->getStringA(hContact, "jid"));
		if (!jid)
			return CMStringA();

		return dbGetSuffix(jid, device_id);
	}

	CMStringA omemo_impl::dbGetSuffix(const char *jid, int device_id)
	{
		if (!jid || device_id <= 0)
			return CMStringA();

		size_t len = strlen(jid);
		char *jiddev = (char *)mir_alloc(len + sizeof(int32_t));
		memcpy(jiddev, jid, len);
		memcpy(jiddev + len, &device_id, sizeof(int32_t));
		ptrA suffix(mir_base64_encode(jiddev, len + sizeof(int32_t)));
		mir_free(jiddev);

		return CMStringA(suffix);
	}

	int omemo_impl::TOFUAllDevices(MCONTACT hContact)
	{
		int i;
		for (i = 0;; i++) {
			int device_id = dbGetDeviceId(hContact, i);
			if (device_id == 0)
				break;

			MBinBuffer fp(proto->getBlob(hContact, IdentityPrefix + dbGetSuffix(hContact, device_id)));
			CMStringA fp_hex(hex_string(fp.data(), fp.length()));
			proto->setByte(hContact, "OmemoFingerprintTrusted_" + fp_hex, FP_TOFU);
		}

		if (i) {
			POPUPDATAW ppd;
			ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_HTTP_AUTH));
			if (hContact) {
				ppd.lchContact = hContact;
				wcsncpy(ppd.lpwzContactName, Clist_GetContactDisplayName(hContact), MAX_CONTACTNAME - 1);
			}
			else
				wcsncpy(ppd.lpwzContactName, TranslateT("(My devices)"), MAX_CONTACTNAME - 1);

			wcsncpy(ppd.lpwzText,
				CMStringW(FORMAT, TranslateT("Trust on first use for\n%d devices"), i),
				MAX_SECONDLINE - 1);
			PUAddPopupW(&ppd);
		}

		return i;
	}

	CMStringA hex_string(const uint8_t *pData, const size_t length)
	{
		CMStringA hexstr;
		if (pData) {
			hexstr.Truncate((int)length * 2);
			bin2hex(pData, length, hexstr.GetBuffer());
		}
		return hexstr;
	}
};

void CJabberProto::OmemoInitDevice()
{
	ratchet_identity_key_pair *device_key;
	int32_t dev_id = getDword("OmemoDeviceId");

	if (!dev_id || signal_protocol_identity_get_key_pair(m_omemo.store_context, &device_key) < 0) {
		// generate and save device id
		do {
			Utils_GetRandom((void *)&(dev_id), 4);
			dev_id &= ~0x80000000;
		} while (!dev_id);
		setDword("OmemoDeviceId", dev_id);

		// generate and save a device key
		if (signal_protocol_key_helper_generate_identity_key_pair(&device_key, m_omemo.global_context)) {
			debugLogA("Jabber OMEMO: signal_protocol_key_helper_generate_identity_key_pair failed");
			//TODO: handle error
		}
		setString("OmemoDevicePublicKey",
			SignalBuffer(ratchet_identity_key_pair_get_public(device_key)).toBase64());
		setString("OmemoDevicePrivateKey",
			SignalBuffer(ratchet_identity_key_pair_get_private(device_key)).toBase64());

		m_omemo.RefreshPrekeys(device_key, dev_id);
	}
	SIGNAL_UNREF(device_key);
}

void CJabberProto::OmemoPutMessageToOutgoingQueue(MCONTACT hContact, const char *pszSrc)
{
	char *msg = mir_strdup(pszSrc);
	m_omemo.outgoing_messages.push_back(omemo::outgoing_message(hContact, msg));
}

void CJabberProto::OmemoHandleMessageQueue()
{
	for (auto &i : m_omemo.outgoing_messages) {
		SendMsg(i.hContact, 0, i.pszSrc);
		mir_free(i.pszSrc);
	}
	m_omemo.outgoing_messages.clear();
}

uint32_t JabberGetLastContactMessageTime(MCONTACT hContact);

bool CJabberProto::OmemoHandleMessage(const TiXmlElement *node, const char *jid, time_t msgTime, bool isCarbon)
{
	auto *header_node = XmlFirstChild(node, "header");
	if (!header_node) {
		debugLogA("Jabber OMEMO: error: omemo message does not contain header");
		return true; //this should never happen
	}

	const char *iv_base64 = XmlGetChildText(header_node, "iv");
	if (!iv_base64) {
		Netlib_Log(nullptr, "Jabber OMEMO: error: failed to get iv data");
		return true;
	}

	const char *sender_dev_id = XmlGetAttr(header_node, "sid");
	if (!sender_dev_id) {
		debugLogA("Jabber OMEMO: error: failed to get sender device id");
		return true;
	}

	int32_t sender_dev_id_int = strtol(sender_dev_id, nullptr, 10);

	uint32_t own_id = m_omemo.GetOwnDeviceId();
	const char *encrypted_key_base64 = nullptr;
	bool isprekey = false;
	for (auto *it : TiXmlFilter(header_node, "key")) {
		uint32_t dev_id_int = it->IntAttribute("rid");
		if (dev_id_int == own_id) {
			encrypted_key_base64 = it->GetText();
			isprekey = it->BoolAttribute("prekey");
			break;
		}
	}

	CMStringA result;
	auto *payload_base64 = XmlGetChildText(node, "payload");
	if (encrypted_key_base64) {
		size_t encrypted_key_len;
		mir_ptr<uint8_t> encrypted_key((uint8_t *)mir_base64_decode(encrypted_key_base64, &encrypted_key_len));

		size_t iv_len;
		mir_ptr<uint8_t> iv((uint8_t *)mir_base64_decode(iv_base64, &iv_len));

		session_cipher *cipher;
		char szBareJid[JABBER_MAX_JID_LEN];
		JabberStripJid(isCarbon ? m_szJabberJID : jid, szBareJid, _countof(szBareJid));
		signal_protocol_address address = { szBareJid, mir_strlen(szBareJid), sender_dev_id_int };
		if (session_cipher_create(&cipher, m_omemo.store_context, &address, m_omemo.global_context) != SG_SUCCESS)
			debugLogA("Jabber OMEMO: error: Cannot create session cipher for decrypt");

		signal_buffer *decrypted_key = nullptr;
		bool decrypted = false;
		if (isprekey) {
			//try to decrypt as a pre_key_signal_message
			pre_key_signal_message *pm = nullptr;
			int ret = pre_key_signal_message_deserialize(&pm, encrypted_key, encrypted_key_len, m_omemo.global_context);
			if (ret == SG_SUCCESS && pm) {
				ret = session_cipher_decrypt_pre_key_signal_message(cipher, pm, nullptr, &decrypted_key);
				if (ret == SG_SUCCESS) {
					decrypted = true;
					omemo::OmemoRefreshUsedPreKey(this, pm);
				}
				else debugLogA("Jabber OMEMO: error %d at session_cipher_decrypt_pre_key_signal_message", ret);
				SIGNAL_UNREF(pm);
			}
			else debugLogA("Jabber OMEMO: error %d at pre_key_signal_message_deserialize", ret);
		}
		else { //try to decrypt as a signal message
			signal_message *sm = nullptr;
			int ret = signal_message_deserialize(&sm, encrypted_key, encrypted_key_len, m_omemo.global_context);
			if (ret == SG_SUCCESS && sm) {
				ret = session_cipher_decrypt_signal_message(cipher, sm, nullptr, &decrypted_key);
				if (ret == SG_SUCCESS)
					decrypted = true;
				else
					debugLogA("Jabber OMEMO: error %d at session_cipher_decrypt_signal_message", ret);
				SIGNAL_UNREF(sm);
			}
			else debugLogA("Jabber OMEMO: error %d at signal_message_deserialize", ret);
		}
		session_cipher_free(cipher);
		if (decrypted) {
			if (!payload_base64) {
				signal_buffer_free(decrypted_key);
				debugLogA("Jabber OMEMO: Session accept message received");
				return true;
			}

			size_t payload_len;
			mir_ptr<uint8_t> payload((uint8_t *)mir_base64_decode(payload_base64, &payload_len));
			ptrA out((char *)mir_alloc(payload_len + 32)); //TODO: check this

			//Will be freed by signal_buffer_free(decrypted_key);
			unsigned char *keytag = signal_buffer_data(decrypted_key);
			size_t keytag_len = signal_buffer_len(decrypted_key);

			EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
			EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv_len, NULL);
			EVP_DecryptInit(ctx, EVP_aes_128_gcm(), keytag, iv);
			EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)keytag_len - 16, keytag + 16);

			int outl = 0, round_len = 0;
			EVP_DecryptUpdate(ctx, (unsigned char *)out.get(), &outl, payload, (int)payload_len);
			int dec_success = EVP_DecryptFinal(ctx, (unsigned char *)out.get() + outl, &round_len);
			outl += round_len;
			out[outl] = 0;
			EVP_CIPHER_CTX_free(ctx);
			signal_buffer_free(decrypted_key);

			if (!dec_success)
				result = "<< OMEMO message verification failed. Bug or attack? >>\n";
			result.Append(out);
		}
		else result = "<< Undecryptable incomming OMEMO message >>";
	}
	else {
		if (payload_base64)
			result = "<< OMEMO message is not encrypted for this device >>";
		else
			return true;
	}


	MCONTACT hContact = HContactFromJID(jid);
	MCONTACT carbonContact = isCarbon ? 0 : hContact;

	time_t now = time(0);
	if (!msgTime || m_bFixIncorrectTimestamps && (msgTime > now || (msgTime < (time_t)JabberGetLastContactMessageTime(carbonContact))))
		msgTime = now;

	CMStringA suffix(m_omemo.dbGetSuffix(carbonContact, sender_dev_id_int));
	MBinBuffer fp(getBlob(carbonContact, CMStringA(omemo::IdentityPrefix) + suffix));
	uint8_t trusted = getByte(carbonContact, "OmemoFingerprintTrusted_" + omemo::hex_string(fp.data(), fp.length()));

	char protocol[7], hexkey[89], suburl[5001];
	int ret = sscanf(result.GetBuffer(), "%6[^:]://%5000[^#]#%88s", protocol, suburl, hexkey);
	protocol[6] = hexkey[88] = suburl[5000] = 0;
	if (ret == 3 && !strcmp(protocol, "aesgcm") && strlen(hexkey) == 88) {
		CMStringA szName;
		const char *b = strrchr(suburl, '/') + 1;
		while (*b != 0 && *b != '#' && *b != '?')
			szName.AppendChar(*b++);

		ptrW pwszName(mir_utf8decodeW(szName.c_str()));

		JSONNode root;
		root << WCHAR_PARAM("f", pwszName) << CHAR_PARAM("u", result.GetBuffer());

		DBEVENTINFO dbei = {};
		dbei.szModule = Proto_GetBaseAccountName(hContact);
		dbei.timestamp = msgTime;
		dbei.eventType = EVENTTYPE_FILE;
		if (trusted)
			dbei.flags = DBEF_SECURE;
		if (isCarbon)
			dbei.flags = DBEF_SENT;

		std::string text = root.write();
		dbei.cbBlob = (int)text.size() + 1;
		dbei.pBlob = (char *)text.c_str();
		db_event_add(hContact, &dbei);
	}
	else {
		DB::EventInfo dbei;
		dbei.timestamp = (uint32_t)msgTime;
		dbei.pBlob = result.GetBuffer();
		if (trusted)
			dbei.flags = DBEF_STRONG;
		if (isCarbon)
			dbei.flags = DBEF_SENT;
		ProtoChainRecvMsg(hContact, dbei);
	}

	return true;
}

bool CJabberProto::OmemoHandleDeviceList(const char *from, const TiXmlElement *node)
{
	if (!node)
		return false;

	node = XmlGetChildByTag(XmlFirstChild(node, "item"), "list", "xmlns", JABBER_FEAT_OMEMO);

	MCONTACT hContact = 0;
	if (from) {
		hContact = HContactFromJID(from);
		if (!hContact)
			return true; //unknown jid
	}

	//remove session and pubkey if a device doesn't exist anymore
	for (int i = 0;; i++) {
		int device_id = m_omemo.dbGetDeviceId(hContact, i);
		if (device_id == 0)
			break;

		if (!XmlGetChildByTag(node, "device", "id", CMStringA(FORMAT, "%d", device_id))) {
			CMStringA suffix(m_omemo.dbGetSuffix(hContact, device_id));
			delSetting(hContact, omemo::IdentityPrefix + suffix);
			delSetting(hContact, "OmemoSignalSession_" + suffix);
		}
	}

	//check if our device exists
	bool own_device_listed = false;
	int own_id = m_omemo.GetOwnDeviceId();
	int i = 0;
	for (auto *list_item : TiXmlFilter(node, "device")) {
		int current_id = list_item->IntAttribute("id");
		if (hContact == 0 && current_id == own_id)
			own_device_listed = true;
		else
			setDword(hContact, CMStringA(FORMAT, "OmemoDeviceId%d", i++), current_id);
	}

	for (;; i++) {
		CMStringA szSetting(FORMAT, "OmemoDeviceId%d", i);
		int device_id = getDword(hContact, szSetting);
		if (device_id == 0)
			break;
		delSetting(hContact, szSetting);
	}

	if (hContact == 0 && !own_device_listed) {
		OmemoAnnounceDevice(true, true);
		OmemoSendBundle();
	}

	return true;
}

void CJabberProto::OmemoAnnounceDevice(bool include_cache, bool include_own)
{
	// construct node
	char szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq("set", SerialNext());
	iq << XATTR("from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof(szBareJid)));
	TiXmlElement *publish_node = iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub")
		<< XCHILD("publish") << XATTR("node", JABBER_FEAT_OMEMO ".devicelist");
	TiXmlElement *list_node = publish_node << XCHILD("item") << XATTR("id", "current")
		<< XCHILDNS("list", JABBER_FEAT_OMEMO);
	if (include_cache) { //use cache only if it is fresh
		CMStringA szSetting;
		for (int i = 0; ; ++i) {
			szSetting.Format("OmemoDeviceId%d", i);
			uint32_t val = getDword(szSetting);
			if (val == 0)
				break;

			list_node << XCHILD("device") << XATTRI64("id", val);
		}
	}
	if (include_own)
		list_node << XCHILD("device") << XATTRI64("id", m_omemo.GetOwnDeviceId());

	// send device list back
	// TODO handle response
	m_ThreadInfo->send(iq);

	//make it available without subscription
	ConfigurePepNode(JABBER_FEAT_OMEMO ".devicelist", "open");
}

void CJabberProto::OmemoSendBundle()
{
	// get own device id
	uint32_t own_id = m_omemo.GetOwnDeviceId();

	// construct bundle node
	char szBareJid[JABBER_MAX_JID_LEN];
	XmlNodeIq iq("set", SerialNext());
	iq << XATTR("from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof(szBareJid)));

	TiXmlElement *publish_node = iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub")
		<< XCHILD("publish") << XATTR("node", CMStringA(FORMAT, "%s.bundles:%u", JABBER_FEAT_OMEMO, own_id));

	TiXmlElement *bundle_node = publish_node << XCHILD("item") << XATTR("id", "current") << XCHILDNS("bundle", JABBER_FEAT_OMEMO);

	session_signed_pre_key *sspk;
	signal_protocol_signed_pre_key_load_key(m_omemo.store_context, &sspk, 1);

	// add signed pre key public
	CMStringA spkey_pub(SignalBuffer(ec_key_pair_get_public(session_signed_pre_key_get_key_pair(sspk))).toBase64());
	bundle_node << XCHILD("signedPreKeyPublic", spkey_pub) << XATTRI("signedPreKeyId", 1);

	// add pre key signature
	ptrA signature(mir_base64_encode(session_signed_pre_key_get_signature(sspk), session_signed_pre_key_get_signature_len(sspk)));
	bundle_node << XCHILD("signedPreKeySignature", signature);
	SIGNAL_UNREF(sspk);

	// add an identity key
	// it is must be a public key right ?, standart is a bit confusing...
	bundle_node << XCHILD("identityKey", ptrA(getUStringA("OmemoDevicePublicKey")));

	// add prekeys
	TiXmlElement *prekeys_node = XmlAddChild(bundle_node, "prekeys");

	CMStringA szSetting;
	for (int i = 1;; i++) {
		session_pre_key *pre_key;
		signal_protocol_pre_key_load_key(m_omemo.store_context, &pre_key, i);
		if (!pre_key)
			break;

		CMStringA val(SignalBuffer(ec_key_pair_get_public(session_pre_key_get_key_pair(pre_key))).toBase64());
		SIGNAL_UNREF(pre_key);

		prekeys_node << XCHILD("preKeyPublic", val) << XATTRI("preKeyId", i);
	}

	// send bundle
	//TODOL handle response
	m_ThreadInfo->send(iq);

	ConfigurePepNode(CMStringA(FORMAT, "%s.bundles:%u", JABBER_FEAT_OMEMO, own_id), "open");
}

bool CJabberProto::OmemoCheckSession(MCONTACT hContact, bool requestBundles)
{
	{	ptrA jid(ContactToJID(hContact));
		if (strchr(jid, '/')) {
			int ret = MessageBoxW(nullptr, TranslateT("OMEMO can not encrypt private messages in public groupchats. Continue using plain text?"), _A2T(jid), MB_YESNO);
			if (ret == IDYES)
				setByte(hContact, "bDisableOmemo", 1);

			return true;
		}
	}

	bool ok = true;
	bool enCarbons = hContact && m_bEnableCarbons && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_CARBONS);
	for (int c = 0; c < (enCarbons ? 2 : 1); c++) {
		MCONTACT _hContact = !c ? hContact : 0;
		ptrA jid(ContactToJID(_hContact));

		for (int i = 0;; i++) {
			int device_id = m_omemo.dbGetDeviceId(_hContact, i);
			if (device_id == 0)
				break;

			signal_protocol_address address = { jid, mir_strlen(jid), device_id };
			if (!signal_protocol_session_contains_session(m_omemo.store_context, &address)) {

				ok = false;

				if (!requestBundles)
					break;

				XmlNodeIq iq(AddIQ(&CJabberProto::OmemoOnIqResultGetBundle, JABBER_IQ_TYPE_GET, nullptr, (void *)hContact));

				char szBareJid[JABBER_MAX_JID_LEN];
				iq << XATTR("from", JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof(szBareJid))) << XATTR("to", jid);
				TiXmlElement *items = iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub") << XCHILD("items");
				CMStringA szBundle(FORMAT, "%s%s%u", JABBER_FEAT_OMEMO, ".bundles:", device_id);
				XmlAddAttr(items, "node", szBundle);
				m_ThreadInfo->send(iq);
			}
		}
	}

	if (ok) {
		uint32_t count = 0;
		db_enum_settings(hContact, omemo::db_enum_settings_fps_cb, m_szModuleName, &count);
		if (count == 0)
			m_omemo.TOFUAllDevices(hContact);

		count = 0;
		db_enum_settings(0, omemo::db_enum_settings_fps_cb, m_szModuleName, &count);
		if (enCarbons && count == 0) {
			m_omemo.TOFUAllDevices(0);
			setByte(0, "OmemoFingerprintTrusted_" "05600dc0ffee", FP_TOFU);
		}

		if (!requestBundles)
			OmemoHandleMessageQueue();
	}

	return ok;
}

void CJabberProto::OmemoOnIqResultGetBundle(const TiXmlElement *iqNode, CJabberIqInfo *IqInfo)
{
	if (iqNode == nullptr || !m_bUseOMEMO)
		return;

	const char *jid = XmlGetAttr(iqNode, "from");
	MCONTACT hContact;
	if (jid) {
		hContact = HContactFromJID(jid);
		if (!hContact && !IsMyOwnJID(jid))
			return;
	}
	else {
		hContact = 0;
		jid = m_szJabberJID;
	}

	if (!OmemoIsEnabled(hContact))
		return;

	const char *type = XmlGetAttr(iqNode, "type");
	if (mir_strcmp(type, "result")) {
		// failed to get a bundle, do not try to build session
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

	int PKs = XmlGetChildCount(prekeys);
	if (PKs < 1)
		return;
	uint8_t nPK;
	Utils_GetRandom(&nPK, 1);
	nPK %= PKs;

	auto prekey_node = TiXmlEnum(prekeys).begin();
	for (int i = 0; i < nPK; i++)
		++prekey_node;

	const char *preKeyPublic = (*prekey_node)->GetText();
	if (!preKeyPublic) {
		debugLogA("Jabber OMEMO: error: failed to get preKeyPublic data");
		return;
	}

	const char *preKeyId = XmlGetAttr(*prekey_node, "preKeyId");
	if (!preKeyId) {
		debugLogA("Jabber OMEMO: error: failed to get preKeyId data");
		return;
	}

	if (!m_omemo.build_session(jid, device_id, preKeyId, preKeyPublic, signedPreKeyId, signedPreKeyPublic->GetText(), signedPreKeySignature->GetText(), identityKey->GetText())) {
		debugLogA("Jabber OMEMO: error: omemo::build_session failed");
		return; //failed to build signal(omemo) session
	}

	OmemoCheckSession((MCONTACT)IqInfo->GetUserData(), false);
}

int CJabberProto::OmemoEncryptMessage(XmlNode &msg, const char *msg_text, MCONTACT hContact)
{
	// a message for clients that do not support OMEMO
	msg << XCHILD("body", "I sent you an OMEMO encrypted message but your client doesn\'t seem to support that");

	const EVP_CIPHER *cipher = EVP_aes_128_gcm();
	unsigned char key[16], iv[12], tag[16] /*, aad[48]*/;
	Utils_GetRandom(key, _countof(key));
	Utils_GetRandom(iv, _countof(iv));

	//Utils_GetRandom(aad, _countof(aad));
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, _countof(iv), nullptr);
	EVP_EncryptInit(ctx, cipher, key, iv);

	const size_t inl = strlen(msg_text);
	int tmp_len = 0, outl;
	//EVP_EncryptUpdate(ctx, nullptr, &outl, aad, _countof(aad));
	unsigned char *out = (unsigned char*)mir_alloc(inl + _countof(key) - 1);
	EVP_EncryptUpdate(ctx, out, &outl, (unsigned char*)msg_text, (int)inl);
	tmp_len += outl;

	EVP_EncryptFinal(ctx, (unsigned char*)(out + tmp_len), &outl);
	tmp_len += outl;
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, _countof(tag), tag);
	EVP_CIPHER_CTX_free(ctx);

	TiXmlElement *encrypted = msg << XCHILDNS("encrypted", JABBER_FEAT_OMEMO);
	TiXmlElement *payload = encrypted << XCHILD("payload");
	payload->SetText(ptrA(mir_base64_encode(out, tmp_len)).get());
	mir_free(out);

	TiXmlElement *header = encrypted << XCHILD("header");
	header << XATTRI64("sid", m_omemo.GetOwnDeviceId());

	int session_count = 0;
	char key_plus_tag[32];
	memcpy(key_plus_tag, key, 16);
	memcpy(key_plus_tag + 16, tag, 16);

	bool enCarbons = m_bEnableCarbons && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_CARBONS);
	for (int c = 0; c < (enCarbons ? 2 : 1); c++) {
		MCONTACT _hContact = !c ? hContact : 0;
		ptrA jid(getStringA(_hContact, "jid"));
		for (int i = 0;; i++) {
			int device_id = m_omemo.dbGetDeviceId(_hContact, i);
			if (device_id == 0)
				break;

			// ignore untrusted identity
			CMStringA suffix(m_omemo.dbGetSuffix(_hContact, device_id));
			MBinBuffer fp(getBlob(_hContact, CMStringA(omemo::IdentityPrefix) + suffix));
			if (!getByte(_hContact, "OmemoFingerprintTrusted_" + omemo::hex_string(fp.data(), fp.length())))
				continue;

			session_cipher *scipher;
			signal_protocol_address address = { jid, mir_strlen(jid), device_id };
			if (session_cipher_create(&scipher, m_omemo.store_context, &address, m_omemo.global_context) != SG_SUCCESS) {
				debugLogA("Jabber OMEMO: error: Cannot create session cipher for encrypt");
				continue;
			}

			ciphertext_message *encrypted_key;
			if (session_cipher_encrypt(scipher, (uint8_t*)key_plus_tag, 32, &encrypted_key) != SG_SUCCESS) {
				// TODO: handle encryption error
				session_cipher_free(scipher);
				debugLogA("Jabber OMEMO: Session_cipher_encrypt failed. Untrusted device?");
				continue;
			}

			TiXmlElement *key_node = header << XCHILD("key");
			key_node << XATTRI64("rid", device_id);
			int msg_type = ciphertext_message_get_type(encrypted_key);
			if (msg_type == CIPHERTEXT_PREKEY_TYPE)
				key_node << XATTR("prekey", "true");

			signal_buffer *serialized_encrypted_key = ciphertext_message_get_serialized(encrypted_key);
			ptrA key_base64(mir_base64_encode(signal_buffer_data(serialized_encrypted_key), signal_buffer_len(serialized_encrypted_key)));
			key_node->SetText(key_base64.get());

			SIGNAL_UNREF(encrypted_key);
			session_cipher_free(scipher);
			session_count++;
		}
	}

	TiXmlElement *iv_node = header << XCHILD("iv");
	iv_node->SetText(ptrA(mir_base64_encode(iv, _countof(iv))).get());

	msg << XCHILDNS("store", "urn:xmpp:hints");
	if (!session_count)
		debugLogA("Jabber OMEMO: error: message does not encrypted for any sessions");

	return session_count;
}

bool CJabberProto::OmemoIsEnabled(MCONTACT hContact)
{
	if (!m_bUseOMEMO) // no OMEMO at all
		return false;

	return !getByte(hContact, "bDisableOmemo") && !isChatRoom(hContact);
}

void CJabberProto::OmemoRequestDeviceList(const char *szBareJid)
{
	if (!m_bJabberOnline)
		return;

	if (szBareJid && strchr(szBareJid, '/'))
		return;

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultGetOmemodevicelist, JABBER_IQ_TYPE_GET));
	if (szBareJid)
		iq << XATTR("to", szBareJid);
	iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub")
		<< XCHILD("items") << XATTR("node", JABBER_FEAT_OMEMO ".devicelist");
	m_ThreadInfo->send(iq);
}

void CJabberProto::OmemoDeleteBundle(int device_id)
{
	if (!m_bJabberOnline)
		return;

	XmlNodeIq iq("set", SerialNext());
	iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub#owner")
		<< XCHILD("delete") << XATTR("node", CMStringA(FORMAT, "%s.bundles:%u", JABBER_FEAT_OMEMO, device_id));
	m_ThreadInfo->send(iq);
}
