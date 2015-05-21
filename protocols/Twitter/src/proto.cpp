/*
Copyright © 2012-15 Miranda NG team
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
#include "proto.h"

#include "utility.h"
#include "theme.h"
#include "ui.h"
#include "oauth.dev.h"

static volatile LONG g_msgid = 1;

TwitterProto::TwitterProto(const char *proto_name, const TCHAR *username) :
	PROTO<TwitterProto>(proto_name, username)
{
	CreateProtoService(PS_CREATEACCMGRUI, &TwitterProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETNAME, &TwitterProto::GetName);
	CreateProtoService(PS_GETSTATUS, &TwitterProto::GetStatus);

	CreateProtoService(PS_JOINCHAT, &TwitterProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &TwitterProto::OnLeaveChat);

	CreateProtoService(PS_GETMYAVATAR, &TwitterProto::GetAvatar);
	CreateProtoService(PS_SETMYAVATAR, &TwitterProto::SetAvatar);

	HookProtoEvent(ME_OPT_INITIALISE, &TwitterProto::OnOptionsInit);
	HookProtoEvent(ME_DB_CONTACT_DELETED, &TwitterProto::OnContactDeleted);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &TwitterProto::OnBuildStatusMenu);

	// Initialize hotkeys
	char text[512];
	mir_snprintf(text, SIZEOF(text), "%s/Tweet", m_szModuleName);

	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.pszSection = m_szModuleName; // Section title; TODO: use username?
	hkd.pszDescription = "Send Tweet";
	Hotkey_Register(&hkd);

	// set Tokens and stuff

	//mirandas keys
	ConsumerKey = OAUTH_CONSUMER_KEY;
	ConsumerSecret = OAUTH_CONSUMER_SECRET;

	AuthorizeUrl = _T("https://api.twitter.com/oauth/authorize?oauth_token=%s");
}

TwitterProto::~TwitterProto()
{
	twit_.Disconnect();

	if (m_hNetlibUser)
		Netlib_CloseHandle(m_hNetlibUser);
	if (hAvatarNetlib_)
		Netlib_CloseHandle(hAvatarNetlib_);
}

// *************************

DWORD_PTR TwitterProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL |
			PF1_SERVERCLIST | PF1_CHANGEINFO;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_AVATARS;
	case PFLAG_MAXLENOFMESSAGE:
		return 159; // 140 + <max length of a users name (15 apparently)> + 4 ("RT @").  this allows for the new style retweets
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR) "Username";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)TWITTER_KEY_UN;
	}
	return 0;
}

// *************************

int TwitterProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	Proto_RecvMessage(hContact, pre);
	return 0;
}

// *************************

struct send_direct
{
	__inline send_direct(MCONTACT _hContact, const std::string &_msg, int _msgid) :
		hContact(_hContact), msg(_msg), msgid(_msgid)
	{}

	MCONTACT hContact;
	std::string msg;
	int msgid;
};

void TwitterProto::SendSuccess(void *p)
{
	if (p == 0)
		return;
	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;
	if (!db_get_s(data->hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
		mir_cslock s(twitter_lock_);
		twit_.send_direct(dbv.pszVal, data->msg);

		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)data->msgid, 0);
		db_free(&dbv);
	}

	delete data;
}

int TwitterProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	int seq = InterlockedIncrement(&g_msgid);
	ForkThread(&TwitterProto::SendSuccess, new send_direct(hContact, msg, seq));
	return seq;
}

// *************************

int TwitterProto::SetStatus(int new_status)
{
	int old_status = m_iStatus;
	if (new_status == m_iStatus)
		return 0;

	m_iDesiredStatus = new_status;
	// 40072 - 40080 are the "online" statuses, basically every status except offline.  see statusmodes.h
	if (new_status >= 40072 && new_status <= 40080) {

		m_iDesiredStatus = ID_STATUS_ONLINE; //i think i have to set this so it forces the twitter proto to be online (and not away, DND, etc)

		// if we're already connecting and they want to go online, BAIL!  we're already trying to connect you dumbass
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		// if we're already connected, and we change to another connected status, don't try and reconnect!
		if (old_status >= 40072 && old_status <= 40080)
			return 0;

		// i think here we tell the proto interface struct that we're connecting, just so it knows
		m_iStatus = ID_STATUS_CONNECTING;
		// ok.. here i think we're telling the core that this protocol something.. but why?
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ForkThread(&TwitterProto::SignOn, this);
	}
	else if (new_status == ID_STATUS_OFFLINE) {
		twit_.Disconnect();
		m_iStatus = m_iDesiredStatus;
		SetAllContactStatuses(ID_STATUS_OFFLINE);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	}

	return 0;
}

// *************************

int TwitterProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event) {
	case EV_PROTO_ONLOAD:    return OnModulesLoaded(wParam, lParam);
	case EV_PROTO_ONEXIT:    return OnPreShutdown(wParam, lParam);
	case EV_PROTO_ONOPTIONS: return OnOptionsInit(wParam, lParam);
	}

	return 1;
}

// *************************

INT_PTR TwitterProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_TWITTERACCOUNT), (HWND)lParam, first_run_dialog, (LPARAM)this);
}

INT_PTR TwitterProto::GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy(reinterpret_cast<char*>(lParam), m_szModuleName, (int)wParam);
	return 0;
}

INT_PTR TwitterProto::GetStatus(WPARAM, LPARAM)
{
	return m_iStatus;
}

INT_PTR TwitterProto::ReplyToTweet(WPARAM hContact, LPARAM)
{
	// TODO: support replying to tweets instead of just users
	HWND hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_TWEET), 0, tweet_proc, reinterpret_cast<LPARAM>(this));

	DBVARIANT dbv;
	if (!db_get_s(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
		SendMessage(hDlg, WM_SETREPLY, reinterpret_cast<WPARAM>(dbv.pszVal), 0);
		db_free(&dbv);
	}

	ShowWindow(hDlg, SW_SHOW);

	return 0;
}

INT_PTR TwitterProto::VisitHomepage(WPARAM hContact, LPARAM)
{
	DBVARIANT dbv;
	// TODO: remove this
	if (!db_get_s(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
		std::string url = profile_base_url("https://twitter.com/") + http::url_encode(dbv.pszVal);
		db_set_s(hContact, m_szModuleName, "Homepage", url.c_str());

		CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, reinterpret_cast<LPARAM>(url.c_str()));
		db_free(&dbv);
	}

	return 0;
}

// *************************

int TwitterProto::OnBuildStatusMenu(WPARAM, LPARAM)
{
	HGENMENU hRoot = pcli->pfnGetProtocolMenu(m_szModuleName);
	if (hRoot == NULL)
		return 0;

	char text[200];
	strcpy(text, m_szModuleName);
	char *tDest = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	mi.hParentMenu = hRoot;
	mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi.position = 1001;
	Menu_AddStatusMenuItem(&mi);

	// TODO: Disable this menu item when offline
	// "Send Tweet..."
	CreateProtoService("/Tweet", &TwitterProto::OnTweet);
	strcpy(tDest, "/Tweet");
	mi.ptszName = LPGENT("Send Tweet...");
	mi.popupPosition = 200001;
	mi.icolibItem = GetIconHandle("tweet");
	Menu_AddStatusMenuItem(&mi);
	return 0;
}

int TwitterProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 271828;
	odp.hInstance = g_hInstance;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab = LPGENT("Basic");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = options_proc;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.ptszTab = LPGENT("Popups");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_POPUPS);
		odp.pfnDlgProc = popup_options_proc;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

INT_PTR TwitterProto::OnTweet(WPARAM, LPARAM)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 1;

	HWND hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_TWEET), 0, tweet_proc, reinterpret_cast<LPARAM>(this));
	ShowWindow(hDlg, SW_SHOW);
	return 0;
}

int TwitterProto::OnModulesLoaded(WPARAM, LPARAM)
{
	TCHAR descr[512];
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;

	// Create standard network connection
	mir_sntprintf(descr, SIZEOF(descr), TranslateT("%s server connection"), m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	if (m_hNetlibUser == 0)
		MessageBox(0, TranslateT("Unable to get Netlib connection for Twitter"), TranslateT("Twitter"), 0);

	// Create avatar network connection (TODO: probably remove this)
	char module[512];
	mir_snprintf(module, SIZEOF(module), "%sAv", m_szModuleName);
	nlu.szSettingsModule = module;
	mir_sntprintf(descr, SIZEOF(descr), TranslateT("%s avatar connection"), m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	hAvatarNetlib_ = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	if (hAvatarNetlib_ == 0)
		MessageBox(0, TranslateT("Unable to get avatar Netlib connection for Twitter"), TranslateT("Twitter"), 0);

	twit_.set_handle(this, m_hNetlibUser);

	GCREGISTER gcr = { sizeof(gcr) };
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = 159;
	CallService(MS_GC_REGISTER, 0, reinterpret_cast<LPARAM>(&gcr));

	DBEVENTTYPEDESCR evt = { sizeof(evt) };
	evt.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
	evt.module = m_szModuleName;
	evt.descr = "Tweet";
	evt.flags = DETF_HISTORY | DETF_MSGWINDOW;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, reinterpret_cast<LPARAM>(&evt));

	SetAllContactStatuses(ID_STATUS_OFFLINE); // In case we crashed last time
	return 0;
}

int TwitterProto::OnPreShutdown(WPARAM, LPARAM)
{
	Netlib_Shutdown(m_hNetlibUser);
	Netlib_Shutdown(hAvatarNetlib_);
	return 0;
}

int TwitterProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (IsMyContact(hContact))
		ShowContactMenus(true);

	return 0;
}

int TwitterProto::ShowPinDialog()
{
	HWND hDlg = (HWND)DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_TWITTERPIN), 0, pin_proc, reinterpret_cast<LPARAM>(this));
	ShowWindow(hDlg, SW_SHOW);
	return 0;
}

void TwitterProto::ShowPopup(const wchar_t *text, int Error)
{
	POPUPDATAT popup = {};
	mir_sntprintf(popup.lptzContactName, SIZEOF(popup.lptzContactName), TranslateT("%s Protocol"), m_tszUserName);
	wcsncpy_s(popup.lptzText, text, _TRUNCATE);

	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}

	if (ServiceExists(MS_POPUP_ADDPOPUPT))
		PUAddPopupT(&popup);
	else
		MessageBox(0, popup.lptzText, popup.lptzContactName, 0);
}

void TwitterProto::ShowPopup(const char *text, int Error)
{
	POPUPDATAT popup = {};
	mir_sntprintf(popup.lptzContactName, SIZEOF(popup.lptzContactName), TranslateT("%s Protocol"), m_tszUserName);
	mbcs_to_tcs(CP_UTF8, text, popup.lptzText, SIZEOF(popup.lptzText));
	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}

	if (ServiceExists(MS_POPUP_ADDPOPUPT))
		PUAddPopupT(&popup);
	else
		MessageBox(0, popup.lptzText, popup.lptzContactName, 0);
}

// TODO: the more I think about it, the more I think all twit.* methods should
// be in MessageLoop
void TwitterProto::SendTweetWorker(void *p)
{
	if (p == 0)
		return;

	char *text = static_cast<char*>(p);
	if (strlen(text) > 140) { // looks like the chat max outgoing msg thing doesn't work, so i'll do it here.
		TCHAR errorPopup[280];
		mir_sntprintf(errorPopup, SIZEOF(errorPopup), _T("Don't be crazy! Everyone knows the max tweet size is 140, and you're trying to fit %d chars in there?"), strlen(text));
		ShowPopup(errorPopup, 1);
		return;
	}

	mir_cslock s(twitter_lock_);
	twit_.set_status(text);

	mir_free(text);
}

void TwitterProto::UpdateSettings()
{
	if (db_get_b(0, m_szModuleName, TWITTER_KEY_CHATFEED, 0)) {
		if (!in_chat_)
			OnJoinChat(0, 0);
	}
	else {
		if (in_chat_)
			OnLeaveChat(0, 0);

		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact;) {
			MCONTACT hNext = db_find_next(hContact, m_szModuleName);
			if (isChatRoom(hContact))
				CallService(MS_DB_CONTACT_DELETE, WPARAM(hContact), 0);
			hContact = hNext;
		}
	}
}

std::tstring TwitterProto::GetAvatarFolder()
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%s"), VARST(_T("%miranda_avatarcache%")), m_tszUserName);
	return path;
}

INT_PTR TwitterProto::GetAvatar(WPARAM, LPARAM)
{
	return 0;
}

INT_PTR TwitterProto::SetAvatar(WPARAM, LPARAM)
{
	return 0;
}
