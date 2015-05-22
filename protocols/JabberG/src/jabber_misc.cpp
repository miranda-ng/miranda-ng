/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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

#include "stdafx.h"
#include "jabber_list.h"
#include "jabber_caps.h"

///////////////////////////////////////////////////////////////////////////////
// JabberAddContactToRoster() - adds a contact to the roster

void CJabberProto::AddContactToRoster(const TCHAR *jid, const TCHAR *nick, const TCHAR *grpName)
{
	XmlNodeIq iq(_T("set"), SerialNext());
	HXML query = iq << XQUERY(JABBER_FEAT_IQ_ROSTER)
		<< XCHILD(_T("item")) << XATTR(_T("jid"), jid) << XATTR(_T("name"), nick);
	if (grpName)
		query << XCHILD(_T("group"), grpName);
	m_ThreadInfo->send(iq);
}

///////////////////////////////////////////////////////////////////////////////
// JabberChatDllError() - missing CHAT.DLL

void JabberChatDllError()
{
	MessageBox(NULL,
		TranslateT("CHAT plugin is required for conferences. Install it before chatting"),
		TranslateT("Jabber Error"), MB_OK|MB_SETFOREGROUND);
}

///////////////////////////////////////////////////////////////////////////////
// JabberCompareJids

int JabberCompareJids(const TCHAR *jid1, const TCHAR *jid2)
{
	if (!mir_tstrcmpi(jid1, jid2))
		return 0;

	// match only node@domain part
	TCHAR szTempJid1[JABBER_MAX_JID_LEN], szTempJid2[JABBER_MAX_JID_LEN];
	return mir_tstrcmpi(
		JabberStripJid(jid1, szTempJid1, SIZEOF(szTempJid1)),
		JabberStripJid(jid2, szTempJid2, SIZEOF(szTempJid2)));
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBAddAuthRequest()

void CJabberProto::DBAddAuthRequest(const TCHAR *jid, const TCHAR *nick)
{
	MCONTACT hContact = DBCreateContact(jid, nick, TRUE, TRUE);
	delSetting(hContact, "Hidden");

	char* szJid = mir_utf8encodeT(jid);
	char* szNick = mir_utf8encodeT(nick);

	//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
	//blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), email(ASCIIZ), ""(ASCIIZ)
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = (DWORD)(sizeof(DWORD)*2 + mir_strlen(szNick) + mir_strlen(szJid) + 5);
	PBYTE pCurBlob = dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
	*((PDWORD)pCurBlob) = 0; pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact; pCurBlob += sizeof(DWORD);
	strcpy((char*)pCurBlob, szNick); pCurBlob += mir_strlen(szNick)+1;
	*pCurBlob = '\0'; pCurBlob++;		//firstName
	*pCurBlob = '\0'; pCurBlob++;		//lastName
	strcpy((char*)pCurBlob, szJid); pCurBlob += mir_strlen(szJid)+1;
	*pCurBlob = '\0';					//reason

	db_event_add(NULL, &dbei);
	debugLogA("Setup DBAUTHREQUEST with nick='%s' jid='%s'", szNick, szJid);

	mir_free(szJid);
	mir_free(szNick);
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBCreateContact()

MCONTACT CJabberProto::DBCreateContact(const TCHAR *jid, const TCHAR *nick, BOOL temporary, BOOL stripResource)
{
	if (jid == NULL || jid[0]=='\0')
		return NULL;

	TCHAR *s = NEWTSTR_ALLOCA(jid);
	TCHAR *q = NULL, *p;
	// strip resource if present
	if ((p = _tcschr(s, '@')) != NULL)
		if ((q = _tcschr(p, '/')) != NULL)
			*q = '\0';

	if (!stripResource && q != NULL)	// so that resource is not stripped
		*q = '/';

	// We can't use JabberHContactFromJID() here because of the stripResource option
	size_t len = _tcslen(s);
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		ptrT jid( getTStringA(hContact, "jid"));
		if (jid == NULL)
			continue;

		TCHAR *p = jid;
		if (p && _tcslen(p) >= len && (p[len]=='\0'||p[len]=='/') && !_tcsnicmp(p, s, len))
			return hContact;
	}

	MCONTACT hNewContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hNewContact, (LPARAM)m_szModuleName);
	setTString(hNewContact, "jid", s);
	if (nick != NULL && *nick != '\0')
		setTString(hNewContact, "Nick", nick);
	if (temporary)
		db_set_b(hNewContact, "CList", "NotOnList", 1);
	else
		SendGetVcard(s);
	debugLog(_T("Create Jabber contact jid=%s, nick=%s"), s, nick);
	DBCheckIsTransportedContact(s,hNewContact);
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

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.pBlob = &btEventType;
	dbei.cbBlob = sizeof(btEventType);
	dbei.eventType = EVENTTYPE_JABBER_PRESENCE;
	dbei.flags = DBEF_READ;
	dbei.timestamp = time(NULL);
	dbei.szModule = m_szModuleName;
	db_event_add(hContact, &dbei);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarFileName() - gets a file name for the avatar image

void CJabberProto::GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen)
{
	int tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(pszDest);

	pszDest[ tPathLen++ ] = '\\';

	const TCHAR* szFileType = ProtoGetAvatarExtension( getByte(hContact, "AvatarType", PA_FORMAT_PNG));

	if (hContact != NULL) {
		char str[256];
		JabberShaStrBuf buf;
		DBVARIANT dbv;
		if (!db_get_utf(hContact, m_szModuleName, "jid", &dbv)) {
			strncpy_s(str, dbv.pszVal, _TRUNCATE);
			str[ sizeof(str)-1 ] = 0;
			db_free(&dbv);
		}
		else _i64toa((LONG_PTR)hContact, str, 10);
		mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%S%s"), JabberSha1(str, buf), szFileType);
	}
	else if (m_ThreadInfo != NULL) {
		mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%s@%S avatar%s"),
			m_ThreadInfo->conn.username, m_ThreadInfo->conn.server, szFileType);
	}
	else {
		ptrA res1( getStringA("LoginName")), res2( getStringA("LoginServer"));
		mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%S@%S avatar%s"),
			(res1) ? (LPSTR)res1 : "noname", (res2) ? (LPSTR)res2 : m_szModuleName, szFileType);
	}
}

///////////////////////////////////////////////////////////////////////////////
// JabberResolveTransportNicks - massive vcard update

void CJabberProto::ResolveTransportNicks(const TCHAR *jid)
{
	// Set all contacts to offline
	MCONTACT hContact = m_ThreadInfo->resolveContact;
	if (hContact == NULL)
		hContact = db_find_first(m_szModuleName);

	for (; hContact != NULL; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!getByte(hContact, "IsTransported", 0))
			continue;

		ptrT dbJid( getTStringA(hContact, "jid")); if (dbJid == NULL) continue;
		ptrT dbNick( getTStringA(hContact, "Nick")); if (dbNick == NULL) continue;

		TCHAR *p = _tcschr(dbJid, '@');
		if (p == NULL)
			continue;

		*p = 0;
		if (!mir_tstrcmp(jid, p+1) && !mir_tstrcmp(dbJid, dbNick)) {
			*p = '@';
			m_ThreadInfo->resolveID = SendGetVcard(dbJid);
			m_ThreadInfo->resolveContact = hContact;
			return;
		}
	}

	m_ThreadInfo->resolveID = -1;
	m_ThreadInfo->resolveContact = NULL;
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
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}

// Process a string, and double all % characters, according to chat.dll's restrictions
// Returns a pointer to the new string (old one is not freed)

TCHAR* UnEscapeChatTags(TCHAR* str_in)
{
	TCHAR *s = str_in, *d = str_in;
	while (*s) {
		if (*s == '%' && s[1] == '%')
			s++;
		*d++ = *s++;
	}
	*d = 0;
	return str_in;
}

//////////////////////////////////////////////////////////////////////////
// update MirVer with data for active resource

struct
{
	TCHAR *node;
	TCHAR *name;
}
static sttCapsNodeToName_Map[] =
{
	{ _T("http://miranda-im.org"), _T("Miranda IM Jabber") },
	{ _T("http://miranda-ng.org"), _T("Miranda NG Jabber") },
	{ _T("http://www.google.com"), _T("GTalk") },
	{ _T("http://mail.google.com"), _T("GMail") },
	{ _T("http://talk.google.com/xmpp/bot"), _T("GTalk Bot") },
	{ _T("http://www.android.com"), _T("Android") },
};

void CJabberProto::UpdateMirVer(JABBER_LIST_ITEM *item)
{
	MCONTACT hContact = HContactFromJID(item->jid);
	if (!hContact)
		return;

	debugLog(_T("JabberUpdateMirVer: for jid %s"), item->jid);

	pResourceStatus p(NULL);
	if (item->resourceMode == RSMODE_LASTSEEN)
		p = item->m_pLastSeenResource;
	else if (item->resourceMode == RSMODE_MANUAL)
		p = item->m_pManualResource;

	if (p)
		UpdateMirVer(hContact, p);
}

void CJabberProto::FormatMirVer(pResourceStatus &resource, CMString &res)
{
	res.Empty();
	if (resource == NULL)
		return;

	// jabber:iq:version info requested and exists?
	if (resource->m_dwVersionRequestTime && resource->m_tszSoftware) {
		debugLog(_T("JabberUpdateMirVer: for iq:version rc %s: %s"), resource->m_tszResourceName, resource->m_tszSoftware);
		if (!resource->m_tszSoftwareVersion || _tcsstr(resource->m_tszSoftware, resource->m_tszSoftwareVersion))
			res = resource->m_tszSoftware;
		else
			res.Format(_T("%s %s"), resource->m_tszSoftware, resource->m_tszSoftwareVersion);
	}
	// no version info and no caps info? set MirVer = resource name
	else if (!resource->m_tszCapsNode || !resource->m_tszCapsVer) {
		debugLog(_T("JabberUpdateMirVer: for rc %s: %s"), resource->m_tszResourceName, resource->m_tszResourceName);
		if (resource->m_tszResourceName)
			res = resource->m_tszResourceName;
	}
	// XEP-0115 caps mode
	else {
		debugLog(_T("JabberUpdateMirVer: for rc %s: %s#%s"), resource->m_tszResourceName, resource->m_tszCapsNode, resource->m_tszCapsVer);

		int i;

		// search through known software list
		for (i = 0; i < SIZEOF(sttCapsNodeToName_Map); i++)
			if (_tcsstr(resource->m_tszCapsNode, sttCapsNodeToName_Map[i].node)) {
				res.Format(_T("%s %s"), sttCapsNodeToName_Map[i].name, resource->m_tszCapsVer);
				break;
			}

		// unknown software
		if (i == SIZEOF(sttCapsNodeToName_Map))
			res.Format(_T("%s %s"), resource->m_tszCapsNode, resource->m_tszCapsVer);
	}

	// attach additional info for fingerprint plguin
	if (resource->m_tszCapsExt && _tcsstr(resource->m_tszCapsExt, JABBER_EXT_PLATFORMX86) && !_tcsstr(res, _T("x86")))
		res.Append(_T(" x86"));

	if (resource->m_tszCapsExt && _tcsstr(resource->m_tszCapsExt, JABBER_EXT_PLATFORMX64) && !_tcsstr(res, _T("x64")))
		res.Append(_T(" x64"));

	if (resource->m_tszCapsExt && _tcsstr(resource->m_tszCapsExt, JABBER_EXT_SECUREIM) && !_tcsstr(res, _T("(SecureIM)")))
		res.Append(_T(" (SecureIM)"));

	if (resource->m_tszCapsExt && _tcsstr(resource->m_tszCapsExt, JABBER_EXT_MIROTR) && !_tcsstr(res, _T("(MirOTR)")))
		res.Append(_T(" (MirOTR)"));

	if (resource->m_tszCapsExt && _tcsstr(resource->m_tszCapsExt, JABBER_EXT_NEWGPG) && !_tcsstr(res, _T("(New_GPG)")))
		res.Append(_T(" (New_GPG)"));

	if (resource->m_tszResourceName && !_tcsstr(res, resource->m_tszResourceName))
		if (_tcsstr(res, _T("Miranda IM")) || _tcsstr(res, _T("Miranda NG")) || m_options.ShowForeignResourceInMirVer)
			res.AppendFormat(_T(" [%s]"), resource->m_tszResourceName);
}


void CJabberProto::UpdateMirVer(MCONTACT hContact, pResourceStatus &resource)
{
	CMString tszMirVer;
	FormatMirVer(resource, tszMirVer);
	if (!tszMirVer.IsEmpty())
		setTString(hContact, "MirVer", tszMirVer);

	ptrT jid( getTStringA(hContact, "jid"));
	if (jid == NULL)
		return;

	TCHAR szFullJid[JABBER_MAX_JID_LEN];
	if (resource->m_tszResourceName && !_tcschr(jid, '/'))
		mir_sntprintf(szFullJid, SIZEOF(szFullJid), _T("%s/%s"), jid, resource->m_tszResourceName);
	else
		mir_tstrncpy(szFullJid, jid, SIZEOF(szFullJid));
	setTString(hContact, DBSETTING_DISPLAY_UID, szFullJid);
}

void CJabberProto::UpdateSubscriptionInfo(MCONTACT hContact, JABBER_LIST_ITEM *item)
{
	switch (item->subscription) {
	case SUB_TO:
		setTString(hContact, "SubscriptionText", TranslateT("To"));
		setString(hContact, "Subscription", "to");
		setByte(hContact, "Auth", 0);
		setByte(hContact, "Grant", 1);
		break;
	case SUB_FROM:
		setTString(hContact, "SubscriptionText", TranslateT("From"));
		setString(hContact, "Subscription", "from");
		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 0);
		break;
	case SUB_BOTH:
		setTString(hContact, "SubscriptionText", TranslateT("Both"));
		setString(hContact, "Subscription", "both");
		setByte(hContact, "Auth", 0);
		setByte(hContact, "Grant", 0);
		break;
	case SUB_NONE:
		setTString(hContact, "SubscriptionText", TranslateT("None"));
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

	//JabberUpdateContactExtraIcon(hContact);
}

void CJabberProto::InitPopups(void)
{
	TCHAR desc[256];
	mir_sntprintf(desc, SIZEOF(desc), _T("%s %s"), m_tszUserName, TranslateT("Errors"));

	char name[256];
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Error");

	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = LoadIconEx("main");
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClass = Popup_RegisterClass(&ppc);

	Skin_ReleaseIcon(ppc.hIcon);
}

void CJabberProto::MsgPopup(MCONTACT hContact, const TCHAR *szMsg, const TCHAR *szTitle)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		char name[256];

		POPUPDATACLASS ppd = { sizeof(ppd) };
		ppd.ptszTitle = szTitle;
		ppd.ptszText = szMsg;
		ppd.pszClassName = name;
		ppd.hContact = hContact;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Error");

		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
	}
	else {
		DWORD mtype = MB_OK | MB_SETFOREGROUND | MB_ICONSTOP;
		MessageBox(NULL, szMsg, szTitle, mtype);
	}
}
