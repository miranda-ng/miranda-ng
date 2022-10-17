/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// MSignalStore members

static int CompareSessions(const MSignalSession *p1, const MSignalSession *p2)
{
	if (int ret = mir_strcmp(p1->szName, p2->szName))
		return ret;
	
	return p1->getDeviceId() - p2->getDeviceId();
}

MSignalStore::MSignalStore(PROTO_INTERFACE *_1, const char *_2) :
	pProto(_1),
	prefix(_2),
	arSessions(1, &CompareSessions)
{
	init();
}

MSignalStore::~MSignalStore()
{
	signal_protocol_store_context_destroy(m_pStore);
	signal_context_destroy(m_pContext);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void log_func(int level, const char *pmsg, size_t /*msgLen*/, void *pUserData)
{
	auto *pStore = (MSignalStore *)pUserData;
	pStore->pProto->debugLogA("libsignal {%d}: %s", level, pmsg);
}

static int hmac_sha256_init(void **hmac_context, const uint8_t *key, size_t key_len, void *)
{
	HMAC_CTX *ctx = HMAC_CTX_new();
	*hmac_context = ctx;
	HMAC_Init(ctx, key, (int)key_len, EVP_sha256());
	return 0;
}

static int hmac_sha256_update(void *hmac_context, const uint8_t *data, size_t data_len, void *)
{
	return HMAC_Update((HMAC_CTX *)hmac_context, data, data_len);
}

static int hmac_sha256_final(void *hmac_context, signal_buffer **output, void *)
{
	BYTE data[200];
	unsigned len = 0;
	if (!HMAC_Final((HMAC_CTX *)hmac_context, data, &len))
		return 1;

	*output = signal_buffer_create(data, len);
	return 0;
}

static void hmac_sha256_cleanup(void *hmac_context, void *)
{
	HMAC_CTX_free((HMAC_CTX *)hmac_context);
}

static int random_func(uint8_t *pData, size_t size, void *)
{
	Utils_GetRandom(pData, size);
	return 0;
}

static int decrypt_func(signal_buffer **output,
	int /*cipher*/,
	const uint8_t *key, size_t /*key_len*/,
	const uint8_t *iv, size_t /*iv_len*/,
	const uint8_t *ciphertext, size_t ciphertext_len,
	void * /*user_data*/)
{
	MBinBuffer res = aesDecrypt(EVP_aes_256_gcm(), key, iv, ciphertext, ciphertext_len);
	*output = signal_buffer_create(res.data(), res.length());
	return SG_SUCCESS;
}

static int contains_session_func(const signal_protocol_address *address, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);
	return pStore->arSessions.find(&tmp) == nullptr;
}

static int delete_all_sessions_func(const char *name, size_t name_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	int count = 0;
	auto &pList = pStore->arSessions;
	CMStringA szName(name, (int)name_len);
	for (auto &it : pList.rev_iter())
		if (it->szName == szName) {
			pStore->pProto->delSetting(it->getSetting(pStore));
			pList.remove(pList.indexOf(&it));
			count++;
		}

	return count;
}

int delete_session_func(const signal_protocol_address *address, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;
	auto &pList = pStore->arSessions;

	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);
	int idx = pList.getIndex(&tmp);
	if (idx == -1)
		return 0;

	pStore->pProto->delSetting(tmp.getSetting(pStore));
	pList.remove(idx);
	return 0;
}

static int get_sub_device_sessions_func(signal_int_list **sessions, const char *name, size_t name_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;
	CMStringA szName(name, (int)name_len);

	signal_int_list *l = signal_int_list_alloc();
	unsigned int array_size = 0;

	for (auto &it : pStore->arSessions)
		if (it->szName == szName) {
			array_size++;
			signal_int_list_push_back(l, it->getDeviceId());
		}

	*sessions = l;
	return array_size;
}

static void destroy_func(void *)
{}

int load_session_func(signal_buffer **record, signal_buffer ** /*user_data_storage*/, const signal_protocol_address *address, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);

	MBinBuffer blob(pStore->pProto->getBlob(tmp.getSetting(pStore)));
	if (blob.data() == 0)
		return 0;

	*record = signal_buffer_create((uint8_t *)blob.data(), blob.length());
	return 1;
}

static int store_session_func(const signal_protocol_address *address, uint8_t *record, size_t record_len, uint8_t * /*user_record*/, size_t /*user_record_len*/, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);
	db_set_blob(0, pStore->pProto->m_szModuleName, tmp.getSetting(pStore), record, (unsigned int)record_len); //TODO: check return value
	return 0;
}

static int contains_pre_key(uint32_t pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "PreKey", pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	return (blob.data() != 0);
}

static int load_pre_key(signal_buffer **record, uint32_t pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "PreKey", pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	if (blob.data() == 0)
		return SG_ERR_INVALID_KEY_ID;

	*record = signal_buffer_create((uint8_t *)blob.data(), blob.length());
	return SG_SUCCESS; //key exists and succesfully loaded
}

static int remove_pre_key(uint32_t pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "PreKey", pre_key_id);
	pStore->pProto->delSetting(szSetting);

	szSetting.Format("PreKey%uPublic", pre_key_id);
	pStore->pProto->delSetting(szSetting);

	szSetting.Format("PreKey%uPrivate", pre_key_id);
	pStore->pProto->delSetting(szSetting);
	return 0;
}

static int store_pre_key(uint32_t pre_key_id, uint8_t *record, size_t record_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "PreKey", pre_key_id);
	db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, record, (unsigned int)record_len);

	session_pre_key *prekey = nullptr;
	session_pre_key_deserialize(&prekey, record, record_len, pStore->CTX()); //TODO: handle error
	if (prekey) {
		ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(prekey);

		SignalBuffer key_buf(ec_key_pair_get_public(pre_key_pair));
		szSetting.Format("PreKey%uPublic", pre_key_id);
		db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, key_buf.data(), key_buf.len());
	}

	return 0;
}

static int contains_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "SignedPreKey", signed_pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	return blob.data() != 0;
}

static int load_signed_pre_key(signal_buffer **record, uint32_t signed_pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "SignedPreKey", signed_pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	if (blob.data() == 0)
		return SG_ERR_INVALID_KEY_ID;

	*record = signal_buffer_create(blob.data(), blob.length());
	return SG_SUCCESS; //key exist and succesfully loaded
}

static int store_signed_pre_key(uint32_t signed_pre_key_id, uint8_t *record, size_t record_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "SignedPreKey", signed_pre_key_id);
	db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, record, (unsigned int)record_len);
	return 0;
}

static int remove_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s%d", "SignedPreKey", signed_pre_key_id);
	pStore->pProto->delSetting(szSetting);
	return 0;
}

static int get_identity_key_pair(signal_buffer **public_data, signal_buffer **private_data, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	MBinBuffer buf;
	buf.append(KEY_BUNDLE_TYPE, 1);
	buf.append(pStore->signedIdentity.pub);
	*public_data = signal_buffer_create(buf.data(), (int)buf.length());

	*private_data = signal_buffer_create((uint8_t *)pStore->signedIdentity.priv.data(), (int)pStore->signedIdentity.priv.length());
	return 0;
}

static int get_local_registration_id(void *user_data, uint32_t *registration_id)
{
	auto *pStore = (MSignalStore *)user_data;
	*registration_id = pStore->pProto->getDword(DBKEY_REG_ID);
	return 0;
}

static int save_identity(const signal_protocol_address *address, uint8_t *key_data, size_t key_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%s_%d", "SignalIdentity", CMStringA(address->name, (int)address->name_len).c_str(), address->device_id);
	if (key_data != nullptr)
		db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, key_data, (unsigned int)key_len); //TODO: check return value
	else
		pStore->pProto->delSetting(szSetting);
	return 0;
}

static int is_trusted_identity(const signal_protocol_address * /*address*/, uint8_t * /*key_data*/, size_t /*key_len*/, void * /*user_data*/)
{
	return 1;
}

void MSignalStore::init()
{
	signal_context_create(&m_pContext, this);
	signal_context_set_log_function(m_pContext, log_func);

	signal_crypto_provider prov;
	memset(&prov, 0xFF, sizeof(prov));
	prov.hmac_sha256_init_func = hmac_sha256_init;
	prov.hmac_sha256_final_func = hmac_sha256_final;
	prov.hmac_sha256_update_func = hmac_sha256_update;
	prov.hmac_sha256_cleanup_func = hmac_sha256_cleanup;
	prov.random_func = random_func;
	prov.decrypt_func = decrypt_func;
	signal_context_set_crypto_provider(m_pContext, &prov);

	// default values calculation
	if (pProto->getDword(DBKEY_PREKEY_NEXT_ID, 0xFFFF) == 0xFFFF) {
		// generate signed identity keys (private & public)
		ratchet_identity_key_pair *keyPair;
		signal_protocol_key_helper_generate_identity_key_pair(&keyPair, m_pContext);

		auto *pPubKey = ratchet_identity_key_pair_get_public(keyPair);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_SIGNED_IDENTITY_PUB, pPubKey->data, sizeof(pPubKey->data));

		auto *pPrivKey = ratchet_identity_key_pair_get_private(keyPair);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_SIGNED_IDENTITY_PRIV, pPrivKey->data, sizeof(pPrivKey->data));

		session_signed_pre_key *signed_pre_key;
		signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, keyPair, 1, time(0), m_pContext);

		SignalBuffer prekeyBuf(signed_pre_key);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_PREKEY, prekeyBuf.data(), prekeyBuf.len());
		SIGNAL_UNREF(signed_pre_key);
		SIGNAL_UNREF(keyPair);
	}

	// read resident data from database
	signedIdentity.pub = pProto->getBlob(DBKEY_SIGNED_IDENTITY_PUB);
	signedIdentity.priv = pProto->getBlob(DBKEY_SIGNED_IDENTITY_PRIV);

	MBinBuffer blob(pProto->getBlob(DBKEY_PREKEY));
	session_signed_pre_key *signed_pre_key;
	session_signed_pre_key_deserialize(&signed_pre_key, blob.data(), blob.length(), m_pContext);

	ec_key_pair *pKeys = session_signed_pre_key_get_key_pair(signed_pre_key);
	auto *pPubKey = ec_key_pair_get_public(pKeys);
	preKey.pub.assign(pPubKey->data, sizeof(pPubKey->data));

	auto *pPrivKey = ec_key_pair_get_private(pKeys);
	preKey.priv.assign(pPrivKey->data, sizeof(pPrivKey->data));

	preKey.signature.assign(session_signed_pre_key_get_signature(signed_pre_key), session_signed_pre_key_get_signature_len(signed_pre_key));
	preKey.keyid = session_signed_pre_key_get_id(signed_pre_key);
	SIGNAL_UNREF(signed_pre_key);

	// create store with callbacks
	signal_protocol_store_context_create(&m_pStore, m_pContext);

	signal_protocol_session_store ss;
	ss.contains_session_func = &contains_session_func;
	ss.delete_all_sessions_func = &delete_all_sessions_func;
	ss.delete_session_func = &delete_session_func;
	ss.destroy_func = &destroy_func;
	ss.get_sub_device_sessions_func = &get_sub_device_sessions_func;
	ss.load_session_func = &load_session_func;
	ss.store_session_func = &store_session_func;
	ss.user_data = this;
	signal_protocol_store_context_set_session_store(m_pStore, &ss);

	signal_protocol_pre_key_store sp;
	sp.contains_pre_key = &contains_pre_key;
	sp.destroy_func = &destroy_func;
	sp.load_pre_key = &load_pre_key;
	sp.remove_pre_key = &remove_pre_key;
	sp.store_pre_key = &store_pre_key;
	sp.user_data = this;
	signal_protocol_store_context_set_pre_key_store(m_pStore, &sp);

	signal_protocol_signed_pre_key_store ssp;
	ssp.contains_signed_pre_key = &contains_signed_pre_key;
	ssp.destroy_func = &destroy_func;
	ssp.load_signed_pre_key = &load_signed_pre_key;
	ssp.remove_signed_pre_key = &remove_signed_pre_key;
	ssp.store_signed_pre_key = &store_signed_pre_key;
	ssp.user_data = this;
	signal_protocol_store_context_set_signed_pre_key_store(m_pStore, &ssp);

	signal_protocol_identity_key_store sip;
	sip.destroy_func = &destroy_func;
	sip.get_identity_key_pair = &get_identity_key_pair;
	sip.get_local_registration_id = &get_local_registration_id;
	sip.is_trusted_identity = &is_trusted_identity;
	sip.save_identity = &save_identity;
	sip.user_data = this;
	signal_protocol_store_context_set_identity_key_store(m_pStore, &sip);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSignalSession members

MSignalSession::MSignalSession(const CMStringA &_1, int _2) :
	szName(_1)
{
	address.name = szName.GetBuffer();
	address.name_len = szName.GetLength();
	address.device_id = _2;
}

MSignalSession::~MSignalSession()
{
	session_cipher_free(cipher);
}

CMStringA MSignalSession::getSetting(const MSignalStore *pStore) const
{
	return CMStringA(FORMAT, "%s%s_%s_%d", 
		(pStore) ? pStore->prefix : "", "SignalSession", szName.c_str(), getDeviceId());
}

/////////////////////////////////////////////////////////////////////////////////////////

MSignalSession* MSignalStore::createSession(const CMStringA &szName, int deviceId)
{
	MSignalSession tmp(szName, deviceId);
	auto *pSession = arSessions.find(&tmp);
	if (pSession == nullptr) {
		pSession = new MSignalSession(szName, deviceId);
		arSessions.insert(pSession);
	}

	if (pSession->cipher == nullptr)
		if (session_cipher_create(&pSession->cipher, m_pStore, &pSession->address, m_pContext) < 0)
			throw "session_cipher_create failure";

	return pSession;
}

/////////////////////////////////////////////////////////////////////////////////////////

signal_buffer* MSignalStore::decryptSignalProto(const CMStringA &from, const char *pszType, const MBinBuffer &encrypted)
{
	WAJid jid(from);
	auto *pSession = createSession(jid.user, 0);

	signal_buffer *result = nullptr;
	if (!mir_strcmp(pszType, "pkmsg")) {
		pre_key_signal_message *pMsg; 
		if (pre_key_signal_message_deserialize(&pMsg, (BYTE *)encrypted.data(), encrypted.length(), m_pContext) < 0)
			throw "unable to deserialize prekey message";

		if (session_cipher_decrypt_pre_key_signal_message(pSession->getCipher(), pMsg, this, &result) < 0)
			throw "unable to decrypt prekey message";

		pre_key_signal_message_destroy((signal_type_base*)pMsg);
	}
	else {
		signal_message *pMsg;
		if (signal_message_deserialize(&pMsg, (BYTE *)encrypted.data(), encrypted.length(), m_pContext) < 0)
			throw "unable to deserialize signal message";

		if (session_cipher_decrypt_signal_message(pSession->getCipher(), pMsg, this, &result) < 0)
			throw "unable to decrypt signal message";

		signal_message_destroy((signal_type_base *)pMsg);
	}

	return result;
}

signal_buffer* MSignalStore::decryptGroupSignalProto(const CMStringA &group, const CMStringA &sender, const MBinBuffer &encrypted)
{
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// generate and save pre keys set

void MSignalStore::generatePrekeys(int count)
{
	int iNextKeyId = pProto->getDword(DBKEY_PREKEY_NEXT_ID, 1);

	CMStringA szSetting;
	signal_protocol_key_helper_pre_key_list_node *keys_root;
	signal_protocol_key_helper_generate_pre_keys(&keys_root, iNextKeyId, count, m_pContext);
	for (auto *it = keys_root; it; it = signal_protocol_key_helper_key_list_next(it)) {
		session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
		uint32_t pre_key_id = session_pre_key_get_id(pre_key);

		SignalBuffer buf(pre_key);
		szSetting.Format("PreKey%d", pre_key_id);
		db_set_blob(0, pProto->m_szModuleName, szSetting, buf.data(), buf.len());

		ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(pre_key);
		auto *pPubKey = ec_key_pair_get_public(pre_key_pair);
		szSetting.Format("PreKey%dPublic", pre_key_id);
		db_set_blob(0, pProto->m_szModuleName, szSetting, pPubKey->data, sizeof(pPubKey->data));
	}
	signal_protocol_key_helper_key_list_free(keys_root);

	pProto->setDword(DBKEY_PREKEY_NEXT_ID, iNextKeyId + count);
}

/////////////////////////////////////////////////////////////////////////////////////////

void MSignalStore::processSenderKeyMessage(const proto::Message_SenderKeyDistributionMessage &msg)
{
}
