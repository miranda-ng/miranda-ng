/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "variables.h"

static TCHAR *parseUrlEnc(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	char *res = mir_t2a(ai->targv[1]);
	if (res == NULL)
		return NULL;

	size_t cur = 0;
	while (cur < mir_strlen(res)) {
		if (((*(res + cur) >= '0') && (*(res + cur) <= '9')) || ((*(res + cur) >= 'a') && (*(res + cur) <= 'z')) || ((*(res + cur) >= 'A') && (*(res + cur) <= 'Z'))) {
			cur++;
			continue;
		}
		res = (char*)mir_realloc(res, mir_strlen(res) + 4);
		if (res == NULL)
			return NULL;

		char hex[8];
		memmove(res + cur + 3, res + cur + 1, mir_strlen(res + cur + 1) + 1);
		mir_snprintf(hex, SIZEOF(hex), "%%%x", *(res + cur));
		strncpy(res + cur, hex, mir_strlen(hex));
		cur += mir_strlen(hex);
	}

	TCHAR *tres = mir_a2t(res);
	mir_free(res);
	return tres;
}

static TCHAR *parseUrlDec(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	char *res = mir_t2a(ai->targv[1]);
	if (res == NULL)
		return NULL;

	unsigned int cur = 0;
	while (cur < mir_strlen(res)) {
		if ((*(res + cur) == '%') && (mir_strlen(res + cur) >= 3)) {
			char hex[8];
			memset(hex, '\0', sizeof(hex));
			strncpy(hex, res + cur + 1, 2);
			*(res + cur) = (char)strtol(hex, NULL, 16);
			memmove(res + cur + 1, res + cur + 3, mir_strlen(res + cur + 3) + 1);
		}
		cur++;
	}

	res = (char*)mir_realloc(res, mir_strlen(res) + 1);
	TCHAR *tres = mir_a2t(res);
	mir_free(res);
	return tres;
}

static TCHAR *parseNToA(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	struct in_addr in;
	in.s_addr = ttoi(ai->targv[1]);
	return mir_a2t(inet_ntoa(in));
}

static TCHAR *parseHToA(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	struct in_addr in;
	in.s_addr = htonl(ttoi(ai->targv[1]));
	return mir_a2t(inet_ntoa(in));
}

void registerInetTokens()
{
	registerIntToken(URLENC, parseUrlEnc, TRF_FUNCTION, LPGEN("Internet Related")"\t(x)\t"LPGEN("converts each non-html character into hex format"));
	registerIntToken(URLDEC, parseUrlDec, TRF_FUNCTION, LPGEN("Internet Related")"\t(x)\t"LPGEN("converts each hex value into non-html character"));
	registerIntToken(NTOA, parseNToA, TRF_FUNCTION, LPGEN("Internet Related")"\t(x)\t"LPGEN("converts a 32-bit number to IPv4 dotted notation"));
	registerIntToken(HTOA, parseHToA, TRF_FUNCTION, LPGEN("Internet Related")"\t(x)\t"LPGEN("converts a 32-bit number (in host byte order) to IPv4 dotted notation"));
}
