#include "common.h"
#include "str_utils.h"

int code_page = CP_ACP;

void set_codepage() {
	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
}

bool a2w(const char *as, wchar_t *buff, int bufflen){
	if(as) MultiByteToWideChar(code_page, 0, as, -1, buff, bufflen);
	return true;
}

bool w2a(const wchar_t *ws, char *buff, int bufflen) {
	if(ws) WideCharToMultiByte(code_page, 0, ws, -1, buff, bufflen, 0, 0);
	return true;
}

bool t2w(const TCHAR *ts, wchar_t *buff, int bufflen) {
#ifdef _UNICODE
	wcsncpy(buff, ts, bufflen);
	return true;
#else
	return a2w(ts, buff, bufflen);
#endif
}

bool w2t(const wchar_t *ws, TCHAR *buff, int bufflen) {
#ifdef _UNICODE
	wcsncpy(buff, ws, bufflen);
	return true;
#else
	return w2a(ws, buff, bufflen);
#endif
}

bool t2a(const TCHAR *ts, char *buff, int bufflen) {
#ifdef _UNICODE
	return w2a(ts, buff, bufflen);
#else
	strncpy(buff, ts, bufflen);
	return true;
#endif
}

bool a2t(const char *as, TCHAR *buff, int bufflen) {
#ifdef _UNICODE
	return a2w(as, buff, bufflen);
#else
	strncpy(buff, as, bufflen);
	return true;
#endif
}

wchar_t *a2w(const char *as) {
	int size = MultiByteToWideChar(code_page, 0, as, -1, 0, 0);
	wchar_t *buff = (wchar_t *)malloc(size * sizeof(wchar_t));
	MultiByteToWideChar(code_page, 0, as, -1, buff, size);
	return buff;
}

char *w2a(const wchar_t *ws) {
	int size = WideCharToMultiByte(code_page, 0, ws, -1, 0, 0, 0, 0);
	char *buff = (char *)malloc(size);
	WideCharToMultiByte(code_page, 0, ws, -1, buff, size, 0, 0);
	return buff;
}

TCHAR *w2t(const wchar_t *ws) {
#ifdef _UNICODE
	return wcsdup(ws);
#else
	return w2a(ws);
#endif
}

wchar_t *t2w(const TCHAR *ts) {
#ifdef _UNICODE
	return _tcsdup(ts);
#else
	return a2w(ts);
#endif
}


char *t2a(const TCHAR *ts) {
#ifdef _UNICODE
	return w2a(ts);
#else
	return _strdup(ts);
#endif
}

TCHAR *a2t(const char *as) {
#ifdef _UNICODE
	return a2w(as);
#else
	return _strdup(as);
#endif
}
