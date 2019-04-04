/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-19 Miranda NG team

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

void CJabberProto::AddContactToRoster(const char *jid, const char *nick, const char *grpName)
{
	XmlNodeIq iq("set", SerialNext());
	TiXmlElement *query = iq << XQUERY(JABBER_FEAT_IQ_ROSTER)
		<< XCHILD("item") << XATTR("jid", jid) << XATTR("name", nick);
	if (grpName)
		query << XCHILD("group", grpName);
	m_ThreadInfo->send(iq);
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBAddAuthRequest()

void CJabberProto::DBAddAuthRequest(const char *jid, const char *nick)
{
	MCONTACT hContact = DBCreateContact(jid, nick, true, true);
	delSetting(hContact, "Hidden");

	DB_AUTH_BLOB blob(hContact, nick, nullptr, nullptr, jid, nullptr);

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (DWORD)time(0);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = blob.size();
	dbei.pBlob = blob;
	db_event_add(0, &dbei);
	debugLogA("Setup DBAUTHREQUEST with nick='%s' jid='%s'", blob.get_nick(), blob.get_email());
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBCreateContact()

MCONTACT CJabberProto::DBCreateContact(const char *jid, const char *nick, bool temporary, bool stripResource)
{
	if (jid == nullptr || jid[0] == '\0')
		return 0;

	MCONTACT hContact = HContactFromJID(jid, stripResource);
	if (hContact != 0)
		return hContact;

	// strip resource if present
	char szJid[JABBER_MAX_JID_LEN];
	if (stripResource)
		JabberStripJid(jid, szJid, _countof(szJid));
	else
		strncpy_s(szJid, jid, _TRUNCATE);

	MCONTACT hNewContact = db_add_contact();
	Proto_AddToContact(hNewContact, m_szModuleName);
	setUString(hNewContact, "jid", szJid);
	if (nick != nullptr && *nick != '\0')
		setUString(hNewContact, "Nick", nick);
	if (temporary)
		db_set_b(hNewContact, "CList", "NotOnList", 1);
	else
		SendGetVcard(szJid);
	
	if (JABBER_LIST_ITEM *pItem = ListAdd(LIST_ROSTER, jid, hNewContact))
		pItem->bUseResource = strchr(szJid, '/') != nullptr;
	
	debugLogA("Create Jabber contact jid=%s, nick=%s", szJid, nick);
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
		if (!m_bLogPresence)
			return FALSE;
		break;

	case JABBER_DB_EVENT_PRESENCE_ERROR:
		if (!m_bLogPresenceErrors)
			return FALSE;
		break;
	}

	DBEVENTINFO dbei = {};
	dbei.pBlob = &btEventType;
	dbei.cbBlob = sizeof(btEventType);
	dbei.eventType = EVENTTYPE_JABBER_PRESENCE;
	dbei.flags = DBEF_READ;
	dbei.timestamp = time(0);
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
		mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%s@%s avatar%s",
			Utf2T(m_ThreadInfo->conn.username).get(), Utf2T(m_ThreadInfo->conn.server).get(), szFileType);
	}
	else {
		ptrA res1(getStringA("LoginName")), res2(getStringA("LoginServer"));
		mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%S@%S avatar%s",
			(res1) ? (LPSTR)res1 : "noname", (res2) ? (LPSTR)res2 : m_szModuleName, szFileType);
	}
}

///////////////////////////////////////////////////////////////////////////////
// JabberResolveTransportNicks - massive vcard update

void CJabberProto::ResolveTransportNicks(const char *jid)
{
	// Set all contacts to offline
	MCONTACT hContact = m_ThreadInfo->resolveContact;
	if (hContact == 0)
		hContact = db_find_first(m_szModuleName);

	for (; hContact != 0; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!getByte(hContact, "IsTransported", 0))
			continue;

		ptrA dbJid(getUStringA(hContact, "jid")); if (dbJid == nullptr) continue;
		ptrA dbNick(getUStringA(hContact, "Nick")); if (dbNick == nullptr) continue;

		char *p = strchr(dbJid, '@');
		if (p == nullptr)
			continue;

		*p = 0;
		if (!mir_strcmp(jid, p + 1) && !mir_strcmp(dbJid, dbNick)) {
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
	char *node;
	char *name;
}
static sttCapsNodeToName_Map[] =
{
	{ "http://miranda-im.org",   "Miranda IM Jabber" },
	{ "http://miranda-ng.org",   "Miranda NG Jabber" },
	{ "http://www.google.com",   "GTalk" },
	{ "http://mail.google.com",  "GMail" },
	{ "http://www.android.com",  "Android" },
	{ "http://qip.ru",           "QIP 2012" },
	{ "http://2010.qip.ru",      "QIP 2010"},
	{ "http://conversations.im", "Conversations IM" }
};

const char* CJabberProto::GetSoftName(const char *szName)
{
	// search through known software list
	for (auto &it : sttCapsNodeToName_Map)
		if (strstr(szName, it.node))
			return it.name;

	return nullptr;
}

void CJabberProto::UpdateMirVer(JABBER_LIST_ITEM *item)
{
	MCONTACT hContact = HContactFromJID(item->jid);
	if (!hContact)
		return;

	debugLogA("JabberUpdateMirVer: for jid %s", item->jid);

	if (item->resourceMode == RSMODE_LASTSEEN)
		UpdateMirVer(hContact, pResourceStatus(item->m_pLastSeenResource));
	else if (item->resourceMode == RSMODE_MANUAL)
		UpdateMirVer(hContact, pResourceStatus(item->m_pManualResource));
}

void CJabberProto::FormatMirVer(const pResourceStatus &resource, CMStringA &res)
{
	res.Empty();
	if (resource == nullptr)
		return;

	// no caps info? set MirVer = resource name
	if (resource->m_pCaps == nullptr) {
		debugLogA("JabberUpdateMirVer: for rc %s: %s", resource->m_szResourceName, resource->m_szResourceName);
		if (resource->m_szResourceName)
			res = resource->m_szResourceName;
	}
	// XEP-0115 caps mode
	else {
		CJabberClientPartialCaps *pCaps = resource->m_pCaps;
		debugLogA("JabberUpdateMirVer: for rc %s: %s#%s", resource->m_szResourceName, pCaps->GetNode(), pCaps->GetHash());

		// unknown software
		const char *szDefaultName = GetSoftName(pCaps->GetNode());
		res = (szDefaultName == nullptr) ? pCaps->GetSoft() : szDefaultName;
		if (pCaps->GetSoftVer())
			res.AppendFormat(" %s", pCaps->GetSoftVer());
		if (pCaps->GetSoftMir())
			res.AppendFormat(" %s", pCaps->GetSoftMir());
	}

	// attach additional info for fingerprint plguin
	if (resource->m_tszCapsExt) {
		if (strstr(resource->m_tszCapsExt, JABBER_EXT_PLATFORMX86) && !strstr(res, "x86"))
			res.Append(" x86");

		if (strstr(resource->m_tszCapsExt, JABBER_EXT_PLATFORMX64) && !strstr(res, "x64"))
			res.Append(" x64");

		if (strstr(resource->m_tszCapsExt, JABBER_EXT_SECUREIM) && !strstr(res, "(SecureIM)"))
			res.Append(" (SecureIM)");

		if (strstr(resource->m_tszCapsExt, JABBER_EXT_MIROTR) && !strstr(res, "(MirOTR)"))
			res.Append(" (MirOTR)");

		if (strstr(resource->m_tszCapsExt, JABBER_EXT_NEWGPG) && !strstr(res, "(New_GPG)"))
			res.Append(" (New_GPG)");

		if (strstr(resource->m_tszCapsExt, JABBER_EXT_OMEMO) && !strstr(res, "(omemo)"))
			res.Append(" (omemo)");
	}

	if (resource->m_szResourceName && !strstr(res, resource->m_szResourceName))
		if (strstr(res, "Miranda IM") || strstr(res, "Miranda NG") || m_bShowForeignResourceInMirVer)
			res.AppendFormat(" [%s]", resource->m_szResourceName);
}

void CJabberProto::UpdateMirVer(MCONTACT hContact, const pResourceStatus &r)
{
	if (r == nullptr)
		return;

	CMStringA tszMirVer;
	FormatMirVer(r, tszMirVer);
	if (!tszMirVer.IsEmpty())
		setUString(hContact, "MirVer", tszMirVer);

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid == nullptr)
		return;

	if (r->m_szResourceName && !strchr(jid, '/'))
		setUString(hContact, DBSETTING_DISPLAY_UID, MakeJid(jid, r->m_szResourceName));
	else
		setUString(hContact, DBSETTING_DISPLAY_UID, jid);
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
	char name[256];
	wchar_t desc[256];

	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_UNICODE;
	ppc.pszName = name;
	ppc.pszDescription.w = desc;

	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Errors"));
	ppc.hIcon = LoadIconEx("main");
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClass = Popup_RegisterClass(&ppc);

	IcoLib_ReleaseIcon(ppc.hIcon);
}

void CJabberProto::MsgPopup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle)
{
	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	CMStringW wszTitle(szTitle);
	if (hContact == 0) {
		wszTitle.Insert(0, L": ");
		wszTitle.Insert(0, m_tszUserName);
	}

	POPUPDATACLASS ppd = { sizeof(ppd) };
	ppd.szTitle.w = wszTitle;
	ppd.szText.w = szMsg;
	ppd.pszClassName = name;
	ppd.hContact = hContact;
	Popup_AddClass(&ppd);
}

CMStringA CJabberProto::ExtractImage(const TiXmlElement *node)
{
	const TiXmlElement *nHtml, *nBody, *nImg;
	const char *src;
	CMStringA link;

	if ((nHtml = XmlFirstChild(node, "html")) != nullptr &&
		(nBody = XmlFirstChild(nHtml, "body")) != nullptr &&
		(nImg = XmlFirstChild(nBody, "img")) != nullptr &&
		(src = XmlGetAttr(nImg, "src")) != nullptr) {

		CMStringA strSrc(src);
		if (strSrc.Left(11).Compare("data:image/") == 0) {
			int end = strSrc.Find(';');
			if (end != -1) {
				CMStringW ext(strSrc.c_str() + 11, end - 11);
				int comma = strSrc.Find(L',', end);
				if (comma != -1) {
					CMStringA image(strSrc.c_str() + comma + 1, strSrc.GetLength() - comma - 1);
					image.Replace("%2B", "+");
					image.Replace("%2F", "/");
					image.Replace("%3D", "=");

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
						size_t bufferLen;
						ptrA buffer((char*)mir_base64_decode(image, &bufferLen));
						WriteFile(h, buffer, (DWORD)bufferLen, &n, nullptr);
						CloseHandle(h);

						link = " file:///";
						link += T2Utf(tszTempFile);
					}
				}
			}
		}
	}
	return link;
}
