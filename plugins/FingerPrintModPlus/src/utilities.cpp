/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

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

//Start of header
#include "global.h"

LPWSTR make_unicode_string(LPCSTR utf8);
LPWSTR make_unicode_string_static(LPCSTR utf8, LPWSTR unicode, size_t unicode_len);
LPSTR make_utf8_string(LPCWSTR unicode);
LPSTR make_utf8_string_static(LPCWSTR unicode, LPSTR utf8, size_t utf_size);

LPVOID __fastcall SAFE_MALLOC(size_t size)
{
	LPVOID p = NULL;

	if (size)
	{
		p = malloc(size);

		if (p)
			ZeroMemory(p, size);
	}
	return p;
}

size_t __fastcall strlennull(LPCSTR string)
{
	if (string)
		return strlen(string);

	return 0;
}

int null_snprintf(LPSTR buffer, size_t count, LPCSTR fmt, ...)
{
	va_list va;
	int len;

	ZeroMemory(buffer, count);
	va_start(va, fmt);
	len = _vsnprintf(buffer, count-1, fmt, va);
	va_end(va);
	return len;
}

LPSTR __fastcall TranslateUtfStatic(LPCSTR src, LPSTR buf, size_t bufsize)
{ // this takes UTF-8 strings only!!!
	if (strlennull(src))
	{ // we can use unicode translate (0.5+)
		LPWSTR usrc = make_unicode_string(src);

		make_utf8_string_static(TranslateW(usrc), buf, bufsize);

		SAFE_FREE(usrc);
	}
	else
		buf[0] = '\0';

	return buf;
}

//unsigned char *make_utf8_string_static(LPCWSTR unicode, unsigned char *utf8, size_t utf_size)
LPSTR make_utf8_string_static(LPCWSTR unicode, LPSTR utf8, size_t utf_size)
{
	size_t index = 0;
	size_t out_index = 0;
	WORD c;

	c = (WORD)unicode[index++];
	while(c)
	{
		if (c < 0x080)
		{
			if (out_index + 1 >= utf_size) break;
			utf8[out_index++] = (CHAR)c;
		}
		else if (c < 0x800) 
		{
			if (out_index + 2 >= utf_size) break;
			utf8[out_index++] = 0xc0 | (c >> 6);
			utf8[out_index++] = 0x80 | (c & 0x3f);
		}
		else
		{
			if (out_index + 3 >= utf_size) break;
			utf8[out_index++] = 0xe0 | (c >> 12);
			utf8[out_index++] = 0x80 | ((c >> 6) & 0x3f);
			utf8[out_index++] = 0x80 | (c & 0x3f);
		}
		c = (WORD)unicode[index++];
	}
	utf8[out_index] = 0x00;

	return utf8;
}

LPSTR make_utf8_string(LPCWSTR unicode)
{
	size_t size = 0;
	size_t index = 0;
	LPSTR out;
	WORD c;

	if (!unicode) return NULL;

	/* first calculate the size of the target string */
	c = (WORD)unicode[index++];
	while(c)
	{
		if (c < 0x0080)
			size++;
		else if (c < 0x0800)
			size += 2;
		else
			size += 3;
		c = (WORD)unicode[index++];
	}

	//out = (unsigned char*)SAFE_MALLOC(size + 1);
	out = (LPSTR)SAFE_MALLOC(size + 1);
	if (out == NULL)
		return NULL;
	else
		return make_utf8_string_static(unicode, out, size + 1);
}

LPWSTR make_unicode_string_static(LPCSTR utf8, LPWSTR unicode, size_t unicode_len)
{
	size_t index = 0;
	size_t out_index = 0;
	BYTE c;

	c = (BYTE)utf8[index++];
	while(c)
	{
		if (out_index + 1 >= unicode_len) break;
		if ((c & 0x80) == 0)
		{
			unicode[out_index++] = c;
		} 
		else if ((c & 0xe0) == 0xe0)
		{
			unicode[out_index] = (c & 0x1F) << 12;
			c = (BYTE)utf8[index++];
			unicode[out_index] |= (c & 0x3F) << 6;
			c = (BYTE)utf8[index++];
			unicode[out_index++] |= (c & 0x3F);
		}
		else
		{
			unicode[out_index] = (c & 0x3F) << 6;
			c = (BYTE)utf8[index++];
			unicode[out_index++] |= (c & 0x3F);
		}
		c = (BYTE)utf8[index++];
	}
	unicode[out_index] = 0;

	return unicode;
}

LPWSTR make_unicode_string(LPCSTR utf8)
{
	size_t size = 0, index = 0;
	LPWSTR out;
	BYTE c;

	if (!utf8) return NULL;

	/* first calculate the size of the target string */
	c = (BYTE)utf8[index++];
	while(c)
	{
		if ((c & 0x80) == 0)
		{
			index += 0;
		}
		else if ((c & 0xe0) == 0xe0)
		{
			index += 2;
		}
		else
		{
			index++;
		}
		size++;
		c = (BYTE)utf8[index++];
	}

	out = (LPWSTR)SAFE_MALLOC((size + 1) * sizeof(WCHAR));
	if (out == NULL)
		return NULL;
	else
		return make_unicode_string_static(utf8, out, size + 1);
}

int UTF8_IsValid(LPCSTR pszInput)
{
	int nb, i;
	LPCSTR c = pszInput;

	if (!pszInput) return 0;

	for(c = pszInput; *c; c += (nb + 1))
	{
		if (!(*c & 0x80)) nb = 0;
		else if ((*c & 0xc0) == 0x80) return 0;
		else if ((*c & 0xe0) == 0xc0) nb = 1;
		else if ((*c & 0xf0) == 0xe0) nb = 2;
		else if ((*c & 0xf8) == 0xf0) nb = 3;
		else if ((*c & 0xfc) == 0xf8) nb = 4;
		else if ((*c & 0xfe) == 0xfc) nb = 5;

		for(i = 1; i <= nb; i++) // we this forward, do not cross end of string
			if ((*(c + i) & 0xc0) != 0x80)
				return 0;
	}

	return 1;
}

static BOOL bHasCP_UTF8 = FALSE;
int utf8_decode_static(LPCSTR from, LPSTR to, int to_size)
{
	int nResult = 0;
	// Validate the string
	if (!UTF8_IsValid(from))
		return 0;

	// Use the native conversion routines when available
	if (bHasCP_UTF8)
	{
		LPWSTR wszTemp = NULL;
		size_t inlen = strlennull(from);

		wszTemp = (LPWSTR)_alloca((inlen + 1) * sizeof(WCHAR));

		// Convert the UTF-8 string to UCS
		if (MultiByteToWideChar(CP_UTF8, 0, from, -1, wszTemp, (int)inlen + 1))
		{
			// Convert the UCS string to local ANSI codepage
			if (WideCharToMultiByte(CP_ACP, 0, wszTemp, -1, to, to_size, NULL, NULL))
			{
				nResult = 1;
			}
		}
	}
	else
	{
		size_t chars = strlennull(from) + 1;
		LPWSTR unicode = (LPWSTR)_alloca(chars * sizeof(WCHAR));
    
		make_unicode_string_static(from, unicode, chars);

		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, to, to_size, NULL, NULL);

		nResult = 1;
	}

	return nResult;
}

static LRESULT ControlAddStringUtf(HWND ctrl, DWORD msg, LPCSTR szString)
{
	char str[MAX_PATH];
	LPSTR szItem = TranslateUtfStatic(szString, str, MAX_PATH);
	LRESULT item = -1;

	LPWSTR wItem = make_unicode_string(szItem);
	item = SendMessageW(ctrl, msg, 0, (LPARAM)wItem);
	SAFE_FREE(wItem);
	return item;
}

static LRESULT ControlAddStringW(HWND ctrl, DWORD msg, LPCWSTR wszString)
{
	return SendMessageW(ctrl, msg, 0, (LPARAM)TranslateW(wszString));
}

LRESULT ComboBoxAddStringUtf(HWND hCombo, LPCSTR szString, DWORD data)
{
	LRESULT item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
	SendMessage(hCombo, CB_SETITEMDATA, item, data);

	return item;
}

LRESULT ComboBoxAddStringW(HWND hCombo, LPCWSTR wszString, DWORD data)
{
	LRESULT item = ControlAddStringW(hCombo, CB_ADDSTRING, wszString);
	SendMessageW(hCombo, CB_SETITEMDATA, item, data);

	return item;
}

LRESULT ListBoxAddStringUtf(HWND hList, LPCSTR szString)
{
	return ControlAddStringUtf(hList, LB_ADDSTRING, szString);
}

LRESULT ListBoxAddStringW(HWND hList, LPCWSTR wszString)
{
	return ControlAddStringW(hList, LB_ADDSTRING, wszString);
}
