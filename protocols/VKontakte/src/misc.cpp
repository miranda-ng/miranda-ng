/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

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

TCHAR* CVkProto::GetUserStoredPassword()
{
	ptrA szRawPass( getStringA("Password"));
	if (szRawPass != NULL) {
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(szRawPass), szRawPass);
		return mir_utf8decodeT(szRawPass);
	}
	return NULL;
}

void CVkProto::SetAllContactStatuses(int iStatus)
{
	for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		if (getWord(hContact, "Status", 0) != iStatus)
			setWord(hContact, "Status", iStatus);
	}
}

int CVkProto::SetServerStatus(int iStatus)
{
	return 0;
}
