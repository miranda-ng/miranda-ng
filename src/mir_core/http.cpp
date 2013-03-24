/*
Copyright (C) 2012-13 Miranda NG team (http://miranda-ng.org)

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

	const char *s;
	int outputLen;
	for (outputLen = 0, s = szUrl; *s; s++) {
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
	for (s = szUrl; *s; s++) {
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
