/*
Copyright © 2012-22 Miranda NG team
Copyright © 2009 Jim Porter

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

#include "theme.h"
#include "ui.h"

static volatile LONG g_msgid = 1;

CTwitterProto::CTwitterProto(const char *proto_name, const wchar_t *username) :
	PROTO<CTwitterProto>(proto_name, username),
	m_szChatId(mir_utf8encodeW(username)),
	m_arChatMarks(10, NumericKeySortT)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CTwitterProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_JOINCHAT, &CTwitterProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &CTwitterProto::OnLeaveChat);

	CreateProtoService(PS_GETMYAVATAR, &CTwitterProto::GetAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CTwitterProto::SetAvatar);

	HookProtoEvent(ME_OPT_INITIALISE, &CTwitterProto::OnOptionsInit);
	HookProtoEvent(ME_DB_CONTACT_DELETED, &CTwitterProto::OnContactDeleted);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CTwitterProto::OnMarkedRead);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &CTwitterProto::OnBuildStatusMenu);

	// Initialize hotkeys
	char text[512];
	mir_snprintf(text, "%s/Tweet", m_szModuleName);

	HOTKEYDESC hkd = {};
	hkd.dwFlags = HKD_UNICODE;
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.szSection.w = m_tszUserName;
	hkd.szDescription.w = LPGENW("Send Tweet");
	g_plugin.addHotkey(&hkd);

	// register netlib handles
	wchar_t descr[512];
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;

	// Create standard network connection
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
	if (m_hNetlibUser == nullptr) {
		wchar_t error[200];
		mir_snwprintf(error, TranslateT("Unable to initialize Netlib for %s."), m_tszUserName);
		MessageBox(nullptr, error, L"Miranda NG", MB_OK | MB_ICONERROR);
	}

	// register group chats
	GCREGISTER gcr = {};
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = 159;
	Chat_Register(&gcr);

	// Create avatar network connection (TODO: probably remove this)
	char module[512];
	mir_snprintf(module, "%sAv", m_szModuleName);
	nlu.szSettingsModule = module;
	mir_snwprintf(descr, TranslateT("%s avatar connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr;
	hAvatarNetlib_ = Netlib_RegisterUser(&nlu);
	if (hAvatarNetlib_ == nullptr) {
		wchar_t error[200];
		mir_snwprintf(error, TranslateT("Unable to initialize Netlib for %s."), TranslateT("Twitter (avatars)"));
		MessageBox(nullptr, error, L"Miranda NG", MB_OK | MB_ICONERROR);
	}

	db_set_resident(m_szModuleName, "Homepage");
	for (auto &it : AccContacts())
		setString(it, "Homepage", "https://twitter.com/" + getMStringA(it, TWITTER_KEY_UN));
}

CTwitterProto::~CTwitterProto()
{
	Disconnect();

	if (hAvatarNetlib_)
		Netlib_CloseHandle(hAvatarNetlib_);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CTwitterProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SERVERCLIST | PF1_CHANGEINFO;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_AVATARS | PF4_SERVERMSGID;
	case PFLAG_MAXLENOFMESSAGE:
		return 159; // 140 + <max length of a users name (15 apparently)> + 4 ("RT @").  this allows for the new style retweets
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("User name");
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTwitterProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	CMStringA id(getMStringA(hContact, TWITTER_KEY_ID));
	if (id.IsEmpty())
		return 0;

	send_direct(id, msg);

	int seq = InterlockedIncrement(&g_msgid);
	ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)seq);
	return seq;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTwitterProto::SetStatus(int new_status)
{
	int old_status = m_iStatus;
	if (new_status == m_iStatus)
		return 0;

	m_iDesiredStatus = new_status;
	// 40072 - 40078 are the "online" statuses, basically every status except offline.  see statusmodes.h
	if (new_status >= 40072 && new_status <= 40078) {

		m_iDesiredStatus = ID_STATUS_ONLINE; //i think i have to set this so it forces the CTwitterProto proto to be online (and not away, DND, etc)

		// if we're already connecting and they want to go online, BAIL!  we're already trying to connect you dumbass
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		// if we're already connected, and we change to another connected status, don't try and reconnect!
		if (old_status >= 40072 && old_status <= 40078)
			return 0;

		// i think here we tell the proto interface struct that we're connecting, just so it knows
		m_iStatus = ID_STATUS_CONNECTING;
		// ok.. here i think we're telling the core that this protocol something.. but why?
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ForkThread(&CTwitterProto::SignOn, this);
	}
	else if (new_status == ID_STATUS_OFFLINE) {
		Disconnect();
		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE);
		SetChatStatus(ID_STATUS_OFFLINE);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CTwitterProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_TWITTERACCOUNT), (HWND)lParam, first_run_dialog, (LPARAM)this);
}

INT_PTR CTwitterProto::ReplyToTweet(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wParam;
	// TODO: support replying to tweets instead of just users
	HWND hDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_TWEET), nullptr, tweet_proc, reinterpret_cast<LPARAM>(this));

	DBVARIANT dbv;
	if (!getString(hContact, TWITTER_KEY_UN, &dbv)) {
		SendMessage(hDlg, WM_SETREPLY, reinterpret_cast<WPARAM>(dbv.pszVal), 0);
		db_free(&dbv);
	}

	ShowWindow(hDlg, SW_SHOW);

	return 0;
}

INT_PTR CTwitterProto::VisitHomepage(WPARAM hContact, LPARAM)
{
	Utils_OpenUrl(getMStringA(MCONTACT(hContact), "Homepage"));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTwitterProto::OnBuildStatusMenu(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNICODE;
	mi.position = 1001;
	Menu_AddStatusMenuItem(&mi, m_szModuleName);

	// TODO: Disable this menu item when offline
	// "Send Tweet..."
	mi.pszService = "/Tweet";
	CreateProtoService(mi.pszService, &CTwitterProto::OnTweet);
	mi.name.w = LPGENW("Send Tweet...");
	mi.position = 200001;
	mi.hIcolibItem = GetIconHandle("tweet");
	Menu_AddStatusMenuItem(&mi, m_szModuleName);
	return 0;
}

int CTwitterProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 271828;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	odp.szTab.w = LPGENW("Basic");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = options_proc;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Popups");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_POPUPS);
	odp.pfnDlgProc = popup_options_proc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

INT_PTR CTwitterProto::OnTweet(WPARAM, LPARAM)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 1;

	HWND hDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_TWEET), nullptr, tweet_proc, reinterpret_cast<LPARAM>(this));
	ShowWindow(hDlg, SW_SHOW);
	return 0;
}

void CTwitterProto::OnModulesLoaded()
{
	DBEVENTTYPEDESCR evt = {};
	evt.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
	evt.module = m_szModuleName;
	evt.descr = "Tweet";
	evt.flags = DETF_HISTORY | DETF_MSGWINDOW;
	DbEvent_RegisterType(&evt);

	setAllContactStatuses(ID_STATUS_OFFLINE); // In case we crashed last time
	SetChatStatus(ID_STATUS_OFFLINE);
}

int CTwitterProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wParam;
	if (IsMyContact(hContact))
		ShowContactMenus(true);

	return 0;
}

int CTwitterProto::ShowPinDialog()
{
	HWND hDlg = (HWND)DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_TWITTERPIN), nullptr, pin_proc, reinterpret_cast<LPARAM>(this));
	ShowWindow(hDlg, SW_SHOW);
	return 0;
}

void CTwitterProto::ShowPopup(const wchar_t *text, int Error)
{
	POPUPDATAW popup = {};
	mir_snwprintf(popup.lpwzContactName, TranslateT("%s Protocol"), m_tszUserName);
	wcsncpy_s(popup.lpwzText, text, _TRUNCATE);

	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}
	PUAddPopupW(&popup);
}

void CTwitterProto::ShowPopup(const char *text, int Error)
{
	POPUPDATAW popup = {};
	mir_snwprintf(popup.lpwzContactName, TranslateT("%s Protocol"), m_tszUserName);
	wcsncpy_s(popup.lpwzText, Utf2T(text), _TRUNCATE);
	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}
	PUAddPopupW(&popup);
}

/////////////////////////////////////////////////////////////////////////////////////////
// TODO: the more I think about it, the more I think all twit.* methods should
// be in MessageLoop

void CTwitterProto::SendTweetWorker(void *p)
{
	if (p == nullptr)
		return;

	char *text = static_cast<char*>(p);
	if (mir_strlen(mir_utf8decodeA(text)) > 140) { // looks like the chat max outgoing msg thing doesn't work, so i'll do it here.
		wchar_t errorPopup[280];
		mir_snwprintf(errorPopup, TranslateT("Don't be crazy! Everyone knows the max tweet size is 140, and you're trying to fit %d chars in there?"), mir_strlen(text));
		ShowPopup(errorPopup, 1);
		return;
	}

	mir_cslock s(twitter_lock_);
	set_status(text);

	mir_free(text);
}

void CTwitterProto::UpdateSettings()
{
	if (getByte(TWITTER_KEY_CHATFEED)) {
		if (!in_chat_)
			OnJoinChat(0, 0);
	}
	else {
		if (in_chat_)
			OnLeaveChat(0, 0);

		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact;) {
			MCONTACT hNext = db_find_next(hContact, m_szModuleName);
			if (isChatRoom(hContact))
				db_delete_contact(hContact);
			hContact = hNext;
		}
	}
}

CMStringW CTwitterProto::GetAvatarFolder()
{
	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\%s", VARSW(L"%miranda_avatarcache%").get(), m_tszUserName);
	return path;
}

INT_PTR CTwitterProto::GetAvatar(WPARAM, LPARAM)
{
	return 0;
}

INT_PTR CTwitterProto::SetAvatar(WPARAM, LPARAM)
{
	return 0;
}
