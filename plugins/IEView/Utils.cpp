/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "Utils.h"
#include <ctype.h>
#include <m_netlib.h>
wchar_t Utils::base_dir[MAX_PATH];
unsigned Utils::hookNum = 0;
unsigned Utils::serviceNum = 0;
HANDLE* Utils::hHooks = NULL;
HANDLE* Utils::hServices = NULL;

const wchar_t *Utils::getBaseDir() {
	char temp[MAX_PATH];
	base_dir[0] = '\0';
	long tlen = CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)"miranda32.exe", (LPARAM)temp);
	if (tlen) {
		temp[tlen - 13]=0;
		MultiByteToWideChar(CP_ACP, 0, temp, (int)strlen(temp), base_dir, MAX_PATH);
	}
	return base_dir;
}

wchar_t* Utils::toAbsolute(wchar_t* relative) {
	const wchar_t* bdir = getBaseDir();
	long len = (int)wcslen(bdir);
	long tlen = len + (int)wcslen(relative);
	wchar_t* result = new wchar_t[tlen + 1];
	if(result){
		wcscpy(result,bdir);
		wcscpy(result + len,relative);
	}
	return result;
}

static int countNoWhitespace(const wchar_t *str) {
	int c;
	for (c=0; *str!='\n' && *str!='\r' && *str!='\t' && *str!=' ' && *str!='\0'; str++, c++);
	return c;
}

void Utils::appendText(char **str, int *sizeAlloced, const char *fmt, ...) {
	va_list vararg;
	char *p;
	int size, len;

	if (str == NULL) return;

	if (*str==NULL || *sizeAlloced<=0) {
		*sizeAlloced = size = 2048;
		*str = (char *) malloc(size);
		len = 0;
	}
	else {
		len = (int)strlen(*str);
		size = *sizeAlloced - len;
	}

	if (size < 128) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (char *) realloc(*str, *sizeAlloced);
	}
	p = *str + len;
	va_start(vararg, fmt);
	while (_vsnprintf(p, size  - 1, fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (char *) realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	p[ size - 1 ] = '\0';
	va_end(vararg);
}

void Utils::appendText(wchar_t **str, int *sizeAlloced, const wchar_t *fmt, ...) {
	va_list vararg;
	wchar_t *p;
	int size, len;

	if (str == NULL) return;

	if (*str==NULL || *sizeAlloced<=0) {
		*sizeAlloced = size = 2048;
		*str = (wchar_t *) malloc(size);
		len = 0;
	}
	else {
		len = (int)wcslen(*str);
		size = *sizeAlloced - sizeof(wchar_t) * len;
	}

	if (size < 128) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (wchar_t *) realloc(*str, *sizeAlloced);
	}
	p = *str + len;
	va_start(vararg, fmt);
	while (_vsnwprintf(p, size / sizeof(wchar_t) - 1, fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (wchar_t *) realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	p[ size / sizeof(wchar_t) - 1] = '\0';
	va_end(vararg);
}

char *Utils::dupString(const char *a) {
	if (a!=NULL) {
		char *b = new char[strlen(a)+1];
		strcpy(b, a);
		return b;
	}
	return NULL;
}

char *Utils::dupString(const char *a, int l) {
	if (a!=NULL) {
		char *b = new char[l+1];
		strncpy(b, a, l);
		b[l] ='\0';
		return b;
	}
	return NULL;
}

wchar_t *Utils::dupString(const wchar_t *a) {
	if (a!=NULL) {
		wchar_t *b = new wchar_t[wcslen(a)+1];
		wcscpy(b, a);
		return b;
	}
	return NULL;
}

wchar_t *Utils::dupString(const wchar_t *a, int l) {
	if (a!=NULL) {
		wchar_t *b = new wchar_t[l+1];
		wcsncpy(b, a, l);
		b[l] ='\0';
		return b;
	}
	return NULL;
}


wchar_t *Utils::convertToWCS(const char *a) {
	if (a!=NULL) {
		int len = (int)strlen(a)+1;
		wchar_t *b = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, a, len, b, len);
		return b;
	}
	return NULL;
}

wchar_t *Utils::convertToWCS(const char *a, int cp) {
	if (a!=NULL) {
		int len = (int)strlen(a)+1;
		wchar_t *b = new wchar_t[len];
		MultiByteToWideChar(cp, 0, a, len, b, len);
		return b;
	}
	return NULL;
}

char *Utils::convertToString(const wchar_t *a) {
	if (a!=NULL) {
		int len = (int)wcslen(a)+1;
		char *b = new char[len];
		WideCharToMultiByte(CP_ACP, 0, a, len, b, len, NULL, FALSE);
		return b;
	}
	return NULL;
}


char *Utils::convertToString(const wchar_t *a, int cp) {
	if (a!=NULL) {
		int len = (int)wcslen(a)+1;
		char *b = new char[len];
		WideCharToMultiByte(cp, 0, a, len, b, len, NULL, FALSE);
		return b;
	}
	return NULL;
}

void Utils::convertPath(char *path) {
	if (path != NULL) {
		for (; *path!='\0'; path++) {
			if (*path == '\\') *path = '/';
		}
	}
}

void Utils::convertPath(wchar_t *path) {
	if (path != NULL) {
		for (; *path!='\0'; path++) {
			if (*path == '\\') *path = '/';
		}
	}
}

DWORD Utils::safe_wcslen(wchar_t *msg, DWORD maxLen) {
	DWORD i;
	for (i = 0; i < maxLen; i++) {
		if (msg[i] == (wchar_t)0)
			return i;
	}
	return 0;
}

char * Utils::UTF8Encode(const wchar_t *wtext) {
	unsigned char *szOut;
	int len, i;
	const wchar_t *w;

	if (wtext == NULL) return NULL;
	for (len=0, w=wtext; *w; w++) {
		if (*w < 0x0080) len++;
		else if (*w < 0x0800) len += 2;
		else len += 3;
	}
	szOut = new unsigned char [len+1];
	if (szOut == NULL) return NULL;

	for (i=0, w=wtext; *w; w++) {
		if (*w < 0x0080)
			szOut[i++] = (unsigned char) *w;
		else if (*w < 0x0800) {
			szOut[i++] = 0xc0 | ((*w) >> 6);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
		else {
			szOut[i++] = 0xe0 | ((*w) >> 12);
			szOut[i++] = 0x80 | (((*w) >> 6) & 0x3f);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
	}
	szOut[i] = '\0';
	return (char *) szOut;
}

char *Utils::UTF8Encode(const char *text) {
	wchar_t *wtext = Utils::convertToWCS(text);
	char *atext = UTF8Encode(wtext);
	delete wtext;
	return atext;
}

void Utils::UTF8Encode(const char *text, char *output, int maxLen) {
	wchar_t *wtext = Utils::convertToWCS(text);
	char *atext = UTF8Encode(wtext);
	int slen = (int)strlen(atext) + 1;
	memcpy(output, atext, slen > maxLen ? maxLen : slen);
	output[maxLen - 1] = '\0';
	delete atext;
	delete wtext;
}

void Utils::UTF8Encode(const wchar_t *wtext, char *output, int maxLen) {
	char *atext = UTF8Encode(wtext);
	int slen = (int)strlen(atext) + 1;
	memcpy(output, atext, slen > maxLen ? maxLen : slen);
	output[maxLen - 1] = '\0';
	delete atext;
}

int Utils::detectURL(const wchar_t *text) {
	int i;
	for (i=0;text[i]!='\0';i++) {
		if (!((text[i] >= '0' && text[i]<='9') || iswalpha(text[i]))) {
			break;
		}
	}
	if (i > 0 && text[i]==':' && text[i+1]=='/' && text[i+2]=='/') {
		i += countNoWhitespace(text+i);
		for (; i > 0; i --) {
			if ((text[i-1] >= '0' && text[i-1]<='9') || iswalpha(text[i-1]) || text[i-1]=='/') {
				break;
			}
		}
		return i;
	}
	return 0;
}

char *Utils::escapeString(const char *a) {
	int i, l, len;
	char *out;
	if (a == NULL) {
		return NULL;
	}
	len = (int)strlen(a);
	for (i = l = 0; i < len; i++, l++) {
		if (a[i] == '\\' || a[i] == '\n' || a[i] == '\r' || a[i] == '\"'
			|| a[i] == '\'' || a[i] == '\b' || a[i] == '\t' || a[i] == '\f') {
			l++;
		}
	}
	out = new char[l+1];
	for (i = l = 0; i < len; i++, l++) {
		if (a[i] == '\\' || a[i] == '\n' || a[i] == '\r' || a[i] == '\"'
			|| a[i] == '\'' || a[i] == '\b' || a[i] == '\t' || a[i] == '\f') {
			out[l++] = '\\';
		}
		out[l] = a[i];
	}
	out[l] = '\0';
	return out;
}

struct FORK_ARG {
	HANDLE hEvent;
	void (__cdecl *threadcode)(void*);
	void *arg;
};

static void __cdecl forkthread_r(struct FORK_ARG *fa)
{
	void (*callercode)(void*) = fa->threadcode;
	void *arg = fa->arg;
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	SetEvent(fa->hEvent);
	callercode(arg);
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return;
}

unsigned long Utils::forkThread(void (__cdecl *threadcode)(void*), unsigned long stacksize,	void *arg) {

	unsigned long rc;
	struct FORK_ARG fa;

	fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	fa.threadcode = threadcode;
	fa.arg = arg;
	rc = _beginthread((void (__cdecl *)(void*))forkthread_r, stacksize, &fa);
	if ((unsigned long) -1L != rc) {
		WaitForSingleObject(fa.hEvent, INFINITE);
	}
	CloseHandle(fa.hEvent);
	return rc;
}

HANDLE Utils::hookEvent_Ex(const char *name, MIRANDAHOOK hook) {
	hookNum ++;
	hHooks = (HANDLE *) mir_realloc(hHooks, sizeof(HANDLE) * (hookNum));
	hHooks[hookNum - 1] = HookEvent(name, hook);
	return hHooks[hookNum - 1] ;
}

HANDLE Utils::createServiceFunction_Ex(const char *name, MIRANDASERVICE service) {
	serviceNum++;
	hServices = (HANDLE *) mir_realloc(hServices, sizeof(HANDLE) * (serviceNum));
	hServices[serviceNum - 1] = CreateServiceFunction(name, service);
	return hServices[serviceNum - 1] ;
}

void Utils::unhookEvents_Ex() {
	unsigned i;
	for (i=0; i<hookNum ; ++i) {
		if (hHooks[i] != NULL) {
			UnhookEvent(hHooks[i]);
		}
	}
	mir_free(hHooks);
	hookNum = 0;
	hHooks = NULL;
}

void Utils::destroyServices_Ex() {
	unsigned i;
	for (i=0; i<serviceNum; ++i) {
		if (hServices[i] != NULL) {
			DestroyServiceFunction(hServices[i]);
		}
	}
	mir_free(hServices);
	serviceNum = 0;
	hServices = NULL;
}

wchar_t *Utils::urlEncode(const wchar_t *text) {
	char *utf8 = UTF8Encode(text);
	wchar_t *result = urlEncode(utf8);
	delete utf8;
	return result;
}

wchar_t *Utils::urlEncode(const char *text) {
	char *pszReturnString = (char *)CallService(MS_NETLIB_URLENCODE, (WPARAM)0, (LPARAM)text);
	wchar_t *result = convertToWCS(pszReturnString);
	HeapFree(GetProcessHeap(), 0, pszReturnString);
	return result;
}


void Utils::appendIcon(char **str, int *sizeAlloced, const char *iconFile) {
	Utils::appendText(str, sizeAlloced, "<img class=\"img\" src=\"file://%s/plugins/ieview/%s\"/> ", workingDirUtf8, iconFile);
}
