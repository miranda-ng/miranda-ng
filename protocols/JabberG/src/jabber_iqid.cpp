/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

void CJabberProto::OnIqResultServerItemsInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	const char *from = XmlGetAttr(iqNode, "from");
	if (from == nullptr || pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT)
		return;

	int hul_ver = -1;
	auto query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_INFO);
	for (auto *feature : TiXmlFilter(query, "feature")) {
		auto *var = XmlGetAttr(feature, "var");

		// HTTP Upload
		if (!mir_strcmp(var, JABBER_FEAT_UPLOAD0) && hul_ver < 0)
			hul_ver = 0;
		if (!mir_strcmp(var, JABBER_FEAT_UPLOAD))
			hul_ver = 1;
		
		// SOCKS5 Proxy
		if (!mir_strcmp(var, JABBER_FEAT_BYTESTREAMS)) {
			m_bBsProxyManual = true;
			setString("BsProxyServer", from);
		}
	}

	if (hul_ver >= 0) {
		m_bUseHttpUpload = true;
		setString("HttpUpload", from);
		setByte("HttpUploadVer", hul_ver);

		for (auto *x : TiXmlFilter(query, "x")) {
			if (mir_strcmp(XmlGetAttr(x, "type"), "result") || mir_strcmp(XmlGetAttr(x, "xmlns"), JABBER_FEAT_DATA_FORMS))
				continue;
			
			if (auto *field = XmlGetChildByTag(x, "field", "var", "FORM_TYPE"))
				if (!mir_strcmp(XmlGetChildText(field, "value"), hul_ver ? JABBER_FEAT_UPLOAD : JABBER_FEAT_UPLOAD0))
					if (auto *sfield = XmlGetChildByTag(x, "field", "var", "max-file-size"))
						setDword("HttpUploadMaxSize", XmlGetChildInt(sfield, "value"));
		}
	}
}

void CJabberProto::OnIqResultServerDiscoItems(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode == nullptr)
		return;

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		for (auto *item : TiXmlFilter(XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_ITEMS), "item")) {
			const char *szJid = XmlGetAttr(item, "jid");

			XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultServerItemsInfo, JABBER_IQ_TYPE_GET));
			iq << XATTR("from", m_ThreadInfo->fullJID) << XATTR("to", szJid) << XQUERY(JABBER_FEAT_DISCO_INFO);
			m_ThreadInfo->send(iq);
		}
	}
}

void CJabberProto::OnIqResultServerDiscoInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode == nullptr)
		return;

	const char *type = XmlGetAttr(iqNode, "type");
	if (mir_strcmp(type, "result"))
		return;

	auto *query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_INFO);
	if (query == nullptr)
		return;

	for (auto *identity : TiXmlFilter(query, "identity")) {
		JABBER_DISCO_FIELD tmp = {
			XmlGetAttr(identity, "category"),
			XmlGetAttr(identity, "type"),
			XmlGetAttr(identity, "name") };

		if (!mir_strcmp(tmp.category, "pubsub") && !mir_strcmp(tmp.type, "pep")) {
			m_bPepSupported = true;

			if (m_bUseOMEMO) {
				XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultGetOmemodevicelist, JABBER_IQ_TYPE_GET));
				iq << XATTR("from", m_ThreadInfo->fullJID);
				iq << XCHILDNS("pubsub", "http://jabber.org/protocol/pubsub")
					<< XCHILD("items") << XATTR("node", JABBER_FEAT_OMEMO ".devicelist");

				m_ThreadInfo->send(iq);
			}

			EnableMenuItems(true);
			continue;
		}

		NotifyFastHook(hDiscoInfoResult, (WPARAM)&tmp, (LPARAM)(IJabberInterface*)this);
	}

	JabberCapsBits jcb = 0;

	for (auto *feature : TiXmlFilter(query, "feature")) {
		const char *featureName = XmlGetAttr(feature, "var");
		if (!featureName)
			continue;

		for (int j = 0; j < g_cJabberFeatCapPairs; j++) {
			if (!mir_strcmp(g_JabberFeatCapPairs[j].szFeature, featureName)) {
				jcb |= g_JabberFeatCapPairs[j].jcbCap;
				break;
			}
		}
	}

	if (void *p = pInfo->GetUserData()) {
		const char *szNode = (const char *)p;
		const char *szVer = szNode + strlen(szNode) + 1;
		g_clientCapsManager.SetClientCaps(szNode, szVer, "", jcb);
		mir_free(p);
	}

	if (m_ThreadInfo)
		m_ThreadInfo->jabberServerCaps |= jcb;

	OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_SERVERINFO);

	// http upload autodetect
	if ((char)getByte("HttpUploadVer", -1) == -1) {
		setByte("HttpUploadVer", 0); // not to call autodetect twice

		XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultServerDiscoItems, JABBER_IQ_TYPE_GET));
		iq << XATTR("from", m_ThreadInfo->fullJID) << XATTR("to", m_ThreadInfo->conn.server) << XCHILDNS("query", JABBER_FEAT_DISCO_ITEMS);
		m_ThreadInfo->send(iq);
	}
}

void CJabberProto::OnIqResultNestedRosterGroups(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	const char *szGroupDelimiter = nullptr;
	bool bPrivateStorageSupport = false;

	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		bPrivateStorageSupport = true;
		auto *xmlDelimiter = XmlGetChildByTag(XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_PRIVATE_STORAGE), "roster", "xmlns", JABBER_FEAT_NESTED_ROSTER_GROUPS);
		if (xmlDelimiter)
			if (!mir_strcmp(szGroupDelimiter = xmlDelimiter->GetText(), "\\"))
				szGroupDelimiter = nullptr;
	}

	// is our default delimiter?
	if (!szGroupDelimiter && bPrivateStorageSupport)
		m_ThreadInfo->send(
			XmlNodeIq("set", SerialNext()) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
				<< XCHILD("roster", "\\") << XATTR("xmlns", JABBER_FEAT_NESTED_ROSTER_GROUPS));

	// roster request
	m_szGroupDelimiter = mir_strdup(szGroupDelimiter);
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetRoster, JABBER_IQ_TYPE_GET))
			<< XCHILDNS("query", JABBER_FEAT_IQ_ROSTER));
}

void CJabberProto::OnIqResultNotes(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		if (auto *query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_PRIVATE_STORAGE))
			if (auto *storage = XmlGetChildByTag(query, "storage", "xmlns", JABBER_FEAT_MIRANDA_NOTES))
				m_notes.LoadXml(storage);
}

void CJabberProto::OnProcessLoginRq(ThreadData *info, uint32_t rq)
{
	if (info == nullptr)
		return;

	info->dwLoginRqs |= rq;

	uint32_t dwMask = JABBER_LOGIN_ROSTER | JABBER_LOGIN_BOOKMARKS | JABBER_LOGIN_SERVERINFO;
	if ((info->dwLoginRqs & dwMask) == dwMask && !(info->dwLoginRqs & JABBER_LOGIN_BOOKMARKS_AJ)) {
		if (info->jabberServerCaps & JABBER_CAPS_ARCHIVE_AUTO)
			EnableArchive(m_bEnableMsgArchive != 0);

		// Server seems to support carbon copies, let's enable/disable them
		if (info->jabberServerCaps & JABBER_CAPS_CARBONS)
			m_ThreadInfo->send(XmlNodeIq("set", SerialNext()) << XCHILDNS((m_bEnableCarbons) ? "enable" : "disable", JABBER_FEAT_CARBONS));

		// Server seems to support MAM, let's retrieve MAM settings
		if (m_bEnableMam)
			if (info->jabberServerCaps & JABBER_CAPS_MAM)
				m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResultMamInfo, JABBER_IQ_TYPE_GET, 0, this)) << XCHILDNS("prefs", JABBER_FEAT_MAM));

		if (m_bAutoJoinBookmarks) {
			LIST<JABBER_LIST_ITEM> ll(10);
			LISTFOREACH(i, this, LIST_BOOKMARK)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				debugLogA("BOOKMARK #%d: %s %s", i, item->type, item->jid);

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
	m_bMamPrefsAvailable = false;
	m_bJabberOnline = true;
	m_tmJabberLoggedInTime = time(0);

	m_ThreadInfo->dwLoginRqs = 0;

	// XEP-0083 support
	if (!(m_StrmMgmt.IsSessionResumed()))
	{
		// ugly hack to prevent hangup during login process
		CJabberIqInfo *pIqInfo = AddIQ(&CJabberProto::OnIqResultNestedRosterGroups, JABBER_IQ_TYPE_GET);
		pIqInfo->SetTimeout(30000);
		m_ThreadInfo->send(XmlNodeIq(pIqInfo) << XQUERY(JABBER_FEAT_PRIVATE_STORAGE) << XCHILDNS("roster", JABBER_FEAT_NESTED_ROSTER_GROUPS));

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

	if (m_ThreadInfo->pPendingQuery) {
		m_ThreadInfo->send(XmlNodeIq(m_ThreadInfo->pPendingQuery) << XQUERY(JABBER_FEAT_DISCO_INFO));
		m_ThreadInfo->pPendingQuery = nullptr;
	}

	char szBareJid[JABBER_MAX_JID_LEN];
	JabberStripJid(m_ThreadInfo->fullJID, szBareJid, _countof(szBareJid));
	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIqResultServerDiscoInfo, JABBER_IQ_TYPE_GET, szBareJid))
			<< XQUERY(JABBER_FEAT_DISCO_INFO));

	QueryPrivacyLists(m_ThreadInfo);

	ptrA szServerName(getStringA("LastLoggedServer"));
	if (szServerName == nullptr || mir_strcmp(m_ThreadInfo->conn.server, szServerName)) {
		setString("LastLoggedServer", m_ThreadInfo->conn.server);
		SendGetVcard(0);
	}
	else {
		time_t elapsed = time(0) - getDword("LastGetVcard");
		if (elapsed > 84600) // read vcard on login once a day
			SendGetVcard(0);
		else
			m_impl.m_heartBeat.Start(elapsed * 1000);
	}

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
	if ((type = XmlGetAttr(iqNode, "type")) == nullptr) return;
	if ((queryNode = XmlFirstChild(iqNode, "query")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultSetAuth, JABBER_IQ_TYPE_SET));
		auto *query = iq << XQUERY("jabber:iq:auth");
		query << XCHILD("username", m_ThreadInfo->conn.username);
		if (XmlFirstChild(queryNode, "digest") != nullptr && m_ThreadInfo->szStreamId) {
			JabberShaStrBuf buf;
			char text[200];
			mir_snprintf(text, "%s%s", m_ThreadInfo->szStreamId.get(), m_ThreadInfo->conn.password);
			query << XCHILD("digest", JabberSha1(text, buf));
		}
		else if (XmlFirstChild(queryNode, "password") != nullptr)
			query << XCHILD("password", m_ThreadInfo->conn.password);
		else {
			debugLogA("No known authentication mechanism accepted by the server.");
			m_ThreadInfo->send("</stream:stream>");
			return;
		}

		if (XmlFirstChild(queryNode, "resource") != nullptr)
			query << XCHILD("resource", m_ThreadInfo->resource);

		m_ThreadInfo->send(iq);
	}
	else if (!mir_strcmp(type, "error")) {
		m_ThreadInfo->send("</stream:stream>");

		wchar_t text[128];
		mir_snwprintf(text, TranslateT("Authentication failed for %s."), m_ThreadInfo->conn.username);
		MsgPopup(0, text, TranslateT("Authentication"));
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
	if ((type = XmlGetAttr(iqNode, "type")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		ptrA szNick(getUStringA("Nick"));
		if (!mir_strlen(szNick))
			setUString("Nick", m_ThreadInfo->conn.username);

		OnLoggedIn();
	}
	// What to do if password error? etc...
	else if (!mir_strcmp(type, "error")) {
		m_ThreadInfo->send("</stream:stream>");

		wchar_t text[128];
		mir_snwprintf(text, TranslateT("Authentication failed for %s."), Utf2T(m_ThreadInfo->conn.username).get());
		MsgPopup(0, text, TranslateT("Authentication"));
		
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		m_ThreadInfo = nullptr;	// To disallow auto reconnect
	}
}

void CJabberProto::OnIqResultBind(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!m_ThreadInfo || !iqNode)
		return;
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		const char *szJid = XmlGetChildText(XmlGetChildByTag(iqNode, "bind", "xmlns", JABBER_FEAT_BIND), "jid");
		if (szJid) {
			if (!strncmp(m_ThreadInfo->fullJID, szJid, _countof(m_ThreadInfo->fullJID)))
				debugLogA("Result Bind: %s confirmed ", m_ThreadInfo->fullJID);
			else {
				debugLogA("Result Bind: %s changed to %s", m_ThreadInfo->fullJID, szJid);
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
	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT)
		return;

	auto *queryNode = XmlFirstChild(iqNode, "query");
	if (queryNode == nullptr)
		return;

	if (mir_strcmp(XmlGetAttr(queryNode, "xmlns"), JABBER_FEAT_IQ_ROSTER))
		return;

	LIST<void> chatRooms(10);
	OBJLIST<JABBER_HTTP_AVATARS> *httpavatars = new OBJLIST<JABBER_HTTP_AVATARS>(20, JABBER_HTTP_AVATARS::compare);

	for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
		bool bIsTransport = false;
		const char *str = XmlGetAttr(itemNode, "subscription");

		JABBER_SUBSCRIPTION sub;
		if (str == nullptr) sub = SUB_NONE;
		else if (!mir_strcmp(str, "both")) sub = SUB_BOTH;
		else if (!mir_strcmp(str, "to")) sub = SUB_TO;
		else if (!mir_strcmp(str, "from")) sub = SUB_FROM;
		else sub = SUB_NONE;

		const char *jid = XmlGetAttr(itemNode, "jid");
		if (jid == nullptr)
			continue;
		if (strchr(jid, '@') == nullptr)
			bIsTransport = true;

		const char *name = XmlGetAttr(itemNode, "name");
		char *nick = (name != nullptr) ? mir_strdup(name) : JabberNickFromJID(jid);
		if (nick == nullptr)
			continue;

		MCONTACT hContact = HContactFromJID(jid);
		if (hContact == 0) // Received roster has a new JID.
			hContact = DBCreateContact(jid, nick, false, false); // Add the jid (with empty resource) to Miranda contact list.

		JABBER_LIST_ITEM *item = ListAdd(LIST_ROSTER, jid, hContact);
		item->subscription = sub;
		item->bRealContact = true;

		replaceStr(item->nick, nick);
		replaceStr(item->group, XmlGetChildText(itemNode, "group"));
		UpdateItem(item, name);

		if (isChatRoom(hContact)) {
			char *szTitle = NEWSTR_ALLOCA(jid);
			if (char *p = strchr(szTitle, '@'))
				*p = 0;
			Chat_NewSession(GCW_CHATROOM, m_szModuleName, Utf2T(jid), Utf2T(szTitle));

			Contact::Hide(hContact, false);
			chatRooms.insert((HANDLE)hContact);
		}
		else UpdateSubscriptionInfo(hContact, item);

		if (hContact != 0) {
			if (bIsTransport)
				setByte(hContact, "IsTransport", true);
			else
				setByte(hContact, "IsTransport", false);
		}

		const char *imagepath = XmlGetAttr(itemNode, "vz:img");
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
				debugLogA("Syncing roster: preparing to delete %s (hContact=0x%x)", item->jid, item->hContact);
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

	UI_SAFE_NOTIFY_HWND(m_hwndJabberAddBookmark, WM_PROTO_CHECK_ONLINE);
	WindowList_Broadcast(m_hWindowList, WM_PROTO_CHECK_ONLINE, 0, 0);

	UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);

	OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_ROSTER);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetVcard - processes the server-side v-card

void CJabberProto::OnIqResultGetVcardPhoto(const TiXmlElement *n, MCONTACT hContact, bool &hasPhoto)
{
	debugLogA("JabberIqResultGetVcardPhoto: %d", hasPhoto);
	if (hasPhoto)
		return;

	const char *ptszBinval = XmlGetChildText(n, "BINVAL");
	if (ptszBinval == nullptr)
		return;

	size_t bufferLen;
	ptrA buffer((char*)mir_base64_decode(ptszBinval, &bufferLen));
	if (buffer == nullptr)
		return;

	const char *szPicType = XmlGetChildText(n, "TYPE");
	if (ProtoGetAvatarFormatByMimeType(szPicType) == PA_FORMAT_UNKNOWN)
		szPicType = ProtoGetAvatarMimeType(ProtoGetBufferFormat(buffer));
	if (szPicType == nullptr)
		return;

	uint8_t digest[MIR_SHA1_HASH_SIZE];
	mir_sha1_ctx sha1ctx;
	mir_sha1_init(&sha1ctx);
	mir_sha1_append(&sha1ctx, (uint8_t *)buffer.get(), bufferLen);
	mir_sha1_finish(&sha1ctx, digest);

	char digestHex[MIR_SHA1_HASH_SIZE*2 + 1];
	bin2hex(digest, sizeof(digest), digestHex);
	CMStringA oldHash(getMStringA(hContact, "AvatarHash"));
	if (oldHash == digestHex) {
		hasPhoto = true;
		debugLogA("same avatar hash, skipping");
		return;
	}

	wchar_t szAvatarFileName[MAX_PATH];
	GetAvatarFileName(hContact, szAvatarFileName, _countof(szAvatarFileName));

	debugLogW(L"Picture file name set to %s", szAvatarFileName);
	HANDLE hFile = CreateFile(szAvatarFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	debugLogA("Writing %d bytes", bufferLen);
	DWORD nWritten;
	bool bRes = WriteFile(hFile, buffer, (uint32_t)bufferLen, &nWritten, nullptr) != 0;
	CloseHandle(hFile);
	if (!bRes)
		return;

	debugLogA("%d bytes written", nWritten);

	setString(hContact, "AvatarHash", digestHex);

	if (hContact == 0) {
		hasPhoto = true;
		debugLogW(L"My picture saved to %s", szAvatarFileName);
		ReportSelfAvatarChanged();
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
				OnIqResultGotAvatar(hContact, ptszBinval, szPicType);
			}
		}
	}

	if (!hasPhoto)
		DeleteFile(szAvatarFileName);
}

void CJabberProto::OnIqResultGetVcard(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdGetVcard");

	const char *type, *jid;
	if ((type = XmlGetAttr(iqNode, "type")) == nullptr) return;
	if ((jid = XmlGetAttr(iqNode, "from")) == nullptr) return;
	int id = JabberGetPacketID(iqNode);

	if (id == m_nJabberSearchID) {
		m_nJabberSearchID = -1;

		if (auto *vCardNode = XmlFirstChild(iqNode, "vCard")) {
			if (!mir_strcmp(type, "result")) {
				PROTOSEARCHRESULT  psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.nick.a = (char*)XmlGetChildText(vCardNode, "NICKNAME");
				psr.firstName.a = (char*)XmlGetChildText(vCardNode, "FN");
				psr.lastName.a = "";
				psr.email.a = (char*)XmlGetChildText(vCardNode, "EMAIL");
				psr.id.a = NEWSTR_ALLOCA(jid);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
			}
			else if (!mir_strcmp(type, "error"))
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
		}
		else ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
		return;
	}

	MCONTACT hContact;
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

	CMStringA szNodeHash;
	if (auto *vCardNode = XmlFirstChild(iqNode, "vCard")) {
		if (hContact == 0) {
			XmlNodeHash hasher;
			vCardNode->Accept(&hasher);
			szNodeHash = hasher.getResult();
		}

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
					if (auto *p = XmlGetChildText(n, "GIVEN")) {
						hasGiven = true;
						setUString(hContact, "FirstName", p);
					}
					if (auto *p = XmlGetChildText(n, "FAMILY")) {
						hasFamily = true;
						setUString(hContact, "LastName", p);
					}
					if (auto *p = XmlGetChildText(n, "MIDDLE")) {
						hasMiddle = true;
						setUString(hContact, "MiddleName", p);
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "EMAIL")) {
				// E-mail address(es)
				if (auto *p = XmlGetChildText(n, "USERID")) {
					char text[100];
					if (hContact != 0) {
						if (nEmail == 0)
							mir_strcpy(text, "e-mail");
						else
							mir_snprintf(text, "e-mail%d", nEmail - 1);
					}
					else mir_snprintf(text, "e-mail%d", nEmail);
					setUString(hContact, text, p);

					if (hContact == 0) {
						mir_snprintf(text, "e-mailFlag%d", nEmail);
						int nFlag = 0;
						if (XmlFirstChild(n, "HOME") != nullptr) nFlag |= JABBER_VCEMAIL_HOME;
						if (XmlFirstChild(n, "WORK") != nullptr) nFlag |= JABBER_VCEMAIL_WORK;
						if (XmlFirstChild(n, "INTERNET") != nullptr) nFlag |= JABBER_VCEMAIL_INTERNET;
						if (XmlFirstChild(n, "X400") != nullptr) nFlag |= JABBER_VCEMAIL_X400;
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
							setWord(hContact, "BirthYear", (uint16_t)nYear);
							setByte(hContact, "BirthMonth", (uint8_t)nMonth);
							setByte(hContact, "BirthDay", (uint8_t)nDay);

							SYSTEMTIME sToday = { 0 };
							GetLocalTime(&sToday);
							int nAge = sToday.wYear - nYear;
							if (sToday.wMonth < nMonth || (sToday.wMonth == nMonth && sToday.wDay < nDay))
								nAge--;
							if (nAge)
								setWord(hContact, "Age", (uint16_t)nAge);
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
							setByte(hContact, "Gender", (uint8_t)toupper(n->GetText()[0]));
						}
					}
					else {
						hasGender = true;
						setUString("GenderString", n->GetText());
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "ADR")) {
				if (!hasHome && XmlFirstChild(n, "HOME") != nullptr) {
					// Home address
					char text[128];
					hasHome = true;
					if (auto *p = XmlGetChildText(n, "STREET")) {
						hasHomeStreet = true;
						if (hContact != 0) {
							if (auto *o = XmlGetChildText(n, "EXTADR"))
								mir_snprintf(text, "%s\r\n%s", p, o);
							else if (o = XmlGetChildText(n, "EXTADD"))
								mir_snprintf(text, "%s\r\n%s", p, o);
							else
								strncpy_s(text, p, _TRUNCATE);

							setUString(hContact, "Street", text);
						}
						else {
							setUString(hContact, "Street", p);
							if (p = XmlGetChildText(n, "EXTADR")) {
								hasHomeStreet2 = true;
								setUString(hContact, "Street2", p);
							}
							else if (p = XmlGetChildText(n, "EXTADD")) {
								hasHomeStreet2 = true;
								setUString(hContact, "Street2", p);
							}
						}
					}

					if (auto *p = XmlGetChildText(n, "LOCALITY")) {
						hasHomeLocality = true;
						setUString(hContact, "City", p);
					}
					if (auto *p = XmlGetChildText(n, "REGION")) {
						hasHomeRegion = true;
						setUString(hContact, "State", p);
					}
					if (auto *p = XmlGetChildText(n, "PCODE")) {
						hasHomePcode = true;
						setUString(hContact, "ZIP", p);
					}
					if (auto *p = XmlGetChildText(n, "CTRY"))	{ // Some bad client use <COUNTRY/> instead of <CTRY/>
						hasHomeCtry = true;
						setUString(hContact, "Country", p);
					}
				}

				if (!hasWork && XmlFirstChild(n, "WORK") != nullptr) {
					// Work address
					hasWork = true;
					if (auto *p = XmlGetChildText(n, "STREET")) {
						char text[128];
						hasWorkStreet = true;
						if (hContact != 0) {
							if (auto *o = XmlGetChildText(n, "EXTADR"))
								mir_snprintf(text, "%s\r\n%s", p, o);
							else if (o = XmlGetChildText(n, "EXTADD"))
								mir_snprintf(text, "%s\r\n%s", p, o);
							else
								strncpy_s(text, p, _TRUNCATE);
							text[_countof(text) - 1] = '\0';
							setUString(hContact, "CompanyStreet", text);
						}
						else {
							setUString(hContact, "CompanyStreet", p);
							
							if (p = XmlGetChildText(n, "EXTADR")) {
								hasWorkStreet2 = true;
								setUString(hContact, "CompanyStreet2", p);
							}
							else if (p = XmlGetChildText(n, "EXTADD")) {
								hasWorkStreet2 = true;
								setUString(hContact, "CompanyStreet2", p);
							}
						}
					}

					if (auto *p = XmlGetChildText(n, "LOCALITY")) {
						hasWorkLocality = true;
						setUString(hContact, "CompanyCity", p);
					}
					if (auto *p = XmlGetChildText(n, "REGION")) {
						hasWorkRegion = true;
						setUString(hContact, "CompanyState", p);
					}
					if (auto *p = XmlGetChildText(n, "PCODE")) {
						hasWorkPcode = true;
						setUString(hContact, "CompanyZIP", p);
					}
					if (auto *p = XmlGetChildText(n, "CTRY")) {
						hasWorkCtry = true;
						setUString(hContact, "CompanyCountry", p);
					}
				}
			}
			else if (!mir_strcmp(n->Name(), "TEL")) {
				// Telephone/Fax/Cellular
				if (auto *p = XmlGetChildText(n, "NUMBER")) {
					if (hContact != 0) {
						if (!hasFax && XmlFirstChild(n, "FAX") != nullptr) {
							hasFax = true;
							setUString(hContact, "Fax", p);
						}
						else if (!hasCell && XmlFirstChild(n, "CELL") != nullptr) {
							hasCell = true;
							setUString(hContact, "Cellular", p);
						}
						else if (!hasPhone &&
							(XmlFirstChild(n, "HOME") != nullptr || XmlFirstChild(n, "WORK") != nullptr || XmlFirstChild(n, "VOICE") != nullptr ||
							(XmlFirstChild(n, "FAX") == nullptr &&
							 XmlFirstChild(n, "PAGER") == nullptr &&
							 XmlFirstChild(n, "MSG") == nullptr &&
							 XmlFirstChild(n, "CELL") == nullptr &&
							 XmlFirstChild(n, "VIDEO") == nullptr &&
							 XmlFirstChild(n, "BBS") == nullptr &&
							 XmlFirstChild(n, "MODEM") == nullptr &&
							 XmlFirstChild(n, "ISDN") == nullptr &&
							 XmlFirstChild(n, "PCS") == nullptr)))
						{
							hasPhone = true;
							setUString(hContact, "Phone", p);
						}
					}
					else {
						char text[100];
						mir_snprintf(text, "Phone%d", nPhone);
						setUString(text, p);

						mir_snprintf(text, "PhoneFlag%d", nPhone);
						int nFlag = 0;
						if (XmlFirstChild(n, "HOME")  != nullptr) nFlag |= JABBER_VCTEL_HOME;
						if (XmlFirstChild(n, "WORK")  != nullptr) nFlag |= JABBER_VCTEL_WORK;
						if (XmlFirstChild(n, "VOICE") != nullptr) nFlag |= JABBER_VCTEL_VOICE;
						if (XmlFirstChild(n, "FAX")   != nullptr) nFlag |= JABBER_VCTEL_FAX;
						if (XmlFirstChild(n, "PAGER") != nullptr) nFlag |= JABBER_VCTEL_PAGER;
						if (XmlFirstChild(n, "MSG")   != nullptr) nFlag |= JABBER_VCTEL_MSG;
						if (XmlFirstChild(n, "CELL")  != nullptr) nFlag |= JABBER_VCTEL_CELL;
						if (XmlFirstChild(n, "VIDEO") != nullptr) nFlag |= JABBER_VCTEL_VIDEO;
						if (XmlFirstChild(n, "BBS")   != nullptr) nFlag |= JABBER_VCTEL_BBS;
						if (XmlFirstChild(n, "MODEM") != nullptr) nFlag |= JABBER_VCTEL_MODEM;
						if (XmlFirstChild(n, "ISDN")  != nullptr) nFlag |= JABBER_VCTEL_ISDN;
						if (XmlFirstChild(n, "PCS")   != nullptr) nFlag |= JABBER_VCTEL_PCS;
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
					if (auto *p = XmlGetChildText(n, "ORGNAME")) {
						hasOrgname = true;
						setUString(hContact, "Company", p);
					}
					if (auto *p = XmlGetChildText(n, "ORGUNIT")) {
						hasOrgunit = true;
						setUString(hContact, "CompanyDepartment", p);
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
				if (!ptrA(getStringA(hContact, text))) break;
				delSetting(hContact, text);
			}
			nEmail++;
		}
	}
	else {
		while (true) {
			char text[100];
			mir_snprintf(text, "e-mail%d", nEmail);
			if (delSetting(text))
				break;
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
			if (delSetting(text))
				break;
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
		if (!delSetting(hContact, "AvatarHash"))
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr);
	}
	
	if (id == m_ThreadInfo->resolveID) {
		const char *p = strchr(jid, '@');
		ResolveTransportNicks((p != nullptr) ? p + 1 : jid);
	}
	else {
		if (hContact != 0)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1);
		else {
			CMStringA oldHash(getMStringA("VCardHash"));
			if (oldHash != szNodeHash) {
				setString("VCardHash", szNodeHash);
				SendPresence(m_iStatus, false);
			}
		}
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
	}
}

void CJabberProto::OnIqResultSetVcard(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetVcard");
	if (XmlGetAttr(iqNode, "type")) {
		WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
		SendPresence(m_iStatus, false);
	}
}

void CJabberProto::OnIqResultGetOmemodevicelist(const TiXmlElement* iqNode, CJabberIqInfo*)
{
	const char *from = XmlGetAttr(iqNode, "from"); //replies for our jid don't contain "from"
	bool res = false;
	if (auto *pubsubNode = XmlGetChildByTag(iqNode, "pubsub", "xmlns", JABBER_FEAT_PUBSUB))
		if (auto *itemsNode = XmlGetChildByTag(pubsubNode, "items", "node", JABBER_FEAT_OMEMO ".devicelist"))
			res = OmemoHandleDeviceList(from, itemsNode);

	if (!from && !res) {
		for (int i = 0;; i++) {
			CMStringA szSetting(FORMAT, "%s%d", omemo::DevicePrefix, i);
			if (!getDword(szSetting, 0))
				break;

			delSetting(szSetting);
		}

		OmemoAnnounceDevice(false); //Publish own device if we can't retrieve up to date list 
		OmemoSendBundle();
	}
}

void CJabberProto::OnIqResultSetSearch(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdGetSearch");

	const char *type = XmlGetAttr(iqNode, "type");
	int id = JabberGetPacketID(iqNode);
	if (type == nullptr || id == -1)
		return;

	if (!mir_strcmp(type, "result")) {
		if (auto *queryNode = XmlFirstChild(iqNode, "query")) {
			PROTOSEARCHRESULT psr = {};
			psr.cbSize = sizeof(psr);
			for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
				if (auto *jid = XmlGetAttr(itemNode, "jid")) {
					psr.id.w = mir_utf8decodeW(jid);
					debugLogA("Result jid = %s", jid);
					if (auto *p = XmlGetChildText(itemNode, "nick"))
						psr.nick.w = mir_utf8decodeW(p);
					if (auto *p = XmlGetChildText(itemNode, "first"))
						psr.firstName.w = mir_utf8decodeW(p);
					if (auto *p = XmlGetChildText(itemNode, "last"))
						psr.lastName.w = mir_utf8decodeW(p);
					if (auto *p = XmlGetChildText(itemNode, "email"))
						psr.email.w = mir_utf8decodeW(p);
					ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&psr);

					replaceStrW(psr.id.w, 0);
					replaceStrW(psr.nick.w, 0);
					replaceStrW(psr.firstName.w, 0);
					replaceStrW(psr.lastName.w, 0);
					replaceStrW(psr.email.w, 0);
				}
			}
		}
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
	}
	else if (!mir_strcmp(type, "error"))
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
}

void CJabberProto::OnIqResultExtSearch(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdGetExtSearch");

	const char *type = XmlGetAttr(iqNode, "type");
	int id = JabberGetPacketID(iqNode);
	if (type == nullptr || id == -1)
		return;

	if (!mir_strcmp(type, "result")) {
		if (auto *queryNode = XmlFirstChild(iqNode, "query"))
			if (queryNode = XmlFirstChild(queryNode, "x"))
				for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
					PROTOSEARCHRESULT psr = {};
					psr.cbSize = sizeof(psr);
					psr.flags = PSR_UNICODE;

					for (auto *fieldNode : TiXmlFilter(itemNode, "field")) {
						const char *fieldName = XmlGetAttr(fieldNode, "var");
						if (fieldName == nullptr)
							continue;

						auto *n = XmlFirstChild(fieldNode, "value");
						if (n == nullptr)
							continue;

						if (!mir_strcmp(fieldName, "jid")) {
							psr.id.w = mir_utf8decodeW(n->GetText());
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

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
	}
	else if (!mir_strcmp(type, "error"))
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
}

void CJabberProto::OnIqResultSetPassword(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdSetPassword");

	const char *type = XmlGetAttr(iqNode, "type");
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

	const char *from = XmlGetAttr(iqNode, "from");
	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	const char *type;
	if ((type = XmlGetAttr(iqNode, "type")) == nullptr) return;
	if (mir_strcmp(type, "result")) return;

	auto *vCard = XmlFirstChild(iqNode, "vCard");
	if (vCard == nullptr) return;
	vCard = XmlFirstChild(vCard, "PHOTO");
	if (vCard == nullptr) return;

	if (vCard->NoChildren()) {
		if (!delSetting(hContact, "AvatarHash"))
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr);
		return;
	}

	if (auto *pszText = XmlGetChildText(vCard, "BINVAL"))
		OnIqResultGotAvatar(hContact, pszText, XmlGetChildText(vCard, "TYPE"));
}

void CJabberProto::OnIqResultGetServerAvatar(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdResultGetServerAvatar");

	const char *from = XmlGetAttr(iqNode, "from");
	MCONTACT hContact = HContactFromJID(from);
	if (hContact == 0)
		return;

	const TiXmlElement *n = nullptr;
	const char *type = XmlGetAttr(iqNode, "type");
	if (!mir_strcmp(type, "result")) {
		auto *queryNode = XmlFirstChild(iqNode, "query");
		if (queryNode != nullptr) {
			const char *xmlns = XmlGetAttr(queryNode, "xmlns");
			if (!mir_strcmp(xmlns, JABBER_FEAT_SERVER_AVATAR))
				n = XmlFirstChild(queryNode, "data");
		}
	}

	if (n != nullptr) {
		OnIqResultGotAvatar(hContact, n->GetText(), XmlGetAttr(n, "mimetype"));
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


void CJabberProto::OnIqResultGotAvatar(MCONTACT hContact, const char *pszText, const char *mimeType)
{
	size_t resultLen;
	ptrA body((char*)mir_base64_decode(pszText, &resultLen));
	if (body == nullptr)
		return;

	int pictureType;
	if (mimeType != nullptr)
		pictureType = ProtoGetAvatarFormatByMimeType(mimeType);
	else
		pictureType = ProtoGetBufferFormat(body, nullptr);
	if (pictureType == PA_FORMAT_UNKNOWN) {
		debugLogA("Invalid mime type specified for picture: %s", mimeType);
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

	uint8_t digest[MIR_SHA1_HASH_SIZE];
	mir_sha1_ctx sha;
	mir_sha1_init(&sha);
	mir_sha1_append(&sha, (uint8_t*)(char*)body, resultLen);
	mir_sha1_finish(&sha, digest);

	GetAvatarFileName(hContact, tszFileName, _countof(tszFileName));
	wcsncpy_s(ai.filename, tszFileName, _TRUNCATE);

	FILE *out = _wfopen(tszFileName, L"wb");
	if (out != nullptr) {
		fwrite(body, resultLen, 1, out);
		fclose(out);

		char buffer[41];
		setString(hContact, "AvatarHash", bin2hex(digest, sizeof(digest), buffer));
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai));
		debugLogW(L"Broadcast new avatar: %s", ai.filename);
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks

void CJabberProto::OnIqResultDiscoBookmarks(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	// RECVED: list of bookmarks
	// ACTION: refresh bookmarks dialog
	debugLogA("<iq/> iqIdGetBookmarks");
	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	const char *jid;
	if (!mir_strcmp(type, "result")) {
		if (m_ThreadInfo && !(m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)) {
			m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(true);
		}

		if (auto *storageNode = XmlGetChildByTag(XmlFirstChild(iqNode, "query"), "storage", "xmlns", "storage:bookmarks")) {
			ListRemoveList(LIST_BOOKMARK);

			for (auto *itemNode : TiXmlEnum(storageNode)) {
				if (const char *name = itemNode->Name()) {
					if (!mir_strcmp(name, "conference") && (jid = XmlGetAttr(itemNode, "jid"))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->name = mir_utf8decodeW(XmlGetAttr(itemNode, "name"));
						item->type = mir_strdup("conference");
						item->bUseResource = true;
						item->nick = mir_strdup(XmlGetChildText(itemNode, "nick"));
						item->password = mir_strdup(XmlGetChildText(itemNode, "password"));

						const char *autoJ = XmlGetAttr(itemNode, "autojoin");
						if (autoJ != nullptr)
							item->bAutoJoin = !mir_strcmp(autoJ, "true") || !mir_strcmp(autoJ, "1");
						debugLogA("+BOOKMARK %s %s", item->type, item->jid);
					}
					else if (!mir_strcmp(name, "url") && (jid = XmlGetAttr(itemNode, "url"))) {
						JABBER_LIST_ITEM *item = ListAdd(LIST_BOOKMARK, jid);
						item->bUseResource = true;
						item->name = mir_utf8decodeW(XmlGetAttr(itemNode, "name"));
						item->type = mir_strdup("url");
					}
				}
			}

			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_PROTO_REFRESH);
			m_ThreadInfo->bBookmarksLoaded = true;
			OnProcessLoginRq(m_ThreadInfo, JABBER_LOGIN_BOOKMARKS);
		}
	}
	else if (!mir_strcmp(type, "error")) {
		if (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE) {
			m_ThreadInfo->jabberServerCaps &= ~JABBER_CAPS_PRIVATE_STORAGE;
			EnableMenuItems(true);
			UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_PROTO_ACTIVATE);
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

	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	if (!mir_strcmp(type, "result")) {
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_PROTO_REFRESH);
	}
	else if (!mir_strcmp(type, "error")) {
		MessageBox(nullptr, JabberErrorMsg(iqNode), TranslateT("Jabber Bookmarks Error"), MB_OK | MB_SETFOREGROUND);
		UI_SAFE_NOTIFY(m_pDlgBookmarks, WM_PROTO_ACTIVATE);
	}
}

// last activity (XEP-0012) support
void CJabberProto::OnIqResultLastActivity(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->GetFrom()));
	if (r == nullptr)
		return;

	time_t lastActivity = -1;
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		if (auto *xmlLast = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_LAST_ACTIVITY)) {
			int nSeconds = xmlLast->IntAttribute("seconds");
			lastActivity = (nSeconds == 0) ? 0 : time(0) - nSeconds;

			if (const char *szLastStatusMessage = xmlLast->GetText())
				r->m_szStatusMessage = mir_strdup(szLastStatusMessage);
		}
	}

	r->m_dwIdleStartTime = lastActivity;

	JabberUserInfoUpdate(pInfo->GetHContact());
}

// entity time (XEP-0202) support
void CJabberProto::OnIqResultEntityTime(const TiXmlElement *pIqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetHContact())
		return;

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		auto *xmlTime = XmlGetChildByTag(pIqNode, "time", "xmlns", JABBER_FEAT_ENTITY_TIME);
		if (xmlTime) {
			const char *szTzo = XmlGetChildText(xmlTime, "tzo");
			if (szTzo && szTzo[0]) {
				const char *szMin = strchr(szTzo, ':');
				int nTz = atoi(szTzo) * -2;
				nTz += (nTz < 0 ? -1 : 1) * (szMin ? atoi(szMin + 1) / 30 : 0);

				TIME_ZONE_INFORMATION tzinfo;
				if (GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_DAYLIGHT)
					nTz -= tzinfo.DaylightBias / 30;

				setByte(pInfo->GetHContact(), "Timezone", (signed char)nTz);

				const char *szTz = XmlGetChildText(xmlTime, "tz");
				if (szTz)
					setUString(pInfo->GetHContact(), "TzName", szTz);
				else
					delSetting(pInfo->GetHContact(), "TzName");
				return;
			}
		}
	}
	else if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR) {
		if (getWord(pInfo->GetHContact(), "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			return;
	}

	delSetting(pInfo->GetHContact(), "Timezone");
	delSetting(pInfo->GetHContact(), "TzName");
}
