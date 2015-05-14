/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Standard functions

int CMsnProto::getStringUtf(MCONTACT hContact, const char* name, DBVARIANT* result)
{
	return db_get_utf(hContact, m_szModuleName, name, result);
}

int CMsnProto::getStringUtf(const char* name, DBVARIANT* result)
{
	return db_get_utf(NULL, m_szModuleName, name, result);
}

void CMsnProto::setStringUtf(MCONTACT hContact, const char* name, const char* value)
{
	db_set_utf(hContact, m_szModuleName, name, value);
}

/////////////////////////////////////////////////////////////////////////////////////////

TCHAR* CMsnProto::GetContactNameT(MCONTACT hContact)
{
	if (hContact)
		return (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM(hContact), GCDNF_TCHAR);

	CONTACTINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	ci.szProto = m_szModuleName;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (m_DisplayNameCache) mir_free(m_DisplayNameCache);
		return (TCHAR*)m_DisplayNameCache = ci.pszVal;
	}
	else
		return _T("Me");
}

unsigned MSN_GenRandom(void)
{
	unsigned rndnum;
	CallService(MS_UTILS_GETRANDOM, sizeof(rndnum), (LPARAM)&rndnum);
	return rndnum & 0x7FFFFFFF;
}
