/*
 * $Id: services.cpp 14142 2012-03-09 21:11:27Z george.hazan $
 *
 * myYahoo Miranda Plugin
 *
 * Authors: Gennady Feldman (aka Gena01)
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#include "stdafx.h"

#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_nudge.h>

#include "avatar.h"
#include "resource.h"
#include "file_transfer.h"
#include "im.h"
#include "search.h"

void CYahooProto::logoff_buddies()
{
	//set all contacts to 'offline'
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		setWord(hContact, "Status", ID_STATUS_OFFLINE);
		setDword(hContact, "IdleTS", 0);
		setDword(hContact, "PictLastCheck", 0);
		setDword(hContact, "PictLoading", 0);
		db_unset(hContact, "CList", "StatusMsg");
		delSetting(hContact, "YMsg");
		delSetting(hContact, "YGMsg");
	}
}

//=======================================================
//Broadcast the user status
//=======================================================
void CYahooProto::BroadcastStatus(int s)
{
	int oldStatus = m_iStatus;
	if (oldStatus == s)
		return;

	//m_iStatus = s;
	switch (s) {
	case ID_STATUS_OFFLINE:
	case ID_STATUS_CONNECTING:
	case ID_STATUS_ONLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
	case ID_STATUS_OCCUPIED:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
	case ID_STATUS_INVISIBLE:
		m_iStatus = s;
		break;
	case ID_STATUS_DND:
		m_iStatus = ID_STATUS_OCCUPIED;
		break;
	default:
		m_iStatus = ID_STATUS_ONLINE;
	}

	debugLogA("[yahoo_util_broadcaststatus] Old Status: %S (%d), New Status: %S (%d)",
		pcli->pfnGetStatusModeDescription(oldStatus, 0), oldStatus,
		pcli->pfnGetStatusModeDescription(m_iStatus, 0), m_iStatus);
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, (LPARAM)m_iStatus);
}

//=======================================================
//Contact deletion event
//=======================================================
int __cdecl CYahooProto::OnContactDeleted(WPARAM hContact, LPARAM lParam)
{
	debugLogA("[YahooContactDeleted]");

	if (!m_bLoggedIn) {//should never happen for Yahoo contacts
		debugLogA("[YahooContactDeleted] We are not Logged On!!!");
		return 0;
	}

	// he is not a permanent contact!
	if (db_get_b(hContact, "CList", "NotOnList") != 0) {
		debugLogA("[YahooContactDeleted] Not a permanent buddy!!!");
		return 0;
	}

	DBVARIANT dbv;
	if (!getString(hContact, YAHOO_LOGINID, &dbv)) {
		debugLogA("[YahooContactDeleted] Removing %s", dbv.pszVal);
		remove_buddy(dbv.pszVal, getWord(hContact, "yprotoid", 0));

		db_free(&dbv);
	}
	else debugLogA("[YahooContactDeleted] Can't retrieve contact Yahoo ID");

	return 0;
}

//=======================================================
//Custom status message windows handling
//=======================================================
static INT_PTR CALLBACK DlgProcSetCustStat(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CYahooProto* ppro = (CYahooProto*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)ppro->LoadIconEx("yahoo", true));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)ppro->LoadIconEx("yahoo"));

			if (!ppro->getString(YAHOO_CUSTSTATDB, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_CUSTSTAT, dbv.pszVal);

				EnableWindow(GetDlgItem(hwndDlg, IDOK), mir_strlen(dbv.pszVal) > 0);
				db_free(&dbv);
			}
			else {
				SetDlgItemTextA(hwndDlg, IDC_CUSTSTAT, "");
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
			}

			CheckDlgButton(hwndDlg, IDC_CUSTSTATBUSY, ppro->getByte("BusyCustStat", 0) ? BST_CHECKED : BST_UNCHECKED);
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			{
				char str[255 + 1];
				CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				/* Get String from dialog */
				GetDlgItemTextA(hwndDlg, IDC_CUSTSTAT, str, _countof(str));

				/* Save it for later use */
				ppro->setString(YAHOO_CUSTSTATDB, str);
				ppro->setByte("BusyCustStat", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CUSTSTATBUSY));

				/* set for Idle/AA */
				if (ppro->m_startMsg) mir_free(ppro->m_startMsg);
				ppro->m_startMsg = mir_strdup(str);

				/* notify Server about status change */
				ppro->set_status(YAHOO_CUSTOM_STATUS, str, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CUSTSTATBUSY));

				/* change local/miranda status */
				ppro->BroadcastStatus((BYTE)IsDlgButtonChecked(hwndDlg, IDC_CUSTSTATBUSY) ? ID_STATUS_AWAY : ID_STATUS_ONLINE);
			}
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}

		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
			if (LOWORD(wParam) == IDC_CUSTSTAT) {
				char str[255 + 1];

				BOOL toSet;

				toSet = GetDlgItemTextA(hwndDlg, IDC_CUSTSTAT, str, _countof(str)) != 0;

				EnableWindow(GetDlgItem(hwndDlg, IDOK), toSet);
			}
		}
		break; /* case WM_COMMAND */

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		{
			CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			ppro->ReleaseIconEx("yahoo", true);
			ppro->ReleaseIconEx("yahoo");
		}
		break;
	}
	return FALSE;
}

INT_PTR __cdecl CYahooProto::SetCustomStatCommand(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn) {
		ShowNotification(TranslateT("Yahoo Error"), TranslateT("You need to be connected to set the custom message"), NIIF_ERROR);
		return 0;
	}

	HWND hwndSetCustomStatus = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_SETCUSTSTAT), NULL, DlgProcSetCustStat, (LPARAM)this);
	SetForegroundWindow(hwndSetCustomStatus);
	SetFocus(hwndSetCustomStatus);
	ShowWindow(hwndSetCustomStatus, SW_SHOW);
	return 0;
}

//=======================================================
//Open URL
//=======================================================
void CYahooProto::OpenURL(const char *url, int autoLogin)
{
	char tUrl[4096];

	debugLogA("[YahooOpenURL] url: %s Auto Login: %d", url, autoLogin);

	if (autoLogin && getByte("MailAutoLogin", 0) && m_bLoggedIn && m_id > 0) {
		char  *y, *t, *u;

		y = yahoo_urlencode(yahoo_get_cookie(m_id, "y"));
		t = yahoo_urlencode(yahoo_get_cookie(m_id, "t"));
		u = yahoo_urlencode(url);
		mir_snprintf(tUrl, _countof(tUrl),
			"http://msg.edit.yahoo.com/config/reset_cookies?&.y=Y=%s&.t=T=%s&.ver=2&.done=http%%3a//us.rd.yahoo.com/messenger/client/%%3f%s",
			y, t, u);

		FREE(y);
		FREE(t);
		FREE(u);
	}
	else {
		mir_snprintf(tUrl, _countof(tUrl), url);
	}

	debugLogA("[YahooOpenURL] url: %s Final URL: %s", url, tUrl);
	Utils_OpenUrl(tUrl);
}

//=======================================================
// Show buddy profile
//=======================================================
INT_PTR __cdecl CYahooProto::OnShowProfileCommand(WPARAM wParam, LPARAM lParam)
{
	char tUrl[4096];
	DBVARIANT dbv;

	/**
	 * We don't show profile for users using other IM clients through the IM server bridge
	 */
	if (getWord(wParam, "yprotoid", 0) != 0) {
		return 0;
	}

	if (getString(wParam, YAHOO_LOGINID, &dbv))
		return 0;

	mir_snprintf(tUrl, _countof(tUrl), "http://profiles.yahoo.com/%s", dbv.pszVal);
	db_free(&dbv);

	OpenURL(tUrl, 0);
	return 0;
}

INT_PTR __cdecl CYahooProto::OnEditMyProfile(WPARAM, LPARAM)
{
	OpenURL("http://edit.yahoo.com/config/eval_profile", 1);
	return 0;
}

//=======================================================
//Show My profile
//=======================================================
INT_PTR __cdecl CYahooProto::OnShowMyProfileCommand(WPARAM, LPARAM)
{
	DBVARIANT dbv;

	if (getString(YAHOO_LOGINID, &dbv) != 0) {
		ShowError(TranslateT("Yahoo Error"), TranslateT("Please enter your Yahoo ID in Options/Network/Yahoo"));
		return 0;
	}

	char tUrl[4096];
	mir_snprintf(tUrl, _countof(tUrl), "http://profiles.yahoo.com/%s", dbv.pszVal);
	db_free(&dbv);

	OpenURL(tUrl, 0);
	return 0;
}

//=======================================================
//Show Goto mailbox
//=======================================================
INT_PTR __cdecl CYahooProto::OnGotoMailboxCommand(WPARAM, LPARAM)
{
	OpenURL(getByte("YahooJapan", 0) ? "http://mail.yahoo.co.jp/" : "http://mail.yahoo.com/", 1);
	return 0;
}

INT_PTR __cdecl CYahooProto::OnABCommand(WPARAM, LPARAM)
{
	OpenURL("http://address.yahoo.com/yab/", 1);
	return 0;
}

INT_PTR __cdecl CYahooProto::OnCalendarCommand(WPARAM, LPARAM)
{
	OpenURL("http://calendar.yahoo.com/", 1);
	return 0;
}

//=======================================================
//Refresh Yahoo
//=======================================================
INT_PTR __cdecl CYahooProto::OnRefreshCommand(WPARAM, LPARAM)
{
	if (!m_bLoggedIn) {
		ShowNotification(TranslateT("Yahoo Error"), TranslateT("You need to be connected to refresh your buddy list"), NIIF_ERROR);
		return 0;
	}

	yahoo_refresh(m_id);
	return 0;
}

int __cdecl CYahooProto::OnIdleEvent(WPARAM wParam, LPARAM lParam)
{
	BOOL bIdle = (lParam & IDF_ISIDLE);

	debugLogA("[YAHOO_IDLE_EVENT] Idle: %s", bIdle ? "Yes" : "No");

	if (lParam & IDF_PRIVACY)
		return 0; /* we support Privacy settings */

	if (m_bLoggedIn) {
		/* set me to idle or back */
		if (m_iStatus == ID_STATUS_INVISIBLE)
			debugLogA("[YAHOO_IDLE_EVENT] WARNING: INVISIBLE! Don't change my status!");
		else
			set_status(m_iStatus, m_startMsg, (bIdle) ? 2 : (m_iStatus == ID_STATUS_ONLINE) ? 0 : 1);
	}
	else {
		debugLogA("[YAHOO_IDLE_EVENT] WARNING: NOT LOGGED IN???");
	}

	return 0;
}

INT_PTR __cdecl CYahooProto::GetUnreadEmailCount(WPARAM, LPARAM)
{
	return m_bLoggedIn ? m_unreadMessages : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CYahooProto::MenuMainInit(void)
{
	CMenuItem mi;
	mi.flags = 0;
	mi.root = Menu_GetProtocolRoot(this);

	// Show custom status menu
	mi.pszService = YAHOO_SET_CUST_STAT;
	CreateProtoService(mi.pszService, &CYahooProto::SetCustomStatCommand);
	mi.position = 290000;
	mi.hIcolibItem = GetIconHandle(IDI_SET_STATUS);
	mi.name.a = LPGEN("Set &Custom Status");
	menuItemsAll[0] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Edit My profile
	mi.pszService = YAHOO_EDIT_MY_PROFILE;
	CreateProtoService(mi.pszService, &CYahooProto::OnEditMyProfile);
	mi.position = 290005;
	mi.hIcolibItem = GetIconHandle(IDI_PROFILE);
	mi.name.a = LPGEN("&Edit My Profile");
	menuItemsAll[1] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Show My profile
	mi.pszService = YAHOO_SHOW_MY_PROFILE;
	CreateProtoService(mi.pszService, &CYahooProto::OnShowMyProfileCommand);
	mi.position = 290006;
	mi.hIcolibItem = GetIconHandle(IDI_PROFILE);
	mi.name.a = LPGEN("&My Profile");
	menuItemsAll[2] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Show Yahoo mail
	mi.pszService = YAHOO_YAHOO_MAIL;
	CreateProtoService(mi.pszService, &CYahooProto::OnGotoMailboxCommand);
	mi.position = 290010;
	mi.hIcolibItem = GetIconHandle(IDI_INBOX);
	mi.name.a = LPGEN("&Yahoo Mail");
	menuItemsAll[3] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Show Address Book
	mi.pszService = YAHOO_AB;
	CreateProtoService(mi.pszService, &CYahooProto::OnABCommand);
	mi.position = 290015;
	mi.hIcolibItem = GetIconHandle(IDI_YAB);
	mi.name.a = LPGEN("&Address Book");
	menuItemsAll[4] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Show Calendar
	mi.pszService = YAHOO_CALENDAR;
	CreateProtoService(mi.pszService, &CYahooProto::OnCalendarCommand);
	mi.position = 290017;
	mi.hIcolibItem = GetIconHandle(IDI_CALENDAR);
	mi.name.a = LPGEN("&Calendar");
	menuItemsAll[5] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Join chatroom
	mi.pszService = "/JoinChatRoom";
	CreateProtoService(mi.pszService, &CYahooProto::CreateConference);
	mi.position = 290018;
	mi.hIcolibItem = GetIconHandle(IDI_YAHOO);
	mi.name.a = LPGEN("Create Conference");
	menuItemsAll[6] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CYahooProto::MenuContactInit(void)
{
	// Show Profile
	CreateProtoService(YAHOO_SHOW_PROFILE, &CYahooProto::OnShowProfileCommand);

	CMenuItem mi;
	mi.pszService = YAHOO_SHOW_PROFILE;
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle(IDI_PROFILE);
	mi.name.a = LPGEN("&Show Profile");
	hShowProfileMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);
}

void CYahooProto::MenuUninit(void)
{
	Menu_RemoveItem(hShowProfileMenuItem);
}

int __cdecl CYahooProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!IsMyContact(hContact)) {
		debugLogA("[OnPrebuildContactMenu] Not a Yahoo Contact!!!");
		return 0;
	}

	Menu_ShowItem(hShowProfileMenuItem, getWord(hContact, "yprotoid", 0) == 0);
	return 0;
}


//=======================================================================================
// Load the yahoo service/plugin
//=======================================================================================
void CYahooProto::LoadYahooServices(void)
{
	//----| Events hooking |--------------------------------------------------------------
	HookProtoEvent(ME_OPT_INITIALISE, &CYahooProto::OnOptionsInit);

	//----| Create nudge event |----------------------------------------------------------
	hYahooNudge = CreateProtoEvent(PE_NUDGE);

	//----| Service creation |------------------------------------------------------------
	CreateProtoService(PS_CREATEACCMGRUI, &CYahooProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &CYahooProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CYahooProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CYahooProto::SetMyAvatar);
	CreateProtoService(PS_GETAVATARCAPS, &CYahooProto::GetAvatarCaps);

	CreateProtoService(PS_GETMYAWAYMSG, &CYahooProto::GetMyAwayMsg);
	CreateProtoService(PS_SEND_NUDGE, &CYahooProto::SendNudge);

	CreateProtoService(PS_GETUNREADEMAILCOUNT, &CYahooProto::GetUnreadEmailCount);

	//----| Set resident variables |------------------------------------------------------

	db_set_resident(m_szModuleName, "Mobile");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "PictLastCheck");
	db_set_resident(m_szModuleName, "PictLoading");

	db_set_resident(m_szModuleName, "YAway");
	db_set_resident(m_szModuleName, "YGMsg");
	db_set_resident(m_szModuleName, "YStatus");
}
