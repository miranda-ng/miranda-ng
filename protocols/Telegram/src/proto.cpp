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

static int CompareOwnMsg(const TG_OWN_MESSAGE *p1, const TG_OWN_MESSAGE *p2)
{	return CompareId(p1->tmpMsgId, p2->tmpMsgId);
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
	m_arFiles(1, PtrKeySortT),
	m_arChats(100, CompareChats),
	m_arUsers(100, CompareUsers),
	m_arOwnMsg(1, CompareOwnMsg),
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
	m_bCompressFiles(this, "CompressFiles", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	m_iOwnId = GetId(0);

	CreateProtoService(PS_GETAVATARCAPS, &CTelegramProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFO, &CTelegramProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CTelegramProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CTelegramProto::SvcSetMyAvatar);
	CreateProtoService(PS_MENU_LOADHISTORY, &CTelegramProto::SvcLoadServerHistory);

	HookProtoEvent(ME_HISTORY_EMPTY, &CTelegramProto::OnEmptyHistory);
	HookProtoEvent(ME_OPT_INITIALISE, &CTelegramProto::OnOptionsInit);

	// avatar
	CreateDirectoryTreeW(GetAvatarPath());

	// default contacts group
	m_iBaseGroup = Clist_GroupCreate(0, m_wszDefaultGroup);

	// menus
	InitMenus();

	// Cloud file transfer
	CreateProtoService(PS_OFFLINEFILE, &CTelegramProto::SvcOfflineFile);

	// create standard network connection
	NETLIBUSER nlu = {};
	nlu.flags = NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_DATABASE | GC_PERSISTENT;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_MUTE, &CTelegramProto::GcMuteHook);
	HookProtoEvent(ME_GC_EVENT, &CTelegramProto::GcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CTelegramProto::GcMenuHook);
}

CTelegramProto::~CTelegramProto()
{
}

void CTelegramProto::OnContactDeleted(MCONTACT hContact)
{
	TD::int53 id = GetId(hContact);
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
		if (auto *pUser = FindUser(GetId(hContact)))
			SendQuery(new TD::deleteChatHistory(pUser->chatId, true, true));
	}

	return 0;
}

void CTelegramProto::OnModulesLoaded()
{
	int iCompatLevel = getByte(DBKEY_COMPAT);
	VARSW cachePath(L"%miranda_userdata%\\ChatCache");

	for (auto &cc : AccContacts()) {
		if (int64_t id = GetId(cc)) {
			bool isGroupChat = isChatRoom(cc);
			auto *pUser = new TG_USER(id, cc, isGroupChat);
			pUser->szAvatarHash = getMStringA(cc, DBKEY_AVATAR_HASH);
			m_arUsers.insert(pUser);
			if (!isGroupChat)
				m_arChats.insert(pUser);
			else if (iCompatLevel < 3)
				_wremove(CMStringW(FORMAT, L"%s\\%d.json", cachePath.get(), cc));
		}
	}
	setByte(DBKEY_COMPAT, 3);

	m_bSmileyAdd = ServiceExists(MS_SMILEYADD_REPLACESMILEYS);
	if (m_bSmileyAdd)
		SmileyAdd_LoadContactSmileys(SMADD_FOLDER, m_szModuleName, GetAvatarPath() + L"\\Stickers\\*.*");
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
	CreateProtoService(mi.pszService, &CTelegramProto::SvcAddByPhone);
	mi.name.a = LPGEN("Add phone contact");
	mi.position = 200001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CTelegramProto::OnErase()
{
	if (getByte(DBKEY_AUTHORIZED)) {
		m_bUnregister = true;
		ServerThread(0);
	}

	DeleteDirectoryTreeW(GetProtoFolder(), false);
}

void CTelegramProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent)
{
	if (!hContact)
		return;

	auto *pUser = FindUser(GetId(hContact));
	if (!pUser)
		return;

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (dbei.szId) {
		mir_cslock lck(m_csDeleteMsg);
		if (m_deleteChatId) {
			if (m_deleteChatId != pUser->chatId)
				SendDeleteMsg();

			m_impl.m_deleteMsg.Stop();
		}

		m_deleteChatId = pUser->chatId;
		m_deleteIds.push_back(_atoi64(dbei.szId));
		m_impl.m_deleteMsg.Start(500);
	}
}

void CTelegramProto::OnEventEdited(MCONTACT hContact, MEVENT, const DBEVENTINFO &dbei)
{
	if (!hContact)
		return;

	auto *pUser = FindUser(GetId(hContact));
	if (!pUser)
		return;

	if (dbei.szId && dbei.cbBlob && dbei.eventType == EVENTTYPE_MESSAGE) {
		auto text = TD::make_object<TD::formattedText>();
		text->text_ = (char*)dbei.pBlob;

		auto content = TD::make_object<TD::inputMessageText>(std::move(text), false, false);
		SendQuery(new TD::editMessageText(pUser->chatId, _atoi64(dbei.szId), 0, std::move(content)));
	}
}

void CTelegramProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (!hContact)
		return;

	auto *pUser = FindUser(GetId(hContact));
	if (!pUser)
		return;

	DB::EventInfo dbei(hDbEvent, false);
	if (dbei && dbei.szId) {
		mir_cslock lck(m_csMarkRead);
		if (m_markChatId) {
			if (m_markChatId != hContact)
				SendMarkRead();

			m_impl.m_markRead.Stop();
		}

		m_markChatId = pUser->chatId;
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

int CTelegramProto::AuthRequest(MCONTACT hContact, const wchar_t *)
{
	auto *pUser = FindUser(GetId(hContact));
	if (pUser == nullptr)
		return 1;  // error

	auto cc = TD::make_object<TD::contact>();
	cc->user_id_ = pUser->id;
	if (!pUser->wszFirstName.IsEmpty())
		cc->first_name_ = T2Utf(pUser->wszFirstName);
	else
		cc->first_name_ = getMStringU(hContact, "FirstName").c_str();

	if (!pUser->wszLastName.IsEmpty())
		cc->last_name_ = T2Utf(pUser->wszLastName);
	else
		cc->last_name_ = getMStringU(hContact, "LastName").c_str();
	
	SendQuery(new TD::addContact(std::move(cc), false));
	return 0;
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

MEVENT CTelegramProto::RecvFile(MCONTACT hContact, PROTORECVFILE *pre)
{
	MEVENT hEvent = CSuper::RecvFile(hContact, pre);
	if (hEvent)
		if (auto *ft = (TG_FILE_REQUEST *)pre->lParam) {
			DBVARIANT dbv = { DBVT_DWORD };
			dbv.dVal = ft->m_type;
			db_event_setJson(hEvent, "t", &dbv);
		}

	return hEvent;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnSearchResults(td::ClientManager::Response &response)
{
	int iCount = ::InterlockedDecrement(&m_iSearchCount);
	if (iCount == 0)
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

	if (iCount == 0)
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, this);
}

HANDLE CTelegramProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	CMStringA szQuery(FORMAT, "%s %s %s", T2Utf(nick).get(), T2Utf(firstName).get(), T2Utf(lastName).get());
	if (szQuery.GetLength() == 2)
		return nullptr;

	m_iSearchCount = 2;
	szQuery.Trim();
	SendQuery(new TD::searchPublicChats(szQuery.c_str()), &CTelegramProto::OnSearchResults);
	SendQuery(new TD::searchChatsOnServer(szQuery.c_str(), 100), &CTelegramProto::OnSearchResults);
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE CTelegramProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	auto *pUser = FindUser(GetId(hContact));
	if (pUser == nullptr) {
		debugLogA("request from unknown contact %d, ignored", hContact);
		return nullptr;
	}

	TG_FILE_REQUEST *pTransfer = nullptr;

	for (int i = 0; ppszFiles[i] != 0; i++) {
		struct _stat statbuf;
		if (_wstat(ppszFiles[0], &statbuf)) {
			debugLogW(L"'%s' is an invalid filename", ppszFiles[i]);
			continue;
		}

		pTransfer = new TG_FILE_REQUEST(TG_FILE_REQUEST::FILE, 0, "");
		pTransfer->m_fileName = ppszFiles[i];
		if (m_bCompressFiles)
			pTransfer->AutoDetectType();
		
		pTransfer->m_hContact = hContact;
		pTransfer->m_fileSize = statbuf.st_size;
		if (mir_wstrlen(szDescription))
			pTransfer->m_wszDescr = szDescription;

		// create a message with embedded file
		auto *pMessage = new TD::sendMessage();
		pMessage->chat_id_ = pUser->chatId;
		auto caption = TD::make_object<TD::formattedText>();
		caption->text_ = T2Utf(szDescription).get();

		if (pTransfer->m_type == TG_FILE_REQUEST::FILE) {
			auto pContent = TD::make_object<TD::inputMessageDocument>();
			pContent->document_= makeFile(pTransfer->m_fileName);
			pContent->caption_ = std::move(caption);
			pContent->thumbnail_ = 0;
			pMessage->input_message_content_ = std::move(pContent);
		}
		else if (pTransfer->m_type == TG_FILE_REQUEST::PICTURE) {
			auto pContent = TD::make_object<TD::inputMessagePhoto>();
			pContent->photo_ = makeFile(pTransfer->m_fileName);
			pContent->thumbnail_ = 0;
			pContent->caption_ = std::move(caption);
			pContent->height_ = 0;
			pContent->width_ = 0;
			pMessage->input_message_content_ = std::move(pContent);
		}
		else if (pTransfer->m_type == TG_FILE_REQUEST::VOICE) {
			auto pContent = TD::make_object<TD::inputMessageVoiceNote>();
			pContent->voice_note_ = makeFile(pTransfer->m_fileName);
			pContent->caption_ = std::move(caption);
			pContent->duration_ = 0;
			pMessage->input_message_content_ = std::move(pContent);
		}
		else if (pTransfer->m_type == TG_FILE_REQUEST::VIDEO) {
			auto pContent = TD::make_object<TD::inputMessageVideo>();
			pContent->video_ = makeFile(pTransfer->m_fileName);
			pContent->caption_ = std::move(caption);
			pContent->duration_ = 0;
			pContent->height_ = 0;
			pContent->width_ = 0;
			pMessage->input_message_content_ = std::move(pContent);
		}
		else return nullptr;

		SendQuery(pMessage, &CTelegramProto::OnSendFile, pTransfer);
	}

	return pTransfer;
}

void CTelegramProto::OnSendFile(td::ClientManager::Response &response, void *pUserInfo)
{
	auto *ft = (TG_FILE_REQUEST *)pUserInfo;

	if (response.object->get_id() == TD::message::ID) {
		auto *pMsg = (TD::message *)response.object.get();
		ft->m_uniqueId.Format("%lld", pMsg->id_);

		if (auto *pUser = FindChat(pMsg->chat_id_)) {
			char szUserId[100];
			if (this->GetGcUserId(pUser, pMsg, szUserId))
				ft->m_szUserId = szUserId;

			auto *pOwnMsg = new TG_OWN_MESSAGE(pUser->hContact, 0, pMsg->id_);
			const TD::MessageContent *pBody = pMsg->content_.get();
			switch (pBody->get_id()) {
			case TD::messagePhoto::ID:
				pOwnMsg->tmpFileId = ((TD::messagePhoto*)pBody)->photo_->sizes_[0]->photo_->id_;
				break;

			case TD::messageDocument::ID:
				pOwnMsg->tmpFileId = ((TD::messageDocument *)pBody)->document_->document_->id_;
				break;
			}
			m_arOwnMsg.insert(pOwnMsg);
		}
	}

	ProtoBroadcastAck(ft->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft);
	delete ft;  
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTelegramProto::SendMsg(MCONTACT hContact, const char *pszMessage)
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
