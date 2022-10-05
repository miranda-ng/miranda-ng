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
	if (pProto->getDword(DBKEY_PREKEY_KEYID, 0xFFFF) == 0xFFFF) {
		// generate pre keys
		const unsigned int signed_pre_key_id = 1;
		pProto->setDword(DBKEY_PREKEY_KEYID, 1);
		pProto->setDword(DBKEY_PREKEY_NEXT_ID, 1);
		pProto->setDword(DBKEY_PREKEY_UPLOAD_ID, 1);

		// generate signed identity keys (private & public)
		ratchet_identity_key_pair *keyPair;
		signal_protocol_key_helper_generate_identity_key_pair(&keyPair, g_plugin.pCtx);

		auto *pPubKey = ratchet_identity_key_pair_get_public(keyPair);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_SIGNED_IDENTITY_PUB, pPubKey->data, sizeof(pPubKey->data));

		auto *pPrivKey = ratchet_identity_key_pair_get_private(keyPair);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_SIGNED_IDENTITY_PRIV, pPrivKey->data, sizeof(pPrivKey->data));

		session_signed_pre_key *signed_pre_key;
		signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, keyPair, signed_pre_key_id, time(0), g_plugin.pCtx);
		SIGNAL_UNREF(keyPair);

		signal_buffer *serialized_signed_pre_key;
		session_signed_pre_key_serialize(&serialized_signed_pre_key, signed_pre_key);

		ec_key_pair *pKeys = session_signed_pre_key_get_key_pair(signed_pre_key);
		pPubKey = ec_key_pair_get_public(pKeys);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_PREKEY_PUB, pPubKey->data, sizeof(pPubKey->data));

		pPrivKey = ec_key_pair_get_private(pKeys);
		db_set_blob(0, pProto->m_szModuleName, DBKEY_PREKEY_PRIV, pPrivKey->data, sizeof(pPrivKey->data));

		db_set_blob(0, pProto->m_szModuleName, DBKEY_PREKEY_SIGN, (void *)session_signed_pre_key_get_signature(signed_pre_key), (int)session_signed_pre_key_get_signature_len(signed_pre_key));

		// generate and save pre keys set
		CMStringA szSetting;
		signal_protocol_key_helper_pre_key_list_node *keys_root;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, 1, 20, g_plugin.pCtx);
		for (auto *it = keys_root; it; it = signal_protocol_key_helper_key_list_next(it)) {
			session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
			uint32_t pre_key_id = session_pre_key_get_id(pre_key);
			{
				signal_buffer *serialized_pre_key;
				session_pre_key_serialize(&serialized_pre_key, pre_key);
				szSetting.Format("PreKey%d", pre_key_id);
				db_set_blob(0, pProto->m_szModuleName, szSetting, signal_buffer_data(serialized_pre_key), (unsigned int)signal_buffer_len(serialized_pre_key));
				SIGNAL_UNREF(serialized_pre_key);
			}

			ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(pre_key);
			pPubKey = ec_key_pair_get_public(pre_key_pair);
			szSetting.Format("PreKey%dPublic", pre_key_id);
			db_set_blob(0, pProto->m_szModuleName, szSetting, pPubKey->data, sizeof(pPubKey->data));
		}
		signal_protocol_key_helper_key_list_free(keys_root);
	}

	// read resident data from database
	signedIdentity.pub = pProto->getBlob(DBKEY_SIGNED_IDENTITY_PUB);
	signedIdentity.priv = pProto->getBlob(DBKEY_SIGNED_IDENTITY_PRIV);

	preKey.pub = pProto->getBlob(DBKEY_PREKEY_PUB);
	preKey.priv = pProto->getBlob(DBKEY_PREKEY_PRIV);
	preKey.keyid = pProto->getDword(DBKEY_PREKEY_KEYID);
	preKey.signature = pProto->getBlob(DBKEY_PREKEY_SIGN);

	// context cretion
	init();
}

MSignalStore::~MSignalStore()
{
	signal_protocol_store_context_destroy(m_pContext);
}

/////////////////////////////////////////////////////////////////////////////////////////

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

	CMStringA szSetting(FORMAT, "%s_%d", "PreKey", pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	return (blob.data() != 0);
}

static int load_pre_key(signal_buffer **record, uint32_t pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%d", "PreKey", pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	if (blob.data() == 0)
		return SG_ERR_INVALID_KEY_ID;

	*record = signal_buffer_create((uint8_t *)blob.data(), blob.length());
	return SG_SUCCESS; //key exists and succesfully loaded
}

static int remove_pre_key(uint32_t pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%d", "PreKey", pre_key_id);
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

	CMStringA szSetting(FORMAT, "%s_%d", "PreKey", pre_key_id);
	db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, record, (unsigned int)record_len);

	session_pre_key *prekey = nullptr;
	session_pre_key_deserialize(&prekey, record, record_len, g_plugin.pCtx); //TODO: handle error
	if (prekey) {
		ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(prekey);
		signal_buffer *key_buf = nullptr;
		ec_public_key *public_key = ec_key_pair_get_public(pre_key_pair);
		ec_public_key_serialize(&key_buf, public_key);
		SIGNAL_UNREF(public_key);

		szSetting.Format("PreKey%uPublic", pre_key_id);
		db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, signal_buffer_data(key_buf), (int)signal_buffer_len(key_buf));
		signal_buffer_free(key_buf);
	}

	return 0;
}

static int contains_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%d", "SignalSignedPreKey", signed_pre_key_id);
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	if (db_get(0, pStore->pProto->m_szModuleName, szSetting, &dbv))
		return 0;

	db_free(&dbv);
	return 1;
}

static int load_signed_pre_key(signal_buffer **record, uint32_t signed_pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%d", "SignalSignedPreKey", signed_pre_key_id);
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	if (db_get(0, pStore->pProto->m_szModuleName, szSetting, &dbv))
		return SG_ERR_INVALID_KEY_ID;

	*record = signal_buffer_create(dbv.pbVal, dbv.cpbVal);
	db_free(&dbv);
	return SG_SUCCESS; //key exist and succesfully loaded

}

static int store_signed_pre_key(uint32_t signed_pre_key_id, uint8_t *record, size_t record_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%d", "SignalSignedPreKey", signed_pre_key_id);
	db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, record, (unsigned int)record_len);
	return 0;
}

static int remove_signed_pre_key(uint32_t signed_pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%d", "SignalSignedPreKey", signed_pre_key_id);
	pStore->pProto->delSetting(szSetting);
	return 0;
}

int get_identity_key_pair(signal_buffer **public_data, signal_buffer **private_data, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	*public_data = signal_buffer_create((uint8_t *)pStore->preKey.pub.data(), (int)pStore->preKey.pub.length());
	*private_data = signal_buffer_create((uint8_t *)pStore->preKey.priv.data(), (int)pStore->preKey.priv.length());
	return 0;
}

int get_local_registration_id(void *user_data, uint32_t *registration_id)
{
	auto *pStore = (MSignalStore *)user_data;
	*registration_id = pStore->pProto->getDword(DBKEY_REG_ID);
	return 0;
}

int save_identity(const signal_protocol_address *address, uint8_t *key_data, size_t key_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(FORMAT, "%s_%s_%d", "SignalIdentity", CMStringA(address->name, (int)address->name_len).c_str(), address->device_id);

	if (key_data != nullptr)
		db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, key_data, (unsigned int)key_len); //TODO: check return value
	else
		pStore->pProto->delSetting(szSetting);
	return 0;
}

int is_trusted_identity(const signal_protocol_address * /*address*/, uint8_t * /*key_data*/, size_t /*key_len*/, void * /*user_data*/)
{
	return 1;
}

void MSignalStore::init()
{
	signal_protocol_store_context_create(&m_pContext, g_plugin.pCtx);

	signal_protocol_session_store ss;
	ss.contains_session_func = &contains_session_func;
	ss.delete_all_sessions_func = &delete_all_sessions_func;
	ss.delete_session_func = &delete_session_func;
	ss.destroy_func = &destroy_func;
	ss.get_sub_device_sessions_func = &get_sub_device_sessions_func;
	ss.load_session_func = &load_session_func;
	ss.store_session_func = &store_session_func;
	ss.user_data = this;
	signal_protocol_store_context_set_session_store(m_pContext, &ss);

	signal_protocol_pre_key_store sp;
	sp.contains_pre_key = &contains_pre_key;
	sp.destroy_func = &destroy_func;
	sp.load_pre_key = &load_pre_key;
	sp.remove_pre_key = &remove_pre_key;
	sp.store_pre_key = &store_pre_key;
	sp.user_data = this;
	signal_protocol_store_context_set_pre_key_store(m_pContext, &sp);

	signal_protocol_signed_pre_key_store ssp;
	ssp.contains_signed_pre_key = &contains_signed_pre_key;
	ssp.destroy_func = &destroy_func;
	ssp.load_signed_pre_key = &load_signed_pre_key;
	ssp.remove_signed_pre_key = &remove_signed_pre_key;
	ssp.store_signed_pre_key = &store_signed_pre_key;
	ssp.user_data = this;
	signal_protocol_store_context_set_signed_pre_key_store(m_pContext, &ssp);

	signal_protocol_identity_key_store sip;
	sip.destroy_func = &destroy_func;
	sip.get_identity_key_pair = &get_identity_key_pair;
	sip.get_local_registration_id = &get_local_registration_id;
	sip.is_trusted_identity = &is_trusted_identity;
	sip.save_identity = &save_identity;
	sip.user_data = this;
	signal_protocol_store_context_set_identity_key_store(m_pContext, &sip);
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

MSignalSession *MSignalStore::createSession(const CMStringA &szName, int deviceId)
{
	MSignalSession tmp(szName, deviceId);
	auto *pSession = arSessions.find(&tmp);
	if (pSession == nullptr) {
		pSession = new MSignalSession(szName, deviceId);
		arSessions.insert(pSession);
	}

	if (pSession->cipher == nullptr)
		if (session_cipher_create(&pSession->cipher, m_pContext, &pSession->address, g_plugin.pCtx) < 0)
			throw "session_cipher_create failure";

	return pSession;
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer MSignalStore::decryptSignalProto(const CMStringA &from, const char *pszType, const MBinBuffer &encrypted)
{
	WAJid jid(from);
	auto *pSession = createSession(jid.user, 0);

	signal_buffer *result = nullptr;
	if (!mir_strcmp(pszType, "pkmsg")) {
		ec_public_key pBaseKey, pIdentityKey;
		memcpy(&pBaseKey.data, preKey.pub.data(), 32);
		memcpy(&pIdentityKey.data, signedIdentity.pub.data(), 32);

		pre_key_signal_message *pMsg; 
		if (pre_key_signal_message_deserialize(&pMsg, (BYTE *)encrypted.data(), encrypted.length(), g_plugin.pCtx) < 0)
			throw "unable to deserialize prekey message";

		if (session_cipher_decrypt_pre_key_signal_message(pSession->getCipher(), pMsg, 0, &result) < 0)
			throw "unable to decrypt prekey message";

		pre_key_signal_message_destroy((signal_type_base*)pMsg);
	}
	else {
		signal_message *pMsg;
		if (signal_message_deserialize(&pMsg, (BYTE *)encrypted.data(), encrypted.length(), g_plugin.pCtx) < 0)
			throw "unable to deserialize signal message";

		if (session_cipher_decrypt_signal_message(pSession->getCipher(), pMsg, 0, &result) < 0)
			throw "unable to decrypt signal message";

		signal_message_destroy((signal_type_base *)pMsg);
	}

	MBinBuffer ret;
	if (result != nullptr) {
		ret.append(result->data, result->len);
		signal_buffer_free(result);
	}
	return ret;
}

MBinBuffer MSignalStore::decryptGroupSignalProto(const CMStringA &from, const CMStringA &author, const MBinBuffer &encrypted)
{
	MBinBuffer ret;
	return ret;
}

void MSignalStore::processSenderKeyMessage(const proto::Message_SenderKeyDistributionMessage &msg)
{
}
