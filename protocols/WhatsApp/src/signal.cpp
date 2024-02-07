/*

WhatsApp plugin for Miranda NG
Copyright © 2019-24 George Hazan

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

void MSignalStore::logError(int err, const char *pszMessage)
{
	if (err < 0) {
		pProto->debugLogA("libsignal error %d", err);
		throw pszMessage;
	}
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
	MBinBuffer res = aesDecrypt(EVP_aes_256_cbc(), key, iv, ciphertext, ciphertext_len);
	*output = signal_buffer_create(res.data(), res.length());
	return SG_SUCCESS;
}

static int encrypt_func(signal_buffer **output,
	int /*cipher*/,
	const uint8_t *key, size_t /*key_len*/,
	const uint8_t *iv, size_t /*iv_len*/,
	const uint8_t *ciphertext, size_t ciphertext_len,
	void * /*user_data*/)
{
	MBinBuffer res = aesEncrypt(EVP_aes_256_cbc(), key, iv, ciphertext, ciphertext_len);
	*output = signal_buffer_create(res.data(), res.length());
	return SG_SUCCESS;
}

static int contains_session_func(const signal_protocol_address *address, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;
	auto *pSession = pStore->getSession(address);
	return pSession != nullptr;
}

static int delete_all_sessions_func(const char *name, size_t name_len, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;
	auto &pList = pStore->arSessions;

	int count = 0;
	for (auto &it : pList.rev_iter()) {
		if (it->hasAddress(name, name_len)) {
			pStore->pProto->delSetting(it->getSetting());
			pList.removeItem(&it);
			count++;
		}
	}
	return count;
}

int delete_session_func(const signal_protocol_address *address, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;
	auto &pList = pStore->arSessions;

	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);
	int idx = pList.getIndex(&tmp);
	if (idx != -1) {
		pStore->pProto->delSetting(tmp.getSetting());
		pList.remove(idx);
	}
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

int load_session_func(signal_buffer **record, signal_buffer **user_data_storage, const signal_protocol_address *address, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	auto *pSession = pStore->getSession(address);
	if (pSession == nullptr)
		return 0;
		
	*record = signal_buffer_create(pSession->sessionData.data(), pSession->sessionData.length());
	*user_data_storage = 0;
	return 1;
}

static int store_session_func(const signal_protocol_address *address, uint8_t *record, size_t record_len, uint8_t *, size_t, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);
	auto *pSession = pStore->arSessions.find(&tmp);
	if (pSession == nullptr) {
		pSession = new MSignalSession(tmp);
		pStore->arSessions.insert(pSession);
	}

	pSession->sessionData.assign(record, record_len);
	db_set_blob(0, pStore->pProto->m_szModuleName, pSession->getSetting(), record, (unsigned)record_len);
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
	if (blob.isEmpty()) {
		pStore->pProto->debugLogA("Prekey #%d not found", pre_key_id);
		return SG_ERR_INVALID_KEY_ID;
	}

	*record = signal_buffer_create(blob.data(), blob.length());
	return SG_SUCCESS; //key exists and succesfully loaded
}

static int remove_pre_key(uint32_t pre_key_id, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;
	pStore->pProto->debugLogA("Request to remove prekey #%d", pre_key_id);

	/*
	CMStringA szSetting(FORMAT, "%s%d", "PreKey", pre_key_id);
	pStore->pProto->delSetting(szSetting);

	szSetting.Format("PreKey%uPublic", pre_key_id);
	pStore->pProto->delSetting(szSetting);

	szSetting.Format("PreKey%uPrivate", pre_key_id);
	pStore->pProto->delSetting(szSetting);
	*/
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

	if (signed_pre_key_id == 0)
		signed_pre_key_id = 1;

	CMStringA szSetting(FORMAT, "%s%d", "SignedPreKey", signed_pre_key_id);
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	if (blob.isEmpty()) {
		pStore->pProto->debugLogA("Signed prekey #%d not found", signed_pre_key_id);
		return SG_ERR_INVALID_KEY_ID;
	}

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

	*private_data = signal_buffer_create(pStore->signedIdentity.priv.data(), (int)pStore->signedIdentity.priv.length());
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

static CMStringA get_sender_setting(const signal_protocol_sender_key_name *skn)
{
	WAJid jid(CMStringA(skn->sender.name, (int)skn->sender.name_len));
	return CMStringA(FORMAT, "SenderKey_%*s_%s_%d", (unsigned)skn->group_id_len, skn->group_id, jid.user.c_str(), skn->sender.device_id);
}

static int load_sender_key(signal_buffer **record, signal_buffer **, const signal_protocol_sender_key_name *skn, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(get_sender_setting(skn));
	MBinBuffer blob(pStore->pProto->getBlob(szSetting));
	if (blob.isEmpty())
		return 0;

	*record = signal_buffer_create(blob.data(), blob.length());
	return 1;
}

static int store_sender_key(const signal_protocol_sender_key_name *skn, uint8_t *record, size_t record_len, uint8_t*, size_t, void *user_data)
{
	auto *pStore = (MSignalStore *)user_data;

	CMStringA szSetting(get_sender_setting(skn));
	db_set_blob(0, pStore->pProto->m_szModuleName, szSetting, record, (unsigned)record_len);
	return 0;
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
	prov.encrypt_func = encrypt_func;
	signal_context_set_crypto_provider(m_pContext, &prov);

	// read resident data from database
	MBinBuffer blob(pProto->getBlob(DBKEY_PREKEY));
	if (blob.isEmpty()) {
		// nothing? generate signed identity keys (private & public)
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
		blob.assign(prekeyBuf.data(), prekeyBuf.len());

		SIGNAL_UNREF(signed_pre_key);
		SIGNAL_UNREF(keyPair);
	}

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

	signedIdentity.pub = pProto->getBlob(DBKEY_SIGNED_IDENTITY_PUB);
	signedIdentity.priv = pProto->getBlob(DBKEY_SIGNED_IDENTITY_PRIV);

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

	signal_protocol_sender_key_store sk;
	sk.destroy_func = destroy_func;
	sk.load_sender_key = load_sender_key;
	sk.store_sender_key = store_sender_key;
	sk.user_data = this;
	signal_protocol_store_context_set_sender_key_store(m_pStore, &sk);

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

bool MSignalSession::hasAddress(const char *name, size_t name_len) const
{
	if (address.name_len != name_len)
		return false;
	return memcmp(address.name, name, name_len) == 0;
}

CMStringA MSignalSession::getSetting() const
{
	return CMStringA(FORMAT, "SignalSession_%s_%d", szName.c_str(), getDeviceId());
}

/////////////////////////////////////////////////////////////////////////////////////////

MSignalSession* MSignalStore::createSession(const CMStringA &szName, int deviceId)
{
	signal_protocol_address tmp = {szName.c_str(), (unsigned)szName.GetLength(), deviceId};
	auto *pSession = getSession(&tmp);
	if (pSession == nullptr) {
		pSession = new MSignalSession(szName, deviceId);
		arSessions.insert(pSession);
	}

	if (pSession->cipher == nullptr)
		logError(
			session_cipher_create(&pSession->cipher, m_pStore, &pSession->address, m_pContext),
			"session_cipher_create failure");

	return pSession;
}

MSignalSession* MSignalStore::getSession(const signal_protocol_address *address)
{
	MSignalSession tmp(CMStringA(address->name, (int)address->name_len), address->device_id);
	auto *pSession = arSessions.find(&tmp);
	if (pSession == nullptr) {
		MBinBuffer blob(pProto->getBlob(tmp.getSetting()));
		if (blob.isEmpty())
			return nullptr;

		pSession = new MSignalSession(tmp);
		pSession->sessionData.assign(blob.data(), blob.length());
		arSessions.insert(pSession);
	}

	return pSession;
}

void MSignalStore::importPublicKey(ec_public_key **result, MBinBuffer &buf)
{
	buf.appendBefore("\x05", 1);
	curve_decode_point(result, buf.data(), buf.length(), m_pContext);
}

void MSignalStore::injectSession(const char *szJid, const WANode *pNode, const WANode *pKey)
{
	WAJid jid(szJid);
	auto *signedKey = pKey->getChild("skey");
	auto *key = pKey->getChild("key");
	auto *identity = pKey->getChild("identity");
	auto *registration = pNode->getChild("registration");
	if (!signedKey || !key || !identity || !registration) {
		pProto->debugLogA("Bad key data for %s", jid.toString().c_str());
		return;
	}

	signal_protocol_address address = {jid.user.c_str(), (unsigned)jid.user.GetLength(), jid.device};

	session_builder *builder;
	logError(
		session_builder_create(&builder, m_pStore, &address, m_pContext),
		"unable to create session cipher");

	int regId = decodeBigEndian(registration->content);
	int preKeyId = decodeBigEndian(key->getChild("id")->content);
	int signedPreKeyId = decodeBigEndian(signedKey->getChild("id")->content);

	ec_public_key *preKeyPub, *signedPreKeyPub, *identityKey;
	importPublicKey(&preKeyPub, key->getChild("value")->content);
	importPublicKey(&identityKey, identity->content);
	importPublicKey(&signedPreKeyPub, signedKey->getChild("value")->content);

	auto &sign = signedKey->getChild("signature")->content;

	session_pre_key_bundle *bundle;
	logError(
		session_pre_key_bundle_create(&bundle, regId, jid.device, preKeyId, preKeyPub, signedPreKeyId, signedPreKeyPub, sign.data(), sign.length(), identityKey),
		"unable to create pre key bundle");

	logError(
		session_builder_process_pre_key_bundle(builder, bundle),
		"unable to process pre key bundle");

	session_pre_key_bundle_destroy((signal_type_base*)bundle);
	session_builder_free(builder);
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer MSignalStore::decryptSignalProto(const CMStringA &from, const char *pszType, const MBinBuffer &encrypted)
{
	WAJid jid(from);
	auto *pSession = createSession(jid.user, jid.device);

	signal_buffer *result = nullptr;
	if (!mir_strcmp(pszType, "pkmsg")) {
		pre_key_signal_message *pMsg; 
		logError(
			pre_key_signal_message_deserialize(&pMsg, encrypted.data(), encrypted.length(), m_pContext),
			"unable to deserialize prekey message");

		logError(
			session_cipher_decrypt_pre_key_signal_message(pSession->getCipher(), pMsg, this, &result),
			"unable to decrypt prekey message");

		pre_key_signal_message_destroy((signal_type_base*)pMsg);
	}
	else {
		signal_message *pMsg;
		logError(
			signal_message_deserialize(&pMsg, encrypted.data(), encrypted.length(), m_pContext),
			"unable to deserialize signal message");

		logError(
			session_cipher_decrypt_signal_message(pSession->getCipher(), pMsg, this, &result),
			"unable to decrypt signal message");

		signal_message_destroy((signal_type_base *)pMsg);
	}

	MBinBuffer res;
	res.assign(result->data, result->len);
	signal_buffer_free(result);
	return res;
}

MBinBuffer MSignalStore::decryptGroupSignalProto(const CMStringA &group, const CMStringA &sender, const MBinBuffer &encrypted)
{
	WAJid jid(sender);
	signal_protocol_sender_key_name senderKeyName;
	senderKeyName.group_id = group.c_str();
	senderKeyName.group_id_len = group.GetLength();
	senderKeyName.sender.device_id = 0;
	senderKeyName.sender.name = jid.user.c_str();
	senderKeyName.sender.name_len = jid.user.GetLength();

	group_cipher *cipher;
	logError(
		group_cipher_create(&cipher, m_pStore, &senderKeyName, m_pContext),
		"unable to create group cipher");

	sender_key_message *skmsg;
	logError(
		sender_key_message_deserialize(&skmsg, encrypted.data(), encrypted.length(), m_pContext),
		"unable to deserialize skmsg");

	signal_buffer *result = nullptr;
	logError(
		group_cipher_decrypt(cipher, skmsg, this, &result),
		"unable to decrypt skmsg");

	sender_key_message_destroy((signal_type_base *)skmsg);
	group_cipher_free(cipher);

	MBinBuffer res;
	res.assign(result->data, result->len);
	signal_buffer_free(result);
	return res;
}

void MSignalStore::processSenderKeyMessage(const CMStringA &author, const Wa__Message__SenderKeyDistributionMessage *msg)
{
	WAJid jid(author);
	signal_protocol_sender_key_name senderKeyName;
	senderKeyName.group_id = msg->groupid;
	senderKeyName.group_id_len = mir_strlen(msg->groupid);
	senderKeyName.sender.device_id = 0;
	senderKeyName.sender.name = jid.user.c_str();
	senderKeyName.sender.name_len = jid.user.GetLength();

	group_session_builder *builder;
	logError(
		group_session_builder_create(&builder, m_pStore, m_pContext),
		"unable to create session builder");

	sender_key_distribution_message *skmsg;
	logError(
		sender_key_distribution_message_deserialize(&skmsg, msg->axolotlsenderkeydistributionmessage.data, msg->axolotlsenderkeydistributionmessage.len, m_pContext),
		"unable to decode skd message");

	logError(
		group_session_builder_process_session(builder, &senderKeyName, skmsg),
		"unable to process skd message");

	sender_key_distribution_message_destroy((signal_type_base *)skmsg);
	group_session_builder_free(builder);
}

/////////////////////////////////////////////////////////////////////////////////////////
// encryption

MBinBuffer MSignalStore::encryptSenderKey(const WAJid &to, const CMStringA &from, const MBinBuffer &buf, MBinBuffer &skmsgKey)
{
	signal_protocol_sender_key_name senderKeyName;
	senderKeyName.group_id = to.user.c_str();
	senderKeyName.group_id_len = to.user.GetLength();
	senderKeyName.sender.device_id = 0;
	senderKeyName.sender.name = from.c_str();
	senderKeyName.sender.name_len = from.GetLength();

	group_session_builder *builder;
	logError(
		group_session_builder_create(&builder, m_pStore, m_pContext),
		"unable to create session builder");

	sender_key_distribution_message *skmsg;
	logError(
		group_session_builder_create_session(builder, &skmsg, &senderKeyName),
		"unable to create session");

	group_cipher *cipher;
	logError(
		group_cipher_create(&cipher, m_pStore, &senderKeyName, m_pContext),
		"unable to create group cipher");

	ciphertext_message *encMessage;
	logError(
		group_cipher_encrypt(cipher, buf.data(), buf.length(), &encMessage),
		"unable to encrypt group message");

	MBinBuffer res;
	auto *cipherText = ciphertext_message_get_serialized(encMessage);
	res.assign(cipherText->data, cipherText->len);

	auto *pKey = sender_key_distribution_message_get_signature_key(skmsg);
	skmsgKey.assign(pKey->data, sizeof(pKey->data));

	sender_key_distribution_message_destroy((signal_type_base*)skmsg);
	group_cipher_free(cipher);
	group_session_builder_free(builder);
	return res;
}

MBinBuffer MSignalStore::encryptSignalProto(const WAJid &to, const MBinBuffer &buf, int &type)
{
	auto *pSession = createSession(to.user, to.device);

	ciphertext_message *pEncrypted;
	logError(
		session_cipher_encrypt(pSession->getCipher(), buf.data(), buf.length(), &pEncrypted),
		"unable to encrypt signal message");

	type = ciphertext_message_get_type(pEncrypted);

	MBinBuffer res;
	auto *encBuf = ciphertext_message_get_serialized(pEncrypted);
	res.assign(encBuf->data, encBuf->len);
	SIGNAL_UNREF(pEncrypted);
	return res;
}

MBinBuffer MSignalStore::encodeSignedIdentity(bool bIncludeSignatureKey)
{
	proto::ADVSignedDeviceIdentity identity(pProto->getBlob("WAAccount"));
	
	if (!bIncludeSignatureKey)
		proto::CleanBinary(identity->accountsignaturekey), identity->has_accountsignaturekey = false;
	
	return proto::Serialize(identity);
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
