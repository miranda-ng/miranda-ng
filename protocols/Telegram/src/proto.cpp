#include "stdafx.h"

int __forceinline CompareId(int64_t id1, int64_t id2)
{
	if (id1 == id2)
		return 0;

	return (id1 < id2) ? -1 : 1;
}

static int CompareRequests(const TG_REQUEST_BASE *p1, const TG_REQUEST_BASE *p2)
{
	if (p1->requestId == p2->requestId)
		return 0;

	return (p1->requestId < p2->requestId) ? -1 : 1;
}

static int CompareChats(const TG_USER *p1, const TG_USER *p2)
{	return CompareId(p1->chatId, p2->chatId);
}

static int CompareUsers(const TG_USER *p1, const TG_USER *p2)
{	return CompareId(p1->id, p2->id);
}

static int CompareBasicGroups(const TG_BASIC_GROUP *p1, const TG_BASIC_GROUP *p2)
{	return CompareId(p1->id, p2->id);
}

static int CompareSuperGroups(const TG_SUPER_GROUP *p1, const TG_SUPER_GROUP *p2)
{	return CompareId(p1->id, p2->id);
}

CTelegramProto::CTelegramProto(const char* protoName, const wchar_t* userName) :
	PROTO<CTelegramProto>(protoName, userName),
	m_impl(*this),
	m_arFiles(1),
	m_arChats(100, CompareChats),
	m_arUsers(100, CompareUsers),
	m_arRequests(10, CompareRequests),
	m_arBasicGroups(10, CompareBasicGroups),
	m_arSuperGroups(10, CompareSuperGroups),
	m_szOwnPhone(this, "Phone"), 
	m_iCountry(this, "Country", 9999),
	m_iStatus1(this, "Status1", ID_STATUS_AWAY),
	m_iStatus2(this, "Status2", ID_STATUS_NA),
	m_iTimeDiff1(this, "TimeDiff1", 600),
	m_iTimeDiff2(this, "TimeDiff2", 600),
	m_wszDeviceName(this, "DeviceName", L"Miranda NG"),
	m_wszDefaultGroup(this, "DefaultGroup", L"Telegram"),
	m_bUsePopups(this, "UsePopups", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	m_iOwnId = _atoi64(getMStringA(DBKEY_ID));

	CreateProtoService(PS_GETAVATARCAPS, &CTelegramProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFO, &CTelegramProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CTelegramProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CTelegramProto::SvcSetMyAvatar);

	HookProtoEvent(ME_HISTORY_EMPTY, &CTelegramProto::OnEmptyHistory);
	HookProtoEvent(ME_OPT_INITIALISE, &CTelegramProto::OnOptionsInit);
	
	// avatar
	CreateDirectoryTreeW(GetAvatarPath());

	// default contacts group
	m_iBaseGroup = Clist_GroupCreate(0, m_wszDefaultGroup);

	// create standard network connection
	NETLIBUSER nlu = {};
	nlu.flags = NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_DATABASE;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &CTelegramProto::GcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CTelegramProto::GcMenuHook);
}

CTelegramProto::~CTelegramProto()
{
}

void CTelegramProto::OnContactDeleted(MCONTACT hContact)
{
	TD::int53 id = _atoi64(getMStringA(hContact, DBKEY_ID));
	if (id == 0)
		return;

	TD::array<TD::int53> ids;
	ids.push_back(id);
	SendQuery(new TD::removeContacts(std::move(ids)));

	if (auto *pUser = FindUser(id)) {
		pUser->hContact = INVALID_CONTACT_ID;
		pUser->wszFirstName = getMStringW(hContact, "FirstName");
		pUser->wszLastName = getMStringW(hContact, "LastName");
	}
}

int CTelegramProto::OnEmptyHistory(WPARAM hContact, LPARAM)
{
	if (Proto_IsProtoOnContact(hContact, m_szModuleName)) {
		TD::int53 id = _atoi64(getMStringA(hContact, DBKEY_ID));
		if (auto *pUser = FindUser(id))
			SendQuery(new TD::deleteChatHistory(pUser->chatId, true, true));
	}

	return 0;
}

void CTelegramProto::OnModulesLoaded()
{
	CMStringA szId(getMStringA(DBKEY_ID));
	if (!szId.IsEmpty()) {
		auto *pUser = new TG_USER(_atoi64(szId.c_str()), 0);
		m_arUsers.insert(pUser);
		m_arChats.insert(pUser);
	}

	for (auto &cc : AccContacts()) {
		ptrA szPath(getStringA(cc, "AvatarPath"));
		if (szPath) {
			delSetting(cc, "AvatarPath");
			delSetting(cc, DBKEY_AVATAR_HASH);
		}

		bool isGroupChat = isChatRoom(cc);
		szId = getMStringA(cc, DBKEY_ID);
		if (!szId.IsEmpty()) {
			auto *pUser = new TG_USER(_atoi64(szId.c_str()), cc, isGroupChat);
			pUser->szAvatarHash = getMStringA(cc, DBKEY_AVATAR_HASH);
			m_arUsers.insert(pUser);
			if (!isGroupChat)
				m_arChats.insert(pUser);
		}
	}

	m_bSmileyAdd = ServiceExists(MS_SMILEYADD_LOADCONTACTSMILEYS);
	if (m_bSmileyAdd) {
		CMStringW wszStickersPath(GetAvatarPath() + L"\\Stickers\\*.*");
		SMADD_CONT cont = {2, m_szModuleName, wszStickersPath};
		CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));
	}
}

void CTelegramProto::OnShutdown()
{
	m_bTerminated = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	// Groups uploader
	mi.pszService = "/UploadGroups";
	CreateProtoService(mi.pszService, &CTelegramProto::AddByPhone);
	mi.name.a = LPGEN("Add phone contact");
	mi.position = 200001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CTelegramProto::OnErase()
{
	m_bUnregister = true;
	ServerThread(0);

	DeleteDirectoryTreeW(GetProtoFolder(), false);
}

void CTelegramProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent)
{
	if (!hContact)
		return;

	ptrA userId(getStringA(hContact, DBKEY_ID));
	if (!userId)
		return;

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (dbei.szId) {
		mir_cslock lck(m_csDeleteMsg);
		if (m_deleteMsgContact) {
			if (m_deleteMsgContact != hContact)
				SendDeleteMsg();

			m_impl.m_deleteMsg.Stop();
		}

		m_deleteMsgContact = hContact;
		m_deleteIds.push_back(_atoi64(dbei.szId));
		m_impl.m_deleteMsg.Start(500);
	}
}

void CTelegramProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (!hContact)
		return;

	ptrA userId(getStringA(hContact, DBKEY_ID));
	if (!userId)
		return;

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (dbei.szId) {
		mir_cslock lck(m_csMarkRead);
		if (m_markContact) {
			if (m_markContact != hContact)
				SendMarkRead();

			m_impl.m_markRead.Stop();
		}

		m_markContact = hContact;
		m_markIds.push_back(_atoi64(dbei.szId));
		m_impl.m_markRead.Start(500);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CTelegramProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT) && psr->id.w == nullptr)
		return 0;

	auto id = _wtoi64(psr->id.w);
	auto *pUser = AddUser(id, false);
	if (flags & PALF_TEMPORARY)
		Contact::RemoveFromList(pUser->hContact);

	auto cc = TD::make_object<TD::contact>(); 
	cc->user_id_ = id;
	if (psr->firstName.w)
		cc->first_name_ = T2Utf(psr->firstName.w);
	if (psr->lastName.w)
		cc->last_name_ = T2Utf(psr->lastName.w);
	SendQuery(new TD::addContact(std::move(cc), false));
	return pUser->hContact;
}

INT_PTR CTelegramProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_CHAT | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES | PF1_MODEMSGRECV | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SERVERMSGID;
	case PFLAGNUM_5:
		return PF2_SHORTAWAY | PF2_LONGAWAY;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)L"ID";
	default:
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE CTelegramProto::FileAllow(MCONTACT, HANDLE hTransfer, const wchar_t *pwszSavePath)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;
	if (ft == nullptr)
		return nullptr;

	ft->m_destPath = pwszSavePath;
	ft->pfts.szCurrentFile.w = ft->m_fileName.GetBuffer();
	ft->pfts.szWorkingDir.w = ft->m_destPath.GetBuffer();
	SendQuery(new TD::downloadFile(ft->m_fileId, 5, 0, 0, false));
	return ft;
}

int CTelegramProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;
	delete ft;
	return 1;
}

int CTelegramProto::FileResume(HANDLE hTransfer, int, const wchar_t *pwszFilename)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;
	if (ft == nullptr)
		return 1;

	ft->m_destPath = pwszFilename;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnSearchResults(td::ClientManager::Response &response)
{
	m_searchIds.clear();

	if (!response.object)
		return;

	if (response.object->get_id() != TD::chats::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::chats::ID);
		return;
	}

	auto *pChats = ((TD::chats*)response.object.get());
	if (pChats->total_count_) {
		for (auto &it : pChats->chat_ids_) {
			if (auto *pUser = FindChat(it))
				ReportSearchUser(pUser);
			else
				m_searchIds.push_back(it);
		}
	}

	if (m_searchIds.empty())
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, this);
}

HANDLE CTelegramProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	CMStringA szQuery(FORMAT, "%s %s %s", T2Utf(nick).get(), T2Utf(firstName).get(), T2Utf(lastName).get());
	if (szQuery.GetLength() == 2)
		return nullptr;

	szQuery.Trim();
	SendQuery(new TD::searchPublicChats(szQuery.c_str()), &CTelegramProto::OnSearchResults);
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTelegramProto::SendMsg(MCONTACT hContact, int, const char *pszMessage)
{
	ptrA szId(getStringA(hContact, DBKEY_ID));
	if (szId == nullptr)
		return 0;

	return SendTextMessage(_atoi64(szId), pszMessage);
}

int CTelegramProto::SetStatus(int iNewStatus)
{
	if (m_iDesiredStatus == iNewStatus)
		return 0;
	
	int oldStatus = m_iStatus;

	// Routing statuses not supported by Telegram
	switch (iNewStatus) {
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = iNewStatus;
		break;

	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
	default:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		if (isRunning())
			SendQuery(new TD::close());

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (!isRunning() && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

		ForkThread(&CTelegramProto::ServerThread);
	}
	else if (isRunning()) {
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}
