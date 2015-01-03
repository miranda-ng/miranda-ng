#include "common.h"

/* MAIN FUNCTIONS */

Tox *tox_new(Tox_Options *options)
{
	return CreateFunction<Tox*(*)(void*)>(__FUNCTION__)(options);
}

void tox_kill(Tox *tox)
{
	CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
	tox = NULL;
}

void tox_get_address(const Tox *tox, uint8_t *address)
{
	CreateFunction<void(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, address);
}

int32_t tox_add_friend(Tox *tox,const uint8_t *address, const uint8_t *data, uint16_t length)
{
	return CreateFunction<int32_t(*)(Tox*, const uint8_t*, const uint8_t*, uint16_t)>(__FUNCTION__)(tox, address, data, length);
}

int32_t tox_add_friend_norequest(Tox *tox, const uint8_t *client_id)
{
	return CreateFunction<int32_t(*)(Tox*, const uint8_t*)>(__FUNCTION__)(tox, client_id);
}

int32_t tox_get_friend_number(const Tox *tox, const uint8_t *client_id)
{
	return CreateFunction<int32_t(*)(const Tox*, const uint8_t*)>(__FUNCTION__)(tox, client_id);
}

int tox_get_client_id(const Tox *tox, int32_t friendnumber, uint8_t *client_id)
{
	return CreateFunction<int(*)(const Tox*, int32_t, uint8_t*)>(__FUNCTION__)(tox, friendnumber, client_id);
}

int tox_del_friend(Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_get_friend_connection_status(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_friend_exists(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

uint32_t tox_send_message(Tox *tox, int32_t friendnumber, const uint8_t *message, uint32_t length)
{
	return CreateFunction<uint32_t(*)(Tox*, int32_t, const uint8_t*, uint32_t)>(__FUNCTION__)(tox, friendnumber, message, length);
}

uint32_t tox_send_action(Tox *tox, int32_t friendnumber, const uint8_t *action, uint32_t length)
{
	return CreateFunction<uint32_t(*)(Tox*, int32_t, const uint8_t*, uint32_t)>(__FUNCTION__)(tox, friendnumber, action, length);
}

int tox_set_name(Tox *tox, const uint8_t *name, uint16_t length)
{
	return CreateFunction<int(*)(Tox*, const uint8_t*, uint16_t)>(__FUNCTION__ )(tox, name, length);
}

uint16_t tox_get_self_name(const Tox *tox, uint8_t *name)
{
	return CreateFunction<uint16_t(*)(const Tox*, uint8_t*)>(__FUNCTION__)(tox, name);
}

int tox_get_name(const Tox *tox, int32_t friendnumber, uint8_t *name)
{
	return CreateFunction<int(*)(const Tox*, int32_t, uint8_t*)>(__FUNCTION__)(tox, friendnumber, name);
}

int tox_get_name_size(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_get_self_name_size(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

int tox_set_status_message(Tox *tox, const uint8_t *status, uint16_t length)
{
	return CreateFunction<int(*)(Tox*, const uint8_t*, uint16_t)>(__FUNCTION__)(tox, status, length);
}

int tox_set_user_status(Tox *tox, uint8_t userstatus)
{
	return CreateFunction<int(*)(Tox*, uint8_t)>(__FUNCTION__)(tox, userstatus);
}

int tox_get_status_message_size(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_get_self_status_message_size(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
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

uint64_t tox_get_last_online(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

int tox_set_user_is_typing(Tox *tox, int32_t friendnumber, uint8_t is_typing)
{
	return CreateFunction<int(*)(Tox*, int32_t, uint8_t)>(__FUNCTION__)(tox, friendnumber, is_typing);
}

uint8_t tox_get_is_typing(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

uint32_t tox_count_friendlist(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint32_t tox_get_num_online_friends(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint32_t tox_get_friendlist(const Tox *tox, int32_t *out_list, uint32_t list_size)
{
	return CreateFunction<int(*)(const Tox*, int32_t*, uint32_t)>(__FUNCTION__)(tox, out_list, list_size);
}

void tox_callback_friend_request(Tox *tox, void(*function)(Tox *tox, const uint8_t *, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, const uint8_t*, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_friend_message(Tox *tox, void(*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_friend_action(Tox *tox, void(*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_name_change(Tox *tox, void(*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*tox, void(*)(Tox*, int32_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_status_message(Tox *tox, void(*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_user_status(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_typing_change(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_read_receipt(Tox *tox, void(*function)(Tox *tox, int32_t, uint32_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint32_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_connection_status(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

/* SAVING AND LOADING FUNCTIONS */

uint32_t tox_size(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

void tox_save(const Tox *tox, uint8_t *data)
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