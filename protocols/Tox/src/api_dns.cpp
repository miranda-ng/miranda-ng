#include "stdafx.h"

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