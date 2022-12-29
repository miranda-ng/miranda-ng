/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

void __cdecl CMTProto::ServerThread(void *)
{
	m_bRunning = true;
	m_bTerminated = m_bAuthorized = false;

	SendQuery(new TD::getOption("version"));

	while (!m_bTerminated) {
		ProcessResponse(m_pClientMmanager->receive(1));
	}

	m_bRunning = false;
}

void CMTProto::LogOut()
{
	if (m_bTerminated)
		return;

	debugLogA("CMTProto::OnLoggedOut");
	m_bTerminated = true;
	m_bAuthorized = false;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void CMTProto::OnLoggedIn()
{
	m_bAuthorized = true;

	debugLogA("CMTProto::OnLoggedIn");

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::SendKeepAlive()
{
	time_t now = time(0);

	for (auto &it : m_arUsers) {
		if (it->m_timer1 && now - it->m_timer1 > 600) {
			it->m_timer1 = 0;
			it->m_timer2 = now;
			setWord(it->hContact, "Status", ID_STATUS_AWAY);
		}
		else if (it->m_timer2 && now - it->m_timer2 > 600) {
			it->m_timer2 = 0;
			setWord(it->hContact, "Status", ID_STATUS_OFFLINE);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::ProcessResponse(td::ClientManager::Response response)
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

	case TD::updateChatFilters::ID:
		ProcessGroups((TD::updateChatFilters *)response.object.get());
		break;

	case TD::updateChatPosition::ID:
		ProcessChatPosition((TD::updateChatPosition *)response.object.get());
		break;

	case TD::updateNewChat::ID:
		ProcessChat((TD::updateNewChat *)response.object.get());
		break;

	case TD::updateNewMessage::ID:
		ProcessMessage((TD::updateNewMessage *)response.object.get());
		break;

	case TD::updateUserStatus::ID:
		ProcessStatus((TD::updateUserStatus *)response.object.get());
		break;

	case TD::updateUser::ID:
		ProcessUser((TD::updateUser *)response.object.get());
		break;
	}
}

void CMTProto::OnSendMessage(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::message::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::message::ID);
		return;
	}

	auto *pMessage = ((TD::message *)response.object.get());
	auto *pUser = FindUser(pMessage->chat_id_);
	if (pUser) {
		char szMsgId[100];
		_i64toa(pMessage->id_, szMsgId, 10);
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, pUserInfo, (LPARAM)szMsgId);
	}
}

int CMTProto::SendTextMessage(uint64_t chatId, const char *pszMessage)
{
	int ret = m_iMsgId++;

	auto pContent = TD::make_object<TD::inputMessageText>();
	pContent->text_ = TD::make_object<TD::formattedText>();
	pContent->text_->text_ = std::move(pszMessage);

	auto *pMessage = new TD::sendMessage();
	pMessage->chat_id_ = chatId;
	pMessage->input_message_content_ = std::move(pContent);
	SendQuery(pMessage, &CMTProto::OnSendMessage, (void*)ret);

	return ret;
}

void CMTProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	int queryId = ++m_iQueryId;
	m_pClientMmanager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
}

void CMTProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo)
{
	int queryId = ++m_iQueryId;
	m_pClientMmanager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST_FULL(queryId, pHandler, pUserInfo));
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::ProcessChat(TD::updateNewChat *pObj)
{
	auto &pChat = pObj->chat_;
	if (pChat->type_->get_id() != TD::chatTypePrivate::ID) {
		debugLogA("Only private chats are currently supported");
		return;
	}

	auto *pUser = AddUser(pChat->id_, false);
	if (!pChat->title_.empty())
		setUString(pUser->hContact, "Nick", pChat->title_.c_str());
}

void CMTProto::ProcessChatPosition(TD::updateChatPosition *pObj)
{
	if (pObj->position_->get_id() != TD::chatPosition::ID) {
		debugLogA("Unsupport position");
		return;
	}

	auto *pUser = FindUser(pObj->chat_id_);
	if (pUser == nullptr) {
		debugLogA("Unknown chat, skipping");
		return;
	}

	auto *pPos = (TD::chatPosition *)pObj->position_.get();
	if (pPos->list_) {
		auto *pList = (TD::chatListFilter*)pPos->list_.get();
		
		CMStringA szSetting(FORMAT, "ChatFilter%d", pList->chat_filter_id_);
		CMStringW wszGroup(getMStringW(szSetting));
		if (!wszGroup.IsEmpty()) {
			ptrW pwszExistingGroup(Clist_GetGroup(pUser->hContact));
			if (!pwszExistingGroup || !mir_wstrcmp(pwszExistingGroup, m_wszDefaultGroup)) {
				CMStringW wszNewGroup(FORMAT, L"%s\\%s", (wchar_t *)m_wszDefaultGroup, wszGroup.c_str());
				Clist_SetGroup(pUser->hContact, wszNewGroup);
			}
		}		
	}
}

void CMTProto::ProcessGroups(TD::updateChatFilters *pObj)
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
			CMStringW wszFullGroup(FORMAT, L"%s\\%s", (wchar_t*)m_wszDefaultGroup, wszNewValue);
			MGROUP oldGroup = Clist_GroupExists(wszFullGroup);
			if (!oldGroup)
				Clist_GroupCreate(m_iBaseGroup, wszFullGroup);
			else
				Clist_GroupRename(oldGroup, wszFullGroup);
			setWString(szSetting, wszNewValue);
		}

		SendQuery(new TD::getChats(TD::make_object<TD::chatListFilter>(grp->id_), 1000));
	}
}

void CMTProto::ProcessMessage(TD::updateNewMessage *pObj)
{
	auto &pMessage = pObj->message_;

	auto *pUser = FindUser(pMessage->chat_id_);
	if (pUser == nullptr) {
		debugLogA("message from unknown chat/user, ignored");
		return;
	}

	if (pUser->isGroupChat) {
		debugLogA("message from group chat, ignored");
		return;
	}

	CMStringA szText(getMessageText(pMessage->content_.get()));
	if (szText.IsEmpty()) {
		debugLogA("this message was not processed, ignored");
		return;
	}

	char szId[100];
	_i64toa(pMessage->id_, szId, 10);

	PROTORECVEVENT pre = {};
	pre.szMessage = szText.GetBuffer();
	pre.szMsgId = szId;
	pre.timestamp = pMessage->date_;
	if (pMessage->sender_id_->get_id() == TD::messageSenderUser::ID)
		if (((TD::messageSenderUser *)pMessage->sender_id_.get())->user_id_ == m_iOwnId)
			pre.flags |= PREF_SENT;
	ProtoChainRecvMsg(pUser->hContact, &pre);
}

void CMTProto::ProcessStatus(TD::updateUserStatus *pObj)
{
	if (auto *pUser = FindUser(pObj->user_id_)) {
		if (pObj->status_->get_id() == TD::userStatusOnline::ID)
			setWord(pUser->hContact, "Status", ID_STATUS_ONLINE);
		else if (pObj->status_->get_id() == TD::userStatusOffline::ID) {
			setWord(pUser->hContact, "Status", ID_STATUS_AWAY);
			pUser->m_timer1 = time(0);
		}
		else debugLogA("!!!!! Unknown status packet, report it to the developers");
	}
}

void CMTProto::ProcessUser(TD::updateUser *pObj)
{
	auto *pUser = pObj->user_.get();

	if (pUser->phone_number_ == _T2A(m_szOwnPhone).get()) {
		m_iOwnId = pUser->id_;

		if (!FindUser(pUser->id_))
			m_arUsers.insert(new TG_USER(pUser->id_, 0));
	}

	auto *pu = AddUser(pUser->id_, false);
	UpdateString(pu->hContact, "FirstName", pUser->first_name_);
	UpdateString(pu->hContact, "LastName", pUser->last_name_);
	UpdateString(pu->hContact, "Phone", pUser->phone_number_);
	if (pUser->usernames_)
		UpdateString(pu->hContact, "Nick", pUser->usernames_->editable_username_);

	if (pUser->is_premium_)
		ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, "tg_premium");
	else
		ExtraIcon_SetIconByName(g_plugin.m_hIcon, pu->hContact, nullptr);

	if (pUser->status_) {
		if (pUser->status_->get_id() == TD::userStatusOffline::ID) {
			auto *pOffline = (TD::userStatusOffline *)pUser->status_.get();
			setDword(pu->hContact, "LastSeen", pOffline->was_online_);
		}
	}
}
