/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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
	m_bTerminated = true;
}

void __cdecl CTelegramProto::ServerThread(void *)
{
	m_bTerminated = m_bAuthorized = false;
	m_pClientManager = std::make_unique<td::ClientManager>();
	m_iClientId = m_pClientManager->create_client_id();

	SendQuery(new TD::getOption("version"));

	while (!m_bTerminated) {
		ProcessResponse(m_pClientManager->receive(1));
	}

	m_pClientManager = std::move(nullptr);
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

		SendQuery(new TD::getChats(td::tl::unique_ptr<TD::chatListMain>(), 1000));
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::SendKeepAlive()
{
	time_t now = time(0);

	for (auto &it : m_arUsers) {
		if (it->m_timer1 && now - it->m_timer1 > STATUS_SWITCH_TIMEOUT) {
			it->m_timer1 = 0;
			it->m_timer2 = now;
			setWord(it->hContact, "Status", ID_STATUS_NA);
		}
		else if (it->m_timer2 && now - it->m_timer2 > STATUS_SWITCH_TIMEOUT) {
			it->m_timer2 = 0;
			setWord(it->hContact, "Status", ID_STATUS_OFFLINE);
		}
	}
}

void CTelegramProto::SendDeleteMsg()
{
	m_impl.m_deleteMsg.Stop();

	mir_cslock lck(m_csDeleteMsg);
	SendQuery(new TD::deleteMessages(m_deleteChatId, std::move(m_deleteIds), true));
	m_deleteChatId = 0;
}

void CTelegramProto::SendMarkRead()
{
	m_impl.m_markRead.Stop();

	mir_cslock lck(m_csMarkRead);
	SendQuery(new TD::viewMessages(m_markChatId, 0, std::move(m_markIds), true));
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
		auto *p = m_arRequests.find(&tmp);
		if (p) {
			p->Execute(this, response);
			m_arRequests.remove(p);
		}
		return;
	}

	switch (response.object->get_id()) {
	case TD::updateAuthorizationState::ID:
		ProcessAuth((TD::updateAuthorizationState *)response.object.get());
		break;

	case TD::updateBasicGroup::ID:
		ProcessBasicGroup((TD::updateBasicGroup*)response.object.get());
		break;

	case TD::updateChatFilters::ID:
		ProcessGroups((TD::updateChatFilters *)response.object.get());
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

	case TD::updateDeleteMessages::ID:
		ProcessDeleteMessage((TD::updateDeleteMessages*)response.object.get());
		break;

	case TD::updateConnectionState::ID:
		ProcessConnectionState((TD::updateConnectionState *)response.object.get());
		break;

	case TD::updateFile::ID:
		ProcessFile((TD::updateFile *)response.object.get());
		break;

	case TD::updateMessageContent::ID:
		ProcessMessageContent((TD::updateMessageContent *)response.object.get());
		break;

	case TD::updateMessageSendSucceeded::ID:
		ProcessMessage(((TD::updateMessageSendSucceeded *)response.object.get())->message_.get());
		break;

	case TD::updateNewChat::ID:
		ProcessChat((TD::updateNewChat *)response.object.get());
		break;

	case TD::updateNewMessage::ID:
		ProcessMessage(((TD::updateNewMessage *)response.object.get())->message_.get());
		break;

	case TD::updateOption::ID:
		ProcessOption((TD::updateOption *)response.object.get());
		break;

	case TD::updateSupergroup::ID:
		ProcessSuperGroup((TD::updateSupergroup *)response.object.get());
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

void CTelegramProto::OnSendMessage(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::message::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::message::ID);
		return;
	}

	auto *pMessage = ((TD::message *)response.object.get());
	auto *pUser = FindChat(pMessage->chat_id_);
	if (pUser) {
		char szMsgId[100];
		_i64toa(pMessage->id_, szMsgId, 10);
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, pUserInfo, (LPARAM)szMsgId);
	}
}

int CTelegramProto::SendTextMessage(int64_t chatId, const char *pszMessage)
{
	int ret = m_iMsgId++;

	auto pContent = TD::make_object<TD::inputMessageText>();
	pContent->text_ = TD::make_object<TD::formattedText>();
	pContent->text_->text_ = std::move(pszMessage);

	auto *pMessage = new TD::sendMessage();
	pMessage->chat_id_ = chatId;
	pMessage->input_message_content_ = std::move(pContent);
	SendQuery(pMessage, &CTelegramProto::OnSendMessage, (void *)ret);

	return ret;
}

void CTelegramProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	if (!m_pClientManager)
		return;

	int queryId = ++m_iQueryId;

	auto szDescr = to_string(*pFunc);
	debugLogA("Sending query %d:\n%s", queryId, szDescr.c_str());

	m_pClientManager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
}

void CTelegramProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo)
{
	if (!m_pClientManager)
		return;

	int queryId = ++m_iQueryId;

	auto szDescr = to_string(*pFunc);
	debugLogA("Sending full query %d:\n%s", queryId, szDescr.c_str());

	m_pClientManager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST_FULL(queryId, pHandler, pUserInfo));
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

		char szMsgId[100], szUserId[100];
		_i64toa(pMsg->id_, szMsgId, 10);
		if (db_event_getById(m_szModuleName, szMsgId))
			continue;

		CMStringA szBody = GetMessageText(pUser, pMsg);

		DBEVENTINFO dbei = {};
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		dbei.timestamp = pMsg->date_;
		dbei.cbBlob = szBody.GetLength();
		dbei.pBlob = (uint8_t*)szBody.c_str();
		dbei.szId = szMsgId;
		dbei.flags = DBEF_READ | DBEF_UTF | DBEF_HAS_ID;
		if (pMsg->is_outgoing_)
			dbei.flags |= DBEF_SENT;
		if (this->GetGcUserId(pUser, pMsg, szUserId))
			dbei.szUserId = szUserId;
		db_event_add(pUser->hContact, &dbei);
	}

	if (lastMsgId != INT64_MAX)
		SendQuery(new TD::getChatHistory(pUser->chatId, lastMsgId, 0, 100, false), &CTelegramProto::OnGetHistory, pUser);
}

INT_PTR CTelegramProto::SvcLoadServerHistory(WPARAM hContact, LPARAM)
{
	if (auto *pUser = FindUser(GetId(hContact)))
		SendQuery(new TD::getChatHistory(pUser->chatId, 0, 0, 100, false), &CTelegramProto::OnGetHistory, pUser);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ProcessBasicGroup(TD::updateBasicGroup *pObj)
{
	auto *pBasicGroup = pObj->basic_group_.get();
	if (pBasicGroup->upgraded_to_supergroup_id_)
		if (auto *pUser = FindUser(pBasicGroup->upgraded_to_supergroup_id_)) {
			pUser->bLoadMembers = true;
			if (pUser->m_si)
				pUser->m_si->bHasNicklist = true;
		}

	auto iStatusId = pBasicGroup->status_->get_id();
	if (iStatusId == TD::chatMemberStatusBanned::ID) {
		debugLogA("We are banned here, skipping");
		return;
	}
	
	TG_BASIC_GROUP tmp(pBasicGroup->id_, 0);
	auto *pGroup = m_arBasicGroups.find(&tmp);
	if (pGroup == nullptr) {
		pGroup = new TG_BASIC_GROUP(tmp.id, std::move(pObj->basic_group_));
		m_arBasicGroups.insert(pGroup);
	}
	else pGroup->group = std::move(pObj->basic_group_);

	TG_USER *pUser;
	if (iStatusId == TD::chatMemberStatusLeft::ID) {
		pUser = AddFakeUser(tmp.id, true);
		pUser->wszLastName.Format(TranslateT("%d member(s)"), pGroup->group->member_count_);
	}
	else pUser = AddUser(tmp.id, true);

	pUser->bLoadMembers = true;
}

void CTelegramProto::ProcessChat(TD::updateNewChat *pObj)
{
	int64_t chatId;
	auto *pChat = pObj->chat_.get();
	std::string szTitle;

	switch(pChat->type_->get_id()) {
	case TD::chatTypePrivate::ID:
	case TD::chatTypeSecret::ID:
		chatId = pChat->id_;
		break;

	case TD::chatTypeBasicGroup::ID:
		chatId = ((TD::chatTypeBasicGroup*)pChat->type_.get())->basic_group_id_;
		szTitle = pChat->title_;
		break;

	case TD::chatTypeSupergroup::ID:
		chatId = ((TD::chatTypeSupergroup *)pChat->type_.get())->supergroup_id_;
		szTitle = pChat->title_;
		break;

	default:
		debugLogA("Invalid chat type %d, ignoring", pChat->type_->get_id());
		return;
	}

	if (auto *pUser = FindUser(chatId)) {
		pUser->chatId = pChat->id_;
		MCONTACT hContact = (pUser->id == m_iOwnId) ? 0 : pUser->hContact;

		if (!m_arChats.find(pUser))
			m_arChats.insert(pUser);

		if (!szTitle.empty()) {
			if (hContact != INVALID_CONTACT_ID)
				setUString(hContact, "Nick", szTitle.c_str());
			else if (pUser->wszNick.IsEmpty())
				pUser->wszFirstName = Utf2T(szTitle.c_str());
		}

		if (CheckSearchUser(pUser))
			return;

		if (pUser->isGroupChat && pUser->hContact != INVALID_CONTACT_ID)
			InitGroupChat(pUser, pChat);
	}
	else debugLogA("Unknown chat id %lld, ignoring", chatId);
}

void CTelegramProto::ProcessChatLastMessage(TD::updateChatLastMessage *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("Unknown chat, skipping");
		return;
	}

	if (pUser->hContact == INVALID_CONTACT_ID) {
		debugLogA("Last message for a temporary contact, skipping");
		return;
	}

	// according to #3406 we wipe history for the contacts from contacts' list
	// but remove the contact itself if it's a temporary one
	if (pObj->last_message_ == nullptr) {
		if (Contact::OnList(pUser->hContact))
			CallService(MS_HISTORY_EMPTY, pUser->hContact, TRUE);
		else
			db_delete_contact(pUser->hContact, true);
	}
}

void CTelegramProto::ProcessChatNotification(TD::updateChatNotificationSettings *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr || pUser->hContact == INVALID_CONTACT_ID)
		return;

	auto &pSettings = pObj->notification_settings_;
	if (!pSettings->use_default_mute_for_ && pSettings->mute_for_ != 0)
		Chat_Mute(pUser->hContact, CHATMODE_MUTE);
	else
		Chat_Mute(pUser->hContact, CHATMODE_NORMAL);

	memcpy(&pUser->notificationSettings, pSettings.get(), sizeof(pUser->notificationSettings));
}

void CTelegramProto::ProcessChatPosition(TD::updateChatPosition *pObj)
{
	if (pObj->position_->get_id() != TD::chatPosition::ID) {
		debugLogA("Unsupport position");
		return;
	}

	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("Unknown chat, skipping");
		return;
	}

	if (pUser->hContact == INVALID_CONTACT_ID)
		return;

	auto *pPos = (TD::chatPosition *)pObj->position_.get();
	if (pPos->list_) {
		auto *pList = (TD::chatListFilter *)pPos->list_.get();

		CMStringA szSetting(FORMAT, "ChatFilter%d", pList->chat_filter_id_);
		CMStringW wszGroup(getMStringW(szSetting));
		if (!wszGroup.IsEmpty()) {
			ptrW pwszExistingGroup(Clist_GetGroup(pUser->hContact));
			if (!pwszExistingGroup
				|| (!pUser->isGroupChat && !mir_wstrcmp(pwszExistingGroup, m_wszDefaultGroup))
				|| (pUser->isGroupChat && !mir_wstrcmp(pwszExistingGroup, ptrW(Chat_GetGroup())))) {
				CMStringW wszNewGroup(FORMAT, L"%s\\%s", (wchar_t *)m_wszDefaultGroup, wszGroup.c_str());
				Clist_GroupCreate(0, wszNewGroup);
				Clist_SetGroup(pUser->hContact, wszNewGroup);
			}
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

void CTelegramProto::ProcessDeleteMessage(TD::updateDeleteMessages *pObj)
{
	if (!pObj->is_permanent_)
		return;

	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr || pUser->hContact == INVALID_CONTACT_ID) {
		debugLogA("message from unknown chat, ignored");
		return;
	}

	for (auto &it : pObj->message_ids_) {
		char id[100];
		_i64toa(it, id, 10);
		if (MEVENT hEvent = db_event_getById(m_szModuleName, id))
			db_event_delete(hEvent, true);
	}
}

void CTelegramProto::ProcessGroups(TD::updateChatFilters *pObj)
{
	for (auto &grp : pObj->chat_filters_) {
		if (grp->icon_name_ != "Custom")
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

	char szId[100];
	_i64toa(pObj->last_read_inbox_message_id_, szId, 10);
	MEVENT hLastRead = db_event_getById(m_szModuleName, szId);
	if (hLastRead == 0) {
		debugLogA("unknown event, ignored");
		return;
	}

	bool bExit = false;
	for (MEVENT hEvent = db_event_firstUnread(pUser->hContact); hEvent; hEvent = db_event_next(pUser->hContact, hEvent)) {
		if (bExit)
			break;

		bExit = (hEvent == hLastRead);

		DBEVENTINFO dbei = {};
		if (db_event_get(hEvent, &dbei))
			continue;

		if (!dbei.markedRead())
			db_event_markRead(pUser->hContact, hEvent, true);
	}
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

	CMStringA szText(GetMessageText(pUser, pMessage));
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

	char szId[100], szUserId[100];
	_i64toa(pMessage->id_, szId, 10);

	PROTORECVEVENT pre = {};
	pre.szMessage = szText.GetBuffer();
	pre.szMsgId = szId;
	pre.timestamp = pMessage->date_;
	if (pMessage->is_outgoing_)
		pre.flags |= PREF_SENT;
	if (GetGcUserId(pUser, pMessage, szUserId))
		pre.szUserId = szUserId;
	ProtoChainRecvMsg((pUser->hContact) ? pUser->hContact : m_iSavedMessages, &pre);
}

void CTelegramProto::ProcessMessageContent(TD::updateMessageContent *pObj)
{
	auto *pUser = FindChat(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	char szMsgId[100];
	_i64toa(pObj->message_id_, szMsgId, 10);
	
	MEVENT hDbEvent = db_event_getById(m_szModuleName, szMsgId);
	if (hDbEvent == 0) {
		debugLogA("Unknown message with id=%lld (chat id %lld, ignored", pObj->message_id_, pObj->chat_id_);
		return;
	}

	/*
	CMStringA szText(GetMessageText(pUser, pObj->new_content_.get()));
	if (szText.IsEmpty()) {
		debugLogA("this message was not processed, ignored");
		return;
	}*/
}

void CTelegramProto::ProcessOption(TD::updateOption *pObj)
{
	TD::int53 iValue = 0;
	if (pObj->value_->get_id() == TD::optionValueInteger::ID)
		iValue = ((TD::optionValueInteger *)pObj->value_.get())->value_;

	if (pObj->name_ == "my_id") {
		m_iOwnId = iValue;
		SetId(0, m_iOwnId);

		if (auto *pUser = FindUser(iValue)) {
			m_iSavedMessages = pUser->hContact;
			pUser->hContact = 0;
		}
		else {
			m_iSavedMessages = db_add_contact();
			Proto_AddToContact(m_iSavedMessages, m_szModuleName);
			SetId(m_iSavedMessages, m_iOwnId);
			Clist_SetGroup(m_iSavedMessages, m_wszDefaultGroup);
			setWString(m_iSavedMessages, "Nick", TranslateT("Saved messages"));

			pUser = new TG_USER(m_iOwnId, 0);
			m_arUsers.insert(pUser);
			m_arChats.insert(pUser);
		}
	}
}

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
			setWord(pUser->hContact, "Status", ID_STATUS_AWAY);
			pUser->m_timer1 = time(0);
			break;

		default:
			debugLogA("!!!!! Unknown status packet, report it to the developers");
		}
	}
}

void CTelegramProto::ProcessSuperGroup(TD::updateSupergroup *pObj)
{
	auto iStatusId = pObj->supergroup_->status_->get_id();
	if (iStatusId == TD::chatMemberStatusBanned::ID) {
		debugLogA("We are banned here, skipping");
		return;
	}

	TG_SUPER_GROUP tmp(pObj->supergroup_->id_, 0);

	auto *pGroup = m_arSuperGroups.find(&tmp);
	if (pGroup == nullptr) {
		pGroup = new TG_SUPER_GROUP(tmp.id, std::move(pObj->supergroup_));
		m_arSuperGroups.insert(pGroup);
	}
	else pGroup->group = std::move(pObj->supergroup_);

	if (iStatusId == TD::chatMemberStatusLeft::ID) {
		auto *pUser = AddFakeUser(tmp.id, true);
		pUser->wszNick = getName(pGroup->group->usernames_.get());
		pUser->wszLastName.Format(TranslateT("%d member(s)"), pGroup->group->member_count_);
	}
	else AddUser(tmp.id, true);
}

void CTelegramProto::ProcessUser(TD::updateUser *pObj)
{
	auto *pUser = pObj->user_.get();
	bool bIsMe = pUser->id_ == m_iOwnId;

	if (!bIsMe && !pUser->is_contact_) {
		auto *pu = AddFakeUser(pUser->id_, false);
		if (pu->hContact != INVALID_CONTACT_ID)
			Contact::RemoveFromList(pu->hContact);

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

		debugLogA("User doesn't belong to your contacts, skipping");
		return;
	}

	auto *pu = AddUser(pUser->id_, false);
	UpdateString(pu->hContact, "FirstName", pUser->first_name_);
	UpdateString(pu->hContact, "LastName", pUser->last_name_);
	if (pu->hContact)
		UpdateString(pu->hContact, "Phone", pUser->phone_number_);

	if (pUser->usernames_)
		UpdateString(pu->hContact, "Nick", pUser->usernames_->editable_username_);

	Contact::PutOnList(pu->hContact);

	if (bIsMe)
		pu->wszNick = ptrW(Contact::GetInfo(CNF_DISPLAY, 0, m_szModuleName));

	if (pUser->is_premium_)
		ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, "tg_premium");
	else
		ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, nullptr);

	if (auto *pPhoto = pUser->profile_photo_.get()) {
		if (auto *pSmall = pPhoto->small_.get()) {
			auto remoteId = pSmall->remote_->unique_id_;
			auto storedId = getMStringA(pu->hContact, DBKEY_AVATAR_HASH);
			if (remoteId != storedId.c_str()) {
				if (!remoteId.empty()) {
					if (pu)
						pu->szAvatarHash = remoteId.c_str();
					setString(pu->hContact, DBKEY_AVATAR_HASH, remoteId.c_str());
					SendQuery(new TD::downloadFile(pSmall->id_, 5, 0, 0, false));
				}
				else delSetting(pu->hContact, DBKEY_AVATAR_HASH);
			}
		}
	}

	if (pUser->status_) {
		if (pUser->status_->get_id() == TD::userStatusOffline::ID) {
			auto *pOffline = (TD::userStatusOffline *)pUser->status_.get();
			setDword(pu->hContact, "LastSeen", pOffline->was_online_);
		}
	}
}
