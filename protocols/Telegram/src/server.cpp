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

	SendQuery(new TD::getChats(td::tl::unique_ptr<TD::chatListMain>(), 1000));
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::ProcessResponse(td::ClientManager::Response response)
{
	if (!response.object)
		return;

	debugLogA("ProcessResponse: id=%d (%s)", int(response.request_id), to_string(response.object).c_str());

	if (response.request_id) {
		auto *p = m_arRequests.find((TG_REQUEST *)&response.request_id);
		if (p) {
			(this->*p->pHandler)(response);
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

	case TD::updateNewChat::ID:
		ProcessChat((TD::updateNewChat *)response.object.get());
		break;

	case TD::updateNewMessage::ID:
		ProcessMessage((TD::updateNewMessage *)response.object.get());
		break;

	case TD::updateUser::ID:
		ProcessUser((TD::updateUser *)response.object.get());
		break;
	}
}

void CMTProto::SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	int queryId = ++m_iQueryId;
	m_pClientMmanager->send(m_iClientId, queryId, TD::object_ptr<TD::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
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

void CMTProto::ProcessGroups(TD::updateChatFilters *pObj)
{
	for (auto &grp : pObj->chat_filters_) {
		if (grp->icon_name_ != "Custom")
			continue;

		CMStringA szSetting(FORMAT, "ChatFilter%d", grp->id_);
		CMStringW wszOldValue(getMStringW(szSetting));
		Utf2T wszNewValue(grp->title_.c_str());
		if (wszOldValue.IsEmpty()) {
			Clist_GroupCreate(0, wszNewValue);
			setWString(szSetting, wszNewValue);
		}
		else if (wszOldValue != wszNewValue) {
			MGROUP oldGroup = Clist_GroupExists(wszNewValue);
			if (!oldGroup)
				Clist_GroupCreate(0, wszNewValue);
			else
				Clist_GroupRename(oldGroup, wszNewValue);
			setWString(szSetting, wszNewValue);
		}
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
	_i64toa(pMessage->id_, szId, _countof(szId));

	PROTORECVEVENT pre = {};
	pre.szMessage = szText.GetBuffer();
	pre.szMsgId = szId;
	pre.timestamp = pMessage->date_;
	if (pMessage->sender_id_->get_id() == TD::messageSenderUser::ID)
		if (((TD::messageSenderUser *)pMessage->sender_id_.get())->user_id_ == m_iOwnId)
			pre.flags |= PREF_SENT;
	ProtoChainRecvMsg(pUser->hContact, &pre);
}

void CMTProto::ProcessUser(TD::updateUser *pObj)
{
	auto *pUser = pObj->user_.get();
	
	auto *pu = AddUser(pUser->id_, false);
	UpdateString(pu->hContact, "FirstName", pUser->first_name_);
	UpdateString(pu->hContact, "LastName", pUser->last_name_);
	UpdateString(pu->hContact, "Phone", pUser->phone_number_);
	if (pUser->usernames_)
		UpdateString(pu->hContact, "Nick", pUser->usernames_->editable_username_);

	if (pUser->phone_number_ == _T2A(m_szOwnPhone).get())
		m_iOwnId = pUser->id_;

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
