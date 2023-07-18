/*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

wchar_t* PFTS_StringToTchar(int flags, const MAllStrings s)
{
	if (flags & PFTS_UTF)
		return mir_utf8decodeW(s.a);
	if (flags & PFTS_UNICODE)
		return mir_wstrdup(s.w);
	return mir_a2u(s.a);
}

int PFTS_CompareWithTchar(PROTOFILETRANSFERSTATUS *ft, const MAllStrings s, wchar_t *r)
{
	if (ft->flags & PFTS_UTF)
		return mir_wstrcmp(Utf2T(s.a), r);

	if (ft->flags & PFTS_UNICODE)
		return mir_wstrcmp(s.w, r);

	return mir_wstrcmp(_A2T(s.a), r);
}
