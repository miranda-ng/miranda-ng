/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "commons.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HINSTANCE hInst;
int hLangpack = 0;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"My Details",
	PLUGIN_MAKE_VERSION(0,0,1,11),
	"Show and allows you to edit your details for all protocols.",
	"Ricardo Pescuma Domenecci, Drugwash",
	"",
	"© 2005-2008 Ricardo Pescuma Domenecci, Drugwash",
	"http://pescuma.org/miranda/mydetails",
	UNICODE_AWARE,
	{ 0xa82baeb3, 0xa33c, 0x4036, { 0xb8, 0x37, 0x78, 0x3, 0xa5, 0xb6, 0xc2, 0xab } } // {A82BAEB3-A33C-4036-B837-7803A5B6C2AB}
};

// Hooks
HANDLE hModulesLoadedHook = NULL;
HANDLE hPreShutdownHook = NULL;

long nickname_dialog_open;
HWND hwndSetNickname;

long status_msg_dialog_open;
HWND hwndSetStatusMsg;

// Hook called after init
static int MainInit(WPARAM wparam,LPARAM lparam);
static int MainUninit(WPARAM wParam, LPARAM lParam);

// Services
static int PluginCommand_SetMyNicknameUI(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyNickname(WPARAM wParam,LPARAM lParam);
static int PluginCommand_GetMyNickname(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyAvatarUI(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyAvatar(WPARAM wParam,LPARAM lParam);
static int PluginCommand_GetMyAvatar(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyStatusMessageUI(WPARAM wParam,LPARAM lParam);
static int PluginCommand_CicleThroughtProtocols(WPARAM wParam,LPARAM lParam);


// Functions //////////////////////////////////////////////////////////////////////////////////////


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID interfaces[] = { MIID_MDETAILS, MIID_LAST };

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&pluginInfo);

	// Hook event to load messages and show first one
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainUninit);

	nickname_dialog_open = 0;
	status_msg_dialog_open = 0;

	// Options
	InitOptions();

	// Register services
	CreateServiceFunction(MS_MYDETAILS_SETMYNICKNAME, PluginCommand_SetMyNickname);
	CreateServiceFunction(MS_MYDETAILS_SETMYNICKNAMEUI, PluginCommand_SetMyNicknameUI);
	CreateServiceFunction(MS_MYDETAILS_SETMYAVATAR, PluginCommand_SetMyAvatar);
	CreateServiceFunction(MS_MYDETAILS_SETMYAVATARUI, PluginCommand_SetMyAvatarUI);
	CreateServiceFunction(MS_MYDETAILS_GETMYNICKNAME, PluginCommand_GetMyNickname);
	CreateServiceFunction(MS_MYDETAILS_GETMYAVATAR, PluginCommand_GetMyAvatar);
	CreateServiceFunction(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, PluginCommand_SetMyStatusMessageUI);
	CreateServiceFunction(MS_MYDETAILS_SHOWNEXTPROTOCOL, PluginCommand_ShowNextProtocol);
	CreateServiceFunction(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, PluginCommand_ShowPreviousProtocol);
	CreateServiceFunction(MS_MYDETAILS_SHOWPROTOCOL, PluginCommand_ShowProtocol);
	CreateServiceFunction(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, PluginCommand_CicleThroughtProtocols);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyServiceFunction(MS_MYDETAILS_SETMYNICKNAME);
	DestroyServiceFunction(MS_MYDETAILS_SETMYNICKNAMEUI);
	DestroyServiceFunction(MS_MYDETAILS_SETMYAVATAR);
	DestroyServiceFunction(MS_MYDETAILS_SETMYAVATARUI);
	DestroyServiceFunction(MS_MYDETAILS_GETMYNICKNAME);
	DestroyServiceFunction(MS_MYDETAILS_GETMYAVATAR);
	DestroyServiceFunction(MS_MYDETAILS_SETMYSTATUSMESSAGEUI);
	DestroyServiceFunction(MS_MYDETAILS_SHOWNEXTPROTOCOL);
	DestroyServiceFunction(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL);
	DestroyServiceFunction(MS_MYDETAILS_SHOWPROTOCOL);
	DestroyServiceFunction(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS);

	DeInitProtocolData();
	DeInitOptions();

	return 0;
}


static int Menu_SetMyAvatarUI(WPARAM wParam,LPARAM lParam)
{
	return PluginCommand_SetMyAvatarUI(0, 0);
}
static int Menu_SetMyNicknameUI(WPARAM wParam,LPARAM lParam)
{
	return PluginCommand_SetMyNicknameUI(0, 0);
}
static int Menu_SetMyStatusMessageUI(WPARAM wParam,LPARAM lParam)
{
	return PluginCommand_SetMyStatusMessageUI(0, 0);
}


// Hook called after init
static int MainInit(WPARAM wparam,LPARAM lparam) 
{
	InitProtocolData();

	// Add options to menu
	CLISTMENUITEM mi;

	if (protocols->CanSetAvatars()) {
		ZeroMemory(&mi,sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.flags = 0;
		mi.popupPosition = 500050000;
		mi.pszPopupName = LPGEN("My Details");
		mi.position = 100001;
		mi.pszName = LPGEN("Set My Avatar...");
		CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYAVATARUI, Menu_SetMyAvatarUI);
		mi.pszService = "MENU_" MS_MYDETAILS_SETMYAVATARUI;
		Menu_AddMainMenuItem(&mi);
	}

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.popupPosition = 500050000;
	mi.pszPopupName = LPGEN("My Details");
	mi.position = 100002;
	mi.pszName = LPGEN("Set My Nickname...");
	CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYNICKNAMEUI, Menu_SetMyNicknameUI);
	mi.pszService = "MENU_" MS_MYDETAILS_SETMYNICKNAMEUI;
	Menu_AddMainMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.popupPosition = 500050000;
	mi.pszPopupName = LPGEN("My Details");
	mi.position = 100003;
	mi.pszName = LPGEN("Set My Status Message...");
	CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYSTATUSMESSAGEUI, Menu_SetMyStatusMessageUI);
	mi.pszService = "MENU_" MS_MYDETAILS_SETMYSTATUSMESSAGEUI;
	Menu_AddMainMenuItem(&mi);

	// Set protocols to show frame
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.popupPosition = 500050000;
	mi.pszPopupName = LPGEN("My Details");
	mi.position = 200001;
	mi.pszName = LPGEN("Show next protocol");
	mi.pszService = MS_MYDETAILS_SHOWNEXTPROTOCOL;
	Menu_AddMainMenuItem(&mi);

	InitFrames();

	if (CallService(MS_SKIN2_GETICON, 0, (LPARAM) "LISTENING_TO_ICON") == NULL) {
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = LPGEN("Contact List");
		sid.pszDescription = LPGEN("Listening to");
		sid.pszName = "LISTENING_TO_ICON";
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LISTENINGTO));
		Skin_AddIcon(&sid);
	}
	{
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = LPGEN("My Details");
		sid.pszDescription = LPGEN("Previous protocol");
		sid.pszName = "MYDETAILS_PREV_PROTOCOL";
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LEFT_ARROW));
		Skin_AddIcon(&sid);
	}
	{
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = LPGEN("My Details");
		sid.pszDescription = LPGEN("Next protocol");
		sid.pszName = "MYDETAILS_NEXT_PROTOCOL";
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RIGHT_ARROW));
		Skin_AddIcon(&sid);
	}

	return 0;
}

static int MainUninit(WPARAM wParam, LPARAM lParam) 
{
	DeInitFrames();
	return 0;
}

// Set nickname ///////////////////////////////////////////////////////////////////////////////////

#define WMU_SETDATA (WM_USER+1)

static BOOL CALLBACK DlgProcSetNickname(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg, IDC_NICKNAME), EM_LIMITTEXT, 
			MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE - 1, 0);
		return TRUE;

	case WMU_SETDATA:
		{
			int proto_num = (int)wParam;

			SetWindowLong(hwndDlg, GWL_USERDATA, proto_num);

			if (proto_num == -1) {
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

				// All protos have the same nick?
				if (protocols->GetSize() > 0) {
					TCHAR *nick = protocols->Get(0)->nickname;

					bool foundDefNick = true;
					for (int i=1 ; foundDefNick && i < protocols->GetSize() ; i++) {
						if (_tcsicmp(protocols->Get(i)->nickname, nick) != 0) {
							foundDefNick = false;
							break;
						}
					}

					if (foundDefNick)
						if ( _tcsicmp(protocols->default_nick, nick) != 0)
							lstrcpy(protocols->default_nick, nick);
				}

				SetDlgItemText(hwndDlg, IDC_NICKNAME, protocols->default_nick);
				SendDlgItemMessage(hwndDlg, IDC_NICKNAME, EM_LIMITTEXT, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE, 0);
			}
			else {
				Protocol *proto = protocols->Get(proto_num);

				TCHAR tmp[128];
				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set My Nickname for %s"), proto->description);

				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)tmp);

				HICON hIcon = (HICON)CallProtoService(proto->name, PS_LOADICON, PLI_PROTOCOL, 0);
				if (hIcon != NULL)
				{
					SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
					DestroyIcon(hIcon);
				}

				SetDlgItemText(hwndDlg, IDC_NICKNAME, proto->nickname);
				SendDlgItemMessage(hwndDlg, IDC_NICKNAME, EM_LIMITTEXT, 
					min(MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE, proto->GetNickMaxLength()), 0);
			}

			return TRUE;
		}

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			{
				TCHAR tmp[MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE];
				GetDlgItemText(hwndDlg, IDC_NICKNAME, tmp, SIZEOF(tmp));

				int proto_num = (int)GetWindowLong(hwndDlg, GWL_USERDATA);
				if (proto_num == -1)
					protocols->SetNicks(tmp);
				else
					protocols->Get(proto_num)->SetNick(tmp);

				DestroyWindow(hwndDlg);
				break;
			}
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		InterlockedExchange(&nickname_dialog_open, 0);
		break;
	}

	return FALSE;
}

static int PluginCommand_SetMyNicknameUI(WPARAM wParam,LPARAM lParam)
{
	char *proto = (char*)lParam;
	int proto_num = -1;

	if (proto != NULL) {
		int i;
		for (i = 0 ; i < protocols->GetSize() ; i++) {
			if (_stricmp(protocols->Get(i)->name, proto) == 0) {
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if ( !protocols->Get(i)->CanSetNick())
			return -2;

	}

	if ( !nickname_dialog_open) {
		InterlockedExchange(&nickname_dialog_open, 1);

		hwndSetNickname = CreateDialog(hInst, MAKEINTRESOURCE( IDD_SETNICKNAME ), NULL, DlgProcSetNickname );
		
		SendMessage(hwndSetNickname, WMU_SETDATA, proto_num, 0);
	}

	SetForegroundWindow( hwndSetNickname );
	SetFocus( hwndSetNickname );
 	ShowWindow( hwndSetNickname, SW_SHOW );

	return 0;
}

static int PluginCommand_SetMyNickname(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)wParam;
	if (proto != NULL) {
		for (int i = 0 ; i < protocols->GetSize() ; i++) {
			if (_stricmp(protocols->Get(i)->name, proto) == 0) {
				if ( !protocols->Get(i)->CanSetNick())
					return -2;

				protocols->Get(i)->SetNick((TCHAR*)lParam);
				return 0;
			}
		}

		return -1;
	}

	protocols->SetNicks((TCHAR*)lParam);
	return 0;
}

static int PluginCommand_GetMyNickname(WPARAM wParam,LPARAM lParam)
{
	TCHAR* ret = (TCHAR*)lParam;
	char * proto = (char *)wParam;

	if (ret == NULL)
		return -1;

	if (proto == NULL) {
		if (protocols->default_nick != NULL)
			lstrcpyn(ret, protocols->default_nick, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE);
		else
			ret[0] = '\0';

		return 0;
	}
	else {
		Protocol *protocol = protocols->Get(proto);
		if (protocol != NULL) {
			lstrcpyn(ret, protocol->nickname, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE);
			return 0;
		}

		return -1;
	}
}

// Set avatar /////////////////////////////////////////////////////////////////////////////////////

static int PluginCommand_SetMyAvatarUI(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)lParam;
	int proto_num = -1;

	if (proto != NULL)
	{
		int i;
		for (i = 0 ; i < protocols->GetSize() ; i++)
		{
			if (_stricmp(protocols->Get(i)->name, proto) == 0)
			{
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if ( !protocols->Get(i)->CanSetAvatar())
		{
			return -2;
		}
	}

	if (proto_num == -1)
	{
		protocols->SetAvatars(NULL);
	}
	else
	{
		protocols->Get(proto_num)->SetAvatar(NULL);
	}

	return 0;
}

static int PluginCommand_SetMyAvatar(WPARAM wParam,LPARAM lParam)
{
	char *proto = (char*)wParam;
	if (proto != NULL) {
		for (int i = 0 ; i < protocols->GetSize() ; i++) {
			if (_stricmp(protocols->Get(i)->name, proto) == 0) {
				if ( !protocols->Get(i)->CanSetAvatar())
					return -2;

				protocols->Get(i)->SetAvatar((TCHAR*)lParam);
				return 0;
			}
		}

		return -1;
	}

	protocols->SetAvatars((TCHAR*)lParam);
	return 0;
}

int Status2SkinIcon(int status)
{
	switch(status) {
		case ID_STATUS_AWAY: return SKINICON_STATUS_AWAY;
		case ID_STATUS_NA: return SKINICON_STATUS_NA;
		case ID_STATUS_DND: return SKINICON_STATUS_DND;
		case ID_STATUS_OCCUPIED: return SKINICON_STATUS_OCCUPIED;
		case ID_STATUS_FREECHAT: return SKINICON_STATUS_FREE4CHAT;
		case ID_STATUS_ONLINE: return SKINICON_STATUS_ONLINE;
		case ID_STATUS_OFFLINE: return SKINICON_STATUS_OFFLINE;
		case ID_STATUS_INVISIBLE: return SKINICON_STATUS_INVISIBLE;
		case ID_STATUS_ONTHEPHONE: return SKINICON_STATUS_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH: return SKINICON_STATUS_OUTTOLUNCH;
		case ID_STATUS_IDLE: return SKINICON_STATUS_AWAY;
	}
	return SKINICON_STATUS_OFFLINE;
}

static int PluginCommand_GetMyAvatar(WPARAM wParam,LPARAM lParam)
{
	TCHAR* ret = (TCHAR*)lParam;
	char * proto = (char *)wParam;

	if (ret == NULL)
		return -1;

	if (proto == NULL) {
		if (protocols->default_avatar_file != NULL)
			lstrcpyn(ret, protocols->default_avatar_file, MS_MYDETAILS_GETMYAVATAR_BUFFER_SIZE);
		else 
			ret[0] = '\0';

		return 0;
	}

	for (int i = 0 ; i < protocols->GetSize() ; i++) {
		if ( _stricmp(protocols->Get(i)->name, proto) == 0) {
			if ( !protocols->Get(i)->CanGetAvatar())
				return -2;

			protocols->Get(i)->GetAvatar();

			if (protocols->Get(i)->avatar_file != NULL)
				lstrcpyn(ret, protocols->Get(i)->avatar_file, MS_MYDETAILS_GETMYAVATAR_BUFFER_SIZE);
			else 
				ret[0] = '\0';

			return 0;
		}
	}

	return -1;
}

static LRESULT CALLBACK StatusMsgEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 0x0a && (GetKeyState(VK_CONTROL) & 0x8000) != 0) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;
	}

	return CallWindowProc((WNDPROC) GetWindowLong(hwnd, GWL_USERDATA), hwnd, msg, wParam, lParam);
}

struct SetStatusMessageData {
	int status;
	int proto_num;
};

static BOOL CALLBACK DlgProcSetStatusMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), EM_LIMITTEXT, 
			MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE - 1, 0);
		{
			WNDPROC old_proc = (WNDPROC) SetWindowLong(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), 
				GWL_WNDPROC, (LONG) StatusMsgEditSubclassProc);
			SetWindowLong(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), GWL_USERDATA, (long) old_proc);
		}
		return TRUE;

	case WMU_SETDATA:
		{
			SetStatusMessageData *data = (SetStatusMessageData *) malloc(sizeof(SetStatusMessageData));
			data->status = (int)wParam;
			data->proto_num = (int)lParam;

			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) data);

			if (data->proto_num >= 0) {
				Protocol *proto = protocols->Get(data->proto_num);

				HICON hIcon = (HICON)CallProtoService(proto->name, PS_LOADICON, PLI_PROTOCOL, 0);
				if (hIcon != NULL) {
					SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
					DestroyIcon(hIcon);
				}

				TCHAR title[256];
				mir_sntprintf(title, SIZEOF(title), TranslateT("Set My Status Message for %s"), proto->description);
				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, proto->GetStatusMsg());
			}
			else if (data->status != 0)
			{
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(Status2SkinIcon(data->status)));

				TCHAR title[256];
				mir_sntprintf(title, SIZEOF(title), TranslateT("Set My Status Message for %s"),
					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, data->status, GSMDF_TCHAR));
				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, protocols->GetDefaultStatusMsg(data->status));
			}
			else
			{
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, protocols->GetDefaultStatusMsg());
			}

			return TRUE;
		}
	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			{
				TCHAR tmp[MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE];
				GetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, tmp, sizeof(tmp));

				SetStatusMessageData *data = (SetStatusMessageData *) GetWindowLong(hwndDlg, GWL_USERDATA);

				if (data->proto_num >= 0)
					protocols->Get(data->proto_num)->SetStatusMsg(tmp);
				else if (data->status == 0)
					protocols->SetStatusMsgs(tmp);
				else
					protocols->SetStatusMsgs(data->status, tmp);

				DestroyWindow(hwndDlg);
			}
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		SetWindowLong(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), GWL_WNDPROC, 
			GetWindowLong(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), GWL_USERDATA));
		free((SetStatusMessageData *) GetWindowLong(hwndDlg, GWL_USERDATA));
		InterlockedExchange(&status_msg_dialog_open, 0);
		break;
	}

	return FALSE;
}

static int PluginCommand_SetMyStatusMessageUI(WPARAM wParam,LPARAM lParam)
{
	int status = (int)wParam;
	char * proto_name = (char *)lParam;
	int proto_num = -1;
	Protocol *proto = NULL;

	if (status != 0 && (status < ID_STATUS_OFFLINE || status > ID_STATUS_OUTTOLUNCH))
		return -10;

	if (proto_name != NULL)
	{
		for (int i = 0 ; i < protocols->GetSize() ; i++)
		{
			proto = protocols->Get(i);

			if (_stricmp(proto->name, proto_name) == 0)
			{
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1) {
			return -1;
		}

		if (protocols->CanSetStatusMsgPerProtocol() && !proto->CanSetStatusMsg()) {
			return -2;
		}
	}
	else if (ServiceExists(MS_SA_CHANGESTATUSMSG))
	{
		if (proto == NULL && status == 0)
		{
			CallService(MS_SA_CHANGESTATUSMSG, protocols->GetGlobalStatus(), NULL);
		}
		else if (status == 0)
		{
			CallService(MS_SA_CHANGESTATUSMSG, proto->status, (LPARAM) proto_name);
		}
		else
		{
			CallService(MS_SA_CHANGESTATUSMSG, status, (LPARAM) proto_name);
		}

		return 0;
	}
	
	if (proto == NULL || proto->status != ID_STATUS_OFFLINE)
	{
		if ( !status_msg_dialog_open)
		{
			InterlockedExchange(&status_msg_dialog_open, 1);

			hwndSetStatusMsg = CreateDialog(hInst, MAKEINTRESOURCE( IDD_SETSTATUSMESSAGE ), NULL, DlgProcSetStatusMessage );
			
			SendMessage(hwndSetStatusMsg, WMU_SETDATA, status, proto_num);
		}

		SetForegroundWindow( hwndSetStatusMsg );
		SetFocus( hwndSetStatusMsg );
 		ShowWindow( hwndSetStatusMsg, SW_SHOW );

		return 0;
	}

	return -3;
}


static int PluginCommand_CicleThroughtProtocols(WPARAM wParam,LPARAM lParam)
{
	DBWriteContactSettingByte(NULL,"MyDetails","CicleThroughtProtocols", (BYTE) wParam);

	LoadOptions();

	return 0;
}