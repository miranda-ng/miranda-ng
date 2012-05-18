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
#include "parse_inet.h"
#include <wininet.h>

static TCHAR *parseUrlEnc(ARGUMENTSINFO *ai) {

	TCHAR *tres;
	char hex[8], *res;
	unsigned int cur;

	if (ai->argc != 2) {
		return NULL;
	}
#ifdef UNICODE
	res = u2a(ai->targv[1]);
#else
	res = _strdup(ai->argv[1]);
#endif
	if (res == NULL) {
		return NULL;
	}
	cur = 0;
	while (cur < strlen(res)) {
		if ( ( (*(res+cur) >= '0') && (*(res+cur) <= '9') ) || ( (*(res+cur) >= 'a') && (*(res+cur) <= 'z') ) || ( (*(res+cur) >= 'A') && (*(res+cur) <= 'Z') ) ) {
			cur++;
			continue;
		}
		res = ( char* )realloc(res, strlen(res)+4);
		if (res == NULL)
			return NULL;

		MoveMemory(res+cur+3, res+cur+1, strlen(res+cur+1)+1);
		_snprintf(hex, sizeof(hex), "%%%x", *(res+cur));
		strncpy(res+cur, hex, strlen(hex));
		cur+=strlen(hex);
	}
#ifdef UNICODE
	tres = a2u(res);
#else
	tres = _strdup(res);
#endif
	free(res);

	return tres;
}

static TCHAR *parseUrlDec(ARGUMENTSINFO *ai) {

	char *res, hex[8];
	TCHAR *tres;
	unsigned int cur;

	if (ai->argc != 2) {
		return NULL;
	}
#ifdef UNICODE
	res = u2a(ai->targv[1]);
#else
	res = _strdup(ai->argv[1]);
#endif
	if (res == NULL) {
		return NULL;
	}
	cur = 0;
	while (cur < strlen(res)) {
		if ( (*(res+cur) == '%') && (strlen(res+cur) >= 3) ) {
			memset(hex, '\0', sizeof(hex));
			strncpy(hex, res+cur+1, 2);
			*(res+cur) = (char)strtol(hex, NULL, 16);
			MoveMemory(res+cur+1, res+cur+3, strlen(res+cur+3)+1);
		}
		cur++;
	}
	res = ( char* )realloc(res, strlen(res)+1);
#ifdef UNICODE
	tres = a2u(res);
#else
	tres = _strdup(res);
#endif
	free(res);
	
	return tres;
}

static TCHAR *parseNToA(ARGUMENTSINFO *ai) {

	char *res;
	struct in_addr in;

	if (ai->argc != 2) {
		return NULL;
	}
	
	in.s_addr = ttoi(ai->targv[1]);
	res = inet_ntoa(in);
	if (res != NULL) {
#ifdef UNICODE
		return a2u(res);
#else
		return _strdup(res);
#endif
	}

	return NULL;
}

static TCHAR *parseHToA(ARGUMENTSINFO *ai) {

	char *res;
	struct in_addr in;

	if (ai->argc != 2) {
		return NULL;
	}
	
	in.s_addr = htonl(ttoi(ai->targv[1]));
	res = inet_ntoa(in);
	if (res != NULL) {
#ifdef UNICODE
		return a2u(res);
#else
		return _strdup(res);
#endif
	}

	return NULL;
}

int registerInetTokens() {

	registerIntToken(_T(URLENC), parseUrlEnc, TRF_FUNCTION, "Internet Related\t(x)\tconverts each non-html character into hex format");
	registerIntToken(_T(URLDEC), parseUrlDec, TRF_FUNCTION, "Internet Related\t(x)\tconverts each hex value into non-html character");
	registerIntToken(_T(NTOA), parseNToA, TRF_FUNCTION, "Internet Related\t(x)\tconverts a 32-bit number to IPv4 dotted notation");
	registerIntToken(_T(HTOA), parseHToA, TRF_FUNCTION, "Internet Related\t(x)\tconverts a 32-bit number (in host byte order) to IPv4 dotted notation");

	return 0;
}
