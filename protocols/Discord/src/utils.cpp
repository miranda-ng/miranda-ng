/*
Copyright © 2016 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CDiscordProto::SetAllContactStatuses(int status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (!getByte(hContact, "ChatRoom"))
			setWord(hContact, "Status", (WORD)status);
}

/////////////////////////////////////////////////////////////////////////////////////////

JSONNode& operator<<(JSONNode &json, const INT_PARAM &param)
{
	json.push_back(JSONNode(param.szName, param.iValue));
	return json;
}

JSONNode& operator<<(JSONNode &json, const CHAR_PARAM &param)
{
	json.push_back(JSONNode(param.szName, param.szValue));
	return json;
}

JSONNode& operator<<(JSONNode &json, const WCHAR_PARAM &param)
{
	json.push_back(JSONNode(param.szName, ptrA(mir_utf8encodeW(param.wszValue)).get()));
	return json;
}

/////////////////////////////////////////////////////////////////////////////////////////

SnowFlake CDiscordProto::getId(const char *szSetting)
{
	SnowFlake result;
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	dbv.pbVal = (BYTE*)&result;
	dbv.cpbVal = sizeof(result);
	return (db_get(NULL, m_szModuleName, szSetting, &dbv)) ? 0 : result;
}

SnowFlake CDiscordProto::getId(MCONTACT hContact, const char *szSetting)
{
	SnowFlake result;
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	dbv.pbVal = (BYTE*)&result;
	dbv.cpbVal = sizeof(result);
	return (db_get(hContact, m_szModuleName, szSetting, &dbv)) ? 0 : result;
}

void CDiscordProto::setId(const char *szSetting, SnowFlake iValue)
{
	db_set_blob(NULL, m_szModuleName, szSetting, &iValue, sizeof(iValue));
}

void CDiscordProto::setId(MCONTACT hContact, const char *szSetting, SnowFlake iValue)
{
	db_set_blob(hContact, m_szModuleName, szSetting, &iValue, sizeof(iValue));
}
