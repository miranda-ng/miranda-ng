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

	SendQuery(new td::td_api::getOption("version"));

	while (!m_bTerminated) {
		ProcessResponse(m_pClientMmanager->receive(10));
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

	SendQuery(new td::td_api::getChats(td::tl::unique_ptr<td::td_api::chatListMain>(), 1000));
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
	case td::td_api::updateAuthorizationState::ID:
		ProcessAuth((td::td_api::updateAuthorizationState *)response.object.get());
		break;

	case td::td_api::updateChatFilters::ID:
		ProcessGroups((td::td_api::updateChatFilters *)response.object.get());
		break;

	case td::td_api::updateUser::ID:
		ProcessUser((td::td_api::updateUser *)response.object.get());
		break;

	}
}

void CMTProto::SendQuery(td::td_api::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	int queryId = ++m_iQueryId;
	m_pClientMmanager->send(m_iClientId, queryId, td::td_api::object_ptr<td::td_api::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::ProcessGroups(td::td_api::updateChatFilters *pObj)
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

void CMTProto::ProcessUser(td::td_api::updateUser *pObj)
{
	auto *pUser = pObj->user_.get();
	
	MCONTACT hContact = AddUser(pUser->id_, false);
	UpdateString(hContact, "FirstName", pUser->first_name_);
	UpdateString(hContact, "LastName", pUser->last_name_);
	UpdateString(hContact, "Phone", pUser->phone_number_);
	if (pUser->usernames_)
		UpdateString(hContact, "Nick", pUser->usernames_->editable_username_);

	if (pUser->status_) {
		if (pUser->status_->get_id() == td::td_api::userStatusOffline::ID) {
			auto *pOffline = (td::td_api::userStatusOffline *)pUser->status_.get();
			setDword(hContact, "LastSeen", pOffline->was_online_);
		}
	}
}
