#include "_globals.h"
#include "protocol.h"

#include "utils.h"

ext::string Protocol::getDisplayName(const ext::a::string& protocol)
{
	mu_text protoName[128];
	
	if (mu::protosvc::getName(protocol.c_str(), 128, protoName) == 0)
	{
		return protoName;
	}
	else
	{
		return utils::fromA(protocol);
	}
}
