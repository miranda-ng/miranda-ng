// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2022 Miranda NG team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Protocol Interface Implementation
// -----------------------------------------------------------------------------

#include "stdafx.h"

#include "m_icolib.h"

#pragma warning(disable:4355)

static int CompareCache(const IcqCacheItem *p1, const IcqCacheItem *p2)
{
	return mir_wstrcmp(p1->m_aimid, p2->m_aimid);
}

CIcqProto::CIcqProto(const char *aProtoName, const wchar_t *aUserName) :
	PROTO<CIcqProto>(aProtoName, aUserName),
	m_impl(*this),
	m_arHttpQueue(10),
	m_arOwnIds(1, PtrKeySortT),
	m_arCache(20, &CompareCache),
	m_arGroups(10, NumericKeySortT),
	m_arMarkReadQueue(10, NumericKeySortT),
	m_evRequestsQueue(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_szOwnId(this, DB_KEY_ID),
	m_iStatus1(this, "Status1", ID_STATUS_AWAY),
	m_iStatus2(this, "Status2", ID_STATUS_NA),
	m_iTimeDiff1(this, "TimeDiff1", 0),
	m_iTimeDiff2(this, "TimeDiff2", 0),
	m_bHideGroupchats(this, "HideChats", true),
	m_bUseTrayIcon(this, "UseTrayIcon", false),
	m_bErrorPopups(this, "ShowErrorPopups", true),
	m_bLaunchMailbox(this, "LaunchMailbox", true)
{
	db_set_resident(m_szModuleName, DB_KEY_IDLE);
	db_set_resident(m_szModuleName, DB_KEY_ONLINETS);

	m_isMra = !stricmp(Proto_GetAccount(m_szModuleName)->szProtoName, "MRA");

	// services
	CreateProtoService(PS_CREATEACCMGRUI, &CIcqProto::CreateAccMgrUI);

	CreateProtoService(PS_GETAVATARCAPS, &CIcqProto::GetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFO, &CIcqProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CIcqProto::GetAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CIcqProto::SetAvatar);

	CreateProtoService(PS_MENU_LOADHISTORY, &CIcqProto::OnMenuLoadHistory);
	CreateProtoService(PS_GETUNREADEMAILCOUNT, &CIcqProto::GetEmailCount);
	CreateProtoService(PS_GOTO_INBOX, &CIcqProto::GotoInbox);

	// events
	HookProtoEvent(ME_CLIST_GROUPCHANGE, &CIcqProto::OnGroupChange);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CIcqProto::OnDbEventRead);
	HookProtoEvent(ME_GC_EVENT, &CIcqProto::GroupchatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CIcqProto::GroupchatMenuHook);
	HookProtoEvent(ME_OPT_INITIALISE, &CIcqProto::OnOptionsInit);

	// group chats
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	// netlib handle
	NETLIBUSER nlu = {};
	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// this was previously an old ICQ account
	ptrW wszUin(GetUIN(0));
	if (wszUin != nullptr) {
		delSetting("UIN");

		m_szOwnId = wszUin;

		for (auto &it : AccContacts())
			delSetting(it, "e-mail");
	}
	// this was previously an old MRA account
	else {
		CMStringW wszEmail(getMStringW("e-mail"));
		if (!wszEmail.IsEmpty()) {
			m_szOwnId = wszEmail;
			delSetting("e-mail");
		}
	}

	m_hWorkerThread = ForkThreadEx(&CIcqProto::ServerThread, nullptr, nullptr);
}

CIcqProto::~CIcqProto()
{
	::CloseHandle(m_evRequestsQueue);
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

void CIcqProto::OnModulesLoaded()
{
	InitContactCache();

	HookProtoEvent(ME_USERINFO_INITIALISE, &CIcqProto::OnUserInfoInit);

	// load custom smilies
	CMStringW wszPath(FORMAT, L"%s\\%S\\Stickers\\*.png", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
	SMADD_CONT cont = { 2, m_szModuleName, wszPath };
	CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));
}

void CIcqProto::OnShutdown()
{
	m_bTerminated = true;
}

void CIcqProto::OnContactAdded(MCONTACT hContact)
{
	CMStringW wszId(getMStringW(hContact, DB_KEY_ID));
	if (!wszId.IsEmpty() && !FindContactByUIN(wszId)) {
		mir_cslock l(m_csCache);
		m_arCache.insert(new IcqCacheItem(wszId, hContact));
	}
}

void CIcqProto::OnContactDeleted(MCONTACT hContact)
{
	CMStringW szId(GetUserId(hContact));
	if (!isChatRoom(hContact))
		m_arCache.remove(FindContactByUIN(szId));

	Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/removeBuddy")
		<< AIMSID(this) << WCHAR_PARAM("buddy", szId) << INT_PARAM("allGroups", 1));
}

void CIcqProto::OnEventEdited(MCONTACT, MEVENT)
{

}

INT_PTR CIcqProto::OnMenuLoadHistory(WPARAM hContact, LPARAM)
{
	delSetting(hContact, DB_KEY_LASTMSGID);

	RetrieveUserHistory(hContact, 1, true);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	// "Bookmarks..."
	mi.pszService = "/UploadGroups";
	CreateProtoService(mi.pszService, &CIcqProto::UploadGroups);
	mi.name.a = LPGEN("Synchronize server groups");
	mi.position = 200001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUP);
	m_hUploadGroups = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/EditGroups";
	CreateProtoService(mi.pszService, &CIcqProto::EditGroups);
	mi.name.a = LPGEN("Edit server groups");
	mi.position = 200002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUP);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/EditProfile";
	CreateProtoService(mi.pszService, &CIcqProto::EditProfile);
	mi.name.a = LPGEN("Edit my web profile");
	mi.position = 210001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDAWEB);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	Menu_ShowItem(m_hUploadGroups, false);
}

INT_PTR CIcqProto::UploadGroups(WPARAM, LPARAM)
{
	for (auto &it : AccContacts()) {
		if (isChatRoom(it))
			continue;

		ptrW wszIcqGroup(getWStringA(it, "IcqGroup"));
		if (wszIcqGroup == nullptr)
			continue;

		ptrW wszMirGroup(Clist_GetGroup(it));
		if (!wszMirGroup)
			wszMirGroup = mir_wstrdup(L"General");
		if (mir_wstrcmp(wszIcqGroup, wszMirGroup))
			MoveContactToGroup(it, wszIcqGroup, wszMirGroup);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CGroupEditDlg : public CIcqDlgBase
{
	CCtrlListView groups;

public:
	
	static CGroupEditDlg *pDlg;

	CGroupEditDlg(CIcqProto *ppro) :
		CIcqDlgBase(ppro, IDD_EDITGROUPS),
		groups(this, IDC_GROUPS)
	{
		groups.OnBuildMenu = Callback(this, &CGroupEditDlg::onMenu);
	}

	void RefreshGroups()
	{
		for (auto &it : m_proto->m_arGroups.rev_iter())
			groups.AddItem(it->wszName, 0, (LPARAM)it);
	}

	bool OnInitDialog() override
	{
		pDlg = this;
		groups.AddColumn(0, TranslateT("Name"), 300);
		RefreshGroups();
		return true;
	}

	void OnDestroy() override
	{
		pDlg = nullptr;
	}

	void onMenu(void *)
	{
		int cur = groups.GetSelectionMark();
		if (cur == -1)
			return;

		IcqGroup *pGroup = (IcqGroup *)groups.GetItemData(cur);

		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, TranslateT("Rename"));
		AppendMenu(hMenu, MF_STRING, 2, TranslateT("Delete"));

		POINT pt;
		GetCursorPos(&pt);
		int cmd = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
		DestroyMenu(hMenu);

		if (cmd == 1) { // rename
			ENTER_STRING es = {};
			es.szModuleName = m_proto->m_szModuleName;
			es.caption = TranslateT("Enter new group name");
			if (!EnterString(&es))
				return;

			m_proto->Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/renameGroup")
				<< AIMSID(m_proto) << WCHAR_PARAM("oldGroup", pGroup->wszSrvName) << GROUP_PARAM("newGroup", es.ptszResult));

			mir_free(es.ptszResult);
		}
		else if (cmd == 2) { // delete
			m_proto->Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/removeGroup")
				<< AIMSID(m_proto) << WCHAR_PARAM("group", pGroup->wszSrvName));
		}
	}
};

CGroupEditDlg *CGroupEditDlg::pDlg = nullptr;

INT_PTR CIcqProto::EditGroups(WPARAM, LPARAM)
{
	(new CGroupEditDlg(this))->Show();
	return 0;
}

INT_PTR CIcqProto::EditProfile(WPARAM, LPARAM)
{
	if (mir_wstrlen(m_szOwnId))
		Utils_OpenUrlW(CMStringW(FORMAT, L"https://icq.com/people/%s/edit/", (wchar_t*)m_szOwnId));
	return 0;
}

void RefreshGroups(void)
{
	if (CGroupEditDlg::pDlg != nullptr)
		CGroupEditDlg::pDlg->RefreshGroups();
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CIcqProto::GetEmailCount(WPARAM, LPARAM)
{
	if (!m_bOnline)
		return 0;
	return m_unreadEmails;
}

INT_PTR CIcqProto::GotoInbox(WPARAM, LPARAM)
{
	Utils_OpenUrl("https://e.mail.ru/messages/inbox");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::SendMarkRead()
{
	mir_cslock lck(m_csMarkReadQueue);
	while (m_arMarkReadQueue.getCount()) {
		IcqCacheItem *pUser = m_arMarkReadQueue[0];

		auto *pReq = new AsyncRapiRequest(this, "setDlgStateWim");
		pReq->params << WCHAR_PARAM("sn", GetUserId(pUser->m_hContact)) << INT64_PARAM("lastRead", getId(pUser->m_hContact, DB_KEY_LASTMSGID));
		Push(pReq);

		m_arMarkReadQueue.remove(0);
	}
}

int CIcqProto::OnDbEventRead(WPARAM, LPARAM hDbEvent)
{
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	MarkAsRead(hContact);
	return 0;
}

int CIcqProto::OnGroupChange(WPARAM hContact, LPARAM lParam)
{
	if (!m_bOnline)
		return 0;

	CLISTGROUPCHANGE *pParam = (CLISTGROUPCHANGE*)lParam;
	if (hContact == 0) { // whole group is changed
		if (pParam->pszOldName == nullptr) {
			for (auto &it : m_arGroups)
				if (it->wszName == pParam->pszNewName)
					return 0;

			Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/addGroup")
				<< AIMSID(this) << GROUP_PARAM("group", pParam->pszNewName));
		}
		else if (pParam->pszNewName == nullptr) {
			Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/removeGroup") 
				<< AIMSID(this) << GROUP_PARAM("group", pParam->pszOldName));
		}
		else {
			Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/renameGroup") 
				<< AIMSID(this) << GROUP_PARAM("oldGroup", pParam->pszOldName) << GROUP_PARAM("newGroup", pParam->pszNewName));
		}
	}
	else MoveContactToGroup(hContact, ptrW(getWStringA(hContact, "IcqGroup")), pParam->pszNewName);
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_AddToList - adds a contact to the contact list

MCONTACT CIcqProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	if (mir_wstrlen(psr->id.w) == 0)
		return 0;

	MCONTACT hContact = CreateContact(psr->id.w, true);
	if (psr->nick.w)
		setWString(hContact, "Nick", psr->nick.w);
	if (psr->firstName.w)
		setWString(hContact, "FirstName", psr->firstName.w);
	if (psr->lastName.w)
		setWString(hContact, "LastName", psr->lastName.w);

	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int CIcqProto::AuthRecv(MCONTACT, PROTORECVEVENT *pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int CIcqProto::AuthRequest(MCONTACT hContact, const wchar_t* szMessage)
{
	ptrW wszGroup(Clist_GetGroup(hContact));
	if (!wszGroup)
		wszGroup = mir_wstrdup(L"General");

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/buddylist/addBuddy", &CIcqProto::OnAddBuddy);
	pReq << AIMSID(this) << WCHAR_PARAM("authorizationMsg", szMessage) << WCHAR_PARAM("buddy", GetUserId(hContact)) << WCHAR_PARAM("group", wszGroup) << INT_PARAM("preAuthorized", 1);
	pReq->hContact = hContact;
	Push(pReq);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// File operations

HANDLE CIcqProto::FileAllow(MCONTACT, HANDLE hTransfer, const wchar_t *pwszSavePath)
{
	if (!m_bOnline)
		return nullptr;

	auto *ft = (IcqFileTransfer *)hTransfer;
	ft->m_wszFileName.Insert(0, pwszSavePath);
	ft->pfts.szCurrentFile.w = ft->m_wszFileName.GetBuffer();

	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_GET, ft->m_szHost, &CIcqProto::OnFileRecv);
	pReq->pUserInfo = ft;
	pReq->AddHeader("Sec-Fetch-User", "?1");
	pReq->AddHeader("Sec-Fetch-Site", "cross-site");
	pReq->AddHeader("Sec-Fetch-Mode", "navigate");
	Push(pReq);
	
	return hTransfer;
}

int CIcqProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, hTransfer);

	auto *ft = (IcqFileTransfer *)hTransfer;
	if (ft->pfts.currentFileTime != 0)
		ft->m_bCanceled = true;
	else
		delete ft;
	return 0;
}

int CIcqProto::FileResume(HANDLE hTransfer, int, const wchar_t *szFilename)
{
	auto *ft = (IcqFileTransfer *)hTransfer;
	if (!m_bOnline || ft == nullptr)
		return 1;

	if (szFilename != nullptr) {
		ft->m_wszFileName = szFilename;
		ft->pfts.szCurrentFile.w = ft->m_wszFileName.GetBuffer();
	}

	::SetEvent(ft->hWaitEvent);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

INT_PTR CIcqProto::GetCaps(int type, MCONTACT)
{
	INT_PTR nReturn = 0;

	switch (type) {
	case PFLAGNUM_1:
		nReturn = PF1_IM | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES | /*PF1_SEARCHBYNAME | TODO */
			PF1_VISLIST | PF1_FILE | PF1_CONTACT | PF1_SERVERCLIST;
		break;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_INVISIBLE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_INVISIBLE;
	
	case PFLAGNUM_5:
		return PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_INVISIBLE;

	case PFLAGNUM_4:
		nReturn = PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_OFFLINEFILES | PF4_IMSENDOFFLINE | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SERVERMSGID | PF4_READNOTIFY;
		break;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("UIN/e-mail/phone");
	}

	return nReturn;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int CIcqProto::GetInfo(MCONTACT hContact, int)
{
	RetrieveUserInfo(hContact);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by UID

HANDLE CIcqProto::SearchBasic(const wchar_t *pszSearch)
{
	if (!m_bOnline)
		return nullptr;

	auto *pReq = new AsyncRapiRequest(this, "search", &CIcqProto::OnSearchResults);
	pReq->params << WCHAR_PARAM(*pszSearch == '+' ? "phonenum" : "keyword", pszSearch);
	Push(pReq);

	return pReq;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE CIcqProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	// we can't send more than one file at a time
	if (ppszFiles[1] != 0)
		return nullptr;

	struct _stat statbuf;
	if (_wstat(ppszFiles[0], &statbuf)) {
		debugLogW(L"'%s' is an invalid filename", ppszFiles[0]);
		return nullptr;
	}

	int iFileId = _wopen(ppszFiles[0], _O_RDONLY | _O_BINARY, _S_IREAD);
	if (iFileId < 0)
		return nullptr;

	auto *pTransfer = new IcqFileTransfer(hContact, ppszFiles[0]);
	pTransfer->pfts.totalFiles = 1;
	pTransfer->pfts.currentFileSize = pTransfer->pfts.totalBytes = statbuf.st_size;
	pTransfer->m_fileId = iFileId;
	if (mir_wstrlen(szDescription))
		pTransfer->m_wszDescr = szDescription;

	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_GET, "https://files.icq.com/files/init", &CIcqProto::OnFileInit);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("f", "json") << WCHAR_PARAM("filename", pTransfer->m_wszShortName) 
		<< CHAR_PARAM("k", appId()) << INT_PARAM("size", statbuf.st_size) << INT_PARAM("ts", TS());
	CalcHash(pReq);
	pReq->pUserInfo = pTransfer;
	Push(pReq);

	return pTransfer; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SendMessage - sends a message

int CIcqProto::SendMsg(MCONTACT hContact, int, const char *pszSrc)
{
	CMStringA szUserid(GetUserId(hContact));
	if (szUserid.IsEmpty())
		return 0;

	int id = InterlockedIncrement(&m_msgId);
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/im/sendIM", &CIcqProto::OnSendMessage);

	auto *pOwn = new IcqOwnMessage(hContact, id, pReq->m_reqId);
	pReq->pUserInfo = pOwn;
	{
		mir_cslock lck(m_csOwnIds);
		m_arOwnIds.insert(pOwn);
	}

	pReq << AIMSID(this) << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", appId()) << CHAR_PARAM("mentions", "") 
		<< CHAR_PARAM("message", pszSrc) << CHAR_PARAM("offlineIM", "true") << CHAR_PARAM("t", szUserid) << INT_PARAM("ts", TS());
	Push(pReq);
	return id;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetStatus - sets the protocol status

int CIcqProto::SetStatus(int iNewStatus)
{
	debugLogA("CIcqProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %p", iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread);

	if (iNewStatus == m_iStatus)
		return 0;

	m_iDesiredStatus = iNewStatus;
	int iOldStatus = m_iStatus;

	// go offline
	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_bOnline)
			SetServerStatus(ID_STATUS_OFFLINE);

		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE, false);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (!m_bOnline && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);

		if (mir_wstrlen(m_szOwnId) == 0) {
			debugLogA("Thread ended, UIN/password are not configured");
			ConnectionFailed(LOGINERR_BADUSERID);
			return 0;
		}

		if (!RetrievePassword()) {
			debugLogA("Thread ended, password is not configured");
			ConnectionFailed(LOGINERR_BADUSERID);
			return 0;
		}

		CheckPassword();
	}
	else if (m_bOnline) {
		debugLogA("setting server online status to %d", iNewStatus);
		SetServerStatus(iNewStatus);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PS_UserIsTyping - sends a UTN notification

int CIcqProto::UserIsTyping(MCONTACT hContact, int type)
{
	Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/im/setTyping")
		<< AIMSID(this) << WCHAR_PARAM("t", GetUserId(hContact)) << CHAR_PARAM("typingStatus", (type == PROTOTYPE_SELFTYPING_ON) ? "typing" : "typed"));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetApparentMode - sets the visibility status

int CIcqProto::SetApparentMode(MCONTACT hContact, int iMode)
{
	int oldMode = getWord(hContact, "ApparentMode");
	if (oldMode != iMode) {
		setWord(hContact, "ApparentMode", iMode);
		SetPermitDeny(GetUserId(hContact), iMode != ID_STATUS_OFFLINE);
	}
	return 0;
}
