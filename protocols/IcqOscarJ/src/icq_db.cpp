// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Internal Database API
//
// -----------------------------------------------------------------------------
#include "icqoscar.h"

int CIcqProto::getSetting(HANDLE hContact, const char *szSetting, DBVARIANT *dbv)
{
	return db_get_s(hContact, m_szModuleName, szSetting, dbv, 0);
}

BYTE CIcqProto::getSettingByte(HANDLE hContact, const char *szSetting, BYTE byDef)
{
	return db_get_b(hContact, m_szModuleName, szSetting, byDef);
}

WORD CIcqProto::getSettingWord(HANDLE hContact, const char *szSetting, WORD wDef)
{
	return db_get_w(hContact, m_szModuleName, szSetting, wDef);
}

DWORD CIcqProto::getSettingDword(HANDLE hContact, const char *szSetting, DWORD dwDef)
{
	DBVARIANT dbv = {DBVT_DELETED};
	DWORD dwRes;

	if (getSetting(hContact, szSetting, &dbv))
		return dwDef; // not found, give default

	if (dbv.type != DBVT_DWORD)
		dwRes = dwDef; // invalid type, give default
	else // found and valid, give result
		dwRes = dbv.dVal;

	db_free(&dbv);
	return dwRes;
}

double CIcqProto::getSettingDouble(HANDLE hContact, const char *szSetting, double dDef)
{
	DBVARIANT dbv = {DBVT_DELETED};
	double dRes;

	if (getSetting(hContact, szSetting, &dbv))
		return dDef; // not found, give default

	if (dbv.type != DBVT_BLOB || dbv.cpbVal != sizeof(double))
		dRes = dDef;
	else
		dRes = *(double*)dbv.pbVal;

	db_free(&dbv);
	return dRes;
}

DWORD CIcqProto::getContactUin(HANDLE hContact)
{
	return getSettingDword(hContact, UNIQUEIDSETTING, 0);
}

int CIcqProto::getContactUid(HANDLE hContact, DWORD *pdwUin, uid_str *ppszUid)
{
	DBVARIANT dbv = {DBVT_DELETED};
	int iRes = 1;

	*pdwUin = 0;
	if (ppszUid) *ppszUid[0] = '\0';

	if (!getSetting(hContact, UNIQUEIDSETTING, &dbv)) {
		if (dbv.type == DBVT_DWORD) {
			*pdwUin = dbv.dVal;
			iRes = 0;
		}
		else if (dbv.type == DBVT_ASCIIZ) {
			if (ppszUid && m_bAimEnabled) {
				strcpy(*ppszUid, dbv.pszVal);
				iRes = 0;
			}
			else
				NetLog_Server("AOL screennames not accepted");
		}
		db_free(&dbv);
	}
	return iRes;
}

int CIcqProto::getSettingString(HANDLE hContact, const char *szSetting, DBVARIANT *dbv)
{
	int res = db_get_s(hContact, m_szModuleName, szSetting, dbv);
	if (res)
		db_free(dbv);

	return res;
}

int CIcqProto::getSettingStringW(HANDLE hContact, const char *szSetting, DBVARIANT *dbv)
{
	int res = db_get_ws(hContact, m_szModuleName, szSetting, dbv);
	if (res)
		db_free(dbv);

	return res;
}

char* CIcqProto::getSettingStringUtf(HANDLE hContact, const char *szModule, const char *szSetting, char *szDef)
{
	DBVARIANT dbv = {DBVT_DELETED};
	if ( db_get_utf(hContact, szModule, szSetting, &dbv)) {
		db_free(&dbv); // for a setting with invalid contents/type
		return null_strdup(szDef);
	}

	char *szRes = null_strdup(dbv.pszVal);
	db_free(&dbv);
	return szRes;
}

char* CIcqProto::getSettingStringUtf(HANDLE hContact, const char *szSetting, char *szDef)
{
	return getSettingStringUtf(hContact, m_szModuleName, szSetting, szDef);
}

WORD CIcqProto::getContactStatus(HANDLE hContact)
{
	return getSettingWord(hContact, "Status", ID_STATUS_OFFLINE);
}

int CIcqProto::getSettingStringStatic(HANDLE hContact, const char *szSetting, char *dest, int dest_len)
{
	DBVARIANT dbv = {DBVT_DELETED};
	DBCONTACTGETSETTING sVal = {0};

	dbv.pszVal = dest;
	dbv.cchVal = dest_len;
	dbv.type = DBVT_ASCIIZ;

	sVal.pValue = &dbv;
	sVal.szModule = m_szModuleName;
	sVal.szSetting = szSetting;

	if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&sVal) != 0) {
		dbv.pszVal = dest;
		dbv.cchVal = dest_len;
		dbv.type = DBVT_UTF8;

		if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&sVal) != 0)
			return 1; // nothing found
	}

	return (dbv.type != DBVT_ASCIIZ);
}

int CIcqProto::deleteSetting(HANDLE hContact, const char *szSetting)
{
	return db_unset(hContact, m_szModuleName, szSetting);
}

int CIcqProto::setSettingByte(HANDLE hContact, const char *szSetting, BYTE byValue)
{
	return db_set_b(hContact, m_szModuleName, szSetting, byValue);
}

int CIcqProto::setSettingWord(HANDLE hContact, const char *szSetting, WORD wValue)
{
	return db_set_w(hContact, m_szModuleName, szSetting, wValue);
}

int CIcqProto::setSettingDword(HANDLE hContact, const char *szSetting, DWORD dwValue)
{
	return db_set_dw(hContact, m_szModuleName, szSetting, dwValue);
}

int CIcqProto::setSettingDouble(HANDLE hContact, const char *szSetting, double dValue)
{
	return setSettingBlob(hContact, szSetting, (BYTE*)&dValue, sizeof(double));
}

int CIcqProto::setSettingString(HANDLE hContact, const char *szSetting, const char *szValue)
{
	return db_set_s(hContact, m_szModuleName, szSetting, szValue);
}

int CIcqProto::setSettingStringW(HANDLE hContact, const char *szSetting, const WCHAR *wszValue)
{
	return db_set_ws(hContact, m_szModuleName, szSetting, wszValue);
}

int CIcqProto::setSettingStringUtf(HANDLE hContact, const char *szModule, const char *szSetting, const char *szValue)
{
	return db_set_utf(hContact, szModule, szSetting, szValue);
}

int CIcqProto::setSettingStringUtf(HANDLE hContact, const char *szSetting, const char *szValue)
{
	return db_set_utf(hContact, m_szModuleName, szSetting, szValue);
}

int CIcqProto::setSettingBlob(HANDLE hContact, const char *szSetting, const BYTE *pValue, const int cbValue)
{
	return db_set_blob(hContact, m_szModuleName, szSetting, (void*)pValue, cbValue);
}

int CIcqProto::setContactHidden(HANDLE hContact, BYTE bHidden)
{
	int nResult = db_set_b(hContact, "CList", "Hidden", bHidden);
	if (!bHidden) // clear zero setting
		db_unset(hContact, "CList", "Hidden");

	return nResult;
}

void CIcqProto::setStatusMsgVar(HANDLE hContact, char* szStatusMsg, bool isAnsi)
{
	if (szStatusMsg && szStatusMsg[0]) {
		if (isAnsi) {
			char* szStatusNote = getSettingStringUtf(hContact, DBSETTING_STATUS_NOTE, "");
			wchar_t* szStatusNoteW = make_unicode_string(szStatusNote);
			int len = (int)wcslen(szStatusNoteW) * 3 + 1;
			char* szStatusNoteAnsi = (char*)alloca(len);
			WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, szStatusNoteW, -1, szStatusNoteAnsi, len, NULL, NULL);
			bool notmatch = false;
			for (int i=0; ;++i) {
				if (szStatusNoteAnsi[i] != szStatusMsg[i] && szStatusNoteAnsi[i] != '?' && szStatusMsg[i] != '?') {
					notmatch = true;
					break;
				}
				if (!szStatusNoteAnsi[i] || !szStatusMsg[i])
					break;
			}
			szStatusMsg = notmatch ? ansi_to_utf8(szStatusMsg) : szStatusNote;
			SAFE_FREE(&szStatusNoteW);
			if (notmatch)
				SAFE_FREE(&szStatusNote);
		}

		char* oldStatusMsg = NULL;
		DBVARIANT dbv;
		if ( !db_get_ts(hContact, "CList", "StatusMsg", &dbv)) {
			oldStatusMsg = make_utf8_string(dbv.ptszVal);
			db_free(&dbv);
		}

		if (!oldStatusMsg || strcmp(oldStatusMsg, szStatusMsg))
			setSettingStringUtf(hContact, "CList", "StatusMsg", szStatusMsg);
		SAFE_FREE(&oldStatusMsg);
		if (isAnsi) SAFE_FREE(&szStatusMsg);
	}
	else db_unset(hContact, "CList", "StatusMsg");
}

int CIcqProto::IsICQContact(HANDLE hContact)
{
	char* szProto = GetContactProto(hContact);

	return !strcmpnull(szProto, m_szModuleName);
}

HANDLE CIcqProto::AddEvent(HANDLE hContact, WORD wType, DWORD dwTime, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = dwTime;
	dbei.flags = flags;
	dbei.eventType = wType;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	return db_event_add(hContact, &dbei);
}

HANDLE CIcqProto::FindFirstContact()
{
	HANDLE hContact = db_find_first(m_szModuleName);

	if (IsICQContact(hContact))
		return hContact;

	return FindNextContact(hContact);
}

HANDLE CIcqProto::FindNextContact(HANDLE hContact)
{
	hContact = db_find_next(hContact, m_szModuleName);
	while (hContact != NULL)
	{
		if (IsICQContact(hContact))
			return hContact;
		hContact = db_find_next(hContact, m_szModuleName);
	}
	return hContact;
}

char* CIcqProto::getContactCListGroup(HANDLE hContact)
{
	return getSettingStringUtf(hContact, "CList", "Group", NULL);
}
