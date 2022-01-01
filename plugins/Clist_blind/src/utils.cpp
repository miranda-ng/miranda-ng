/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

wchar_t* MyDBGetContactSettingTString(MCONTACT hContact, char *module, char *setting, wchar_t *out, size_t len, wchar_t *def)
{
	out[0] = '\0';

	DBVARIANT dbv;
	if (!db_get_ws(hContact, module, setting, &dbv)) {
		if (dbv.type == DBVT_ASCIIZ)
			MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, out, (int)len);
		else if (dbv.type == DBVT_UTF8)
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, out, (int)len);
		else if (dbv.type == DBVT_WCHAR)
			mir_wstrncpy(out, dbv.pwszVal, len);
		else if (def != nullptr)
			mir_wstrncpy(out, def, len);

		db_free(&dbv);
	}
	else if (def != nullptr)
		mir_wstrncpy(out, def, len);

	return out;
}
