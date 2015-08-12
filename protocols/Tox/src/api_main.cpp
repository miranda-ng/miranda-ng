#include "stdafx.h"

/* MAIN FUNCTIONS */

struct Tox_Options *tox_options_new(TOX_ERR_OPTIONS_NEW *error)
{
	return CreateFunction<struct Tox_Options*(*)(TOX_ERR_OPTIONS_NEW*)>(__FUNCTION__)(error);
}

void tox_options_free(struct Tox_Options *options)
{
	CreateFunction<void(*)(struct Tox_Options*)>(__FUNCTION__)(options);
}

Tox *tox_new(const struct Tox_Options *options, TOX_ERR_NEW *error)
{
	return CreateFunction<Tox*(*)(const struct Tox_Options*, TOX_ERR_NEW*)>(__FUNCTION__)(options, error);
}

void tox_kill(Tox *tox)
{
	CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
	tox = NULL;
}

void tox_self_get_address(const Tox *tox, uint8_t *address)
{
	CreateFunction<void(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, address);
}

bool tox_self_set_name(Tox *tox, const uint8_t *name, size_t length, TOX_ERR_SET_INFO *error)
{
	return CreateFunction<bool(*)(Tox*, const uint8_t*, size_t, TOX_ERR_SET_INFO*)>(__FUNCTION__)(tox, name, length, error);
}

void tox_self_get_name(const Tox *tox, uint8_t *name)
{
	CreateFunction<void(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, name);
}

int tox_get_self_name_size(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

size_t tox_self_get_status_message_size(const Tox *tox)
{
	return CreateFunction<size_t(*)(const Tox*)>(__FUNCTION__)(tox);
}

bool tox_self_set_status_message(Tox *tox, const uint8_t *status, size_t length, TOX_ERR_SET_INFO *error)
{
	return CreateFunction<bool(*)(Tox*, const uint8_t*, size_t, TOX_ERR_SET_INFO*)>(__FUNCTION__)(tox, status, length, error);
}

void tox_self_set_status(Tox *tox, TOX_USER_STATUS user_status)
{
	CreateFunction<void(*)(Tox*, TOX_USER_STATUS)>(__FUNCTION__)(tox, user_status);
}

void tox_self_get_status_message(const Tox *tox, uint8_t *status)
{
	CreateFunction<void(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, status);
}

int tox_get_self_status_message(const Tox *tox, uint8_t *buf, uint32_t maxlen)
{
	return CreateFunction<int(*)(const Tox*, uint8_t*, uint32_t)>(__FUNCTION__)(tox, buf, maxlen);
}

uint8_t tox_get_self_user_status(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

/* SAVING AND LOADING FUNCTIONS */

size_t tox_get_savedata_size(const Tox *tox)
{
	return CreateFunction<size_t(*)(const Tox*)>(__FUNCTION__)(tox);
}

void tox_get_savedata(const Tox *tox, uint8_t *data)
{
	CreateFunction<int(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, data);
}

int tox_load(Tox *tox, const uint8_t *data, uint32_t length)
{
	return CreateFunction<int(*)(Tox*, const uint8_t*, uint32_t)>(__FUNCTION__)(tox, data, length);
}

/* ADVANCED FUNCTIONS (If you don't know what they do you can safely ignore them.) */
/*
uint32_t tox_get_nospam(const Tox *tox)
{
}

void tox_set_nospam(Tox *tox, uint32_t nospam)
{
}

void tox_get_keys(Tox *tox, uint8_t *public_key, uint8_t *secret_key)
{
}

int tox_lossy_packet_registerhandler(Tox *tox, int32_t friendnumber, uint8_t byte, int(*packet_handler_callback)(Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t len, void *object), void *object)
{
}

int tox_send_lossy_packet(const Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t length)
{
}

int tox_lossless_packet_registerhandler(Tox *tox, int32_t friendnumber, uint8_t byte, int(*packet_handler_callback)(Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t len, void *object), void *object)
{
}

int tox_send_lossless_packet(const Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t length)
{
}
*/