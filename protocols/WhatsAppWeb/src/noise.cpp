/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

WANoise class implementation

*/

#include "stdafx.h"

static uint8_t intro_header[] = {87, 65, 6, DICT_VERSION};
static uint8_t noise_init[] = "Noise_XX_25519_AESGCM_SHA256\0\0\0\0";

WANoise::WANoise(WhatsAppProto *_ppro) :
	ppro(_ppro)
{
	salt.assign(noise_init, 32);
	encKey.assign(noise_init, 32);
	decKey.assign(noise_init, 32);

	// generate ephemeral keys: public & private
	ec_key_pair *pKeys;
	curve_generate_key_pair(g_plugin.pCtx, &pKeys);

	auto *pPubKey = ec_key_pair_get_public(pKeys);
	ephemeral.pub.assign(pPubKey->data, sizeof(pPubKey->data));

	auto *pPrivKey = ec_key_pair_get_private(pKeys);
	ephemeral.priv.assign(pPrivKey->data, sizeof(pPrivKey->data));
	ec_key_pair_destroy(pKeys);

	// prepare hash
	memcpy(hash, noise_init, 32);
	updateHash(intro_header, 4);
	updateHash(ephemeral.pub.data(), ephemeral.pub.length());
}

/////////////////////////////////////////////////////////////////////////////////////////
// libsignal data initialization

void WANoise::init()
{
	// no data? generate them
	if (ppro->getDword(DBKEY_REG_ID, 0xFFFF) == 0xFFFF) {
		ppro->setDword(DBKEY_PREKEY_NEXT_ID, 1);
		ppro->setDword(DBKEY_PREKEY_UPLOAD_ID, 1);

		// generate registration id
		uint32_t regId;
		Utils_GetRandom(&regId, sizeof(regId));
		ppro->setDword(DBKEY_REG_ID, regId & 0x3FFF);

		// generate secret key
		uint8_t secretKey[32];
		Utils_GetRandom(secretKey, sizeof(secretKey));
		db_set_blob(0, ppro->m_szModuleName, DBKEY_SECRET_KEY, secretKey, sizeof(secretKey));

		// generate noise keys (private & public)
		ec_key_pair *pKeys;
		curve_generate_key_pair(g_plugin.pCtx, &pKeys);

		auto *pPubKey = ec_key_pair_get_public(pKeys);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_NOISE_PUB, pPubKey->data, sizeof(pPubKey->data));

		auto *pPrivKey = ec_key_pair_get_private(pKeys);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_NOISE_PRIV, pPrivKey->data, sizeof(pPrivKey->data));
		ec_key_pair_destroy(pKeys);

		// generate signed identity keys (private & public)
		ratchet_identity_key_pair *keyPair;
		signal_protocol_key_helper_generate_identity_key_pair(&keyPair, g_plugin.pCtx);

		pPubKey = ratchet_identity_key_pair_get_public(keyPair);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_SIGNED_IDENTITY_PUB, pPubKey->data, sizeof(pPubKey->data));

		pPrivKey = ratchet_identity_key_pair_get_private(keyPair);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_SIGNED_IDENTITY_PRIV, pPrivKey->data, sizeof(pPrivKey->data));

		// generate pre keys
		const unsigned int signed_pre_key_id = 1;
		ppro->setDword(DBKEY_PREKEY_KEYID, 1);

		session_signed_pre_key *signed_pre_key;
		signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, keyPair, signed_pre_key_id, time(0), g_plugin.pCtx);
		SIGNAL_UNREF(keyPair);

		signal_buffer *serialized_signed_pre_key;
		session_signed_pre_key_serialize(&serialized_signed_pre_key, signed_pre_key);

		pKeys = session_signed_pre_key_get_key_pair(signed_pre_key);
		pPubKey = ec_key_pair_get_public(pKeys);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_PREKEY_PUB, pPubKey->data, sizeof(pPubKey->data));

		pPrivKey = ec_key_pair_get_private(pKeys);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_PREKEY_PRIV, pPrivKey->data, sizeof(pPrivKey->data));

		db_set_blob(0, ppro->m_szModuleName, DBKEY_PREKEY_SIGN, (void*)session_signed_pre_key_get_signature(signed_pre_key), (int)session_signed_pre_key_get_signature_len(signed_pre_key));

		// generate and save pre keys set
		CMStringA szSetting;
		signal_protocol_key_helper_pre_key_list_node *keys_root;
		signal_protocol_key_helper_generate_pre_keys(&keys_root, 0, 20, g_plugin.pCtx);
		for (auto *it = keys_root; it; it = signal_protocol_key_helper_key_list_next(it)) {
			session_pre_key *pre_key = signal_protocol_key_helper_key_list_element(it);
			uint32_t pre_key_id = session_pre_key_get_id(pre_key);
			{
				signal_buffer *serialized_pre_key;
				session_pre_key_serialize(&serialized_pre_key, pre_key);
				szSetting.Format("PreKey%d", pre_key_id);
				db_set_blob(0, ppro->m_szModuleName, szSetting, signal_buffer_data(serialized_pre_key), (unsigned int)signal_buffer_len(serialized_pre_key));
				SIGNAL_UNREF(serialized_pre_key);
			}

			ec_key_pair *pre_key_pair = session_pre_key_get_key_pair(pre_key);
			pPubKey = ec_key_pair_get_public(pre_key_pair);
			szSetting.Format("PreKey%dPublic", pre_key_id);
			db_set_blob(0, ppro->m_szModuleName, szSetting, pPrivKey->data, sizeof(pPrivKey->data));
		}
		signal_protocol_key_helper_key_list_free(keys_root);
	}

	noiseKeys.pub = ppro->getBlob(DBKEY_NOISE_PUB);
	noiseKeys.priv = ppro->getBlob(DBKEY_NOISE_PRIV);

	signedIdentity.pub = ppro->getBlob(DBKEY_SIGNED_IDENTITY_PUB);
	signedIdentity.priv = ppro->getBlob(DBKEY_SIGNED_IDENTITY_PRIV);

	preKey.pub = ppro->getBlob(DBKEY_PREKEY_PUB);
	preKey.priv = ppro->getBlob(DBKEY_PREKEY_PRIV);
	preKey.keyid = ppro->getDword(DBKEY_PREKEY_KEYID);
	preKey.signature = ppro->getBlob(DBKEY_PREKEY_SIGN);
}

void WANoise::finish()
{
	deriveKey("", 0, encKey, decKey);
	readCounter = writeCounter = 0;
	memset(hash, 0, sizeof(hash));
	bInitFinished = true;
}

void WANoise::deriveKey(const void *pData, size_t cbLen, MBinBuffer &write, MBinBuffer &read)
{
	size_t outlen = 64;
	uint8_t out[64];
	HKDF(EVP_sha256(), (BYTE *)salt.data(), (int)salt.length(), (BYTE *)pData, (int)cbLen, (BYTE *)"", 0, out, outlen);

	write.assign(out, 32);
	read.assign(out + 32, 32);
}

void WANoise::mixIntoKey(const void *n, const void *p)
{
	uint8_t tmp[32];
	crypto_scalarmult((unsigned char *)tmp, (const unsigned char *)n, (const unsigned char *)p);

	deriveKey(tmp, sizeof(tmp), salt, encKey);
	decKey.assign(encKey.data(), encKey.length());
	readCounter = writeCounter = 0;
}

MBinBuffer WANoise::decrypt(const void *pData, size_t cbLen)
{
	uint8_t iv[12];
	generateIV(iv, (bInitFinished) ? readCounter : writeCounter);

	MBinBuffer res;
	uint8_t outbuf[1024 + EVP_MAX_BLOCK_LENGTH];

	int tag_len = 0, dec_len = 0, final_len = 0;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, (BYTE *)decKey.data(), iv);

	if (!bInitFinished)
		EVP_DecryptUpdate(ctx, nullptr, &tag_len, hash, sizeof(hash));

	cbLen -= 16;
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (BYTE *)pData + cbLen);

	for (size_t len = 0; len < cbLen; len += 1024) {
		size_t portionSize = cbLen - len;
		EVP_DecryptUpdate(ctx, outbuf, &dec_len, (BYTE *)pData + len, (int)min(portionSize, 1024));
		res.append(outbuf, dec_len);
	}

	if (!EVP_DecryptFinal_ex(ctx, outbuf, &final_len))
		ppro->debugLogA("Decryption failed");

	if (final_len)
		res.append(outbuf, final_len);
	EVP_CIPHER_CTX_free(ctx);

	updateHash(pData, cbLen + 16);
	return res;
}

size_t WANoise::decodeFrame(const void *&pData, size_t &cbLen)
{
	auto *p = (const uint8_t *)pData;

	if (cbLen < 3)
		return 0;

	size_t payloadLen = 0;
	for (int i = 0; i < 3; i++) {
		payloadLen <<= 8;
		payloadLen += p[i];
	}

	// ppro->debugLogA("got payload of size %d", payloadLen);

	cbLen -= 3;
	if (payloadLen > cbLen) {
		ppro->debugLogA("payload length %d exceeds capacity %d", payloadLen, cbLen);
		return 0;
	}

	pData = p + 3;
	return payloadLen;
}

MBinBuffer WANoise::encodeFrame(const void *pData, size_t cbLen)
{
	MBinBuffer res;
	if (!bSendIntro) {
		bSendIntro = true;
		res.append(intro_header, 4);
	}

	uint8_t buf[3];
	size_t foo = cbLen;
	for (int i = 0; i < 3; i++) {
		buf[2 - i] = foo & 0xFF;
		foo >>= 8;
	}
	res.append(buf, 3);
	res.append(pData, cbLen);
	return res;
}

MBinBuffer WANoise::encrypt(const void *pData, size_t cbLen)
{
	uint8_t iv[12];
	generateIV(iv, writeCounter);

	MBinBuffer res;
	uint8_t outbuf[1024 + 64];

	int enc_len = 0, final_len = 0;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, (BYTE *)encKey.data(), iv);

	if (!bInitFinished)
		EVP_EncryptUpdate(ctx, NULL, &enc_len, hash, sizeof(hash));
	
	for (size_t len = 0; len < cbLen; len += 1024) {
		size_t portionSize = cbLen - len;
		EVP_EncryptUpdate(ctx, outbuf, &enc_len, (BYTE *)pData + len, (int)min(portionSize, 1024));
		res.append(outbuf, enc_len);
	}
	EVP_EncryptFinal_ex(ctx, outbuf, &final_len);
	if (final_len)
		res.append(outbuf, final_len);

	uint8_t tag[16];
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag);
	res.append(tag, sizeof(tag));

	EVP_CIPHER_CTX_free(ctx);

	updateHash(res.data(), res.length());
	return res;
}

void WANoise::updateHash(const void *pData, size_t cbLen)
{
	if (bInitFinished)
		return;

	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, hash, sizeof(hash));
	SHA256_Update(&ctx, pData, cbLen);
	SHA256_Final(hash, &ctx);
}
