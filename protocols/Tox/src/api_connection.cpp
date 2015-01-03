#include "common.h"

/* CONNECTION FUNCTIONS */

int tox_bootstrap_from_address(Tox *tox, const char *address, uint16_t port, const uint8_t *public_key)
{
	return CreateFunction<int(*)(Tox*, const char*, uint16_t, const uint8_t*)>(__FUNCTION__)(tox, address, port, public_key);
}

int tox_add_tcp_relay(Tox *tox, const char *address, uint16_t port, const uint8_t *public_key)
{
	return CreateFunction<int(*)(Tox*, const char*, uint16_t, const uint8_t*)>(__FUNCTION__)(tox, address, port, public_key);
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