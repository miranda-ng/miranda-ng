#include "stdafx.h"

/* ENCRYPTION FUNCTIONS */

bool tox_pass_decrypt(const uint8_t *data, size_t length, uint8_t *passphrase, size_t pplength, uint8_t *out, TOX_ERR_DECRYPTION *error)
{
	return CreateFunction<bool(*)(const uint8_t *, size_t, uint8_t*, size_t, uint8_t*, TOX_ERR_DECRYPTION*)>(__FUNCTION__)(data, length, passphrase, pplength, out, error);
}

bool tox_pass_encrypt(const uint8_t *data, size_t data_len, uint8_t *passphrase, size_t pplength, uint8_t *out, TOX_ERR_ENCRYPTION *error)
{
	return CreateFunction<bool(*)(const uint8_t *, size_t, uint8_t*, size_t, uint8_t*, TOX_ERR_ENCRYPTION*)>(__FUNCTION__)(data, data_len, passphrase, pplength, out, error);
}

bool tox_is_data_encrypted(const uint8_t *data)
{
	return CreateFunction<bool(*)(const uint8_t*)>(__FUNCTION__)(data);
}