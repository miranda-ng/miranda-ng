#include "stdafx.h"

wchar_t* utils::removeA(wchar_t *str)
{
	if (str == NULL)
		return NULL;

	wchar_t *p = wcschr(str, '@');
	if (p) *p = 0;
	return str;
}

void utils::copyText(HWND hwnd, const wchar_t *text)
{
	if (!hwnd || !text) return;

	if (!OpenClipboard(hwnd))
		return;

	EmptyClipboard();
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(mir_wstrlen(text) + 1));
	mir_wstrcpy((wchar_t*)GlobalLock(hMem), text);
	GlobalUnlock(hMem);
	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();
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

void utils::setStatusMessage(MCONTACT hContact, const wchar_t *ptszMessage)
{
	if (ptszMessage != nullptr)
		Srmm_SetStatusText(hContact, ptszMessage, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
	else
		Srmm_SetStatusText(hContact, nullptr);
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

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		if (item.length() > 0) {
			elems.push_back(item);
		}
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}
