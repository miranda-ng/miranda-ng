/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov
Copyright (C) 2012-13  Miranda NG Project

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
	if ( !lstrcmpi(jid1, jid2))
		return 0;

	// match only node@domain part
	TCHAR szTempJid1[JABBER_MAX_JID_LEN], szTempJid2[JABBER_MAX_JID_LEN];
	return lstrcmpi(
		JabberStripJid(jid1, szTempJid1, SIZEOF(szTempJid1)),
		JabberStripJid(jid2, szTempJid2, SIZEOF(szTempJid2)));
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBAddAuthRequest()

void CJabberProto::DBAddAuthRequest(const TCHAR *jid, const TCHAR *nick)
{
	HANDLE hContact = DBCreateContact(jid, NULL, TRUE, TRUE);
	delSetting(hContact, "Hidden");
	//setTString(hContact, "Nick", nick);

	char* szJid = mir_utf8encodeT(jid);
	char* szNick = mir_utf8encodeT(nick);

	//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
	//blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), email(ASCIIZ), ""(ASCIIZ)
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = (DWORD)(sizeof(DWORD)*2 + strlen(szNick) + strlen(szJid) + 5);
	PBYTE pCurBlob = dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
	*((PDWORD)pCurBlob) = 0; pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact; pCurBlob += sizeof(DWORD);
	strcpy((char*)pCurBlob, szNick); pCurBlob += strlen(szNick)+1;
	*pCurBlob = '\0'; pCurBlob++;		//firstName
	*pCurBlob = '\0'; pCurBlob++;		//lastName
	strcpy((char*)pCurBlob, szJid); pCurBlob += strlen(szJid)+1;
	*pCurBlob = '\0';					//reason

	db_event_add(NULL, &dbei);
	Log("Setup DBAUTHREQUEST with nick='%s' jid='%s'", szNick, szJid);

	mir_free(szJid);
	mir_free(szNick);
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBCreateContact()

HANDLE CJabberProto::DBCreateContact(const TCHAR *jid, const TCHAR *nick, BOOL temporary, BOOL stripResource)
{
	HANDLE hContact;
	TCHAR *s, *p, *q;
	size_t len;

	if (jid == NULL || jid[0]=='\0')
		return NULL;

	s = mir_tstrdup(jid);
	q = NULL;
	// strip resource if present
	if ((p = _tcschr(s, '@')) != NULL)
		if ((q = _tcschr(p, '/')) != NULL)
			*q = '\0';

	if ( !stripResource && q != NULL)	// so that resource is not stripped
		*q = '/';
	len = _tcslen(s);

	// We can't use JabberHContactFromJID() here because of the stripResource option
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		DBVARIANT dbv;
		if ( !getTString(hContact, "jid", &dbv)) {
			p = dbv.ptszVal;
			if (p && _tcslen(p) >= len && (p[len]=='\0'||p[len]=='/') && !_tcsnicmp(p, s, len)) {
				db_free(&dbv);
				break;
			}
			db_free(&dbv);
		}
	}

	if (hContact == NULL) {
		hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName);
		setTString(hContact, "jid", s);
		if (nick != NULL && *nick != '\0')
			setTString(hContact, "Nick", nick);
		if (temporary)
			db_set_b(hContact, "CList", "NotOnList", 1);
		else
			SendGetVcard(s);
		Log("Create Jabber contact jid=%S, nick=%S", s, nick);
		DBCheckIsTransportedContact(s,hContact);
	}

	mir_free(s);
	return hContact;
}

BOOL CJabberProto::AddDbPresenceEvent(HANDLE hContact, BYTE btEventType)
{
	if ( !hContact)
		return FALSE;

	switch (btEventType) {
	case JABBER_DB_EVENT_PRESENCE_SUBSCRIBE:
	case JABBER_DB_EVENT_PRESENCE_SUBSCRIBED:
	case JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBE:
	case JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBED:
		if ( !m_options.LogPresence)
			return FALSE;
		break;

	case JABBER_DB_EVENT_PRESENCE_ERROR:
		if ( !m_options.LogPresenceErrors)
			return FALSE;
		break;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.pBlob = &btEventType;
	dbei.cbBlob = sizeof(btEventType);
	dbei.eventType = JABBER_DB_EVENT_TYPE_PRESENCE;
	dbei.flags = DBEF_READ;
	dbei.timestamp = time(NULL);
	dbei.szModule = m_szModuleName;
	db_event_add(hContact, &dbei);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarFileName() - gets a file name for the avatar image

void CJabberProto::InitCustomFolders(void)
{
	if (m_bFoldersInitDone)
		return;

	m_bFoldersInitDone = true;
	TCHAR AvatarsFolder[MAX_PATH];
	mir_sntprintf(AvatarsFolder, SIZEOF(AvatarsFolder), _T("%%miranda_avatarcache%%\\%S"), m_szModuleName);
	m_hJabberAvatarsFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), m_szModuleName, AvatarsFolder, m_tszUserName);
}

void CJabberProto::GetAvatarFileName(HANDLE hContact, TCHAR* pszDest, size_t cbLen)
{
	size_t tPathLen;
	TCHAR *path = (TCHAR*)alloca(cbLen * sizeof(TCHAR));

	InitCustomFolders();

	if (m_hJabberAvatarsFolder == NULL || FoldersGetCustomPathT(m_hJabberAvatarsFolder, path, (int)cbLen, _T("")))
		tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), (TCHAR*)VARST(_T("%miranda_avatarcache%")), m_szModuleName);
	else
		tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s"), path);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(pszDest);

	pszDest[ tPathLen++ ] = '\\';

	const TCHAR* szFileType = ProtoGetAvatarExtension( getByte(hContact, "AvatarType", PA_FORMAT_PNG));

	if (hContact != NULL) {
		char str[ 256 ];
		DBVARIANT dbv;
		if ( !db_get_utf(hContact, m_szModuleName, "jid", &dbv)) {
			strncpy(str, dbv.pszVal, sizeof str);
			str[ sizeof(str)-1 ] = 0;
			db_free(&dbv);
		}
		else _i64toa((LONG_PTR)hContact, str, 10);
		mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%S%s"), ptrA(JabberSha1(str)), szFileType);
	}
	else if (m_ThreadInfo != NULL) {
		mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%s@%S avatar%s"),
			m_ThreadInfo->username, m_ThreadInfo->server, szFileType);
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
	HANDLE hContact = m_ThreadInfo->resolveContact;
	if (hContact == NULL)
		hContact = (HANDLE)db_find_first(m_szModuleName);

	for (; hContact != NULL; hContact = db_find_next(hContact, m_szModuleName)) {
		if ( !getByte(hContact, "IsTransported", 0))
			continue;

		DBVARIANT dbv, nick;
		if (getTString(hContact, "jid", &dbv))
			continue;
		if (getTString(hContact, "Nick", &nick)) {
			db_free(&dbv);
			continue;
		}

		TCHAR *p = _tcschr(dbv.ptszVal, '@');
		if (p) {
			*p = 0;
			if ( !lstrcmp(jid, p+1) && !lstrcmp(dbv.ptszVal, nick.ptszVal)) {
				*p = '@';
				m_ThreadInfo->resolveID = SendGetVcard(dbv.ptszVal);
				m_ThreadInfo->resolveContact = hContact;
				db_free(&dbv);
				db_free(&nick);
				return;
		}	}

		db_free(&dbv);
		db_free(&nick);
	}

	m_ThreadInfo->resolveID = -1;
	m_ThreadInfo->resolveContact = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// JabberSetServerStatus()

void CJabberProto::SetServerStatus(int iNewStatus)
{
	if ( !m_bJabberOnline)
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
TCHAR* EscapeChatTags(TCHAR* pszText)
{
	int nChars = 0;
	for (TCHAR *p = pszText; (p = _tcschr(p, '%')) != NULL; p++)
		nChars++;

	if (nChars == 0)
		return mir_tstrdup(pszText);

	TCHAR *pszNewText = (TCHAR*)mir_alloc(sizeof(TCHAR)*(_tcslen(pszText) + 1 + nChars)), *s, *d;
	if (pszNewText == NULL)
		return mir_tstrdup(pszText);

	for (s = pszText, d = pszNewText; *s; s++) {
		if (*s == '%')
			*d++ = '%';
		*d++ = *s;
	}
	*d = 0;
	return pszNewText;
}

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
	HANDLE hContact = HContactFromJID(item->jid);
	if ( !hContact)
		return;

	Log("JabberUpdateMirVer: for jid %S", item->jid);

	pResourceStatus p(NULL);
	if (item->resourceMode == RSMODE_LASTSEEN)
		p = item->pLastSeenResource;
	else if (item->resourceMode == RSMODE_MANUAL)
		p = item->pManualResource;

	if (p)
		UpdateMirVer(hContact, p);
}

void CJabberProto::FormatMirVer(pResourceStatus &resource, TCHAR *buf, int bufSize)
{
	if ( !buf || !bufSize) return;
	buf[ 0 ] = _T('\0');
	if ( !resource) return;

	// jabber:iq:version info requested and exists?
	if (resource->dwVersionRequestTime && resource->software) {
		Log("JabberUpdateMirVer: for iq:version rc %S: %S", resource->resourceName, resource->software);
		if ( !resource->version || _tcsstr(resource->software, resource->version))
			lstrcpyn(buf, resource->software, bufSize);
		else
			mir_sntprintf(buf, bufSize, _T("%s %s"), resource->software, resource->version);
	}
	// no version info and no caps info? set MirVer = resource name
	else if ( !resource->szCapsNode || !resource->szCapsVer) {
		Log("JabberUpdateMirVer: for rc %S: %S", resource->resourceName, resource->resourceName);
		if (resource->resourceName)
			lstrcpyn(buf, resource->resourceName, bufSize);
	}
	// XEP-0115 caps mode
	else {
		Log("JabberUpdateMirVer: for rc %S: %S#%S", resource->resourceName, resource->szCapsNode, resource->szCapsVer);

		int i;

		// search through known software list
		for (i = 0; i < SIZEOF(sttCapsNodeToName_Map); i++)
			if (_tcsstr(resource->szCapsNode, sttCapsNodeToName_Map[i].node))
			{
				mir_sntprintf(buf, bufSize, _T("%s %s"), sttCapsNodeToName_Map[i].name, resource->szCapsVer);
				break;
			}

		// unknown software
		if (i == SIZEOF(sttCapsNodeToName_Map))
			mir_sntprintf(buf, bufSize, _T("%s %s"), resource->szCapsNode, resource->szCapsVer);
	}

	// attach additional info for fingerprint plguin
	if (resource->szCapsExt && _tcsstr(resource->szCapsExt, _T(JABBER_EXT_PLATFORMX86)) && !_tcsstr(buf, _T("x86")))
	{
		int offset = lstrlen(buf);
		mir_sntprintf(buf + offset, bufSize - offset, _T(" x86"));
	}

	if (resource->szCapsExt && _tcsstr(resource->szCapsExt, _T(JABBER_EXT_PLATFORMX64)) && !_tcsstr(buf, _T("x64")))
	{
		int offset = lstrlen(buf);
		mir_sntprintf(buf + offset, bufSize - offset, _T(" x64"));
	}

	if (resource->szCapsExt && _tcsstr(resource->szCapsExt, _T(JABBER_EXT_SECUREIM)) && !_tcsstr(buf, _T("(SecureIM)")))
	{
		int offset = lstrlen(buf);
		mir_sntprintf(buf + offset, bufSize - offset, _T(" (SecureIM)"));
	}

	if (resource->szCapsExt && _tcsstr(resource->szCapsExt, _T(JABBER_EXT_MIROTR)) && !_tcsstr(buf, _T("(MirOTR)")))
	{
		int offset = lstrlen(buf);
		mir_sntprintf(buf + offset, bufSize - offset, _T(" (MirOTR)"));
	}

	if (resource->szCapsExt && _tcsstr(resource->szCapsExt, _T(JABBER_EXT_NEWGPG)) && !_tcsstr(buf, _T("(New_GPG)")))
	{
		int offset = lstrlen(buf);
		mir_sntprintf(buf + offset, bufSize - offset, _T(" (New_GPG)"));
	}

	if (resource->resourceName && !_tcsstr(buf, resource->resourceName))
	{
		if (_tcsstr(buf, _T("Miranda IM")) || _tcsstr(buf, _T("Miranda NG")) || m_options.ShowForeignResourceInMirVer)
		{
			int offset = lstrlen(buf);
			mir_sntprintf(buf + offset, bufSize - offset, _T(" [%s]"), resource->resourceName);
		}
	}
}


void CJabberProto::UpdateMirVer(HANDLE hContact, pResourceStatus &resource)
{
	TCHAR szMirVer[ 512 ];
	FormatMirVer(resource, szMirVer, SIZEOF(szMirVer));
	if (szMirVer[0])
		setTString(hContact, "MirVer", szMirVer);
//	else
//		delSetting(hContact, "MirVer");

	DBVARIANT dbv;
	if ( !getTString(hContact, "jid", &dbv)) {
		TCHAR szFullJid[JABBER_MAX_JID_LEN];
		if (resource->resourceName)
			mir_sntprintf(szFullJid, SIZEOF(szFullJid), _T("%s/%s"), dbv.ptszVal, resource->resourceName);
		else
			lstrcpyn(szFullJid, dbv.ptszVal, SIZEOF(szFullJid));
		setTString(hContact, DBSETTING_DISPLAY_UID, szFullJid);
		db_free(&dbv);
	}
}

void CJabberProto::UpdateSubscriptionInfo(HANDLE hContact, JABBER_LIST_ITEM *item)
{
	switch (item->subscription)
	{
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

void CJabberProto::SetContactOfflineStatus(HANDLE hContact)
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

void CJabberProto::MsgPopup(HANDLE hContact, const TCHAR *szMsg, const TCHAR *szTitle)
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
	} else {
		DWORD mtype = MB_OK | MB_SETFOREGROUND | MB_ICONSTOP;
		MessageBox(NULL, szMsg, szTitle, mtype);
	}
}
