#include "stdafx.h"

/* FRIEND FUNCTIONS */

uint32_t tox_friend_add(Tox *tox, const uint8_t *address, const uint8_t *message, size_t length, TOX_ERR_FRIEND_ADD *error)
{
	return CreateFunction<uint32_t(*)(Tox*, const uint8_t*, const uint8_t*, size_t, TOX_ERR_FRIEND_ADD*)>(__FUNCTION__)(tox, address, message, length, error);
}

uint32_t tox_friend_add_norequest(Tox *tox, const uint8_t *public_key, TOX_ERR_FRIEND_ADD *error)
{
	return CreateFunction<uint32_t(*)(Tox*, const uint8_t*, TOX_ERR_FRIEND_ADD*)>(__FUNCTION__)(tox, public_key, error);
}

bool tox_friend_delete(Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_DELETE *error)
{
	return CreateFunction<bool(*)(Tox*, uint32_t, TOX_ERR_FRIEND_DELETE*)>(__FUNCTION__)(tox, friend_number, error);
}

uint32_t tox_friend_by_public_key(const Tox *tox, const uint8_t *public_key, TOX_ERR_FRIEND_BY_PUBLIC_KEY *error)
{
	return CreateFunction<uint32_t(*)(const Tox*, const uint8_t*, TOX_ERR_FRIEND_BY_PUBLIC_KEY*)>(__FUNCTION__)(tox, public_key, error);
}

int tox_friend_exists(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

size_t tox_self_get_friend_list_size(const Tox *tox)
{
	return CreateFunction<size_t(*)(const Tox*)>(__FUNCTION__)(tox);
}

void tox_self_get_friend_list(const Tox *tox, uint32_t *list)
{
	CreateFunction<void(*)(const Tox*, uint32_t*)>(__FUNCTION__)(tox, list);
}

bool tox_friend_get_public_key(const Tox *tox, uint32_t friend_number, uint8_t *public_key, TOX_ERR_FRIEND_GET_PUBLIC_KEY *error)
{
	return CreateFunction<bool(*)(const Tox*, int32_t, uint8_t*, TOX_ERR_FRIEND_GET_PUBLIC_KEY*)>(__FUNCTION__)(tox, friend_number, public_key, error);
}

uint64_t tox_friend_get_last_online(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_GET_LAST_ONLINE *error)
{
	return CreateFunction<uint64_t(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_GET_LAST_ONLINE*)>(__FUNCTION__)(tox, friend_number, error);
}

size_t tox_friend_get_name_size(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<size_t(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, error);
}

bool tox_friend_get_name(const Tox *tox, uint32_t friend_number, uint8_t *name, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<bool(*)(const Tox*, uint32_t, uint8_t*, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, name, error);
}

void tox_callback_friend_name(Tox *tox, tox_friend_name_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*tox, tox_friend_name_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

size_t tox_friend_get_status_message_size(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<size_t(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, error);
}

bool tox_friend_get_status_message(const Tox *tox, uint32_t friend_number, uint8_t *status_message, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<bool(*)(const Tox*, uint32_t, uint8_t*, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, status_message, error);
}

void tox_callback_friend_status_message(Tox *tox, tox_friend_status_message_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_status_message_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

TOX_USER_STATUS tox_friend_get_status(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<TOX_USER_STATUS(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, error);
}

void tox_callback_friend_status(Tox *tox, tox_friend_status_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_status_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

TOX_CONNECTION tox_friend_get_connection_status(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<TOX_CONNECTION(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, error);
}

void tox_callback_friend_connection_status(Tox *tox, tox_friend_connection_status_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_connection_status_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

bool tox_friend_get_typing(const Tox *tox, uint32_t friend_number, TOX_ERR_FRIEND_QUERY *error)
{
	return CreateFunction<bool(*)(const Tox*, uint32_t, TOX_ERR_FRIEND_QUERY*)>(__FUNCTION__)(tox, friend_number, error);
}

void tox_callback_friend_typing(Tox *tox, tox_friend_typing_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_typing_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

/*  */

bool tox_self_set_typing(Tox *tox, uint32_t friend_number, bool is_typing, TOX_ERR_SET_TYPING *error)
{
	return CreateFunction<bool(*)(Tox*, uint32_t, bool, TOX_ERR_SET_TYPING*)>(__FUNCTION__)(tox, friend_number, is_typing, error);
}

uint32_t tox_friend_send_message(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, TOX_ERR_FRIEND_SEND_MESSAGE *error)
{
	return CreateFunction<uint32_t(*)(Tox*, uint32_t, TOX_MESSAGE_TYPE, const uint8_t*, size_t, TOX_ERR_FRIEND_SEND_MESSAGE*)>(__FUNCTION__)(tox, friend_number, type, message, length, error);
}

void tox_callback_friend_read_receipt(Tox *tox, tox_friend_read_receipt_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_read_receipt_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_request(Tox *tox, tox_friend_request_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_request_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_friend_message(Tox *tox, tox_friend_message_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_friend_message_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}