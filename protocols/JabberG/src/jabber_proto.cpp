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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_disco.h"

#pragma warning(disable:4355)

static int compareTransports(const char *p1, const char *p2)
{
	return mir_strcmpi(p1, p2);
}

static int compareListItems(const JABBER_LIST_ITEM *p1, const JABBER_LIST_ITEM *p2)
{
	if (p1->list != p2->list)
		return p1->list - p2->list;

	// for bookmarks, temporary contacts & groupchat members
	// resource must be used in the comparison
	if ((p1->list == LIST_ROSTER && (p1->bUseResource == true || p2->bUseResource == true))
		|| (p1->list == LIST_BOOKMARK) || (p1->list == LIST_VCARD_TEMP))
		return mir_strcmpi(p1->jid, p2->jid);

	char szp1[JABBER_MAX_JID_LEN], szp2[JABBER_MAX_JID_LEN];
	JabberStripJid(p1->jid, szp1, _countof(szp1));
	JabberStripJid(p2->jid, szp2, _countof(szp2));
	return mir_strcmpi(szp1, szp2);
}

CJabberProto::CJabberProto(const char *aProtoName, const wchar_t *aUserName) :
	PROTO<CJabberProto>(aProtoName, aUserName),
	m_omemo(this),
	m_lstTransports(50, compareTransports),
	m_lstRoster(50, compareListItems),
	m_iqManager(this),
	m_messageManager(this),
	m_presenceManager(this),
	m_sendManager(this),
	m_adhocManager(this),
	m_clientCapsManager(this),
	m_privacyListManager(this),
	m_privacyMenuServiceAllocated(-1),
	m_priorityMenuVal(0),
	m_priorityMenuValSet(false),
	m_hPrivacyMenuRoot(nullptr),
	m_hPrivacyMenuItems(10),
	m_lstJabberFeatCapPairsDynamic(2),
	m_uEnabledFeatCapsDynamic(0),
	m_StrmMgmt(this),

	m_bBsDirect(this, "BsDirect", true),
	m_bAllowVersionRequests(this, "m_bAllowVersionRequests", true),
	m_bAcceptHttpAuth(this, "m_bAcceptHttpAuth", true),
	m_bAddRoster2Bookmarks(this, "m_bAddRoster2Bookmarks", true),
	m_bAutoAcceptAuthorization(this, "AutoAcceptAuthorization", false),
	m_bAutoAcceptMUC(this, "AutoAcceptMUC", false),
	m_bAutoAdd(this, "AutoAdd", true),
	m_bAutoJoinBookmarks(this, "AutoJoinBookmarks", true),
	m_bAutoJoinConferences(this, "AutoJoinConferences", 0),
	m_bAutoJoinHidden(this, "AutoJoinHidden", true),
	m_bAvatarType(this, "AvatarType", PA_FORMAT_UNKNOWN),
	m_bBsDirectManual(this, "BsDirectManual", false),
	m_bBsOnlyIBB(this, "BsOnlyIBB", false),
	m_bBsProxyManual(this, "BsProxyManual", false),
	m_bDisable3920auth(this, "Disable3920auth", false),
	m_bDisableFrame(this, "DisableFrame", true),
	m_bEnableAvatars(this, "EnableAvatars", true),
	m_bEnableRemoteControl(this, "EnableRemoteControl", false),
	m_bEnableMsgArchive(this, "EnableMsgArchive", false),
	m_bEnableUserActivity(this, "EnableUserActivity", true),
	m_bEnableUserMood(this, "EnableUserMood", true),
	m_bEnableUserTune(this, "EnableUserTune", false),
	m_bEnableZlib(this, "EnableZlib", true),
	m_bExtendedSearch(this, "ExtendedSearch", true),
	m_bFixIncorrectTimestamps(this, "FixIncorrectTimestamps", true),
	m_bGcLogAffiliations(this, "GcLogAffiliations", false),
	m_bGcLogBans(this, "GcLogBans", true),
	m_bGcLogConfig(this, "GcLogConfig", false),
	m_bGcLogRoles(this, "GcLogRoles", false),
	m_bGcLogStatuses(this, "GcLogStatuses", false),
	m_bGcLogChatHistory(this, "GcLogChatHistory", true),
	m_bHostNameAsResource(this, "HostNameAsResource", false),
	m_bIgnoreMUCInvites(this, "IgnoreMUCInvites", false),
	m_bKeepAlive(this, "KeepAlive", true),
	m_bLogChatstates(this, "LogChatstates", false),
	m_bLogPresence(this, "LogPresence", true),
	m_bLogPresenceErrors(this, "LogPresenceErrors", false),
	m_bManualConnect(this, "ManualConnect", false),
	m_bMsgAck(this, "MsgAck", false),
	m_bRosterSync(this, "RosterSync", false),
	m_bSavePassword(this, "SavePassword", true),
	m_bShowForeignResourceInMirVer(this, "ShowForeignResourceInMirVer", false),
	m_bShowOSVersion(this, "ShowOSVersion", true),
	m_bShowTransport(this, "ShowTransport", true),
	m_bUseSSL(this, "UseSSL", false),
	m_bUseTLS(this, "UseTLS", true),
	m_bUseDomainLogin(this, "UseDomainLogin", false),
	m_bAcceptNotes(this, "AcceptNotes", true),
	m_bAutosaveNotes(this, "AutosaveNotes", false),
	m_bRcMarkMessagesAsRead(this, "RcMarkMessagesAsRead", 1),
	m_iConnectionKeepAliveInterval(this, "ConnectionKeepAliveInterval", 60000),
	m_iConnectionKeepAliveTimeout(this, "ConnectionKeepAliveTimeout", 50000),
	m_bProcessXMPPLinks(this, "ProcessXMPPLinks", false),
	m_bIgnoreRosterGroups(this, "IgnoreRosterGroups", false),
	m_bEnableCarbons(this, "EnableCarbons", true),
	m_bUseOMEMO(this, "UseOMEMO", false),
	m_bEnableStreamMgmt(this, "UseStreamMgmt", false)
{
	debugLogA("Setting protocol/module name to '%s'", m_szModuleName);

	// Protocol services and events...
	m_hEventNudge = CreateProtoEvent(PE_NUDGE);
	m_hEventXStatusIconChanged = CreateProtoEvent(JE_CUSTOMSTATUS_EXTRAICON_CHANGED);
	m_hEventXStatusChanged = CreateProtoEvent(JE_CUSTOMSTATUS_CHANGED);

	CreateProtoService(PS_CREATEACCMGRUI, &CJabberProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &CJabberProto::JabberGetAvatarInfo);
	CreateProtoService(PS_GETMYAWAYMSG, &CJabberProto::GetMyAwayMsg);
	CreateProtoService(PS_SET_LISTENINGTO, &CJabberProto::OnSetListeningTo);

	CreateProtoService(PS_JOINCHAT, &CJabberProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &CJabberProto::OnLeaveChat);

	CreateProtoService(PS_GETCUSTOMSTATUSEX, &CJabberProto::OnGetXStatusEx);
	CreateProtoService(PS_SETCUSTOMSTATUSEX, &CJabberProto::OnSetXStatusEx);
	CreateProtoService(PS_GETCUSTOMSTATUSICON, &CJabberProto::OnGetXStatusIcon);
	CreateProtoService(PS_GETADVANCEDSTATUSICON, &CJabberProto::JGetAdvancedStatusIcon);

	CreateProtoService(JS_HTTP_AUTH, &CJabberProto::OnHttpAuthRequest);
	CreateProtoService(JS_INCOMING_NOTE_EVENT, &CJabberProto::OnIncomingNoteEvent);

	CreateProtoService(JS_SENDXML, &CJabberProto::ServiceSendXML);
	CreateProtoService(PS_GETMYAVATAR, &CJabberProto::JabberGetAvatar);
	CreateProtoService(PS_GETAVATARCAPS, &CJabberProto::JabberGetAvatarCaps);
	CreateProtoService(PS_SETMYAVATAR, &CJabberProto::JabberSetAvatar);
	CreateProtoService(PS_SETMYNICKNAME, &CJabberProto::JabberSetNickname);

	CreateProtoService(PS_MENU_REQAUTH, &CJabberProto::OnMenuHandleRequestAuth);
	CreateProtoService(PS_MENU_GRANTAUTH, &CJabberProto::OnMenuHandleGrantAuth);
	CreateProtoService(PS_MENU_REVOKEAUTH, &CJabberProto::OnMenuHandleRevokeAuth);

	CreateProtoService(JS_DB_GETEVENTTEXT_CHATSTATES, &CJabberProto::OnGetEventTextChatStates);
	CreateProtoService(JS_DB_GETEVENTTEXT_PRESENCE, &CJabberProto::OnGetEventTextPresence);

	CreateProtoService(JS_GETJABBERAPI, &CJabberProto::JabberGetApi);

	// XEP-0224 support (Attention/Nudge)
	CreateProtoService(PS_SEND_NUDGE, &CJabberProto::JabberSendNudge);

	// service to get from protocol chat buddy info
	CreateProtoService(MS_GC_PROTO_GETTOOLTIPTEXT, &CJabberProto::JabberGCGetToolTipText);

	// XMPP URI parser service for "File Association Manager" plugin
	CreateProtoService(JS_PARSE_XMPP_URI, &CJabberProto::JabberServiceParseXmppURI);

	HookProtoEvent(ME_LANGPACK_CHANGED, &CJabberProto::OnLangChanged);
	HookProtoEvent(ME_OPT_INITIALISE, &CJabberProto::OnOptionsInit);
	HookProtoEvent(ME_SKIN_ICONSCHANGED, &CJabberProto::OnReloadIcons);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CJabberProto::OnDbSettingChanged);

	m_iqManager.FillPermanentHandlers();
	m_messageManager.FillPermanentHandlers();
	m_adhocManager.FillDefaultNodes();

	AddDefaultCaps();

	IconsInit();
	InitPopups();
	GlobalMenuInit();
	WsInit();

	m_pepServices.insert(new CPepMood(this));
	m_pepServices.insert(new CPepActivity(this));

	if (m_bUseOMEMO) {
		db_set_resident(m_szModuleName, "OmemoSessionChecked");
		OmemoInitDevice();
	}

	db_set_resident(m_szModuleName, DBSETTING_XSTATUSID);
	db_set_resident(m_szModuleName, DBSETTING_XSTATUSNAME);
	db_set_resident(m_szModuleName, DBSETTING_XSTATUSMSG);
	db_set_resident(m_szModuleName, DBSETTING_DISPLAY_UID);
	db_set_resident(m_szModuleName, "SubscriptionText");
	db_set_resident(m_szModuleName, "Subscription");
	db_set_resident(m_szModuleName, "Auth");
	db_set_resident(m_szModuleName, "Grant");

	if ((m_tszSelectedLang = getUStringA("XmlLang")) == nullptr)
		m_tszSelectedLang = mir_strdup("en");
}


CJabberProto::~CJabberProto()
{
	ConsoleUninit();
	GlobalMenuUninit();

	if (m_hPopupClass)
		Popup_UnregisterClass(m_hPopupClass);

	delete m_pInfoFrame;

	DestroyHookableEvent(m_hEventNudge);
	DestroyHookableEvent(m_hEventXStatusIconChanged);
	DestroyHookableEvent(m_hEventXStatusChanged);

	ListWipe();

	mir_free(m_tszSelectedLang);
	mir_free(m_AuthMechs.m_gssapiHostName);

	mir_free(m_modeMsgs.szOnline);
	mir_free(m_modeMsgs.szAway);
	mir_free(m_modeMsgs.szNa);
	mir_free(m_modeMsgs.szDnd);
	mir_free(m_modeMsgs.szFreechat);

	mir_free(m_transportProtoTableStartIndex);

	for (auto &it : m_lstTransports)
		mir_free(it);

	for (auto &it : m_lstJabberFeatCapPairsDynamic) {
		mir_free(it->szExt);
		mir_free(it->szFeature);
		mir_free(it->szDescription);
		delete it;
	}

}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

void CJabberProto::OnModulesLoaded()
{
	HookProtoEvent(ME_USERINFO_INITIALISE, &CJabberProto::OnUserInfoInit);
	XStatusInit();
	m_pepServices.InitGui();

	m_iqManager.Start();

	ConsoleInit();
	InitInfoFrame();

	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &CJabberProto::JabberGcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CJabberProto::JabberGcMenuHook);

	StatusIconData sid = {};
	sid.szModule = m_szModuleName;
	sid.hIcon = IcoLib_GetIconByHandle(m_hProtoIcon);
	sid.flags = MBF_HIDDEN;
	sid.szTooltip.a = LPGEN("Jabber Resource");
	Srmm_AddIcon(&sid, &g_plugin);

	HookProtoEvent(ME_MSG_ICONPRESSED, &CJabberProto::OnProcessSrmmIconClick);
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CJabberProto::OnProcessSrmmEvent);

	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.eventType = EVENTTYPE_JABBER_CHATSTATES;
	dbEventType.descr = "Chat state notifications";
	DbEvent_RegisterType(&dbEventType);

	dbEventType.eventType = EVENTTYPE_JABBER_PRESENCE;
	dbEventType.descr = "Presence notifications";
	DbEvent_RegisterType(&dbEventType);

	HookProtoEvent(ME_IDLE_CHANGED, &CJabberProto::OnIdleChanged);

	CheckAllContactsAreTransported();

	// Set all contacts to offline
	for (auto &hContact : AccContacts()) {
		SetContactOfflineStatus(hContact);

		if (getByte(hContact, "IsTransport", 0)) {
			ptrA jid(getUStringA(hContact, "jid"));
			if (jid == nullptr)
				continue;

			char *resourcepos = strchr(jid, '/');
			if (resourcepos != nullptr)
				*resourcepos = '\0';

			m_lstTransports.insert(mir_strdup(jid));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnPreShutdown - prepares Miranda to be shut down

void CJabberProto::OnShutdown()
{
	m_bShutdown = true;

	UI_SAFE_CLOSE_HWND(m_hwndAgentRegInput);
	UI_SAFE_CLOSE_HWND(m_hwndRegProgress);
	UI_SAFE_CLOSE_HWND(m_hwndJabberChangePassword);
	UI_SAFE_CLOSE_HWND(m_hwndJabberAddBookmark);
	UI_SAFE_CLOSE_HWND(m_hwndPrivacyRule);

	UI_SAFE_CLOSE(m_pDlgPrivacyLists);
	UI_SAFE_CLOSE(m_pDlgBookmarks);
	UI_SAFE_CLOSE(m_pDlgServiceDiscovery);
	UI_SAFE_CLOSE(m_pDlgJabberJoinGroupchat);
	UI_SAFE_CLOSE(m_pDlgNotes);

	MucShutdown();

	m_iqManager.ExpireAll();
	m_iqManager.Shutdown();
	ConsoleUninit();

	Srmm_RemoveIcon(m_szModuleName, 0);
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAddToList - adds a contact to the contact list

MCONTACT CJabberProto::AddToListByJID(const char *newJid, DWORD flags)
{
	debugLogA("AddToListByJID jid = %s", newJid);

	MCONTACT hContact = DBCreateContact(newJid, nullptr, true, false);
	if (flags & PALF_TEMPORARY)
		db_set_b(hContact, "CList", "Hidden", 1);

	return hContact;
}

MCONTACT CJabberProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT) && psr->id.w == nullptr)
		return 0;

	return AddToListByJID(T2Utf(psr->id.w), flags);
}

MCONTACT CJabberProto::AddToListByEvent(int flags, int /*iContact*/, MEVENT hDbEvent)
{
	debugLogA("AddToListByEvent");

	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return 0;
	if ((dbei.pBlob = (PBYTE)alloca(dbei.cbBlob)) == nullptr)
		return 0;
	if (db_event_get(hDbEvent, &dbei))
		return 0;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 0;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 0;

	DB_AUTH_BLOB blob(dbei.pBlob);
	return AddToListByJID(blob.get_email(), flags);
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthAllow - processes the successful authorization

int CJabberProto::Authorize(MEVENT hDbEvent)
{
	if (!m_bJabberOnline)
		return 1;

	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return 1;
	if ((dbei.pBlob = (PBYTE)alloca(dbei.cbBlob)) == nullptr)
		return 1;
	if (db_event_get(hDbEvent, &dbei))
		return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 1;

	DB_AUTH_BLOB blob(dbei.pBlob);
	debugLogA("Send 'authorization allowed' to %s", blob.get_email());

	m_ThreadInfo->send(XmlNode("presence") << XATTR("to", blob.get_email()) << XATTR("type", "subscribed"));

	// Automatically add this user to my roster if option is enabled
	if (m_bAutoAdd) {
		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, blob.get_email());
		if (item == nullptr || (item->subscription != SUB_BOTH && item->subscription != SUB_TO)) {
			debugLogA("Try adding contact automatically jid = %s", blob.get_email());
			if (MCONTACT hContact = AddToListByJID(blob.get_email(), 0)) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				db_unset(hContact, "CList", "NotOnList");
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthDeny - handles the unsuccessful authorization

int CJabberProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	if (!m_bJabberOnline)
		return 1;

	debugLogA("Entering AuthDeny");

	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return 1;

	mir_ptr<BYTE> pBlob((PBYTE)mir_alloc(dbei.cbBlob));
	if ((dbei.pBlob = pBlob) == nullptr)
		return 1;

	if (db_event_get(hDbEvent, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 1;

	char *nick = (char*)(dbei.pBlob + sizeof(DWORD) * 2);
	char *firstName = nick + mir_strlen(nick) + 1;
	char *lastName = firstName + mir_strlen(firstName) + 1;
	char *jid = lastName + mir_strlen(lastName) + 1;

	debugLogA("Send 'authorization denied' to %s", jid);

	ptrA newJid(dbei.flags & DBEF_UTF ? mir_strdup(jid) : mir_utf8encode(jid));
	m_ThreadInfo->send(XmlNode("presence") << XATTR("to", newJid) << XATTR("type", "unsubscribed"));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileAllow - starts a file transfer

HANDLE CJabberProto::FileAllow(MCONTACT /*hContact*/, HANDLE hTransfer, const wchar_t *szPath)
{
	if (!m_bJabberOnline)
		return nullptr;

	filetransfer *ft = (filetransfer*)hTransfer;
	ft->std.szWorkingDir.w = mir_wstrdup(szPath);
	size_t len = mir_wstrlen(ft->std.szWorkingDir.w) - 1;
	if (ft->std.szWorkingDir.w[len] == '/' || ft->std.szWorkingDir.w[len] == '\\')
		ft->std.szWorkingDir.w[len] = 0;

	switch (ft->type) {
	case FT_OOB:
		ForkThread((MyThreadFunc)&CJabberProto::FileReceiveThread, ft);
		break;
	case FT_BYTESTREAM:
		FtAcceptSiRequest(ft);
		break;
	case FT_IBB:
		FtAcceptIbbRequest(ft);
		break;
	}
	return hTransfer;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileCancel - cancels a file transfer

int CJabberProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	filetransfer *ft = (filetransfer*)hTransfer;
	HANDLE hEvent;

	debugLogA("Invoking FileCancel()");
	if (ft->type == FT_OOB) {
		if (ft->s) {
			debugLogA("FT canceled");
			debugLogA("Closing ft->s = %d", ft->s);
			ft->state = FT_ERROR;
			Netlib_CloseHandle(ft->s);
			ft->s = nullptr;
			if (ft->hFileEvent != nullptr) {
				hEvent = ft->hFileEvent;
				ft->hFileEvent = nullptr;
				SetEvent(hEvent);
			}
			debugLogA("ft->s is now nullptr, ft->state is now FT_ERROR");
		}
	}
	else FtCancel(ft);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileDeny - denies a file transfer

int CJabberProto::FileDeny(MCONTACT, HANDLE hTransfer, const wchar_t *)
{
	if (!m_bJabberOnline)
		return 1;

	filetransfer *ft = (filetransfer*)hTransfer;

	switch (ft->type) {
	case FT_OOB:
		m_ThreadInfo->send(XmlNodeIq("error", ft->szId, ft->jid) << XCHILD("error", "File transfer refused") << XATTRI("code", 406));
		break;

	case FT_BYTESTREAM:
	case FT_IBB:
		m_ThreadInfo->send(
			XmlNodeIq("error", ft->szId, ft->jid)
			<< XCHILD("error", "File transfer refused") << XATTRI("code", 403) << XATTR("type", "cancel")
			<< XCHILDNS("forbidden", "urn:ietf:params:xml:ns:xmpp-stanzas")
			<< XCHILD("text", "File transfer refused") << XATTR("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas"));
		break;
	}
	delete ft;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileResume - processes file renaming etc

int CJabberProto::FileResume(HANDLE hTransfer, int *action, const wchar_t **szFilename)
{
	filetransfer *ft = (filetransfer*)hTransfer;
	if (!m_bJabberOnline || ft == nullptr)
		return 1;

	if (*action == FILERESUME_RENAME)
		replaceStrW(ft->std.szCurrentFile.w, *szFilename);

	SetEvent(ft->hWaitEvent);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

INT_PTR CJabberProto::GetCaps(int type, MCONTACT hContact)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_SERVERCLIST | PF1_MODEMSG | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_FILE | PF1_CONTACT;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("JID");
	case PFLAG_MAXCONTACTSPERPACKET:
		char szClientJid[JABBER_MAX_JID_LEN];
		if (GetClientJID(hContact, szClientJid, _countof(szClientJid))) {
			JabberCapsBits jcb = GetResourceCapabilities(szClientJid);
			return ((~jcb & JABBER_CAPS_ROSTER_EXCHANGE) ? 0 : 50);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int CJabberProto::GetInfo(MCONTACT hContact, int /*infoType*/)
{
	if (!m_bJabberOnline || isChatRoom(hContact))
		return 1;

	ptrA jid(getUStringA(hContact, "jid"));
	if (!jid)
		return 1;

	char szBareJid[JABBER_MAX_JID_LEN];
	JabberStripJid(jid, szBareJid, _countof(szBareJid));

	if (m_ThreadInfo) {
		auto *pInfo = AddIQ(&CJabberProto::OnIqResultEntityTime, JABBER_IQ_TYPE_GET, jid);
		pInfo->SetParamsToParse(JABBER_IQ_PARSE_HCONTACT);
		m_ThreadInfo->send(XmlNodeIq(pInfo) << XCHILDNS("time", JABBER_FEAT_ENTITY_TIME));

		// XEP-0012, last logoff time
		pInfo = AddIQ(&CJabberProto::OnIqResultLastActivity, JABBER_IQ_TYPE_GET, jid);
		pInfo->SetParamsToParse(JABBER_IQ_PARSE_FROM);
		m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_LAST_ACTIVITY));

		JABBER_LIST_ITEM *item = nullptr;
		if ((item = ListGetItemPtr(LIST_VCARD_TEMP, jid)) == nullptr)
			item = ListGetItemPtr(LIST_ROSTER, jid);

		if (item == nullptr) {
			bool bHasResource = mir_strcmp(jid, szBareJid) != 0;
			JABBER_LIST_ITEM *tmpItem = nullptr;
			if (bHasResource && (tmpItem = ListGetItemPtr(LIST_CHATROOM, szBareJid))) {
				pResourceStatus him(tmpItem->findResource(szBareJid + mir_strlen(szBareJid) + 1));
				if (him) {
					item = ListAdd(LIST_VCARD_TEMP, jid, hContact);
					ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_szStatusMessage, him->m_iPriority);
				}
			}
			else item = ListAdd(LIST_VCARD_TEMP, jid, hContact);
		}

		if (item != nullptr) {
			if (item->arResources.getCount()) {
				for (auto &it : item->arResources) {
					pResourceStatus r(it);
					CMStringA tmp(MakeJid(szBareJid, r->m_szResourceName));

					if (r->m_jcbCachedCaps & JABBER_CAPS_DISCO_INFO) {
						pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, tmp);
						pInfo->SetParamsToParse(JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_HCONTACT);
						m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO));
					}

					if (mir_strcmp(tmp, jid)) { // skip current resource, we've already sent this iq to it
						pInfo = AddIQ(&CJabberProto::OnIqResultLastActivity, JABBER_IQ_TYPE_GET, tmp);
						pInfo->SetParamsToParse(JABBER_IQ_PARSE_FROM);
						m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_LAST_ACTIVITY));
					}
				}
			}
		}
	}

	SendGetVcard(jid);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by JID

struct JABBER_SEARCH_BASIC
{
	int hSearch;
	wchar_t jid[128];
};

void __cdecl CJabberProto::BasicSearchThread(JABBER_SEARCH_BASIC *jsb)
{
	Thread_SetName("Jabber: BasicSearchThread");
	Sleep(100);

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.nick.w = jsb->jid;
	psr.firstName.w = L"";
	psr.lastName.w = L"";
	psr.id.w = jsb->jid;

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)jsb->hSearch, (LPARAM)&psr);
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)jsb->hSearch, 0);
	mir_free(jsb);
}

HANDLE CJabberProto::SearchBasic(const wchar_t *szJid)
{
	debugLogA("JabberBasicSearch called with lParam = '%s'", szJid);

	JABBER_SEARCH_BASIC *jsb;
	if (!m_bJabberOnline || (jsb = (JABBER_SEARCH_BASIC*)mir_alloc(sizeof(JABBER_SEARCH_BASIC))) == nullptr)
		return nullptr;

	if (wcschr(szJid, '@') == nullptr) {
		wchar_t *szServer = mir_a2u(m_ThreadInfo->conn.server);
		const wchar_t *p = wcsstr(szJid, szServer);
		if (p == nullptr) {
			bool numericjid = true;
			for (const wchar_t *i = szJid; *i && numericjid; i++)
				numericjid = (*i >= '0') && (*i <= '9');

			mir_free(szServer);
			szServer = getWStringA(0, "LoginServer");
			if (szServer == nullptr)
				szServer = mir_wstrdup(L"jabber.org");
			else if (numericjid && !mir_wstrcmpi(szServer, L"S.ms")) {
				mir_free(szServer);
				szServer = mir_wstrdup(L"sms");
			}
			mir_snwprintf(jsb->jid, L"%s@%s", szJid, szServer);
		}
		else wcsncpy_s(jsb->jid, szJid, _TRUNCATE);
		mir_free(szServer);
	}
	else wcsncpy_s(jsb->jid, szJid, _TRUNCATE);

	debugLogA("Adding '%s' without validation", jsb->jid);
	jsb->hSearch = SerialNext();
	ForkThread((MyThreadFunc)&CJabberProto::BasicSearchThread, jsb);
	return (HANDLE)jsb->hSearch;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE CJabberProto::SearchByEmail(const wchar_t *email)
{
	if (!m_bJabberOnline || email == nullptr)
		return nullptr;

	ptrA szServerName(getStringA("Jud"));

	LPCSTR jid = szServerName == 0 ? "users.jabber.org" : szServerName;
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultSetSearch, JABBER_IQ_TYPE_SET, jid);
	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY("jabber:iq:search") << XCHILD("email", T2Utf(email)));
	return (HANDLE)pInfo->GetIqId();
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByName - searches the contact by its first or last name, or by a nickname

HANDLE CJabberProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	if (!m_bJabberOnline)
		return nullptr;

	BOOL bIsExtFormat = m_bExtendedSearch;

	ptrA szServerName(getStringA("Jud"));

	CJabberIqInfo *pInfo = AddIQ(
		(bIsExtFormat) ? &CJabberProto::OnIqResultExtSearch : &CJabberProto::OnIqResultSetSearch,
		JABBER_IQ_TYPE_SET, szServerName == 0 ? "users.jabber.org" : szServerName);
	XmlNodeIq iq(pInfo);
	TiXmlElement *query = iq << XQUERY("jabber:iq:search");

	if (bIsExtFormat) {
		if (m_tszSelectedLang)
			iq << XATTR("xml:lang", m_tszSelectedLang);

		TiXmlElement *x = query << XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit");
		if (nick[0] != '\0')
			x << XCHILD("field") << XATTR("var", "user") << XATTR("value", T2Utf(nick));

		if (firstName[0] != '\0')
			x << XCHILD("field") << XATTR("var", "fn") << XATTR("value", T2Utf(firstName));

		if (lastName[0] != '\0')
			x << XCHILD("field") << XATTR("var", "given") << XATTR("value", T2Utf(lastName));
	}
	else {
		if (nick[0] != '\0')
			query << XCHILD("nick", T2Utf(nick));

		if (firstName[0] != '\0')
			query << XCHILD("first", T2Utf(firstName));

		if (lastName[0] != '\0')
			query << XCHILD("last", T2Utf(lastName));
	}

	m_ThreadInfo->send(iq);
	return (HANDLE)pInfo->GetIqId();
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int CJabberProto::SendContacts(MCONTACT hContact, int, int nContacts, MCONTACT *hContactsList)
{
	if (!m_bJabberOnline)
		return 0;

	char szClientJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, szClientJid, _countof(szClientJid)))
		return 0;

	JabberCapsBits jcb = GetResourceCapabilities(szClientJid);
	if (~jcb & JABBER_CAPS_ROSTER_EXCHANGE)
		return 0;

	XmlNode m("message");
	TiXmlElement *x = m << XCHILDNS("x", JABBER_FEAT_ROSTER_EXCHANGE);

	for (int i = 0; i < nContacts; i++) {
		ptrA jid(getUStringA(hContactsList[i], "jid"));
		if (jid != nullptr)
			x << XCHILD("item") << XATTR("action", "add") << XATTR("jid", jid);
	}

	m << XATTR("to", szClientJid) << XATTRID(SerialNext());
	m_ThreadInfo->send(m);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE CJabberProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t** ppszFiles)
{
	if (!m_bJabberOnline) return nullptr;

	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
		return nullptr;

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid == nullptr)
		return nullptr;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return nullptr;

	// Check if another file transfer session request is pending (waiting for disco result)
	if (item->ft != nullptr)
		return nullptr;

	JabberCapsBits jcb = GetResourceCapabilities(item->jid);
	if (jcb == JABBER_RESOURCE_CAPS_IN_PROGRESS) {
		Sleep(600);
		jcb = GetResourceCapabilities(item->jid);
	}

	// fix for very smart clients, like gajim
	if (!m_bBsDirect && !m_bBsProxyManual) {
		// disable bytestreams
		jcb &= ~JABBER_CAPS_BYTESTREAMS;
	}

	// if only JABBER_CAPS_SI_FT feature set (without BS or IBB), disable JABBER_CAPS_SI_FT
	if ((jcb & (JABBER_CAPS_SI_FT | JABBER_CAPS_IBB | JABBER_CAPS_BYTESTREAMS)) == JABBER_CAPS_SI_FT)
		jcb &= ~JABBER_CAPS_SI_FT;

	if (
		// can't get caps
		(jcb & JABBER_RESOURCE_CAPS_ERROR)
		// caps not already received
		|| (jcb == JABBER_RESOURCE_CAPS_NONE)
		// XEP-0096 and OOB not supported?
		|| !(jcb & (JABBER_CAPS_SI_FT | JABBER_CAPS_OOB))) {
		MsgPopup(hContact, TranslateT("No compatible file transfer mechanism exists"), Utf2T(item->jid));
		return nullptr;
	}

	filetransfer *ft = new filetransfer(this);
	ft->std.hContact = hContact;
	while (ppszFiles[ft->std.totalFiles] != nullptr)
		ft->std.totalFiles++;

	ft->std.pszFiles.w = (wchar_t**)mir_calloc(sizeof(wchar_t*)* ft->std.totalFiles);
	ft->fileSize = (unsigned __int64*)mir_calloc(sizeof(unsigned __int64)* ft->std.totalFiles);

	int i, j;
	for (i = j = 0; i < ft->std.totalFiles; i++) {
		struct _stati64 statbuf;
		if (_wstat64(ppszFiles[i], &statbuf))
			debugLogW(L"'%s' is an invalid filename", ppszFiles[i]);
		else {
			ft->std.pszFiles.w[j] = mir_wstrdup(ppszFiles[i]);
			ft->fileSize[j] = statbuf.st_size;
			j++;
			ft->std.totalBytes += statbuf.st_size;
		}
	}
	if (j == 0) {
		delete ft;
		return nullptr;
	}

	ft->std.szCurrentFile.w = mir_wstrdup(ppszFiles[0]);
	ft->szDescription = mir_wstrdup(szDescription);
	ft->jid = mir_strdup(jid);

	if (jcb & JABBER_CAPS_SI_FT)
		FtInitiate(item->jid, ft);
	else if (jcb & JABBER_CAPS_OOB)
		ForkThread((MyThreadFunc)&CJabberProto::FileServerThread, ft);

	return ft;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSendMessage - sends a message

struct TFakeAckParams
{
	inline TFakeAckParams(MCONTACT _hContact, const wchar_t *_msg, int _msgid = 0)
		: hContact(_hContact), msg(_msg), msgid(_msgid)
	{
	}

	MCONTACT hContact;
	const wchar_t *msg;
	int msgid;
};

void __cdecl CJabberProto::SendMessageAckThread(void* param)
{
	Thread_SetName("Jabber: SendMessageAckThread");
	TFakeAckParams *par = (TFakeAckParams*)param;
	Sleep(100);
	debugLogA("Broadcast ACK");
	ProtoBroadcastAck(par->hContact, ACKTYPE_MESSAGE, par->msg ? ACKRESULT_FAILED : ACKRESULT_SUCCESS, (HANDLE)par->msgid, (LPARAM)par->msg);
	debugLogA("Returning from thread");
	delete par;
}

static char PGP_PROLOG[] = "-----BEGIN PGP MESSAGE-----\r\n\r\n";
static char PGP_EPILOG[] = "\r\n-----END PGP MESSAGE-----\r\n";

int CJabberProto::SendMsg(MCONTACT hContact, int unused_unknown, const char *pszSrc)
{
	char szClientJid[JABBER_MAX_JID_LEN];
	if (!m_bJabberOnline || !GetClientJID(hContact, szClientJid, _countof(szClientJid))) {
		TFakeAckParams *param = new TFakeAckParams(hContact, TranslateT("Protocol is offline or no JID"));
		ForkThread(&CJabberProto::SendMessageAckThread, param);
		return 1;
	}

	if (m_bUseOMEMO) {
		if (!OmemoCheckSession(hContact)) {
			OmemoPutMessageToOutgoingQueue(hContact, unused_unknown, pszSrc);
			int id = SerialNext();
			TFakeAckParams *param = new TFakeAckParams(hContact, nullptr, id);
			ForkThread(&CJabberProto::SendMessageAckThread, param);
			return id;
		}
	}

	int  isEncrypted, id = SerialNext();
	if (!strncmp(pszSrc, PGP_PROLOG, mir_strlen(PGP_PROLOG))) {
		const char *szEnd = strstr(pszSrc, PGP_EPILOG);
		char *tempstring = (char*)alloca(mir_strlen(pszSrc) + 2);
		size_t nStrippedLength = mir_strlen(pszSrc) - mir_strlen(PGP_PROLOG) - (szEnd ? mir_strlen(szEnd) : 0) + 1;
		strncpy_s(tempstring, nStrippedLength, pszSrc + mir_strlen(PGP_PROLOG), _TRUNCATE);
		tempstring[nStrippedLength] = 0;
		pszSrc = tempstring;
		isEncrypted = 1;
	}
	else isEncrypted = 0;

	char *msgType;
	if (ListGetItemPtr(LIST_CHATROOM, szClientJid) && strchr(szClientJid, '/') == nullptr)
		msgType = "groupchat";
	else
		msgType = "chat";
	XmlNode m("message");

	// omemo enabled in options, omemo enabled for contact
	if (m_bUseOMEMO && OmemoIsEnabled(hContact) && !mir_strcmp(msgType, "chat")) {
		// TODO: check if message encrypted for at least one session and return error if not
		if (!OmemoEncryptMessage(m, pszSrc, hContact)) {
			TFakeAckParams *param = new TFakeAckParams(hContact, TranslateT("No valid OMEMO session exists"));
			ForkThread(&CJabberProto::SendMessageAckThread, param);
			return 0;
		}
	}
	else {
		XmlAddAttr(m, "type", msgType);
		if (!isEncrypted)
			m << XCHILD("body", pszSrc);
		else {
			m << XCHILD("body", "[This message is encrypted.]");
			m << XCHILD("x", pszSrc) << XATTR("xmlns", "jabber:x:encrypted");
		}
	}

	pResourceStatus r(ResourceInfoFromJID(szClientJid));
	if (r)
		r->m_bMessageSessionActive = true;

	JabberCapsBits jcb = GetResourceCapabilities(szClientJid);

	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	if (jcb & JABBER_CAPS_CHATSTATES)
		m << XCHILDNS("active", JABBER_FEAT_CHATSTATES);

	m << XATTR("to", szClientJid);

	if (
		// if message delivery check disabled by entity caps manager
		(jcb & JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY) ||
		// if client knows nothing about delivery
		!(jcb & (JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_RECEIPTS)) ||
		// if message sent to groupchat
		!mir_strcmp(msgType, "groupchat") ||
		// if message delivery check disabled in settings
		!m_bMsgAck || !getByte(hContact, "MsgAck", true))
	{
		if (mir_strcmp(msgType, "groupchat")) {
			id = SerialNext();
			XmlAddAttrID(m, id);
		}
		m_ThreadInfo->send(m);

		ForkThread(&CJabberProto::SendMessageAckThread, new TFakeAckParams(hContact, nullptr, id));
	}
	else {
		XmlAddAttrID(m, id);

		// message receipts XEP priority
		if (jcb & JABBER_CAPS_MESSAGE_RECEIPTS)
			m << XCHILDNS("request", JABBER_FEAT_MESSAGE_RECEIPTS);
		else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
			TiXmlElement *x = m << XCHILDNS("x", JABBER_FEAT_MESSAGE_EVENTS);
			x << XCHILD("delivered"); x << XCHILD("offline");
		}

		m_ThreadInfo->send(m);
	}
	return id;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetApparentMode - sets the visibility status

int CJabberProto::SetApparentMode(MCONTACT hContact, int mode)
{
	if (mode != 0 && mode != ID_STATUS_ONLINE && mode != ID_STATUS_OFFLINE)
		return 1;

	int oldMode = getWord(hContact, "ApparentMode", 0);
	if (mode == oldMode)
		return 1;

	setWord(hContact, "ApparentMode", (WORD)mode);
	if (!m_bJabberOnline)
		return 0;

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid == nullptr)
		return 0;

	switch (mode) {
	case ID_STATUS_ONLINE:
		if (m_iStatus == ID_STATUS_INVISIBLE || oldMode == ID_STATUS_OFFLINE)
			m_ThreadInfo->send(XmlNode("presence") << XATTR("to", jid));
		break;
	case ID_STATUS_OFFLINE:
		if (m_iStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE)
			SendPresenceTo(ID_STATUS_INVISIBLE, jid);
		break;
	case 0:
		if (oldMode == ID_STATUS_ONLINE && m_iStatus == ID_STATUS_INVISIBLE)
			SendPresenceTo(ID_STATUS_INVISIBLE, jid);
		else if (oldMode == ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_INVISIBLE)
			SendPresenceTo(m_iStatus, jid);
		break;
	}

	// TODO: update the zebra list (jabber:iq:privacy)
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetStatus - sets the protocol status

int CJabberProto::SetStatus(int iNewStatus)
{
	debugLogA("PS_SETSTATUS(%d,%d) => %d", m_iStatus, m_iDesiredStatus, iNewStatus);

	if (m_iDesiredStatus == iNewStatus)
		return 0;

	int oldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_StrmMgmt.ResetState();
		if (m_ThreadInfo) {
			if (m_bEnableStreamMgmt)
				m_StrmMgmt.SendAck();
			m_ThreadInfo->send("</stream:stream>");
			m_ThreadInfo->shutdown();
			RebuildInfoFrame();
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (!m_ThreadInfo && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		ForkThread((MyThreadFunc)&CJabberProto::ServerThread, nullptr);

		RebuildInfoFrame();
	}
	else if (m_bJabberOnline)
		SetServerStatus(iNewStatus);
	else
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAwayMsg - returns a contact's away message

void __cdecl CJabberProto::GetAwayMsgThread(void *param)
{
	Thread_SetName("Jabber: GetAwayMsgThread");

	MCONTACT hContact = (DWORD_PTR)param;

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid != nullptr) {
		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
		if (item != nullptr) {
			if (item->arResources.getCount() > 0) {
				int nRes = 0;
				CMStringA str, strLast;
				for (auto &r : item->arResources)
					if (r->m_szStatusMessage) {
						nRes++;
						strLast = r->m_szStatusMessage;
						str.AppendFormat("(%s): %s\r\n", r->m_szResourceName, r->m_szStatusMessage);
					}

				if (nRes == 1)
					str = strLast;

				str.TrimRight();
				ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, Utf2T(str));
				return;
			}

			Utf2T tszStatusMsg(item->getTemp()->m_szStatusMessage);
			if (tszStatusMsg != nullptr) {
				ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, tszStatusMsg);
				return;
			}
		}
	}

	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

HANDLE CJabberProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CJabberProto::GetAwayMsgThread, (void*)hContact);
	return (HANDLE)1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetAwayMsg - sets the away status message

int CJabberProto::SetAwayMsg(int status, const wchar_t *msg)
{
	char **szMsg;
	mir_cslockfull lck(m_csModeMsgMutex);

	switch (status) {
	case ID_STATUS_ONLINE:
		szMsg = &m_modeMsgs.szOnline;
		break;

	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		szMsg = &m_modeMsgs.szAway;
		status = ID_STATUS_AWAY;
		break;

	case ID_STATUS_NA:
		szMsg = &m_modeMsgs.szNa;
		break;

	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		szMsg = &m_modeMsgs.szDnd;
		status = ID_STATUS_DND;
		break;

	case ID_STATUS_FREECHAT:
		szMsg = &m_modeMsgs.szFreechat;
		break;

	default:
		return 1;
	}

	T2Utf szNewMsg(msg);
	if ((*szMsg == nullptr && msg == nullptr) || (*szMsg != nullptr && msg != nullptr && !mir_strcmp(*szMsg, szNewMsg)))
		return 0; // Message is the same, no update needed

	// Update with the new mode message
	replaceStr(*szMsg, szNewMsg);

	// Send a presence update if needed
	lck.unlock();
	if (status == m_iStatus)
		SendPresence(m_iStatus, true);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserIsTyping - sends a UTN notification

int CJabberProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (!m_bJabberOnline) return 0;

	char szClientJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, szClientJid, _countof(szClientJid)))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, szClientJid);
	if (item == nullptr)
		return 0;

	JabberCapsBits jcb = GetResourceCapabilities(szClientJid);
	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	XmlNode m("message"); XmlAddAttr(m, "to", szClientJid);

	if (jcb & JABBER_CAPS_CHATSTATES) {
		m << XATTR("type", "chat") << XATTRID(SerialNext());
		switch (type) {
		case PROTOTYPE_SELFTYPING_OFF:
			m << XCHILDNS("paused", JABBER_FEAT_CHATSTATES);
			m_ThreadInfo->send(m);
			break;
		case PROTOTYPE_SELFTYPING_ON:
			m << XCHILDNS("composing", JABBER_FEAT_CHATSTATES);
			m_ThreadInfo->send(m);
			break;
		}
	}
	else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
		TiXmlElement *x = m << XCHILDNS("x", JABBER_FEAT_MESSAGE_EVENTS);
		if (item->messageEventIdStr != nullptr)
			x << XCHILD("id", item->messageEventIdStr);

		switch (type) {
		case PROTOTYPE_SELFTYPING_OFF:
			m_ThreadInfo->send(m);
			break;
		case PROTOTYPE_SELFTYPING_ON:
			x << XCHILD("composing");
			m_ThreadInfo->send(m);
			break;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// InfoFrame events

void CJabberProto::InfoFrame_OnSetup(CJabberInfoFrame_Event*)
{
	OnMenuOptions(0, 0);
}

void CJabberProto::InfoFrame_OnTransport(CJabberInfoFrame_Event *evt)
{
	if (evt->m_event == CJabberInfoFrame_Event::CLICK) {
		MCONTACT hContact = (MCONTACT)evt->m_pUserData;
		HMENU hContactMenu = Menu_BuildContactMenu(hContact);
		POINT pt;
		GetCursorPos(&pt);
		int res = TrackPopupMenu(hContactMenu, TPM_RETURNCMD, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
		Clist_MenuProcessCommand(res, MPCF_CONTACTMENU, hContact);
	}
}
