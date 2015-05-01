/*
 * WARegister.cpp
 *
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "WARegister.h"
#include "PhoneNumber.h"

using namespace Utilities;

/////////////////////////////////////////////////////////////////////////////////////////
// Token generation

static char WaToken[] = "PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk1425519315543";

std::string WAToken::GenerateToken(const string &number)
{
	uint8_t digest[16];
	md5_string(WaToken + number, digest);

	char dest[33];
	bin2hex(digest, sizeof(digest), dest);
	return dest;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account registration

CMStringA WARegister::RequestCodeUrl(const std::string &phoneNumber, const std::string &code)
{
	try {
		std::string id = GenerateIdentity(phoneNumber);

		PhoneNumber pn(phoneNumber);
		std::string token = WAToken::GenerateToken(pn.Number);

		const char *n = pn.Number.c_str();

		if (!code.empty())
			return CMStringA(FORMAT, "https://v.whatsapp.net/v2/register?cc=%d&in=%s&id=%s&code=%s", pn.countryCode, n, id.c_str(), code.c_str());

		return CMStringA(FORMAT, "https://v.whatsapp.net/v2/code?cc=%d&in=%s&to=%d%s&method=sms&mcc=%03d&mnc=%03d&token=%s&id=%s&lg=%s&lc=%s",
							 pn.countryCode, n, pn.countryCode, n, pn.mcc, pn.mnc, token.c_str(), id.c_str(), pn.ISO639, pn.ISO3166);
	}
	catch (...)
	{}
	
	return CMStringA();
}

std::string WARegister::GenerateIdentity(const std::string &phone)
{
	std::string id = phone;
	std::reverse(id.begin(), id.end());

	BYTE hash[MIR_SHA1_HASH_SIZE];
	mir_sha1_hash((PBYTE)id.c_str(), (int)id.length(), hash);
	
	id.clear();
	for (int i = 0; i < sizeof(hash); i++) {
		char buf[10];
		sprintf_s(buf, "%%%02x", hash[i]);
		id += buf;
	}

	return id;
}
