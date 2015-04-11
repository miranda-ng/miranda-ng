/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-15 Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "jabber.h"

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::JLoginFailed(int errorCode)
{
	m_savedPassword = NULL;
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, errorCode);
}

// save/load crypted strings
void __forceinline sttCryptString(char *str)
{
	for (;*str; ++str) {
		const char c = *str ^ 0xc3;
		if (c) *str = c;
	}
}

static TCHAR* JSetStringCrypt(LPCSTR szModule, MCONTACT hContact, char *valueName)
{
	DBVARIANT dbv;
	if (db_get_s(hContact, szModule, valueName, &dbv))
		return NULL;

	sttCryptString(dbv.pszVal);
	WCHAR *res = mir_utf8decodeW(dbv.pszVal);
	db_free(&dbv);
	return res;
}

void CJabberProto::ConvertPasswords()
{
	ptrT passw(JSetStringCrypt(m_szModuleName, NULL, "LoginPassword"));
	if (passw == NULL)
		return;

	setTString("Password", passw);
	delSetting("LoginPassword");

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if ((passw = JSetStringCrypt(m_szModuleName, hContact, "LoginPassword")) == NULL)
			continue;

		setTString(hContact, "Password", passw);
		delSetting(hContact, "LoginPassword");
	}

	for (int i = 0;; i++) {
		char varName[100];
		mir_snprintf(varName, SIZEOF(varName), "rcMuc_%d_server", i);
		ptrA str(getStringA(NULL, varName));
		if (str == NULL)
			break;

		mir_snprintf(varName, SIZEOF(varName), "rcMuc_%d", i);
		ptrT passw(JSetStringCrypt(m_szModuleName, NULL, varName));
		if (passw != NULL) {
			delSetting(varName);

			mir_snprintf(varName, SIZEOF(varName), "password_rcMuc_%d", i);
			setTString(varName, passw);
		}
	}
}