/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-14  Miranda NG project

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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_disco.h"

#pragma warning(disable:4355)

static int compareTransports(const TCHAR *p1, const TCHAR *p2)
{	return _tcsicmp(p1, p2);
}

static int compareListItems(const JABBER_LIST_ITEM *p1, const JABBER_LIST_ITEM *p2)
{
	if (p1->list != p2->list)
		return p1->list - p2->list;

	// for bookmarks, temporary contacts & groupchat members
	// resource must be used in the comparison
	if ((p1->list == LIST_ROSTER && (p1->bUseResource == TRUE || p2->bUseResource == TRUE))
		|| (p1->list == LIST_BOOKMARK) || (p1->list == LIST_VCARD_TEMP))
		return lstrcmpi(p1->jid, p2->jid);

	TCHAR szp1[JABBER_MAX_JID_LEN], szp2[JABBER_MAX_JID_LEN];
	JabberStripJid(p1->jid, szp1, SIZEOF(szp1));
	JabberStripJid(p2->jid, szp2, SIZEOF(szp2));
	return lstrcmpi(szp1, szp2);
}

CJabberProto::CJabberProto(const char *aProtoName, const TCHAR *aUserName) :
	PROTO<CJabberProto>(aProtoName, aUserName),
	m_options(this),
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
	m_hPrivacyMenuRoot(0),
	m_hPrivacyMenuItems(10),
	m_lstJabberFeatCapPairsDynamic(2),
	m_uEnabledFeatCapsDynamic(0)
{
	m_szXmlStreamToBeInitialized = NULL;

	debugLogA("Setting protocol/module name to '%s'", m_szModuleName);

	// Jabber dialog list
	m_windowList = WindowList_Create();

	// Protocol services and events...
	m_hEventNudge = CreateProtoEvent(PE_NUDGE);
	m_hEventXStatusIconChanged = CreateProtoEvent(JE_CUSTOMSTATUS_EXTRAICON_CHANGED);
	m_hEventXStatusChanged = CreateProtoEvent(JE_CUSTOMSTATUS_CHANGED);

	CreateProtoService(PS_CREATEACCMGRUI, &CJabberProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFOT, &CJabberProto::JabberGetAvatarInfo);
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
	CreateProtoService(PS_GETMYAVATART, &CJabberProto::JabberGetAvatar);
	CreateProtoService(PS_GETAVATARCAPS, &CJabberProto::JabberGetAvatarCaps);
	CreateProtoService(PS_SETMYAVATART, &CJabberProto::JabberSetAvatar);
	CreateProtoService(PS_SETMYNICKNAME, &CJabberProto::JabberSetNickname);

	CreateProtoService(JS_DB_GETEVENTTEXT_CHATSTATES, &CJabberProto::OnGetEventTextChatStates);
	CreateProtoService(JS_DB_GETEVENTTEXT_PRESENCE, &CJabberProto::OnGetEventTextPresence);

	CreateProtoService(JS_GETJABBERAPI, &CJabberProto::JabberGetApi);

	// XEP-0224 support (Attention/Nudge)
	CreateProtoService(PS_SEND_NUDGE, &CJabberProto::JabberSendNudge);

	// service to get from protocol chat buddy info
	CreateProtoService(MS_GC_PROTO_GETTOOLTIPTEXT, &CJabberProto::JabberGCGetToolTipText);

	// XMPP URI parser service for "File Association Manager" plugin
	CreateProtoService(JS_PARSE_XMPP_URI, &CJabberProto::JabberServiceParseXmppURI);

	HookProtoEvent(ME_MODERNOPT_INITIALIZE, &CJabberProto::OnModernOptInit);
	HookProtoEvent(ME_OPT_INITIALISE, &CJabberProto::OnOptionsInit);
	HookProtoEvent(ME_SKIN2_ICONSCHANGED, &CJabberProto::OnReloadIcons);

	m_iqManager.FillPermanentHandlers();
	m_iqManager.Start();
	m_messageManager.FillPermanentHandlers();
	m_adhocManager.FillDefaultNodes();
	m_clientCapsManager.AddDefaultCaps();

	IconsInit();
	InitPopups();
	GlobalMenuInit();
	WsInit();
	ConsoleInit();

	m_pepServices.insert(new CPepMood(this));
	m_pepServices.insert(new CPepActivity(this));

	db_set_resident(m_szModuleName, "Status");
	db_set_resident(m_szModuleName, DBSETTING_DISPLAY_UID);

	db_set_resident(m_szModuleName, "SubscriptionText");
	db_set_resident(m_szModuleName, "Subscription");
	db_set_resident(m_szModuleName, "Auth");
	db_set_resident(m_szModuleName, "Grant");

	if ((m_tszSelectedLang = getTStringA("XmlLang")) == NULL)
		m_tszSelectedLang = mir_tstrdup(_T("en"));
}

CJabberProto::~CJabberProto()
{
	WsUninit();
	ConsoleUninit();
	GlobalMenuUninit();

	if (m_hPopupClass)
		Popup_UnregisterClass(m_hPopupClass);

	delete m_pInfoFrame;

	WindowList_Destroy(m_windowList);

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

	mir_free(m_szStreamId);

	for (int i=0; i < m_lstTransports.getCount(); i++)
		mir_free(m_lstTransports[i]);

	for (int i = 0; i < m_lstJabberFeatCapPairsDynamic.getCount(); i++) {
		mir_free(m_lstJabberFeatCapPairsDynamic[i]->szExt);
		mir_free(m_lstJabberFeatCapPairsDynamic[i]->szFeature);
		mir_free(m_lstJabberFeatCapPairsDynamic[i]->szDescription);
		delete m_lstJabberFeatCapPairsDynamic[i];
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

static COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

int CJabberProto::OnModulesLoadedEx(WPARAM, LPARAM)
{
	HookProtoEvent(ME_USERINFO_INITIALISE, &CJabberProto::OnUserInfoInit);
	XStatusInit();
	m_pepServices.InitGui();

	m_pInfoFrame = new CJabberInfoFrame(this);

	GCREGISTER gcr = { sizeof(gcr) };
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.nColors = 16;
	gcr.pColors = &crCols[0];
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, NULL, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CJabberProto::JabberGcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CJabberProto::JabberGcMenuHook);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = m_szModuleName;
	sid.hIcon = LoadIconEx("main");
	sid.flags = MBF_HIDDEN;
	sid.szTooltip = LPGEN("Jabber Resource");
	Srmm_AddIcon(&sid);

	HookProtoEvent(ME_MSG_ICONPRESSED, &CJabberProto::OnProcessSrmmIconClick);
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CJabberProto::OnProcessSrmmEvent);

	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.eventType = EVENTTYPE_JABBER_CHATSTATES;
	dbEventType.descr = "Chat state notifications";
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = EVENTTYPE_JABBER_PRESENCE;
	dbEventType.descr = "Presence notifications";
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	HookProtoEvent(ME_IDLE_CHANGED, &CJabberProto::OnIdleChanged);

	CheckAllContactsAreTransported();
	ConvertPasswords();

	// Set all contacts to offline
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		SetContactOfflineStatus(hContact);

		if (!getByte(hContact, "IsTransport", 0))
			continue;

		ptrT jid( getTStringA(hContact, "jid"));
		if (jid == NULL) continue;

		TCHAR *domain = NEWTSTR_ALLOCA(jid);
		TCHAR *resourcepos = _tcschr(domain, '/');
		if (resourcepos != NULL)
			*resourcepos = '\0';
		m_lstTransports.insert( mir_tstrdup(domain));
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnPreShutdown - prepares Miranda to be shut down

int __cdecl CJabberProto::OnPreShutdown(WPARAM, LPARAM)
{
	m_bShutdown = true;

	UI_SAFE_CLOSE_HWND(m_hwndAgentRegInput);
	UI_SAFE_CLOSE_HWND(m_hwndRegProgress);
	UI_SAFE_CLOSE_HWND(m_hwndMucVoiceList);
	UI_SAFE_CLOSE_HWND(m_hwndMucMemberList);
	UI_SAFE_CLOSE_HWND(m_hwndMucModeratorList);
	UI_SAFE_CLOSE_HWND(m_hwndMucBanList);
	UI_SAFE_CLOSE_HWND(m_hwndMucAdminList);
	UI_SAFE_CLOSE_HWND(m_hwndMucOwnerList);
	UI_SAFE_CLOSE_HWND(m_hwndJabberChangePassword);
	UI_SAFE_CLOSE_HWND(m_hwndJabberAddBookmark);
	UI_SAFE_CLOSE_HWND(m_hwndPrivacyRule);

	UI_SAFE_CLOSE(m_pDlgPrivacyLists);
	UI_SAFE_CLOSE(m_pDlgBookmarks);
	UI_SAFE_CLOSE(m_pDlgServiceDiscovery);
	UI_SAFE_CLOSE(m_pDlgJabberJoinGroupchat);
	UI_SAFE_CLOSE(m_pDlgNotes);

	m_iqManager.ExpireAll();
	m_iqManager.Shutdown();
	ConsoleUninit();

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = m_szModuleName;
	Srmm_RemoveIcon(&sid);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAddToList - adds a contact to the contact list

MCONTACT CJabberProto::AddToListByJID(const TCHAR *newJid, DWORD flags)
{
	MCONTACT hContact;
	TCHAR *jid, *nick;

	debugLog(_T("AddToListByJID jid = %s"), newJid);

	if ((hContact=HContactFromJID(newJid)) == NULL) {
		// not already there: add
		jid = mir_tstrdup(newJid);
		debugLog(_T("Add new jid to contact jid = %s"), jid);
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);
		setTString(hContact, "jid", jid);
		if ((nick=JabberNickFromJID(newJid)) == NULL)
			nick = mir_tstrdup(newJid);

		mir_free(nick);
		mir_free(jid);

		// Note that by removing or disable the "NotOnList" will trigger
		// the plugin to add a particular contact to the roster list.
		// See DBSettingChanged hook at the bottom part of this source file.
		// But the add module will delete "NotOnList". So we will not do it here.
		// Also because we need "MyHandle" and "Group" info, which are set after
		// PS_ADDTOLIST is called but before the add dialog issue deletion of
		// "NotOnList".
		// If temporary add, "NotOnList" won't be deleted, and that's expected.
		db_set_b(hContact, "CList", "NotOnList", 1);
		if (flags & PALF_TEMPORARY)
			db_set_b(hContact, "CList", "Hidden", 1);
	}
	else {
		// already exist
		// Set up a dummy "NotOnList" when adding permanently only
		if (!(flags & PALF_TEMPORARY))
			db_set_b(hContact, "CList", "NotOnList", 1);
	}

	if (hContact && newJid)
		DBCheckIsTransportedContact(newJid, hContact);
	return hContact;
}

MCONTACT CJabberProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->cbSize != sizeof(JABBER_SEARCH_RESULT) && psr->id == NULL)
		return NULL;

	JABBER_SEARCH_RESULT *jsr = (JABBER_SEARCH_RESULT*)psr;
	TCHAR *jid = psr->id ? psr->id : jsr->jid;
	return AddToListByJID(jid, flags);
}

MCONTACT __cdecl CJabberProto::AddToListByEvent(int flags, int /*iContact*/, HANDLE hDbEvent)
{
	debugLogA("AddToListByEvent");

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return NULL;
	if ((dbei.pBlob=(PBYTE)alloca(dbei.cbBlob)) == NULL)
		return NULL;
	if (db_event_get(hDbEvent, &dbei))
		return NULL;
	if (strcmp(dbei.szModule, m_szModuleName))
		return NULL;

/*
	// EVENTTYPE_CONTACTS is when adding from when we receive contact list (not used in Jabber)
	// EVENTTYPE_ADDED is when adding from when we receive "You are added" (also not used in Jabber)
	// Jabber will only handle the case of EVENTTYPE_AUTHREQUEST
	// EVENTTYPE_AUTHREQUEST is when adding from the authorization request dialog
*/

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return NULL;

	char *nick = (char*)(dbei.pBlob + sizeof(DWORD)*2);
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	TCHAR *newJid = (dbei.flags & DBEF_UTF) ? mir_utf8decodeT(jid) : mir_a2t(jid);
	MCONTACT hContact = (MCONTACT)AddToListByJID(newJid, flags);
	mir_free(newJid);
	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthAllow - processes the successful authorization

int CJabberProto::Authorize(HANDLE hDbEvent)
{
	if (!m_bJabberOnline)
		return 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return 1;
	if ((dbei.pBlob = (PBYTE)alloca(dbei.cbBlob)) == NULL)
		return 1;
	if (db_event_get(hDbEvent, &dbei))
		return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;
	if (strcmp(dbei.szModule, m_szModuleName))
		return 1;

	char *nick = (char*)(dbei.pBlob + sizeof(DWORD)*2);
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	debugLog(_T("Send 'authorization allowed' to %s"), jid);

	TCHAR *newJid = (dbei.flags & DBEF_UTF) ? mir_utf8decodeT(jid) : mir_a2t(jid);

	m_ThreadInfo->send(XmlNode(_T("presence")) << XATTR(_T("to"), newJid) << XATTR(_T("type"), _T("subscribed")));

	// Automatically add this user to my roster if option is enabled
	if (m_options.AutoAdd == TRUE) {
		JABBER_LIST_ITEM *item;

		if ((item = ListGetItemPtr(LIST_ROSTER, newJid)) == NULL || (item->subscription != SUB_BOTH && item->subscription != SUB_TO)) {
			debugLog(_T("Try adding contact automatically jid = %s"), jid);
			if (MCONTACT hContact = AddToListByJID(newJid, 0)) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				db_unset(hContact, "CList", "NotOnList");
	}	}	}

	mir_free(newJid);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthDeny - handles the unsuccessful authorization

int CJabberProto::AuthDeny(HANDLE hDbEvent, const TCHAR*)
{
	if (!m_bJabberOnline)
		return 1;

	debugLogA("Entering AuthDeny");

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return 1;

	mir_ptr<BYTE> pBlob((PBYTE) mir_alloc(dbei.cbBlob));
	if ((dbei.pBlob = pBlob) == NULL)
		return 1;

	if (db_event_get(hDbEvent, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if (strcmp(dbei.szModule, m_szModuleName))
		return 1;

	char *nick = (char*)(dbei.pBlob + sizeof(DWORD)*2);
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	debugLogA("Send 'authorization denied' to %s", jid);

	ptrT newJid(dbei.flags & DBEF_UTF ? mir_utf8decodeT(jid) : mir_a2t(jid));
	m_ThreadInfo->send( XmlNode(_T("presence")) << XATTR(_T("to"), newJid) << XATTR(_T("type"), _T("unsubscribed")));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int __cdecl CJabberProto::AuthRecv(MCONTACT, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CJabberProto::AuthRequest(MCONTACT, const TCHAR*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileAllow - starts a file transfer

HANDLE __cdecl CJabberProto::FileAllow(MCONTACT /*hContact*/, HANDLE hTransfer, const TCHAR *szPath)
{
	if (!m_bJabberOnline)
		return 0;

	filetransfer *ft = (filetransfer*)hTransfer;
	ft->std.tszWorkingDir = mir_tstrdup(szPath);
	size_t len = _tcslen(ft->std.tszWorkingDir)-1;
	if (ft->std.tszWorkingDir[len] == '/' || ft->std.tszWorkingDir[len] == '\\')
		ft->std.tszWorkingDir[len] = 0;

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

int __cdecl CJabberProto::FileCancel(MCONTACT, HANDLE hTransfer)
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
			ft->s = NULL;
			if (ft->hFileEvent != NULL) {
				hEvent = ft->hFileEvent;
				ft->hFileEvent = NULL;
				SetEvent(hEvent);
			}
			debugLogA("ft->s is now NULL, ft->state is now FT_ERROR");
		}
	}
	else FtCancel(ft);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileDeny - denies a file transfer

int __cdecl CJabberProto::FileDeny(MCONTACT, HANDLE hTransfer, const TCHAR *)
{
	if (!m_bJabberOnline)
		return 1;

	filetransfer *ft = (filetransfer*)hTransfer;

	switch (ft->type) {
	case FT_OOB:
		m_ThreadInfo->send(XmlNodeIq(_T("error"), ft->szId, ft->jid) << XCHILD(_T("error"), _T("File transfer refused")) << XATTRI(_T("code"), 406));
		break;

	case FT_BYTESTREAM:
	case FT_IBB:
		m_ThreadInfo->send(
			XmlNodeIq(_T("error"), ft->szId, ft->jid)
				<< XCHILD(_T("error"), _T("File transfer refused")) << XATTRI(_T("code"), 403) << XATTR(_T("type"), _T("cancel"))
					<< XCHILDNS(_T("forbidden"), _T("urn:ietf:params:xml:ns:xmpp-stanzas"))
						<< XCHILD(_T("text"), _T("File transfer refused")) << XATTR(_T("xmlns"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
		break;
	}
	delete ft;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileResume - processes file renaming etc

int __cdecl CJabberProto::FileResume(HANDLE hTransfer, int *action, const TCHAR **szFilename)
{
	filetransfer *ft = (filetransfer*)hTransfer;
	if (!m_bJabberOnline || ft == NULL)
		return 1;

	if (*action == FILERESUME_RENAME)
		replaceStrT(ft->std.tszCurrentFile, *szFilename);

	SetEvent(ft->hWaitEvent);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

DWORD_PTR __cdecl CJabberProto::GetCaps(int type, MCONTACT hContact)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_SERVERCLIST | PF1_MODEMSG | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_FILE | PF1_CONTACT;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDUTF | PF4_FORCEADDED;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("JID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"jid";
	case PFLAG_MAXCONTACTSPERPACKET:
		TCHAR szClientJid[JABBER_MAX_JID_LEN];
		if (GetClientJID(hContact, szClientJid, SIZEOF(szClientJid))) {
			JabberCapsBits jcb = GetResourceCapabilites(szClientJid, TRUE);
			return ((~jcb & JABBER_CAPS_ROSTER_EXCHANGE) ? 0 : 50);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int __cdecl CJabberProto::GetInfo(MCONTACT hContact, int /*infoType*/)
{
	if (!m_bJabberOnline || isChatRoom(hContact))
		return 1;

	TCHAR jid[JABBER_MAX_JID_LEN], szBareJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, jid, SIZEOF(jid)))
		return 1;

	JabberStripJid(jid, szBareJid, SIZEOF(szBareJid));
	bool bUseResource = ListGetItemPtr(LIST_CHATROOM, szBareJid) != NULL;

	if (m_ThreadInfo) {
		m_ThreadInfo->send(
			XmlNodeIq(AddIQ(&CJabberProto::OnIqResultEntityTime, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_HCONTACT))
			<< XCHILDNS(_T("time"), JABBER_FEAT_ENTITY_TIME));

		// XEP-0012, last logoff time
		XmlNodeIq iq2(AddIQ(&CJabberProto::OnIqResultLastActivity, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_FROM));
		iq2 << XQUERY(JABBER_FEAT_LAST_ACTIVITY);
		m_ThreadInfo->send(iq2);

		JABBER_LIST_ITEM *item = NULL;

		if ((item = ListGetItemPtr(LIST_VCARD_TEMP, jid)) == NULL)
			item = ListGetItemPtr(LIST_ROSTER, jid);

		if (item == NULL) {
			bool bHasResource = _tcscmp(jid, szBareJid) != 0;
			JABBER_LIST_ITEM *tmpItem = NULL;
			if (bHasResource && (tmpItem = ListGetItemPtr(LIST_CHATROOM, szBareJid))) {
				pResourceStatus him(tmpItem->findResource(szBareJid+_tcslen(szBareJid)+1));
				if (him) {
					item = ListAdd(LIST_VCARD_TEMP, jid);
					ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_tszStatusMessage, him->m_iPriority);
				}
			}
			else item = ListAdd(LIST_VCARD_TEMP, jid);
		}

		if (item != NULL) {
			if (item->arResources.getCount()) {
				for (int i=0; i < item->arResources.getCount(); i++) {
					pResourceStatus r(item->arResources[i]);
					TCHAR tmp[JABBER_MAX_JID_LEN];
					mir_sntprintf(tmp, SIZEOF(tmp), _T("%s/%s"), szBareJid, r->m_tszResourceName);

					if (r->m_jcbCachedCaps & JABBER_CAPS_DISCO_INFO) {
						XmlNodeIq iq5(AddIQ(&CJabberProto::OnIqResultCapsDiscoInfoSI, JABBER_IQ_TYPE_GET, tmp, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_HCONTACT));
						iq5 << XQUERY(JABBER_FEAT_DISCO_INFO);
						m_ThreadInfo->send(iq5);
					}

					if (r->m_dwVersionRequestTime == 0) {
						XmlNodeIq iq4(AddIQ(&CJabberProto::OnIqResultVersion, JABBER_IQ_TYPE_GET, tmp, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_CHILD_TAG_NODE));
						iq4 << XQUERY(JABBER_FEAT_VERSION);
						m_ThreadInfo->send(iq4);
					}

					if (!_tcscmp(tmp, jid)) {
						XmlNodeIq iq3(AddIQ(&CJabberProto::OnIqResultLastActivity, JABBER_IQ_TYPE_GET, tmp, JABBER_IQ_PARSE_FROM));
						iq3 << XQUERY(JABBER_FEAT_LAST_ACTIVITY);
						m_ThreadInfo->send(iq3);
					}
				}
			}
			else if (item->getTemp()->m_dwVersionRequestTime == 0) {
				XmlNodeIq iq4(AddIQ(&CJabberProto::OnIqResultVersion, JABBER_IQ_TYPE_GET, item->jid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_CHILD_TAG_NODE));
				iq4 << XQUERY(JABBER_FEAT_VERSION);
				m_ThreadInfo->send(iq4);
			}
		}
	}

	SendGetVcard(bUseResource ? jid : szBareJid);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by JID

struct JABBER_SEARCH_BASIC
{
	int hSearch;
	TCHAR jid[128];
};

void __cdecl CJabberProto::BasicSearchThread(JABBER_SEARCH_BASIC *jsb)
{
	Sleep(100);

	JABBER_SEARCH_RESULT jsr = { 0 };
	jsr.hdr.cbSize = sizeof(JABBER_SEARCH_RESULT);
	jsr.hdr.flags = PSR_TCHAR;
	jsr.hdr.nick = jsb->jid;
	jsr.hdr.firstName = _T("");
	jsr.hdr.lastName = _T("");
	jsr.hdr.id = jsb->jid;

	_tcsncpy_s(jsr.jid, jsb->jid, _TRUNCATE);

	jsr.jid[SIZEOF(jsr.jid)-1] = '\0';
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)jsb->hSearch, (LPARAM)&jsr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)jsb->hSearch, 0);
	mir_free(jsb);
}

HANDLE __cdecl CJabberProto::SearchBasic(const TCHAR *szJid)
{
	debugLog(_T("JabberBasicSearch called with lParam = '%s'"), szJid);

	JABBER_SEARCH_BASIC *jsb;
	if (!m_bJabberOnline || (jsb = (JABBER_SEARCH_BASIC*)mir_alloc(sizeof(JABBER_SEARCH_BASIC))) == NULL)
		return 0;

	if (_tcschr(szJid, '@') == NULL) {
		TCHAR *szServer = mir_a2t(m_ThreadInfo->server);
		const TCHAR *p = _tcsstr(szJid, szServer);
		if (p == NULL) {
			bool numericjid = true;
			for (const TCHAR *i = szJid; *i && numericjid; i++)
				numericjid = (*i >= '0') && (*i <= '9');

			mir_free(szServer);
			szServer = getTStringA(NULL, "LoginServer");
			if (szServer == NULL)
				szServer = mir_tstrdup(_T("jabber.org"));
			else if (numericjid && !_tcsicmp(szServer, _T("S.ms"))) {
				mir_free(szServer);
				szServer = mir_tstrdup(_T("sms"));
			}
			mir_sntprintf(jsb->jid, SIZEOF(jsb->jid), _T("%s@%s"), szJid, szServer);
		}
		else _tcsncpy_s(jsb->jid, szJid, _TRUNCATE);
		mir_free(szServer);
	}
	else _tcsncpy_s(jsb->jid, szJid, _TRUNCATE);

	debugLog(_T("Adding '%s' without validation"), jsb->jid);
	jsb->hSearch = SerialNext();
	ForkThread((MyThreadFunc)&CJabberProto::BasicSearchThread, jsb);
	return (HANDLE)jsb->hSearch;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE __cdecl CJabberProto::SearchByEmail(const TCHAR *email)
{
	if (!m_bJabberOnline || email == NULL)
		return 0;

	ptrA szServerName(getStringA("Jud"));

	LPCSTR jid = szServerName == 0 ? "users.jabber.org" : szServerName;
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultSetSearch, JABBER_IQ_TYPE_SET, _A2T(jid));
	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(_T("jabber:iq:search")) << XCHILD(_T("email"), email));
	return (HANDLE)pInfo->GetIqId();
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByName - searches the contact by its first or last name, or by a nickname

HANDLE __cdecl CJabberProto::SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName)
{
	if (!m_bJabberOnline)
		return NULL;

	BOOL bIsExtFormat = m_options.ExtendedSearch;

	ptrA szServerName(getStringA("Jud"));

	CJabberIqInfo *pInfo = AddIQ(
		(bIsExtFormat) ? &CJabberProto::OnIqResultExtSearch : &CJabberProto::OnIqResultSetSearch,
		JABBER_IQ_TYPE_SET, _A2T(szServerName == 0 ? "users.jabber.org" : szServerName));
	XmlNodeIq iq(pInfo);
	HXML query = iq << XQUERY(_T("jabber:iq:search"));

	if (bIsExtFormat) {
		if (m_tszSelectedLang)
			iq << XATTR(_T("xml:lang"), m_tszSelectedLang);

		HXML x = query << XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("submit"));
		if (nick[0] != '\0')
			x << XCHILD(_T("field")) << XATTR(_T("var"), _T("user")) << XATTR(_T("value"), nick);

		if (firstName[0] != '\0')
			x << XCHILD(_T("field")) << XATTR(_T("var"), _T("fn")) << XATTR(_T("value"), firstName);

		if (lastName[0] != '\0')
			x << XCHILD(_T("field")) << XATTR(_T("var"), _T("given")) << XATTR(_T("value"), lastName);
	}
	else {
		if (nick[0] != '\0')
			query << XCHILD(_T("nick"), nick);

		if (firstName[0] != '\0')
			query << XCHILD(_T("first"), firstName);

		if (lastName[0] != '\0')
			query << XCHILD(_T("last"), lastName);
	}

	m_ThreadInfo->send(iq);
	return (HANDLE)pInfo->GetIqId();
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvContacts

int __cdecl CJabberProto::RecvContacts(MCONTACT, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvFile

int __cdecl CJabberProto::RecvFile(MCONTACT hContact, PROTORECVFILET *evt)
{
	return Proto_RecvFile(hContact, evt);
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

int __cdecl CJabberProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *evt)
{
	ptrA szResUtf(mir_utf8encodeT((LPCTSTR)evt->lParam));
	evt->pCustomData = szResUtf;
	evt->cbCustomDataSize = lstrlenA(szResUtf);
	Proto_RecvMessage(hContact, evt);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvUrl

int __cdecl CJabberProto::RecvUrl(MCONTACT, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CJabberProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	if (!m_bJabberOnline)
		return 0;

	TCHAR szClientJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, szClientJid, SIZEOF(szClientJid)))
		return 0;

	JabberCapsBits jcb = GetResourceCapabilites(szClientJid, TRUE);
	if (~jcb & JABBER_CAPS_ROSTER_EXCHANGE)
		return 0;

	XmlNode m(_T("message"));
	HXML x = m << XCHILDNS(_T("x"), JABBER_FEAT_ROSTER_EXCHANGE);

	for (int i = 0; i < nContacts; i++) {
		ptrT jid(getTStringA(hContactsList[i], "jid"));
		if (jid != NULL)
			x << XCHILD(_T("item")) << XATTR(_T("action"), _T("add")) << XATTR(_T("jid"), jid);
	}

	m << XATTR(_T("to"), szClientJid) << XATTRID(SerialNext());
	m_ThreadInfo->send(m);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE __cdecl CJabberProto::SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR** ppszFiles)
{
	if (!m_bJabberOnline) return 0;

	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
		return 0;

	ptrT jid(getTStringA(hContact, "jid"));
	if (jid == NULL)
		return 0;

	int i, j;
	struct _stati64 statbuf;
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == NULL)
		return 0;

	// Check if another file transfer session request is pending (waiting for disco result)
	if (item->ft != NULL)
		return 0;

	JabberCapsBits jcb = GetResourceCapabilites(item->jid, TRUE);
	if (jcb == JABBER_RESOURCE_CAPS_IN_PROGRESS) {
		Sleep(600);
		jcb = GetResourceCapabilites(item->jid, TRUE);
	}

	// fix for very smart clients, like gajim
	if (!m_options.BsDirect && !m_options.BsProxyManual) {
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
		MsgPopup(hContact, TranslateT("No compatible file transfer mechanism exists"), item->jid);
		return 0;
	}

	filetransfer *ft = new filetransfer(this);
	ft->std.hContact = hContact;
	while (ppszFiles[ft->std.totalFiles] != NULL)
		ft->std.totalFiles++;

	ft->std.ptszFiles = (TCHAR**)mir_calloc(sizeof(TCHAR*)* ft->std.totalFiles);
	ft->fileSize = (unsigned __int64*)mir_calloc(sizeof(unsigned __int64)* ft->std.totalFiles);
	for (i = j = 0; i < ft->std.totalFiles; i++) {
		if (_tstati64(ppszFiles[i], &statbuf))
			debugLog(_T("'%s' is an invalid filename"), ppszFiles[i]);
		else {
			ft->std.ptszFiles[j] = mir_tstrdup(ppszFiles[i]);
			ft->fileSize[j] = statbuf.st_size;
			j++;
			ft->std.totalBytes += statbuf.st_size;
		}
	}
	if (j == 0) {
		delete ft;
		return NULL;
	}

	ft->std.tszCurrentFile = mir_tstrdup(ppszFiles[0]);
	ft->szDescription = mir_tstrdup(szDescription);
	ft->jid = mir_tstrdup(jid);

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
	inline TFakeAckParams(MCONTACT _hContact, const char* _msg, int _msgid = 0)
		: hContact(_hContact), msg(_msg), msgid(_msgid)
	{}

	MCONTACT hContact;
	const char *msg;
	int msgid;
};

void __cdecl CJabberProto::SendMessageAckThread(void* param)
{
	TFakeAckParams *par = (TFakeAckParams*)param;
	Sleep(100);
	debugLogA("Broadcast ACK");
	ProtoBroadcastAck(par->hContact, ACKTYPE_MESSAGE,
							par->msg ? ACKRESULT_FAILED : ACKRESULT_SUCCESS,
							(HANDLE)par->msgid, (LPARAM)par->msg);
	debugLogA("Returning from thread");
	delete par;
}

static char PGP_PROLOG[] = "-----BEGIN PGP MESSAGE-----\r\n\r\n";
static char PGP_EPILOG[] = "\r\n-----END PGP MESSAGE-----\r\n";

int __cdecl CJabberProto::SendMsg(MCONTACT hContact, int flags, const char* pszSrc)
{
	TCHAR szClientJid[JABBER_MAX_JID_LEN];
	if (!m_bJabberOnline || !GetClientJID(hContact, szClientJid, SIZEOF(szClientJid))) {
		TFakeAckParams *param = new TFakeAckParams(hContact, Translate("Protocol is offline or no JID"));
		ForkThread(&CJabberProto::SendMessageAckThread, param);
		return 1;
	}

	TCHAR *msg;
	int  isEncrypted, id = SerialNext();

	if (!strncmp(pszSrc, PGP_PROLOG, strlen(PGP_PROLOG))) {
		const char* szEnd = strstr(pszSrc, PGP_EPILOG);
		char* tempstring = (char*)alloca(strlen(pszSrc) + 1);
		size_t nStrippedLength = strlen(pszSrc) - strlen(PGP_PROLOG) - (szEnd ? strlen(szEnd) : 0);
		strncpy_s(tempstring, nStrippedLength, pszSrc + strlen(PGP_PROLOG), _TRUNCATE);
		tempstring[nStrippedLength] = 0;
		pszSrc = tempstring;
		isEncrypted = 1;
		flags &= ~PREF_UNICODE;
	}
	else isEncrypted = 0;

	if (flags & PREF_UTF)
		mir_utf8decode(NEWSTR_ALLOCA(pszSrc), &msg);
	else if (flags & PREF_UNICODE)
		msg = mir_u2t((wchar_t*)&pszSrc[strlen(pszSrc) + 1]);
	else
		msg = mir_a2t(pszSrc);

	if (msg == NULL)
		return 0;

	TCHAR *msgType;
	if (ListGetItemPtr(LIST_CHATROOM, szClientJid) && _tcschr(szClientJid, '/') == NULL)
		msgType = _T("groupchat");
	else
		msgType = _T("chat");

	XmlNode m(_T("message")); xmlAddAttr(m, _T("type"), msgType);
	if (!isEncrypted)
		m << XCHILD(_T("body"), msg);
	else {
		m << XCHILD(_T("body"), _T("[This message is encrypted.]"));
		m << XCHILD(_T("x"), msg) << XATTR(_T("xmlns"), _T("jabber:x:encrypted"));
	}
	mir_free(msg);

	pResourceStatus r(ResourceInfoFromJID(szClientJid));
	if (r)
		r->m_bMessageSessionActive = TRUE;

	JabberCapsBits jcb = GetResourceCapabilites(szClientJid, TRUE);

	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	if (jcb & JABBER_CAPS_CHATSTATES)
		m << XCHILDNS(_T("active"), JABBER_FEAT_CHATSTATES);

	if (
		// if message delivery check disabled by entity caps manager
		(jcb & JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY) ||
		// if client knows nothing about delivery
		!(jcb & (JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_RECEIPTS)) ||
		// if message sent to groupchat
		!lstrcmp(msgType, _T("groupchat")) ||
		// if message delivery check disabled in settings
		!m_options.MsgAck || !getByte(hContact, "MsgAck", TRUE)) {
		if (!lstrcmp(msgType, _T("groupchat")))
			xmlAddAttr(m, _T("to"), szClientJid);
		else {
			id = SerialNext();
			xmlAddAttr(m, _T("to"), szClientJid); xmlAddAttrID(m, id);
		}
		m_ThreadInfo->send(m);

		ForkThread(&CJabberProto::SendMessageAckThread, new TFakeAckParams(hContact, 0, id));
	}
	else {
		xmlAddAttr(m, _T("to"), szClientJid); xmlAddAttrID(m, id);

		// message receipts XEP priority
		if (jcb & JABBER_CAPS_MESSAGE_RECEIPTS)
			m << XCHILDNS(_T("request"), JABBER_FEAT_MESSAGE_RECEIPTS);
		else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
			HXML x = m << XCHILDNS(_T("x"), JABBER_FEAT_MESSAGE_EVENTS);
			x << XCHILD(_T("delivered")); x << XCHILD(_T("offline"));
		}

		m_ThreadInfo->send(m);
	}
	return id;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int __cdecl CJabberProto::SendUrl(MCONTACT, int /*flags*/, const char* /*url*/)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetApparentMode - sets the visibility status

int __cdecl CJabberProto::SetApparentMode(MCONTACT hContact, int mode)
{
	if (mode != 0 && mode != ID_STATUS_ONLINE && mode != ID_STATUS_OFFLINE)
		return 1;

	int oldMode = getWord(hContact, "ApparentMode", 0);
	if (mode == oldMode)
		return 1;

	setWord(hContact, "ApparentMode", (WORD)mode);
	if (!m_bJabberOnline)
		return 0;

	ptrT jid(getTStringA(hContact, "jid"));
	if (jid == NULL)
		return 0;

	switch (mode) {
	case ID_STATUS_ONLINE:
		if (m_iStatus == ID_STATUS_INVISIBLE || oldMode == ID_STATUS_OFFLINE)
			m_ThreadInfo->send(XmlNode(_T("presence")) << XATTR(_T("to"), jid));
		break;
	case ID_STATUS_OFFLINE:
		if (m_iStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE)
			SendPresenceTo(ID_STATUS_INVISIBLE, jid, NULL);
		break;
	case 0:
		if (oldMode == ID_STATUS_ONLINE && m_iStatus == ID_STATUS_INVISIBLE)
			SendPresenceTo(ID_STATUS_INVISIBLE, jid, NULL);
		else if (oldMode == ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_INVISIBLE)
			SendPresenceTo(m_iStatus, jid, NULL);
		break;
	}

	// TODO: update the zebra list (jabber:iq:privacy)
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetStatus - sets the protocol status

int __cdecl CJabberProto::SetStatus(int iNewStatus)
{
	if (m_iDesiredStatus == iNewStatus)
		return 0;

	int oldStatus = m_iStatus;

	debugLogA("PS_SETSTATUS(%d)", iNewStatus);
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_ThreadInfo) {
			m_ThreadInfo->send("</stream:stream>");
			m_ThreadInfo->shutdown();
			RebuildInfoFrame();
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (!m_ThreadInfo && !(m_iStatus >= ID_STATUS_CONNECTING && m_iStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ThreadData* thread = new ThreadData(this, JABBER_SESSION_NORMAL);
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		thread->hThread = ForkThreadEx((MyThreadFunc)&CJabberProto::ServerThread, thread, 0);

		RebuildInfoFrame();
	}
	else if (m_bJabberOnline)
		SetServerStatus(iNewStatus);
	else
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAwayMsg - returns a contact's away message

void __cdecl CJabberProto::GetAwayMsgThread(void *param)
{
	MCONTACT hContact = (MCONTACT)param;

	ptrT jid(getTStringA(hContact, "jid"));
	if (jid != NULL) {
		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
		if (item != NULL) {
			if (item->arResources.getCount() > 0) {
				debugLogA("arResources.getCount() > 0");
				int msgCount = 0;
				size_t len = 0;
				for (int i = 0; i < item->arResources.getCount(); i++) {
					JABBER_RESOURCE_STATUS *r = item->arResources[i];
					if (r->m_tszStatusMessage) {
						msgCount++;
						len += (_tcslen(r->m_tszResourceName) + _tcslen(r->m_tszStatusMessage) + 8);
					}
				}

				TCHAR *str = (TCHAR*)alloca(sizeof(TCHAR)*(len + 1));
				str[0] = str[len] = '\0';
				for (int i = 0; i < item->arResources.getCount(); i++) {
					JABBER_RESOURCE_STATUS *r = item->arResources[i];
					if (r->m_tszStatusMessage) {
						if (str[0] != '\0') _tcscat(str, _T("\r\n"));
						if (msgCount > 1) {
							_tcscat(str, _T("("));
							_tcscat(str, r->m_tszResourceName);
							_tcscat(str, _T("): "));
						}
						_tcscat(str, r->m_tszStatusMessage);
					}
				}

				ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)str);
				return;
			}

			TCHAR *tszStatusMsg = item->getTemp()->m_tszStatusMessage;
			if (tszStatusMsg != NULL) {
				ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)tszStatusMsg);
				return;
			}
		}
	}

	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

HANDLE __cdecl CJabberProto::GetAwayMsg(MCONTACT hContact)
{
	debugLogA("GetAwayMsg called, hContact=%08X", hContact);

	ForkThread(&CJabberProto::GetAwayMsgThread, (void*)hContact);
	return (HANDLE)1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG

int __cdecl CJabberProto::RecvAwayMsg(MCONTACT, int /*statusMode*/, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetAwayMsg - sets the away status message

int __cdecl CJabberProto::SetAwayMsg(int status, const TCHAR *msg)
{
	debugLog(_T("SetAwayMsg called, wParam=%d lParam=%s"), status, msg);

	TCHAR **szMsg;
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

	if ((*szMsg == NULL && msg == NULL) || (*szMsg != NULL && msg != NULL && !lstrcmp(*szMsg, msg)))
		return 0; // Message is the same, no update needed

	// Update with the new mode message
	replaceStrT(*szMsg, msg);

	// Send a presence update if needed
	lck.unlock();
	if (status == m_iStatus)
		SendPresence(m_iStatus, true);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserIsTyping - sends a UTN notification

int __cdecl CJabberProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (!m_bJabberOnline) return 0;

	TCHAR szClientJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, szClientJid, SIZEOF(szClientJid)))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, szClientJid);
	if (item == NULL)
		return 0;

	JabberCapsBits jcb = GetResourceCapabilites(szClientJid, TRUE);
	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	XmlNode m(_T("message")); xmlAddAttr(m, _T("to"), szClientJid);

	if (jcb & JABBER_CAPS_CHATSTATES) {
		m << XATTR(_T("type"), _T("chat")) << XATTRID(SerialNext());
		switch (type) {
		case PROTOTYPE_SELFTYPING_OFF:
			m << XCHILDNS(_T("paused"), JABBER_FEAT_CHATSTATES);
			m_ThreadInfo->send(m);
			break;
		case PROTOTYPE_SELFTYPING_ON:
			m << XCHILDNS(_T("composing"), JABBER_FEAT_CHATSTATES);
			m_ThreadInfo->send(m);
			break;
		}
	}
	else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
		HXML x = m << XCHILDNS(_T("x"), JABBER_FEAT_MESSAGE_EVENTS);
		if (item->messageEventIdStr != NULL)
			x << XCHILD(_T("id"), item->messageEventIdStr);

		switch (type) {
		case PROTOTYPE_SELFTYPING_OFF:
			m_ThreadInfo->send(m);
			break;
		case PROTOTYPE_SELFTYPING_ON:
			x << XCHILD(_T("composing"));
			m_ThreadInfo->send(m);
			break;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Notify dialogs

void CJabberProto::WindowSubscribe(HWND hwnd)
{
	WindowList_Add(m_windowList, hwnd, NULL);
}

void CJabberProto::WindowUnsubscribe(HWND hwnd)
{
	WindowList_Remove(m_windowList, hwnd);
}

void CJabberProto::WindowNotify(UINT msg, bool async)
{
	if (async)
		WindowList_BroadcastAsync(m_windowList, msg, 0, 0);
	else
		WindowList_Broadcast(m_windowList, msg, 0, 0);
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
		HMENU hContactMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
		POINT pt;
		GetCursorPos(&pt);
		int res = TrackPopupMenu(hContactMenu, TPM_RETURNCMD, pt.x, pt.y, 0, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL);
		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(res, MPCF_CONTACTMENU), hContact);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnEvent - maintain protocol events

int __cdecl CJabberProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) {
	case EV_PROTO_ONLOAD:    return OnModulesLoadedEx(0, 0);
	case EV_PROTO_ONEXIT:    return OnPreShutdown(0, 0);
	case EV_PROTO_ONOPTIONS: return OnOptionsInit(wParam, lParam);

	case EV_PROTO_ONMENU:
		MenuInit();
		break;

	case EV_PROTO_ONRENAME:
		if (m_hMenuRoot) {
			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.flags = CMIM_NAME | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			clmi.ptszName = m_tszUserName;
			Menu_ModifyItem(m_hMenuRoot, &clmi);
		}
		break;

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return OnDbSettingChanged(wParam, lParam);
	}
	return 1;
}
