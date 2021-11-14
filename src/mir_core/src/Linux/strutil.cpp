/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

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

#include "../stdafx.h"

MIR_CORE_DLL(char*) strlwr(char *str)
{
	for (char *p = str; *p; p++)
		*p = tolower(*p);

    return str;
}

MIR_CORE_DLL(char*) strupr(char *str)
{
	for (char *p = str; *p; p++)
        *p = toupper(*p);

    return str;
}

MIR_CORE_DLL(char*) strrev(char *str)
{
	if (!str || !*str)
		return str;
			
	char *p1, *p2;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}
