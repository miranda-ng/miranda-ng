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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"

void CJabberProto::OnIqResultServerDiscoInfo(HXML iqNode, CJabberIqInfo*)
{
	if (iqNode == nullptr)
		return;

	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (mir_wstrcmp(type, L"result"))
		return;

	HXML query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_INFO);
	if (query == nullptr)
		return;

	HXML identity;
	for (int i = 1; (identity = XmlGetNthChild(query, L"identity", i)) != nullptr; i++) {
		JABBER_DISCO_FIELD tmp = {
			XmlGetAttrValue(identity, L"category"),
			XmlGetAttrValue(identity, L"type"),
			XmlGetAttrValue(identity, L"name") };

		if (!mir_wstrcmp(tmp.category, L"pubsub") && !mir_wstrcmp(tmp.type, L"pep")) {
			m_bPepSupported = true;

			if (m_options.UseOMEMO)
			{
				//publish ndes, precreation is not required
				OmemoPublishNodes();
			}

			EnableMenuItems(true);
			RebuildInfoFrame();
			continue;
		}

		NotifyFastHook(hDiscoInfoResult, (WPARAM)&tmp, (LPARAM)(IJabberInterface*)this);
	}

	if (m_ThreadInfo) {
		HXML feature;
		for (int i = 1; (feature = XmlGetNthChild(query, L"feature", i)) != nullptr; i++) {
			const wchar_t *featureName = XmlGetAttrValue(feature, L"var");
			if (!featureName)
				continue;

			for (int j = 0; g_JabberFeatCapPairs[j].szFeature; j++) {
				if (!mir_wstrcmp(g_JabberFeatCapPairs[j].szFeature, featureName)) {
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
	const wchar_t *szGroupDelimeter = nullptr;
	bool bPrivateStorageSupport = false;

	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		bPrivateStorageSupport = true;
		szGroupDelimeter = XPathFmt(iqNode, L"query[@xmlns='%s']/roster[@xmlns='%s']", JABBER_FEAT_PRIVATE_STORAGE, JABBER_FEAT_NESTED_ROSTER_GROUPS);
		if (szGroupDelimeter && !szGroupDelimeter[0])
			szGroupDelimeter = nullptr; // "" as roster delimeter is not supported :)
	}

	// global fuckup
	if (m_ThreadInfo == nullptr)
		return;

	// is our default delimiter?
	if ((!szGroupDelimeter && bPrivateStorageSupport) || (szGroupDelimeter && mir_wstrcmp(szGroupDelimeter, L"\\")))
		m_ThreadInfo->send(
			XmlNodeIq(L"set", SerialNext()) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
				<< XCHILD(L"roster", L"\\") << XATTR(L"xmlns", JABBER_FEAT_NESTED_ROSTER_GROUPS));

	// roster request
	wchar_t *szUserData = mir_wstrdup(szGroupDelimeter ? szGroupDelimeter : L"\\");
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetRoster, JABBER_IQ_TYPE_GET, nullptr, 0, -1, (void*)szUserData))
			<< XCHILDNS(L"query", JABBER_FEAT_IQ_ROSTER));
}

void CJabberProto::OnIqResultNotes(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML hXmlData = XPathFmt(iqNode, L"query[@xmlns='%s']/storage[@xmlns='%s']",
			JABBER_FEAT_PRIVATE_STORAGE, JABBER_FEAT_MIRANDA_NOTES);
		if (hXmlData) m_notes.LoadXml(hXmlData);
	}
}

void CJabberProto::OnProcessLoginRq(ThreadData *info, DWORD rq)
{
	if (info == nullptr)
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
				if (item != nullptr && !mir_wstrcmp(item->type, L"conference") && item->bAutoJoin)
					ll.insert(item);
			}

			for (int j=0; j < ll.getCount(); j++) {
				JABBER_LIST_ITEM *item = ll[j];

				wchar_t room[256], text[128];
				wcsncpy_s(text, item->jid, _TRUNCATE);
				wcsncpy_s(room, text, _TRUNCATE);
				wchar_t *p = wcstok(room, L"@");
				wchar_t *server = wcstok(nullptr, L"@");
				if (item->nick && item->nick[0])
					GroupchatJoinRoom(server, p, item->nick, item->password, true);
				else {
					ptrW nick(getWStringA(HContactFromJID(m_szJabberJID), "MyNick"));
					if (nick == nullptr)
						nick = getWStringA("Nick");
					if (nick == nullptr)
						nick = JabberNickFromJID(m_szJabberJID);

					GroupchatJoinRoom(server, p, nick, item->password, true);
				}
			}
		}

		OnProcessLoginRq(info, JABBER_LOGIN_BOOKMARKS_AJ);
	}
}

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
				<< XCHILDNS(L"roster", JABBER_FEAT_NESTED_ROSTER_GROUPS));
	}

	// Server-side notes
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultNotes, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
			<< XCHILDNS(L"storage", JABBER_FEAT_MIRANDA_NOTES));
	
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultDiscoBookmarks, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE) << XCHILDNS(L"storage", L"storage:bookmarks"));

	m_bPepSupported = false;
	m_ThreadInfo->jabberServerCaps = JABBER_RESOURCE_CAPS_NONE;

	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultServerDiscoInfo, JABBER_IQ_TYPE_GET, _A2T(m_ThreadInfo->conn.server)))
			<< XQUERY(JABBER_FEAT_DISCO_INFO));

	QueryPrivacyLists(m_ThreadInfo);

	ptrA szServerName(getStringA("LastLoggedServer"));
	if (szServerName == nullptr || mir_strcmp(m_ThreadInfo->conn.server, szServerName))
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
	const wchar_t *type;
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;
	if ((queryNode = XmlGetChild(iqNode, "query")) == nullptr) return;

	if (!mir_wstrcmp(type, L"result")) {
		XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultSetAuth, JABBER_IQ_TYPE_SET));
		HXML query = iq << XQUERY(L"jabber:iq:auth");
		query << XCHILD(L"username", m_ThreadInfo->conn.username);
		if (XmlGetChild(queryNode, "digest") != nullptr && m_ThreadInfo->szStreamId) {
			JabberShaStrBuf buf;
			T2Utf str(m_ThreadInfo->conn.password);
			char text[200];
			mir_snprintf(text, "%s%s", m_ThreadInfo->szStreamId, str);
			query << XCHILD(L"digest", _A2T(JabberSha1(text, buf)));
		}
		else if (XmlGetChild(queryNode, "password") != nullptr)
			query << XCHILD(L"password", m_ThreadInfo->conn.password);
		else {
			debugLogA("No known authentication mechanism accepted by the server.");
			m_ThreadInfo->send("</stream:stream>");
			return;
		}

		if (XmlGetChild(queryNode, "resource") != nullptr)
			query << XCHILD(L"resource", m_ThreadInfo->resource);

		m_ThreadInfo->send(iq);
	}
	else if (!mir_wstrcmp(type, L"error")) {
		m_ThreadInfo->send("</stream:stream>");

		wchar_t text[128];
		mir_snwprintf(text, TranslateT("Authentication failed for %s."), m_ThreadInfo->conn.username);
		MsgPopup(0, text, TranslateT("Jabber Authentication"));
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultSetAuth(HXML iqNode, CJabberIqInfo*)
{
	const wchar_t *type;

	// RECVED: authentication result
	// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
	debugLogA("<iq/> iqIdSetAuth");
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;

	if (!mir_wstrcmp(type, L"result")) {
		ptrW tszNick(getWStringA("Nick"));
		if (tszNick == nullptr)
			setWString("Nick", m_ThreadInfo->conn.username);

		OnLoggedIn();
	}
	// What to do if password error? etc...
	else if (!mir_wstrcmp(type, L"error")) {
		wchar_t text[128];

		m_ThreadInfo->send("</stream:stream>");
		mir_snwprintf(text, TranslateT("Authentication failed for %s."), m_ThreadInfo->conn.username);
		MsgPopup(0, text, TranslateT("Jabber Authentication"));
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultBind(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!m_ThreadInfo || !iqNode)
		return;
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		LPCTSTR szJid = XPathT(iqNode, "bind[@xmlns='urn:ietf:params:xml:ns:xmpp-bind']/jid");
		if (szJid) {
			if (!wcsncmp(m_ThreadInfo->fullJID, szJid, _countof(m_ThreadInfo->fullJID)))
				debugLogW(L"Result Bind: %s confirmed ", m_ThreadInfo->fullJID);
			else {
				debugLogW(L"Result Bind: %s changed to %s", m_ThreadInfo->fullJID, szJid);
				wcsncpy_s(m_ThreadInfo->fullJID, szJid, _TRUNCATE);
			}
		}
		if (m_ThreadInfo->bIsSessionAvailable)
			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResultSession, JABBER_IQ_TYPE_SET))
				<< XCHILDNS(L"session", L"urn:ietf:params:xml:ns:xmpp-session"));
		else
			OnLoggedIn();
	}
	else {
		//rfc3920 page 39
		m_ThreadInfo->send("</stream:stream>");
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultSession(HXML, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		OnLoggedIn();
}

void CJabberProto::GroupchatJoinByHContact(MCONTACT hContact, bool autojoin)
{
	ptrW roomjid(getWStringA(hContact, "ChatRoomID"));
	if (roomjid == nullptr)
		return;

	wchar_t *room = roomjid;
	wchar_t *server = wcschr(roomjid, '@');
	if (!server)
		return;

	server[0] = 0; server++;

	ptrW nick(getWStringA(hContact, "MyNick"));
	if (nick == nullptr) {
		nick = JabberNickFromJID(m_szJabberJID);
		if (nick == nullptr)
			return;
	}

	GroupchatJoinRoom(server, room, nick, ptrW(getWStringA(hContact, "Password")), autojoin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetRoster - populates LIST_ROSTER and creates contact for any new rosters

void CJabberProto::OnIqResultGetRoster(HXML iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqIdGetRoster");
	wchar_t *szGroupDelimeter = (wchar_t *)pInfo->GetUserData();
	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT) {
		mir_free(szGroupDelimeter);
		return;
	}

	HXML queryNode = XmlGetChild(iqNode, "query");
	if (queryNode == nullptr) {
		mir_free(szGroupDelimeter);
		return;
	}

	if (mir_wstrcmp(XmlGetAttrValue(queryNode, L"xmlns"), JABBER_FEAT_IQ_ROSTER)) {
		mir_free(szGroupDelimeter);
		return;
	}

	if (!mir_wstrcmp(szGroupDelimeter, L"\\")) {
		mir_free(szGroupDelimeter);
		szGroupDelimeter = nullptr;
	}

	LIST<void> chatRooms(10);
	OBJLIST<JABBER_HTTP_AVATARS> *httpavatars = new OBJLIST<JABBER_HTTP_AVATARS>(20, JABBER_HTTP_AVATARS::compare);

	for (int i = 0;; i++) {
		bool bIsTransport = false;

		HXML itemNode = XmlGetChild(queryNode, i);
		if (!itemNode)
			break;

		if (mir_wstrcmp(XmlGetName(itemNode), L"item"))
			continue;

		const wchar_t *str = XmlGetAttrValue(itemNode, L"subscription");

		JABBER_SUBSCRIPTION sub;
		if (str == nullptr) sub = SUB_NONE;
		else if (!mir_wstrcmp(str, L"both")) sub = SUB_BOTH;
		else if (!mir_wstrcmp(str, L"to")) sub = SUB_TO;
		else if (!mir_wstrcmp(str, L"from")) sub = SUB_FROM;
		else sub = SUB_NONE;

		const wchar_t *jid = XmlGetAttrValue(itemNode, L"jid");
		if (jid == nullptr)
			continue;
		if (wcschr(jid, '@') == nullptr)
			bIsTransport = true;

		const wchar_t *name = XmlGetAttrValue(itemNode, L"name");
		wchar_t *nick = (name != nullptr) ? mir_wstrdup(name) : JabberNickFromJID(jid);
		if (nick == nullptr)
			continue;

		MCONTACT hContact = HContactFromJID(jid);
		if (hContact == 0) // Received roster has a new JID.
			hContact = DBCreateContact(jid, nick, false, false); // Add the jid (with empty resource) to Miranda contact list.

		JABBER_LIST_ITEM *item = ListAdd(LIST_ROSTER, jid, hContact);
		item->subscription = sub;
		item->bRealContact = true;

		mir_free(item->nick); item->nick = nick;

		HXML groupNode = XmlGetChild(itemNode, "group");
		replaceStrW(item->group, XmlGetText(groupNode));

		// check group delimiters
		if (item->group && szGroupDelimeter) {
			while (wchar_t *szPos = wcsstr(item->group, szGroupDelimeter)) {
				*szPos = 0;
				szPos += mir_wstrlen(szGroupDelimeter);
				wchar_t *szNewGroup = (wchar_t *)mir_alloc(sizeof(wchar_t) * (mir_wstrlen(item->group) + mir_wstrlen(szPos) + 2));
				mir_wstrcpy(szNewGroup, item->group);
				mir_wstrcat(szNewGroup, L"\\");
				mir_wstrcat(szNewGroup, szPos);
				mir_free(item->group);
				item->group = szNewGroup;
			}
		}

		if (name != nullptr) {
			ptrW tszNick(getWStringA(hContact, "Nick"));
			if (tszNick != nullptr) {
				if (mir_wstrcmp(nick, tszNick) != 0)
					db_set_ws(hContact, "CList", "MyHandle", nick);
				else
					db_unset(hContact, "CList", "MyHandle");
			}
			else db_set_ws(hContact, "CList", "MyHandle", nick);
		}
		else db_unset(hContact, "CList", "MyHandle");

		if (isChatRoom(hContact)) {
			wchar_t *wszTitle = NEWWSTR_ALLOCA(jid);
			if (wchar_t *p = wcschr(wszTitle, '@')) *p = 0;
			Chat_NewSession(GCW_CHATROOM, m_szModuleName, jid, wszTitle);

			db_unset(hContact, "CList", "Hidden");
			chatRooms.insert((HANDLE)hContact);
		}
		else UpdateSubscriptionInfo(hContact, item);

		if (!m_options.IgnoreRosterGroups) {
			if (item->group != nullptr) {
				Clist_GroupCreate(0, item->group);

				// Don't set group again if already correct, or Miranda may show wrong group count in some case
				ptrW tszGroup(db_get_wsa(hContact, "CList", "Group"));
				if (tszGroup != nullptr) {
					if (mir_wstrcmp(tszGroup, item->group))
						db_set_ws(hContact, "CList", "Group", item->group);
				}
				else db_set_ws(hContact, "CList", "Group", item->group);
			}
			else db_unset(hContact, "CList", "Group");
		}

		if (hContact != 0) {
			if (bIsTransport)
				setByte(hContact, "IsTransport", true);
			else
				setByte(hContact, "IsTransport", false);
		}

		const wchar_t *imagepath = XmlGetAttrValue(itemNode, L"vz:img");
		if (imagepath)
			httpavatars->insert(new JABBER_HTTP_AVATARS(imagepath, hContact));
	}

	if (httpavatars->getCount())
		ForkThread(&CJabberProto::LoadHttpAvatars, httpavatars);
	else
		delete httpavatars;

	// Delete orphaned contacts (if roster sync is enabled)
	if (m_options.RosterSync) {
		LISTFOREACH(i, this, LIST_ROSTER)
		{
			JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
			if (item && item->hContact && !item->bRealContact) {
				debugLogW(L"Syncing roster: preparing to delete %s (hContact=0x%x)", item->jid, item->hContact);
				db_delete_contact(item->hContact);
			}
		}
	}

	EnableMenuItems(true);

	debugLogA("Status changed via THREADSTART");
	SetServerStatus(m_iDesiredStatus);

	if (m_options.AutoJoinConferences)
		for (int i = 0; i < chatRooms.getCount(); i++)
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
	const wchar_t *type;
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;
	if ((queryNode = XmlGetChild(iqNode, "query")) == nullptr) return;

	if (!mir_wstrcmp(type, L"result")) {
		if (m_hwndAgentRegInput)
			SendMessage(m_hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 1 /*success*/, (LPARAM)xmlCopyNode(iqNode));
	}
	else if (!mir_wstrcmp(type, L"error")) {
		if (m_hwndAgentRegInput) {
			HXML errorNode = XmlGetChild(iqNode, "error");
			wchar_t *str = JabberErrorMsg(errorNode);
			SendMessage(m_hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 0 /*error*/, (LPARAM)str);
			mir_free(str);
		}
	}
}

void CJabberProto::OnIqResultSetRegister(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: result of registration process
	// ACTION: notify of successful agent registration
	debugLogA("<iq/> iqIdSetRegister");

	const wchar_t *type, *from;
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;
	if ((from = XmlGetAttrValue(iqNode, L"from")) == nullptr) return;

	if (!mir_wstrcmp(type, L"result")) {
		MCONTACT hContact = HContactFromJID(from);
		if (hContact != 0)
			setByte(hContact, "IsTransport", true);

		if (m_hwndRegProgress)
			SendMessage(m_hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Registration successful"));
	}
	else if (!mir_wstrcmp(type, L"error")) {
		if (m_hwndRegProgress) {
			HXML errorNode = XmlGetChild(iqNode, "error");
			wchar_t *str = JabberErrorMsg(errorNode);
			SendMessage(m_hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)str);
			mir_free(str);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetVcard - processes the server-side v-card

void CJabberProto::OnIqResultGetVcardPhoto(HXML n, MCONTACT hContact, bool &hasPhoto)
{
	debugLogA("JabberIqResultGetVcardPhoto: %d", hasPhoto);
	if (hasPhoto)
		return;

	HXML o = XmlGetChild(n, "BINVAL");
	LPCTSTR ptszBinval = XmlGetText(o);
	if (o == nullptr || ptszBinval == nullptr)
		return;

	unsigned bufferLen;
	ptrA buffer((char*)mir_base64_decode(_T2A(ptszBinval), &bufferLen));
	if (buffer == nullptr)
		return;

	const wchar_t *szPicType = JabberGetPictureType(n, buffer);
	if (szPicType == nullptr)
		return;

	wchar_t szAvatarFileName[MAX_PATH];
	GetAvatarFileName(hContact, szAvatarFileName, _countof(szAvatarFileName));

	debugLogW(L"Picture file name set to %s", szAvatarFileName);
	HANDLE hFile = CreateFile(szAvatarFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	debugLogA("Writing %d bytes", bufferLen);
	DWORD nWritten;
	bool bRes = WriteFile(hFile, buffer, bufferLen, &nWritten, nullptr) != 0;
	CloseHandle(hFile);
	if (!bRes)
		return;

	debugLogA("%d bytes written", nWritten);
	if (hContact == 0) {
		hasPhoto = true;
		CallService(MS_AV_SETMYAVATARW, (WPARAM)m_szModuleName, (LPARAM)szAvatarFileName);

		debugLogW(L"My picture saved to %s", szAvatarFileName);
	}
	else {
		ptrW jid(getWStringA(hContact, "jid"));
		if (jid != nullptr) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
			if (item == nullptr) {
				item = ListAdd(LIST_VCARD_TEMP, jid); // adding to the temp list to store information about photo
				if (item != nullptr)
					item->bUseResource = true;
			}
			if (item != nullptr) {
				hasPhoto = true;
				if (item->photoFileName && mir_wstrcmp(item->photoFileName, szAvatarFileName))
					DeleteFile(item->photoFileName);
				replaceStrW(item->photoFileName, szAvatarFileName);
				debugLogW(L"Contact's picture saved to %s", szAvatarFileName);
				OnIqResultGotAvatar(hContact, o, szPicType);
			}
		}
	}

	if (!hasPhoto)
		DeleteFile(szAvatarFileName);
}

static wchar_t* sttGetText(HXML node, char* tag)
{
	HXML n = XmlGetChild(node, tag);
	if (n == nullptr)
		return nullptr;

	return (wchar_t*)XmlGetText(n);
}

void CJabberProto::OnIqResultGetVcard(HXML iqNode, CJabberIqInfo*)
{
	HXML vCardNode, m, n, o;
	const wchar_t *type, *jid;
	MCONTACT hContact;
	DBVARIANT dbv;

	debugLogA("<iq/> iqIdGetVcard");
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;
	if ((jid = XmlGetAttrValue(iqNode, L"from")) == nullptr) return;
	int id = JabberGetPacketID(iqNode);

	if (id == m_nJabberSearchID) {
		m_nJabberSearchID = -1;

		if ((vCardNode = XmlGetChild(iqNode, "vCard")) != nullptr) {
			if (!mir_wstrcmp(type, L"result")) {
				PROTOSEARCHRESULT  psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.nick.w = sttGetText(vCardNode, "NICKNAME");
				psr.firstName.w = sttGetText(vCardNode, "FN");
				psr.lastName.w = L"";
				psr.email.w = sttGetText(vCardNode, "EMAIL");
				psr.id.w = NEWWSTR_ALLOCA(jid);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
			}
			else if (!mir_wstrcmp(type, L"error"))
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		}
		else ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		return;
	}

	size_t len = mir_wstrlen(m_szJabberJID);
	if (!wcsnicmp(jid, m_szJabberJID, len) && (jid[len] == '/' || jid[len] == '\0')) {
		hContact = 0;
		debugLogA("Vcard for myself");
	}
	else {
		if ((hContact = HContactFromJID(jid)) == 0)
			return;
		debugLogA("Other user's vcard");
	}

	if (!mir_wstrcmp(type, L"error")) {
		if ((hContact = HContactFromJID(jid)) != 0)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
		return;
	}

	if (mir_wstrcmp(type, L"result"))
		return;

	bool hasFn = false, hasNick = false, hasGiven = false, hasFamily = false, hasMiddle = false,
		hasBday = false, hasGender = false, hasPhone = false, hasFax = false, hasCell = false, hasUrl = false,
		hasHome = false, hasHomeStreet = false, hasHomeStreet2 = false, hasHomeLocality = false,
		hasHomeRegion = false, hasHomePcode = false, hasHomeCtry = false,
		hasWork = false, hasWorkStreet = false, hasWorkStreet2 = false, hasWorkLocality = false,
		hasWorkRegion = false, hasWorkPcode = false, hasWorkCtry = false,
		hasOrgname = false, hasOrgunit = false, hasRole = false, hasTitle = false, hasDesc = false, hasPhoto = false;
	int nEmail = 0, nPhone = 0, nYear, nMonth, nDay;

	if ((vCardNode = XmlGetChild(iqNode, "vCard")) != nullptr) {
		for (int i = 0;; i++) {
			n = XmlGetChild(vCardNode, i);
			if (!n)
				break;
			if (XmlGetName(n) == nullptr) continue;
			if (!mir_wstrcmp(XmlGetName(n), L"FN")) {
				if (XmlGetText(n) != nullptr) {
					hasFn = true;
					setWString(hContact, "FullName", XmlGetText(n));
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"NICKNAME")) {
				if (XmlGetText(n) != nullptr) {
					hasNick = true;
					setWString(hContact, "Nick", XmlGetText(n));
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"N")) {
				// First/Last name
				if (!hasGiven && !hasFamily && !hasMiddle) {
					if ((m = XmlGetChild(n, "GIVEN")) != nullptr && XmlGetText(m) != nullptr) {
						hasGiven = true;
						setWString(hContact, "FirstName", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "FAMILY")) != nullptr && XmlGetText(m) != nullptr) {
						hasFamily = true;
						setWString(hContact, "LastName", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "MIDDLE")) != nullptr && XmlGetText(m) != nullptr) {
						hasMiddle = true;
						setWString(hContact, "MiddleName", XmlGetText(m));
					}
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"EMAIL")) {
				// E-mail address(es)
				if ((m = XmlGetChild(n, "USERID")) == nullptr)	// Some bad client put e-mail directly in <EMAIL/> instead of <USERID/>
					m = n;
				if (XmlGetText(m) != nullptr) {
					char text[100];
					if (hContact != 0) {
						if (nEmail == 0)
							mir_strcpy(text, "e-mail");
						else
							mir_snprintf(text, "e-mail%d", nEmail - 1);
					}
					else mir_snprintf(text, "e-mail%d", nEmail);
					setWString(hContact, text, XmlGetText(m));

					if (hContact == 0) {
						mir_snprintf(text, "e-mailFlag%d", nEmail);
						int nFlag = 0;
						if (XmlGetChild(n, "HOME") != nullptr) nFlag |= JABBER_VCEMAIL_HOME;
						if (XmlGetChild(n, "WORK") != nullptr) nFlag |= JABBER_VCEMAIL_WORK;
						if (XmlGetChild(n, "INTERNET") != nullptr) nFlag |= JABBER_VCEMAIL_INTERNET;
						if (XmlGetChild(n, "X400") != nullptr) nFlag |= JABBER_VCEMAIL_X400;
						setWord(text, nFlag);
					}
					nEmail++;
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"BDAY")) {
				// Birthday
				if (!hasBday && XmlGetText(n) != nullptr) {
					if (hContact != 0) {
						if (swscanf(XmlGetText(n), L"%d-%d-%d", &nYear, &nMonth, &nDay) == 3) {
							hasBday = true;
							setWord(hContact, "BirthYear", (WORD)nYear);
							setByte(hContact, "BirthMonth", (BYTE)nMonth);
							setByte(hContact, "BirthDay", (BYTE)nDay);

							SYSTEMTIME sToday = { 0 };
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
						setWString("BirthDate", XmlGetText(n));
					}
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"GENDER")) {
				// Gender
				if (!hasGender && XmlGetText(n) != nullptr) {
					if (hContact != 0) {
						if (XmlGetText(n)[0] && strchr("mMfF", XmlGetText(n)[0]) != nullptr) {
							hasGender = true;
							setByte(hContact, "Gender", (BYTE)toupper(XmlGetText(n)[0]));
						}
					}
					else {
						hasGender = true;
						setWString("GenderString", XmlGetText(n));
					}
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"ADR")) {
				if (!hasHome && XmlGetChild(n, "HOME") != nullptr) {
					// Home address
					wchar_t text[128];
					hasHome = true;
					if ((m = XmlGetChild(n, "STREET")) != nullptr && XmlGetText(m) != nullptr) {
						hasHomeStreet = true;
						if (hContact != 0) {
							if ((o = XmlGetChild(n, "EXTADR")) != nullptr && XmlGetText(o) != nullptr)
								mir_snwprintf(text, L"%s\r\n%s", XmlGetText(m), XmlGetText(o));
							else if ((o = XmlGetChild(n, "EXTADD")) != nullptr && XmlGetText(o) != nullptr)
								mir_snwprintf(text, L"%s\r\n%s", XmlGetText(m), XmlGetText(o));
							else
								wcsncpy_s(text, XmlGetText(m), _TRUNCATE);
							text[_countof(text) - 1] = '\0';
							setWString(hContact, "Street", text);
						}
						else {
							setWString(hContact, "Street", XmlGetText(m));
							if ((m = XmlGetChild(n, "EXTADR")) == nullptr)
								m = XmlGetChild(n, "EXTADD");
							if (m != nullptr && XmlGetText(m) != nullptr) {
								hasHomeStreet2 = true;
								setWString(hContact, "Street2", XmlGetText(m));
							}
						}
					}

					if ((m = XmlGetChild(n, "LOCALITY")) != nullptr && XmlGetText(m) != nullptr) {
						hasHomeLocality = true;
						setWString(hContact, "City", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "REGION")) != nullptr && XmlGetText(m) != nullptr) {
						hasHomeRegion = true;
						setWString(hContact, "State", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "PCODE")) != nullptr && XmlGetText(m) != nullptr) {
						hasHomePcode = true;
						setWString(hContact, "ZIP", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "CTRY")) == nullptr || XmlGetText(m) == nullptr)	// Some bad client use <COUNTRY/> instead of <CTRY/>
						m = XmlGetChild(n, "COUNTRY");
					if (m != nullptr && XmlGetText(m) != nullptr) {
						hasHomeCtry = true;
						setWString(hContact, "Country", XmlGetText(m));
					}
				}

				if (!hasWork && XmlGetChild(n, "WORK") != nullptr) {
					// Work address
					hasWork = true;
					if ((m = XmlGetChild(n, "STREET")) != nullptr && XmlGetText(m) != nullptr) {
						wchar_t text[128];
						hasWorkStreet = true;
						if (hContact != 0) {
							if ((o = XmlGetChild(n, "EXTADR")) != nullptr && XmlGetText(o) != nullptr)
								mir_snwprintf(text, L"%s\r\n%s", XmlGetText(m), XmlGetText(o));
							else if ((o = XmlGetChild(n, "EXTADD")) != nullptr && XmlGetText(o) != nullptr)
								mir_snwprintf(text, L"%s\r\n%s", XmlGetText(m), XmlGetText(o));
							else
								wcsncpy_s(text, XmlGetText(m), _TRUNCATE);
							text[_countof(text) - 1] = '\0';
							setWString(hContact, "CompanyStreet", text);
						}
						else {
							setWString(hContact, "CompanyStreet", XmlGetText(m));
							if ((m = XmlGetChild(n, "EXTADR")) == nullptr)
								m = XmlGetChild(n, "EXTADD");
							if (m != nullptr && XmlGetText(m) != nullptr) {
								hasWorkStreet2 = true;
								setWString(hContact, "CompanyStreet2", XmlGetText(m));
							}
						}
					}

					if ((m = XmlGetChild(n, "LOCALITY")) != nullptr && XmlGetText(m) != nullptr) {
						hasWorkLocality = true;
						setWString(hContact, "CompanyCity", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "REGION")) != nullptr && XmlGetText(m) != nullptr) {
						hasWorkRegion = true;
						setWString(hContact, "CompanyState", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "PCODE")) != nullptr && XmlGetText(m) != nullptr) {
						hasWorkPcode = true;
						setWString(hContact, "CompanyZIP", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "CTRY")) == nullptr || XmlGetText(m) == nullptr)	// Some bad client use <COUNTRY/> instead of <CTRY/>
						m = XmlGetChild(n, "COUNTRY");
					if (m != nullptr && XmlGetText(m) != nullptr) {
						hasWorkCtry = true;
						setWString(hContact, "CompanyCountry", XmlGetText(m));
					}
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"TEL")) {
				// Telephone/Fax/Cellular
				if ((m = XmlGetChild(n, "NUMBER")) != nullptr && XmlGetText(m) != nullptr) {
					if (hContact != 0) {
						if (!hasFax && XmlGetChild(n, "FAX") != nullptr) {
							hasFax = true;
							setWString(hContact, "Fax", XmlGetText(m));
						}
						else if (!hasCell && XmlGetChild(n, "CELL") != nullptr) {
							hasCell = true;
							setWString(hContact, "Cellular", XmlGetText(m));
						}
						else if (!hasPhone &&
							(XmlGetChild(n, "HOME") != nullptr || XmlGetChild(n, "WORK") != nullptr || XmlGetChild(n, "VOICE") != nullptr ||
							(XmlGetChild(n, "FAX") == nullptr &&
							 XmlGetChild(n, "PAGER") == nullptr &&
							 XmlGetChild(n, "MSG") == nullptr &&
							 XmlGetChild(n, "CELL") == nullptr &&
							 XmlGetChild(n, "VIDEO") == nullptr &&
							 XmlGetChild(n, "BBS") == nullptr &&
							 XmlGetChild(n, "MODEM") == nullptr &&
							 XmlGetChild(n, "ISDN") == nullptr &&
							 XmlGetChild(n, "PCS") == nullptr)))
						{
							hasPhone = true;
							setWString(hContact, "Phone", XmlGetText(m));
						}
					}
					else {
						char text[100];
						mir_snprintf(text, "Phone%d", nPhone);
						setWString(text, XmlGetText(m));

						mir_snprintf(text, "PhoneFlag%d", nPhone);
						int nFlag = 0;
						if (XmlGetChild(n, "HOME")  != nullptr) nFlag |= JABBER_VCTEL_HOME;
						if (XmlGetChild(n, "WORK")  != nullptr) nFlag |= JABBER_VCTEL_WORK;
						if (XmlGetChild(n, "VOICE") != nullptr) nFlag |= JABBER_VCTEL_VOICE;
						if (XmlGetChild(n, "FAX")   != nullptr) nFlag |= JABBER_VCTEL_FAX;
						if (XmlGetChild(n, "PAGER") != nullptr) nFlag |= JABBER_VCTEL_PAGER;
						if (XmlGetChild(n, "MSG")   != nullptr) nFlag |= JABBER_VCTEL_MSG;
						if (XmlGetChild(n, "CELL")  != nullptr) nFlag |= JABBER_VCTEL_CELL;
						if (XmlGetChild(n, "VIDEO") != nullptr) nFlag |= JABBER_VCTEL_VIDEO;
						if (XmlGetChild(n, "BBS")   != nullptr) nFlag |= JABBER_VCTEL_BBS;
						if (XmlGetChild(n, "MODEM") != nullptr) nFlag |= JABBER_VCTEL_MODEM;
						if (XmlGetChild(n, "ISDN")  != nullptr) nFlag |= JABBER_VCTEL_ISDN;
						if (XmlGetChild(n, "PCS")   != nullptr) nFlag |= JABBER_VCTEL_PCS;
						setWord(text, nFlag);
						nPhone++;
					}
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"URL")) {
				// Homepage
				if (!hasUrl && XmlGetText(n) != nullptr) {
					hasUrl = true;
					setWString(hContact, "Homepage", XmlGetText(n));
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"ORG")) {
				if (!hasOrgname && !hasOrgunit) {
					if ((m = XmlGetChild(n, "ORGNAME")) != nullptr && XmlGetText(m) != nullptr) {
						hasOrgname = true;
						setWString(hContact, "Company", XmlGetText(m));
					}
					if ((m = XmlGetChild(n, "ORGUNIT")) != nullptr && XmlGetText(m) != nullptr) {	// The real vCard can have multiple <ORGUNIT/> but we will only display the first one
						hasOrgunit = true;
						setWString(hContact, "CompanyDepartment", XmlGetText(m));
					}
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"ROLE")) {
				if (!hasRole && XmlGetText(n) != nullptr) {
					hasRole = true;
					setWString(hContact, "Role", XmlGetText(n));
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"TITLE")) {
				if (!hasTitle && XmlGetText(n) != nullptr) {
					hasTitle = true;
					setWString(hContact, "CompanyPosition", XmlGetText(n));
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"DESC")) {
				if (!hasDesc && XmlGetText(n) != nullptr) {
					hasDesc = true;
					CMStringW tszMemo(XmlGetText(n));
					tszMemo.Replace(L"\n", L"\r\n");
					setWString(hContact, "About", tszMemo);
				}
			}
			else if (!mir_wstrcmp(XmlGetName(n), L"PHOTO"))
				OnIqResultGetVcardPhoto(n, hContact, hasPhoto);
		}
	}

	if (hasFn && !hasNick) {
		ptrW nick(getWStringA(hContact, "Nick"));
		ptrW jidNick(JabberNickFromJID(jid));
		if (!nick || (jidNick && !mir_wstrcmpi(nick, jidNick)))
			setWString(hContact, "Nick", ptrW(getWStringA(hContact, "FullName")));
	}
	if (!hasFn)
		delSetting(hContact, "FullName");
	if (!hasGiven)
		delSetting(hContact, "FirstName");
	if (!hasFamily)
		delSetting(hContact, "LastName");
	if (!hasMiddle)
		delSetting(hContact, "MiddleName");
	if (hContact != 0) {
		while (true) {
			if (nEmail <= 0)
				delSetting(hContact, "e-mail");
			else {
				char text[100];
				mir_snprintf(text, "e-mail%d", nEmail - 1);
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
			mir_snprintf(text, "e-mail%d", nEmail);
			if (getString(text, &dbv)) break;
			db_free(&dbv);
			delSetting(text);
			mir_snprintf(text, "e-mailFlag%d", nEmail);
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
		if (hContact != 0)
			delSetting(hContact, "Gender");
		else
			delSetting("GenderString");
	}
	if (hContact != 0) {
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
			mir_snprintf(text, "Phone%d", nPhone);
			if (getString(text, &dbv)) break;
			db_free(&dbv);
			delSetting(text);
			mir_snprintf(text, "PhoneFlag%d", nPhone);
			delSetting(text);
			nPhone++;
		}
	}

	if (!hasHomeStreet)
		delSetting(hContact, "Street");
	if (!hasHomeStreet2 && hContact == 0)
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
	if (!hasWorkStreet2 && hContact == 0)
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
	if (!hasPhoto) {
		debugLogA("Has no avatar");
		delSetting(hContact, "AvatarHash");

		if (ptrW(getWStringA(hContact, "AvatarSaved")) != nullptr) {
			delSetting(hContact, "AvatarSaved");
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr, 0);
		}
	}
	
	if (id == m_ThreadInfo->resolveID) {
		const wchar_t *p = wcschr(jid, '@');
		ResolveTransportNicks((p != nullptr) ? p + 1 : jid);
	}
	else {
		if ((hContact = HContactFromJID(jid)) != 0)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
	}
}

void CJabberProto::OnIqResultSetVcard(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetVcard");
	if (XmlGetAttrValue(iqNode, L"type"))
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
}

void CJabberProto::OnIqResultSetSearch(HXML iqNode, CJabberIqInfo*)
{
	HXML queryNode, n;
	const wchar_t *type, *jid;
	int id;

	debugLogA("<iq/> iqIdGetSearch");
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;
	if ((id = JabberGetPacketID(iqNode)) == -1) return;

	if (!mir_wstrcmp(type, L"result")) {
		if ((queryNode = XmlGetChild(iqNode, "query")) == nullptr)
			return;

		PROTOSEARCHRESULT psr = { 0 };
		psr.cbSize = sizeof(psr);
		for (int i = 0;; i++) {
			HXML itemNode = XmlGetChild(queryNode, i);
			if (!itemNode)
				break;

			if (!mir_wstrcmp(XmlGetName(itemNode), L"item")) {
				if ((jid = XmlGetAttrValue(itemNode, L"jid")) != nullptr) {
					psr.id.w = (wchar_t*)jid;
					debugLogW(L"Result jid = %s", jid);
					if ((n = XmlGetChild(itemNode, "nick")) != nullptr && XmlGetText(n) != nullptr)
						psr.nick.w = (wchar_t*)XmlGetText(n);
					else
						psr.nick.w = L"";
					if ((n = XmlGetChild(itemNode, "first")) != nullptr && XmlGetText(n) != nullptr)
						psr.firstName.w = (wchar_t*)XmlGetText(n);
					else
						psr.firstName.w = L"";
					if ((n = XmlGetChild(itemNode, "last")) != nullptr && XmlGetText(n) != nullptr)
						psr.lastName.w = (wchar_t*)XmlGetText(n);
					else
						psr.lastName.w = L"";
					if ((n = XmlGetChild(itemNode, "email")) != nullptr && XmlGetText(n) != nullptr)
						psr.email.w = (wchar_t*)XmlGetText(n);
					else
						psr.email.w = L"";
					psr.flags = PSR_UNICODE;
					ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
				}
			}
		}

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
	else if (!mir_wstrcmp(type, L"error"))
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

void CJabberProto::OnIqResultExtSearch(HXML iqNode, CJabberIqInfo*)
{
	HXML queryNode;

	debugLogA("<iq/> iqIdGetExtSearch");
	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;

	int id = JabberGetPacketID(iqNode);
	if (id == -1)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		if ((queryNode = XmlGetChild(iqNode, "query")) == nullptr) return;
		if ((queryNode = XmlGetChild(queryNode, "x")) == nullptr) return;
		for (int i = 0;; i++) {
			HXML itemNode = XmlGetChild(queryNode, i);
			if (!itemNode)
				break;
			if (mir_wstrcmp(XmlGetName(itemNode), L"item"))
				continue;

			PROTOSEARCHRESULT  psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;

			for (int j = 0;; j++) {
				HXML fieldNode = XmlGetChild(itemNode, j);
				if (!fieldNode)
					break;

				if (mir_wstrcmp(XmlGetName(fieldNode), L"field"))
					continue;

				const wchar_t *fieldName = XmlGetAttrValue(fieldNode, L"var");
				if (fieldName == nullptr)
					continue;

				HXML n = XmlGetChild(fieldNode, "value");
				if (n == nullptr)
					continue;

				if (!mir_wstrcmp(fieldName, L"jid")) {
					psr.id.w = (wchar_t*)XmlGetText(n);
					debugLogW(L"Result jid = %s", psr.id.w);
				}
				else if (!mir_wstrcmp(fieldName, L"nickname"))
					psr.nick.w = (XmlGetText(n) != nullptr) ? (wchar_t*)XmlGetText(n) : L"";
				else if (!mir_wstrcmp(fieldName, L"fn"))
					psr.firstName.w = (XmlGetText(n) != nullptr) ? (wchar_t*)XmlGetText(n) : L"";
				else if (!mir_wstrcmp(fieldName, L"given"))
					psr.firstName.w = (XmlGetText(n) != nullptr) ? (wchar_t*)XmlGetText(n) : L"";
				else if (!mir_wstrcmp(fieldName, L"family"))
					psr.lastName.w = (XmlGetText(n) != nullptr) ? (wchar_t*)XmlGetText(n) : L"";
				else if (!mir_wstrcmp(fieldName, L"email"))
					psr.email.w = (XmlGetText(n) != nullptr) ? (wchar_t*)XmlGetText(n) : L"";
			}

			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
		}

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
	else if (!mir_wstrcmp(type, L"error"))
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

void CJabberProto::OnIqResultSetPassword(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetPassword");

	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		wcsncpy_s(m_ThreadInfo->conn.password, m_ThreadInfo->tszNewPassword, _TRUNCATE);
		MessageBox(nullptr, TranslateT("Password is successfully changed. Don't forget to update your password in the Jabber protocol option."), TranslateT("Change Password"), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
	}
	else if (!mir_wstrcmp(type, L"error"))
		MessageBox(nullptr, TranslateT("Password cannot be changed."), TranslateT("Change Password"), MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
}

void CJabberProto::OnIqResultGetVCardAvatar(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> OnIqResultGetVCardAvatar");

	const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;

	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	const wchar_t *type;
	if ((type = XmlGetAttrValue(iqNode, L"type")) == nullptr) return;
	if (mir_wstrcmp(type, L"result")) return;

	HXML vCard = XmlGetChild(iqNode, "vCard");
	if (vCard == nullptr) return;
	vCard = XmlGetChild(vCard, "PHOTO");
	if (vCard == nullptr) return;

	if (XmlGetChildCount(vCard) == 0) {
		delSetting(hContact, "AvatarHash");
		if (ptrW(getWStringA(hContact, "AvatarSaved")) != nullptr) {
			delSetting(hContact, "AvatarSaved");
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr, 0);
		}
		return;
	}

	const wchar_t *mimeType = XmlGetText(XmlGetChild(vCard, "TYPE"));
	HXML n = XmlGetChild(vCard, "BINVAL");
	if (n == nullptr)
		return;

	setByte(hContact, "AvatarXVcard", 1);
	OnIqResultGotAvatar(hContact, n, mimeType);
}

void CJabberProto::OnIqResultGetClientAvatar(HXML iqNode, CJabberIqInfo*)
{
	const wchar_t *type;

	debugLogA("<iq/> iqIdResultGetClientAvatar");

	const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;
	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	HXML n = nullptr;
	if ((type = XmlGetAttrValue(iqNode, L"type")) != nullptr && !mir_wstrcmp(type, L"result")) {
		HXML queryNode = XmlGetChild(iqNode, "query");
		if (queryNode != nullptr) {
			const wchar_t *xmlns = XmlGetAttrValue(queryNode, L"xmlns");
			if (!mir_wstrcmp(xmlns, JABBER_FEAT_AVATAR))
				n = XmlGetChild(queryNode, "data");
		}
	}

	if (n != nullptr) {
		OnIqResultGotAvatar(hContact, n, XmlGetAttrValue(n, L"mimetype"));
		return;
	}

	wchar_t szJid[JABBER_MAX_JID_LEN];
	mir_wstrncpy(szJid, from, _countof(szJid));
	wchar_t *res = wcschr(szJid, '/');
	if (res != nullptr)
		*res = 0;

	// Try server stored avatar
	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultGetServerAvatar, JABBER_IQ_TYPE_GET, szJid));
	iq << XQUERY(JABBER_FEAT_SERVER_AVATAR);
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnIqResultGetServerAvatar(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdResultGetServerAvatar");

	const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;

	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	HXML n = nullptr;
	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (!mir_wstrcmp(type, L"result")) {
		HXML queryNode = XmlGetChild(iqNode, "query");
		if (queryNode != nullptr) {
			const wchar_t *xmlns = XmlGetAttrValue(queryNode, L"xmlns");
			if (!mir_wstrcmp(xmlns, JABBER_FEAT_SERVER_AVATAR))
				n = XmlGetChild(queryNode, "data");
		}
	}

	if (n != nullptr) {
		OnIqResultGotAvatar(hContact, n, XmlGetAttrValue(n, L"mimetype"));
		return;
	}

	wchar_t szJid[JABBER_MAX_JID_LEN];
	mir_wstrncpy(szJid, from, _countof(szJid));
	wchar_t *res = wcschr(szJid, '/');
	if (res != nullptr)
		*res = 0;

	// Try VCard photo
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetVCardAvatar, JABBER_IQ_TYPE_GET, szJid)) << XCHILDNS(L"vCard", JABBER_FEAT_VCARD_TEMP));
}


void CJabberProto::OnIqResultGotAvatar(MCONTACT hContact, HXML n, const wchar_t *mimeType)
{
	unsigned resultLen;
	ptrA body((char*)mir_base64_decode(_T2A(XmlGetText(n)), &resultLen));
	if (body == nullptr)
		return;

	int pictureType;
	if (mimeType != nullptr)
		pictureType = ProtoGetAvatarFormatByMimeType(mimeType);
	else
		pictureType = ProtoGetBufferFormat(body, 0);
	if (pictureType == PA_FORMAT_UNKNOWN) {
		debugLogW(L"Invalid mime type specified for picture: %s", mimeType);
		return;
	}

	PROTO_AVATAR_INFORMATION ai;
	ai.format = pictureType;
	ai.hContact = hContact;

	wchar_t tszFileName[MAX_PATH];
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
	wcsncpy_s(ai.filename, tszFileName, _TRUNCATE);

	FILE *out = _wfopen(tszFileName, L"wb");
	if (out != nullptr) {
		fwrite(body, resultLen, 1, out);
		fclose(out);

		char buffer[41];
		setString(hContact, "AvatarSaved", bin2hex(digest, sizeof(digest), buffer));
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai), 0);
		debugLogW(L"Broadcast new avatar: %s", ai.filename);
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks

void CJabberProto::OnIqResultDiscoBookmarks(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: list of bookmarks
	// ACTION: refresh bookmarks dialog
	debugLogA("<iq/> iqIdGetBookmarks");
	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;

	const wchar_t *jid;
	if (!mir_wstrcmp(type, L"result")) {
		if (m_ThreadInfo && !(m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)) {
			m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(true);
		}

		if (HXML storageNode = XPathT(iqNode, "query/storage[@xmlns='storage:bookmarks']")) {
			ListRemoveList(LIST_BOOKMARK);

			HXML itemNode;
			for (int i = 0; itemNode = XmlGetChild(storageNode, i); i++) {
				if (LPCTSTR name = XmlGetName(itemNode)) {
					if (!mir_wstrcmp(name, L"conference") && (jid = XmlGetAttrValue(itemNode, L"jid"))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->name = mir_wstrdup(XmlGetAttrValue(itemNode, L"name"));
						item->type = mir_wstrdup(L"conference");
						item->bUseResource = true;
						item->nick = mir_wstrdup(XPathT(itemNode, "nick"));
						item->password = mir_wstrdup(XPathT(itemNode, "password"));

						const wchar_t *autoJ = XmlGetAttrValue(itemNode, L"autojoin");
						if (autoJ != nullptr)
							item->bAutoJoin = !mir_wstrcmp(autoJ, L"true") || !mir_wstrcmp(autoJ, L"1");
					}
					else if (!mir_wstrcmp(name, L"url") && (jid = XmlGetAttrValue(itemNode, L"url"))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->bUseResource = true;
						item->name = mir_wstrdup(XmlGetAttrValue(itemNode, L"name"));
						item->type = mir_wstrdup(L"url");
					}
				}
			}

			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_REFRESH);
			m_ThreadInfo->bBookmarksLoaded = true;
			OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_BOOKMARKS);
		}
	}
	else if (!mir_wstrcmp(type, L"error")) {
		if (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE) {
			m_ThreadInfo->jabberServerCaps &= ~JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(true);
			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_ACTIVATE);
		}
	}
}

void CJabberProto::SetBookmarkRequest(XmlNodeIq& iq)
{
	HXML query = iq << XQUERY(JABBER_FEAT_PRIVATE_STORAGE);
	HXML storage = query << XCHILDNS(L"storage", L"storage:bookmarks");

	LISTFOREACH(i, this, LIST_BOOKMARK)
	{
		JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
		if (item == nullptr || item->jid == nullptr)
			continue;

		if (!mir_wstrcmp(item->type, L"conference")) {
			HXML itemNode = storage << XCHILD(L"conference") << XATTR(L"jid", item->jid);
			if (item->name)
				itemNode << XATTR(L"name", item->name);
			if (item->bAutoJoin)
				itemNode << XATTRI(L"autojoin", 1);
			if (item->nick)
				itemNode << XCHILD(L"nick", item->nick);
			if (item->password)
				itemNode << XCHILD(L"password", item->password);
		}

		if (!mir_wstrcmp(item->type, L"url")) {
			HXML itemNode = storage << XCHILD(L"url") << XATTR(L"url", item->jid);
			if (item->name)
				itemNode << XATTR(L"name", item->name);
		}
	}
}

void CJabberProto::OnIqResultSetBookmarks(HXML iqNode, CJabberIqInfo*)
{
	// RECVED: server's response
	// ACTION: refresh bookmarks list dialog

	debugLogA("<iq/> iqIdSetBookmarks");

	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_REFRESH);
	}
	else if (!mir_wstrcmp(type, L"error")) {
		HXML errorNode = XmlGetChild(iqNode, "error");
		wchar_t *str = JabberErrorMsg(errorNode);
		MessageBox(nullptr, str, TranslateT("Jabber Bookmarks Error"), MB_OK | MB_SETFOREGROUND);
		mir_free(str);
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_ACTIVATE);
	}
}

// last activity (XEP-0012) support
void CJabberProto::OnIqResultLastActivity(HXML iqNode, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->m_szFrom));
	if (r == nullptr)
		return;

	time_t lastActivity = -1;
	if (pInfo->m_nIqType == JABBER_IQ_TYPE_RESULT) {
		LPCTSTR szSeconds = XPathT(iqNode, "query[@xmlns='jabber:iq:last']/@seconds");
		if (szSeconds) {
			int nSeconds = _wtoi(szSeconds);
			if (nSeconds > 0)
				lastActivity = time(0) - nSeconds;
		}

		LPCTSTR szLastStatusMessage = XPathT(iqNode, "query[@xmlns='jabber:iq:last']");
		if (szLastStatusMessage) // replace only if it exists
			r->m_tszStatusMessage = mir_wstrdup(szLastStatusMessage);
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
		LPCTSTR szTzo = XPathFmt(pIqNode, L"time[@xmlns='%s']/tzo", JABBER_FEAT_ENTITY_TIME);
		if (szTzo && szTzo[0]) {
			LPCTSTR szMin = wcschr(szTzo, ':');
			int nTz = _wtoi(szTzo) * -2;
			nTz += (nTz < 0 ? -1 : 1) * (szMin ? _wtoi(szMin + 1) / 30 : 0);

			TIME_ZONE_INFORMATION tzinfo;
			if (GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_DAYLIGHT)
				nTz -= tzinfo.DaylightBias / 30;

			setByte(pInfo->m_hContact, "Timezone", (signed char)nTz);

			LPCTSTR szTz = XPathFmt(pIqNode, L"time[@xmlns='%s']/tz", JABBER_FEAT_ENTITY_TIME);
			if (szTz)
				setWString(pInfo->m_hContact, "TzName", szTz);
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
