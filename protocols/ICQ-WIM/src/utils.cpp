// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-23 Miranda NG team
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

#include "stdafx.h"

void CIcqProto::InitContactCache()
{
	mir_cslock l(m_csCache);
	for (auto &it : AccContacts()) {
		if (isChatRoom(it))
			continue;

		// that was previously an ICQ contact
		ptrW wszUin(GetUIN(it));
		if (wszUin != nullptr) {
			delSetting(it, "UIN");
			setWString(it, DB_KEY_ID, wszUin);
		}
		// that was previously a MRA contact
		else {
			CMStringW wszEmail(getMStringW(it, "e-mail"));
			if (!wszEmail.IsEmpty()) {
				delSetting(it, "e-mail");
				setWString(it, DB_KEY_ID, wszEmail);
			}
		}

		CMStringW wszId = GetUserId(it);
		auto *pUser = FindUser(wszId);
		if (pUser == nullptr) {
			pUser = new IcqUser(wszId, it);

			mir_cslock lck(m_csCache);
			m_arCache.insert(pUser);
		}
		pUser->m_iProcessedMsgId = getId(it, DB_KEY_LASTMSGID);
	}
}

IcqUser* CIcqProto::FindUser(const CMStringW &wszId)
{
	IcqUser tmp(wszId, -1);

	mir_cslock l(m_csCache);
	return m_arCache.find(&tmp);
}

wchar_t* CIcqProto::GetUIN(MCONTACT hContact)
{
	DBVARIANT dbv = {};
	if (!db_get(hContact, m_szModuleName, "UIN", &dbv)) {
		switch (dbv.type) {
		case DBVT_DWORD:
			wchar_t buf[40], *ret;
			_itow_s(dbv.dVal, buf, 10);
			return mir_wstrdup(buf);

		case DBVT_ASCIIZ:
			ret = mir_a2u(dbv.pszVal);
			db_free(&dbv);
			return ret;

		case DBVT_UTF8:
			ret = mir_utf8decodeW(dbv.pszVal);
			db_free(&dbv);
			return ret;

		case DBVT_WCHAR:
			return dbv.pwszVal;
		}
		db_free(&dbv);
	}
	return nullptr;
}

MCONTACT CIcqProto::CreateContact(const CMStringW &wszId, bool bTemporary)
{
	auto *pUser = FindUser(wszId);
	if (pUser != nullptr)
		return pUser->m_hContact;

	MCONTACT hContact = db_add_contact();
	setWString(hContact, DB_KEY_ID, wszId);
	Proto_AddToContact(hContact, m_szModuleName);
	RetrieveUserInfo(hContact);

	if (bTemporary)
		Contact::RemoveFromList(hContact);

	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::CalcHash(AsyncHttpRequest *pReq)
{
	CMStringA hashData(FORMAT, "%s&%s&%s", 
		pReq->requestType == REQUEST_POST ? "POST" : "GET",
		mir_urlEncode(pReq->m_szUrl).c_str(), mir_urlEncode(pReq->m_szParam).c_str());

	unsigned int len;
	uint8_t hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), m_szSessionKey, m_szSessionKey.GetLength(), (uint8_t*)hashData.c_str(), hashData.GetLength(), hashOut, &len);
	pReq << CHAR_PARAM("sig_sha256", ptrA(mir_base64_encode(hashOut, sizeof(hashOut))));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::Json2int(MCONTACT hContact, const JSONNode &node, const char *szJson, const char *szSetting, bool bIsPartial)
{
	const JSONNode &var = node[szJson];
	if (var)
		setDword(hContact, szSetting, var.as_int());
	else if (!bIsPartial)
		delSetting(hContact, szSetting);
}

void CIcqProto::Json2string(MCONTACT hContact, const JSONNode &node, const char *szJson, const char *szSetting, bool bIsPartial)
{
	const JSONNode &var = node[szJson];
	if (var) {
		CMStringW wszStr(var.as_mstring());
		if (wszStr == L"[deleted]") {
			setByte(hContact, "IcqDeleted", 1);
			Contact::PutOnList(hContact);
		}
		else setWString(hContact, szSetting, wszStr);
	}
	else if (!bIsPartial)
		delSetting(hContact, szSetting);
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW CIcqProto::GetUserId(MCONTACT hContact)
{
	return getMStringW(hContact, DB_KEY_ID);
}

bool IsChat(const CMStringW &aimid)
{
	return aimid.Right(11) == "@chat.agent";
}

bool IsValidType(const JSONNode &n)
{
	auto type = n["userType"].as_string();
	return type == "icq" || type == "aim" || type == "interop" || type == "";
}

/////////////////////////////////////////////////////////////////////////////////////////

int CIcqProto::StatusFromPresence(const JSONNode &presence, MCONTACT hContact)
{
	CMStringW wszStatus = presence["state"].as_mstring();
	int iStatus;
	if (wszStatus == L"online")
		iStatus = ID_STATUS_ONLINE;
	else if (wszStatus == L"offline")
		iStatus = ID_STATUS_OFFLINE;
	else if (wszStatus == L"n/a")
		iStatus = ID_STATUS_NA;
	else if (wszStatus == L"away")
		iStatus = ID_STATUS_AWAY;
	else if (wszStatus == L"occupied")
		iStatus = ID_STATUS_OCCUPIED;
	else if (wszStatus == L"dnd")
		iStatus = ID_STATUS_DND;
	else
		return -1;

	int iLastSeen = presence["lastseen"].as_int();
	if (iLastSeen != 0)
		setDword(hContact, DB_KEY_LASTSEEN, iLastSeen);
	else {
		if (getDword(hContact, DB_KEY_ONLINETS))
			iStatus = ID_STATUS_ONLINE;
		else {
			if (auto *pUser = FindUser(GetUserId(hContact))) {
				mir_cslock lck(m_csLastSeenQueue);
				m_arLastSeenQueue.insert(pUser);
				m_impl.m_lastSeen.Start(500);
			}
			return -1;
		}
	}

	return iStatus;
}

void CIcqProto::ProcessStatus(IcqUser *pUser, int iStatus)
{
	// major crutch dedicated to the official client behaviour to go offline
	// when its window gets closed. we change the status of a contact to the
	// first chosen one from options and initialize a timer
	if (iStatus == ID_STATUS_OFFLINE) {
		if (m_iTimeDiff1) {
			iStatus = m_iStatus1;
			pUser->m_timer1 = time(0);
		}
	}
	// if a client returns back online, we clear timers not to play with statuses anymore
	else pUser->m_timer1 = pUser->m_timer2 = 0;

	if (iStatus == ID_STATUS_ONLINE && !pUser->m_bGotCaps)
		RetrieveUserCaps(pUser);

	setWord(pUser->m_hContact, "Status", iStatus);
}

/////////////////////////////////////////////////////////////////////////////////////////

__int64 CIcqProto::getId(MCONTACT hContact, const char *szSetting)
{
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(hContact, m_szModuleName, szSetting, &dbv))
		return 0;

	__int64 result = (dbv.cpbVal == sizeof(__int64)) ? *(__int64*)dbv.pbVal : 0;
	db_free(&dbv);
	return result;
}

void CIcqProto::setId(MCONTACT hContact, const char *szSetting, __int64 iValue)
{
	__int64 oldVal = getId(hContact, szSetting);
	if (oldVal != iValue)
		db_set_blob(hContact, m_szModuleName, szSetting, &iValue, sizeof(iValue));
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW fileText2url(const CMStringW &wszText)
{
	return wszText.Mid(26);
}

/////////////////////////////////////////////////////////////////////////////////////////

wchar_t* time2text(time_t ts)
{
	if (ts == 0)
		return L"";

	static wchar_t buf[100];
	TimeZone_PrintTimeStamp(NULL, ts, L"D t", buf, _countof(buf), 0);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case WM_COMMAND:
		CIcqProto *ppro = (CIcqProto*)PUGetPluginData(hWnd);
		CallProtoService(ppro->m_szModuleName, PS_GOTO_INBOX);
		PUDeletePopup(hWnd);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CIcqProto::EmailNotification(const wchar_t *pwszText)
{
	POPUPDATAW Popup = {};
	Popup.lchIcon = g_plugin.getIcon(IDI_INBOX);
	wcsncpy_s(Popup.lpwzText, pwszText, _TRUNCATE);
	wcsncpy_s(Popup.lpwzContactName, m_tszUserName, _TRUNCATE);
	Popup.iSeconds = 20;
	Popup.PluginData = this;
	Popup.PluginWindowProc = PopupDlgProc;
	PUAddPopupW(&Popup);

	if (m_bUseTrayIcon) {
		char szServiceFunction[MAX_PATH];
		if (m_bLaunchMailbox)
			mir_snprintf(szServiceFunction, "%s%s", m_szModuleName, PS_GOTO_INBOX);
		else
			mir_snprintf(szServiceFunction, "%s%s", m_szModuleName, PS_DUMMY);

		int i = 0;
		while (auto *pcle = Clist_GetEvent(-1, i++))
			if (!mir_strcmp(pcle->pszService, szServiceFunction))
				return;

		CLISTEVENT cle = {};
		cle.hDbEvent = ICQ_FAKE_EVENT_ID;
		cle.moduleName = m_szModuleName;
		cle.hIcon = g_plugin.getIcon(IDI_INBOX);
		cle.flags = CLEF_UNICODE | CLEF_PROTOCOLGLOBAL;
		cle.pszService = szServiceFunction;
		cle.szTooltip.w = pwszText;
		g_clistApi.pfnAddEvent(&cle);
	}
}
