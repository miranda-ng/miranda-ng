#include "stdafx.h"

char * __cdecl strstri(char *a, const char *b)
{
	char * x, *y;
	if (!a || !b) return FALSE;
	x = _strdup(a);
	y = _strdup(b);
	x = _strupr(x);
	y = _strupr(y);
	char * pos = strstr(x, y);
	if (pos)
	{
		char * retval = a + (pos - x);
		free(x);
		free(y);
		return retval;
	}
	free(x);
	free(y);
	return nullptr;
}

void TRACE_ERROR()
{
	uint32_t t = GetLastError();
	LPVOID lpMsgBuf;
	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		t,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		nullptr))
	{
		// Handle the error.
		return;
	}
#ifdef _DEBUG
	MessageBox(nullptr, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
	DebugBreak();
#endif
	LocalFree(lpMsgBuf);
}

// load small icon (not shared) it IS NEED to be destroyed
HICON LoadSmallIcon(HINSTANCE hInstance, int index)
{
	wchar_t filename[MAX_PATH] = { 0 };
	GetModuleFileName(hInstance, filename, MAX_PATH);

	HICON hIcon = nullptr;
	ExtractIconEx(filename, index, nullptr, &hIcon, 1);
	return hIcon;
}

BOOL DestroyIcon_protect(HICON icon)
{
	if (icon) return DestroyIcon(icon);
	return FALSE;
}
