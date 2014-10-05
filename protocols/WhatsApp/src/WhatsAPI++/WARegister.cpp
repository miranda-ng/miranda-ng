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

static char WaKey[] = "/UIGKU1FVQa+ATM2A0za7G2KI9S/CwPYjgAbc67v7ep42eO/WeTLx1lb1cHwxpsEgF4+PmYpLd2YpGUdX/A2JQitsHzDwgcdBpUf7psX1BU=";
static char WaSignature[] = "MIIDMjCCAvCgAwIBAgIETCU2pDALBgcqhkjOOAQDBQAwfDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWExFDASBgNVBAcTC1NhbnRhIENsYXJhMRYwFAYDVQQKEw1XaGF0c0FwcCBJbmMuMRQwEgYDVQQLEwtFbmdpbmVlcmluZzEUMBIGA1UEAxMLQnJpYW4gQWN0b24wHhcNMTAwNjI1MjMwNzE2WhcNNDQwMjE1MjMwNzE2WjB8MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTEUMBIGA1UEBxMLU2FudGEgQ2xhcmExFjAUBgNVBAoTDVdoYXRzQXBwIEluYy4xFDASBgNVBAsTC0VuZ2luZWVyaW5nMRQwEgYDVQQDEwtCcmlhbiBBY3RvbjCCAbgwggEsBgcqhkjOOAQBMIIBHwKBgQD9f1OBHXUSKVLfSpwu7OTn9hG3UjzvRADDHj+AtlEmaUVdQCJR+1k9jVj6v8X1ujD2y5tVbNeBO4AdNG/yZmC3a5lQpaSfn+gEexAiwk+7qdf+t8Yb+DtX58aophUPBPuD9tPFHsMCNVQTWhaRMvZ1864rYdcq7/IiAxmd0UgBxwIVAJdgUI8VIwvMspK5gqLrhAvwWBz1AoGBAPfhoIXWmz3ey7yrXDa4V7l5lK+7+jrqgvlXTAs9B4JnUVlXjrrUWU/mcQcQgYC0SRZxI+hMKBYTt88JMozIpuE8FnqLVHyNKOCjrh4rs6Z1kW6jfwv6ITVi8ftiegEkO8yk8b6oUZCJqIPf4VrlnwaSi2ZegHtVJWQBTDv+z0kqA4GFAAKBgQDRGYtLgWh7zyRtQainJfCpiaUbzjJuhMgo4fVWZIvXHaSHBU1t5w//S0lDK2hiqkj8KpMWGywVov9eZxZy37V26dEqr/c2m5qZ0E+ynSu7sqUD7kGx/zeIcGT0H+KAVgkGNQCo5Uc0koLRWYHNtYoIvt5R3X6YZylbPftF/8ayWTALBgcqhkjOOAQDBQADLwAwLAIUAKYCp0d6z4QQdyN74JDfQ2WCyi8CFDUM4CaNB+ceVXdKtOrNTQcc0e+t";
static char WaClassesMd5[] = "xOyKd7AoN0uoos7Fkeup5w=="; // 2.11.407

std::string WAToken::GenerateToken(const string &number)
{
	unsigned int keyLen, dataLen, classesLen;
	mir_ptr<BYTE> key((BYTE*)mir_base64_decode(WaKey, &keyLen));
	mir_ptr<BYTE> data((BYTE*)mir_base64_decode(WaSignature, &dataLen));
	mir_ptr<BYTE> classes((BYTE*)mir_base64_decode(WaClassesMd5, &classesLen));

	BYTE opad[64], ipad[64];
	memset(opad, 0x5C, sizeof(opad));
	memset(ipad, 0x36, sizeof(ipad));
	for (int i = 0; i < sizeof(opad); i++) {
		opad[i] = (BYTE)(opad[i] ^ key[i]);
		ipad[i] = (BYTE)(ipad[i] ^ key[i]);
	}

	BYTE hash1[MIR_SHA1_HASH_SIZE], hash2[MIR_SHA1_HASH_SIZE];
	mir_sha1_ctx ctx;
	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, ipad, sizeof(ipad));
	mir_sha1_append(&ctx, data, dataLen);
	mir_sha1_append(&ctx, classes, classesLen);
	mir_sha1_append(&ctx, (PBYTE)number.c_str(), (int)number.size());
	mir_sha1_finish(&ctx, hash1);

	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, opad, sizeof(opad));
	mir_sha1_append(&ctx, hash1, sizeof(hash1));
	mir_sha1_finish(&ctx, hash2);

	ptrA result(mir_urlEncode(ptrA(mir_base64_encode(hash2, sizeof(hash2)))));
	return std::string(result);
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
