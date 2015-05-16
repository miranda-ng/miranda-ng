#include "hdr/modern_commonheaders.h"

BYTE gl_TrimText = 1;

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
	return NULL;
}

BOOL __cdecl mir_bool_strcmpi(const char *a, const char *b)
{
	if (a == NULL && b == NULL) return 1;
	if (a == NULL || b == NULL) return _stricmp(a ? a : "", b ? b : "") == 0;
	return _stricmp(a, b) == 0;
}

BOOL __cdecl mir_bool_tstrcmpi(const TCHAR *a, const TCHAR *b)
{
	if (a == NULL && b == NULL) return 1;
	if (a == NULL || b == NULL) return _tcsicmp(a ? a : _T(""), b ? b : _T("")) == 0;
	return _tcsicmp(a, b) == 0;
}

#ifdef strlen
#undef mir_strcmp
#undef strlen
#endif

//copy len symbols from string - do not check is it null terminated or len is more then actual
char * strdupn(const char * src, int len)
{
	char * p;
	if (src == NULL) return NULL;
	p = (char*)malloc(len + 1);
	if (!p) return 0;
	memcpy(p, src, len);
	p[len] = '\0';
	return p;
}

DWORD exceptFunction(LPEXCEPTION_POINTERS EP)
{
	char buf[4096];
	mir_snprintf(buf, SIZEOF(buf), "\r\nExceptCode: %x\r\nExceptFlags: %x\r\nExceptAddress: %p\r\n",
		EP->ExceptionRecord->ExceptionCode,
		EP->ExceptionRecord->ExceptionFlags,
		EP->ExceptionRecord->ExceptionAddress);

	TRACE(buf);
	MessageBoxA(0, buf, "clist_mw Exception", 0);
	return EXCEPTION_EXECUTE_HANDLER;
}

#ifdef _DEBUG
#undef DeleteObject
#endif

void TRACE_ERROR()
{
	DWORD t = GetLastError();
	LPVOID lpMsgBuf;
	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		t,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL))
	{
		// Handle the error.
		return;
	}
#ifdef _DEBUG
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION);
	DebugBreak();
#endif
	LocalFree(lpMsgBuf);
}

BOOL DebugDeleteObject(HGDIOBJ a)
{
	BOOL res = DeleteObject(a);
	if (!res) TRACE_ERROR();
	return res;
}

#ifdef _DEBUG
#define DeleteObject(a) DebugDeleteObject(a)
#endif

// load small icon (not shared) it IS NEED to be destroyed
HICON LoadSmallIcon(HINSTANCE hInstance, int index)
{
	TCHAR filename[MAX_PATH] = { 0 };
	GetModuleFileName(hInstance, filename, MAX_PATH);

	HICON hIcon = NULL;
	ExtractIconEx(filename, index, NULL, &hIcon, 1);
	return hIcon;
}

BOOL DestroyIcon_protect(HICON icon)
{
	if (icon) return DestroyIcon(icon);
	return FALSE;
}
