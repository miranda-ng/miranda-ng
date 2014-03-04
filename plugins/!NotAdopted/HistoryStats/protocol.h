#if !defined(HISTORYSTATS_GUARD_PROTOCOL_H)
#define HISTORYSTATS_GUARD_PROTOCOL_H

#include "_globals.h"
#include "_consts.h"

#include <set>

class Protocol
{
public:
	static ext::string getDisplayName(const ext::a::string& protocol);

public:
	ext::string displayName;
};

#endif // HISTORYSTATS_GUARD_PROTOCOL_H