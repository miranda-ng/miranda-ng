#include "stdafx.h"

/* FILE SENDING FUNCTIONS */

bool tox_hash(uint8_t *hash, const uint8_t *data, size_t datalen)
{
	return CreateFunction<bool(*)(uint8_t*, const uint8_t*, size_t)>(__FUNCTION__)(hash, data, datalen);
}

bool tox_file_get_file_id(const Tox *tox, uint32_t friend_number, uint32_t file_number, uint8_t *file_id, TOX_ERR_FILE_GET *error)
{
	return CreateFunction<bool(*)(const Tox*, uint32_t, uint32_t, uint8_t*, TOX_ERR_FILE_GET*)>(__FUNCTION__)(tox, friend_number, file_number, file_id, error);
}

uint32_t tox_file_send(Tox *tox, uint32_t friend_number, uint32_t kind, uint64_t file_size, const uint8_t *file_id, const uint8_t *filename, size_t filename_length, TOX_ERR_FILE_SEND *error)
{
	return CreateFunction<uint32_t(*)(Tox*, uint32_t, uint32_t, uint64_t, const uint8_t*, const uint8_t*, size_t, TOX_ERR_FILE_SEND*)>(__FUNCTION__)(tox, friend_number, kind, file_size, file_id, filename, filename_length, error);
}

bool tox_file_send_chunk(Tox *tox, uint32_t friend_number, uint32_t file_number, uint64_t position, const uint8_t *data, size_t length, TOX_ERR_FILE_SEND_CHUNK *error)
{
	return CreateFunction<bool(*)(Tox*, uint32_t, uint32_t, uint64_t, const uint8_t*, size_t, TOX_ERR_FILE_SEND_CHUNK*)>(__FUNCTION__)(tox, friend_number, file_number, position, data, length, error);
}

void tox_callback_file_chunk_request(Tox *tox, tox_file_chunk_request_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_file_chunk_request_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_file_recv(Tox *tox, tox_file_recv_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_file_recv_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_file_recv_control(Tox *tox, tox_file_recv_control_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_file_recv_control_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

void tox_callback_file_recv_chunk(Tox *tox, tox_file_recv_chunk_cb *function, void *user_data)
{
	CreateFunction<void(*)(Tox*, tox_file_recv_chunk_cb, void*)>(__FUNCTION__)(tox, function, user_data);
}

bool tox_file_control(Tox *tox, uint32_t friend_number, uint32_t file_number, TOX_FILE_CONTROL control, TOX_ERR_FILE_CONTROL *error)
{
	return CreateFunction<bool(*)(Tox*, uint32_t, uint32_t, TOX_FILE_CONTROL, TOX_ERR_FILE_CONTROL*)>(__FUNCTION__)(tox, friend_number, file_number, control, error);
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