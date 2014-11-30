// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Contains helper functions to convert text messages between different
//  character sets.
// -----------------------------------------------------------------------------

#include "icqoscar.h"

static BOOL bHasCP_UTF8 = FALSE;

void InitI18N(void)
{
	CPINFO CPInfo;
	bHasCP_UTF8 = GetCPInfo(CP_UTF8, &CPInfo);
}

// Returns true if the buffer only contains 7-bit characters.
BOOL __stdcall IsUSASCII(const char *pBuffer, size_t nSize)
{
	for (size_t nIndex = 0; nIndex < nSize; nIndex++)
		if (BYTE(pBuffer[nIndex]) > 0x7F)
			return FALSE;

	return TRUE;
}

// Returns true if the unicode buffer only contains 7-bit characters.
BOOL __stdcall IsUnicodeAscii(const WCHAR *pBuffer, size_t nSize)
{
	for (size_t nIndex = 0; nIndex < nSize; nIndex++)
		if (WORD(pBuffer[nIndex]) > 0x7F)
			return FALSE;

	return TRUE;
}


// Scans a string encoded in UTF-8 to verify that it contains
// only valid sequences. It will return 1 if the string contains
// only legitimate encoding sequences; otherwise it will return 0;
// From 'Secure Programming Cookbook', John Viega & Matt Messier, 2003
int __stdcall UTF8_IsValid(const char *pszInput)
{
	int nb;
	if (!pszInput)
		return 0;

	for (BYTE* c = (BYTE*)pszInput; *c; c += (nb + 1)) {
		if (!(*c & 0x80))
			nb = 0;
		else if ((*c & 0xc0) == 0x80) return 0;
		else if ((*c & 0xe0) == 0xc0) nb = 1;
		else if ((*c & 0xf0) == 0xe0) nb = 2;
		else if ((*c & 0xf8) == 0xf0) nb = 3;
		else if ((*c & 0xfc) == 0xf8) nb = 4;
		else if ((*c & 0xfe) == 0xfc) nb = 5;
		else nb = 0;

		for (int i = 1; i <= nb; i++) // we this forward, do not cross end of string
			if ((*(c + i) & 0xc0) != 0x80)
				return 0;
	}

	return 1;
}


int __stdcall get_utf8_size(const WCHAR *unicode)
{
	int size = 0;
	int index = 0;
	/* calculate the size of the utf-8 string */
	WCHAR c = unicode[index++];
	while (c) {
		if (c < 0x0080)
			size += 1;
		else if (c < 0x0800)
			size += 2;
		else
			size += 3;
		c = unicode[index++];
	}
	return size;
}


// returns ansi string in all cases
char* __stdcall detect_decode_utf8(const char *from)
{
	char *temp = NULL;

	if (IsUSASCII(from, mir_strlen(from)) || !UTF8_IsValid(from) || !utf8_decode(from, &temp)) return (char*)from;
	SAFE_FREE((void**)&from);

	return temp;
}


/*
* The following UTF8 routines are
*
* Copyright (C) 2001 Peter Harris <peter.harris@hummingbird.com>
* Copyright (C) 2001 Edmund Grimley Evans <edmundo@rano.org>
*
* under a GPL license
*
* --------------------------------------------------------------
* Convert a string between UTF-8 and the locale's charset.
* Invalid bytes are replaced by '#', and characters that are
* not available in the target encoding are replaced by '?'.
*
* If the locale's charset is not set explicitly then it is
* obtained using nl_langinfo(CODESET), where available, the
* environment variable CHARSET, or assumed to be US-ASCII.
*
* Return value of conversion functions:
*
*  -1 : memory allocation failed
*   0 : data was converted exactly
*   1 : valid data was converted approximately (using '?')
*   2 : input was invalid (but still converted, using '#')
*   3 : unknown encoding (but still converted, using '?')
*/



/*
* Convert a string between UTF-8 and the locale's charset.
*/
char* __stdcall make_utf8_string_static(const WCHAR *unicode, char *utf8, size_t utf_size)
{
	int index = 0;
	unsigned int out_index = 0;
	unsigned short c;

	c = unicode[index++];
	while (c) {
		if (c < 0x080) {
			if (out_index + 1 >= utf_size) break;
			utf8[out_index++] = (unsigned char)c;
		}
		else if (c < 0x800) {
			if (out_index + 2 >= utf_size) break;
			utf8[out_index++] = 0xc0 | (c >> 6);
			utf8[out_index++] = 0x80 | (c & 0x3f);
		}
		else {
			if (out_index + 3 >= utf_size) break;
			utf8[out_index++] = 0xe0 | (c >> 12);
			utf8[out_index++] = 0x80 | ((c >> 6) & 0x3f);
			utf8[out_index++] = 0x80 | (c & 0x3f);
		}
		c = unicode[index++];
	}
	utf8[out_index] = 0x00;

	return utf8;
}


char* __stdcall make_utf8_string(const WCHAR *unicode)
{
	if (!unicode) return NULL;

	/* first calculate the size of the target string */
	size_t size = get_utf8_size(unicode);

	char *out = (char*)SAFE_MALLOC(size + 1);
	if (!out)
		return NULL;

	return make_utf8_string_static(unicode, out, size + 1);
}


WCHAR* __stdcall make_unicode_string_static(const char *utf8, WCHAR *unicode, size_t unicode_size)
{
	unsigned int out_index = 0;

	if (utf8) {
		unsigned int index = 0;
		unsigned char c = utf8[index++];

		while (c) {
			if (out_index + 1 >= unicode_size) break;
			if ((c & 0x80) == 0) {
				unicode[out_index++] = c;
			}
			else if ((c & 0xe0) == 0xe0) {
				unicode[out_index] = (c & 0x1F) << 12;
				c = utf8[index++];
				unicode[out_index] |= (c & 0x3F) << 6;
				c = utf8[index++];
				unicode[out_index++] |= (c & 0x3F);
			}
			else {
				unicode[out_index] = (c & 0x3F) << 6;
				c = utf8[index++];
				unicode[out_index++] |= (c & 0x3F);
			}
			c = utf8[index++];
		}
	}
	unicode[out_index] = 0;

	return unicode;
}


WCHAR* __stdcall make_unicode_string(const char *utf8)
{
	int size = 0, index = 0;

	if (!utf8) return NULL;

	/* first calculate the size of the target string */
	unsigned char c = utf8[index++];
	while (c) {
		if ((c & 0x80) == 0) {
			index += 0;
		}
		else if ((c & 0xe0) == 0xe0) {
			index += 2;
		}
		else {
			index += 1;
		}
		size += 1;
		c = utf8[index++];
	}

	WCHAR *out = (WCHAR*)SAFE_MALLOC((size + 1) * sizeof(WCHAR));
	if (!out)
		return NULL;
	else
		return make_unicode_string_static(utf8, out, size + 1);
}


int __stdcall utf8_encode(const char *from, char **to)
{
	int wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, from, (int)mir_strlen(from), NULL, 0);

	if (wchars == 0) {
#ifdef _DEBUG
		fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
		return -1;
	}

	WCHAR *unicode = (WCHAR*)_alloca((wchars + 1) * sizeof(WCHAR));
	ZeroMemory(unicode, (wchars + 1) * sizeof(WCHAR));

	int err = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, from, (int)mir_strlen(from), unicode, wchars);
	if (err != wchars) {
#ifdef _DEBUG
		fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
		return -1;
	}

	/* On NT-based windows systems, we could use WideCharToMultiByte(), but
	* MS doesn't actually have a consistent API across win32.
	*/
	*to = make_utf8_string(unicode);
	return 0;
}


char* __stdcall ansi_to_utf8(const char *ansi)
{
	char *szUtf = NULL;

	if (mir_strlen(ansi)) {
		utf8_encode(ansi, &szUtf);
		return szUtf;
	}

	return null_strdup("");
}


char* __stdcall ansi_to_utf8_codepage(const char *ansi, WORD wCp)
{
	size_t wchars = mir_strlen(ansi);
	WCHAR *unicode = (WCHAR*)_alloca((wchars + 1) * sizeof(WCHAR));
	ZeroMemory(unicode, (wchars + 1) * sizeof(WCHAR));

	MultiByteToWideChar(wCp, MB_PRECOMPOSED, ansi, (int)wchars, unicode, (int)wchars);

	return make_utf8_string(unicode);
}


// Returns 0 on error, 1 on success
int __stdcall utf8_decode_codepage(const char *from, char **to, WORD wCp)
{
	int nResult = 0;

	_ASSERTE(!(*to)); // You passed a non-zero pointer, make sure it doesnt point to unfreed memory

	// Validate the string
	if (!UTF8_IsValid(from))
		return 0;

	// Use the native conversion routines when available
	if (bHasCP_UTF8) {
		size_t inlen = mir_strlen(from) + 1;
		WCHAR *wszTemp = (WCHAR *)_alloca(inlen * sizeof(WCHAR));
		ZeroMemory(wszTemp, inlen * sizeof(WCHAR));

		// Convert the UTF-8 string to UCS
		if (MultiByteToWideChar(CP_UTF8, 0, from, -1, wszTemp, (int)inlen)) {
			// Convert the UCS string to local ANSI codepage
			*to = (char*)SAFE_MALLOC(inlen);
			if (WideCharToMultiByte(wCp, 0, wszTemp, -1, *to, (int)inlen, NULL, NULL)) {
				nResult = 1;
			}
			else {
				SAFE_FREE(to);
			}
		}
	}
	else {
		size_t chars = mir_strlen(from) + 1;
		WCHAR *unicode = (WCHAR*)_alloca(chars * sizeof(WCHAR));
		make_unicode_string_static(from, unicode, chars);

		chars = WideCharToMultiByte(wCp, WC_COMPOSITECHECK, unicode, -1, NULL, 0, NULL, NULL);

		if (chars == 0) {
#ifdef _DEBUG
			fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
			return 0;
		}

		*to = (char*)SAFE_MALLOC((chars + 1)*sizeof(char));
		if (*to == NULL) {
#ifdef _DEBUG
			fprintf(stderr, "Out of memory processing string to local charset\n");
#endif
			return 0;
		}

		int err = WideCharToMultiByte(wCp, WC_COMPOSITECHECK, unicode, -1, *to, (int)chars, NULL, NULL);
		if (err != chars) {
#ifdef _DEBUG
			fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
			SAFE_FREE(to);
			return 0;
		}

		nResult = 1;
	}

	return nResult;
}


// Standard version with current codepage
int __stdcall utf8_decode(const char *from, char **to)
{
	return utf8_decode_codepage(from, to, CP_ACP);
}


// Returns 0 on error, 1 on success
int __stdcall utf8_decode_static(const char *from, char *to, size_t to_size)
{
	int nResult = 0;

	_ASSERTE(to); // You passed a zero pointer

	// Validate the string
	if (!UTF8_IsValid(from))
		return 0;

	// Clear target
	ZeroMemory(to, to_size);

	// Use the native conversion routines when available
	if (bHasCP_UTF8) {
		size_t inlen = mir_strlen(from) + 1;
		WCHAR *wszTemp = (WCHAR*)_alloca(inlen * sizeof(WCHAR));
		ZeroMemory(wszTemp, inlen * sizeof(WCHAR));

		// Convert the UTF-8 string to UCS
		if (MultiByteToWideChar(CP_UTF8, 0, from, -1, wszTemp, (int)inlen)) {
			// Convert the UCS string to local ANSI codepage
			if (WideCharToMultiByte(CP_ACP, 0, wszTemp, -1, to, (int)to_size, NULL, NULL)) {
				nResult = 1;
			}
		}
	}
	else {
		size_t chars = mir_strlen(from) + 1;
		WCHAR *unicode = (WCHAR*)_alloca(chars * sizeof(WCHAR));

		make_unicode_string_static(from, unicode, chars);

		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, to, (int)to_size, NULL, NULL);

		nResult = 1;
	}

	return nResult;
}


WCHAR* __stdcall ansi_to_unicode(const char *ansi)
{
	int wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ansi, (int)mir_strlen(ansi), NULL, 0);

	if (wchars == 0) {
#ifdef _DEBUG
		fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
		return NULL;
	}

	WCHAR *unicode = (WCHAR*)SAFE_MALLOC((wchars + 1) * sizeof(WCHAR));

	int err = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ansi, (int)mir_strlen(ansi), unicode, wchars);
	if (err != wchars) {
#ifdef _DEBUG
		fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
		SAFE_FREE(&unicode);
		return NULL;
	}
	return unicode;
}


char* __stdcall unicode_to_ansi_static(const WCHAR *unicode, char *ansi, size_t ansi_size)
{
	ZeroMemory(ansi, ansi_size);

	if (WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, (int)mir_wstrlen(unicode), ansi, (int)ansi_size, NULL, NULL) > 1)
		return ansi;

	return NULL;
}

char* __stdcall unicode_to_ansi(const WCHAR *unicode)
{
	int chars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, (int)mir_wstrlen(unicode), NULL, 0, NULL, NULL);

	if (chars == 0) {
#ifdef _DEBUG
		fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
		return NULL;
	}

	char* ansi = (char*)SAFE_MALLOC((chars + 1)*sizeof(char));
	if (ansi == NULL) {
#ifdef _DEBUG
		fprintf(stderr, "Out of memory processing string to local charset\n");
#endif
		return NULL;
	}

	int err = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, (int)mir_wstrlen(unicode), ansi, chars, NULL, NULL);
	if (err != chars) {
#ifdef _DEBUG
		fprintf(stderr, "Unicode translation error %d\n", GetLastError());
#endif
		return NULL;
	}

	return ansi;
}
