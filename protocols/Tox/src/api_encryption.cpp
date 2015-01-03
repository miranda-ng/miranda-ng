#include "common.h"

/* ENCRYPTION FUNCTIONS */

int tox_pass_encryption_extra_length()
{
	return CreateFunction<int(*)()>(__FUNCTION__)();
}

int tox_pass_key_length()
{
	return CreateFunction<int(*)()>(__FUNCTION__)();
}

int tox_pass_salt_length()
{
	return CreateFunction<int(*)()>(__FUNCTION__)();
}

uint32_t tox_encrypted_size(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

int tox_pass_encrypt(const uint8_t *data, uint32_t data_len, uint8_t *passphrase, uint32_t pplength, uint8_t *out)
{
	return CreateFunction<int(*)(const uint8_t*, uint32_t, uint8_t*, uint32_t, uint8_t*)>(__FUNCTION__)(data, data_len, passphrase, pplength, out);
}

int tox_encrypted_save(const Tox *tox, uint8_t *data, uint8_t *passphrase, uint32_t pplength)
{
	return CreateFunction<int(*)(const Tox*, uint8_t*, uint8_t*, uint32_t)>(__FUNCTION__)(tox, data, passphrase, pplength);
}

int tox_pass_decrypt(const uint8_t *data, uint32_t length, uint8_t *passphrase, uint32_t pplength, uint8_t *out)
{
	return CreateFunction<int(*)(const uint8_t*, uint32_t, uint8_t*, uint32_t, uint8_t*)>(__FUNCTION__)(data, length, passphrase, pplength, out);
}

int tox_encrypted_load(Tox *tox, const uint8_t *data, uint32_t length, uint8_t *passphrase, uint32_t pplength)
{
	return CreateFunction<int(*)(Tox*, const uint8_t*, uint32_t, uint8_t*, uint32_t)>(__FUNCTION__)(tox, data, length, passphrase, pplength);
}

int tox_derive_key_from_pass(uint8_t *passphrase, uint32_t pplength, uint8_t *out_key)
{
	return CreateFunction<int(*)(uint8_t*, uint32_t, uint8_t*)>(__FUNCTION__)(passphrase, pplength, out_key);
}

int tox_derive_key_with_salt(uint8_t *passphrase, uint32_t pplength, uint8_t *salt, uint8_t *out_key)
{
	return CreateFunction<int(*)(uint8_t*, uint32_t, uint8_t*, uint8_t*)>(__FUNCTION__)(passphrase, pplength, salt, out_key);
}

int tox_get_salt(uint8_t *data, uint8_t *salt)
{
	return CreateFunction<int(*)(uint8_t*, uint8_t*)>(__FUNCTION__)(data, salt);
}

int tox_pass_key_encrypt(const uint8_t *data, uint32_t data_len, const uint8_t *key, uint8_t *out)
{
	return CreateFunction<int(*)(const uint8_t*, uint32_t, const uint8_t*, uint8_t*)>(__FUNCTION__)(data, data_len, key, out);
}

int tox_encrypted_key_save(const Tox *tox, uint8_t *data, uint8_t *key)
{
	return CreateFunction<int(*)(const Tox*, uint8_t*, uint8_t*)>(__FUNCTION__)(tox, data, key);
}

int tox_pass_key_decrypt(const uint8_t *data, uint32_t length, const uint8_t *key, uint8_t *out)
{
	return CreateFunction<int(*)(const uint8_t*, uint32_t, const uint8_t*, uint8_t*)>(__FUNCTION__)(data, length, key, out);
}

int tox_encrypted_key_load(Tox *tox, const uint8_t *data, uint32_t length, uint8_t *key)
{
	return CreateFunction<int(*)(Tox*, const uint8_t*, uint32_t, uint8_t*)>(__FUNCTION__)(tox, data, length, key);
}

int tox_is_data_encrypted(const uint8_t *data)
{
	return CreateFunction<int(*)(const uint8_t*)>(__FUNCTION__)(data);
}

int tox_is_save_encrypted(const uint8_t *data)
{
	return CreateFunction<int(*)(const uint8_t*)>(__FUNCTION__)(data);
}