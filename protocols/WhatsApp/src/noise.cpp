/*

WhatsApp plugin for Miranda NG
Copyright © 2019-23 George Hazan

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
	curve_generate_key_pair(ppro->m_signalStore.CTX(), &pKeys);

	auto *pPubKey = ec_key_pair_get_public(pKeys);
	ephemeral.pub.assign(pPubKey->data, sizeof(pPubKey->data));

	auto *pPrivKey = ec_key_pair_get_private(pKeys);
	ephemeral.priv.assign(pPrivKey->data, sizeof(pPrivKey->data));
	ec_key_pair_destroy((signal_type_base*)pKeys);

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
		curve_generate_key_pair(ppro->m_signalStore.CTX(), &pKeys);

		auto *pPubKey = ec_key_pair_get_public(pKeys);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_NOISE_PUB, pPubKey->data, sizeof(pPubKey->data));

		auto *pPrivKey = ec_key_pair_get_private(pKeys);
		db_set_blob(0, ppro->m_szModuleName, DBKEY_NOISE_PRIV, pPrivKey->data, sizeof(pPrivKey->data));
		ec_key_pair_destroy((signal_type_base *)pKeys);
	}

	noiseKeys.pub = ppro->getBlob(DBKEY_NOISE_PUB);
	noiseKeys.priv = ppro->getBlob(DBKEY_NOISE_PRIV);
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
	HKDF(EVP_sha256(), salt.data(), (int)salt.length(), (BYTE *)pData, (int)cbLen, (BYTE *)"", 0, out, outlen);

	write.assign(out, 32);
	read.assign(out + 32, 32);
}

void WANoise::mixIntoKey(const void *n, const void *p)
{
	uint8_t tmp[32];
	curve25519_donna((unsigned char *)tmp, (const unsigned char *)n, (const unsigned char *)p);

	deriveKey(tmp, sizeof(tmp), salt, encKey);
	decKey.assign(encKey.data(), encKey.length());
	readCounter = writeCounter = 0;
}

MBinBuffer WANoise::decrypt(const void *pData, size_t cbLen)
{
	uint8_t iv[12];
	generateIV(iv, (bInitFinished) ? readCounter : writeCounter);

	MBinBuffer res;
	if (!bInitFinished)
		res = aesDecrypt(EVP_aes_256_gcm(), decKey.data(), iv, pData, cbLen, hash, sizeof(hash));
	else
		res = aesDecrypt(EVP_aes_256_gcm(), decKey.data(), iv, pData, cbLen);

	updateHash(pData, cbLen);
	return res;
}

size_t WANoise::decodeFrame(const uint8_t *&p, size_t &cbLen)
{
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

	p += 3;
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
	EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, encKey.data(), iv);

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
