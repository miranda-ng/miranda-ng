#include "commonheaders.h"

string base64encode(const string& buf)
{
	return (char*)ptrA(mir_base64_encode(buf.data(), buf.length()));
}

string base64decode(const string& buf)
{
	size_t len;
	ptrA plain((char*)mir_base64_decode(buf.data(), &len));
	return (plain == NULL) ? string() : string(plain, len);
}

string base64decode(const char *buf)
{
	size_t len;
	ptrA plain((char*)mir_base64_decode(buf, &len));
	return (plain == NULL) ? string() : string(plain, len);
}
