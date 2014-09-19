#include "common.h"

std::string getLastErrorMsg()
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	DWORD dw = WSAGetLastError();

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL);

	std::string ret((LPSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return ret;
}

BYTE* utils::md5string(const BYTE *data, int size, BYTE *digest)
{
	mir_md5_state_t md5_state;
	mir_md5_init(&md5_state);
	mir_md5_append(&md5_state, data, size);
	mir_md5_finish(&md5_state, digest);
	return digest;
}

/////////////////////////////////////////////////////////////////////////////////////////
// external stubs for WhatsAPI++

std::string base64_encode(void* pData, size_t len)
{
	return (char*)ptrA(mir_base64_encode((BYTE*)pData, (unsigned)len));
}

void md5_string(const std::string &data, BYTE digest[16])
{
	utils::md5string(data, digest);
}
