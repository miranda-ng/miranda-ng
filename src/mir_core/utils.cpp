/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

MIR_CORE_DLL(char*) replaceStr(char* &dest, const char *src)
{
	if (dest != NULL)
		mir_free(dest);

	return dest = (src != NULL) ? mir_strdup(src) : NULL;
}

MIR_CORE_DLL(WCHAR*) replaceStrW(WCHAR* &dest, const WCHAR *src)
{
	if (dest != NULL)
		mir_free(dest);

	return dest = (src != NULL) ? mir_wstrdup(src) : NULL;
}

MIR_CORE_DLL(char*) rtrim(char *str)
{
	if (str == NULL)
		return NULL;

	char* p = strchr(str, 0);
	while (--p >= str) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}

MIR_CORE_DLL(WCHAR*) rtrimw(WCHAR *str)
{
	if (str == NULL)
		return NULL;

	WCHAR *p = _tcschr(str, 0);
	while (--p >= str) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}

MIR_CORE_DLL(char*) ltrim(char *str)
{
	if (str == NULL)
		return NULL;

	char* p = str;
	for (;;) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			++p; break;
		default:
			memmove(str, p, strlen(p) + 1);
			return str;
		}
	}
}

MIR_CORE_DLL(WCHAR*) ltrimw(WCHAR *str)
{
	if (str == NULL)
		return NULL;

	WCHAR *p = str;
	for (;;) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			++p; break;
		default:
			memmove(str, p, sizeof(WCHAR)*(wcslen(p) + 1));
			return str;
		}
	}
}

MIR_CORE_DLL(char*) ltrimp(char *str)
{
	if (str == NULL)
		return NULL;

	char *p = str;
	for (;;) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			++p; break;
		default:
			return p;
		}
	}
}

MIR_CORE_DLL(WCHAR*) ltrimpw(WCHAR *str)
{
	if (str == NULL)
		return NULL;

	WCHAR *p = str;
	for (;;) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			++p; break;
		default:
			return p;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) wildcmp(const char *name, const char *mask)
{
	if (name == NULL || mask == NULL)
		return false;

	const char *last = '\0';
	for (;; mask++, name++) {
		if (*mask != '?' && *mask != *name) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while (*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && *mask != *name) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

MIR_CORE_DLL(int) wildcmpw(const WCHAR *name, const WCHAR *mask)
{
	if (name == NULL || mask == NULL)
		return false;

	const WCHAR* last = '\0';
	for (;; mask++, name++) {
		if (*mask != '?' && *mask != *name) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while (*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && *mask != *name) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

#define _qtoupper(_c) (((_c) >= 'a' && (_c) <= 'z')?((_c)-'a'+'A'):(_c))

MIR_CORE_DLL(int) wildcmpi(const char *name, const char *mask)
{
	if (name == NULL || mask == NULL)
		return false;

	const char *last = NULL;
	for (;; mask++, name++) {
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while (*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

MIR_CORE_DLL(int) wildcmpiw(const WCHAR *name, const WCHAR *mask)
{
	if (name == NULL || mask == NULL)
		return false;

	const WCHAR* last = NULL;
	for (;; mask++, name++) {
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while (*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static char szHexTable[] = "0123456789abcdef";

MIR_CORE_DLL(char*) bin2hex(const void *pData, size_t len, char *dest)
{
	const BYTE *p = (const BYTE*)pData;
	char *d = dest;

	for (size_t i = 0; i < len; i++, p++) {
		*d++ = szHexTable[*p >> 4];
		*d++ = szHexTable[*p & 0x0F];
	}
	*d = 0;

	return dest;
}

MIR_CORE_DLL(WCHAR*) bin2hexW(const void *pData, size_t len, WCHAR *dest)
{
	const BYTE *p = (const BYTE*)pData;
	WCHAR *d = dest;

	for (size_t i = 0; i < len; i++, p++) {
		*d++ = szHexTable[*p >> 4];
		*d++ = szHexTable[*p & 0x0F];
	}
	*d = 0;

	return dest;
}

/////////////////////////////////////////////////////////////////////////////////////////

#pragma intrinsic(strlen, strcpy, strcat, wcslen, wcscpy, wcscat)

MIR_CORE_DLL(size_t) mir_strlen(const char *p)
{
	return (p) ? strlen(p) : 0;
}

MIR_CORE_DLL(size_t) mir_wstrlen(const wchar_t *p)
{
	return (p) ? wcslen(p) : 0;
}

MIR_CORE_DLL(char*) mir_strcpy(char *dest, const char *src)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return strcpy(dest, src);
}

MIR_CORE_DLL(wchar_t*) mir_wstrcpy(wchar_t *dest, const wchar_t *src)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return wcscpy(dest, src);
}

MIR_CORE_DLL(char*) mir_strncpy(char *dest, const char *src, size_t len)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return strncpy(dest, src, len);
}

MIR_CORE_DLL(wchar_t*) mir_wstrncpy(wchar_t *dest, const wchar_t *src, size_t len)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return wcsncpy(dest, src, len);
}

MIR_CORE_DLL(char*) mir_strcat(char *dest, const char *src)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return strcat(dest, src);
}

MIR_CORE_DLL(wchar_t*) mir_wstrcat(wchar_t *dest, const wchar_t *src)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return wcscat(dest, src);
}

MIR_CORE_DLL(char*) mir_strncat(char *dest, const char *src, size_t len)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return strncat(dest, src, len);
}

MIR_CORE_DLL(wchar_t*) mir_wstrncat(wchar_t *dest, const wchar_t *src, size_t len)
{
	if (dest == NULL)
		return NULL;

	if (src == NULL) {
		*dest = 0;
		return dest;
	}

	return wcsncat(dest, src, len);
}
