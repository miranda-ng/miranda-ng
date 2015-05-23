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

uint32_t tox_friend_add(Tox *tox, const uint8_t *address, const uint8_t *message, size_t length, TOX_ERR_FRIEND_ADD *error)
{
	return CreateFunction<uint32_t(*)(Tox*, const uint8_t*, const uint8_t*, size_t, TOX_ERR_FRIEND_ADD*)>(__FUNCTION__)(tox, address, message, length, error);
}

uint32_t tox_friend_add_norequest(Tox *tox, const uint8_t *public_key, TOX_ERR_FRIEND_ADD *error)
{
	return CreateFunction<uint32_t(*)(Tox*, const uint8_t*, TOX_ERR_FRIEND_ADD*)>(__FUNCTION__)(tox, public_key, error);
}

uint32_t tox_friend_by_public_key(const Tox *tox, const uint8_t *public_key, TOX_ERR_FRIEND_BY_PUBLIC_KEY *error)
{
	return CreateFunction<uint32_t(*)(const Tox*, const uint8_t*, TOX_ERR_FRIEND_BY_PUBLIC_KEY*)>(__FUNCTION__)(tox, public_key, error);
}

bool tox_friend_get_public_key(const Tox *tox, uint32_t friend_number, uint8_t *public_key, TOX_ERR_FRIEND_GET_PUBLIC_KEY *error)
{
	return CreateFunction<bool(*)(const Tox*, int32_t, uint8_t*, TOX_ERR_FRIEND_GET_PUBLIC_KEY*)>(__FUNCTION__)(tox, friend_number, public_key, error);
}

bool tox_friend_delete(Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_DELETE *error)
{
	return CreateFunction<bool(*)(Tox*, uint32_t, TOX_ERR_FRIEND_DELETE*)>(__FUNCTION__)(tox, friend_number, error);
}

int tox_get_friend_connection_status(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_friend_exists(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

uint32_t tox_friend_send_message(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, TOX_ERR_FRIEND_SEND_MESSAGE *error)
{
	return CreateFunction<uint32_t(*)(Tox*, uint32_t, TOX_MESSAGE_TYPE, const uint8_t*, size_t, TOX_ERR_FRIEND_SEND_MESSAGE*)>(__FUNCTION__)(tox, friend_number, type, message, length, error);
}

bool tox_self_set_name(Tox *tox, const uint8_t *name, size_t length, TOX_ERR_SET_INFO *error)
{
	return CreateFunction<bool(*)(Tox*, const uint8_t*, size_t, TOX_ERR_SET_INFO*)>(__FUNCTION__)(tox, name, length, error);
}

void tox_self_get_name(const Tox *tox, uint8_t *name)
{
	CreateFunction<void(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, name);
}

bool tox_friend_get_name(const Tox *tox, uint32_t friend_number, uint8_t *name, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<bool(*)(const Tox*, uint32_t, uint8_t*, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, name, error);
}

size_t tox_friend_get_name_size(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<size_t(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, error);
}

int tox_get_self_name_size(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

bool tox_self_set_status_message(Tox *tox, const uint8_t *status, size_t length, TOX_ERR_SET_INFO *error)
{
	return CreateFunction<bool(*)(Tox*, const uint8_t*, size_t, TOX_ERR_SET_INFO*)>(__FUNCTION__)(tox, status, length, error);
}

void tox_self_set_status(Tox *tox, TOX_USER_STATUS user_status)
{
	CreateFunction<void(*)(Tox*, TOX_USER_STATUS)>(__FUNCTION__)(tox, user_status);
}

int tox_get_status_message_size(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

void tox_self_get_status_message(const Tox *tox, uint8_t *status)
{
	CreateFunction<void(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, status);
}

int tox_get_status_message(const Tox *tox, int32_t friendnumber, uint8_t *buf, uint32_t maxlen)
{
	return CreateFunction<int(*)(const Tox*, int32_t, uint8_t*, uint32_t)>(__FUNCTION__)(tox, friendnumber, buf, maxlen);
}

int tox_get_self_status_message(const Tox *tox, uint8_t *buf, uint32_t maxlen)
{
	return CreateFunction<int(*)(const Tox*, uint8_t*, uint32_t)>(__FUNCTION__)(tox, buf, maxlen);
}

uint8_t tox_get_user_status(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

uint8_t tox_get_self_user_status(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint64_t tox_friend_get_last_online(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_GET_LAST_ONLINE *error)
{
	return CreateFunction<uint64_t(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_GET_LAST_ONLINE*)>(__FUNCTION__)(tox, friend_number, error);
}

int tox_friend_get_typing(Tox *tox, int32_t friendnumber, uint8_t is_typing)
{
	return CreateFunction<int(*)(Tox*, int32_t, uint8_t)>(__FUNCTION__)(tox, friendnumber, is_typing);
}

bool tox_self_set_typing(Tox *tox, uint32_t friend_number, bool is_typing, TOX_ERR_SET_TYPING *error)
{
	return CreateFunction<bool(*)(Tox*, uint32_t, bool, TOX_ERR_SET_TYPING*)>(__FUNCTION__)(tox, friend_number, is_typing, error);
}

size_t tox_self_get_friend_list_size(const Tox *tox)
{
	return CreateFunction<size_t(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint32_t tox_get_num_online_friends(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

void tox_self_get_friend_list(const Tox *tox, uint32_t *list)
{
	CreateFunction<void(*)(const Tox*, uint32_t*)>(__FUNCTION__)(tox, list);
}

void tox_callback_friend_request(Tox *tox, tox_friend_request_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_request_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_message(Tox *tox, tox_friend_message_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_message_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_name(Tox *tox, tox_friend_name_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*tox, tox_friend_name_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_status_message(Tox *tox, tox_friend_status_message_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_status_message_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_status(Tox *tox, tox_friend_status_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_status_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_read_receipt(Tox *tox, tox_friend_read_receipt_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_read_receipt_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_typing(Tox *tox, tox_friend_typing_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_typing_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_connection_status(Tox *tox, tox_friend_connection_status_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_connection_status_cb, void*)>(__FUNCTION__)(tox, function, user_data);
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