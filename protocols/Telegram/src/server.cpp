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
}

///////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CMTProto::EnterPhoneCode(void *param)
{
	auto *ppro = (CMTProto *)param;

	ENTER_STRING es = {};
	es.szModuleName = ppro->m_szModuleName;
	es.caption = TranslateT("Enter secret code sent to your phone");
	if (EnterString(&es)) {
		ppro->SendQuery(new td::td_api::checkAuthenticationCode(_T2A(es.ptszResult).get()), &CMTProto::OnUpdateAuth);
		mir_free(es.ptszResult);
	}
	else ppro->LogOut();
	return 0;
}

INT_PTR CALLBACK CMTProto::EnterPassword(void *param)
{
	auto *ppro = (CMTProto *)param;
	CMStringW wszTitle(TranslateT("Enter password"));
	
	auto *pAuth = (td::td_api::authorizationStateWaitPassword *)ppro->pAuthState.get();
	if (!pAuth->password_hint_.empty())
		wszTitle.AppendFormat(TranslateT(" (hint: %s)"), Utf2T(pAuth->password_hint_.c_str()).get());

	ENTER_STRING es = {};
	es.szModuleName = ppro->m_szModuleName;
	es.caption = wszTitle;
	es.type = ESF_PASSWORD;
	if (EnterString(&es)) {
		ppro->SendQuery(new td::td_api::checkAuthenticationPassword(_T2A(es.ptszResult).get()), &CMTProto::OnUpdateAuth);
		mir_free(es.ptszResult);
	}
	else ppro->LogOut();
	return 0;
}

void CMTProto::ProcessAuth(td::td_api::updateAuthorizationState *pObj)
{
	pAuthState = std::move(pObj->authorization_state_);
	switch (pAuthState->get_id()) {
	case td::td_api::authorizationStateWaitTdlibParameters::ID:
		{
			char text[100];
			Miranda_GetVersionText(text, sizeof(text));

			CMStringW wszPath(GetProtoFolder());

			auto *request = new td::td_api::setTdlibParameters();
			request->database_directory_ = T2Utf(wszPath).get();
			request->use_message_database_ = false;
			request->use_secret_chats_ = true;
			request->api_id_ = 94575;
			request->api_hash_ = "a3406de8d171bb422bb6ddf3bbd800e2";
			request->system_language_code_ = "en";
			request->device_model_ = "Miranda NG";
			request->application_version_ = text;
			request->enable_storage_optimizer_ = true;
			SendQuery(request, &CMTProto::OnUpdateAuth);
		}
		break;

	case td::td_api::authorizationStateWaitPhoneNumber::ID:
		SendQuery(new td::td_api::setAuthenticationPhoneNumber(_T2A(m_szOwnPhone).get(), nullptr), &CMTProto::OnUpdateAuth);
		break;

	case td::td_api::authorizationStateWaitCode::ID:
		CallFunctionSync(EnterPhoneCode, this);
		break;

	case td::td_api::authorizationStateWaitPassword::ID:
		CallFunctionSync(EnterPassword, this);
		break;

	case td::td_api::authorizationStateReady::ID:
		OnLoggedIn();
		break;

	case td::td_api::authorizationStateLoggingOut::ID:
		debugLogA("Server required us to log out, exiting");
		LogOut();
		break;

	case td::td_api::authorizationStateClosing::ID:
		debugLogA("Connection terminated, exiting");
		LogOut();
		break;
	}
}

void CMTProto::OnUpdateAuth(td::ClientManager::Response &response)
{
	if (response.object->get_id() == td::td_api::error::ID) {
		auto *pError = (td::td_api::error*)response.object.get();
		debugLogA("error happened: %s", to_string(*pError).c_str());
		
		if (pError->message_ == "PHONE_CODE_EXPIRED")
			Popup(0, TranslateT("Phone code expired"), TranslateT("Error"));
		else if(pError->message_ == "INVALID_PHONE_CODE")
			Popup(0, TranslateT("Invalid phone code"), TranslateT("Error"));

		pAuthState = std::move(nullptr);
		LogOut();
	}
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::ProcessGroups(td::td_api::updateChatFilters *pObj)
{
	for (auto &grp : pObj->chat_filters_) {
		if (grp->icon_name_ != "Custom")
			continue;

		CMStringA szSetting("ChatFilter%d", grp->id_);
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
	}
}

void CMTProto::SendQuery(td::td_api::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	int queryId = ++m_iQueryId;
	m_pClientMmanager->send(m_iClientId, queryId, td::td_api::object_ptr<td::td_api::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
}
