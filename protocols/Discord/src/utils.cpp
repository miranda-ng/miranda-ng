/*
Copyright © 2016-17 Miranda NG team

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

int StrToStatus(const CMStringW &str)
{
	if (str == L"idle")
		return ID_STATUS_NA;
	if (str == L"dnd")
		return ID_STATUS_DND;
	if (str == L"online")
		return ID_STATUS_ONLINE;
	if (str == L"offline")
		return ID_STATUS_OFFLINE;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

JSONNode& operator<<(JSONNode &json, const INT_PARAM &param)
{
	json.push_back(JSONNode(param.szName, param.iValue));
	return json;
}

JSONNode& operator<<(JSONNode &json, const INT64_PARAM &param)
{
	char tmp[100];
	_i64toa_s(param.iValue, tmp, _countof(tmp), 10);
	json.push_back(JSONNode(param.szName, tmp));
	return json;
}

JSONNode& operator<<(JSONNode &json, const BOOL_PARAM &param)
{
	json.push_back(JSONNode(param.szName, param.bValue));
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

time_t StringToDate(const CMStringW &str)
{
	struct tm T = { 0 };
	int boo;
	if (swscanf(str, L"%04d-%02d-%02dT%02d:%02d:%02d.%d", &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, &boo) != 7)
		return 0;

	T.tm_year -= 1900;
	T.tm_mon--;
	time_t t = mktime(&T);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LONG volatile g_counter = 1;

int SerialNext()
{
	return InterlockedIncrement(&g_counter);
}

/////////////////////////////////////////////////////////////////////////////////////////

SnowFlake CDiscordProto::getId(const char *szSetting)
{
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(NULL, m_szModuleName, szSetting, &dbv))
		return 0;
	
	SnowFlake result = (dbv.cpbVal == sizeof(SnowFlake)) ? *(SnowFlake*)dbv.pbVal : 0;
	db_free(&dbv);
	return result;
}

SnowFlake CDiscordProto::getId(MCONTACT hContact, const char *szSetting)
{
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(hContact, m_szModuleName, szSetting, &dbv))
		return 0;

	SnowFlake result = (dbv.cpbVal == sizeof(SnowFlake)) ? *(SnowFlake*)dbv.pbVal : 0;
	db_free(&dbv);
	return result;
}

void CDiscordProto::setId(const char *szSetting, SnowFlake iValue)
{
	db_set_blob(NULL, m_szModuleName, szSetting, &iValue, sizeof(iValue));
}

void CDiscordProto::setId(MCONTACT hContact, const char *szSetting, SnowFlake iValue)
{
	db_set_blob(hContact, m_szModuleName, szSetting, &iValue, sizeof(iValue));
}

/////////////////////////////////////////////////////////////////////////////////////////

static CDiscordUser *g_myUser = new CDiscordUser(0);

CDiscordUser* CDiscordProto::FindUser(SnowFlake id)
{
	return arUsers.find((CDiscordUser*)&id);
}

CDiscordUser* CDiscordProto::FindUser(const wchar_t *pwszUsername, int iDiscriminator)
{
	for (int i = 0; i < arUsers.getCount(); i++) {
		CDiscordUser &p = arUsers[i];
		if (p.wszUsername == pwszUsername && p.iDiscriminator == iDiscriminator)
			return &p;
	}

	return NULL;
}

CDiscordUser* CDiscordProto::FindUserByChannel(SnowFlake channelId)
{
	for (int i = 0; i < arUsers.getCount(); i++) {
		CDiscordUser &p = arUsers[i];
		if (p.channelId == channelId)
			return &p;
	}

	return NULL;
}

CDiscordUser* CDiscordProto::PrepareUser(const JSONNode &user)
{
	SnowFlake id = _wtoi64(user["id"].as_mstring());
	if (id == m_ownId)
		return g_myUser;

	int iDiscriminator = _wtoi(user["discriminator"].as_mstring());
	CMStringW username = user["username"].as_mstring();

	CDiscordUser *pUser = FindUser(id);
	if (pUser == NULL) {
		MCONTACT tmp = INVALID_CONTACT_ID;

		// no user found by userid, try to find him via username+discriminator
		pUser = FindUser(username, iDiscriminator);
		if (pUser != NULL) {
			// if found, remove the object from list to resort it (its userid==0)
			if (pUser->hContact != 0)
				tmp = pUser->hContact;
			arUsers.remove(pUser);
		}
		pUser = new CDiscordUser(id);
		pUser->wszUsername = username;
		pUser->iDiscriminator = iDiscriminator;
		if (tmp != INVALID_CONTACT_ID) {
			// if we previously had a recently added contact without userid, write it down
			pUser->hContact = tmp;
			setId(pUser->hContact, DB_KEY_ID, id);
		}
		arUsers.insert(pUser);
	}

	if (pUser->hContact == 0) {
		MCONTACT hContact = db_add_contact();
		Proto_AddToContact(hContact, m_szModuleName);

		db_set_ws(hContact, "CList", "Group", m_wszDefaultGroup);
		setId(hContact, DB_KEY_ID, id);
		setWString(hContact, DB_KEY_NICK, username);
		setDword(hContact, DB_KEY_DISCR, iDiscriminator);

		pUser->hContact = hContact;
	}

	CheckAvatarChange(pUser->hContact, user["avatar"].as_mstring());
	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ProcessType(CDiscordUser *pUser, const JSONNode &pRoot)
{
	switch (pRoot["type"].as_int()) {
	case 1: // confirmed
		db_unset(pUser->hContact, "CList", "NotOnList");
		delSetting(pUser->hContact, DB_KEY_REQAUTH);
		break;

	case 3: // expecting authorization
		db_set_b(pUser->hContact, "CList", "NotOnList", 1);
		if (!getByte(pUser->hContact, DB_KEY_REQAUTH, 0)) {
			setByte(pUser->hContact, DB_KEY_REQAUTH, 1);

			CMStringA szId(FORMAT, "%lld", pUser->id);
			DB_AUTH_BLOB blob(pUser->hContact, T2Utf(pUser->wszUsername), NULL, NULL, szId, NULL);

			PROTORECVEVENT pre = { 0 };
			pre.timestamp = (DWORD)time(NULL);
			pre.lParam = blob.size();
			pre.szMessage = blob;
			ProtoChainRecv(pUser->hContact, PSR_AUTH, 0, (LPARAM)&pre);
		}
		break;
	}
}
