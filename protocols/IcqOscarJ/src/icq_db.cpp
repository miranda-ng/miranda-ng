// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
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
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Internal Database API
// -----------------------------------------------------------------------------

#include "stdafx.h"

int CIcqProto::getSetting(MCONTACT hContact, const char *szSetting, DBVARIANT *dbv)
{
	return db_get_s(hContact, m_szModuleName, szSetting, dbv, 0);
}

double CIcqProto::getSettingDouble(MCONTACT hContact, const char *szSetting, double dDef)
{
	DBVARIANT dbv = { DBVT_DELETED };
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

DWORD CIcqProto::getContactUin(MCONTACT hContact)
{
	return getDword(hContact, UNIQUEIDSETTING, 0);
}

int CIcqProto::getContactUid(MCONTACT hContact, DWORD *pdwUin, uid_str *ppszUid)
{
	DBVARIANT dbv = { DBVT_DELETED };
	int iRes = 1;

	*pdwUin = 0;
	if (ppszUid) *ppszUid[0] = '\0';

	if (!getSetting(hContact, UNIQUEIDSETTING, &dbv)) {
		switch (dbv.type) {
		case DBVT_DWORD:
			*pdwUin = dbv.dVal;
			iRes = 0;
			break;

		case DBVT_ASCIIZ:
			if (ppszUid && m_bAimEnabled) {
				mir_strcpy(*ppszUid, dbv.pszVal);
				iRes = 0;
			}
			else debugLogA("AOL screennames not accepted");
			break;

		case DBVT_UTF8:
			if (ppszUid && m_bAimEnabled) {
				mir_strcpy(*ppszUid, dbv.pszVal);
				mir_utf8decode(*ppszUid, NULL);
				iRes = 0;
			}
			else debugLogA("AOL screennames not accepted");
			break;
		}
		db_free(&dbv);
	}
	return iRes;
}

char* CIcqProto::getSettingStringUtf(MCONTACT hContact, const char *szModule, const char *szSetting, char *szDef)
{
	DBVARIANT dbv = { DBVT_DELETED };
	if (db_get_utf(hContact, szModule, szSetting, &dbv)) {
		db_free(&dbv); // for a setting with invalid contents/type
		return null_strdup(szDef);
	}

	char *szRes = null_strdup(dbv.pszVal);
	db_free(&dbv);
	return szRes;
}

char* CIcqProto::getSettingStringUtf(MCONTACT hContact, const char *szSetting, char *szDef)
{
	return getSettingStringUtf(hContact, m_szModuleName, szSetting, szDef);
}

WORD CIcqProto::getContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

int CIcqProto::getSettingStringStatic(MCONTACT hContact, const char *szSetting, char *dest, size_t dest_len)
{
	if (!db_get_static(hContact, m_szModuleName, szSetting, dest, (int)dest_len))
		return 0;

	return db_get_static_utf(hContact, m_szModuleName, szSetting, dest, (int)dest_len);
}

int CIcqProto::setSettingDouble(MCONTACT hContact, const char *szSetting, double dValue)
{
	return setSettingBlob(hContact, szSetting, (BYTE*)&dValue, sizeof(double));
}

int CIcqProto::setSettingBlob(MCONTACT hContact, const char *szSetting, const BYTE *pValue, size_t cbValue)
{
	return db_set_blob(hContact, m_szModuleName, szSetting, (void*)pValue, (unsigned)cbValue);
}

int CIcqProto::setContactHidden(MCONTACT hContact, BYTE bHidden)
{
	int nResult = db_set_b(hContact, "CList", "Hidden", bHidden);
	if (!bHidden) // clear zero setting
		db_unset(hContact, "CList", "Hidden");

	return nResult;
}

void CIcqProto::setStatusMsgVar(MCONTACT hContact, char* szStatusMsg, bool isAnsi)
{
	if (szStatusMsg && szStatusMsg[0]) {
		if (isAnsi) {
			char *szStatusNote = getSettingStringUtf(hContact, DBSETTING_STATUS_NOTE, "");
			wchar_t *szStatusNoteW = make_unicode_string(szStatusNote);
			int len = (int)wcslen(szStatusNoteW) * 3 + 1;
			char *szStatusNoteAnsi = (char*)alloca(len);
			WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, szStatusNoteW, -1, szStatusNoteAnsi, len, NULL, NULL);
			bool notmatch = false;
			for (int i = 0;; ++i) {
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

		char *oldStatusMsg = NULL;
		DBVARIANT dbv;
		if (!db_get_ts(hContact, "CList", "StatusMsg", &dbv)) {
			oldStatusMsg = make_utf8_string(dbv.ptszVal);
			db_free(&dbv);
		}

		if (!oldStatusMsg || mir_strcmp(oldStatusMsg, szStatusMsg))
			db_set_utf(hContact, "CList", "StatusMsg", szStatusMsg);
		SAFE_FREE(&oldStatusMsg);
		if (isAnsi) SAFE_FREE(&szStatusMsg);
	}
	else db_unset(hContact, "CList", "StatusMsg");
}

int CIcqProto::IsICQContact(MCONTACT hContact)
{
	char* szProto = GetContactProto(hContact);

	return !mir_strcmp(szProto, m_szModuleName);
}

MEVENT CIcqProto::AddEvent(MCONTACT hContact, WORD wType, DWORD dwTime, DWORD flags, size_t cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = dwTime;
	dbei.flags = flags;
	dbei.eventType = wType;
	dbei.cbBlob = (DWORD)cbBlob;
	dbei.pBlob = pBlob;
	return db_event_add(hContact, &dbei);
}

char* CIcqProto::getContactCListGroup(MCONTACT hContact)
{
	return getSettingStringUtf(hContact, "CList", "Group", NULL);
}
