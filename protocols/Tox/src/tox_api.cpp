#include "common.h"

/* GENERAL FUNCTIONS */

Tox *tox_new(Tox_Options *options)
{
	return CreateFunction<Tox*(*)(void*)>(__FUNCTION__)(options);
}

void tox_kill(Tox *tox)
{
	CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
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
	return CreateFunction<int(*)()>(__FUNCTION__ )();
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
/* GROUP CHAT FUNCTIONS: WARNING Group chats will be rewritten so this might change */
/*
void tox_callback_group_invite(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, const uint8_t *, uint16_t, void *), void *userdata)
{
}

void tox_callback_group_message(Tox *tox, void(*function)(Tox *tox, int, int, const uint8_t *, uint16_t, void *), void *userdata)
{
}

void tox_callback_group_action(Tox *tox, void(*function)(Tox *tox, int, int, const uint8_t *, uint16_t, void *), void *userdata)
{
}

void tox_callback_group_title(Tox *tox, void(*function)(Tox *tox, int, int, const uint8_t *, uint8_t, void *), void *userdata)
{
}

void tox_callback_group_namelist_change(Tox *tox, void(*function)(Tox *tox, int, int, uint8_t, void *), void *userdata)
{
}

int tox_add_groupchat(Tox *tox)
{
}

int tox_del_groupchat(Tox *tox, int groupnumber)
{
}

int tox_group_peername(const Tox *tox, int groupnumber, int peernumber, uint8_t *name)
{
}

int tox_group_peer_pubkey(const Tox *tox, int groupnumber, int peernumber, uint8_t *pk)
{
}

int tox_invite_friend(Tox *tox, int32_t friendnumber, int groupnumber)
{
}

int tox_join_groupchat(Tox *tox, int32_t friendnumber, const uint8_t *data, uint16_t length)
{
}

int tox_group_message_send(Tox *tox, int groupnumber, const uint8_t *message, uint16_t length)
{
}

int tox_group_action_send(Tox *tox, int groupnumber, const uint8_t *action, uint16_t length)
{
}

int tox_group_set_title(Tox *tox, int groupnumber, const uint8_t *title, uint8_t length)
{
}

int tox_group_get_title(Tox *tox, int groupnumber, uint8_t *title, uint32_t max_length)
{
}

unsigned int tox_group_peernumber_is_ours(const Tox *tox, int groupnumber, int peernumber)
{
}

int tox_group_number_peers(const Tox *tox, int groupnumber)
{
}

int tox_group_get_names(const Tox *tox, int groupnumber, uint8_t names[][TOX_MAX_NAME_LENGTH], uint16_t lengths[], uint16_t length)
{
}

uint32_t tox_count_chatlist(const Tox *tox)
{
}

uint32_t tox_get_chatlist(const Tox *tox, int32_t *out_list, uint32_t list_size)
{
}

int tox_group_get_type(const Tox *tox, int groupnumber)
{
}
*/
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

/* FILE SENDING FUNCTIONS */

void tox_callback_file_send_request(Tox *tox, void(*function)(Tox *m, int32_t, uint8_t, uint64_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, uint64_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_file_control(Tox *tox, void(*function)(Tox *m, int32_t, uint8_t, uint8_t, uint8_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, uint8_t, uint8_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

void tox_callback_file_data(Tox *tox, void(*function)(Tox *m, int32_t, uint8_t, const uint8_t *, uint16_t length, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*tox, void(*)(Tox*, int32_t, uint8_t, const uint8_t *, uint16_t length, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

int tox_new_file_sender(Tox *tox, int32_t friendnumber, uint64_t filesize, const uint8_t *filename, uint16_t filename_length)
{
	return CreateFunction<int(*)(Tox*, int32_t, uint64_t, const uint8_t*, uint16_t)>(__FUNCTION__)(tox, friendnumber, filesize, filename, filename_length);
}

int tox_file_send_control(Tox *tox, int32_t friendnumber, uint8_t send_receive, uint8_t filenumber, uint8_t message_id, const uint8_t *data, uint16_t length)
{
	return CreateFunction<int(*)(Tox*, int32_t, uint8_t, uint8_t, uint8_t, const uint8_t*, uint16_t)>(__FUNCTION__)(tox, friendnumber, send_receive, filenumber, message_id, data, length);
}

int tox_file_send_data(Tox *tox, int32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length)
{
	return CreateFunction<int(*)(Tox*, int32_t, uint8_t, const uint8_t*, uint16_t)>(__FUNCTION__)(tox, friendnumber, filenumber, data, length);
}

int tox_file_data_size(const Tox *tox, int32_t friendnumber)
{
	return CreateFunction<int(*)(const Tox*, int32_t)>(__FUNCTION__)(tox, friendnumber);
}

uint64_t tox_file_data_remaining(const Tox *tox, int32_t friendnumber, uint8_t filenumber, uint8_t send_receive)
{
	return CreateFunction<int(*)(const Tox*, int32_t, uint8_t, uint8_t)>(__FUNCTION__)(tox, friendnumber, filenumber, send_receive);
}

/* CONNECTION FUNCTIONS */

int tox_bootstrap_from_address(Tox *tox, const char *address, uint16_t port, const uint8_t *public_key)
{
	return CreateFunction<int(*)(Tox*, const char*, uint16_t, const uint8_t*)>(__FUNCTION__)(tox, address, port, public_key);
}

int tox_add_tcp_relay(Tox *tox, const char *address, uint16_t port, const uint8_t *public_key)
{
	return CreateFunction<int(*)(Tox*, const char*, uint16_t, const uint8_t*)>(__FUNCTION__)(tox, address, port,public_key);
}

int tox_isconnected(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint32_t tox_do_interval(Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

void tox_do(Tox *tox)
{
	CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
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

/* DNS TOXID RESOILVING FUNCTIONS */

void *tox_dns3_new(uint8_t *server_public_key)
{
	return CreateFunction<void*(*)(uint8_t*)>(__FUNCTION__)(server_public_key);
}

void tox_dns3_kill(void *dns3_object)
{
	CreateFunction<void(*)(void*)>(__FUNCTION__)(dns3_object);
}

int tox_generate_dns3_string(void *dns3_object, uint8_t *string, uint16_t string_max_len, uint32_t *request_id, uint8_t *name, uint8_t name_len)
{
	return CreateFunction<int(*)(void*, uint8_t*, uint16_t, uint32_t*, uint8_t*, uint8_t)>(__FUNCTION__)(dns3_object, string, string_max_len, request_id, name, name_len);
}

int tox_decrypt_dns3_TXT(void *dns3_object, uint8_t *tox_id, uint8_t *id_record, uint32_t id_record_len, uint32_t request_id)
{
	return CreateFunction<int(*)(void*, uint8_t*, uint8_t*, uint32_t, uint32_t)>(__FUNCTION__)(dns3_object, tox_id, id_record, id_record_len, request_id);
}