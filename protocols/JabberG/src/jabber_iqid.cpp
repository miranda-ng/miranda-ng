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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"

void CJabberProto::OnIqResultServerDiscoInfo(HXML iqNode, CJabberIqInfo*)
{
	if (iqNode == NULL)
		return;

	const TCHAR *type = XmlGetAttrValue(iqNode, _T("type"));
	if ( mir_tstrcmp(type, _T("result")))
		return;

	HXML query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_INFO);
	if (query == NULL)
		return;

	HXML identity;
	for (int i = 1; (identity = XmlGetNthChild(query, _T("identity"), i)) != NULL; i++) {
		JABBER_DISCO_FIELD tmp = { 
			XmlGetAttrValue(identity, _T("category")),
			XmlGetAttrValue(identity, _T("type")),
			XmlGetAttrValue(identity, _T("name")) };

		if (!mir_tstrcmp(tmp.category, _T("pubsub")) && !mir_tstrcmp(tmp.type, _T("pep"))) {
			m_bPepSupported = true;

			EnableMenuItems(TRUE);
			RebuildInfoFrame();
			continue;
		}

		NotifyFastHook(hDiscoInfoResult, (WPARAM)&tmp, (LPARAM)(IJabberInterface*)this);
	}

	if (m_ThreadInfo) {
		HXML feature;
		for (int i = 1; (feature = XmlGetNthChild(query, _T("feature"), i)) != NULL; i++) {
			const TCHAR *featureName = XmlGetAttrValue(feature, _T("var"));
			if (!featureName)
				continue;

			for (int j = 0; g_JabberFeatCapPairs[j].szFeature; j++) {
				if (!mir_tstrcmp(g_JabberFeatCapPairs[j].szFeature, featureName)) {
					m_ThreadInfo->jabberServerCaps |= g_JabberFeatCapPairs[j].jcbCap;
					break;
				}
			}
		}
	}

	OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_SERVERINFO);
}

void CJabberProto::OnIqResultNestedRosterGroups(HXML iqNode, CJabberIqInfo *pInfo)
{
	const TCHAR *szGroupDelimeter = NULL;
	BOOL bPrivateStorageSupport = FALSE;

	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		bPrivateStorageSupport = TRUE;
		szGroupDelimeter = XPathFmt(iqNode, _T("query[@xmlns='%s']/roster[@xmlns='%s']"), JABBER_FEAT_PRIVATE_STORAGE, JABBER_FEAT_NESTED_ROSTER_GROUPS);
		if (szGroupDelimeter && !szGroupDelimeter[0])
			szGroupDelimeter = NULL; // "" as roster delimeter is not supported :)
	}

	// global fuckup
	if (m_ThreadInfo == NULL)
		return;

	// is our default delimiter?
	if ((!szGroupDelimeter && bPrivateStorageSupport) || (szGroupDelimeter && mir_tstrcmp(szGroupDelimeter, _T("\\"))))
		m_ThreadInfo->send(
			XmlNodeIq(_T("set"), SerialNext()) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
				<< XCHILD(_T("roster"), _T("\\")) << XATTR(_T("xmlns"), JABBER_FEAT_NESTED_ROSTER_GROUPS));

	// roster request
	TCHAR *szUserData = mir_tstrdup(szGroupDelimeter ? szGroupDelimeter : _T("\\"));
	m_ThreadInfo->send(
		XmlNodeIq( AddIQ(&CJabberProto::OnIqResultGetRoster, JABBER_IQ_TYPE_GET, NULL, 0, -1, (void*)szUserData))
			<< XCHILDNS(_T("query"), JABBER_FEAT_IQ_ROSTER));
}

void CJabberProto::OnIqResultNotes(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML hXmlData = XPathFmt(iqNode, _T("query[@xmlns='%s']/storage[@xmlns='%s']"),
			JABBER_FEAT_PRIVATE_STORAGE, JABBER_FEAT_MIRANDA_NOTES);
		if (hXmlData) m_notes.LoadXml(hXmlData);
	}
}

void CJabberProto::OnProcessLoginRq(ThreadData *info, DWORD rq)
{
	if (info == NULL)
		return;

	info->dwLoginRqs |= rq;

	DWORD dwMask = JABBER_LOGIN_ROSTER | JABBER_LOGIN_BOOKMARKS | JABBER_LOGIN_SERVERINFO;
	if ((info->dwLoginRqs & dwMask) == dwMask && !(info->dwLoginRqs & JABBER_LOGIN_BOOKMARKS_AJ)) {
		if (info->jabberServerCaps & JABBER_CAPS_ARCHIVE_AUTO)
			EnableArchive(m_options.EnableMsgArchive != 0);

		if (m_options.AutoJoinBookmarks) {
			LIST<JABBER_LIST_ITEM> ll(10);
			LISTFOREACH(i, this, LIST_BOOKMARK)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				if (item != NULL && !mir_tstrcmp(item->type, _T("conference")) && item->bAutoJoin)
					ll.insert(item);
			}

			for (int j=0; j < ll.getCount(); j++) {
				JABBER_LIST_ITEM *item = ll[j];

				TCHAR room[256], text[128];
				_tcsncpy_s(text, item->jid, _TRUNCATE);
				_tcsncpy_s(room, text, _TRUNCATE);
				TCHAR *p = _tcstok(room, _T("@"));
				TCHAR *server = _tcstok(NULL, _T("@"));
				if (item->nick && item->nick[0])
					GroupchatJoinRoom(server, p, item->nick, item->password, true);
				else
					GroupchatJoinRoom(server, p, ptrT(JabberNickFromJID(m_szJabberJID)), item->password, true);
			}
		}

		OnProcessLoginRq(info, JABBER_LOGIN_BOOKMARKS_AJ);
}	}

void CJabberProto::OnLoggedIn()
{
	m_bJabberOnline = true;
	m_tmJabberLoggedInTime = time(0);

	m_ThreadInfo->dwLoginRqs = 0;

	// XEP-0083 support
	{
		CJabberIqInfo *pIqInfo = AddIQ(&CJabberProto::OnIqResultNestedRosterGroups, JABBER_IQ_TYPE_GET);
		// ugly hack to prevent hangup during login process
		pIqInfo->SetTimeout(30000);
		m_ThreadInfo->send(
			XmlNodeIq(pIqInfo) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
				<< XCHILDNS(_T("roster"), JABBER_FEAT_NESTED_ROSTER_GROUPS));
	}

	// Server-side notes
	m_ThreadInfo->send(
		XmlNodeIq( AddIQ(&CJabberProto::OnIqResultNotes, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
			<< XCHILDNS(_T("storage"), JABBER_FEAT_MIRANDA_NOTES));
	
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultDiscoBookmarks, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE) << XCHILDNS(_T("storage"), _T("storage:bookmarks")));

	m_bPepSupported = false;
	m_ThreadInfo->jabberServerCaps = JABBER_RESOURCE_CAPS_NONE;
	
	m_ThreadInfo->send( 
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultServerDiscoInfo, JABBER_IQ_TYPE_GET, _A2T(m_ThreadInfo->conn.server)))
			<< XQUERY(JABBER_FEAT_DISCO_INFO));

	QueryPrivacyLists(m_ThreadInfo);

	ptrA szServerName(getStringA("LastLoggedServer"));
	if (szServerName == NULL || mir_strcmp(m_ThreadInfo->conn.server, szServerName))
		SendGetVcard(m_szJabberJID);

	setString("LastLoggedServer", m_ThreadInfo->conn.server);
	m_pepServices.ResetPublishAll();
}

void CJabberProto::OnIqResultGetAuth(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: result of the request for authentication method
	// ACTION: send account authentication information to log in
	debugLogA("<iq/> iqIdGetAuth");

	HXML queryNode;
	const TCHAR *type;
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;
	if ((queryNode = XmlGetChild(iqNode, "query")) == NULL) return;

	if (!mir_tstrcmp(type, _T("result"))) {
		XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultSetAuth, JABBER_IQ_TYPE_SET));
		HXML query = iq << XQUERY(_T("jabber:iq:auth"));
		query << XCHILD(_T("username"), m_ThreadInfo->conn.username);
		if (XmlGetChild(queryNode, "digest") != NULL && m_ThreadInfo->szStreamId) {
			JabberShaStrBuf buf;
			T2Utf str(m_ThreadInfo->conn.password);
			char text[200];
			mir_snprintf(text, _countof(text), "%s%s", m_ThreadInfo->szStreamId, str);
			query << XCHILD(_T("digest"), _A2T(JabberSha1(text, buf)));
		}
		else if (XmlGetChild(queryNode, "password") != NULL)
			query << XCHILD(_T("password"), m_ThreadInfo->conn.password);
		else {
			debugLogA("No known authentication mechanism accepted by the server.");
			m_ThreadInfo->send("</stream:stream>");
			return;
		}

		if (XmlGetChild(queryNode , "resource") != NULL)
			query << XCHILD(_T("resource"), m_ThreadInfo->resource);

		m_ThreadInfo->send(iq);
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		m_ThreadInfo->send("</stream:stream>");

		TCHAR text[128];
		mir_sntprintf(text, TranslateT("Authentication failed for %s."), m_ThreadInfo->conn.username);
		MsgPopup(NULL, text, TranslateT("Jabber Authentication"));
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = NULL;	// To disallow auto reconnect
}	}

void CJabberProto::OnIqResultSetAuth(HXML iqNode, CJabberIqInfo*)
{
	const TCHAR *type;

	// RECVED: authentication result
	// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
	debugLogA("<iq/> iqIdSetAuth");
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;

	if (!mir_tstrcmp(type, _T("result"))) {
		ptrT tszNick(getTStringA("Nick"));
		if (tszNick == NULL)
			setTString("Nick", m_ThreadInfo->conn.username);

		OnLoggedIn();
	}
	// What to do if password error? etc...
	else if (!mir_tstrcmp(type, _T("error"))) {
		TCHAR text[128];

		m_ThreadInfo->send("</stream:stream>");
		mir_sntprintf(text, TranslateT("Authentication failed for %s."), m_ThreadInfo->conn.username);
		MsgPopup(NULL, text, TranslateT("Jabber Authentication"));
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = NULL;	// To disallow auto reconnect
}	}

void CJabberProto::OnIqResultBind(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!m_ThreadInfo || !iqNode)
		return;
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		LPCTSTR szJid = XPathT(iqNode, "bind[@xmlns='urn:ietf:params:xml:ns:xmpp-bind']/jid");
		if (szJid) {
			if (!_tcsncmp(m_ThreadInfo->fullJID, szJid, _countof(m_ThreadInfo->fullJID)))
				debugLog(_T("Result Bind: %s confirmed "), m_ThreadInfo->fullJID);
			else {
				debugLog(_T("Result Bind: %s changed to %s"), m_ThreadInfo->fullJID, szJid);
				_tcsncpy_s(m_ThreadInfo->fullJID, szJid, _TRUNCATE);
			}
		}
		if (m_ThreadInfo->bIsSessionAvailable)
			m_ThreadInfo->send(
				XmlNodeIq( AddIQ(&CJabberProto::OnIqResultSession, JABBER_IQ_TYPE_SET))
				<< XCHILDNS(_T("session"), _T("urn:ietf:params:xml:ns:xmpp-session")));
		else
			OnLoggedIn();
	}
	else {
		//rfc3920 page 39
		m_ThreadInfo->send("</stream:stream>");
		m_ThreadInfo = NULL;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultSession(HXML, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		OnLoggedIn();
}

void CJabberProto::GroupchatJoinByHContact(MCONTACT hContact, bool autojoin)
{
	ptrT roomjid( getTStringA(hContact, "ChatRoomID"));
	if (roomjid == NULL)
		return;

	TCHAR *room = roomjid;
	TCHAR *server = _tcschr(roomjid, '@');
	if (!server)
		return;

	server[0] = 0; server++;

	ptrT nick( getTStringA(hContact, "MyNick"));
	if (nick == NULL) {
		nick = JabberNickFromJID(m_szJabberJID);
		if (nick == NULL)
			return;
	}

	GroupchatJoinRoom(server, room, nick, ptrT(getTStringA(hContact, "Password")), autojoin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetRoster - populates LIST_ROSTER and creates contact for any new rosters

void CJabberProto::OnIqResultGetRoster(HXML iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqIdGetRoster");
	TCHAR *szGroupDelimeter = (TCHAR *)pInfo->GetUserData();
	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT) {
		mir_free(szGroupDelimeter);
		return;
	}

	HXML queryNode = XmlGetChild(iqNode , "query");
	if (queryNode == NULL) {
		mir_free(szGroupDelimeter);
		return;
	}

	if (mir_tstrcmp(XmlGetAttrValue(queryNode, _T("xmlns")), JABBER_FEAT_IQ_ROSTER)) {
		mir_free(szGroupDelimeter);
		return;
	}

	if (!mir_tstrcmp(szGroupDelimeter, _T("\\"))) {
		mir_free(szGroupDelimeter);
		szGroupDelimeter = NULL;
	}

	LIST<void> chatRooms(10);
	OBJLIST<JABBER_HTTP_AVATARS> *httpavatars = new OBJLIST<JABBER_HTTP_AVATARS>(20, JABBER_HTTP_AVATARS::compare);

	for (int i=0; ; i++) {
		BOOL bIsTransport=FALSE;

		HXML itemNode = XmlGetChild(queryNode ,i);
		if (!itemNode)
			break;

		if (mir_tstrcmp(XmlGetName(itemNode), _T("item")))
			continue;

		const TCHAR *str = XmlGetAttrValue(itemNode, _T("subscription"));

		JABBER_SUBSCRIPTION sub;
		if (str == NULL) sub = SUB_NONE;
		else if (!mir_tstrcmp(str, _T("both"))) sub = SUB_BOTH;
		else if (!mir_tstrcmp(str, _T("to"))) sub = SUB_TO;
		else if (!mir_tstrcmp(str, _T("from"))) sub = SUB_FROM;
		else sub = SUB_NONE;

		const TCHAR *jid = XmlGetAttrValue(itemNode, _T("jid"));
		if (jid == NULL)
			continue;
		if (_tcschr(jid, '@') == NULL)
			bIsTransport = TRUE;

		const TCHAR *name = XmlGetAttrValue(itemNode, _T("name"));
		TCHAR *nick = (name != NULL) ? mir_tstrdup(name) : JabberNickFromJID(jid);
		if (nick == NULL)
			continue;

		JABBER_LIST_ITEM *item = ListAdd(LIST_ROSTER, jid);
		item->subscription = sub;

		mir_free(item->nick); item->nick = nick;

		HXML groupNode = XmlGetChild(itemNode , "group");
		replaceStrT(item->group, XmlGetText(groupNode));

		// check group delimiters:
		if (item->group && szGroupDelimeter) {
			TCHAR *szPos = NULL;
			while (szPos = _tcsstr(item->group, szGroupDelimeter)) {
				*szPos = 0;
				szPos += mir_tstrlen(szGroupDelimeter);
				TCHAR *szNewGroup = (TCHAR *)mir_alloc(sizeof(TCHAR) * (mir_tstrlen(item->group) + mir_tstrlen(szPos) + 2));
				mir_tstrcpy(szNewGroup, item->group);
				mir_tstrcat(szNewGroup, _T("\\"));
				mir_tstrcat(szNewGroup, szPos);
				mir_free(item->group);
				item->group = szNewGroup;
			}
		}

		MCONTACT hContact = HContactFromJID(jid);
		if (hContact == NULL) {
			// Received roster has a new JID.
			// Add the jid (with empty resource) to Miranda contact list.
			hContact = DBCreateContact(jid, nick, FALSE, FALSE);
		}

		if (name != NULL) {
			ptrT tszNick( getTStringA("Nick"));
			if (tszNick != NULL) {
				if (mir_tstrcmp(nick, tszNick) != 0)
					db_set_ts(hContact, "CList", "MyHandle", nick);
				else
					db_unset(hContact, "CList", "MyHandle");
			}
			else db_set_ts(hContact, "CList", "MyHandle", nick);
		}
		else db_unset(hContact, "CList", "MyHandle");

		if ( isChatRoom(hContact)) {
			GCSESSION gcw = { sizeof(gcw) };
			gcw.iType = GCW_CHATROOM;
			gcw.pszModule = m_szModuleName;
			gcw.ptszID = jid;
			gcw.ptszName = NEWTSTR_ALLOCA(jid);

			TCHAR *p = (TCHAR*)_tcschr(gcw.ptszName, '@');
			if (p)
				*p = 0;

			CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

			db_unset(hContact, "CList", "Hidden");
			chatRooms.insert((HANDLE)hContact);
		}
		else UpdateSubscriptionInfo(hContact, item);

		if (!m_options.IgnoreRosterGroups) {
			if (item->group != NULL) {
				Clist_CreateGroup(0, item->group);

				// Don't set group again if already correct, or Miranda may show wrong group count in some case
				ptrT tszGroup( db_get_tsa(hContact, "CList", "Group"));
				if (tszGroup != NULL) {
					if ( mir_tstrcmp(tszGroup, item->group))
						db_set_ts(hContact, "CList", "Group", item->group);
				}
				else db_set_ts(hContact, "CList", "Group", item->group);
			}
			else db_unset(hContact, "CList", "Group");
		}

		if (hContact != NULL) {
			if (bIsTransport)
				setByte(hContact, "IsTransport", TRUE);
			else
				setByte(hContact, "IsTransport", FALSE);
		}

		const TCHAR *imagepath = XmlGetAttrValue(itemNode, _T("vz:img"));
		if (imagepath)
			httpavatars->insert(new JABBER_HTTP_AVATARS(imagepath, hContact));
	}

	if (httpavatars->getCount())
		ForkThread(&CJabberProto::LoadHttpAvatars, httpavatars);
	else
		delete httpavatars;

	// Delete orphaned contacts (if roster sync is enabled)
	if (m_options.RosterSync == TRUE) {
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; ) {
			MCONTACT hNext = db_find_next(hContact, m_szModuleName);
			ptrT jid( getTStringA(hContact, "jid"));
			if (jid != NULL && !ListGetItemPtr(LIST_ROSTER, jid)) {
				debugLog(_T("Syncing roster: preparing to delete %s (hContact=0x%x)"), jid, hContact);
				CallService(MS_DB_CONTACT_DELETE, hContact, 0);
			}
			hContact = hNext;
		}
	}

	EnableMenuItems(TRUE);

	debugLogA("Status changed via THREADSTART");
	SetServerStatus(m_iDesiredStatus);

	if (m_options.AutoJoinConferences)
		for (int i=0; i < chatRooms.getCount(); i++)
			GroupchatJoinByHContact((DWORD_PTR)chatRooms[i], true);

	UI_SAFE_NOTIFY_HWND(m_hwndJabberAddBookmark, WM_JABBER_CHECK_ONLINE);
	WindowList_Broadcast(m_hWindowList, WM_JABBER_CHECK_ONLINE, 0, 0);

	UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);

	if (szGroupDelimeter)
		mir_free(szGroupDelimeter);

	OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_ROSTER);
	RebuildInfoFrame();
}

void CJabberProto::OnIqResultGetRegister(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: result of the request for (agent) registration mechanism
	// ACTION: activate (agent) registration input dialog
	debugLogA("<iq/> iqIdGetRegister");

	HXML queryNode;
	const TCHAR *type;
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;
	if ((queryNode = XmlGetChild(iqNode , "query")) == NULL) return;

	if (!mir_tstrcmp(type, _T("result"))) {
		if (m_hwndAgentRegInput)
			SendMessage(m_hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 1 /*success*/, (LPARAM)xmlCopyNode(iqNode));
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		if (m_hwndAgentRegInput) {
			HXML errorNode = XmlGetChild(iqNode , "error");
			TCHAR *str = JabberErrorMsg(errorNode);
			SendMessage(m_hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 0 /*error*/, (LPARAM)str);
			mir_free(str);
}	}	}

void CJabberProto::OnIqResultSetRegister(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: result of registration process
	// ACTION: notify of successful agent registration
	debugLogA("<iq/> iqIdSetRegister");

	const TCHAR *type, *from;
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;
	if ((from = XmlGetAttrValue(iqNode, _T("from"))) == NULL) return;

	if (!mir_tstrcmp(type, _T("result"))) {
		MCONTACT hContact = HContactFromJID(from);
		if (hContact != NULL)
			setByte(hContact, "IsTransport", TRUE);

		if (m_hwndRegProgress)
			SendMessage(m_hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Registration successful"));
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		if (m_hwndRegProgress) {
			HXML errorNode = XmlGetChild(iqNode , "error");
			TCHAR *str = JabberErrorMsg(errorNode);
			SendMessage(m_hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)str);
			mir_free(str);
}	}	}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetVcard - processes the server-side v-card

void CJabberProto::OnIqResultGetVcardPhoto(HXML n, MCONTACT hContact, bool &hasPhoto)
{
	debugLogA("JabberIqResultGetVcardPhoto: %d", hasPhoto);
	if (hasPhoto)
		return;

	HXML o = XmlGetChild(n, "BINVAL");
	LPCTSTR ptszBinval = XmlGetText(o);
	if (o == NULL || ptszBinval == NULL)
		return;

	unsigned bufferLen;
	ptrA buffer((char*)mir_base64_decode(_T2A(ptszBinval), &bufferLen));
	if (buffer == NULL)
		return;

	const TCHAR *szPicType = JabberGetPictureType(n, buffer);
	if (szPicType == NULL)
		return;

	TCHAR szAvatarFileName[MAX_PATH];
	GetAvatarFileName(hContact, szAvatarFileName, _countof(szAvatarFileName));

	debugLog(_T("Picture file name set to %s"), szAvatarFileName);
	HANDLE hFile = CreateFile(szAvatarFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	debugLogA("Writing %d bytes", bufferLen);
	DWORD nWritten;
	bool bRes = WriteFile(hFile, buffer, bufferLen, &nWritten, NULL) != 0;
	CloseHandle(hFile);
	if (!bRes)
		return;

	debugLogA("%d bytes written", nWritten);
	if (hContact == NULL) {
		hasPhoto = TRUE;
		CallService(MS_AV_SETMYAVATART, (WPARAM)m_szModuleName, (LPARAM)szAvatarFileName);

		debugLog(_T("My picture saved to %s"), szAvatarFileName);
	}
	else {
		ptrT jid(getTStringA(hContact, "jid"));
		if (jid != NULL) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
			if (item == NULL) {
				item = ListAdd(LIST_VCARD_TEMP, jid); // adding to the temp list to store information about photo
				if (item != NULL)
					item->bUseResource = TRUE;
			}
			if (item != NULL) {
				hasPhoto = TRUE;
				if (item->photoFileName && mir_tstrcmp(item->photoFileName, szAvatarFileName))
					DeleteFile(item->photoFileName);
				replaceStrT(item->photoFileName, szAvatarFileName);
				debugLog(_T("Contact's picture saved to %s"), szAvatarFileName);
				OnIqResultGotAvatar(hContact, o, szPicType);
			}
		}
	}

	if (!hasPhoto)
		DeleteFile(szAvatarFileName);
}

static TCHAR* sttGetText(HXML node, char* tag)
{
	HXML n = XmlGetChild(node , tag);
	if (n == NULL)
		return NULL;

	return (TCHAR*)XmlGetText(n);
}

void CJabberProto::OnIqResultGetVcard(HXML iqNode, CJabberIqInfo*)
{
	HXML vCardNode, m, n, o;
	const TCHAR *type, *jid;
	MCONTACT hContact;
	DBVARIANT dbv;

	debugLogA("<iq/> iqIdGetVcard");
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;
	if ((jid = XmlGetAttrValue(iqNode, _T("from"))) == NULL) return;
	int id = JabberGetPacketID(iqNode);

	if (id == m_nJabberSearchID) {
		m_nJabberSearchID = -1;

		if ((vCardNode = XmlGetChild(iqNode , "vCard")) != NULL) {
			if (!mir_tstrcmp(type, _T("result"))) {
				PROTOSEARCHRESULT  psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_TCHAR;
				psr.nick.t = sttGetText(vCardNode, "NICKNAME");
				psr.firstName.t = sttGetText(vCardNode, "FN");
				psr.lastName.t = _T("");
				psr.email.t = sttGetText(vCardNode, "EMAIL");
				psr.id.t = NEWTSTR_ALLOCA(jid);
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
			}
			else if (!mir_tstrcmp(type, _T("error")))
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		}
		else ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		return;
	}

	size_t len = mir_tstrlen(m_szJabberJID);
	if (!_tcsnicmp(jid, m_szJabberJID, len) && (jid[len] == '/' || jid[len] == '\0')) {
		hContact = NULL;
		debugLogA("Vcard for myself");
	}
	else {
		if ((hContact = HContactFromJID(jid)) == NULL)
			return;
		debugLogA("Other user's vcard");
	}

	if (!mir_tstrcmp(type, _T("error"))) {
		if ((hContact = HContactFromJID(jid)) != NULL)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
		return;
	}

	if (mir_tstrcmp(type, _T("result")))
		return;

	bool hasFn = false, hasNick = false, hasGiven = false, hasFamily = false, hasMiddle = false, 
		hasBday = false, hasGender = false, hasPhone = false, hasFax = false, hasCell = false, hasUrl = false,
		hasHome = false, hasHomeStreet = false, hasHomeStreet2 = false, hasHomeLocality = false, 
		hasHomeRegion = false, hasHomePcode = false, hasHomeCtry = false, 
		hasWork = false, hasWorkStreet = false, hasWorkStreet2 = false, hasWorkLocality = false,
		hasWorkRegion = false, hasWorkPcode = false, hasWorkCtry = false,
		hasOrgname = false, hasOrgunit = false, hasRole = false, hasTitle = false, hasDesc = false, hasPhoto = false;
	int nEmail = 0, nPhone = 0, nYear, nMonth, nDay;

	if ((vCardNode = XmlGetChild(iqNode , "vCard")) != NULL) {
		for (int i=0; ; i++) {
			n = XmlGetChild(vCardNode ,i);
			if (!n)
				break;
			if (XmlGetName(n) == NULL) continue;
			if (!mir_tstrcmp(XmlGetName(n), _T("FN"))) {
				if (XmlGetText(n) != NULL) {
					hasFn = true;
					setTString(hContact, "FullName", XmlGetText(n));
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("NICKNAME"))) {
				if (XmlGetText(n) != NULL) {
					hasNick = true;
					setTString(hContact, "Nick", XmlGetText(n));
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("N"))) {
				// First/Last name
				if (!hasGiven && !hasFamily && !hasMiddle) {
					if ((m=XmlGetChild(n, "GIVEN")) != NULL && XmlGetText(m) != NULL) {
						hasGiven = true;
						setTString(hContact, "FirstName", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "FAMILY")) != NULL && XmlGetText(m) != NULL) {
						hasFamily = true;
						setTString(hContact, "LastName", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "MIDDLE")) != NULL && XmlGetText(m) != NULL) {
						hasMiddle = true;
						setTString(hContact, "MiddleName", XmlGetText(m));
				}	}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("EMAIL"))) {
				// E-mail address(es)
				if ((m=XmlGetChild(n, "USERID")) == NULL)	// Some bad client put e-mail directly in <EMAIL/> instead of <USERID/>
					m = n;
				if (XmlGetText(m) != NULL) {
					char text[100];
					if (hContact != NULL) {
						if (nEmail == 0)
							mir_strcpy(text, "e-mail");
						else
							mir_snprintf(text, _countof(text), "e-mail%d", nEmail - 1);
					}
					else mir_snprintf(text, _countof(text), "e-mail%d", nEmail);
					setTString(hContact, text, XmlGetText(m));

					if (hContact == NULL) {
						mir_snprintf(text, _countof(text), "e-mailFlag%d", nEmail);
						int nFlag = 0;
						if (XmlGetChild(n, "HOME") != NULL) nFlag |= JABBER_VCEMAIL_HOME;
						if (XmlGetChild(n, "WORK") != NULL) nFlag |= JABBER_VCEMAIL_WORK;
						if (XmlGetChild(n, "INTERNET") != NULL) nFlag |= JABBER_VCEMAIL_INTERNET;
						if (XmlGetChild(n, "X400") != NULL) nFlag |= JABBER_VCEMAIL_X400;
						setWord(text, nFlag);
					}
					nEmail++;
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("BDAY"))) {
				// Birthday
				if (!hasBday && XmlGetText(n) != NULL) {
					if (hContact != NULL) {
						if (_stscanf(XmlGetText(n), _T("%d-%d-%d"), &nYear, &nMonth, &nDay) == 3) {
							hasBday = true;
							setWord(hContact, "BirthYear", (WORD)nYear);
							setByte(hContact, "BirthMonth", (BYTE) nMonth);
							setByte(hContact, "BirthDay", (BYTE) nDay);

							SYSTEMTIME sToday = {0};
							GetLocalTime(&sToday);
							int nAge = sToday.wYear - nYear;
							if (sToday.wMonth < nMonth || (sToday.wMonth == nMonth && sToday.wDay < nDay))
								nAge--;
							if (nAge)
								setWord(hContact, "Age", (WORD)nAge);
						}
					}
					else {
						hasBday = true;
						setTString("BirthDate", XmlGetText(n));
				}	}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("GENDER"))) {
				// Gender
				if (!hasGender && XmlGetText(n) != NULL) {
					if (hContact != NULL) {
						if (XmlGetText(n)[0] && strchr("mMfF", XmlGetText(n)[0]) != NULL) {
							hasGender = true;
							setByte(hContact, "Gender", (BYTE) toupper(XmlGetText(n)[0]));
						}
					}
					else {
						hasGender = true;
						setTString("GenderString", XmlGetText(n));
				}	}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("ADR"))) {
				if (!hasHome && XmlGetChild(n, "HOME") != NULL) {
					// Home address
					TCHAR text[128];
					hasHome = true;
					if ((m=XmlGetChild(n, "STREET")) != NULL && XmlGetText(m) != NULL) {
						hasHomeStreet = true;
						if (hContact != NULL) {
							if ((o=XmlGetChild(n, "EXTADR")) != NULL && XmlGetText(o) != NULL)
								mir_sntprintf(text, _T("%s\r\n%s"), XmlGetText(m), XmlGetText(o));
							else if ((o=XmlGetChild(n, "EXTADD")) != NULL && XmlGetText(o) != NULL)
								mir_sntprintf(text, _T("%s\r\n%s"), XmlGetText(m), XmlGetText(o));
							else
								_tcsncpy_s(text, XmlGetText(m), _TRUNCATE);
							text[_countof(text)-1] = '\0';
							setTString(hContact, "Street", text);
						}
						else {
							setTString(hContact, "Street", XmlGetText(m));
							if ((m=XmlGetChild(n, "EXTADR")) == NULL)
								m = XmlGetChild(n, "EXTADD");
							if (m != NULL && XmlGetText(m) != NULL) {
								hasHomeStreet2 = true;
								setTString(hContact, "Street2", XmlGetText(m));
					}	}	}

					if ((m=XmlGetChild(n, "LOCALITY")) != NULL && XmlGetText(m) != NULL) {
						hasHomeLocality = true;
						setTString(hContact, "City", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "REGION")) != NULL && XmlGetText(m) != NULL) {
						hasHomeRegion = true;
						setTString(hContact, "State", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "PCODE")) != NULL && XmlGetText(m) != NULL) {
						hasHomePcode = true;
						setTString(hContact, "ZIP", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "CTRY")) == NULL || XmlGetText(m) == NULL)	// Some bad client use <COUNTRY/> instead of <CTRY/>
						m = XmlGetChild(n, "COUNTRY");
					if (m != NULL && XmlGetText(m) != NULL) {
						hasHomeCtry = true;
						setTString(hContact, "Country", XmlGetText(m));
				}	}

				if (!hasWork && XmlGetChild(n, "WORK") != NULL) {
					// Work address
					hasWork = true;
					if ((m=XmlGetChild(n, "STREET")) != NULL && XmlGetText(m) != NULL) {
						TCHAR text[128];
						hasWorkStreet = true;
						if (hContact != NULL) {
							if ((o=XmlGetChild(n, "EXTADR")) != NULL && XmlGetText(o) != NULL)
								mir_sntprintf(text, _T("%s\r\n%s"), XmlGetText(m), XmlGetText(o));
							else if ((o=XmlGetChild(n, "EXTADD")) != NULL && XmlGetText(o) != NULL)
								mir_sntprintf(text, _T("%s\r\n%s"), XmlGetText(m), XmlGetText(o));
							else
								_tcsncpy_s(text, XmlGetText(m), _TRUNCATE);
							text[_countof(text)-1] = '\0';
							setTString(hContact, "CompanyStreet", text);
						}
						else {
							setTString(hContact, "CompanyStreet", XmlGetText(m));
							if ((m=XmlGetChild(n, "EXTADR")) == NULL)
								m = XmlGetChild(n, "EXTADD");
							if (m != NULL && XmlGetText(m) != NULL) {
								hasWorkStreet2 = true;
								setTString(hContact, "CompanyStreet2", XmlGetText(m));
					}	}	}

					if ((m=XmlGetChild(n, "LOCALITY")) != NULL && XmlGetText(m) != NULL) {
						hasWorkLocality = true;
						setTString(hContact, "CompanyCity", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "REGION")) != NULL && XmlGetText(m) != NULL) {
						hasWorkRegion = true;
						setTString(hContact, "CompanyState", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "PCODE")) != NULL && XmlGetText(m) != NULL) {
						hasWorkPcode = true;
						setTString(hContact, "CompanyZIP", XmlGetText(m));
					}
					if ((m=XmlGetChild(n, "CTRY")) == NULL || XmlGetText(m) == NULL)	// Some bad client use <COUNTRY/> instead of <CTRY/>
						m = XmlGetChild(n, "COUNTRY");
					if (m != NULL && XmlGetText(m) != NULL) {
						hasWorkCtry = true;
						setTString(hContact, "CompanyCountry", XmlGetText(m));
				}	}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("TEL"))) {
				// Telephone/Fax/Cellular
				if ((m=XmlGetChild(n, "NUMBER")) != NULL && XmlGetText(m) != NULL) {
					if (hContact != NULL) {
						if (!hasFax && XmlGetChild(n, "FAX") != NULL) {
							hasFax = true;
							setTString(hContact, "Fax", XmlGetText(m));
						}
						else if (!hasCell && XmlGetChild(n, "CELL") != NULL) {
							hasCell = true;
							setTString(hContact, "Cellular", XmlGetText(m));
						}
						else if (!hasPhone &&
							(XmlGetChild(n, "HOME") != NULL || XmlGetChild(n, "WORK") != NULL || XmlGetChild(n, "VOICE") != NULL ||
							(XmlGetChild(n, "FAX") == NULL &&
							 XmlGetChild(n, "PAGER") == NULL &&
							 XmlGetChild(n, "MSG") == NULL &&
							 XmlGetChild(n, "CELL") == NULL &&
							 XmlGetChild(n, "VIDEO") == NULL &&
							 XmlGetChild(n, "BBS") == NULL &&
							 XmlGetChild(n, "MODEM") == NULL &&
							 XmlGetChild(n, "ISDN") == NULL &&
							 XmlGetChild(n, "PCS") == NULL)))
						{
							hasPhone = true;
							setTString(hContact, "Phone", XmlGetText(m));
						}
					}
					else {
						char text[100];
						mir_snprintf(text, _countof(text), "Phone%d", nPhone);
						setTString(text, XmlGetText(m));

						mir_snprintf(text, _countof(text), "PhoneFlag%d", nPhone);
						int nFlag = 0;
						if (XmlGetChild(n, "HOME")  != NULL) nFlag |= JABBER_VCTEL_HOME;
						if (XmlGetChild(n, "WORK")  != NULL) nFlag |= JABBER_VCTEL_WORK;
						if (XmlGetChild(n, "VOICE") != NULL) nFlag |= JABBER_VCTEL_VOICE;
						if (XmlGetChild(n, "FAX")   != NULL) nFlag |= JABBER_VCTEL_FAX;
						if (XmlGetChild(n, "PAGER") != NULL) nFlag |= JABBER_VCTEL_PAGER;
						if (XmlGetChild(n, "MSG")   != NULL) nFlag |= JABBER_VCTEL_MSG;
						if (XmlGetChild(n, "CELL")  != NULL) nFlag |= JABBER_VCTEL_CELL;
						if (XmlGetChild(n, "VIDEO") != NULL) nFlag |= JABBER_VCTEL_VIDEO;
						if (XmlGetChild(n, "BBS")   != NULL) nFlag |= JABBER_VCTEL_BBS;
						if (XmlGetChild(n, "MODEM") != NULL) nFlag |= JABBER_VCTEL_MODEM;
						if (XmlGetChild(n, "ISDN")  != NULL) nFlag |= JABBER_VCTEL_ISDN;
						if (XmlGetChild(n, "PCS")   != NULL) nFlag |= JABBER_VCTEL_PCS;
						setWord(text, nFlag);
						nPhone++;
					}
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("URL"))) {
				// Homepage
				if (!hasUrl && XmlGetText(n) != NULL) {
					hasUrl = true;
					setTString(hContact, "Homepage", XmlGetText(n));
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("ORG"))) {
				if (!hasOrgname && !hasOrgunit) {
					if ((m = XmlGetChild(n, "ORGNAME")) != NULL && XmlGetText(m) != NULL) {
						hasOrgname = true;
						setTString(hContact, "Company", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "ORGUNIT")) != NULL && XmlGetText(m) != NULL) {	// The real vCard can have multiple <ORGUNIT/> but we will only display the first one
						hasOrgunit = true;
						setTString(hContact, "CompanyDepartment", XmlGetText(m));
					}
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("ROLE"))) {
				if (!hasRole && XmlGetText(n) != NULL) {
					hasRole = true;
					setTString(hContact, "Role", XmlGetText(n));
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("TITLE"))) {
				if (!hasTitle && XmlGetText(n) != NULL) {
					hasTitle = true;
					setTString(hContact, "CompanyPosition", XmlGetText(n));
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("DESC"))) {
				if (!hasDesc && XmlGetText(n) != NULL) {
					hasDesc = true;
					CMString tszMemo(XmlGetText(n));
					tszMemo.Replace(_T("\n"), _T("\r\n"));
					setTString(hContact, "About", tszMemo);
				}
			}
			else if (!mir_tstrcmp(XmlGetName(n), _T("PHOTO")))
				OnIqResultGetVcardPhoto(n, hContact, hasPhoto);
	}	}

	if (hasFn && !hasNick) {
		ptrT nick(getTStringA(hContact, "Nick"));
		ptrT jidNick(JabberNickFromJID(jid));
		if (!nick || (jidNick && !mir_tstrcmpi(nick, jidNick)))
			setTString(hContact, "Nick", ptrT(getTStringA(hContact, "FullName")));
	}
	if (!hasFn)
		delSetting(hContact, "FullName");
	// We are not deleting "Nick"
	//		if (!hasNick)
	//			delSetting(hContact, "Nick");
	if (!hasGiven)
		delSetting(hContact, "FirstName");
	if (!hasFamily)
		delSetting(hContact, "LastName");
	if (!hasMiddle)
		delSetting(hContact, "MiddleName");
	if (hContact != NULL) {
		while (true) {
			if (nEmail <= 0)
				delSetting(hContact, "e-mail");
			else {
				char text[100];
				mir_snprintf(text, _countof(text), "e-mail%d", nEmail - 1);
				if (db_get_s(hContact, m_szModuleName, text, &dbv)) break;
				db_free(&dbv);
				delSetting(hContact, text);
			}
			nEmail++;
		}
	}
	else {
		while (true) {
			char text[100];
			mir_snprintf(text, _countof(text), "e-mail%d", nEmail);
			if (getString(text, &dbv)) break;
			db_free(&dbv);
			delSetting(text);
			mir_snprintf(text, _countof(text), "e-mailFlag%d", nEmail);
			delSetting(text);
			nEmail++;
		}
	}

	if (!hasBday) {
		delSetting(hContact, "BirthYear");
		delSetting(hContact, "BirthMonth");
		delSetting(hContact, "BirthDay");
		delSetting(hContact, "BirthDate");
		delSetting(hContact, "Age");
	}
	if (!hasGender) {
		if (hContact != NULL)
			delSetting(hContact, "Gender");
		else
			delSetting("GenderString");
	}
	if (hContact != NULL) {
		if (!hasPhone)
			delSetting(hContact, "Phone");
		if (!hasFax)
			delSetting(hContact, "Fax");
		if (!hasCell)
			delSetting(hContact, "Cellular");
	}
	else {
		while (true) {
			char text[100];
			mir_snprintf(text, _countof(text), "Phone%d", nPhone);
			if (getString(text, &dbv)) break;
			db_free(&dbv);
			delSetting(text);
			mir_snprintf(text, _countof(text), "PhoneFlag%d", nPhone);
			delSetting(text);
			nPhone++;
		}
	}

	if (!hasHomeStreet)
		delSetting(hContact, "Street");
	if (!hasHomeStreet2 && hContact == NULL)
		delSetting(hContact, "Street2");
	if (!hasHomeLocality)
		delSetting(hContact, "City");
	if (!hasHomeRegion)
		delSetting(hContact, "State");
	if (!hasHomePcode)
		delSetting(hContact, "ZIP");
	if (!hasHomeCtry)
		delSetting(hContact, "Country");
	if (!hasWorkStreet)
		delSetting(hContact, "CompanyStreet");
	if (!hasWorkStreet2 && hContact == NULL)
		delSetting(hContact, "CompanyStreet2");
	if (!hasWorkLocality)
		delSetting(hContact, "CompanyCity");
	if (!hasWorkRegion)
		delSetting(hContact, "CompanyState");
	if (!hasWorkPcode)
		delSetting(hContact, "CompanyZIP");
	if (!hasWorkCtry)
		delSetting(hContact, "CompanyCountry");
	if (!hasUrl)
		delSetting(hContact, "Homepage");
	if (!hasOrgname)
		delSetting(hContact, "Company");
	if (!hasOrgunit)
		delSetting(hContact, "CompanyDepartment");
	if (!hasRole)
		delSetting(hContact, "Role");
	if (!hasTitle)
		delSetting(hContact, "CompanyPosition");
	if (!hasDesc)
		delSetting(hContact, "About");

	if (id == m_ThreadInfo->resolveID) {
		const TCHAR *p = _tcschr(jid, '@');
		ResolveTransportNicks((p != NULL) ? p + 1 : jid);
	}
	else {
		if ((hContact = HContactFromJID(jid)) != NULL)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
	}
}

void CJabberProto::OnIqResultSetVcard(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetVcard");
	if (XmlGetAttrValue(iqNode, _T("type")))
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
}

void CJabberProto::OnIqResultSetSearch(HXML iqNode, CJabberIqInfo*)
{
	HXML queryNode, n;
	const TCHAR *type, *jid;
	int id;

	debugLogA("<iq/> iqIdGetSearch");
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;
	if ((id = JabberGetPacketID(iqNode)) == -1) return;

	if (!mir_tstrcmp(type, _T("result"))) {
		if ((queryNode = XmlGetChild(iqNode, "query")) == NULL)
			return;

		PROTOSEARCHRESULT psr = { 0 };
		psr.cbSize = sizeof(psr);
		for (int i = 0;; i++) {
			HXML itemNode = XmlGetChild(queryNode, i);
			if (!itemNode)
				break;

			if (!mir_tstrcmp(XmlGetName(itemNode), _T("item"))) {
				if ((jid = XmlGetAttrValue(itemNode, _T("jid"))) != NULL) {
					psr.id.t = (TCHAR*)jid;
					debugLog(_T("Result jid = %s"), jid);
					if ((n = XmlGetChild(itemNode, "nick")) != NULL && XmlGetText(n) != NULL)
						psr.nick.t = (TCHAR*)XmlGetText(n);
					else
						psr.nick.t = _T("");
					if ((n = XmlGetChild(itemNode, "first")) != NULL && XmlGetText(n) != NULL)
						psr.firstName.t = (TCHAR*)XmlGetText(n);
					else
						psr.firstName.t = _T("");
					if ((n = XmlGetChild(itemNode, "last")) != NULL && XmlGetText(n) != NULL)
						psr.lastName.t = (TCHAR*)XmlGetText(n);
					else
						psr.lastName.t = _T("");
					if ((n = XmlGetChild(itemNode, "email")) != NULL && XmlGetText(n) != NULL)
						psr.email.t = (TCHAR*)XmlGetText(n);
					else
						psr.email.t = _T("");
					psr.flags = PSR_TCHAR;
					ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
				}
			}
		}

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
	else if (!mir_tstrcmp(type, _T("error")))
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

void CJabberProto::OnIqResultExtSearch(HXML iqNode, CJabberIqInfo*)
{
	HXML queryNode;

	debugLogA("<iq/> iqIdGetExtSearch");
	const TCHAR *type = XmlGetAttrValue(iqNode, _T("type"));
	if (type == NULL)
		return;

	int id = JabberGetPacketID(iqNode);
	if (id == -1)
		return;

	if (!mir_tstrcmp(type, _T("result"))) {
		if ((queryNode=XmlGetChild(iqNode , "query")) == NULL) return;
		if ((queryNode=XmlGetChild(queryNode , "x")) == NULL) return;
		for (int i=0; ; i++) {
			HXML itemNode = XmlGetChild(queryNode ,i);
			if (!itemNode)
				break;
			if (mir_tstrcmp(XmlGetName(itemNode), _T("item")))
				continue;

			PROTOSEARCHRESULT  psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_TCHAR;

			for (int j=0; ; j++) {
				HXML fieldNode = XmlGetChild(itemNode ,j);
				if (!fieldNode)
					break;

				if (mir_tstrcmp(XmlGetName(fieldNode), _T("field")))
					continue;

				const TCHAR *fieldName = XmlGetAttrValue(fieldNode, _T("var"));
				if (fieldName == NULL)
					continue;

				HXML n = XmlGetChild(fieldNode , "value");
				if (n == NULL)
					continue;

				if (!mir_tstrcmp(fieldName, _T("jid"))) {
					psr.id.t = (TCHAR*)XmlGetText(n);
					debugLog(_T("Result jid = %s"), psr.id.t);
				}
				else if (!mir_tstrcmp(fieldName, _T("nickname")))
					psr.nick.t = (XmlGetText(n) != NULL) ? (TCHAR*)XmlGetText(n) : _T("");
				else if (!mir_tstrcmp(fieldName, _T("fn")))
					psr.firstName.t = (XmlGetText(n) != NULL) ? (TCHAR*)XmlGetText(n) : _T("");
				else if (!mir_tstrcmp(fieldName, _T("given")))
					psr.firstName.t = (XmlGetText(n) != NULL) ? (TCHAR*)XmlGetText(n) : _T("");
				else if (!mir_tstrcmp(fieldName, _T("family")))
					psr.lastName.t = (XmlGetText(n) != NULL) ? (TCHAR*)XmlGetText(n) : _T("");
				else if (!mir_tstrcmp(fieldName, _T("email")))
					psr.email.t = (XmlGetText(n) != NULL) ? (TCHAR*)XmlGetText(n) : _T("");
			}

			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
		}

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
	else if (!mir_tstrcmp(type, _T("error")))
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

void CJabberProto::OnIqResultSetPassword(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetPassword");

	const TCHAR *type = XmlGetAttrValue(iqNode, _T("type"));
	if (type == NULL)
		return;

	if (!mir_tstrcmp(type, _T("result"))) {
		_tcsncpy_s(m_ThreadInfo->conn.password, m_ThreadInfo->tszNewPassword, _TRUNCATE);
		MessageBox(NULL, TranslateT("Password is successfully changed. Don't forget to update your password in the Jabber protocol option."), TranslateT("Change Password"), MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND);
	}
	else if (!mir_tstrcmp(type, _T("error")))
		MessageBox(NULL, TranslateT("Password cannot be changed."), TranslateT("Change Password"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
}

void CJabberProto::OnIqResultGetVCardAvatar(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> OnIqResultGetVCardAvatar");

	const TCHAR *from = XmlGetAttrValue(iqNode, _T("from"));
	if (from == NULL)
		return;

	MCONTACT hContact = HContactFromJID(from);
	if (hContact == NULL)
		return;

	const TCHAR *type;
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) == NULL) return;
	if (mir_tstrcmp(type, _T("result"))) return;

	HXML vCard = XmlGetChild(iqNode , "vCard");
	if (vCard == NULL) return;
	vCard = XmlGetChild(vCard , "PHOTO");
	if (vCard == NULL) return;

	if (XmlGetChildCount(vCard) == 0) {
		delSetting(hContact, "AvatarHash");
		if ( ptrT( getTStringA(hContact, "AvatarSaved")) != NULL) {
			delSetting(hContact, "AvatarSaved");
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, NULL);
		}
		return;
	}

	const TCHAR *mimeType = XmlGetText( XmlGetChild(vCard , "TYPE"));
	HXML n = XmlGetChild(vCard , "BINVAL");
	if (n == NULL)
		return;

	setByte(hContact, "AvatarXVcard", 1);
	OnIqResultGotAvatar(hContact, n, mimeType);
}

void CJabberProto::OnIqResultGetClientAvatar(HXML iqNode, CJabberIqInfo*)
{
	const TCHAR *type;

	debugLogA("<iq/> iqIdResultGetClientAvatar");

	const TCHAR *from = XmlGetAttrValue(iqNode, _T("from"));
	if (from == NULL)
		return;
	MCONTACT hContact = HContactFromJID(from);
	if (hContact == NULL)
		return;

	HXML n = NULL;
	if ((type = XmlGetAttrValue(iqNode, _T("type"))) != NULL && !mir_tstrcmp(type, _T("result"))) {
		HXML queryNode = XmlGetChild(iqNode , "query");
		if (queryNode != NULL) {
			const TCHAR *xmlns = XmlGetAttrValue(queryNode, _T("xmlns"));
			if (!mir_tstrcmp(xmlns, JABBER_FEAT_AVATAR))
				n = XmlGetChild(queryNode , "data");
		}
	}

	if (n != NULL) {
		OnIqResultGotAvatar(hContact, n, XmlGetAttrValue(n, _T("mimetype")));
		return;
	}
	
	TCHAR szJid[JABBER_MAX_JID_LEN];
	mir_tstrncpy(szJid, from, _countof(szJid));
	TCHAR *res = _tcschr(szJid, _T('/'));
	if (res != NULL)
		*res = 0;

	// Try server stored avatar
	XmlNodeIq iq( AddIQ(&CJabberProto::OnIqResultGetServerAvatar, JABBER_IQ_TYPE_GET, szJid));
	iq << XQUERY(JABBER_FEAT_SERVER_AVATAR);
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnIqResultGetServerAvatar(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdResultGetServerAvatar");

	const TCHAR *from = XmlGetAttrValue(iqNode, _T("from"));
	if (from == NULL)
		return;

	MCONTACT hContact = HContactFromJID(from);
	if (hContact == NULL)
		return;

	HXML n = NULL;
	const TCHAR *type = XmlGetAttrValue(iqNode, _T("type"));
	if (!mir_tstrcmp(type, _T("result"))) {
		HXML queryNode = XmlGetChild(iqNode , "query");
		if (queryNode != NULL) {
			const TCHAR *xmlns = XmlGetAttrValue(queryNode, _T("xmlns"));
			if (!mir_tstrcmp(xmlns, JABBER_FEAT_SERVER_AVATAR))
				n = XmlGetChild(queryNode, "data");
		}
	}

	if (n != NULL) {
		OnIqResultGotAvatar(hContact, n, XmlGetAttrValue(n, _T("mimetype")));
		return;
	}
	
	TCHAR szJid[JABBER_MAX_JID_LEN];
	mir_tstrncpy(szJid, from, _countof(szJid));
	TCHAR *res = _tcschr(szJid, _T('/'));
	if (res != NULL)
		*res = 0;

	// Try VCard photo
	m_ThreadInfo->send(
		XmlNodeIq( AddIQ(&CJabberProto::OnIqResultGetVCardAvatar, JABBER_IQ_TYPE_GET, szJid))
			<< XCHILDNS(_T("vCard"), JABBER_FEAT_VCARD_TEMP));
}


void CJabberProto::OnIqResultGotAvatar(MCONTACT hContact, HXML n, const TCHAR *mimeType)
{
	unsigned resultLen;
	ptrA body((char*)mir_base64_decode( _T2A(XmlGetText(n)), &resultLen));
	if (body == NULL)
		return;

	int pictureType;
	if (mimeType != NULL) {
		     if (!mir_tstrcmp(mimeType, _T("image/jpeg"))) pictureType = PA_FORMAT_JPEG;
		else if (!mir_tstrcmp(mimeType, _T("image/png")))  pictureType = PA_FORMAT_PNG;
		else if (!mir_tstrcmp(mimeType, _T("image/gif")))  pictureType = PA_FORMAT_GIF;
		else if (!mir_tstrcmp(mimeType, _T("image/bmp")))  pictureType = PA_FORMAT_BMP;
		else {
LBL_ErrFormat:
			debugLog(_T("Invalid mime type specified for picture: %s"), mimeType);
			return;
		}
	}
	else if ((pictureType = ProtoGetBufferFormat(body, 0)) == PA_FORMAT_UNKNOWN)
		goto LBL_ErrFormat;

	PROTO_AVATAR_INFORMATION ai;
	ai.format = pictureType;
	ai.hContact = hContact;

	TCHAR tszFileName[MAX_PATH];
	if (getByte(hContact, "AvatarType", PA_FORMAT_UNKNOWN) != (unsigned char)pictureType) {
		GetAvatarFileName(hContact, tszFileName, _countof(tszFileName));
		DeleteFile(tszFileName);
	}

	setByte(hContact, "AvatarType", pictureType);

	BYTE digest[MIR_SHA1_HASH_SIZE];
	mir_sha1_ctx sha;
	mir_sha1_init(&sha);
	mir_sha1_append(&sha, (BYTE*)(char*)body, resultLen);
	mir_sha1_finish(&sha, digest);

	GetAvatarFileName(hContact, tszFileName, _countof(tszFileName));
	_tcsncpy_s(ai.filename, tszFileName, _TRUNCATE);

	FILE *out = _tfopen(tszFileName, _T("wb"));
	if (out != NULL) {
		fwrite(body, resultLen, 1, out);
		fclose(out);

		char buffer[41];
		setString(hContact, "AvatarSaved", bin2hex(digest, sizeof(digest), buffer));
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai), NULL);
		debugLog(_T("Broadcast new avatar: %s"),ai.filename);
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai), NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks

void CJabberProto::OnIqResultDiscoBookmarks(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: list of bookmarks
	// ACTION: refresh bookmarks dialog
	debugLogA("<iq/> iqIdGetBookmarks");
	const TCHAR *type = XmlGetAttrValue(iqNode, _T("type"));
	if (type == NULL)
		return;

	const TCHAR *jid;
	if (!mir_tstrcmp(type, _T("result"))) {
		if (m_ThreadInfo && !(m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)) {
			m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(TRUE);
		}

		if (HXML storageNode = XPathT(iqNode, "query/storage[@xmlns='storage:bookmarks']")) {
			ListRemoveList(LIST_BOOKMARK);

			HXML itemNode;
			for (int i = 0; itemNode = XmlGetChild(storageNode, i); i++) {
				if (LPCTSTR name = XmlGetName(itemNode)) {
					if (!mir_tstrcmp(name, _T("conference")) && (jid = XmlGetAttrValue(itemNode, _T("jid")))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->name = mir_tstrdup(XmlGetAttrValue(itemNode, _T("name")));
						item->type = mir_tstrdup(_T("conference"));
						item->bUseResource = TRUE;
						item->nick = mir_tstrdup(XPathT(itemNode, "nick"));
						item->password = mir_tstrdup(XPathT(itemNode, "password"));

						const TCHAR *autoJ = XmlGetAttrValue(itemNode, _T("autojoin"));
						if (autoJ != NULL)
							item->bAutoJoin = (!mir_tstrcmp(autoJ, _T("true")) || !mir_tstrcmp(autoJ, _T("1"))) ? true : false;
					}
					else if (!mir_tstrcmp(name, _T("url")) && (jid = XmlGetAttrValue(itemNode, _T("url")))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->bUseResource = TRUE;
						item->name = mir_tstrdup(XmlGetAttrValue(itemNode, _T("name")));
						item->type = mir_tstrdup(_T("url"));
					}
				}
			}

			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_REFRESH);
			m_ThreadInfo->bBookmarksLoaded = TRUE;
			OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_BOOKMARKS);
		}
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		if (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE) {
			m_ThreadInfo->jabberServerCaps &= ~JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(TRUE);
			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_ACTIVATE);
		}
	}
}

void CJabberProto::SetBookmarkRequest (XmlNodeIq& iq)
{
	HXML query = iq << XQUERY(JABBER_FEAT_PRIVATE_STORAGE);
	HXML storage = query << XCHILDNS(_T("storage"), _T("storage:bookmarks"));

	LISTFOREACH(i, this, LIST_BOOKMARK)
	{
		JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
		if (item == NULL || item->jid == NULL)
			continue;
		
		if (!mir_tstrcmp(item->type, _T("conference"))) {
			HXML itemNode = storage << XCHILD(_T("conference")) << XATTR(_T("jid"), item->jid);
			if (item->name)
				itemNode << XATTR(_T("name"), item->name);
			if (item->bAutoJoin)
				itemNode << XATTRI(_T("autojoin"), 1);
			if (item->nick)
				itemNode << XCHILD(_T("nick"), item->nick);
			if (item->password)
				itemNode << XCHILD(_T("password"), item->password);
		}

		if (!mir_tstrcmp(item->type, _T("url"))) {
			HXML itemNode = storage << XCHILD(_T("url")) << XATTR(_T("url"), item->jid);
			if (item->name)
				itemNode << XATTR(_T("name"), item->name);
		}
	}
}

void CJabberProto::OnIqResultSetBookmarks(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: server's response
	// ACTION: refresh bookmarks list dialog

	debugLogA("<iq/> iqIdSetBookmarks");

	const TCHAR *type = XmlGetAttrValue(iqNode, _T("type"));
	if (type == NULL)
		return;

	if (!mir_tstrcmp(type, _T("result"))) {
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_REFRESH);
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		HXML errorNode = XmlGetChild(iqNode, "error");
		TCHAR *str = JabberErrorMsg(errorNode);
		MessageBox(NULL, str, TranslateT("Jabber Bookmarks Error"), MB_OK | MB_SETFOREGROUND);
		mir_free(str);
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_ACTIVATE);
	}
}

// last activity (XEP-0012) support
void CJabberProto::OnIqResultLastActivity(HXML iqNode, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->m_szFrom));
	if (r == NULL)
		return;

	time_t lastActivity = -1;
	if (pInfo->m_nIqType == JABBER_IQ_TYPE_RESULT) {
		LPCTSTR szSeconds = XPathT(iqNode, "query[@xmlns='jabber:iq:last']/@seconds");
		if (szSeconds) {
			int nSeconds = _ttoi(szSeconds);
			if (nSeconds > 0)
				lastActivity = time(0) - nSeconds;
		}

		LPCTSTR szLastStatusMessage = XPathT(iqNode, "query[@xmlns='jabber:iq:last']");
		if (szLastStatusMessage) // replace only if it exists
			r->m_tszStatusMessage = mir_tstrdup(szLastStatusMessage);
	}

	r->m_dwIdleStartTime = lastActivity;

	JabberUserInfoUpdate(pInfo->GetHContact());
}

// entity time (XEP-0202) support
void CJabberProto::OnIqResultEntityTime(HXML pIqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->m_hContact)
		return;

	if (pInfo->m_nIqType == JABBER_IQ_TYPE_RESULT) {
		LPCTSTR szTzo = XPathFmt(pIqNode, _T("time[@xmlns='%s']/tzo"), JABBER_FEAT_ENTITY_TIME);
		if (szTzo && szTzo[0]) {
			LPCTSTR szMin = _tcschr(szTzo, ':');
			int nTz = _ttoi(szTzo) * -2;
			nTz += (nTz < 0 ? -1 : 1) * (szMin ? _ttoi(szMin + 1) / 30 : 0);

			TIME_ZONE_INFORMATION tzinfo;
			if (GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_DAYLIGHT)
				nTz -= tzinfo.DaylightBias / 30;

			setByte(pInfo->m_hContact, "Timezone", (signed char)nTz);

			LPCTSTR szTz = XPathFmt(pIqNode, _T("time[@xmlns='%s']/tz"), JABBER_FEAT_ENTITY_TIME);
			if (szTz)
				setTString(pInfo->m_hContact, "TzName", szTz);
			else
				delSetting(pInfo->m_hContact, "TzName");
			return;
		}
	}
	else if (pInfo->m_nIqType == JABBER_IQ_TYPE_ERROR) {
		if (getWord(pInfo->m_hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			return;
	}

	delSetting(pInfo->m_hContact, "Timezone");
	delSetting(pInfo->m_hContact, "TzName");
}
