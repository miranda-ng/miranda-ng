#include "common.h"

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