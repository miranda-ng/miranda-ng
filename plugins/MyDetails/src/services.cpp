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

static long nickname_dialog_open;
static HWND hwndSetNickname;

static long status_msg_dialog_open;
static HWND hwndSetStatusMsg;

// Set nickname ///////////////////////////////////////////////////////////////////////////////////

#define WMU_SETDATA (WM_USER + 1)

static INT_PTR CALLBACK DlgProcSetNickname(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg, IDC_NICKNAME), EM_LIMITTEXT, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE - 1, 0);
		return TRUE;

	case WMU_SETDATA:
		{
			int proto_num = (int)wParam;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, proto_num);

			if (proto_num == -1) {
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

				// All protos have the same nick?
				if (protocols->GetSize() > 0) {
					TCHAR *nick = protocols->Get(0)->nickname;

					bool foundDefNick = true;
					for (int i = 1; foundDefNick && i < protocols->GetSize(); i++) {
						if (_tcsicmp(protocols->Get(i)->nickname, nick) != 0) {
							foundDefNick = false;
							break;
						}
					}

					if (foundDefNick)
						if (_tcsicmp(protocols->default_nick, nick) != 0)
							lstrcpy(protocols->default_nick, nick);
				}

				SetDlgItemText(hwndDlg, IDC_NICKNAME, protocols->default_nick);
				SendDlgItemMessage(hwndDlg, IDC_NICKNAME, EM_LIMITTEXT, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE, 0);
			}
			else {
				Protocol *proto = protocols->Get(proto_num);

				TCHAR tmp[128];
				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my nickname for %s"), proto->description);

				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)tmp);

				HICON hIcon = (HICON)CallProtoService(proto->name, PS_LOADICON, PLI_PROTOCOL, 0);
				if (hIcon != NULL) {
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
		switch (wParam) {
		case IDOK:
			{
				TCHAR tmp[MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE];
				GetDlgItemText(hwndDlg, IDC_NICKNAME, tmp, SIZEOF(tmp));

				LONG_PTR proto_num = GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
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

INT_PTR PluginCommand_SetMyNicknameUI(WPARAM wParam, LPARAM lParam)
{
	char *proto = (char *)lParam;
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

		if (!protocols->Get(i)->CanSetNick())
			return -2;

	}

	if (!nickname_dialog_open) {
		InterlockedExchange(&nickname_dialog_open, 1);

		hwndSetNickname = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETNICKNAME), NULL, DlgProcSetNickname);

		SendMessage(hwndSetNickname, WMU_SETDATA, proto_num, 0);
	}

	SetForegroundWindow(hwndSetNickname);
	SetFocus(hwndSetNickname);
 	ShowWindow(hwndSetNickname, SW_SHOW);

	return 0;
}

INT_PTR PluginCommand_SetMyNickname(WPARAM wParam, LPARAM lParam)
{
	char *proto = (char *)wParam;
	if (proto != NULL) {
		for (int i = 0; i < protocols->GetSize(); i++) {
			if (_stricmp(protocols->Get(i)->name, proto) == 0) {
				if (!protocols->Get(i)->CanSetNick())
					return -2;

				protocols->Get(i)->SetNick((TCHAR *)lParam);
				return 0;
			}
		}

		return -1;
	}

	protocols->SetNicks((TCHAR *)lParam);
	return 0;
}

INT_PTR PluginCommand_GetMyNickname(WPARAM wParam, LPARAM lParam)
{
	TCHAR *ret = (TCHAR *)lParam;
	if (ret == NULL)
		return -1;

	char *proto = (char *)wParam;
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

INT_PTR PluginCommand_SetMyAvatarUI(WPARAM wParam, LPARAM lParam)
{
	char *proto = (char *)lParam;
	int proto_num = -1;

	if (proto != NULL) {
		int i;
		for (i = 0; i < protocols->GetSize(); i++) {
			if (_stricmp(protocols->Get(i)->name, proto) == 0) {
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if (!protocols->Get(i)->CanSetAvatar())
			return -2;
	}

	if (proto_num == -1)
		protocols->SetAvatars(NULL);
	else
		protocols->Get(proto_num)->SetAvatar(NULL);

	return 0;
}

INT_PTR PluginCommand_SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	char *proto = (char *)wParam;
	if (proto != NULL) {
		for (int i = 0; i < protocols->GetSize(); i++) {
			if (_stricmp(protocols->Get(i)->name, proto) == 0) {
				if (!protocols->Get(i)->CanSetAvatar())
					return -2;

				protocols->Get(i)->SetAvatar((TCHAR *)lParam);
				return 0;
			}
		}

		return -1;
	}

	protocols->SetAvatars((TCHAR *)lParam);
	return 0;
}

INT_PTR PluginCommand_GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *ret = (TCHAR *)lParam;
	char *proto = (char *)wParam;

	if (ret == NULL)
		return -1;

	if (proto == NULL) {
		if (protocols->default_avatar_file != NULL)
			lstrcpyn(ret, protocols->default_avatar_file, MS_MYDETAILS_GETMYAVATAR_BUFFER_SIZE);
		else
			ret[0] = '\0';

		return 0;
	}

	for (int i = 0; i < protocols->GetSize(); i++) {
		if (_stricmp(protocols->Get(i)->name, proto) == 0) {
			if (!protocols->Get(i)->CanGetAvatar())
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

	return mir_callNextSubclass(hwnd, StatusMsgEditSubclassProc, msg, wParam, lParam);
}

struct SetStatusMessageData {
	int status;
	int proto_num;
};

static INT_PTR CALLBACK DlgProcSetStatusMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), EM_LIMITTEXT, MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE - 1, 0);
		mir_subclassWindow(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), StatusMsgEditSubclassProc);
		return TRUE;

	case WMU_SETDATA:
		{
			SetStatusMessageData *data = (SetStatusMessageData *)malloc(sizeof(SetStatusMessageData));
			data->status = (int)wParam;
			data->proto_num = (int)lParam;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);

			if (data->proto_num >= 0) {
				Protocol *proto = protocols->Get(data->proto_num);

				HICON hIcon = (HICON)CallProtoService(proto->name, PS_LOADICON, PLI_PROTOCOL, 0);
				if (hIcon != NULL) {
					SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
					DestroyIcon(hIcon);
				}

				TCHAR title[256];
				mir_sntprintf(title, SIZEOF(title), TranslateT("Set my status message for %s"), proto->description);
				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, proto->GetStatusMsg());
			}
			else if (data->status != 0) {
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIcon(NULL,data->status));

				TCHAR title[256];
				mir_sntprintf(title, SIZEOF(title), TranslateT("Set my status message for %s"),
					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, data->status, GSMDF_TCHAR));
				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, protocols->GetDefaultStatusMsg(data->status));
			}
			else {
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, protocols->GetDefaultStatusMsg());
			}

			return TRUE;
		}
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			{
				TCHAR tmp[MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE];
				GetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, tmp, sizeof(tmp));

				SetStatusMessageData *data = (SetStatusMessageData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

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
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), GWLP_WNDPROC,
			GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), GWLP_USERDATA));
		free((SetStatusMessageData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
		InterlockedExchange(&status_msg_dialog_open, 0);
		break;
	}

	return FALSE;
}

INT_PTR PluginCommand_SetMyStatusMessageUI(WPARAM wParam, LPARAM lParam)
{
	int status = (int)wParam;
	char *proto_name = (char *)lParam;
	int proto_num = -1;
	Protocol *proto = NULL;

	if (status != 0 && (status < ID_STATUS_OFFLINE || status > ID_STATUS_OUTTOLUNCH))
		return -10;

	if (proto_name != NULL) {
		for (int i = 0; i < protocols->GetSize(); i++) {
			proto = protocols->Get(i);

			if (_stricmp(proto->name, proto_name) == 0) {
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if (protocols->CanSetStatusMsgPerProtocol() && !proto->CanSetStatusMsg())
			return -2;
	}
	else if (ServiceExists(MS_SIMPLESTATUSMSG_CHANGESTATUSMSG)) {
		if (proto == NULL && status == 0)
			CallService(MS_SIMPLESTATUSMSG_CHANGESTATUSMSG, protocols->GetGlobalStatus(), NULL);
		else if (status == 0)
			CallService(MS_SIMPLESTATUSMSG_CHANGESTATUSMSG, proto->status, (LPARAM)proto_name);
		else
			CallService(MS_SIMPLESTATUSMSG_CHANGESTATUSMSG, status, (LPARAM)proto_name);

		return 0;
	}

	if (proto == NULL || proto->status != ID_STATUS_OFFLINE) {
		if (!status_msg_dialog_open) {
			InterlockedExchange(&status_msg_dialog_open, 1);

			hwndSetStatusMsg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETSTATUSMESSAGE), NULL, DlgProcSetStatusMessage);

			SendMessage(hwndSetStatusMsg, WMU_SETDATA, status, proto_num);
		}

		SetForegroundWindow(hwndSetStatusMsg);
		SetFocus(hwndSetStatusMsg);
 		ShowWindow(hwndSetStatusMsg, SW_SHOW);

		return 0;
	}

	return -3;
}


INT_PTR PluginCommand_CycleThroughtProtocols(WPARAM wParam, LPARAM lParam)
{
	db_set_b(NULL, "MyDetails", "CicleThroughtProtocols", (BYTE)wParam);

	LoadOptions();

	return 0;
}
