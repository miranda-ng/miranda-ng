#include "stdafx.h"

void CTelegramProto::InitNetwork()
{
	tgl_set_net_methods(TLS, &tgl_conn_methods);
}