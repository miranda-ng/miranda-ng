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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"

void CJabberProto::OnIqResultServerDiscoInfo(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	if (iqNode == nullptr)
		return;

	const char *type = iqNode->Attribute("type");
	if (mir_strcmp(type, "result"))
		return;

	auto *query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_INFO);
	if (query == nullptr)
		return;

	for (auto *identity : TiXmlFilter(query, "identity")) {
		JABBER_DISCO_FIELD tmp = {
			identity->Attribute("category"),
			identity->Attribute("type"),
			identity->Attribute("name") };

		if (!mir_strcmp(tmp.category, "pubsub") && !mir_strcmp(tmp.type, "pep")) {
			m_bPepSupported = true;

			if (m_bUseOMEMO) // publish ndes, precreation is not required
				OmemoPublishNodes();

			EnableMenuItems(true);
			RebuildInfoFrame();
			continue;
		}

		NotifyFastHook(hDiscoInfoResult, (WPARAM)&tmp, (LPARAM)(IJabberInterface*)this);
	}

	if (m_ThreadInfo) {
		for (auto *feature : TiXmlFilter(query, "feature")) {
			const char *featureName = feature->Attribute("var");
			if (!featureName)
				continue;

			for (int j = 0; j < g_cJabberFeatCapPairs; j++) {
				if (!mir_strcmp(g_JabberFeatCapPairs[j].szFeature, featureName)) {
					m_ThreadInfo->jabberServerCaps |= g_JabberFeatCapPairs[j].jcbCap;
					break;
				}
			}
		}
	}

	OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_SERVERINFO);
}

void CJabberProto::OnIqResultNestedRosterGroups(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	const char *szGroupDelimeter = nullptr;
	bool bPrivateStorageSupport = false;

	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		bPrivateStorageSupport = true;
		szGroupDelimeter = XPathFmt(iqNode, "query[@xmlns='%s']/roster[@xmlns='%s']", JABBER_FEAT_PRIVATE_STORAGE, JABBER_FEAT_NESTED_ROSTER_GROUPS);
		if (szGroupDelimeter && !szGroupDelimeter[0])
			szGroupDelimeter = nullptr; // "" as roster delimeter is not supported :)
	}

	// global fuckup
	if (m_ThreadInfo == nullptr)
		return;

	// is our default delimiter?
	if ((!szGroupDelimeter && bPrivateStorageSupport) || (szGroupDelimeter && mir_strcmp(szGroupDelimeter, "\\")))
		m_ThreadInfo->send(
			XmlNodeIq("set", SerialNext()) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
				<< XCHILD("roster", "\\") << XATTR("xmlns", JABBER_FEAT_NESTED_ROSTER_GROUPS));

	// roster request
	char *szUserData = mir_strdup(szGroupDelimeter ? szGroupDelimeter : "\\");
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetRoster, JABBER_IQ_TYPE_GET, nullptr, 0, -1, (void*)szUserData))
			<< XCHILDNS("query", JABBER_FEAT_IQ_ROSTER));
}

void CJabberProto::OnIqResultNotes(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		if (auto *query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_PRIVATE_STORAGE))
			if (auto *storage = XmlGetChildByTag(query, "storage", "xmlns", JABBER_FEAT_MIRANDA_NOTES))
				m_notes.LoadXml(storage);
}

void CJabberProto::OnProcessLoginRq(ThreadData *info, DWORD rq)
{
	if (info == nullptr)
		return;

	info->dwLoginRqs |= rq;

	DWORD dwMask = JABBER_LOGIN_ROSTER | JABBER_LOGIN_BOOKMARKS | JABBER_LOGIN_SERVERINFO;
	if ((info->dwLoginRqs & dwMask) == dwMask && !(info->dwLoginRqs & JABBER_LOGIN_BOOKMARKS_AJ)) {
		if (info->jabberServerCaps & JABBER_CAPS_ARCHIVE_AUTO)
			EnableArchive(m_bEnableMsgArchive != 0);

		if (info->jabberServerCaps & JABBER_CAPS_CARBONS)
			EnableCarbons(m_bEnableCarbons != 0);

		if (m_bAutoJoinBookmarks) {
			LIST<JABBER_LIST_ITEM> ll(10);
			LISTFOREACH(i, this, LIST_BOOKMARK)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				if (item != nullptr && !mir_strcmp(item->type, "conference") && item->bAutoJoin)
					ll.insert(item);
			}

			for (auto &item : ll) {
				char room[256], text[128];
				strncpy_s(text, item->jid, _TRUNCATE);
				strncpy_s(room, text, _TRUNCATE);
				char *p = strtok(room, "@");
				char *server = strtok(nullptr, "@");
				if (item->nick && item->nick[0])
					GroupchatJoinRoom(server, p, item->nick, item->password, true);
				else {
					ptrA nick(getUStringA(HContactFromJID(m_szJabberJID), "MyNick"));
					if (nick == nullptr)
						nick = getUStringA("Nick");
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
	if (!(m_StrmMgmt.IsSessionResumed()))
	{
		{
			CJabberIqInfo *pIqInfo = AddIQ(&CJabberProto::OnIqResultNestedRosterGroups, JABBER_IQ_TYPE_GET);
			// ugly hack to prevent hangup during login process
			pIqInfo->SetTimeout(30000);
			m_ThreadInfo->send(
				XmlNodeIq(pIqInfo) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
				<< XCHILDNS("roster", JABBER_FEAT_NESTED_ROSTER_GROUPS));
		}

		// Server-side notes
		m_ThreadInfo->send(
			XmlNodeIq(AddIQ(&CJabberProto::OnIqResultNotes, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
			<< XCHILDNS("storage", JABBER_FEAT_MIRANDA_NOTES));

		m_ThreadInfo->send(
			XmlNodeIq(AddIQ(&CJabberProto::OnIqResultDiscoBookmarks, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE) << XCHILDNS("storage", "storage:bookmarks"));
	}

	m_bPepSupported = false;
	m_ThreadInfo->jabberServerCaps = JABBER_RESOURCE_CAPS_NONE;

	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultServerDiscoInfo, JABBER_IQ_TYPE_GET, m_ThreadInfo->conn.server))
			<< XQUERY(JABBER_FEAT_DISCO_INFO));

	QueryPrivacyLists(m_ThreadInfo);

	ptrA szServerName(getStringA("LastLoggedServer"));
	if (szServerName == nullptr || mir_strcmp(m_ThreadInfo->conn.server, szServerName))
		SendGetVcard(m_szJabberJID);

	setString("LastLoggedServer", m_ThreadInfo->conn.server);
	m_pepServices.ResetPublishAll();
	if (m_bEnableStreamMgmt)
		m_StrmMgmt.CheckState();
}

void CJabberProto::OnIqResultGetAuth(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	// RECVED: result of the request for authentication method
	// ACTION: send account authentication information to log in
	debugLogA("<iq/> iqIdGetAuth");

	const TiXmlElement *queryNode;
	const char *type;
	if ((type = iqNode->Attribute("type")) == nullptr) return;
	if ((queryNode = iqNode->FirstChildElement("query")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultSetAuth, JABBER_IQ_TYPE_SET));
		auto *query = iq << XQUERY("jabber:iq:auth");
		query << XCHILD("username", m_ThreadInfo->conn.username);
		if (queryNode->FirstChildElement("digest") != nullptr && m_ThreadInfo->szStreamId) {
			JabberShaStrBuf buf;
			char text[200];
			mir_snprintf(text, "%s%s", m_ThreadInfo->szStreamId, m_ThreadInfo->conn.password);
			query << XCHILD("digest", JabberSha1(text, buf));
		}
		else if (queryNode->FirstChildElement("password") != nullptr)
			query << XCHILD("password", m_ThreadInfo->conn.password);
		else {
			debugLogA("No known authentication mechanism accepted by the server.");
			m_ThreadInfo->send("</stream:stream>");
			return;
		}

		if (queryNode->FirstChildElement("resource") != nullptr)
			query << XCHILD("resource", m_ThreadInfo->resource);

		m_ThreadInfo->send(iq);
	}
	else if (!mir_strcmp(type, "error")) {
		m_ThreadInfo->send("</stream:stream>");

		wchar_t text[128];
		mir_snwprintf(text, TranslateT("Authentication failed for %s."), m_ThreadInfo->conn.username);
		MsgPopup(0, text, TranslateT("Jabber Authentication"));
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultSetAuth(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const char *type;

	// RECVED: authentication result
	// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
	debugLogA("<iq/> iqIdSetAuth");
	if ((type = iqNode->Attribute("type")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		ptrA tszNick(getUStringA("Nick"));
		if (tszNick == nullptr)
			setUString("Nick", m_ThreadInfo->conn.username);

		OnLoggedIn();
	}
	// What to do if password error? etc...
	else if (!mir_strcmp(type, "error")) {
		m_ThreadInfo->send("</stream:stream>");

		wchar_t text[128];
		mir_snwprintf(text, TranslateT("Authentication failed for %s."), Utf2T(m_ThreadInfo->conn.username).get());
		MsgPopup(0, text, TranslateT("Jabber Authentication"));
		
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultBind(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!m_ThreadInfo || !iqNode)
		return;
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		const char *szJid = XPath(iqNode, "bind[@xmlns='urn:ietf:params:xml:ns:xmpp-bind']/jid");
		if (szJid) {
			if (!strncmp(m_ThreadInfo->fullJID, szJid, _countof(m_ThreadInfo->fullJID)))
				debugLogW(L"Result Bind: %s confirmed ", m_ThreadInfo->fullJID);
			else {
				debugLogW(L"Result Bind: %s changed to %s", m_ThreadInfo->fullJID, szJid);
				strncpy_s(m_ThreadInfo->fullJID, szJid, _TRUNCATE);
			}
		}
		if (m_ThreadInfo->bIsSessionAvailable)
			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResultSession, JABBER_IQ_TYPE_SET))
				<< XCHILDNS("session", "urn:ietf:params:xml:ns:xmpp-session"));
		else
			OnLoggedIn();
	}
	else {
		//rfc3920 page 39
		m_ThreadInfo->send("</stream:stream>");
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultSession(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		OnLoggedIn();
}

void CJabberProto::GroupchatJoinByHContact(MCONTACT hContact, bool autojoin)
{
	ptrA roomjid(getUStringA(hContact, "ChatRoomID"));
	if (roomjid == nullptr)
		return;

	char *room = roomjid;
	char *server = strchr(roomjid, '@');
	if (!server)
		return;

	server[0] = 0; server++;

	ptrA nick(getUStringA(hContact, "MyNick"));
	if (nick == nullptr) {
		nick = JabberNickFromJID(m_szJabberJID);
		if (nick == nullptr)
			return;
	}

	GroupchatJoinRoom(server, room, nick, ptrA(getUStringA(hContact, "Password")), autojoin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetRoster - populates LIST_ROSTER and creates contact for any new rosters

void CJabberProto::OnIqResultGetRoster(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqIdGetRoster");
	ptrA szGroupDelimeter((char *)pInfo->GetUserData());
	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT)
		return;

	auto *queryNode = iqNode->FirstChildElement("query");
	if (queryNode == nullptr)
		return;

	if (mir_strcmp(queryNode->Attribute("xmlns"), JABBER_FEAT_IQ_ROSTER))
		return;

	if (!mir_strcmp(szGroupDelimeter, "\\"))
		szGroupDelimeter = nullptr;

	LIST<void> chatRooms(10);
	OBJLIST<JABBER_HTTP_AVATARS> *httpavatars = new OBJLIST<JABBER_HTTP_AVATARS>(20, JABBER_HTTP_AVATARS::compare);

	for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
		bool bIsTransport = false;
		const char *str = itemNode->Attribute("subscription");

		JABBER_SUBSCRIPTION sub;
		if (str == nullptr) sub = SUB_NONE;
		else if (!mir_strcmp(str, "both")) sub = SUB_BOTH;
		else if (!mir_strcmp(str, "to")) sub = SUB_TO;
		else if (!mir_strcmp(str, "from")) sub = SUB_FROM;
		else sub = SUB_NONE;

		const char *jid = itemNode->Attribute("jid");
		if (jid == nullptr)
			continue;
		if (strchr(jid, '@') == nullptr)
			bIsTransport = true;

		const char *name = itemNode->Attribute("name");
		char *nick = (name != nullptr) ? mir_strdup(name) : JabberNickFromJID(jid);
		if (nick == nullptr)
			continue;

		MCONTACT hContact = HContactFromJID(jid);
		if (hContact == 0) // Received roster has a new JID.
			hContact = DBCreateContact(jid, nick, false, false); // Add the jid (with empty resource) to Miranda contact list.

		JABBER_LIST_ITEM *item = ListAdd(LIST_ROSTER, jid, hContact);
		item->subscription = sub;
		item->bRealContact = true;

		mir_free(item->nick); item->nick = nick;

		auto *groupNode = itemNode->FirstChildElement("group");
		replaceStr(item->group, groupNode->GetText());

		// check group delimiters
		if (item->group && szGroupDelimeter) {
			while (char *szPos = strstr(item->group, szGroupDelimeter)) {
				*szPos = 0;
				szPos += mir_strlen(szGroupDelimeter);
				CMStringA szNewGroup(FORMAT, "%s\\%s", item->group, szPos);
				replaceStr(item->group, szNewGroup.Detach());
			}
		}

		if (name != nullptr) {
			ptrA tszNick(getUStringA(hContact, "Nick"));
			if (tszNick != nullptr) {
				if (mir_strcmp(nick, tszNick) != 0)
					db_set_utf(hContact, "CList", "MyHandle", nick);
				else
					db_unset(hContact, "CList", "MyHandle");
			}
			else db_set_utf(hContact, "CList", "MyHandle", nick);
		}
		else db_unset(hContact, "CList", "MyHandle");

		if (isChatRoom(hContact)) {
			char *wszTitle = NEWSTR_ALLOCA(jid);
			if (char *p = strchr(wszTitle, '@')) *p = 0;
			Chat_NewSession(GCW_CHATROOM, m_szModuleName, Utf2T(jid), Utf2T(wszTitle));

			db_unset(hContact, "CList", "Hidden");
			chatRooms.insert((HANDLE)hContact);
		}
		else UpdateSubscriptionInfo(hContact, item);

		if (!m_bIgnoreRosterGroups) {
			if (item->group != nullptr) {
				Clist_GroupCreate(0, Utf2T(item->group));

				// Don't set group again if already correct, or Miranda may show wrong group count in some case
				ptrA tszGroup(db_get_utfa(hContact, "CList", "Group"));
				if (tszGroup != nullptr) {
					if (mir_strcmp(tszGroup, item->group))
						db_set_utf(hContact, "CList", "Group", item->group);
				}
				else db_set_utf(hContact, "CList", "Group", item->group);
			}
			else db_unset(hContact, "CList", "Group");
		}

		if (hContact != 0) {
			if (bIsTransport)
				setByte(hContact, "IsTransport", true);
			else
				setByte(hContact, "IsTransport", false);
		}

		const char *imagepath = itemNode->Attribute("vz:img");
		if (imagepath)
			httpavatars->insert(new JABBER_HTTP_AVATARS(imagepath, hContact));
	}

	if (httpavatars->getCount())
		ForkThread(&CJabberProto::LoadHttpAvatars, httpavatars);
	else
		delete httpavatars;

	// Delete orphaned contacts (if roster sync is enabled)
	if (m_bRosterSync) {
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

	if (m_bAutoJoinConferences)
		for (auto &it : chatRooms)
			GroupchatJoinByHContact((DWORD_PTR)it, true);

	UI_SAFE_NOTIFY_HWND(m_hwndJabberAddBookmark, WM_JABBER_CHECK_ONLINE);
	WindowList_Broadcast(m_hWindowList, WM_JABBER_CHECK_ONLINE, 0, 0);

	UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);

	if (szGroupDelimeter)
		mir_free(szGroupDelimeter);

	OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_ROSTER);
	RebuildInfoFrame();
}

void CJabberProto::OnIqResultGetRegister(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	// RECVED: result of the request for (agent) registration mechanism
	// ACTION: activate (agent) registration input dialog
	debugLogA("<iq/> iqIdGetRegister");

	const TiXmlElement *queryNode;
	const char *type;
	if ((type = iqNode->Attribute("type")) == nullptr) return;
	if ((queryNode = iqNode->FirstChildElement("query")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		if (m_hwndAgentRegInput)
			SendMessage(m_hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 1 /*success*/, (LPARAM)iqNode);
	}
	else if (!mir_strcmp(type, "error")) {
		if (m_hwndAgentRegInput) {
			auto *errorNode = iqNode->FirstChildElement("error");
			wchar_t *str = JabberErrorMsg(errorNode);
			SendMessage(m_hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 0 /*error*/, (LPARAM)str);
			mir_free(str);
		}
	}
}

void CJabberProto::OnIqResultSetRegister(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	// RECVED: result of registration process
	// ACTION: notify of successful agent registration
	debugLogA("<iq/> iqIdSetRegister");

	const char *type, *from;
	if ((type = iqNode->Attribute("type")) == nullptr) return;
	if ((from = iqNode->Attribute("from")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		MCONTACT hContact = HContactFromJID(from);
		if (hContact != 0)
			setByte(hContact, "IsTransport", true);

		if (m_hwndRegProgress)
			SendMessage(m_hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Registration successful"));
	}
	else if (!mir_strcmp(type, "error")) {
		if (m_hwndRegProgress) {
			auto *errorNode = iqNode->FirstChildElement("error");
			wchar_t *str = JabberErrorMsg(errorNode);
			SendMessage(m_hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)str);
			mir_free(str);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetVcard - processes the server-side v-card

void CJabberProto::OnIqResultGetVcardPhoto(const TiXmlElement *n, MCONTACT hContact, bool &hasPhoto)
{
	debugLogA("JabberIqResultGetVcardPhoto: %d", hasPhoto);
	if (hasPhoto)
		return;

	auto *o = n->FirstChildElement("BINVAL");
	const char *ptszBinval = o->GetText();
	if (o == nullptr || ptszBinval == nullptr)
		return;

	size_t bufferLen;
	ptrA buffer((char*)mir_base64_decode(ptszBinval, &bufferLen));
	if (buffer == nullptr)
		return;

	const char *szPicType = nullptr;
	if (const char *ptszType = n->FirstChildElement("TYPE")->GetText())
		if (ProtoGetAvatarFormatByMimeType(ptszType) != PA_FORMAT_UNKNOWN)
			szPicType = ptszType;

	if (szPicType == nullptr)
		szPicType = ProtoGetAvatarMimeType(ProtoGetBufferFormat(buffer));

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
	bool bRes = WriteFile(hFile, buffer, (DWORD)bufferLen, &nWritten, nullptr) != 0;
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
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid != nullptr) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
			if (item == nullptr) {
				item = ListAdd(LIST_VCARD_TEMP, jid); // adding to the temp list to store information about photo
				if (item != nullptr)
					item->bUseResource = true;
			}
			if (item != nullptr) {
				hasPhoto = true;
				Utf2T oldFile(item->photoFileName);
				if (item->photoFileName && mir_wstrcmp(oldFile, szAvatarFileName))
					DeleteFile(oldFile);
				replaceStr(item->photoFileName, T2Utf(szAvatarFileName));
				debugLogW(L"Contact's picture saved to %s", szAvatarFileName);
				OnIqResultGotAvatar(hContact, o, szPicType);
			}
		}
	}

	if (!hasPhoto)
		DeleteFile(szAvatarFileName);
}

static char* sttGetText(const TiXmlElement *node, const char *tag)
{
	auto *n = node->FirstChildElement(tag);
	if (n == nullptr)
		return nullptr;

	return (char*)n->GetText();
}

void CJabberProto::OnIqResultGetVcard(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const TiXmlElement *vCardNode, *m, *o;
	const char *type, *jid;
	MCONTACT hContact;
	DBVARIANT dbv;

	debugLogA("<iq/> iqIdGetVcard");
	if ((type = iqNode->Attribute("type")) == nullptr) return;
	if ((jid = iqNode->Attribute("from")) == nullptr) return;
	int id = JabberGetPacketID(iqNode);

	if (id == m_nJabberSearchID) {
		m_nJabberSearchID = -1;

		if ((vCardNode = iqNode->FirstChildElement("vCard")) != nullptr) {
			if (!mir_strcmp(type, "result")) {
				PROTOSEARCHRESULT  psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.nick.a = sttGetText(vCardNode, "NICKNAME");
				psr.firstName.a = sttGetText(vCardNode, "FN");
				psr.lastName.a = "";
				psr.email.a = sttGetText(vCardNode, "EMAIL");
				psr.id.a = NEWSTR_ALLOCA(jid);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
			}
			else if (!mir_strcmp(type, "error"))
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		}
		else ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		return;
	}

	size_t len = mir_strlen(m_szJabberJID);
	if (!strnicmp(jid, m_szJabberJID, len) && (jid[len] == '/' || jid[len] == '\0')) {
		hContact = 0;
		debugLogA("Vcard for myself");
	}
	else {
		if ((hContact = HContactFromJID(jid)) == 0)
			return;
		debugLogA("Other user's vcard");
	}

	if (!mir_strcmp(type, "error")) {
		if ((hContact = HContactFromJID(jid)) != 0)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1);
		return;
	}

	if (mir_strcmp(type, "result"))
		return;

	bool hasFn = false, hasNick = false, hasGiven = false, hasFamily = false, hasMiddle = false,
		hasBday = false, hasGender = false, hasPhone = false, hasFax = false, hasCell = false, hasUrl = false,
		hasHome = false, hasHomeStreet = false, hasHomeStreet2 = false, hasHomeLocality = false,
		hasHomeRegion = false, hasHomePcode = false, hasHomeCtry = false,
		hasWork = false, hasWorkStreet = false, hasWorkStreet2 = false, hasWorkLocality = false,
		hasWorkRegion = false, hasWorkPcode = false, hasWorkCtry = false,
		hasOrgname = false, hasOrgunit = false, hasRole = false, hasTitle = false, hasDesc = false, hasPhoto = false;
	int nEmail = 0, nPhone = 0, nYear, nMonth, nDay;

	if ((vCardNode = iqNode->FirstChildElement("vCard")) != nullptr) {
		for (auto *n : TiXmlEnum(vCardNode)) {
			if (n->Name() == nullptr)
				continue;
			
			if (!mir_strcmp(n->Name(), "FN")) {
				if (n->GetText() != nullptr) {
					hasFn = true;
					setUString(hContact, "FullName", n->GetText());
				}
			}
			else if (!mir_strcmp(n->Name(), "NICKNAME")) {
				if (n->GetText() != nullptr) {
					hasNick = true;
					setUString(hContact, "Nick", n->GetText());
				}
			}
			else if (!mir_strcmp(n->Name(), "N")) {
				// First/Last name
				if (!hasGiven && !hasFamily && !hasMiddle) {
					if ((m = n->FirstChildElement("GIVEN")) != nullptr && m->GetText() != nullptr) {
						hasGiven = true;
						setUString(hContact, "FirstName", m->GetText());
					}
					if ((m = n->FirstChildElement("FAMILY")) != nullptr && m->GetText() != nullptr) {
						hasFamily = true;
						setUString(hContact, "LastName", m->GetText());
					}
					if ((m = n->FirstChildElement("MIDDLE")) != nullptr && m->GetText() != nullptr) {
						hasMiddle = true;
						setUString(hContact, "MiddleName", m->GetText());
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "EMAIL")) {
				// E-mail address(es)
				if ((m = n->FirstChildElement("USERID")) == nullptr)	// Some bad client put e-mail directly in <EMAIL/> instead of <USERID/>
					m = n;
				if (m->GetText() != nullptr) {
					char text[100];
					if (hContact != 0) {
						if (nEmail == 0)
							mir_strcpy(text, "e-mail");
						else
							mir_snprintf(text, "e-mail%d", nEmail - 1);
					}
					else mir_snprintf(text, "e-mail%d", nEmail);
					setUString(hContact, text, m->GetText());

					if (hContact == 0) {
						mir_snprintf(text, "e-mailFlag%d", nEmail);
						int nFlag = 0;
						if (n->FirstChildElement("HOME") != nullptr) nFlag |= JABBER_VCEMAIL_HOME;
						if (n->FirstChildElement("WORK") != nullptr) nFlag |= JABBER_VCEMAIL_WORK;
						if (n->FirstChildElement("INTERNET") != nullptr) nFlag |= JABBER_VCEMAIL_INTERNET;
						if (n->FirstChildElement("X400") != nullptr) nFlag |= JABBER_VCEMAIL_X400;
						setWord(text, nFlag);
					}
					nEmail++;
				}
			}
			else if (!mir_strcmp(n->Name(), "BDAY")) {
				// Birthday
				if (!hasBday && n->GetText() != nullptr) {
					if (hContact != 0) {
						if (sscanf(n->GetText(), "%d-%d-%d", &nYear, &nMonth, &nDay) == 3) {
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
						setUString("BirthDate", n->GetText());
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "GENDER")) {
				// Gender
				if (!hasGender && n->GetText() != nullptr) {
					if (hContact != 0) {
						if (n->GetText()[0] && strchr("mMfF", n->GetText()[0]) != nullptr) {
							hasGender = true;
							setByte(hContact, "Gender", (BYTE)toupper(n->GetText()[0]));
						}
					}
					else {
						hasGender = true;
						setUString("GenderString", n->GetText());
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "ADR")) {
				if (!hasHome && n->FirstChildElement("HOME") != nullptr) {
					// Home address
					char text[128];
					hasHome = true;
					if ((m = n->FirstChildElement("STREET")) != nullptr && m->GetText() != nullptr) {
						hasHomeStreet = true;
						if (hContact != 0) {
							if ((o = n->FirstChildElement("EXTADR")) != nullptr && o->GetText() != nullptr)
								mir_snprintf(text, "%s\r\n%s", m->GetText(), o->GetText());
							else if ((o = n->FirstChildElement("EXTADD")) != nullptr && o->GetText() != nullptr)
								mir_snprintf(text, "%s\r\n%s", m->GetText(), o->GetText());
							else
								strncpy_s(text, m->GetText(), _TRUNCATE);

							setUString(hContact, "Street", text);
						}
						else {
							setUString(hContact, "Street", m->GetText());
							if ((m = n->FirstChildElement("EXTADR")) == nullptr)
								m = n->FirstChildElement("EXTADD");
							if (m != nullptr && m->GetText() != nullptr) {
								hasHomeStreet2 = true;
								setUString(hContact, "Street2", m->GetText());
							}
						}
					}

					if ((m = n->FirstChildElement("LOCALITY")) != nullptr && m->GetText() != nullptr) {
						hasHomeLocality = true;
						setUString(hContact, "City", m->GetText());
					}
					if ((m = n->FirstChildElement("REGION")) != nullptr && m->GetText() != nullptr) {
						hasHomeRegion = true;
						setUString(hContact, "State", m->GetText());
					}
					if ((m = n->FirstChildElement("PCODE")) != nullptr && m->GetText() != nullptr) {
						hasHomePcode = true;
						setUString(hContact, "ZIP", m->GetText());
					}
					if ((m = n->FirstChildElement("CTRY")) == nullptr || m->GetText() == nullptr)	// Some bad client use <COUNTRY/> instead of <CTRY/>
						m = n->FirstChildElement("COUNTRY");
					if (m != nullptr && m->GetText() != nullptr) {
						hasHomeCtry = true;
						setUString(hContact, "Country", m->GetText());
					}
				}

				if (!hasWork && n->FirstChildElement("WORK") != nullptr) {
					// Work address
					hasWork = true;
					if ((m = n->FirstChildElement("STREET")) != nullptr && m->GetText() != nullptr) {
						char text[128];
						hasWorkStreet = true;
						if (hContact != 0) {
							if ((o = n->FirstChildElement("EXTADR")) != nullptr && o->GetText() != nullptr)
								mir_snprintf(text, "%s\r\n%s", m->GetText(), o->GetText());
							else if ((o = n->FirstChildElement("EXTADD")) != nullptr && o->GetText() != nullptr)
								mir_snprintf(text, "%s\r\n%s", m->GetText(), o->GetText());
							else
								strncpy_s(text, m->GetText(), _TRUNCATE);
							text[_countof(text) - 1] = '\0';
							setUString(hContact, "CompanyStreet", text);
						}
						else {
							setUString(hContact, "CompanyStreet", m->GetText());
							if ((m = n->FirstChildElement("EXTADR")) == nullptr)
								m = n->FirstChildElement("EXTADD");
							if (m != nullptr && m->GetText() != nullptr) {
								hasWorkStreet2 = true;
								setUString(hContact, "CompanyStreet2", m->GetText());
							}
						}
					}

					if ((m = n->FirstChildElement("LOCALITY")) != nullptr && m->GetText() != nullptr) {
						hasWorkLocality = true;
						setUString(hContact, "CompanyCity", m->GetText());
					}
					if ((m = n->FirstChildElement("REGION")) != nullptr && m->GetText() != nullptr) {
						hasWorkRegion = true;
						setUString(hContact, "CompanyState", m->GetText());
					}
					if ((m = n->FirstChildElement("PCODE")) != nullptr && m->GetText() != nullptr) {
						hasWorkPcode = true;
						setUString(hContact, "CompanyZIP", m->GetText());
					}
					if ((m = n->FirstChildElement("CTRY")) == nullptr || m->GetText() == nullptr)	// Some bad client use <COUNTRY/> instead of <CTRY/>
						m = n->FirstChildElement("COUNTRY");
					if (m != nullptr && m->GetText() != nullptr) {
						hasWorkCtry = true;
						setUString(hContact, "CompanyCountry", m->GetText());
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "TEL")) {
				// Telephone/Fax/Cellular
				if ((m = n->FirstChildElement("NUMBER")) != nullptr && m->GetText() != nullptr) {
					if (hContact != 0) {
						if (!hasFax && n->FirstChildElement("FAX") != nullptr) {
							hasFax = true;
							setUString(hContact, "Fax", m->GetText());
						}
						else if (!hasCell && n->FirstChildElement("CELL") != nullptr) {
							hasCell = true;
							setUString(hContact, "Cellular", m->GetText());
						}
						else if (!hasPhone &&
							(n->FirstChildElement("HOME") != nullptr || n->FirstChildElement("WORK") != nullptr || n->FirstChildElement("VOICE") != nullptr ||
							(n->FirstChildElement("FAX") == nullptr &&
							 n->FirstChildElement("PAGER") == nullptr &&
							 n->FirstChildElement("MSG") == nullptr &&
							 n->FirstChildElement("CELL") == nullptr &&
							 n->FirstChildElement("VIDEO") == nullptr &&
							 n->FirstChildElement("BBS") == nullptr &&
							 n->FirstChildElement("MODEM") == nullptr &&
							 n->FirstChildElement("ISDN") == nullptr &&
							 n->FirstChildElement("PCS") == nullptr)))
						{
							hasPhone = true;
							setUString(hContact, "Phone", m->GetText());
						}
					}
					else {
						char text[100];
						mir_snprintf(text, "Phone%d", nPhone);
						setUString(text, m->GetText());

						mir_snprintf(text, "PhoneFlag%d", nPhone);
						int nFlag = 0;
						if (n->FirstChildElement("HOME")  != nullptr) nFlag |= JABBER_VCTEL_HOME;
						if (n->FirstChildElement("WORK")  != nullptr) nFlag |= JABBER_VCTEL_WORK;
						if (n->FirstChildElement("VOICE") != nullptr) nFlag |= JABBER_VCTEL_VOICE;
						if (n->FirstChildElement("FAX")   != nullptr) nFlag |= JABBER_VCTEL_FAX;
						if (n->FirstChildElement("PAGER") != nullptr) nFlag |= JABBER_VCTEL_PAGER;
						if (n->FirstChildElement("MSG")   != nullptr) nFlag |= JABBER_VCTEL_MSG;
						if (n->FirstChildElement("CELL")  != nullptr) nFlag |= JABBER_VCTEL_CELL;
						if (n->FirstChildElement("VIDEO") != nullptr) nFlag |= JABBER_VCTEL_VIDEO;
						if (n->FirstChildElement("BBS")   != nullptr) nFlag |= JABBER_VCTEL_BBS;
						if (n->FirstChildElement("MODEM") != nullptr) nFlag |= JABBER_VCTEL_MODEM;
						if (n->FirstChildElement("ISDN")  != nullptr) nFlag |= JABBER_VCTEL_ISDN;
						if (n->FirstChildElement("PCS")   != nullptr) nFlag |= JABBER_VCTEL_PCS;
						setWord(text, nFlag);
						nPhone++;
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "URL")) {
				// Homepage
				if (!hasUrl && n->GetText() != nullptr) {
					hasUrl = true;
					setUString(hContact, "Homepage", n->GetText());
				}
			}
			else if (!mir_strcmp(n->Name(), "ORG")) {
				if (!hasOrgname && !hasOrgunit) {
					if ((m = n->FirstChildElement("ORGNAME")) != nullptr && m->GetText() != nullptr) {
						hasOrgname = true;
						setUString(hContact, "Company", m->GetText());
					}
					if ((m = n->FirstChildElement("ORGUNIT")) != nullptr && m->GetText() != nullptr) {	// The real vCard can have multiple <ORGUNIT/> but we will only display the first one
						hasOrgunit = true;
						setUString(hContact, "CompanyDepartment", m->GetText());
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "ROLE")) {
				if (!hasRole && n->GetText() != nullptr) {
					hasRole = true;
					setUString(hContact, "Role", n->GetText());
				}
			}
			else if (!mir_strcmp(n->Name(), "TITLE")) {
				if (!hasTitle && n->GetText() != nullptr) {
					hasTitle = true;
					setUString(hContact, "CompanyPosition", n->GetText());
				}
			}
			else if (!mir_strcmp(n->Name(), "DESC")) {
				if (!hasDesc && n->GetText() != nullptr) {
					hasDesc = true;
					CMStringA tszMemo(n->GetText());
					tszMemo.Replace("\n", "\r\n");
					setUString(hContact, "About", tszMemo);
				}
			}
			else if (!mir_strcmp(n->Name(), "PHOTO"))
				OnIqResultGetVcardPhoto(n, hContact, hasPhoto);
		}
	}

	if (hasFn && !hasNick) {
		ptrA nick(getUStringA(hContact, "Nick"));
		ptrA jidNick(JabberNickFromJID(jid));
		if (!nick || (jidNick && !mir_strcmpi(nick, jidNick)))
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
		const char *p = strchr(jid, '@');
		ResolveTransportNicks((p != nullptr) ? p + 1 : jid);
	}
	else {
		if ((hContact = HContactFromJID(jid)) != 0)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1);
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
	}
}

void CJabberProto::OnIqResultSetVcard(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetVcard");
	if (iqNode->Attribute("type"))
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
}

void CJabberProto::OnIqResultSetSearch(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const TiXmlElement *queryNode;
	const char *type;
	int id;

	debugLogA("<iq/> iqIdGetSearch");
	if ((type = iqNode->Attribute("type")) == nullptr) return;
	if ((id = JabberGetPacketID(iqNode)) == -1) return;

	if (!mir_strcmp(type, "result")) {
		if ((queryNode = iqNode->FirstChildElement("query")) == nullptr)
			return;

		PROTOSEARCHRESULT psr = {};
		psr.cbSize = sizeof(psr);
		for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
			if (auto *jid = itemNode->Attribute("jid")) {
				psr.id.w = mir_utf8decodeW(jid);
				debugLogW(L"Result jid = %s", jid);
				if (auto *p = itemNode->FirstChildElement("nick")->GetText())
					psr.nick.w = mir_utf8decodeW(p);
				if (auto *p = itemNode->FirstChildElement("first")->GetText())
					psr.firstName.w = mir_utf8decodeW(p);
				if (auto *p = itemNode->FirstChildElement("last")->GetText())
					psr.lastName.w = mir_utf8decodeW(p);
				if (auto *p = itemNode->FirstChildElement("email")->GetText())
					psr.email.w = mir_utf8decodeW(p);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);

				replaceStrW(psr.id.w, 0);
				replaceStrW(psr.nick.w, 0);
				replaceStrW(psr.firstName.w, 0);
				replaceStrW(psr.lastName.w, 0);
				replaceStrW(psr.email.w, 0);
			}
		}

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
	else if (!mir_strcmp(type, "error"))
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

void CJabberProto::OnIqResultExtSearch(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const TiXmlElement *queryNode;

	debugLogA("<iq/> iqIdGetExtSearch");
	const char *type = iqNode->Attribute("type");
	if (type == nullptr)
		return;

	int id = JabberGetPacketID(iqNode);
	if (id == -1)
		return;

	if (!mir_strcmp(type, "result")) {
		if ((queryNode = iqNode->FirstChildElement("query")) == nullptr) return;
		if ((queryNode = queryNode->FirstChildElement("x")) == nullptr) return;
		for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
			PROTOSEARCHRESULT  psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;

			for (auto *fieldNode : TiXmlFilter(itemNode, "field")) {
				const char *fieldName = fieldNode->Attribute("var");
				if (fieldName == nullptr)
					continue;

				auto *n = fieldNode->FirstChildElement("value");
				if (n == nullptr)
					continue;

				if (!mir_strcmp(fieldName, "jid")) {
					psr.id.w = (wchar_t*)n->GetText();
					debugLogW(L"Result jid = %s", psr.id.w);
				}
				else if (!mir_strcmp(fieldName, "nickname"))
					psr.nick.w = mir_utf8decodeW(n->GetText());
				else if (!mir_strcmp(fieldName, "fn"))
					psr.firstName.w = mir_utf8decodeW(n->GetText());
				else if (!mir_strcmp(fieldName, "given"))
					psr.firstName.w = mir_utf8decodeW(n->GetText());
				else if (!mir_strcmp(fieldName, "family"))
					psr.lastName.w = mir_utf8decodeW(n->GetText());
				else if (!mir_strcmp(fieldName, "email"))
					psr.email.w = mir_utf8decodeW(n->GetText());
			}

			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);

			replaceStrW(psr.id.w, 0);
			replaceStrW(psr.nick.w, 0);
			replaceStrW(psr.firstName.w, 0);
			replaceStrW(psr.lastName.w, 0);
			replaceStrW(psr.email.w, 0);
		}

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
	else if (!mir_strcmp(type, "error"))
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

void CJabberProto::OnIqResultSetPassword(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetPassword");

	const char *type = iqNode->Attribute("type");
	if (type == nullptr)
		return;

	if (!mir_strcmp(type, "result")) {
		strncpy_s(m_ThreadInfo->conn.password, m_ThreadInfo->tszNewPassword, _TRUNCATE);
		MessageBox(nullptr, TranslateT("Password is successfully changed. Don't forget to update your password in the Jabber protocol option."), TranslateT("Change Password"), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
	}
	else if (!mir_strcmp(type, "error"))
		MessageBox(nullptr, TranslateT("Password cannot be changed."), TranslateT("Change Password"), MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
}

void CJabberProto::OnIqResultGetVCardAvatar(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> OnIqResultGetVCardAvatar");

	const char *from = iqNode->Attribute("from");
	if (from == nullptr)
		return;

	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	const char *type;
	if ((type = iqNode->Attribute("type")) == nullptr) return;
	if (mir_strcmp(type, "result")) return;

	auto *vCard = iqNode->FirstChildElement("vCard");
	if (vCard == nullptr) return;
	vCard = vCard->FirstChildElement("PHOTO");
	if (vCard == nullptr) return;

	if (vCard->NoChildren()) {
		delSetting(hContact, "AvatarHash");
		if (ptrW(getWStringA(hContact, "AvatarSaved")) != nullptr) {
			delSetting(hContact, "AvatarSaved");
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr, 0);
		}
		return;
	}

	const char *mimeType = vCard->FirstChildElement("TYPE")->GetText();
	auto *n = vCard->FirstChildElement("BINVAL");
	if (n == nullptr)
		return;

	setByte(hContact, "AvatarXVcard", 1);
	OnIqResultGotAvatar(hContact, n, mimeType);
}

void CJabberProto::OnIqResultGetClientAvatar(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const char *type;

	debugLogA("<iq/> iqIdResultGetClientAvatar");

	const char *from = iqNode->Attribute("from");
	if (from == nullptr)
		return;
	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	const TiXmlElement *n = nullptr;
	if ((type = iqNode->Attribute("type")) != nullptr && !mir_strcmp(type, "result")) {
		auto *queryNode = iqNode->FirstChildElement("query");
		if (queryNode != nullptr) {
			const char *xmlns = queryNode->Attribute("xmlns");
			if (!mir_strcmp(xmlns, JABBER_FEAT_AVATAR))
				n = queryNode->FirstChildElement("data");
		}
	}

	if (n != nullptr) {
		OnIqResultGotAvatar(hContact, n, n->Attribute("mimetype"));
		return;
	}

	char szJid[JABBER_MAX_JID_LEN];
	mir_strncpy(szJid, from, _countof(szJid));
	char *res = strchr(szJid, '/');
	if (res != nullptr)
		*res = 0;

	// Try server stored avatar
	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultGetServerAvatar, JABBER_IQ_TYPE_GET, szJid));
	iq << XQUERY(JABBER_FEAT_SERVER_AVATAR);
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnIqResultGetServerAvatar(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdResultGetServerAvatar");

	const char *from = iqNode->Attribute("from");
	if (from == nullptr)
		return;

	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	const TiXmlElement *n = nullptr;
	const char *type = iqNode->Attribute("type");
	if (!mir_strcmp(type, "result")) {
		auto *queryNode = iqNode->FirstChildElement("query");
		if (queryNode != nullptr) {
			const char *xmlns = queryNode->Attribute("xmlns");
			if (!mir_strcmp(xmlns, JABBER_FEAT_SERVER_AVATAR))
				n = queryNode->FirstChildElement("data");
		}
	}

	if (n != nullptr) {
		OnIqResultGotAvatar(hContact, n, n->Attribute("mimetype"));
		return;
	}

	char szJid[JABBER_MAX_JID_LEN];
	mir_strncpy(szJid, from, _countof(szJid));
	char *res = strchr(szJid, '/');
	if (res != nullptr)
		*res = 0;

	// Try VCard photo
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetVCardAvatar, JABBER_IQ_TYPE_GET, szJid)) << XCHILDNS("vCard", JABBER_FEAT_VCARD_TEMP));
}


void CJabberProto::OnIqResultGotAvatar(MCONTACT hContact, const TiXmlElement *n, const char *mimeType)
{
	size_t resultLen;
	ptrA body((char*)mir_base64_decode(n->GetText(), &resultLen));
	if (body == nullptr)
		return;

	int pictureType;
	if (mimeType != nullptr)
		pictureType = ProtoGetAvatarFormatByMimeType(mimeType);
	else
		pictureType = ProtoGetBufferFormat(body, nullptr);
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
		DeleteFileW(tszFileName);
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

void CJabberProto::OnIqResultDiscoBookmarks(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	// RECVED: list of bookmarks
	// ACTION: refresh bookmarks dialog
	debugLogA("<iq/> iqIdGetBookmarks");
	const char *type = iqNode->Attribute("type");
	if (type == nullptr)
		return;

	const char *jid;
	if (!mir_strcmp(type, "result")) {
		if (m_ThreadInfo && !(m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)) {
			m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(true);
		}

		if (auto *storageNode = XmlGetChildByTag(iqNode->FirstChildElement("query"), "storage", "xmlns", "storage:bookmarks")) {
			ListRemoveList(LIST_BOOKMARK);

			for (auto *itemNode : TiXmlEnum(storageNode)) {
				if (const char *name = itemNode->Name()) {
					if (!mir_strcmp(name, "conference") && (jid = itemNode->Attribute("jid"))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->name = mir_utf8decodeW(itemNode->Attribute("name"));
						item->type = mir_strdup("conference");
						item->bUseResource = true;
						item->nick = mir_strdup(XPath(itemNode, "nick"));
						item->password = mir_strdup(XPath(itemNode, "password"));

						const char *autoJ = itemNode->Attribute("autojoin");
						if (autoJ != nullptr)
							item->bAutoJoin = !mir_strcmp(autoJ, "true") || !mir_strcmp(autoJ, "1");
					}
					else if (!mir_strcmp(name, "url") && (jid = itemNode->Attribute("url"))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->bUseResource = true;
						item->name = mir_utf8decodeW(itemNode->Attribute("name"));
						item->type = mir_strdup("url");
					}
				}
			}

			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_REFRESH);
			m_ThreadInfo->bBookmarksLoaded = true;
			OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_BOOKMARKS);
		}
	}
	else if (!mir_strcmp(type, "error")) {
		if (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE) {
			m_ThreadInfo->jabberServerCaps &= ~JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(true);
			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_ACTIVATE);
		}
	}
}

void CJabberProto::SetBookmarkRequest(XmlNodeIq &iq)
{
	TiXmlElement *query = iq << XQUERY(JABBER_FEAT_PRIVATE_STORAGE);
	TiXmlElement *storage = query << XCHILDNS("storage", "storage:bookmarks");

	LISTFOREACH(i, this, LIST_BOOKMARK)
	{
		JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
		if (item == nullptr || item->jid == nullptr)
			continue;

		if (!mir_strcmp(item->type, "conference")) {
			TiXmlElement *itemNode = storage << XCHILD("conference") << XATTR("jid", item->jid);
			if (item->name)
				itemNode << XATTR("name", T2Utf(item->name));
			if (item->bAutoJoin)
				itemNode << XATTRI("autojoin", 1);
			if (item->nick)
				itemNode << XCHILD("nick", item->nick);
			if (item->password)
				itemNode << XCHILD("password", item->password);
		}

		if (!mir_strcmp(item->type, "url")) {
			TiXmlElement *itemNode = storage << XCHILD("url") << XATTR("url", item->jid);
			if (item->name)
				itemNode << XATTR("name", T2Utf(item->name));
		}
	}
}

void CJabberProto::OnIqResultSetBookmarks(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	// RECVED: server's response
	// ACTION: refresh bookmarks list dialog

	debugLogA("<iq/> iqIdSetBookmarks");

	const char *type = iqNode->Attribute("type");
	if (type == nullptr)
		return;

	if (!mir_strcmp(type, "result")) {
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_REFRESH);
	}
	else if (!mir_strcmp(type, "error")) {
		auto *errorNode = iqNode->FirstChildElement("error");
		wchar_t *str = JabberErrorMsg(errorNode);
		MessageBox(nullptr, str, TranslateT("Jabber Bookmarks Error"), MB_OK | MB_SETFOREGROUND);
		mir_free(str);
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_JABBER_ACTIVATE);
	}
}

// last activity (XEP-0012) support
void CJabberProto::OnIqResultLastActivity(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->m_szFrom));
	if (r == nullptr)
		return;

	time_t lastActivity = -1;
	if (pInfo->m_nIqType == JABBER_IQ_TYPE_RESULT) {
		const char *szSeconds = XPath(iqNode, "query[@xmlns='jabber:iq:last']/@seconds");
		if (szSeconds) {
			int nSeconds = atoi(szSeconds);
			if (nSeconds > 0)
				lastActivity = time(0) - nSeconds;
		}

		const char *szLastStatusMessage = XPath(iqNode, "query[@xmlns='jabber:iq:last']");
		if (szLastStatusMessage) // replace only if it exists
			r->m_szStatusMessage = mir_strdup(szLastStatusMessage);
	}

	r->m_dwIdleStartTime = lastActivity;

	JabberUserInfoUpdate(pInfo->GetHContact());
}

// entity time (XEP-0202) support
void CJabberProto::OnIqResultEntityTime(const TiXmlElement *pIqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->m_hContact)
		return;

	if (pInfo->m_nIqType == JABBER_IQ_TYPE_RESULT) {
		const char *szTzo = XPathFmt(pIqNode, "time[@xmlns='%s']/tzo", JABBER_FEAT_ENTITY_TIME);
		if (szTzo && szTzo[0]) {
			const char *szMin = strchr(szTzo, ':');
			int nTz = atoi(szTzo) * -2;
			nTz += (nTz < 0 ? -1 : 1) * (szMin ? atoi(szMin + 1) / 30 : 0);

			TIME_ZONE_INFORMATION tzinfo;
			if (GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_DAYLIGHT)
				nTz -= tzinfo.DaylightBias / 30;

			setByte(pInfo->m_hContact, "Timezone", (signed char)nTz);

			const char *szTz = XPathFmt(pIqNode, "time[@xmlns='%s']/tz", JABBER_FEAT_ENTITY_TIME);
			if (szTz)
				setUString(pInfo->m_hContact, "TzName", szTz);
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
