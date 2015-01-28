#include "common.h"

LONG WhatsAppProto::GetSerial()
{
	return ::_InterlockedIncrement(&m_iSerial);
}

std::string getLastErrorMsg()
{
	LPVOID lpMsgBuf;
	DWORD dw = WSAGetLastError(); // retrieve the system error message for the last-error code

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

void utils::setStatusMessage(MCONTACT hContact, const TCHAR *ptszMessage)
{
	if (ptszMessage != NULL) {
		StatusTextData st = { 0 };
		st.cbSize = sizeof(st);
		st.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		_tcsncpy_s(st.tszText, ptszMessage, _TRUNCATE);
		CallService(MS_MSG_SETSTATUSTEXT, hContact, (LPARAM)&st);
	}
	else CallService(MS_MSG_SETSTATUSTEXT, hContact, NULL);
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
