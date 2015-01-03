#include "common.h"

/* AVATAR FUNCTIONS */

void tox_callback_avatar_info(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, uint8_t *, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, uint8_t*, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_avatar_data(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, uint8_t *, uint8_t *, uint32_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, uint8_t*, uint8_t*, uint32_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

int tox_set_avatar(Tox *tox, uint8_t format, const uint8_t *data, uint32_t length)
{
	return CreateFunction<int(*)(Tox*, uint8_t, const uint8_t*, uint32_t)>(__FUNCTION__)(tox, format, data, length);
}

int tox_unset_avatar(Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

int tox_get_self_avatar(const Tox *tox, uint8_t *format, uint8_t *buf, uint32_t *length, uint32_t maxlen, uint8_t *hash)
{
	return CreateFunction<int(*)(const Tox*, uint8_t*, uint8_t*, uint32_t*, uint32_t, uint8_t*)>(__FUNCTION__)(tox, format, buf, length, maxlen, hash);
}

int tox_hash(uint8_t *hash, const uint8_t *data, const uint32_t datalen)
{
	return CreateFunction<int(*)(uint8_t*, const uint8_t*, const uint32_t)>(__FUNCTION__)(hash, data, datalen);
}

int tox_request_avatar_info(const Tox *tox, const int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, const int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_send_avatar_info(Tox *tox, const int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, const int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_request_avatar_data(const Tox *tox, const int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, const int32_t)>(__FUNCTION__)(tox, friendnumber);
}