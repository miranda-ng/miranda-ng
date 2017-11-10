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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_disco.h"

#pragma warning(disable:4355)

static int compareTransports(const wchar_t *p1, const wchar_t *p2)
{
	return mir_wstrcmpi(p1, p2);
}

static int compareListItems(const JABBER_LIST_ITEM *p1, const JABBER_LIST_ITEM *p2)
{
	if (p1->list != p2->list)
		return p1->list - p2->list;

	// for bookmarks, temporary contacts & groupchat members
	// resource must be used in the comparison
	if ((p1->list == LIST_ROSTER && (p1->bUseResource == true || p2->bUseResource == true))
		|| (p1->list == LIST_BOOKMARK) || (p1->list == LIST_VCARD_TEMP))
		return mir_wstrcmpi(p1->jid, p2->jid);

	wchar_t szp1[JABBER_MAX_JID_LEN], szp2[JABBER_MAX_JID_LEN];
	JabberStripJid(p1->jid, szp1, _countof(szp1));
	JabberStripJid(p2->jid, szp2, _countof(szp2));
	return mir_wstrcmpi(szp1, szp2);
}

CJabberProto::CJabberProto(const char *aProtoName, const wchar_t *aUserName) :
	PROTO<CJabberProto>(aProtoName, aUserName),
	m_omemo(this),
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
	m_szXmlStreamToBeInitialized = nullptr;

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

	CreateProtoService(JS_DB_GETEVENTTEXT_CHATSTATES, &CJabberProto::OnGetEventTextChatStates);
	CreateProtoService(JS_DB_GETEVENTTEXT_PRESENCE, &CJabberProto::OnGetEventTextPresence);

	CreateProtoService(JS_GETJABBERAPI, &CJabberProto::JabberGetApi);

	// XEP-0224 support (Attention/Nudge)
	CreateProtoService(PS_SEND_NUDGE, &CJabberProto::JabberSendNudge);

	// service to get from protocol chat buddy info
	CreateProtoService(MS_GC_PROTO_GETTOOLTIPTEXT, &CJabberProto::JabberGCGetToolTipText);

	// XMPP URI parser service for "File Association Manager" plugin
	CreateProtoService(JS_PARSE_XMPP_URI, &CJabberProto::JabberServiceParseXmppURI);

	HookProtoEvent(ME_OPT_INITIALISE, &CJabberProto::OnOptionsInit);
	HookProtoEvent(ME_SKIN2_ICONSCHANGED, &CJabberProto::OnReloadIcons);

	m_iqManager.FillPermanentHandlers();
	m_iqManager.Start();
	m_messageManager.FillPermanentHandlers();
	m_adhocManager.FillDefaultNodes();
	
	AddDefaultCaps();

	IconsInit();
	InitPopups();
	GlobalMenuInit();
	WsInit();
	ConsoleInit();

	m_pepServices.insert(new CPepMood(this));
	m_pepServices.insert(new CPepActivity(this));

	if (m_options.UseOMEMO)
	{
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

	if ((m_tszSelectedLang = getWStringA("XmlLang")) == nullptr)
		m_tszSelectedLang = mir_wstrdup(L"en");
}


CJabberProto::~CJabberProto()
{
	WsUninit();
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

	for (int i = 0; i < m_lstTransports.getCount(); i++)
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

int CJabberProto::OnModulesLoadedEx(WPARAM, LPARAM)
{
	HookProtoEvent(ME_USERINFO_INITIALISE, &CJabberProto::OnUserInfoInit);
	XStatusInit();
	m_pepServices.InitGui();

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
	DbEvent_RegisterType(&dbEventType);

	dbEventType.eventType = EVENTTYPE_JABBER_PRESENCE;
	dbEventType.descr = "Presence notifications";
	DbEvent_RegisterType(&dbEventType);

	HookProtoEvent(ME_IDLE_CHANGED, &CJabberProto::OnIdleChanged);

	CheckAllContactsAreTransported();

	// Set all contacts to offline
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		SetContactOfflineStatus(hContact);

		if (getByte(hContact, "IsTransport", 0)) {
			ptrW jid(getWStringA(hContact, "jid"));
			if (jid == nullptr)
				continue;

			wchar_t *resourcepos = wcschr(jid, '/');
			if (resourcepos != nullptr)
				*resourcepos = '\0';

			m_lstTransports.insert(mir_wstrdup(jid));
		}
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

	Srmm_RemoveIcon(m_szModuleName, 0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAddToList - adds a contact to the contact list

MCONTACT CJabberProto::AddToListByJID(const wchar_t *newJid, DWORD flags)
{
	debugLogW(L"AddToListByJID jid = %s", newJid);

	MCONTACT hContact = DBCreateContact(newJid, nullptr, true, false);
	if (flags & PALF_TEMPORARY)
		db_set_b(hContact, "CList", "Hidden", 1);

	return hContact;
}

MCONTACT CJabberProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT) && psr->id.w == nullptr)
		return 0;

	return AddToListByJID(psr->id.w, flags);
}

MCONTACT __cdecl CJabberProto::AddToListByEvent(int flags, int /*iContact*/, MEVENT hDbEvent)
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
	return AddToListByJID(ptrW(dbei.getString(blob.get_email())), flags);
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
	debugLogW(L"Send 'authorization allowed' to %s", blob.get_email());

	ptrW newJid(dbei.getString(blob.get_email()));

	m_ThreadInfo->send(XmlNode(L"presence") << XATTR(L"to", newJid) << XATTR(L"type", L"subscribed"));

	// Automatically add this user to my roster if option is enabled
	if (m_options.AutoAdd == TRUE) {
		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, newJid);
		if (item == nullptr || (item->subscription != SUB_BOTH && item->subscription != SUB_TO)) {
			debugLogW(L"Try adding contact automatically jid = %s", blob.get_email());
			if (MCONTACT hContact = AddToListByJID(newJid, 0)) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				db_unset(hContact, "CList", "NotOnList");
			}
		}
	}

	mir_free(newJid);
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

	char *nick = (char*)(dbei.pBlob + sizeof(DWORD)*2);
	char *firstName = nick + mir_strlen(nick) + 1;
	char *lastName = firstName + mir_strlen(firstName) + 1;
	char *jid = lastName + mir_strlen(lastName) + 1;

	debugLogA("Send 'authorization denied' to %s", jid);

	ptrW newJid(dbei.flags & DBEF_UTF ? mir_utf8decodeW(jid) : mir_a2u(jid));
	m_ThreadInfo->send(XmlNode(L"presence") << XATTR(L"to", newJid) << XATTR(L"type", L"unsubscribed"));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileAllow - starts a file transfer

HANDLE __cdecl CJabberProto::FileAllow(MCONTACT /*hContact*/, HANDLE hTransfer, const wchar_t *szPath)
{
	if (!m_bJabberOnline)
		return 0;

	filetransfer *ft = (filetransfer*)hTransfer;
	ft->std.tszWorkingDir = mir_wstrdup(szPath);
	size_t len = mir_wstrlen(ft->std.tszWorkingDir)-1;
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

int __cdecl CJabberProto::FileDeny(MCONTACT, HANDLE hTransfer, const wchar_t *)
{
	if (!m_bJabberOnline)
		return 1;

	filetransfer *ft = (filetransfer*)hTransfer;

	switch (ft->type) {
	case FT_OOB:
		m_ThreadInfo->send(XmlNodeIq(L"error", ft->szId, ft->jid) << XCHILD(L"error", L"File transfer refused") << XATTRI(L"code", 406));
		break;

	case FT_BYTESTREAM:
	case FT_IBB:
		m_ThreadInfo->send(
			XmlNodeIq(L"error", ft->szId, ft->jid)
			<< XCHILD(L"error", L"File transfer refused") << XATTRI(L"code", 403) << XATTR(L"type", L"cancel")
			<< XCHILDNS(L"forbidden", L"urn:ietf:params:xml:ns:xmpp-stanzas")
			<< XCHILD(L"text", L"File transfer refused") << XATTR(L"xmlns", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
		break;
	}
	delete ft;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileResume - processes file renaming etc

int __cdecl CJabberProto::FileResume(HANDLE hTransfer, int *action, const wchar_t **szFilename)
{
	filetransfer *ft = (filetransfer*)hTransfer;
	if (!m_bJabberOnline || ft == nullptr)
		return 1;

	if (*action == FILERESUME_RENAME)
		replaceStrW(ft->std.tszCurrentFile, *szFilename);

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
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_FORCEADDED;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("JID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"jid";
	case PFLAG_MAXCONTACTSPERPACKET:
		wchar_t szClientJid[JABBER_MAX_JID_LEN];
		if (GetClientJID(hContact, szClientJid, _countof(szClientJid))) {
			JabberCapsBits jcb = GetResourceCapabilities(szClientJid);
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

	wchar_t jid[JABBER_MAX_JID_LEN], szBareJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, jid, _countof(jid)))
		return 1;

	JabberStripJid(jid, szBareJid, _countof(szBareJid));
	bool bUseResource = ListGetItemPtr(LIST_CHATROOM, szBareJid) != nullptr;

	if (m_ThreadInfo) {
		m_ThreadInfo->send(
			XmlNodeIq(AddIQ(&CJabberProto::OnIqResultEntityTime, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_HCONTACT))
			<< XCHILDNS(L"time", JABBER_FEAT_ENTITY_TIME));

		// XEP-0012, last logoff time
		XmlNodeIq iq2(AddIQ(&CJabberProto::OnIqResultLastActivity, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_FROM));
		iq2 << XQUERY(JABBER_FEAT_LAST_ACTIVITY);
		m_ThreadInfo->send(iq2);

		JABBER_LIST_ITEM *item = nullptr;

		if ((item = ListGetItemPtr(LIST_VCARD_TEMP, jid)) == nullptr)
			item = ListGetItemPtr(LIST_ROSTER, jid);

		if (item == nullptr) {
			bool bHasResource = mir_wstrcmp(jid, szBareJid) != 0;
			JABBER_LIST_ITEM *tmpItem = nullptr;
			if (bHasResource && (tmpItem = ListGetItemPtr(LIST_CHATROOM, szBareJid))) {
				pResourceStatus him(tmpItem->findResource(szBareJid+mir_wstrlen(szBareJid)+1));
				if (him) {
					item = ListAdd(LIST_VCARD_TEMP, jid, hContact);
					ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_tszStatusMessage, him->m_iPriority);
				}
			}
			else item = ListAdd(LIST_VCARD_TEMP, jid, hContact);
		}

		if (item != nullptr) {
			if (item->arResources.getCount()) {
				for (int i = 0; i < item->arResources.getCount(); i++) {
					pResourceStatus r(item->arResources[i]);
					wchar_t tmp[JABBER_MAX_JID_LEN];
					mir_snwprintf(tmp, L"%s/%s", szBareJid, r->m_tszResourceName);

					if (r->m_jcbCachedCaps & JABBER_CAPS_DISCO_INFO) {
						XmlNodeIq iq5(AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, tmp, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_HCONTACT));
						iq5 << XQUERY(JABBER_FEAT_DISCO_INFO);
						m_ThreadInfo->send(iq5);
					}

					if (!mir_wstrcmp(tmp, jid)) {
						XmlNodeIq iq3(AddIQ(&CJabberProto::OnIqResultLastActivity, JABBER_IQ_TYPE_GET, tmp, JABBER_IQ_PARSE_FROM));
						iq3 << XQUERY(JABBER_FEAT_LAST_ACTIVITY);
						m_ThreadInfo->send(iq3);
					}
				}
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

HANDLE __cdecl CJabberProto::SearchBasic(const wchar_t *szJid)
{
	debugLogW(L"JabberBasicSearch called with lParam = '%s'", szJid);

	JABBER_SEARCH_BASIC *jsb;
	if (!m_bJabberOnline || (jsb = (JABBER_SEARCH_BASIC*)mir_alloc(sizeof(JABBER_SEARCH_BASIC))) == nullptr)
		return 0;

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

	debugLogW(L"Adding '%s' without validation", jsb->jid);
	jsb->hSearch = SerialNext();
	ForkThread((MyThreadFunc)&CJabberProto::BasicSearchThread, jsb);
	return (HANDLE)jsb->hSearch;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE __cdecl CJabberProto::SearchByEmail(const wchar_t *email)
{
	if (!m_bJabberOnline || email == nullptr)
		return 0;

	ptrA szServerName(getStringA("Jud"));

	LPCSTR jid = szServerName == 0 ? "users.jabber.org" : szServerName;
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultSetSearch, JABBER_IQ_TYPE_SET, _A2T(jid));
	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(L"jabber:iq:search") << XCHILD(L"email", email));
	return (HANDLE)pInfo->GetIqId();
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByName - searches the contact by its first or last name, or by a nickname

HANDLE __cdecl CJabberProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	if (!m_bJabberOnline)
		return nullptr;

	BOOL bIsExtFormat = m_options.ExtendedSearch;

	ptrA szServerName(getStringA("Jud"));

	CJabberIqInfo *pInfo = AddIQ(
		(bIsExtFormat) ? &CJabberProto::OnIqResultExtSearch : &CJabberProto::OnIqResultSetSearch,
		JABBER_IQ_TYPE_SET, _A2T(szServerName == 0 ? "users.jabber.org" : szServerName));
	XmlNodeIq iq(pInfo);
	HXML query = iq << XQUERY(L"jabber:iq:search");

	if (bIsExtFormat) {
		if (m_tszSelectedLang)
			iq << XATTR(L"xml:lang", m_tszSelectedLang);

		HXML x = query << XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"submit");
		if (nick[0] != '\0')
			x << XCHILD(L"field") << XATTR(L"var", L"user") << XATTR(L"value", nick);

		if (firstName[0] != '\0')
			x << XCHILD(L"field") << XATTR(L"var", L"fn") << XATTR(L"value", firstName);

		if (lastName[0] != '\0')
			x << XCHILD(L"field") << XATTR(L"var", L"given") << XATTR(L"value", lastName);
	}
	else {
		if (nick[0] != '\0')
			query << XCHILD(L"nick", nick);

		if (firstName[0] != '\0')
			query << XCHILD(L"first", firstName);

		if (lastName[0] != '\0')
			query << XCHILD(L"last", lastName);
	}

	m_ThreadInfo->send(iq);
	return (HANDLE)pInfo->GetIqId();
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

int __cdecl CJabberProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *evt)
{
	T2Utf szResUtf((LPCTSTR)evt->lParam);
	evt->pCustomData = (char*)szResUtf;
	evt->cbCustomDataSize = (DWORD)mir_strlen(szResUtf);
	Proto_RecvMessage(hContact, evt);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CJabberProto::SendContacts(MCONTACT hContact, int, int nContacts, MCONTACT *hContactsList)
{
	if (!m_bJabberOnline)
		return 0;

	wchar_t szClientJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, szClientJid, _countof(szClientJid)))
		return 0;

	JabberCapsBits jcb = GetResourceCapabilities(szClientJid);
	if (~jcb & JABBER_CAPS_ROSTER_EXCHANGE)
		return 0;

	XmlNode m(L"message");
	HXML x = m << XCHILDNS(L"x", JABBER_FEAT_ROSTER_EXCHANGE);

	for (int i = 0; i < nContacts; i++) {
		ptrW jid(getWStringA(hContactsList[i], "jid"));
		if (jid != nullptr)
			x << XCHILD(L"item") << XATTR(L"action", L"add") << XATTR(L"jid", jid);
	}

	m << XATTR(L"to", szClientJid) << XATTRID(SerialNext());
	m_ThreadInfo->send(m);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE __cdecl CJabberProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t** ppszFiles)
{
	if (!m_bJabberOnline) return 0;

	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
		return 0;

	ptrW jid(getWStringA(hContact, "jid"));
	if (jid == nullptr)
		return 0;

	struct _stati64 statbuf;
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return 0;

	// Check if another file transfer session request is pending (waiting for disco result)
	if (item->ft != nullptr)
		return 0;

	JabberCapsBits jcb = GetResourceCapabilities(item->jid);
	if (jcb == JABBER_RESOURCE_CAPS_IN_PROGRESS) {
		Sleep(600);
		jcb = GetResourceCapabilities(item->jid);
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
	while (ppszFiles[ft->std.totalFiles] != nullptr)
		ft->std.totalFiles++;

	ft->std.ptszFiles = (wchar_t**)mir_calloc(sizeof(wchar_t*)* ft->std.totalFiles);
	ft->fileSize = (unsigned __int64*)mir_calloc(sizeof(unsigned __int64)* ft->std.totalFiles);

	int i, j;
	for (i = j = 0; i < ft->std.totalFiles; i++) {
		if (_wstat64(ppszFiles[i], &statbuf))
			debugLogW(L"'%s' is an invalid filename", ppszFiles[i]);
		else {
			ft->std.ptszFiles[j] = mir_wstrdup(ppszFiles[i]);
			ft->fileSize[j] = statbuf.st_size;
			j++;
			ft->std.totalBytes += statbuf.st_size;
		}
	}
	if (j == 0) {
		delete ft;
		return nullptr;
	}

	ft->std.tszCurrentFile = mir_wstrdup(ppszFiles[0]);
	ft->szDescription = mir_wstrdup(szDescription);
	ft->jid = mir_wstrdup(jid);

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

int __cdecl CJabberProto::SendMsg(MCONTACT hContact, int unused_unknown, const char* pszSrc)
{
	wchar_t szClientJid[JABBER_MAX_JID_LEN];
	if (!m_bJabberOnline || !GetClientJID(hContact, szClientJid, _countof(szClientJid))) {
		TFakeAckParams *param = new TFakeAckParams(hContact, Translate("Protocol is offline or no JID"));
		ForkThread(&CJabberProto::SendMessageAckThread, param);
		return 1;
	}

	if (m_options.UseOMEMO)
	{
		if (!OmemoCheckSession(hContact))
		{
			OmemoPutMessageToOutgoingQueue(hContact, unused_unknown, pszSrc);
			int id = SerialNext();
			TFakeAckParams *param = new TFakeAckParams(hContact, 0, id);
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

	wchar_t *msg;
	mir_utf8decode(NEWSTR_ALLOCA(pszSrc), &msg);
	if (msg == nullptr)
		return 0;

	wchar_t *msgType;
	if (ListGetItemPtr(LIST_CHATROOM, szClientJid) && wcschr(szClientJid, '/') == nullptr)
		msgType = L"groupchat";
	else
		msgType = L"chat";
	XmlNode m(L"message");

	if(m_options.UseOMEMO && OmemoIsEnabled(hContact) && !mir_wstrcmp(msgType, L"chat")) //omemo enabled in options, omemo enabled for contact
	{
		//TODO: check if message encrypted for at least one session and return error if not
		if (!OmemoEncryptMessage(m, msg, hContact))
		{
			TFakeAckParams *param = new TFakeAckParams(hContact, Translate("No valid OMEMO session exists"));
			ForkThread(&CJabberProto::SendMessageAckThread, param);
			return 0;
		}
	}
	else
	{

		XmlAddAttr(m, L"type", msgType);
		if (!isEncrypted)
			m << XCHILD(L"body", msg);
		else {
			m << XCHILD(L"body", L"[This message is encrypted.]");
			m << XCHILD(L"x", msg) << XATTR(L"xmlns", L"jabber:x:encrypted");
		}
		mir_free(msg);
	}

	pResourceStatus r(ResourceInfoFromJID(szClientJid));
	if (r)
		r->m_bMessageSessionActive = true;

	JabberCapsBits jcb = GetResourceCapabilities(szClientJid);

	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	if (jcb & JABBER_CAPS_CHATSTATES)
		m << XCHILDNS(L"active", JABBER_FEAT_CHATSTATES);

	if (
		// if message delivery check disabled by entity caps manager
		(jcb & JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY) ||
		// if client knows nothing about delivery
		!(jcb & (JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_RECEIPTS)) ||
		// if message sent to groupchat
		!mir_wstrcmp(msgType, L"groupchat") ||
		// if message delivery check disabled in settings
		!m_options.MsgAck || !getByte(hContact, "MsgAck", true))
	{
		if (!mir_wstrcmp(msgType, L"groupchat"))
			XmlAddAttr(m, L"to", szClientJid);
		else {
			id = SerialNext();
			XmlAddAttr(m, L"to", szClientJid); XmlAddAttrID(m, id);
		}
		m_ThreadInfo->send(m);

		ForkThread(&CJabberProto::SendMessageAckThread, new TFakeAckParams(hContact, 0, id));
	}
	else {
		XmlAddAttr(m, L"to", szClientJid); XmlAddAttrID(m, id);

		// message receipts XEP priority
		if (jcb & JABBER_CAPS_MESSAGE_RECEIPTS)
			m << XCHILDNS(L"request", JABBER_FEAT_MESSAGE_RECEIPTS);
		else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
			HXML x = m << XCHILDNS(L"x", JABBER_FEAT_MESSAGE_EVENTS);
			x << XCHILD(L"delivered"); x << XCHILD(L"offline");
		}

		m_ThreadInfo->send(m);
	}
	return id;
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

	ptrW jid(getWStringA(hContact, "jid"));
	if (jid == nullptr)
		return 0;

	switch (mode) {
	case ID_STATUS_ONLINE:
		if (m_iStatus == ID_STATUS_INVISIBLE || oldMode == ID_STATUS_OFFLINE)
			m_ThreadInfo->send(XmlNode(L"presence") << XATTR(L"to", jid));
		break;
	case ID_STATUS_OFFLINE:
		if (m_iStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE)
			SendPresenceTo(ID_STATUS_INVISIBLE, jid, nullptr);
		break;
	case 0:
		if (oldMode == ID_STATUS_ONLINE && m_iStatus == ID_STATUS_INVISIBLE)
			SendPresenceTo(ID_STATUS_INVISIBLE, jid, nullptr);
		else if (oldMode == ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_INVISIBLE)
			SendPresenceTo(m_iStatus, jid, nullptr);
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

	ptrW jid(getWStringA(hContact, "jid"));
	if (jid != nullptr) {
		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
		if (item != nullptr) {
			if (item->arResources.getCount() > 0) {
				debugLogA("arResources.getCount() > 0");
				int msgCount = 0;
				size_t len = 0;
				for (int i = 0; i < item->arResources.getCount(); i++) {
					JABBER_RESOURCE_STATUS *r = item->arResources[i];
					if (r->m_tszStatusMessage) {
						msgCount++;
						len += (mir_wstrlen(r->m_tszResourceName) + mir_wstrlen(r->m_tszStatusMessage) + 8);
					}
				}

				wchar_t *str = (wchar_t*)alloca(sizeof(wchar_t)*(len + 1));
				str[0] = str[len] = '\0';
				for (int i = 0; i < item->arResources.getCount(); i++) {
					JABBER_RESOURCE_STATUS *r = item->arResources[i];
					if (r->m_tszStatusMessage) {
						if (str[0] != '\0') mir_wstrcat(str, L"\r\n");
						if (msgCount > 1) {
							mir_wstrcat(str, L"(");
							mir_wstrcat(str, r->m_tszResourceName);
							mir_wstrcat(str, L"): ");
						}
						mir_wstrcat(str, r->m_tszStatusMessage);
					}
				}

				ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)str);
				return;
			}

			wchar_t *tszStatusMsg = item->getTemp()->m_tszStatusMessage;
			if (tszStatusMsg != nullptr) {
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
// JabberSetAwayMsg - sets the away status message

int __cdecl CJabberProto::SetAwayMsg(int status, const wchar_t *msg)
{
	debugLogW(L"SetAwayMsg called, wParam=%d lParam=%s", status, msg);

	wchar_t **szMsg;
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

	if ((*szMsg == nullptr && msg == nullptr) || (*szMsg != nullptr && msg != nullptr && !mir_wstrcmp(*szMsg, msg)))
		return 0; // Message is the same, no update needed

	// Update with the new mode message
	replaceStrW(*szMsg, msg);

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

	wchar_t szClientJid[JABBER_MAX_JID_LEN];
	if (!GetClientJID(hContact, szClientJid, _countof(szClientJid)))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, szClientJid);
	if (item == nullptr)
		return 0;

	JabberCapsBits jcb = GetResourceCapabilities(szClientJid);
	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	XmlNode m(L"message"); XmlAddAttr(m, L"to", szClientJid);

	if (jcb & JABBER_CAPS_CHATSTATES) {
		m << XATTR(L"type", L"chat") << XATTRID(SerialNext());
		switch (type) {
		case PROTOTYPE_SELFTYPING_OFF:
			m << XCHILDNS(L"paused", JABBER_FEAT_CHATSTATES);
			m_ThreadInfo->send(m);
			break;
		case PROTOTYPE_SELFTYPING_ON:
			m << XCHILDNS(L"composing", JABBER_FEAT_CHATSTATES);
			m_ThreadInfo->send(m);
			break;
		}
	}
	else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
		HXML x = m << XCHILDNS(L"x", JABBER_FEAT_MESSAGE_EVENTS);
		if (item->messageEventIdStr != nullptr)
			x << XCHILD(L"id", item->messageEventIdStr);

		switch (type) {
		case PROTOTYPE_SELFTYPING_OFF:
			m_ThreadInfo->send(m);
			break;
		case PROTOTYPE_SELFTYPING_ON:
			x << XCHILD(L"composing");
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
		int res = TrackPopupMenu(hContactMenu, TPM_RETURNCMD, pt.x, pt.y, 0, pcli->hwndContactList, nullptr);
		Clist_MenuProcessCommand(res, MPCF_CONTACTMENU, hContact);
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

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return OnDbSettingChanged(wParam, lParam);
	}
	return 1;
}
