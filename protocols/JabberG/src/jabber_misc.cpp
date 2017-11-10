/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

#include "stdafx.h"
#include "jabber_list.h"
#include "jabber_caps.h"

///////////////////////////////////////////////////////////////////////////////
// JabberAddContactToRoster() - adds a contact to the roster

void CJabberProto::AddContactToRoster(const wchar_t *jid, const wchar_t *nick, const wchar_t *grpName)
{
	XmlNodeIq iq(L"set", SerialNext());
	HXML query = iq << XQUERY(JABBER_FEAT_IQ_ROSTER)
		<< XCHILD(L"item") << XATTR(L"jid", jid) << XATTR(L"name", nick);
	if (grpName)
		query << XCHILD(L"group", grpName);
	m_ThreadInfo->send(iq);
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBAddAuthRequest()

void CJabberProto::DBAddAuthRequest(const wchar_t *jid, const wchar_t *nick)
{
	MCONTACT hContact = DBCreateContact(jid, nick, true, true);
	delSetting(hContact, "Hidden");

	DB_AUTH_BLOB blob(hContact, T2Utf(nick), nullptr, nullptr, T2Utf(jid), nullptr);

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (DWORD)time(nullptr);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = blob.size();
	dbei.pBlob = blob;
	db_event_add(0, &dbei);
	debugLogA("Setup DBAUTHREQUEST with nick='%s' jid='%s'", blob.get_nick(), blob.get_email());
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBCreateContact()

MCONTACT CJabberProto::DBCreateContact(const wchar_t *jid, const wchar_t *nick, bool temporary, bool stripResource)
{
	if (jid == nullptr || jid[0] == '\0')
		return 0;

	MCONTACT hContact = HContactFromJID(jid, stripResource);
	if (hContact != 0)
		return hContact;

	// strip resource if present
	wchar_t szJid[JABBER_MAX_JID_LEN];
	if (stripResource)
		JabberStripJid(jid, szJid, _countof(szJid));
	else
		wcsncpy_s(szJid, jid, _TRUNCATE);

	MCONTACT hNewContact = db_add_contact();
	Proto_AddToContact(hNewContact, m_szModuleName);
	setWString(hNewContact, "jid", szJid);
	if (nick != nullptr && *nick != '\0')
		setWString(hNewContact, "Nick", nick);
	if (temporary)
		db_set_b(hNewContact, "CList", "NotOnList", 1);
	else
		SendGetVcard(szJid);
	
	if (JABBER_LIST_ITEM *pItem = ListAdd(LIST_ROSTER, jid, hNewContact))
		pItem->bUseResource = wcschr(szJid, '/') != 0;
	
	debugLogW(L"Create Jabber contact jid=%s, nick=%s", szJid, nick);
	DBCheckIsTransportedContact(szJid, hNewContact);
	return hNewContact;
}

BOOL CJabberProto::AddDbPresenceEvent(MCONTACT hContact, BYTE btEventType)
{
	if (!hContact)
		return FALSE;

	switch (btEventType) {
	case JABBER_DB_EVENT_PRESENCE_SUBSCRIBE:
	case JABBER_DB_EVENT_PRESENCE_SUBSCRIBED:
	case JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBE:
	case JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBED:
		if (!m_options.LogPresence)
			return FALSE;
		break;

	case JABBER_DB_EVENT_PRESENCE_ERROR:
		if (!m_options.LogPresenceErrors)
			return FALSE;
		break;
	}

	DBEVENTINFO dbei = {};
	dbei.pBlob = &btEventType;
	dbei.cbBlob = sizeof(btEventType);
	dbei.eventType = EVENTTYPE_JABBER_PRESENCE;
	dbei.flags = DBEF_READ;
	dbei.timestamp = time(nullptr);
	dbei.szModule = m_szModuleName;
	db_event_add(hContact, &dbei);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarFileName() - gets a file name for the avatar image

void CJabberProto::GetAvatarFileName(MCONTACT hContact, wchar_t* pszDest, size_t cbLen)
{
	int tPathLen = mir_snwprintf(pszDest, cbLen, L"%s\\%S", VARSW(L"%miranda_avatarcache%"), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(pszDest);

	pszDest[tPathLen++] = '\\';

	const wchar_t* szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_PNG));

	if (hContact != 0) {
		char str[256];
		JabberShaStrBuf buf;
		DBVARIANT dbv;
		if (!db_get_utf(hContact, m_szModuleName, "jid", &dbv)) {
			strncpy_s(str, dbv.pszVal, _TRUNCATE);
			str[sizeof(str) - 1] = 0;
			db_free(&dbv);
		}
		else _i64toa((LONG_PTR)hContact, str, 10);
		mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%S%s", JabberSha1(str, buf), szFileType);
	}
	else if (m_ThreadInfo != nullptr) {
		mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%s@%S avatar%s",
			m_ThreadInfo->conn.username, m_ThreadInfo->conn.server, szFileType);
	}
	else {
		ptrA res1(getStringA("LoginName")), res2(getStringA("LoginServer"));
		mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%S@%S avatar%s",
			(res1) ? (LPSTR)res1 : "noname", (res2) ? (LPSTR)res2 : m_szModuleName, szFileType);
	}
}

///////////////////////////////////////////////////////////////////////////////
// JabberResolveTransportNicks - massive vcard update

void CJabberProto::ResolveTransportNicks(const wchar_t *jid)
{
	// Set all contacts to offline
	MCONTACT hContact = m_ThreadInfo->resolveContact;
	if (hContact == 0)
		hContact = db_find_first(m_szModuleName);

	for (; hContact != 0; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!getByte(hContact, "IsTransported", 0))
			continue;

		ptrW dbJid(getWStringA(hContact, "jid")); if (dbJid == nullptr) continue;
		ptrW dbNick(getWStringA(hContact, "Nick")); if (dbNick == nullptr) continue;

		wchar_t *p = wcschr(dbJid, '@');
		if (p == nullptr)
			continue;

		*p = 0;
		if (!mir_wstrcmp(jid, p + 1) && !mir_wstrcmp(dbJid, dbNick)) {
			*p = '@';
			m_ThreadInfo->resolveID = SendGetVcard(dbJid);
			m_ThreadInfo->resolveContact = hContact;
			return;
		}
	}

	m_ThreadInfo->resolveID = -1;
	m_ThreadInfo->resolveContact = 0;
}

///////////////////////////////////////////////////////////////////////////////
// JabberSetServerStatus()

void CJabberProto::SetServerStatus(int iNewStatus)
{
	if (!m_bJabberOnline)
		return;

	// change status
	int oldStatus = m_iStatus;
	switch (iNewStatus) {
	case ID_STATUS_ONLINE:
	case ID_STATUS_NA:
	case ID_STATUS_FREECHAT:
	case ID_STATUS_INVISIBLE:
		m_iStatus = iNewStatus;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		m_iStatus = ID_STATUS_AWAY;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		m_iStatus = ID_STATUS_DND;
		break;
	default:
		return;
	}

	if (m_iStatus == oldStatus)
		return;

	// send presence update
	SendPresence(m_iStatus, true);
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}

//////////////////////////////////////////////////////////////////////////
// update MirVer with data for active resource

struct
{
	wchar_t *node;
	wchar_t *name;
}
static sttCapsNodeToName_Map[] =
{
	{ L"http://miranda-im.org",  L"Miranda IM Jabber" },
	{ L"http://miranda-ng.org",  L"Miranda NG Jabber" },
	{ L"http://www.google.com",  L"GTalk" },
	{ L"http://mail.google.com", L"GMail" },
	{ L"http://www.android.com", L"Android" },
	{ L"http://qip.ru",          L"QIP 2012" },
	{ L"http://2010.qip.ru",     L"QIP 2010"}
};

const wchar_t* CJabberProto::GetSoftName(const wchar_t *wszName)
{
	// search through known software list
	for (int i = 0; i < _countof(sttCapsNodeToName_Map); i++)
		if (wcsstr(wszName, sttCapsNodeToName_Map[i].node))
			return sttCapsNodeToName_Map[i].name;

	return nullptr;
}

void CJabberProto::UpdateMirVer(JABBER_LIST_ITEM *item)
{
	MCONTACT hContact = HContactFromJID(item->jid);
	if (!hContact)
		return;

	debugLogW(L"JabberUpdateMirVer: for jid %s", item->jid);

	if (item->resourceMode == RSMODE_LASTSEEN)
		UpdateMirVer(hContact, pResourceStatus(item->m_pLastSeenResource));
	else if (item->resourceMode == RSMODE_MANUAL)
		UpdateMirVer(hContact, pResourceStatus(item->m_pManualResource));
}

void CJabberProto::FormatMirVer(const pResourceStatus &resource, CMStringW &res)
{
	res.Empty();
	if (resource == nullptr)
		return;

	// no caps info? set MirVer = resource name
	if (resource->m_pCaps == nullptr) {
		debugLogW(L"JabberUpdateMirVer: for rc %s: %s", resource->m_tszResourceName, resource->m_tszResourceName);
		if (resource->m_tszResourceName)
			res = resource->m_tszResourceName;
	}
	// XEP-0115 caps mode
	else {
		CJabberClientPartialCaps *pCaps = resource->m_pCaps;
		debugLogW(L"JabberUpdateMirVer: for rc %s: %s#%s", resource->m_tszResourceName, pCaps->GetNode(), pCaps->GetHash());

		// unknown software
		const wchar_t *szDefaultName = GetSoftName(pCaps->GetNode());
		res.Format(L"%s %s", (szDefaultName == nullptr) ? pCaps->GetSoft() : szDefaultName, pCaps->GetSoftVer());

		if (pCaps->GetSoftMir())
			res.AppendFormat(L" %s", pCaps->GetSoftMir());
	}

	// attach additional info for fingerprint plguin
	if (resource->m_tszCapsExt) {
		if (wcsstr(resource->m_tszCapsExt, JABBER_EXT_PLATFORMX86) && !wcsstr(res, L"x86"))
			res.Append(L" x86");

		if (wcsstr(resource->m_tszCapsExt, JABBER_EXT_PLATFORMX64) && !wcsstr(res, L"x64"))
			res.Append(L" x64");

		if (wcsstr(resource->m_tszCapsExt, JABBER_EXT_SECUREIM) && !wcsstr(res, L"(SecureIM)"))
			res.Append(L" (SecureIM)");

		if (wcsstr(resource->m_tszCapsExt, JABBER_EXT_MIROTR) && !wcsstr(res, L"(MirOTR)"))
			res.Append(L" (MirOTR)");

		if (wcsstr(resource->m_tszCapsExt, JABBER_EXT_NEWGPG) && !wcsstr(res, L"(New_GPG)"))
			res.Append(L" (New_GPG)");

		if (wcsstr(resource->m_tszCapsExt, JABBER_EXT_OMEMO) && !wcsstr(res, L"(omemo)"))
			res.Append(L" (omemo)");
	}

	if (resource->m_tszResourceName && !wcsstr(res, resource->m_tszResourceName))
		if (wcsstr(res, L"Miranda IM") || wcsstr(res, L"Miranda NG") || m_options.ShowForeignResourceInMirVer)
			res.AppendFormat(L" [%s]", resource->m_tszResourceName);
}

void CJabberProto::UpdateMirVer(MCONTACT hContact, const pResourceStatus &r)
{
	if (r == nullptr)
		return;

	CMStringW tszMirVer;
	FormatMirVer(r, tszMirVer);
	if (!tszMirVer.IsEmpty())
		setWString(hContact, "MirVer", tszMirVer);

	ptrW jid(getWStringA(hContact, "jid"));
	if (jid == nullptr)
		return;

	wchar_t szFullJid[JABBER_MAX_JID_LEN];
	if (r->m_tszResourceName && !wcschr(jid, '/'))
		mir_snwprintf(szFullJid, L"%s/%s", jid, r->m_tszResourceName);
	else
		mir_wstrncpy(szFullJid, jid, _countof(szFullJid));
	setWString(hContact, DBSETTING_DISPLAY_UID, szFullJid);
}

void CJabberProto::UpdateSubscriptionInfo(MCONTACT hContact, JABBER_LIST_ITEM *item)
{
	switch (item->subscription) {
	case SUB_TO:
		setWString(hContact, "SubscriptionText", TranslateT("To"));
		setString(hContact, "Subscription", "to");
		setByte(hContact, "Auth", 0);
		setByte(hContact, "Grant", 1);
		break;
	case SUB_FROM:
		setWString(hContact, "SubscriptionText", TranslateT("From"));
		setString(hContact, "Subscription", "from");
		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 0);
		break;
	case SUB_BOTH:
		setWString(hContact, "SubscriptionText", TranslateT("Both"));
		setString(hContact, "Subscription", "both");
		setByte(hContact, "Auth", 0);
		setByte(hContact, "Grant", 0);
		break;
	case SUB_NONE:
		setWString(hContact, "SubscriptionText", TranslateT("None"));
		setString(hContact, "Subscription", "none");
		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 1);
		break;
	}
}

void CJabberProto::SetContactOfflineStatus(MCONTACT hContact)
{
	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
		setWord(hContact, "Status", ID_STATUS_OFFLINE);

	delSetting(hContact, DBSETTING_XSTATUSID);
	delSetting(hContact, DBSETTING_XSTATUSNAME);
	delSetting(hContact, DBSETTING_XSTATUSMSG);
	delSetting(hContact, DBSETTING_DISPLAY_UID);

	ResetAdvStatus(hContact, ADVSTATUS_MOOD);
	ResetAdvStatus(hContact, ADVSTATUS_TUNE);
}

void CJabberProto::InitPopups(void)
{
	wchar_t desc[256];
	mir_snwprintf(desc, L"%s %s", m_tszUserName, TranslateT("Errors"));

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = LoadIconEx("main");
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClass = Popup_RegisterClass(&ppc);

	IcoLib_ReleaseIcon(ppc.hIcon);
}

void CJabberProto::MsgPopup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		char name[256];

		POPUPDATACLASS ppd = { sizeof(ppd) };
		ppd.pwszTitle = szTitle;
		ppd.pwszText = szMsg;
		ppd.pszClassName = name;
		ppd.hContact = hContact;
		mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
	}
	else {
		DWORD mtype = MB_OK | MB_SETFOREGROUND | MB_ICONSTOP;
		MessageBox(nullptr, szMsg, szTitle, mtype);
	}
}

CMStringW CJabberProto::ExtractImage(HXML node)
{
	HXML nHtml, nBody, nImg;
	LPCTSTR src;
	CMStringW link;

	if ((nHtml = XmlGetChild(node, "html")) != nullptr &&
		(nBody = XmlGetChild(nHtml, "body")) != nullptr &&
		(nImg = XmlGetChild(nBody, "img")) != nullptr &&
		(src = XmlGetAttrValue(nImg, L"src")) != nullptr) {

		CMStringW strSrc(src);
		if (strSrc.Left(11).Compare(L"data:image/") == 0) {
			int end = strSrc.Find(L';');
			if (end != -1) {
				CMStringW ext(strSrc.c_str() + 11, end - 11);
				int comma = strSrc.Find(L',', end);
				if (comma != -1) {
					CMStringW image(strSrc.c_str() + comma + 1, strSrc.GetLength() - comma - 1);
					image.Replace(L"%2B", L"+");
					image.Replace(L"%2F", L"/");
					image.Replace(L"%3D", L"=");

					wchar_t tszTempPath[MAX_PATH], tszTempFile[MAX_PATH];
					GetTempPath(_countof(tszTempPath), tszTempPath);
					GetTempFileName(tszTempPath, L"jab", InterlockedIncrement(&g_nTempFileId), tszTempFile);
					wcsncat_s(tszTempFile, L".", 1);
					wcsncat_s(tszTempFile, ext, ext.GetLength());

					HANDLE h = CreateFile(tszTempFile, GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, nullptr);

					if (h != INVALID_HANDLE_VALUE) {
						DWORD n;
						unsigned int bufferLen;
						ptrA buffer((char*)mir_base64_decode(_T2A(image), &bufferLen));
						WriteFile(h, buffer, bufferLen, &n, nullptr);
						CloseHandle(h);

						link = L" file:///";
						link += tszTempFile;
					}
				}
			}
		}
	}
	return link;
}
