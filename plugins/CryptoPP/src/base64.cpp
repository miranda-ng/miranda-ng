#include "commonheaders.h"

string base64encode(const string& buf)
{
	return (char*)ptrA(mir_base64_encode((PBYTE)buf.data(), (unsigned)buf.length()));
}

string base64decode(const string& buf)
{
	unsigned len;
	char *plain = (char*)mir_base64_decode(buf.data(), &len);
	return (plain == NULL) ? string() : string(plain, len);
}

string base64decode(const char *buf)
{
	unsigned len;
	char *plain = (char*)mir_base64_decode(buf, &len);
	return (plain == NULL) ? string() : string(plain, len);
}
