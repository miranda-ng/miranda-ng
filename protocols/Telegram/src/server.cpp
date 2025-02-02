/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CTelegramProto::OnEndSession(td::ClientManager::Response&)
{
	m_bUnregister = false;
	m_bTerminated = true;
	delSetting(DBKEY_AUTHORIZED);
}

void __cdecl CTelegramProto::ServerThread(void *)
{
	m_botIds.clear();

	bool bWasAuthorized = getBool(DBKEY_AUTHORIZED);
	m_bTerminated = m_bAuthorized = false;
	m_pClientManager = std::make_unique<td::ClientManager>();
	m_iClientId = m_pClientManager->create_client_id();

	SendQuery(new TD::getOption("version"));

	NETLIBUSERSETTINGS nluSettings;
	Netlib_GetUserSettings(m_hNetlibUser, &nluSettings);
	if (nluSettings.useProxy) {
		TD::object_ptr<TD::ProxyType> proxyType;
		switch (nluSettings.proxyType) {
		case PROXYTYPE_SOCKS4:
		case PROXYTYPE_SOCKS5:
			proxyType = TD::make_object<TD::proxyTypeSocks5>();
			break;
		case PROXYTYPE_HTTP:
		case PROXYTYPE_HTTPS:
			proxyType = TD::make_object<TD::proxyTypeHttp>();
			break;
		}

		if (proxyType)
			SendQuery(new TD::addProxy(nluSettings.szProxyServer, nluSettings.wProxyPort, true, std::move(proxyType)));
	}

	while (!m_bTerminated)
		ProcessResponse(m_pClientManager->receive(1));

	m_pClientManager = std::move(nullptr);

	if (!bWasAuthorized && !getBool(DBKEY_AUTHORIZED))
		OnErase();
}

void CTelegramProto::UnregisterSession()
{
	if (getByte(DBKEY_AUTHORIZED)) {
		m_bUnregister = true;

		if (m_pClientManager) {
			SendQuery(new TD::terminateSession());
			SendQuery(new TD::logOut(), &CTelegramProto::OnEndSession);
		}
		else ForkThread(&CTelegramProto::ServerThread);
	}
}

void CTelegramProto::LogOut()
{
	if (m_bTerminated)
		return;

	debugLogA("CTelegramProto::OnLoggedOut");
	m_bTerminated = true;
	m_bAuthorized = false;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setWord(m_iSavedMessages, "Status", ID_STATUS_OFFLINE);

	m_impl.m_keepAlive.Stop();
	setAllContactStatuses(ID_STATUS_OFFLINE, false);
	for (auto &it : m_arUsers)
		it->m_si = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnGetChats(td::ClientManager::Response &response)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::chats::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::chats::ID);
		return;
	}

	auto *pChats = (TD::chats *)response.object.get();
	for (auto &it : pChats->chat_ids_) {
		if (auto *pUser = FindChat(it))
			Contact::PutOnList(pUser->hContact);
		else
			SendQuery(new TD::getChat(it));
	}
}

void CTelegramProto::OnLoggedIn()
{
	m_bAuthorized = true;

	debugLogA("CTelegramProto::OnLoggedIn");

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	setWord(m_iSavedMessages, "Status", ID_STATUS_ONLINE);

	if (m_bUnregister) {
		SendQuery(new TD::terminateSession());
		SendQuery(new TD::logOut(), &CTelegramProto::OnEndSession);
	}
	else {
		m_impl.m_keepAlive.Start(1000);
		setByte(DBKEY_AUTHORIZED, 1);

		SendQuery(new TD::getChats(td::tl::unique_ptr<TD::chatListMain>(), 1000), &CTelegramProto::OnGetChats);
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::SendKeepAlive()
{
	time_t now = time(0);
	int iDiff1 = m_iTimeDiff1, iDiff2 = m_iTimeDiff2;

	for (auto &it : m_arUsers) {
		if (it->m_timer1 && now - it->m_timer1 > iDiff1) {
			it->m_timer1 = 0;

			// if the second status is set in the options, enable the second timer
			if (m_iTimeDiff2) {
				it->m_timer2 = now;
				setWord(it->hContact, "Status", m_iStatus2);
			}
			else setWord(it->hContact, "Status", ID_STATUS_OFFLINE);
		}
		else if (it->m_timer2 && now - it->m_timer2 > iDiff2) {
			it->m_timer2 = 0;
			setWord(it->hContact, "Status", ID_STATUS_OFFLINE);
		}
	}
}

void CTelegramProto::SendDeleteMsg()
{
	m_impl.m_deleteMsg.Stop();

	mir_cslock lck(m_csDeleteMsg);
	SendQuery(new TD::deleteMessages(m_deleteChatId, std::move(m_deleteIds), m_bDeleteForAll));
	m_deleteChatId = 0;
}

void CTelegramProto::SendMarkRead()
{
	m_impl.m_markRead.Stop();

	mir_cslock lck(m_csMarkRead);
	SendQuery(new TD::viewMessages(m_markChatId, std::move(m_markIds), 0, true));
	m_markChatId = 0;
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ProcessResponse(td::ClientManager::Response response)
{
	if (!response.object)
		return;

	debugLogA("ProcessResponse: id=%d (%s)", int(response.request_id), to_string(response.object).c_str());

	if (response.request_id) {
		TG_REQUEST tmp(response.request_id, 0);
		mir_cslock lck(m_csRequests);
		auto *p = m_arRequests.find(&tmp);
		if (p) {
			p->Execute(this, response);
			m_arRequests.remove(p);
		}
		return;
	}

	switch (response.object->get_id()) {
	case TD::updateActiveEmojiReactions::ID:
		ProcessActiveEmoji((TD::updateActiveEmojiReactions *)response.object.get());
		break;

	case TD::updateAuthorizationState::ID:
		ProcessAuth((TD::updateAuthorizationState *)response.object.get());
		break;

	case TD::updateBasicGroup::ID:
		ProcessBasicGroup((TD::updateBasicGroup*)response.object.get());
		break;

	case TD::updateBasicGroupFullInfo::ID:
		ProcessBasicGroupInfo((TD::updateBasicGroupFullInfo *)response.object.get());
		break;

	case TD::updateChatAction::ID:
		ProcessChatAction((TD::updateChatAction *)response.object.get());
		break;

	case TD::updateChatAvailableReactions::ID:
		ProcessChatReactions((TD::updateChatAvailableReactions *)response.object.get());
		break;

	case TD::updateChatFolders::ID:
		ProcessGroups((TD::updateChatFolders *)response.object.get());
		break;

	case TD::updateChatHasProtectedContent::ID:
		ProcessChatHasProtected((TD::updateChatHasProtectedContent *)response.object.get());
		break;

	case TD::updateChatLastMessage::ID:
		ProcessChatLastMessage((TD::updateChatLastMessage *)response.object.get());
		break;

	case TD::updateChatNotificationSettings::ID:
		ProcessChatNotification((TD::updateChatNotificationSettings*)response.object.get());
		break;

	case TD::updateChatPosition::ID:
		ProcessChatPosition((TD::updateChatPosition *)response.object.get());
		break;

	case TD::updateChatReadInbox::ID:
		ProcessMarkRead((TD::updateChatReadInbox *)response.object.get());
		break;

	case TD::updateChatReadOutbox::ID:
		ProcessRemoteMarkRead((TD::updateChatReadOutbox *)response.object.get());
		break;

	case TD::updateDeleteMessages::ID:
		ProcessDeleteMessage((TD::updateDeleteMessages*)response.object.get());
		break;
		
	case TD::updateConnectionState::ID:
		ProcessConnectionState((TD::updateConnectionState *)response.object.get());
		break;

	case TD::updateFile::ID:
		ProcessFile((TD::updateFile *)response.object.get());
		break;

	case TD::updateForumTopicInfo::ID:
		ProcessForum((TD::updateForumTopicInfo*)response.object.get());
		break;

	case TD::updateMessageContent::ID:
		ProcessMessageContent((TD::updateMessageContent *)response.object.get());
		break;

	case TD::updateMessageInteractionInfo::ID:
		ProcessMessageReactions((TD::updateMessageInteractionInfo *)response.object.get());
		break;

	case TD::updateMessageSendSucceeded::ID:
		{
			auto *pUpdate = (TD::updateMessageSendSucceeded *)response.object.get();
			auto *pMessage = pUpdate->message_.get();

			auto szOldId = msg2id(pMessage->chat_id_, pUpdate->old_message_id_);
			if (pUpdate->old_message_id_)
				if (auto hDbEvent = db_event_getById(m_szModuleName, szOldId))
					db_event_updateId(hDbEvent, msg2id(pMessage));

			TG_OWN_MESSAGE tmp(0, 0, szOldId);
			if (auto *pOwnMsg = m_arOwnMsg.find(&tmp)) {
				auto szMsgId = msg2id(pMessage);
				if (pOwnMsg->hAck)
					ProtoBroadcastAck(pOwnMsg->hContact ? pOwnMsg->hContact : m_iSavedMessages, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, pOwnMsg->hAck, (LPARAM)szMsgId.c_str());

				if (auto hDbEvent = db_event_getById(m_szModuleName, szMsgId))
					db_event_delivered(pOwnMsg->hContact, hDbEvent);
				m_arOwnMsg.remove(pOwnMsg);
			}

			ProcessMessage(pMessage);
		}
		break;

	case TD::updateNewChat::ID:
		ProcessChat((TD::updateNewChat *)response.object.get());
		break;

	case TD::updateNewMessage::ID:
		{
			auto *pMessage = ((TD::updateNewMessage *)response.object.get())->message_.get();
			TG_OWN_MESSAGE tmp(0, 0, msg2id(pMessage));
			if (!m_arOwnMsg.find(&tmp))
				ProcessMessage(pMessage);
		}
		break;

	case TD::updateOption::ID:
		ProcessOption((TD::updateOption *)response.object.get());
		break;

	case TD::updateScopeNotificationSettings::ID:
		ProcessScopeNotification((TD::updateScopeNotificationSettings *)response.object.get());
		break;

	case TD::updateServiceNotification::ID:
		ProcessServiceNotification((TD::updateServiceNotification *)response.object.get());
		break;

	case TD::updateSupergroup::ID:
		ProcessSuperGroup((TD::updateSupergroup *)response.object.get());
		break;

	case TD::updateSupergroupFullInfo::ID:
		ProcessSuperGroupInfo((TD::updateSupergroupFullInfo *)response.object.get());
		break;

	case TD::updateUserStatus::ID:
		ProcessStatus((TD::updateUserStatus *)response.object.get());
		break;

	case TD::updateUser::ID:
		ProcessUser((TD::updateUser *)response.object.get());
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnSendMessage(td::ClientManager::Response &response)
{
	if (!response.object)
		return;

	switch (response.object->get_id()) {
	case TD::error::ID:
		for (auto &it : m_arOwnMsg)
			if (it->hAck == (HANDLE)response.request_id) {
				auto *pError = ((TD::error *)response.object.get());
				CMStringW wszMsg(FORMAT, TranslateT("Error %d: %s"), pError->code_, TranslateW(Utf2T(pError->message_.c_str())));
				ProtoBroadcastAck(it->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, it->hAck, (LPARAM)wszMsg.c_str());
				break;
			}
		break;

	case TD::message::ID:
		for (auto &it : m_arOwnMsg)
			if (it->hAck == (HANDLE)response.request_id) {
				auto *pMessage = ((TD::message *)response.object.get());
				it->szMsgId = msg2id(pMessage);
				break;
			}
		break;

	default:
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::message::ID);
		return;
	}
}

int CTelegramProto::SendTextMessage(int64_t chatId, int64_t threadId, int64_t replyId, const char *pszMessage)
{
	auto pContent = TD::make_object<TD::inputMessageText>();
	pContent->text_ = formatBbcodes(pszMessage);

	auto *pMessage = new TD::sendMessage();
	pMessage->chat_id_ = chatId;
	pMessage->input_message_content_ = std::move(pContent);
	pMessage->message_thread_id_ = threadId;
	if (replyId)
		pMessage->reply_to_.reset(new TD::inputMessageReplyToMessage(replyId, 0));
	return SendQuery(pMessage, &CTelegramProto::OnSendMessage);
}

int CTelegramProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	if (!m_pClientManager)
		return -1;

	int queryId = ++m_iQueryId;

	auto szDescr = to_string(*pFunc);
	debugLogA("Sending query %d:\n%s", queryId, szDescr.c_str());

	m_pClientManager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler) {
		mir_cslock lck(m_csRequests);
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
	}
	return queryId;
}

int CTelegramProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo)
{
	if (!m_pClientManager)
		return -1;

	int queryId = ++m_iQueryId;

	auto szDescr = to_string(*pFunc);
	debugLogA("Sending full query %d:\n%s", queryId, szDescr.c_str());

	m_pClientManager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler) {
		mir_cslock lck(m_csRequests);
		m_arRequests.insert(new TG_REQUEST_FULL(queryId, pHandler, pUserInfo));
	}
	return queryId;
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnGetHistory(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::messages::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::messages::ID);
		return;
	}

	auto *pUser = (TG_USER *)pUserInfo;
	TD::int53 lastMsgId = INT64_MAX;
	auto *pMessages = (TD::messages *)response.object.get();
	for (auto &it : pMessages->messages_) {
		auto *pMsg = it.get();
		if (pMsg->id_ < lastMsgId)
			lastMsgId = pMsg->id_;

		char szUserId[100];
		auto szMsgId(msg2id(pMsg));
		if (db_event_getById(m_szModuleName, szMsgId))
			continue;

		CMStringA szBody = GetMessageText(pUser, pMsg, true, true), szReplyId;
		if (szBody.IsEmpty())
			continue;

		DBEVENTINFO dbei = {};
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		dbei.iTimestamp = pMsg->date_;
		dbei.cbBlob = szBody.GetLength();
		dbei.pBlob = szBody.GetBuffer();
		dbei.szId = szMsgId;
		dbei.flags = DBEF_READ | DBEF_UTF;
		if (pMsg->is_outgoing_)
			dbei.flags |= DBEF_SENT;
		if (this->GetGcUserId(pUser, pMsg, szUserId))
			dbei.szUserId = szUserId;
		if (auto iReplyId = getReplyId(pMsg->reply_to_.get())) {
			szReplyId = msg2id(pMsg->chat_id_, iReplyId);
			dbei.szReplyId = szReplyId;
		}
		db_event_add(GetRealContact(pUser), &dbei);
	}

	pUser->nHistoryChunks--;

	if (pUser->isForum) {
		if (lastMsgId != INT64_MAX && pUser->nHistoryChunks > 0)
			SendQuery(new TD::getMessageThreadHistory(pUser->chatId, lastMsgId, lastMsgId, 0, 100), &CTelegramProto::OnGetHistory, pUser);
		else
			delete pUser;
	}
	else if (lastMsgId != INT64_MAX && pUser->nHistoryChunks > 0)
		SendQuery(new TD::getChatHistory(pUser->chatId, lastMsgId, 0, 100, false), &CTelegramProto::OnGetHistory, pUser);
}

INT_PTR CTelegramProto::SvcLoadServerHistory(WPARAM hContact, LPARAM)
{
	TD::int53 lastMsgId = 0;

	if (MEVENT hEvent = db_event_first(hContact)) {
		DB::EventInfo dbei(hEvent, false);
		lastMsgId = dbei2id(dbei);
	}

	auto userId = GetId(hContact);

	if (TD::int53 threadId = GetId(hContact, DBKEY_THREAD)) {
		if (FindChat(userId)) {
			auto *pUser = new TG_USER(-1, hContact, true);
			pUser->chatId = userId;
			pUser->isForum = pUser->isGroupChat = true;
			pUser->nHistoryChunks = 5;
			SendQuery(new TD::getMessageThreadHistory(pUser->chatId, lastMsgId, lastMsgId, 0, 100), &CTelegramProto::OnGetHistory, pUser);
			return 0;
		}
	}
	
	if (auto *pUser = FindUser(userId)) {
		pUser->nHistoryChunks = 5;
		SendQuery(new TD::getChatHistory(pUser->chatId, lastMsgId, 0, 100, false), &CTelegramProto::OnGetHistory, pUser);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

INT_PTR CTelegramProto::SvcCanEmptyHistory(WPARAM hContact, LPARAM bIncoming)
{
	if (auto *pUser = FindUser(GetId(hContact))) {
		TG_SUPER_GROUP tmp(pUser->id, 0);
		if (auto *pGroup = m_arSuperGroups.find(&tmp))
			if (pGroup->group->is_channel_)
				return 0;

		if (!pUser->bDelOwn || (bIncoming && !pUser->bDelAll))
			return 0;
		
		return 1;
	}

	return 0;
}

INT_PTR CTelegramProto::SvcEmptyServerHistory(WPARAM hContact, LPARAM lParam)
{
	if (auto *pUser = FindUser(GetId(hContact)))
		if (pUser->chatId != -1)
			SendQuery(new TD::deleteChatHistory(pUser->chatId, false, (lParam & CDF_FOR_EVERYONE) != 0));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ProcessChat(TD::updateNewChat *pObj)
{
	int64_t userId;
	auto *pChat = pObj->chat_.get();
	std::string szTitle;
	bool isChannel = false;
	
	switch (pChat->type_->get_id()) {
	case TD::chatTypePrivate::ID:
	case TD::chatTypeSecret::ID:
		userId = pChat->id_;
		break;

	case TD::chatTypeBasicGroup::ID:
		{
			auto *pGroup = (TD::chatTypeBasicGroup *)pChat->type_.get();
			userId = pGroup->basic_group_id_;
		}
		szTitle = pChat->title_;
		break;

	case TD::chatTypeSupergroup::ID:
		{
			auto *pGroup = (TD::chatTypeSupergroup *)pChat->type_.get();
			userId = pGroup->supergroup_id_;
			isChannel = pGroup->is_channel_;
		}
		szTitle = pChat->title_;
		break;

	default:
		debugLogA("Invalid chat type %d, ignoring", pChat->type_->get_id());
		return;
	}

	auto *pUser = FindUser(userId);
	if (pUser == nullptr) {
		debugLogA("Unknown user id %lld for chat %lld, ignoring", userId, pChat->id_);
		return;
	}

	pUser->chatId = pChat->id_;
	pUser->isChannel = isChannel;
	pUser->bDelAll = pChat->can_be_deleted_for_all_users_;
	pUser->bDelOwn = pChat->can_be_deleted_only_for_self_;

	MCONTACT hContact = (pUser->id == m_iOwnId) ? 0 : pUser->hContact;

	if (!m_arChats.find(pUser))
		m_arChats.insert(pUser);

	if (!szTitle.empty()) {
		if (hContact != INVALID_CONTACT_ID) {
			if (pUser->isForum) {
				pUser->wszNick = Utf2T(szTitle.c_str());
				SendQuery(new TD::getForumTopics(pUser->chatId, "", 0, 0, 0, 100), &CTelegramProto::OnGetTopics, pUser);
			}
			else GcChangeTopic(pUser, szTitle);
		}
		else if (pUser->wszNick.IsEmpty())
			pUser->wszFirstName = Utf2T(szTitle.c_str());
	}

	if (auto *pPhoto = pChat->photo_.get())
		ProcessAvatar(pPhoto->small_.get(), pUser);

	if (CheckSearchUser(pUser))
		return;

	if (pUser->hContact != INVALID_CONTACT_ID) {
		if (pChat->has_protected_content_)
			setByte(pUser->hContact, "Protected", 1);
		else
			delSetting(pUser->hContact, "Protected");

		if (pChat->permissions_)
			Contact::Readonly(hContact, !pChat->permissions_->can_send_basic_messages_);

		if (pUser->isGroupChat && pUser->m_si == nullptr)
			InitGroupChat(pUser, (pUser->isForum) ? TranslateT("General") : Utf2T(pChat->title_.c_str()));
	}
}

void CTelegramProto::ProcessChatAction(TD::updateChatAction *pObj)
{
	auto *pChat = FindChat(pObj->chat_id_);
	if (pChat == nullptr) {
		debugLogA("Unknown chat, skipping");
		return;
	}

	if (pChat->hContact == INVALID_CONTACT_ID) {
		debugLogA("Last message for a temporary contact, skipping");
		return;
	}

	auto hContact = GetRealContact(pChat);
	switch (pObj->action_->get_id()) {
	case TD::chatActionRecordingVideo::ID:
		Srmm_SetStatusText(hContact, TranslateT("Recording video..."));
		break;
	case TD::chatActionRecordingVideoNote::ID:
		Srmm_SetStatusText(hContact, TranslateT("Recording video note..."));
		break;
	case TD::chatActionRecordingVoiceNote::ID:
		Srmm_SetStatusText(hContact, TranslateT("Recording voice note..."));
		break;
	case TD::chatActionUploadingDocument::ID:
		Srmm_SetStatusText(hContact, TranslateT("Uploading file..."));
		break;
	case TD::chatActionUploadingPhoto::ID:
		Srmm_SetStatusText(hContact, TranslateT("Uploading photo..."));
		break;
	case TD::chatActionUploadingVideo::ID:
		Srmm_SetStatusText(hContact, TranslateT("Uploading video..."));
		break;
	case TD::chatActionUploadingVideoNote::ID:
		Srmm_SetStatusText(hContact, TranslateT("Uploading video note..."));
		break;
	case TD::chatActionUploadingVoiceNote::ID:
		Srmm_SetStatusText(hContact, TranslateT("Uploading voice note..."));
		break;
	case TD::chatActionTyping::ID:
		if (!pChat->isGroupChat)
			CallService(MS_PROTO_CONTACTISTYPING, pChat->hContact, 30);
		break;
	case TD::chatActionCancel::ID:
		if (!pChat->isGroupChat) {
			Srmm_SetStatusText(hContact, 0);
			CallService(MS_PROTO_CONTACTISTYPING, pChat->hContact, PROTOTYPE_CONTACTTYPING_OFF);
		}
		break;
	}
}

void CTelegramProto::ProcessChatHasProtected(TD::updateChatHasProtectedContent *pObj)
{
	if (auto *pChat = FindChat(pObj->chat_id_)) {
		if (pObj->has_protected_content_)
			setByte(pChat->hContact, "Protected", 1);
		else
			delSetting(pChat->hContact, "Protected");
	}
}

void CTelegramProto::ProcessChatLastMessage(TD::updateChatLastMessage *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("Unknown chat, skipping");
		return;
	}

	pUser->bInited = true;
	if (pUser->hContact == INVALID_CONTACT_ID) {
		debugLogA("Last message for a temporary contact, skipping");
		return;
	}
}

void CTelegramProto::ProcessChatNotification(TD::updateChatNotificationSettings *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr || pUser->hContact == INVALID_CONTACT_ID)
		return;

	auto &pSettings = pObj->notification_settings_;

	bool bNever = Chat_IsMuted(pUser->hContact) == CHATMODE_UNMUTE;
	TD::int32 muteFor;
	if (!pSettings->use_default_mute_for_)
		muteFor = pSettings->mute_for_;
	else
		muteFor = GetDefaultMute(pUser);
	Chat_Mute(pUser->hContact, muteFor ? CHATMODE_MUTE : (bNever ? CHATMODE_UNMUTE : CHATMODE_NORMAL));

	memcpy(&pUser->notificationSettings, pSettings.get(), sizeof(pUser->notificationSettings));
}

void CTelegramProto::ProcessChatPosition(TD::updateChatPosition *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("Unknown chat, skipping");
		return;
	}

	if (pUser->hContact == INVALID_CONTACT_ID) {
		debugLogA("Temporary contact, skipping");
		return;
	}

	auto *pPos = (TD::chatPosition *)pObj->position_.get();
	if (auto *pList = pPos->list_.get()) {
		CMStringW wszGroup;

		switch (auto typeId = pList->get_id()) {
		case TD::chatListArchive::ID:
			wszGroup = TranslateT("Archive");
			break;

		case TD::chatListMain::ID:  // leave group empty
			if (pUser->folderId != -1)
				return;
			break;

		case TD::chatListFolder::ID:
			{
				int iFolderId = ((TD::chatListFolder *)pList)->chat_folder_id_;
				CMStringA szSetting(FORMAT, "ChatFilter%d", iFolderId);
				wszGroup = getMStringW(szSetting);
				if (wszGroup.IsEmpty()) {
					debugLogA("Empty group name for group #%d, ignored", iFolderId);
					return;
				}
				if (wszGroup == "Unread")
					return;
				pUser->folderId = iFolderId;
			}
			break;

		default:
			debugLogA("Unknown position type ID %d, ignored", typeId);
			return;
		}

		MCONTACT hContact = GetRealContact(pUser);
		ptrW pwszExistingGroup(Clist_GetGroup(hContact));
		debugLogW(L"Existing contact group <%s>, calculated <%s>", pwszExistingGroup.get(), wszGroup.c_str());

		wchar_t *pwszDefaultGroup = m_wszDefaultGroup;
		if (!pwszExistingGroup || pUser->isForum
			|| !mir_wstrncmp(pwszExistingGroup, pwszDefaultGroup, mir_wstrlen(pwszDefaultGroup))
			|| (pUser->isGroupChat && !mir_wstrcmp(pwszExistingGroup, Chat_GetGroup())))
		{
			CMStringW wszNewGroup(pwszDefaultGroup);
			if (!wszGroup.IsEmpty())
				wszNewGroup.AppendFormat(L"\\%s", wszGroup.c_str());
			if (pUser->isForum)
				wszNewGroup.AppendFormat(L"\\%s", pUser->wszNick.c_str());

			debugLogW(L"Setting group for %d to %s", hContact, wszNewGroup.c_str());
			Clist_GroupCreate(0, wszNewGroup);
			Clist_SetGroup(hContact, wszNewGroup);
		}
	}
}

void CTelegramProto::ProcessChatReactions(TD::updateChatAvailableReactions *pObj)
{
	if (pObj->available_reactions_->get_id() != TD::chatAvailableReactionsSome::ID) {
		debugLogA("Unsupported reactions type: %d", pObj->available_reactions_->get_id());
		return;
	}

	auto &pReactions = ((TD::chatAvailableReactionsSome *)pObj->available_reactions_.get())->reactions_;

	if (auto *pChat = FindChat(pObj->chat_id_)) {
		if (!pChat->pReactions)
			pChat->pReactions = new OBJLIST<char>(1);
		else
			pChat->pReactions->destroy();

		for (auto &it : pReactions) {
			if (it->get_id() != TD::reactionTypeEmoji::ID)
				continue;

			auto *pEmoji = (TD::reactionTypeEmoji *)it.get();
			auto &str = pEmoji->emoji_;
			pChat->pReactions->insert(mir_strcpy(new char[str.length() + 1], str.c_str()));
		}

		if (pChat->pReactions->getCount() == 0) {
			delete pChat->pReactions;
			pChat->pReactions = nullptr;
		}
	}
}

void CTelegramProto::ProcessConnectionState(TD::updateConnectionState *pObj)
{
	pConnState = std::move(pObj->state_);

	switch (pConnState->get_id()) {
	case TD::connectionStateConnecting::ID:
		debugLogA("Connection state: connecting");
		break;

	case TD::connectionStateConnectingToProxy::ID:
		debugLogA("Connection state: connecting to proxy");
		break;

	case TD::connectionStateWaitingForNetwork::ID:
		debugLogA("Connection state: waiting for network");
		break;

	case TD::connectionStateUpdating::ID:
		debugLogA("Connection state: updating");
		break;

	case TD::connectionStateReady::ID:
		debugLogA("Connection state: connected");
		if (pAuthState->get_id() == TD::authorizationStateReady::ID)
			OnLoggedIn();
		break;
	}
}

void CTelegramProto::ProcessActiveEmoji(TD::updateActiveEmojiReactions *pObj)
{
	m_defaultEmoji.Empty();

	for (auto &it : pObj->emojis_)
		m_defaultEmoji.AppendFormat("%s ", it.c_str());

	m_defaultEmoji.TrimRight();
}

void CTelegramProto::ProcessDeleteMessage(TD::updateDeleteMessages *pObj)
{
	if (!pObj->is_permanent_)
		return;

	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr || pUser->hContact == INVALID_CONTACT_ID) {
		debugLogA("message from unknown chat, ignored");
		return;
	}

	for (auto &it : pObj->message_ids_)
		if (MEVENT hEvent = db_event_getById(m_szModuleName, msg2id(pObj->chat_id_, it)))
			db_event_delete(hEvent, CDF_FROM_SERVER);
}

void CTelegramProto::ProcessGroups(TD::updateChatFolders *pObj)
{
	for (auto &grp : pObj->chat_folders_) {
		if (grp->icon_->name_ != "Custom")
			continue;

		CMStringA szSetting(FORMAT, "ChatFilter%d", grp->id_);
		CMStringW wszOldValue(getMStringW(szSetting));
		Utf2T wszNewValue(grp->title_.c_str());
		if (wszOldValue.IsEmpty()) {
			Clist_GroupCreate(m_iBaseGroup, wszNewValue);
			setWString(szSetting, wszNewValue);
		}
		else if (wszOldValue != wszNewValue) {
			CMStringW wszFullGroup(FORMAT, L"%s\\%s", (wchar_t *)m_wszDefaultGroup, wszNewValue.get());
			MGROUP oldGroup = Clist_GroupExists(wszFullGroup);
			if (!oldGroup)
				Clist_GroupCreate(m_iBaseGroup, wszFullGroup);
			else
				Clist_GroupRename(oldGroup, wszFullGroup);
			setWString(szSetting, wszNewValue);
		}
	}
}

void CTelegramProto::ProcessMarkRead(TD::updateChatReadInbox *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	if (pObj->last_read_inbox_message_id_)
		pUser->bInited = true;

	CMStringA szMaxId(msg2id(pObj->chat_id_, pObj->last_read_inbox_message_id_));
	if (db_event_getById(m_szModuleName, szMaxId) == 0) {
		debugLogA("unknown event, ignored");
		return;
	}

	// make sure that all events with ids lower or equal than szMaxId are marked read
	MarkRead(pUser->hContact, szMaxId, false);

	if (g_plugin.hasMessageState && pObj->unread_count_ == 0)
		CallService(MS_MESSAGESTATE_UPDATE, GetRealContact(pUser), MRD_TYPE_READ);

	if (Contact::IsGroupChat(pUser->hContact) && pObj->unread_count_ == 0)
		if (pUser->m_si)
			pUser->m_si->markRead(true);
}

void CTelegramProto::ProcessMessage(const TD::message *pMessage)
{
	auto *pUser = FindChat(pMessage->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	if (pMessage->sending_state_)
		if (pMessage->sending_state_->get_id() == TD::messageSendingStatePending::ID)
			return;

	char szUserId[100];
	auto szMsgId(msg2id(pMessage));
	MEVENT hOldEvent = db_event_getById(m_szModuleName, szMsgId);

	CMStringA szText(GetMessageText(pUser, pMessage)), szReplyId;
	if (szText.IsEmpty()) {
		debugLogA("this message was not processed, ignored");
		return;
	}

	// make a temporary contact if needed
	if (pUser->hContact == INVALID_CONTACT_ID) {
		if (pUser->isGroupChat) {
			debugLogA("spam from unknown group chat, ignored");
			return;
		}

		AddUser(pUser->id, false);
		Contact::RemoveFromList(pUser->hContact);
	}

	MCONTACT hContact = GetRealContact(pUser, pMessage->message_thread_id_);

	if (m_bResidentChannels && pUser->isChannel && pUser->m_si) {
		GCEVENT gce = { pUser->m_si, GC_EVENT_MESSAGE };
		gce.dwFlags = GCEF_ADDTOLOG | GCEF_UTF8;
		gce.pszText.a = szText;
		gce.pszNick.a = szUserId;
		gce.pszUID.a = szUserId;
		gce.time = pMessage->date_;
		Chat_Event(&gce);
	}
	else {
		DB::EventInfo dbei(hOldEvent);
		dbei.szId = szMsgId;
		dbei.cbBlob = szText.GetLength();
		dbei.iTimestamp = pMessage->date_;
		if (pMessage->is_outgoing_)
			dbei.flags |= DBEF_SENT;
		if (!pUser->bInited)
			dbei.flags |= DBEF_READ;
		if (GetGcUserId(pUser, pMessage, szUserId))
			dbei.szUserId = szUserId;
		if (auto iReplyId = getReplyId(pMessage->reply_to_.get())) {
			szReplyId = msg2id(pMessage->chat_id_, iReplyId);
			dbei.szReplyId = szReplyId;
		}

		if (dbei) {
			replaceStr(dbei.pBlob, szText.Detach());
			db_event_edit(hOldEvent, &dbei, true);
		}
		else {
			dbei.pBlob = szText.GetBuffer();
			ProtoChainRecvMsg(hContact, dbei);
		}
	}
}

void CTelegramProto::ProcessMessageContent(TD::updateMessageContent *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	auto szMsgId = msg2id(pObj->chat_id_, pObj->message_id_);
	MEVENT hDbEvent = db_event_getById(m_szModuleName, szMsgId);
	if (hDbEvent == 0) {
		debugLogA("Unknown message with id=%lld (chat id %lld, ignored", pObj->message_id_, pObj->chat_id_);
		return;
	}

	auto msg = TD::make_object<TD::message>();
	msg->id_ = pObj->message_id_;
	msg->sender_id_ = TD::make_object<TD::messageSenderChat>(pObj->chat_id_);
	msg->content_ = std::move(pObj->new_content_);

	TG_OWN_MESSAGE tmp(0, 0, szMsgId);
	if (auto *pOwnMsg = m_arOwnMsg.find(&tmp))
		msg->is_outgoing_ = true;

	CMStringA szText(GetMessageText(pUser, msg.get()));
	if (szText.IsEmpty()) {
		debugLogA("this message was not processed, ignored");
		return;
	}

	DBEVENTINFO dbei = {};
	if (db_event_get(hDbEvent, &dbei))
		return;

	dbei.cbBlob = szText.GetLength();
	dbei.pBlob = szText.GetBuffer();
	db_event_edit(hDbEvent, &dbei, true);
}

void CTelegramProto::ProcessMessageReactions(TD::updateMessageInteractionInfo *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	CMStringA szMsgId(msg2id(pObj->chat_id_, pObj->message_id_));
	DB::EventInfo dbei(db_event_getById(m_szModuleName, szMsgId));
	if (!dbei) {
		debugLogA("Unknown message with id=%lld (chat id %lld, ignored", pObj->message_id_, pObj->chat_id_);
		return;
	}

	JSONNode reactions; reactions.set_name("r");
	if (pObj->interaction_info_)
		if (pObj->interaction_info_->reactions_) {
			for (auto &it : pObj->interaction_info_->reactions_->reactions_) {
				if (it->type_->get_id() != TD::reactionTypeEmoji::ID)
					continue;

				auto *pEmoji = (TD::reactionTypeEmoji *)it->type_.get();
				reactions << INT_PARAM(pEmoji->emoji_.c_str(), it->total_count_);
			}
		}

	auto &json = dbei.setJson();
	auto it = json.find("r");
	if (it != json.end())
		json.erase(it);

	json << reactions;
	dbei.flushJson();

	db_event_edit(dbei.getEvent(), &dbei, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

static char *sttBotIds[] = {
	"replies_bot_chat_id",
	"group_anonymous_bot_user_id",
	"channel_bot_user_id",
	"anti_spam_bot_user_id",
};

void CTelegramProto::ProcessOption(TD::updateOption *pObj)
{
	TD::int53 iValue = 0;
	if (pObj->value_->get_id() == TD::optionValueInteger::ID)
		iValue = ((TD::optionValueInteger *)pObj->value_.get())->value_;

	if (pObj->name_ == "my_id") {
		m_iOwnId = iValue;
		SetId(0, m_iOwnId);

		if (m_iSavedMessages != 0 || m_bUnregister)
			return;

		if (auto *pUser = FindUser(iValue)) {
			m_iSavedMessages = pUser->hContact;
			pUser->hContact = 0;
		}
		else {
			m_iSavedMessages = db_add_contact();
			Proto_AddToContact(m_iSavedMessages, m_szModuleName);
			SetId(m_iSavedMessages, m_iOwnId);
			Clist_SetGroup(m_iSavedMessages, m_wszDefaultGroup);

			pUser = new TG_USER(m_iOwnId, 0);
			m_arUsers.insert(pUser);
			m_arChats.insert(pUser);
		}

		setWString(m_iSavedMessages, "Nick", TranslateT("Saved messages"));
		return;
	}

	for (auto &it : sttBotIds)
		if (pObj->name_ == it) {
			m_botIds.push_back(iValue);
			return;
		}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ProcessRemoteMarkRead(TD::updateChatReadOutbox *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	CMStringA szMaxId(msg2id(pUser->chatId, pObj->last_read_outbox_message_id_));
	MarkRead(pUser->hContact, szMaxId, true);

	auto hContact = GetRealContact(pUser);
	if (g_plugin.hasMessageState)
		CallService(MS_MESSAGESTATE_UPDATE, hContact, MRD_TYPE_READ);

	if (auto hEvent = db_event_getById(m_szModuleName, szMaxId)) {
		setDword(hContact, DBKEY_REMOTE_READ, hEvent);
		if (g_plugin.hasNewStory)
			NS_NotifyRemoteRead(hContact, hEvent);
	}
}

void CTelegramProto::ProcessScopeNotification(TD::updateScopeNotificationSettings *pObj)
{
	switch (pObj->scope_->get_id()) {
	case TD::notificationSettingsScopePrivateChats::ID:
		m_iDefaultMutePrivate = pObj->notification_settings_->mute_for_;
		break;

	case TD::notificationSettingsScopeGroupChats::ID:
		m_iDefaultMuteGroup = pObj->notification_settings_->mute_for_;
		break;

	case TD::notificationSettingsScopeChannelChats::ID:
		m_iDefaultMuteChannel = pObj->notification_settings_->mute_for_;
		break;
	}
}

void CTelegramProto::ProcessServiceNotification(TD::updateServiceNotification *pObj)
{
	if (pObj->content_->get_id() != TD::messageText::ID) {
		debugLogA("Expected type %d, but got %d, ignored", TD::messageText::ID, pObj->content_->get_id());
		return;
	}

	auto *pMessageText = (TD::messageText *)pObj->content_.get();
	CMStringA szMessage(GetFormattedText(pMessageText->text_));
	Popup(0, Utf2T(szMessage), TranslateT("Service notification"));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ProcessStatus(TD::updateUserStatus *pObj)
{
	if (auto *pUser = FindUser(pObj->user_id_)) {
		if (pUser->hContact == INVALID_CONTACT_ID)
			return;

		switch (pObj->status_->get_id()) {
		case TD::userStatusOnline::ID:
			setWord(pUser->hContact, "Status", ID_STATUS_ONLINE);
			break;

		case TD::userStatusRecently::ID:
		case TD::userStatusOffline::ID:
			if (m_iTimeDiff1) {
				setWord(pUser->hContact, "Status", m_iStatus1);
				pUser->m_timer1 = time(0);
			}
			else setWord(pUser->hContact, "Status", ID_STATUS_OFFLINE);
			break;

		default:
			debugLogA("!!!!! Unknown status packet, report it to the developers");
		}
	}
}

void CTelegramProto::ProcessUser(TD::updateUser *pObj)
{
	auto *pUser = pObj->user_.get();
	bool bIsMe = pUser->id_ == m_iOwnId;
	auto typeID = (pUser->type_) ? pUser->type_->get_id() : 0;

	if (!bIsMe && !pUser->is_contact_) {
		switch (typeID) {
		case TD::userTypeDeleted::ID:
			return;

		case TD::userTypeBot::ID:
		case TD::userTypeRegular::ID:
			auto *pu = AddFakeUser(pUser->id_, false);
			if (pu->hContact != INVALID_CONTACT_ID)
				RemoveFromClist(pu);

			pu->wszFirstName = Utf2T(pUser->first_name_.c_str());
			pu->wszLastName = Utf2T(pUser->last_name_.c_str());
			if (pUser->usernames_) {
				pu->wszNick = L"@";
				pu->wszNick.Append(Utf2T(pUser->usernames_->editable_username_.c_str()));
			}
			else {
				pu->wszNick = Utf2T(pUser->first_name_.c_str());
				if (!pUser->last_name_.empty())
					pu->wszNick.AppendFormat(L" %s", Utf2T(pUser->last_name_.c_str()).get());
			}

			CheckSearchUser(pu);
		}

		debugLogA("User doesn't belong to your contacts, skipping");
		return;
	}

	for (auto &it : m_botIds)
		if (it == pUser->id_) {
			if (auto *pu = FindUser(it)) {
				Contact::Hide(pu->hContact);
				Contact::RemoveFromList(pu->hContact);
			}
			return;
		}

	std::string szFirstName = pUser->first_name_, szLastName = pUser->last_name_;
	if (szLastName.empty()) {
		size_t p = szFirstName.rfind(' ');
		if (p != -1) {
			szLastName = szFirstName.substr(p + 1);
			szFirstName = szFirstName.substr(0, p);
		}
	}

	auto *pu = AddUser(pUser->id_, false);

	if (szFirstName.empty())
		delSetting(pu->hContact, "FirstName");
	else
		setUString(pu->hContact, "FirstName", szFirstName.c_str());

	if (szLastName.empty())
		delSetting(pu->hContact, "LastName");
	else
		setUString(pu->hContact, "LastName", szLastName.c_str());
	
	if (pu->hContact)
		UpdateString(pu->hContact, "Phone", pUser->phone_number_);

	if (pUser->usernames_)
		UpdateString(pu->hContact, "Nick", pUser->usernames_->editable_username_);
	else
		delSetting(pu->hContact, "Nick");

	Contact::PutOnList(pu->hContact);

	if (bIsMe)
		pu->wszNick = ptrW(Contact::GetInfo(CNF_DISPLAY, 0, m_szModuleName));

	if (pUser->is_premium_)
		ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, "tg_premium");
	else if (typeID == TD::userTypeBot::ID) {
		pu->isBot = true;
		ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, "tg_bot");
	}
	else ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, nullptr);

	if (auto *pPhoto = pUser->profile_photo_.get())
		ProcessAvatar(pPhoto->small_.get(), pu);

	if (pUser->status_) {
		if (pUser->status_->get_id() == TD::userStatusOffline::ID) {
			auto *pOffline = (TD::userStatusOffline *)pUser->status_.get();
			setDword(pu->hContact, "LastSeen", pOffline->was_online_);
		}
	}
}
