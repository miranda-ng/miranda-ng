/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

OmegleProto::OmegleProto(const char* proto_name, const wchar_t* username) :
	PROTO<OmegleProto>(proto_name, username)
{
	this->facy.parent = this;

	this->signon_lock_ = CreateMutex(nullptr, FALSE, nullptr);
	this->log_lock_ = CreateMutex(nullptr, FALSE, nullptr);
	this->facy.connection_lock_ = CreateMutex(nullptr, FALSE, nullptr);
	this->events_loop_lock_ = CreateMutex(nullptr, FALSE, nullptr);

	// Group chats
	CreateProtoService(PS_JOINCHAT, &OmegleProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &OmegleProto::OnLeaveChat);

	CreateProtoService(PS_CREATEACCMGRUI, &OmegleProto::SvcCreateAccMgrUI);

	HookProtoEvent(ME_OPT_INITIALISE, &OmegleProto::OnOptionsInit);
	HookProtoEvent(ME_GC_EVENT, &OmegleProto::OnChatEvent);

	// Create standard network connection
	wchar_t descr[512];
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
	if (m_hNetlibUser == nullptr) {
		wchar_t error[200];
		mir_snwprintf(error, TranslateT("Unable to initialize Netlib for %s."), m_tszUserName);
		MessageBox(nullptr, error, L"Miranda NG", MB_OK | MB_ICONERROR);
	}

	facy.set_handle(m_hNetlibUser);

	g_plugin.addSound("StrangerTyp", m_tszUserName, LPGENW("Stranger is typing"));
	g_plugin.addSound("StrangerTypStop", m_tszUserName, LPGENW("Stranger stopped typing"));
	g_plugin.addSound("StrangerChange", m_tszUserName, LPGENW("Changing stranger"));
	g_plugin.addSound("StrangerMessage", m_tszUserName, LPGENW("Receive message"));

	// Register group chat
	GCREGISTER gcr = {};
	gcr.dwFlags = 0; //GC_TYPNOTIF; //GC_ACKMSG;
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = OMEGLE_MESSAGE_LIMIT;
	Chat_Register(&gcr);
}

OmegleProto::~OmegleProto()
{
	Netlib_CloseHandle(m_hNetlibUser);

	WaitForSingleObject(this->signon_lock_, IGNORE);
	WaitForSingleObject(this->log_lock_, IGNORE);
	WaitForSingleObject(this->events_loop_lock_, IGNORE);

	CloseHandle(this->signon_lock_);
	CloseHandle(this->log_lock_);
	CloseHandle(this->events_loop_lock_);
	CloseHandle(this->facy.connection_lock_);
}

//////////////////////////////////////////////////////////////////////////////

INT_PTR OmegleProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_CHAT;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_SUPPORTTYPING;
	case PFLAG_MAXLENOFMESSAGE:
		return OMEGLE_MESSAGE_LIMIT;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Visible name");
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int OmegleProto::SetStatus(int new_status)
{
	// Routing statuses not supported by Omegle
	switch (new_status) {
	case ID_STATUS_OFFLINE:
	case ID_STATUS_CONNECTING:
		new_status = ID_STATUS_OFFLINE;
		break;
	default:
		new_status = ID_STATUS_ONLINE;
		break;
	}

	m_iDesiredStatus = new_status;

	if (new_status == m_iStatus)
		return 0;

	if (m_iStatus == ID_STATUS_CONNECTING && new_status != ID_STATUS_OFFLINE)
		return 0;

	if (new_status == ID_STATUS_OFFLINE)
		ForkThread(&OmegleProto::SignOff, this);
	else
		ForkThread(&OmegleProto::SignOn, this);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

INT_PTR OmegleProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_OmegleACCOUNT),
		(HWND)lParam, OmegleAccountProc, (LPARAM)this);
}

void OmegleProto::OnShutdown()
{
	SetStatus(ID_STATUS_OFFLINE);
}

int OmegleProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;

	odp.position = 271828;
	odp.szGroup.w = LPGENW("Network");
	odp.szTab.w = LPGENW("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = OmegleOptionsProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void OmegleProto::OnContactDeleted(MCONTACT)
{
	OnLeaveChat(NULL, NULL);
}

int OmegleProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && facy.state_ == STATE_ACTIVE)
		ForkThread(&OmegleProto::SendTypingWorker, new int(type));

	return 0;
}
