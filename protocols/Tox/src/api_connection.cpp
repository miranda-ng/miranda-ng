#include "stdafx.h"

/* CONNECTION FUNCTIONS */

bool tox_bootstrap(Tox *tox, const char *host, uint16_t port, const uint8_t *public_key, TOX_ERR_BOOTSTRAP *error)
{
	return CreateFunction<bool(*)(Tox*, const char*, uint16_t, const uint8_t*, TOX_ERR_BOOTSTRAP*)>(__FUNCTION__)(tox, host, port, public_key, error);
}

bool tox_add_tcp_relay(Tox *tox, const char *host, uint16_t port, const uint8_t *public_key, TOX_ERR_BOOTSTRAP *error)
{
	return CreateFunction<bool(*)(Tox*, const char*, uint16_t, const uint8_t*, TOX_ERR_BOOTSTRAP*)>(__FUNCTION__)(tox, host, port, public_key, error);
}

TOX_CONNECTION tox_self_get_connection_status(const Tox *tox)
{
	return CreateFunction<TOX_CONNECTION(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint32_t tox_iteration_interval(const Tox *tox)
{
	return CreateFunction<uint32_t(*)(const Tox*)>(__FUNCTION__)(tox);
}

void tox_iterate(Tox *tox)
{
	CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}