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

///////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CMTProto::EnterPhoneCode(void *param)
{
	auto *ppro = (CMTProto *)param;

	ENTER_STRING es = {};
	es.szModuleName = ppro->m_szModuleName;
	es.caption = TranslateT("Enter secret code sent to your phone");
	if (EnterString(&es)) {
		ppro->SendQuery(new TD::checkAuthenticationCode(_T2A(es.ptszResult).get()), &CMTProto::OnUpdateAuth);
		mir_free(es.ptszResult);
	}
	else ppro->LogOut();
	return 0;
}

INT_PTR CALLBACK CMTProto::EnterPassword(void *param)
{
	auto *ppro = (CMTProto *)param;
	CMStringW wszTitle(TranslateT("Enter password"));

	auto *pAuth = (TD::authorizationStateWaitPassword *)ppro->pAuthState.get();
	if (!pAuth->password_hint_.empty())
		wszTitle.AppendFormat(TranslateT(" (hint: %s)"), Utf2T(pAuth->password_hint_.c_str()).get());

	ENTER_STRING es = {};
	es.szModuleName = ppro->m_szModuleName;
	es.caption = wszTitle;
	es.type = ESF_PASSWORD;
	if (EnterString(&es)) {
		ppro->SendQuery(new TD::checkAuthenticationPassword(_T2A(es.ptszResult).get()), &CMTProto::OnUpdateAuth);
		mir_free(es.ptszResult);
	}
	else ppro->LogOut();
	return 0;
}

void CMTProto::ProcessAuth(TD::updateAuthorizationState *pObj)
{
	pAuthState = std::move(pObj->authorization_state_);
	switch (pAuthState->get_id()) {
	case TD::authorizationStateWaitTdlibParameters::ID:
		{
			char text[100];
			Miranda_GetVersionText(text, sizeof(text));

			CMStringW wszPath(GetProtoFolder());

			auto *request = new TD::setTdlibParameters();
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

	case TD::authorizationStateWaitPhoneNumber::ID:
		SendQuery(new TD::setAuthenticationPhoneNumber(_T2A(m_szOwnPhone).get(), nullptr), &CMTProto::OnUpdateAuth);
		break;

	case TD::authorizationStateWaitCode::ID:
		CallFunctionSync(EnterPhoneCode, this);
		break;

	case TD::authorizationStateWaitPassword::ID:
		CallFunctionSync(EnterPassword, this);
		break;

	case TD::authorizationStateReady::ID:
		OnLoggedIn();
		break;

	case TD::authorizationStateLoggingOut::ID:
		debugLogA("Server required us to log out, exiting");
		LogOut();
		break;

	case TD::authorizationStateClosing::ID:
		debugLogA("Connection terminated, exiting");
		LogOut();
		break;
	}
}

void CMTProto::OnUpdateAuth(td::ClientManager::Response &response)
{
	if (response.object->get_id() == TD::error::ID) {
		auto *pError = (TD::error *)response.object.get();
		debugLogA("error happened: %s", to_string(*pError).c_str());

		if (pError->message_ == "PHONE_CODE_EXPIRED")
			Popup(0, TranslateT("Phone code expired"), TranslateT("Error"));
		else if (pError->message_ == "INVALID_PHONE_CODE")
			Popup(0, TranslateT("Invalid phone code"), TranslateT("Error"));
		else if (pError->message_ == "PASSWORD_HASH_INVALID")
			Popup(0, TranslateT("Invalid password"), TranslateT("Error"));

		pAuthState = std::move(nullptr);
		LogOut();
	}
}
