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
{	return strcmp(p1->szMsgId, p2->szMsgId);
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
	m_bUrlPreview(this, "UrlPreview", true),
	m_bCompressFiles(this, "CompressFiles", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	m_iOwnId = GetId(0);

	CreateProtoService(PS_GETAVATARCAPS, &CTelegramProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFO, &CTelegramProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CTelegramProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CTelegramProto::SvcSetMyAvatar);
	
	CreateProtoService(PS_MENU_LOADHISTORY, &CTelegramProto::SvcLoadServerHistory);
	CreateProtoService(PS_CAN_EMPTY_HISTORY, &CTelegramProto::SvcCanEmptyHistory);
	CreateProtoService(PS_EMPTY_SRV_HISTORY, &CTelegramProto::SvcEmptyServerHistory);

	HookProtoEvent(ME_OPT_INITIALISE, &CTelegramProto::OnOptionsInit);
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CTelegramProto::OnWindowEvent);

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
	nlu.flags = NUF_OUTGOING | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_DATABASE | GC_PERSISTENT;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	CreateProtoService(PS_LEAVECHAT, &CTelegramProto::SvcLeaveChat);

	HookProtoEvent(ME_GC_MUTE, &CTelegramProto::GcMuteHook);
	HookProtoEvent(ME_GC_EVENT, &CTelegramProto::GcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CTelegramProto::GcMenuHook);
}

CTelegramProto::~CTelegramProto()
{
}

void CTelegramProto::OnCacheInit()
{
	int iCompatLevel = getByte(DBKEY_COMPAT);
	VARSW cachePath(L"%miranda_userdata%\\ChatCache");

	for (auto &cc : AccContacts()) {
		m_bCacheInited = true;

		if (int64_t id = GetId(cc)) {
			bool isGroupChat = isChatRoom(cc);
			auto *pUser = new TG_USER(id, cc, isGroupChat);
			pUser->szAvatarHash = getMStringA(cc, DBKEY_AVATAR_HASH);
			m_arUsers.insert(pUser);
			if (isGroupChat && iCompatLevel < 3)
				_wremove(CMStringW(FORMAT, L"%s\\%d.json", cachePath.get(), cc));
		}
	}
	setByte(DBKEY_COMPAT, 3);
}

void CTelegramProto::OnContactAdded(MCONTACT hContact)
{
	if (int64_t id = GetId(hContact)) {
		auto *pUser = new TG_USER(id, hContact, isChatRoom(hContact));
		pUser->szAvatarHash = getMStringA(hContact, DBKEY_AVATAR_HASH);
		m_arUsers.insert(pUser);
	}
}

bool CTelegramProto::OnContactDeleted(MCONTACT hContact, uint32_t flags)
{
	TD::int53 id = GetId(hContact);
	if (id == 0)
		return false;

	if (auto *pUser = FindUser(id)) {
		if (pUser->m_si) {
			SvcLeaveChat(hContact, 0);
			return false;
		}

		pUser->hContact = INVALID_CONTACT_ID;
		pUser->wszNick = getMStringW(hContact, "Nick");
		pUser->wszFirstName = getMStringW(hContact, "FirstName");
		pUser->wszLastName = getMStringW(hContact, "LastName");
	}

	if (flags & CDF_DEL_CONTACT) {
		TD::array<TD::int53> ids;
		ids.push_back(id);
		SendQuery(new TD::removeContacts(std::move(ids)));
	}
	return true;
}

void CTelegramProto::OnModulesLoaded()
{
	m_bSmileyAdd = ServiceExists(MS_SMILEYADD_REPLACESMILEYS);
	if (m_bSmileyAdd)
		SmileyAdd_LoadContactSmileys(SMADD_FOLDER, m_szModuleName, GetAvatarPath() + L"\\Stickers\\*.*");
}

void CTelegramProto::OnShutdown()
{
	m_bTerminated = true;

	for (auto &cc : m_arUsers)
		if (cc->isBot && !cc->chatId && cc->hContact != INVALID_CONTACT_ID)
			Contact::RemoveFromList(cc->hContact);
}

int CTelegramProto::OnWindowEvent(WPARAM wParam, LPARAM lParam)
{
	if (wParam == MSG_WINDOW_EVT_OPENING) {
		auto *pDlg = (CMsgDialog *)lParam;
		if (Proto_IsProtoOnContact(pDlg->m_hContact, m_szModuleName))
			if (auto *pUser = FindUser(GetId(pDlg->m_hContact)))
				if (pUser->chatId == -1 && !pDlg->isChat())
					SendQuery(new TD::createPrivateChat(pUser->id, true));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	// Groups uploader
	mi.pszService = "/AddByPhone";
	CreateProtoService(mi.pszService, &CTelegramProto::SvcAddByPhone);
	mi.name.a = LPGEN("Add phone contact");
	mi.position = 200001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CTelegramProto::OnErase()
{
	DeleteDirectoryTreeW(GetProtoFolder(), false);
}

void CTelegramProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags)
{
	if (!hContact || !(flags & CDF_DEL_HISTORY))
		return;

	auto *pUser = FindUser(GetId(hContact));
	if (!pUser)
		return;

	DB::EventInfo dbei(hDbEvent, false);
	if (!dbei.szId)
		return;

	mir_cslock lck(m_csDeleteMsg);
	if (m_deleteChatId) {
		if (m_deleteChatId != pUser->chatId)
			SendDeleteMsg();

		m_impl.m_deleteMsg.Stop();
	}

	m_bDeleteForAll = (flags & CDF_FOR_EVERYONE) != 0;
	m_deleteChatId = pUser->chatId;
	m_deleteIds.push_back(dbei2id(dbei));
	m_impl.m_deleteMsg.Start(500);
}

void CTelegramProto::OnEventEdited(MCONTACT hContact, MEVENT, const DBEVENTINFO &dbei)
{
	if (!hContact)
		return;

	auto *pUser = FindUser(GetId(hContact));
	if (!pUser)
		return;

	if (dbei.szId && dbei.cbBlob && dbei.pBlob && dbei.eventType == EVENTTYPE_MESSAGE) {
		auto text = formatBbcodes((char*)dbei.pBlob);
		auto content = TD::make_object<TD::inputMessageText>(std::move(text), false, false);
		SendQuery(new TD::editMessageText(pUser->chatId, dbei2id(dbei), 0, std::move(content)));
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
		m_markIds.push_back(dbei2id(dbei));
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

	if (auto *pChat = FindUser(id)) {
		if (pChat->isGroupChat) {
			SendQuery(new TD::joinChat(pChat->chatId));
			return pUser->hContact;
		}
	}

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

INT_PTR CTelegramProto::GetCaps(int type, MCONTACT hContact)
{
	uint32_t ret;

	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_CHAT | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES | PF1_MODEMSGRECV | PF1_SERVERCLIST;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY;

	case PFLAGNUM_4:
		ret = PF4_NOCUSTOMAUTH | PF4_FORCEAUTH | PF4_OFFLINEFILES | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS 
			| PF4_SERVERMSGID | PF4_REPLY | PF4_GROUPCHATFILES;
		if (GetId(hContact) != m_iOwnId)
			ret |= PF4_DELETEFORALL;
		return ret;
		
	case PFLAGNUM_5:
		return PF2_SHORTAWAY | PF2_LONGAWAY;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)L"ID";

	default:
		return 0;
	}
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

void CTelegramProto::ProcessFileMessage(TG_FILE_REQUEST *ft, const TD::message *pMsg, bool bCreateEvent)
{
	if (auto *pUser = FindChat(pMsg->chat_id_)) {
		const TD::MessageContent *pBody = pMsg->content_.get();

		TD::file *pFile;
		switch (pBody->get_id()) {
		case TD::messagePhoto::ID:
			pFile = ((TD::messagePhoto *)pBody)->photo_->sizes_[0]->photo_.get();
			break;

		case TD::messageAudio::ID:
			pFile = ((TD::messageAudio *)pBody)->audio_->audio_.get();
			break;

		case TD::messageVideo::ID:
			pFile = ((TD::messageVideo *)pBody)->video_->video_.get();
			break;

		case TD::messageDocument::ID:
			pFile = ((TD::messageDocument *)pBody)->document_->document_.get();
			break;

		default: 
			return;
		}

		char szUserId[100];
		auto szMsgId(msg2id(pMsg));

		auto *pOwnMsg = new TG_OWN_MESSAGE(pUser->hContact, 0, szMsgId);
		pOwnMsg->tmpFileId = pFile->id_;
		m_arOwnMsg.insert(pOwnMsg);

		if (!GetGcUserId(pUser, pMsg, szUserId))
			szUserId[0] = 0;

		if (bCreateEvent) {
			auto *pFileName = pFile->local_->path_.c_str();
			CMStringA szDescr = GetMessageText(pUser, pMsg);

			DB::EventInfo dbei;
			dbei.szModule = Proto_GetBaseAccountName(ft->m_hContact);
			dbei.eventType = EVENTTYPE_FILE;
			dbei.flags = DBEF_SENT | DBEF_UTF;
			dbei.timestamp = time(0);

			TG_FILE_REQUEST localft(TG_FILE_REQUEST::FILE, 0, 0);
			localft.m_fileName = Utf2T(pFileName);
			localft.m_fileSize = pFile->size_;
			localft.m_uniqueId = szMsgId;
			localft.m_szUserId = szUserId;

			DB::FILE_BLOB blob(localft.m_fileName, ft->m_wszDescr);
			OnSendOfflineFile(dbei, blob, &localft);
			blob.write(dbei);

			db_event_add(ft->m_hContact, &dbei);
		}
		else {
			ft->m_szUserId = szUserId;
			ft->m_uniqueId = szMsgId;
			ProtoBroadcastAck(ft->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft);
		}
	}
}

void CTelegramProto::OnSendFile(td::ClientManager::Response &response, void *pUserInfo)
{
	auto *ft = (TG_FILE_REQUEST *)pUserInfo;

	if (response.object->get_id() == TD::message::ID) {
		ProcessFileMessage(ft, (TD::message *)response.object.get(), false);
		ProtoBroadcastAck(ft->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft);
	}
	else if (response.object->get_id() == TD::messages::ID) {
		int i = 0;
		auto *pMessages = (TD::messages *)response.object.get();
		for (auto &it : pMessages->messages_) {
			ProcessFileMessage(ft, it.get(), i != 0);
			i++;
		}
	}

	delete ft;
}

HANDLE CTelegramProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	auto *pUser = FindUser(GetId(hContact));
	if (pUser == nullptr) {
		debugLogA("request from unknown contact %d, ignored", hContact);
		return nullptr;
	}

	struct FileItem
	{
		wchar_t *pwszName;
		long iSize;

		FileItem(wchar_t *_1, long _2) :
			pwszName(_1), iSize(_2)
		{}
	};

	OBJLIST<FileItem> arFiles(1);

	for (int i = 0; ppszFiles[i] != 0; i++) {
		struct _stat statbuf;
		if (_wstat(ppszFiles[0], &statbuf)) {
			debugLogW(L"'%s' is an invalid filename", ppszFiles[i]);
			continue;
		}

		arFiles.insert(new FileItem(ppszFiles[i], statbuf.st_size));
	}

	if (!arFiles.getCount()) {
		debugLogA("No files to be sent");
		return nullptr;
	}

	if (arFiles.getCount() > 10) {
		debugLogA("Too many files to be sent");
		return nullptr;
	}

	// create a message with embedded file
	TD::sendMessage *pMessage = nullptr;
	TD::sendMessageAlbum *pAlbum = nullptr;

	if (arFiles.getCount() == 1) {
		pMessage = new TD::sendMessage();
		pMessage->chat_id_ = pUser->chatId;
	}
	else {
		pAlbum = new TD::sendMessageAlbum();
		pAlbum->chat_id_ = pUser->chatId;
	}

	auto *ft = new TG_FILE_REQUEST(TG_FILE_REQUEST::FILE, 0, 0);
	ft->m_hContact = hContact;
	ft->m_fileName = arFiles[0].pwszName;
	ft->m_fileSize = arFiles[0].iSize;
	ft->m_wszDescr = szDescription;

	for (auto &it: arFiles) {
		auto iFileType = (m_bCompressFiles) ? AutoDetectType(it->pwszName) : TG_FILE_REQUEST::FILE;

		auto caption = formatBbcodes(T2Utf(szDescription));
		TD::object_ptr<TD::InputMessageContent> pPart;

		if (iFileType == TG_FILE_REQUEST::FILE) {
			auto pContent = TD::make_object<TD::inputMessageDocument>();
			pContent->document_= makeFile(it->pwszName);
			pContent->caption_ = std::move(caption);
			pContent->thumbnail_ = 0;
			pPart = std::move(pContent);
		}
		else if (iFileType == TG_FILE_REQUEST::PICTURE) {
			auto pContent = TD::make_object<TD::inputMessagePhoto>();
			pContent->photo_ = makeFile(it->pwszName);
			pContent->thumbnail_ = 0;
			pContent->caption_ = std::move(caption);
			pContent->height_ = 0;
			pContent->width_ = 0;
			pPart = std::move(pContent);
		}
		else if (iFileType == TG_FILE_REQUEST::VOICE) {
			auto pContent = TD::make_object<TD::inputMessageVoiceNote>();
			pContent->voice_note_ = makeFile(it->pwszName);
			pContent->caption_ = std::move(caption);
			pContent->duration_ = 0;
			pPart = std::move(pContent);
		}
		else if (iFileType == TG_FILE_REQUEST::VIDEO) {
			auto pContent = TD::make_object<TD::inputMessageVideo>();
			pContent->video_ = makeFile(it->pwszName);
			pContent->caption_ = std::move(caption);
			pContent->duration_ = 0;
			pContent->height_ = 0;
			pContent->width_ = 0;
			pPart = std::move(pContent);
		}
		else return nullptr;

		if (pMessage)
			pMessage->input_message_content_ = std::move(pPart);
		else
			pAlbum->input_message_contents_.push_back(std::move(pPart));
	}

	if (pMessage)
		SendQuery(pMessage, &CTelegramProto::OnSendFile, ft);
	else
		SendQuery(pAlbum, &CTelegramProto::OnSendFile, ft);

	return ft;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTelegramProto::SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *pszMessage)
{
	ptrA szId(getStringA(hContact, DBKEY_ID));
	if (szId == nullptr)
		return 0;

	__int64 id = _atoi64(szId);
	auto *pUser = FindUser(id);
	if (pUser == nullptr)
		return 0;

	TD::int53 iReplyId = 0;
	if (hReplyEvent) {
		DB::EventInfo dbei(hReplyEvent, false);
		if (dbei)
			iReplyId = dbei2id(dbei);
	}

	int msgid = SendTextMessage(pUser->chatId, 0, iReplyId, pszMessage);
	if (msgid != -1)
		m_arOwnMsg.insert(new TG_OWN_MESSAGE(hContact, (HANDLE)msgid, ""));

	return msgid;
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

int CTelegramProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (auto *pUser = FindUser(GetId(hContact)))
		if (type == PROTOTYPE_SELFTYPING_ON)
			SendQuery(new TD::sendChatAction(pUser->chatId, 0, TD::make_object<TD::chatActionTyping>()));

	return 0;
}
