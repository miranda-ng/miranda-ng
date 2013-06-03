/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-12 Miranda IM, 2012-13 Miranda NG project,
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

MIR_CORE_DLL(char*) replaceStr( char* &dest, const char *src )
{
	if ( dest != NULL )
		mir_free( dest );

	return dest = ( src != NULL ) ? mir_strdup( src ) : NULL;
}

MIR_CORE_DLL(WCHAR*) replaceStrW( WCHAR* &dest, const WCHAR *src )
{
	if ( dest != NULL )
		mir_free( dest );

	return dest = ( src != NULL ) ? mir_wstrdup( src ) : NULL;
}

MIR_CORE_DLL(char*) rtrim(char* str)
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

MIR_CORE_DLL(char*) ltrim(char* str)
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

MIR_CORE_DLL(int) wildcmp(const char *name, const char *mask)
{
	const char *last='\0';
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
	const WCHAR* last = '\0';
	for (;; mask++, name++) {
		if (*mask != '?' && *mask != *name) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while(*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && *mask != *name) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

#define _qtoupper(_c) (((_c) >= 'a' && (_c) <= 'z')?((_c)-('a'+'A')):(_c))

MIR_CORE_DLL(int) wildcmpi(const char *name, const char *mask)
{
	const char *last = NULL;
	for (;; mask++, name++) {
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while(*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && _qtoupper(*mask)  != _qtoupper(*name)) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

MIR_CORE_DLL(int) wildcmpiw(const WCHAR *name, const WCHAR *mask)
{
	const WCHAR* last = NULL;
	for (;; mask++, name++) {
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while(*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && _qtoupper(*mask)  != _qtoupper(*name)) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(char*) mir_base64_encode(const BYTE *input, unsigned inputLen)
{
	if (input == NULL)
		return NULL;

	size_t i = 0;
	char chr[3], enc[4];
	static char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	size_t length = inputLen;
	size_t nLength = 4 * ((length + 2) / 3) + 1;

	char *output = (char *)mir_alloc(nLength);
	char *p = output;

	while (i < length)
	{
		chr[0] = input[i++];
		chr[1] = input[i++];
		chr[2] = input[i++];

		enc[0] = chr[0] >> 2;
		enc[1] = ((chr[0] & 0x03) << 4) | (chr[1] >> 4);
		enc[2] = ((chr[1] & 0x0F) << 2) | (chr[2] >> 6);
		enc[3] = chr[2] & 0x3F;

		*p++ = cb64[enc[0]]; *p++ = cb64[enc[1]];

		if (i - 2 >= length) { *p++ = '='; *p++ = '='; }
		else if (i - 1 >= length) { *p++ = cb64[enc[2]]; *p++ = '='; }
		else { *p++ = cb64[enc[2]]; *p++ = cb64[enc[3]]; }
	}

	*p = 0;

	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int Base64DecodeTable[] =
{
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

MIR_CORE_DLL(void*) mir_base64_decode(const char *input, unsigned *outputLen)
{
	if (input == NULL)
		return NULL;

	size_t i = 0;
	char chr[3], enc[4];

	size_t length = strlen(input);
	size_t nLength = (length / 4) * 3 + 1;

	char *output = (char *)mir_alloc(nLength);
	char *p = output;

	while (i < length)
	{
		enc[0] = Base64DecodeTable[input[i++]];
		enc[1] = Base64DecodeTable[input[i++]];
		enc[2] = Base64DecodeTable[input[i++]];
		enc[3] = Base64DecodeTable[input[i++]];

		if (enc[0] == -1 || enc[1] == -1) break;

		chr[0] = (enc[0] << 2) | (enc[1] >> 4);
		chr[1] = ((enc[1] & 15) << 4) | (enc[2] >> 2);
		chr[2] = ((enc[2] & 3) << 6) | enc[3];

		*p++ = chr[0];

		if (enc[2] != -1) *p++ = chr[1];
		if (enc[3] != -1) *p++ = chr[2];
	}

	*p = 0;

	if (outputLen != NULL)
		*outputLen = (unsigned)nLength;

	return output;
}
