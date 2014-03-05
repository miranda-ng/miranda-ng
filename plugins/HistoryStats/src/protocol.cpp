#include "_globals.h"
#include "protocol.h"

#include "utils.h"

ext::string Protocol::getDisplayName(const ext::a::string& protocol)
{
	PROTOACCOUNT *pa = ProtoGetAccount(protocol.c_str());
	return (pa == NULL) ? utils::fromA(protocol) : pa->tszAccountName;
}
