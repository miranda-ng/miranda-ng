/*
Copyright (C) 2012-15 Miranda NG team (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "commonheaders.h"

/////////////////////////////////////////////////////////////////////////////////////////

static const char szHexDigits[] = "0123456789ABCDEF";

MIR_CORE_DLL(char*) mir_urlEncode(const char *szUrl)
{
	if (szUrl == NULL)
		return NULL;

	const BYTE *s;
	int outputLen;
	for (outputLen = 0, s = (const BYTE*)szUrl; *s; s++) {
		if (('0' <= *s && *s <= '9')  || //0-9
			 ('A' <= *s && *s <= 'Z')  || //ABC...XYZ
			 ('a' <= *s && *s <= 'z')  || //abc...xyz
			*s == '-' || *s == '_' || *s == '.' || *s == ' ') outputLen++;
		else outputLen += 3;
	}

	char *szOutput = (char*)mir_alloc(outputLen+1);
	if (szOutput == NULL)
		return NULL;

	char *d = szOutput;
	for (s = (const BYTE*)szUrl; *s; s++) {
		if (('0' <= *s && *s <= '9')  || //0-9
			 ('A' <= *s && *s <= 'Z')  || //ABC...XYZ
			 ('a' <= *s && *s <= 'z')  || //abc...xyz
			*s == '-' || *s == '_' || *s == '.') *d++ = *s;
		else if (*s == ' ') *d++='+';
		else {
			*d++ = '%';
			*d++ = szHexDigits[*s >> 4];
			*d++ = szHexDigits[*s & 0xF];
		}
	}
	*d = '\0';
	return szOutput;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

MIR_CORE_DLL(char*) mir_base64_encode(const BYTE *input, unsigned inputLen)
{
	if (input == NULL)
		return NULL;

	unsigned outputLen = mir_base64_encode_bufsize(inputLen);
	char *output = (char*)mir_alloc(outputLen);
	if (output == NULL)
		return NULL;

	return mir_base64_encodebuf(input, inputLen, output, outputLen);
}

MIR_CORE_DLL(char*) mir_base64_encodebuf(const BYTE *input, unsigned inputLen, char *output, unsigned outputLen)
{
	if (input == NULL)
		return NULL;

	if (outputLen < mir_base64_encode_bufsize(inputLen))
		return NULL;

	char *p = output;
	for (unsigned i=0; i < inputLen; ) {
		int rest = 0;
		BYTE chr[3];
		chr[0] = input[i++];
		chr[1] = (i < inputLen) ? input[i++] : rest++, 0;
		chr[2] = (i < inputLen) ? input[i++] : rest++, 0;

		*p++ = cb64[ chr[0] >> 2 ];
		*p++ = cb64[ ((chr[0] & 0x03) << 4) | (chr[1] >> 4) ];
		int b2 = ((chr[1] & 0x0F) << 2) | (chr[2] >> 6),
		    b3 = chr[2] & 0x3F;

		if (rest == 2) { *p++ = '='; *p++ = '='; }
		else if (rest == 1) { *p++ = cb64[b2]; *p++ = '='; }
		else { *p++ = cb64[b2]; *p++ = cb64[b3]; }
	}

	*p = 0;
	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////

static BYTE Base64DecodeTable[] =
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

	size_t length = strlen(input);
	size_t nLength = (length / 4) * 3;
	const char *stop = input + length;

	char *output = (char *)mir_alloc(nLength+1);
	char *p = output;

	while (input < stop) {
		BYTE e[4];
		for (int i=0; i < 4; ) {
			if (*input == '\n' || *input == '\r') // simply skip a char
				input++;
			else if (*input == 0)  // do not advance input
				e[i++] = (BYTE)-1;
			else
				e[i++] = Base64DecodeTable[*input++];
		}

		if (e[0] == (BYTE)-1 || e[1] == (BYTE)-1)
			break;

		*p++ = (e[0] << 2) | (e[1] >> 4);
		if (e[2] != (BYTE)-1)
			*p++ = ((e[1] & 15) << 4) | (e[2] >> 2);
		if (e[3] != (BYTE)-1)
			*p++ = ((e[2] & 3) << 6) | e[3];
	}

	*p = 0;

	if (outputLen != NULL)
		*outputLen = p - output;

	return output;
}
