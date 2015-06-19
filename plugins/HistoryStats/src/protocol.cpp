#include "stdafx.h"
#include "protocol.h"

#include "utils.h"

ext::string Protocol::getDisplayName(const ext::a::string& protocol)
{
	PROTOACCOUNT *pa = Proto_GetAccount(protocol.c_str());
	return (pa == NULL) ? utils::fromA(protocol) : pa->tszAccountName;
}
