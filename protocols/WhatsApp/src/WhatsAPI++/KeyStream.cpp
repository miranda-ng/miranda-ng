/*
 * WALogin.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "WALogin.h"
#include "ByteArray.h"
#include "ProtocolTreeNode.h"
#include "WAException.h"

using namespace Utilities;

KeyStream::KeyStream() :
	seq(0)
{
	HMAC_CTX_init(&hmac);
}

KeyStream::~KeyStream()
{
	HMAC_CTX_cleanup(&hmac);
}

void KeyStream::init(unsigned char* _key, unsigned char* _keyMac)
{
	memcpy(key, _key, 20);
	memcpy(keyMac, _keyMac, 20);

	RC4_set_key(&this->rc4, 20, this->key);

	unsigned char drop[768];
	RC4(&this->rc4, sizeof(drop), drop, drop);
}

void KeyStream::keyFromPasswordAndNonce(const std::string &pass, const std::vector<unsigned char>& nonce, unsigned char *out)
{
	size_t cbSize = nonce.size();

	uint8_t *pNonce = (uint8_t*)_alloca(cbSize + 1);
	memcpy(pNonce, nonce.data(), cbSize);

	for (int i = 0; i < 4; i++) {
		pNonce[cbSize] = i + 1;
		PKCS5_PBKDF2_HMAC_SHA1(pass.data(), (int)pass.size(), pNonce, (int)cbSize+1, 2, 20, out + i*20);
	}
}

void KeyStream::decodeMessage(unsigned char* buffer, int macOffset, int offset, const int length)
{
	unsigned char digest[20];
	this->hmacsha1(buffer + offset, length, digest);

	if (memcmp(&buffer[macOffset], digest, 4))
		throw WAException("invalid MAC", WAException::CORRUPT_STREAM_EX, 0);

	unsigned char* out = (unsigned char*)_alloca(length);
	RC4(&this->rc4, length, buffer + offset, out);
	memcpy(buffer + offset, out, length);
}

void KeyStream::encodeMessage(unsigned char* buffer, int macOffset, int offset, const int length)
{
	unsigned char* out = (unsigned char*)_alloca(length);
	RC4(&this->rc4, length, buffer + offset, out);
	memcpy(buffer + offset, out, length);

	unsigned char digest[20];
	this->hmacsha1(buffer + offset, length, digest);
	memcpy(buffer + macOffset, digest, 4);
}

void KeyStream::hmacsha1(unsigned char* text, int textLength, unsigned char *out)
{
	HMAC_Init(&hmac, this->keyMac, 20, EVP_sha1());
	HMAC_Update(&hmac, text, textLength);

	unsigned char hmacInt[4];
	hmacInt[0] = (this->seq >> 24);
	hmacInt[1] = (this->seq >> 16);
	hmacInt[2] = (this->seq >> 8);
	hmacInt[3] = (this->seq);
	HMAC_Update(&hmac, hmacInt, sizeof(hmacInt));

	unsigned int mdLength;
	HMAC_Final(&hmac, out, &mdLength);

	this->seq++;
}
