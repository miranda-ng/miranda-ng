#include "commonheaders.h"

LPSTR szOut = NULL;
LPWSTR wszOut = NULL;

LPSTR __cdecl utf8encode(LPCWSTR str)
{
	LPWSTR wszTemp, w;
	int len, i;

	if (str == NULL)
		return NULL;

	wszTemp = (LPWSTR)str;

	len = 0;
	for (w = wszTemp; *w; w++) {
		if (*w < 0x0080) len++;
		else if (*w < 0x0800) len += 2;
		else len += 3;
	}

	SAFE_FREE(szOut);
	if ((szOut = (LPSTR)malloc(len + 1)) == NULL)
		return NULL;

	i = 0;
	for (w = wszTemp; *w; w++) {
		if (*w < 0x0080)
			szOut[i++] = (BYTE)*w;
		else if (*w < 0x0800) {
			szOut[i++] = 0xc0 | (((*w) >> 6) & 0x3f);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
		else {
			szOut[i++] = 0xe0 | ((*w) >> 12);
			szOut[i++] = 0x80 | (((*w) >> 6) & 0x3f);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
	}
	szOut[i] = '\0';
	return szOut;
}

LPWSTR __cdecl utf8decode(LPCSTR str)
{

	int i, len;
	LPSTR p;
	//	LPWSTR wszOut;

	if (str == NULL) return NULL;

	len = strlen(str) + 1;

	SAFE_FREE(wszOut);
	if ((wszOut = (LPWSTR)malloc(len*sizeof(WCHAR))) == NULL)
		return NULL;
	p = (LPSTR)str;
	i = 0;
	while (*p) {
		if ((p[0] & 0x80) == 0) {
			wszOut[i++] = *(p++);
			continue;
		}
		if ((p[0] & 0xe0) == 0xe0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80) {
			wszOut[i] = (*(p++) & 0x0f) << 12;
			wszOut[i] |= (*(p++) & 0x3f) << 6;
			wszOut[i++] |= (*(p++) & 0x3f);
			continue;
		}
		if ((p[0] & 0xe0) == 0xc0 && (p[1] & 0xc0) == 0x80) {
			wszOut[i] = (*(p++) & 0x1f) << 6;
			wszOut[i++] |= (*(p++) & 0x3f);
			continue;
		}
		wszOut[i++] = *p++;
	}
	wszOut[i] = '\0';

	return wszOut;
}


// Returns true if the buffer only contains 7-bit characters.
int __cdecl is_7bit_string(LPCSTR str)
{
	while (*str) {
		if (*str & 0x80) {
			return FALSE;
			break;
		}
		str++;
	}

	return TRUE;
}


//Copyright (C) 2001, 2002 Peter Verthez
//under GNU LGPL
int __cdecl is_utf8_string(LPCSTR str)
{
	int expect_bytes = 0;

	if (!str) return 0;

	while (*str) {
		if ((*str & 0x80) == 0) {
			/* Looks like an ASCII character */
			if (expect_bytes)
				/* byte of UTF-8 character expected */
				return 0;
			else {
				/* OK, ASCII character expected */
				str++;
			}
		}
		else {
			/* Looks like byte of an UTF-8 character */
			if (expect_bytes) {
				/* expect_bytes already set: first byte of UTF-8 char already seen */
				if ((*str & 0xC0) == 0x80) {
					/* OK, next byte of UTF-8 character */
					/* Decrement number of expected bytes */
					expect_bytes--;
					str++;
				}
				else {
					/* again first byte ?!?! */
					return 0;
				}
			}
			else {
				/* First byte of the UTF-8 character */
				/* count initial one bits and set expect_bytes to 1 less */
				char ch = *str;
				while (ch & 0x80) {
					expect_bytes++;
					ch = (ch & 0x7f) << 1;
				}
				expect_bytes--;
				str++;
			}
		}
	}

	return (expect_bytes == 0);
}
