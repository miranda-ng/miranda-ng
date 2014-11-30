/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "tlen.h"
#include "tlen_list.h"
#include "tlen_voice.h"
#include <commctrl.h>
#include "resource.h"

static INT_PTR CALLBACK TlenBasicOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK TlenVoiceOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK TlenAdvOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK TlenPopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct TabDefStruct {
	DLGPROC dlgProc;
	DWORD dlgId;
	TCHAR *tabName;
} TabDef;

static TabDef tabPages[] = {
						{TlenBasicOptDlgProc, IDD_OPTIONS_BASIC, LPGENT("General")},
						{TlenVoiceOptDlgProc, IDD_OPTIONS_VOICE, LPGENT("Voice Chats")},
						{TlenAdvOptDlgProc, IDD_OPTIONS_ADVANCED, LPGENT("Advanced")},
						{TlenPopupsDlgProc, IDD_OPTIONS_POPUPS, LPGENT("Notifications")}
						};

void TlenLoadOptions(TlenProtocol *proto)
{
	proto->tlenOptions.savePassword = db_get_b(NULL, proto->m_szModuleName, "SavePassword", TRUE);
	proto->tlenOptions.useEncryption = db_get_b(NULL, proto->m_szModuleName, "UseEncryption", TRUE);
	proto->tlenOptions.reconnect = db_get_b(NULL, proto->m_szModuleName, "Reconnect", TRUE);
	proto->tlenOptions.alertPolicy = db_get_w(NULL, proto->m_szModuleName, "AlertPolicy", TLEN_ALERTS_IGNORE_NIR);
	proto->tlenOptions.rosterSync = db_get_b(NULL, proto->m_szModuleName, "RosterSync", FALSE);
	proto->tlenOptions.offlineAsInvisible = db_get_b(NULL, proto->m_szModuleName, "OfflineAsInvisible", FALSE);
	proto->tlenOptions.leaveOfflineMessage = db_get_b(NULL, proto->m_szModuleName, "LeaveOfflineMessage", TRUE);
	proto->tlenOptions.offlineMessageOption = db_get_w(NULL, proto->m_szModuleName, "OfflineMessageOption", 0);
	proto->tlenOptions.ignoreAdvertisements = db_get_b(NULL, proto->m_szModuleName, "IgnoreAdvertisements", TRUE);
	proto->tlenOptions.groupChatPolicy = db_get_w(NULL, proto->m_szModuleName, "GroupChatPolicy", TLEN_MUC_ASK);
	proto->tlenOptions.voiceChatPolicy = db_get_w(NULL, proto->m_szModuleName, "VoiceChatPolicy", TLEN_MUC_ASK);
	proto->tlenOptions.imagePolicy = db_get_w(NULL, proto->m_szModuleName, "ImagePolicy",TLEN_IMAGES_IGNORE_NIR);
	proto->tlenOptions.enableAvatars = db_get_b(NULL, proto->m_szModuleName, "EnableAvatars", TRUE);
	proto->tlenOptions.enableVersion = db_get_b(NULL, proto->m_szModuleName, "EnableVersion", TRUE);
	proto->tlenOptions.useNudge = db_get_b(NULL, proto->m_szModuleName, "UseNudge", FALSE);
	proto->tlenOptions.logAlerts = db_get_b(NULL, proto->m_szModuleName, "LogAlerts", TRUE);
	proto->tlenOptions.sendKeepAlive = db_get_b(NULL, proto->m_szModuleName, "KeepAlive", TRUE);
	proto->tlenOptions.useNewP2P = TRUE;
}

static int changed = 0;

static void ApplyChanges(TlenProtocol *proto, int i) {
	changed &= ~i;
	if (changed == 0) {
		TlenLoadOptions(proto);
	}
}

static void MarkChanges(int i, HWND hWnd) {
	SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
	changed |= i;
}


int TlenProtocol::OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.dwInitParam = (LPARAM)this;
	for (int i = 0; i < SIZEOF(tabPages); i++) {
		odp.pszTemplate = MAKEINTRESOURCEA(tabPages[i].dlgId);
		odp.pfnDlgProc = tabPages[i].dlgProc;
		odp.ptszTab = tabPages[i].tabName;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

static LRESULT CALLBACK TlenValidateUsernameWndProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (strchr("\"&'/:<>@", wParam&0xff) != NULL)
			return 0;
		break;
	}
	return mir_callNextSubclass(hwndEdit, TlenValidateUsernameWndProc, msg, wParam, lParam);
}

INT_PTR CALLBACK TlenAccMgrUIDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char text[256];

	TlenProtocol *proto = (TlenProtocol *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			proto = (TlenProtocol *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)proto);
			TranslateDialogDefault(hwndDlg);
			if (!db_get_ts(NULL, proto->m_szModuleName, "LoginName", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EDIT_USERNAME, dbv.ptszVal);
				db_free(&dbv);
			}
			if (!db_get(NULL, proto->m_szModuleName, "Password", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_EDIT_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			CheckDlgButton(hwndDlg, IDC_SAVEPASSWORD, db_get_b(NULL, proto->m_szModuleName, "SavePassword", TRUE));

			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_EDIT_USERNAME), TlenValidateUsernameWndProc);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDIT_USERNAME:
		case IDC_EDIT_PASSWORD:
			if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_SAVEPASSWORD:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_REGISTERACCOUNT:
			CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) TLEN_REGISTER);
			break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
		case PSN_APPLY:
			{
				bool reconnectRequired = false;
				DBVARIANT dbv;

				GetDlgItemTextA(hwndDlg, IDC_EDIT_USERNAME, text, SIZEOF(text));
				if (db_get(NULL, proto->m_szModuleName, "LoginName", &dbv)) {
					reconnectRequired = true;
				}
				else {
					if(lstrcmpA(text, dbv.pszVal))
						reconnectRequired = true;
					db_free(&dbv);
				}
				db_set_s(NULL, proto->m_szModuleName, "LoginName", strlwr(text));

				if (IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD)) {
					GetDlgItemTextA(hwndDlg, IDC_EDIT_PASSWORD, text, SIZEOF(text));
					if (db_get(NULL, proto->m_szModuleName, "Password", &dbv)) {
						reconnectRequired = true;
					}
					else {
						if(lstrcmpA(text, dbv.pszVal))
							reconnectRequired = true;
						db_free(&dbv);
					}
					db_set_s(NULL, proto->m_szModuleName, "Password", text);
				}
				else
					db_unset(NULL, proto->m_szModuleName, "Password");

				db_set_b(NULL, proto->m_szModuleName, "SavePassword", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD));
				if (reconnectRequired && proto->isConnected)
					MessageBox(hwndDlg, TranslateT("These changes will take effect the next time you connect to the Tlen network."), TranslateT("Tlen Protocol Option"), MB_OK|MB_SETFOREGROUND);
				TlenLoadOptions(proto);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK TlenBasicOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char text[256];

	TlenProtocol *proto = (TlenProtocol *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			proto = (TlenProtocol *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)proto);
			TranslateDialogDefault(hwndDlg);
			if (!db_get_ts(NULL, proto->m_szModuleName, "LoginName", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EDIT_USERNAME, dbv.ptszVal);
				db_free(&dbv);
			}
			if (!db_get(NULL, proto->m_szModuleName, "Password", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_EDIT_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			CheckDlgButton(hwndDlg, IDC_SAVEPASSWORD, db_get_b(NULL, proto->m_szModuleName, "SavePassword", TRUE));

			CheckDlgButton(hwndDlg, IDC_RECONNECT, proto->tlenOptions.reconnect);
			CheckDlgButton(hwndDlg, IDC_ROSTER_SYNC, proto->tlenOptions.rosterSync);
			CheckDlgButton(hwndDlg, IDC_SHOW_OFFLINE, proto->tlenOptions.offlineAsInvisible);
			CheckDlgButton(hwndDlg, IDC_OFFLINE_MESSAGE, proto->tlenOptions.leaveOfflineMessage);
			CheckDlgButton(hwndDlg, IDC_IGNORE_ADVERTISEMENTS, proto->tlenOptions.ignoreAdvertisements);
			CheckDlgButton(hwndDlg, IDC_AVATARS, proto->tlenOptions.enableAvatars);
			CheckDlgButton(hwndDlg, IDC_VERSIONINFO, proto->tlenOptions.enableVersion);
			CheckDlgButton(hwndDlg, IDC_NUDGE_SUPPORT, proto->tlenOptions.useNudge);
			CheckDlgButton(hwndDlg, IDC_LOG_ALERTS, proto->tlenOptions.logAlerts);

			SendDlgItemMessage(hwndDlg, IDC_ALERT_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Accept all alerts"));
			SendDlgItemMessage(hwndDlg, IDC_ALERT_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore alerts from unauthorized contacts"));
			SendDlgItemMessage(hwndDlg, IDC_ALERT_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore all alerts"));
			SendDlgItemMessage(hwndDlg, IDC_ALERT_POLICY, CB_SETCURSEL, proto->tlenOptions.alertPolicy, 0);

			SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Always ask me"));
			SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Accept invitations from authorized contacts"));
			SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Accept all invitations"));
			SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore invitations from unauthorized contacts"));
			SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore all invitation"));
			SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_SETCURSEL, proto->tlenOptions.groupChatPolicy, 0);

			SendDlgItemMessage(hwndDlg, IDC_IMAGE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Accept all images"));
			SendDlgItemMessage(hwndDlg, IDC_IMAGE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore images from unauthorized contacts"));
			SendDlgItemMessage(hwndDlg, IDC_IMAGE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore all images"));
			SendDlgItemMessage(hwndDlg, IDC_IMAGE_POLICY, CB_SETCURSEL, proto->tlenOptions.imagePolicy, 0);

			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Last message>"));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(ID_STATUS_ONLINE, 0));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(ID_STATUS_AWAY, 0));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(ID_STATUS_NA, 0));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(ID_STATUS_DND, 0));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(ID_STATUS_FREECHAT, 0));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(ID_STATUS_INVISIBLE, 0));
			SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_SETCURSEL, proto->tlenOptions.offlineMessageOption, 0);

			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_EDIT_USERNAME), TlenValidateUsernameWndProc);
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDIT_USERNAME:
		case IDC_EDIT_PASSWORD:
			if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
			// Fall through
		case IDC_SAVEPASSWORD:
		case IDC_RECONNECT:
		case IDC_ROSTER_SYNC:
		case IDC_IGNORE_ADVERTISEMENTS:
		case IDC_SHOW_OFFLINE:
		case IDC_OFFLINE_MESSAGE:
			MarkChanges(1, hwndDlg);
			break;
		case IDC_LOG_ALERTS:
			CheckDlgButton(hwndDlg, IDC_NUDGE_SUPPORT, BST_UNCHECKED);
			MarkChanges(1, hwndDlg);
			break;
		case IDC_NUDGE_SUPPORT:
			CheckDlgButton(hwndDlg, IDC_LOG_ALERTS, BST_UNCHECKED);
			MarkChanges(1, hwndDlg);
			break;
		case IDC_REGISTERACCOUNT:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM) TLEN_REGISTER);
			break;
		case IDC_OFFLINE_MESSAGE_OPTION:
		case IDC_ALERT_POLICY:
		case IDC_MUC_POLICY:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				MarkChanges(1, hwndDlg);
			break;
		default:
			MarkChanges(1, hwndDlg);
			break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
		case PSN_APPLY:
			{
				bool reconnectRequired = false;
				DBVARIANT dbv;

				GetDlgItemTextA(hwndDlg, IDC_EDIT_USERNAME, text, SIZEOF(text));
				if (db_get(NULL, proto->m_szModuleName, "LoginName", &dbv)) {
					reconnectRequired = true;
				}
				else {
					if(lstrcmpA(text, dbv.pszVal))
						reconnectRequired = true;
					db_free(&dbv);
				}
				db_set_s(NULL, proto->m_szModuleName, "LoginName", strlwr(text));

				if (IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD)) {
					GetDlgItemTextA(hwndDlg, IDC_EDIT_PASSWORD, text, SIZEOF(text));
					if (db_get(NULL, proto->m_szModuleName, "Password", &dbv)) {
						reconnectRequired = true;
					}
					else {
						if(lstrcmpA(text, dbv.pszVal))
							reconnectRequired = true;
						db_free(&dbv);
					}
					db_set_s(NULL, proto->m_szModuleName, "Password", text);
				}
				else
					db_unset(NULL, proto->m_szModuleName, "Password");

				db_set_b(NULL, proto->m_szModuleName, "SavePassword", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD));
				db_set_b(NULL, proto->m_szModuleName, "Reconnect", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_RECONNECT));
				db_set_b(NULL, proto->m_szModuleName, "RosterSync", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ROSTER_SYNC));
				db_set_b(NULL, proto->m_szModuleName, "OfflineAsInvisible", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOW_OFFLINE));
				db_set_b(NULL, proto->m_szModuleName, "IgnoreAdvertisements", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IGNORE_ADVERTISEMENTS));
				db_set_b(NULL, proto->m_szModuleName, "LeaveOfflineMessage", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_OFFLINE_MESSAGE));
				db_set_w(NULL, proto->m_szModuleName, "OfflineMessageOption", (WORD) SendDlgItemMessage(hwndDlg, IDC_OFFLINE_MESSAGE_OPTION, CB_GETCURSEL, 0, 0));
				db_set_w(NULL, proto->m_szModuleName, "AlertPolicy", (WORD) SendDlgItemMessage(hwndDlg, IDC_ALERT_POLICY, CB_GETCURSEL, 0, 0));
				db_set_w(NULL, proto->m_szModuleName, "GroupChatPolicy", (WORD) SendDlgItemMessage(hwndDlg, IDC_MUC_POLICY, CB_GETCURSEL, 0, 0));
				db_set_w(NULL, proto->m_szModuleName, "ImagePolicy", (WORD) SendDlgItemMessage(hwndDlg, IDC_IMAGE_POLICY, CB_GETCURSEL, 0, 0));
				db_set_b(NULL, proto->m_szModuleName, "EnableAvatars", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AVATARS));
				db_set_b(NULL, proto->m_szModuleName, "EnableVersion", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_VERSIONINFO));
				db_set_b(NULL, proto->m_szModuleName, "UseNudge", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_NUDGE_SUPPORT));
				db_set_b(NULL, proto->m_szModuleName, "LogAlerts", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_LOG_ALERTS));
				if (reconnectRequired && proto->isConnected)
					MessageBox(hwndDlg, TranslateT("These changes will take effect the next time you connect to the Tlen network."), TranslateT("Tlen Protocol Option"), MB_OK|MB_SETFOREGROUND);
				ApplyChanges(proto, 1);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK TlenVoiceOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TlenProtocol *proto = (TlenProtocol *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			proto = (TlenProtocol *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)proto);
			SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Always ask me"));
			SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Accept invitations from authorized contacts"));
			SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Accept all invitations"));
			SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore invitations from unauthorized contacts"));
			SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore all invitation"));
			SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_SETCURSEL, proto->tlenOptions.voiceChatPolicy, 0);
			TlenVoiceBuildInDeviceList(proto, GetDlgItem(hwndDlg, IDC_VOICE_DEVICE_IN));
			TlenVoiceBuildOutDeviceList(proto, GetDlgItem(hwndDlg, IDC_VOICE_DEVICE_OUT));
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_VOICE_POLICY:
		case IDC_VOICE_DEVICE_IN:
		case IDC_VOICE_DEVICE_OUT:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				MarkChanges(2, hwndDlg);
			break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
		case PSN_APPLY:
			{
				db_set_w(NULL, proto->m_szModuleName, "VoiceChatPolicy", (WORD) SendDlgItemMessage(hwndDlg, IDC_VOICE_POLICY, CB_GETCURSEL, 0, 0));
				db_set_w(NULL, proto->m_szModuleName, "VoiceDeviceIn", (WORD) SendDlgItemMessage(hwndDlg, IDC_VOICE_DEVICE_IN, CB_GETCURSEL, 0, 0));
				db_set_w(NULL, proto->m_szModuleName, "VoiceDeviceOut", (WORD) SendDlgItemMessage(hwndDlg, IDC_VOICE_DEVICE_OUT, CB_GETCURSEL, 0, 0));
				ApplyChanges(proto, 2);
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK TlenAdvOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char text[256];
	BOOL bChecked;
	TlenProtocol *proto = (TlenProtocol *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			proto = (TlenProtocol *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)proto);
			TranslateDialogDefault(hwndDlg);
			if (!db_get_ts(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EDIT_LOGIN_SERVER, dbv.ptszVal);
				db_free(&dbv);
			}
			else SetDlgItemText(hwndDlg, IDC_EDIT_LOGIN_SERVER, _T("tlen.pl"));

			EnableWindow(GetDlgItem(hwndDlg, IDC_HOST), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HOSTPORT), TRUE);

			if (!db_get_ts(NULL, proto->m_szModuleName, "ManualHost", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOST, dbv.ptszVal);
				db_free(&dbv);
			}
			else SetDlgItemText(hwndDlg, IDC_HOST, _T("s1.tlen.pl"));

			SetDlgItemInt(hwndDlg, IDC_HOSTPORT, db_get_w(NULL, proto->m_szModuleName, "ManualPort", TLEN_DEFAULT_PORT), FALSE);

			CheckDlgButton(hwndDlg, IDC_KEEPALIVE, db_get_b(NULL, proto->m_szModuleName, "KeepAlive", TRUE));

			CheckDlgButton(hwndDlg, IDC_USE_SSL, db_get_b(NULL, proto->m_szModuleName, "UseEncryption", TRUE));

			CheckDlgButton(hwndDlg, IDC_VISIBILITY_SUPPORT, db_get_b(NULL, proto->m_szModuleName, "VisibilitySupport", FALSE));
			// File transfer options
			bChecked = FALSE;
			if (db_get_b(NULL, proto->m_szModuleName, "UseFileProxy", FALSE) == TRUE) {
				bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_FILE_USE_PROXY, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_TYPE_LABEL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_TYPE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_HOST_LABEL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_HOST), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PORT_LABEL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PORT), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_USE_AUTH), bChecked);
			if (db_get_b(NULL, proto->m_szModuleName, "FileProxyAuth", FALSE) == TRUE)
				CheckDlgButton(hwndDlg, IDC_FILE_PROXY_USE_AUTH, TRUE);
			else
				bChecked = FALSE;

			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_USER_LABEL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_USER), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PASSWORD_LABEL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PASSWORD), bChecked);

			SendDlgItemMessage(hwndDlg, IDC_FILE_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Forwarding"));
			SendDlgItemMessage(hwndDlg, IDC_FILE_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("SOCKS4"));
			SendDlgItemMessage(hwndDlg, IDC_FILE_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("SOCKS5"));
			SendDlgItemMessage(hwndDlg, IDC_FILE_PROXY_TYPE, CB_SETCURSEL, db_get_w(NULL, proto->m_szModuleName, "FileProxyType", 0), 0);
			if (!db_get_ts(NULL, proto->m_szModuleName, "FileProxyHost", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FILE_PROXY_HOST, dbv.ptszVal);
				db_free(&dbv);
			}
			SetDlgItemInt(hwndDlg, IDC_FILE_PROXY_PORT, db_get_w(NULL, proto->m_szModuleName, "FileProxyPort", 0), FALSE);
			if (!db_get_ts(NULL, proto->m_szModuleName, "FileProxyUsername", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FILE_PROXY_USER, dbv.ptszVal);
				db_free(&dbv);
			}
			if (!db_get_s(NULL, proto->m_szModuleName, "FileProxyPassword", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_FILE_PROXY_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_FILE_PROXY_TYPE:
				if (HIWORD(wParam) == CBN_SELCHANGE)
					MarkChanges(4, hwndDlg);
				break;
			case IDC_EDIT_LOGIN_SERVER:
			case IDC_HOST:
			case IDC_HOSTPORT:
			case IDC_FILE_PROXY_HOST:
			case IDC_FILE_PROXY_PORT:
			case IDC_FILE_PROXY_USER:
			case IDC_FILE_PROXY_PASSWORD:
				if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
					MarkChanges(4, hwndDlg);
				break;
			case IDC_FILE_USE_PROXY:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_FILE_USE_PROXY);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_TYPE_LABEL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_TYPE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_HOST_LABEL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_HOST), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PORT_LABEL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PORT), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_USE_AUTH), bChecked);
			case IDC_FILE_PROXY_USE_AUTH:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_FILE_PROXY_USE_AUTH) & IsDlgButtonChecked(hwndDlg, IDC_FILE_USE_PROXY);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_USER_LABEL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_USER), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PASSWORD_LABEL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_PROXY_PASSWORD), bChecked);
				MarkChanges(4, hwndDlg);
				break;
			case IDC_KEEPALIVE:
			case IDC_VISIBILITY_SUPPORT:
			case IDC_USE_SSL:
				MarkChanges(4, hwndDlg);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
		case PSN_APPLY:
			WORD port;
			BOOL useEncryption;
			bool reconnectRequired = false;
			DBVARIANT dbv;

			GetDlgItemTextA(hwndDlg, IDC_EDIT_LOGIN_SERVER, text, SIZEOF(text));
			if (db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(lstrcmpA(text, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}
			db_set_s(NULL, proto->m_szModuleName, "LoginServer", strlwr(text));

			GetDlgItemTextA(hwndDlg, IDC_HOST, text, SIZEOF(text));
			if (db_get(NULL, proto->m_szModuleName, "ManualHost", &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(lstrcmpA(text, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}
			db_set_s(NULL, proto->m_szModuleName, "ManualHost", text);

			port = (WORD) GetDlgItemInt(hwndDlg, IDC_HOSTPORT, NULL, FALSE);
			if (db_get_w(NULL, proto->m_szModuleName, "ManualPort", TLEN_DEFAULT_PORT) != port)
				reconnectRequired = true;
			db_set_w(NULL, proto->m_szModuleName, "ManualPort", port);

			proto->tlenOptions.sendKeepAlive = IsDlgButtonChecked(hwndDlg, IDC_KEEPALIVE);
			db_set_b(NULL, proto->m_szModuleName, "KeepAlive", (BYTE) proto->tlenOptions.sendKeepAlive);

			useEncryption = IsDlgButtonChecked(hwndDlg, IDC_USE_SSL);
			if (db_get_b(NULL, proto->m_szModuleName, "UseEncryption", TRUE) != useEncryption)
				reconnectRequired = true;
			db_set_b(NULL, proto->m_szModuleName, "UseEncryption", (BYTE) useEncryption);

			db_set_b(NULL, proto->m_szModuleName, "VisibilitySupport", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_VISIBILITY_SUPPORT));

			// File transfer options
			db_set_b(NULL, proto->m_szModuleName, "UseFileProxy", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FILE_USE_PROXY));
			db_set_w(NULL, proto->m_szModuleName, "FileProxyType", (WORD) SendDlgItemMessage(hwndDlg, IDC_FILE_PROXY_TYPE, CB_GETCURSEL, 0, 0));

			GetDlgItemTextA(hwndDlg, IDC_FILE_PROXY_HOST, text, SIZEOF(text));
			db_set_s(NULL, proto->m_szModuleName, "FileProxyHost", text);

			db_set_w(NULL, proto->m_szModuleName, "FileProxyPort", (WORD) GetDlgItemInt(hwndDlg, IDC_FILE_PROXY_PORT, NULL, FALSE));
			db_set_b(NULL, proto->m_szModuleName, "FileProxyAuth", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FILE_PROXY_USE_AUTH));

			GetDlgItemTextA(hwndDlg, IDC_FILE_PROXY_USER, text, SIZEOF(text));
			db_set_s(NULL, proto->m_szModuleName, "FileProxyUsername", text);

			GetDlgItemTextA(hwndDlg, IDC_FILE_PROXY_PASSWORD, text, SIZEOF(text));
			db_set_s(NULL, proto->m_szModuleName, "FileProxyPassword", text);

			if (reconnectRequired && proto->isConnected)
				MessageBox(hwndDlg, TranslateT("These changes will take effect the next time you connect to the Tlen network."), TranslateT("Tlen Protocol Option"), MB_OK|MB_SETFOREGROUND);
			ApplyChanges(proto, 4);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

#define POPUP_DEFAULT_COLORBKG 0xDCBDA5
#define POPUP_DEFAULT_COLORTXT 0x000000

static void MailPopupPreview(DWORD colorBack, DWORD colorText, TCHAR *title, TCHAR *emailInfo, int delay)
{
	POPUPDATAT ppd = { 0 };
	HICON hIcon = GetIcolibIcon(IDI_MAIL);
	ppd.lchIcon = CopyIcon(hIcon);
	ReleaseIcolibIcon(hIcon);
	_tcscpy(ppd.lptzContactName, title);
	_tcscpy(ppd.lptzText, emailInfo);
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.iSeconds = delay;
	if ( ServiceExists(MS_POPUP_ADDPOPUPT))
		PUAddPopupT(&ppd);
}

static INT_PTR CALLBACK TlenPopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TlenProtocol *proto = (TlenProtocol *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG:
			{
				BYTE delayMode;
				proto = (TlenProtocol *)lParam;
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)proto);
				TranslateDialogDefault(hwndDlg);
				CheckDlgButton(hwndDlg, IDC_ENABLEPOPUP, db_get_b(NULL, proto->m_szModuleName, "MailPopupEnabled", TRUE));
				SendDlgItemMessage(hwndDlg, IDC_COLORBKG, CPM_SETCOLOUR, 0, db_get_dw(NULL, proto->m_szModuleName, "MailPopupBack", POPUP_DEFAULT_COLORBKG));
				SendDlgItemMessage(hwndDlg, IDC_COLORTXT, CPM_SETCOLOUR, 0, db_get_dw(NULL, proto->m_szModuleName, "MailPopupText", POPUP_DEFAULT_COLORTXT));
				SetDlgItemInt(hwndDlg, IDC_DELAY, db_get_dw(NULL, proto->m_szModuleName, "MailPopupDelay", 4), FALSE);
				delayMode = db_get_b(NULL, proto->m_szModuleName, "MailPopupDelayMode", 0);
				if (delayMode == 1) {
					CheckDlgButton(hwndDlg, IDC_DELAY_CUSTOM, TRUE);
				} else if (delayMode == 2) {
					CheckDlgButton(hwndDlg, IDC_DELAY_PERMANENT, TRUE);
				} else {
					CheckDlgButton(hwndDlg, IDC_DELAY_POPUP, TRUE);
				}
				return TRUE;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_COLORTXT:
			case IDC_COLORBKG:
			case IDC_ENABLEPOPUP:
			case IDC_DELAY:
			case IDC_DELAY_POPUP:
			case IDC_DELAY_CUSTOM:
			case IDC_DELAY_PERMANENT:
				MarkChanges(8, hwndDlg);
				break;
			case IDC_PREVIEW:
				{
					int delay;
					TCHAR title[256];
					if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_POPUP)) {
						delay=0;
					} else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERMANENT)) {
						delay=-1;
					} else {
						delay=GetDlgItemInt(hwndDlg, IDC_DELAY, NULL, FALSE);
					}
					mir_sntprintf(title, SIZEOF(title), TranslateT("%S mail"), proto->m_szModuleName);
					MailPopupPreview((DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORBKG,CPM_GETCOLOUR,0,0),
									(DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORTXT,CPM_GETCOLOUR,0,0),
									title,
									_T("From: test@test.test\nSubject: test"),
									delay);
				}

			}
			break;


		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->code) {
				case PSN_APPLY:
				{
					BYTE delayMode;
					db_set_b(NULL, proto->m_szModuleName, "MailPopupEnabled", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEPOPUP));
					db_set_dw(NULL, proto->m_szModuleName, "MailPopupBack", (DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORBKG,CPM_GETCOLOUR,0,0));
					db_set_dw(NULL, proto->m_szModuleName, "MailPopupText", (DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORTXT,CPM_GETCOLOUR,0,0));
					db_set_dw(NULL, proto->m_szModuleName, "MailPopupDelay", (DWORD) GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE));
					delayMode=0;
					if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_CUSTOM)) {
						delayMode=1;
					} else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERMANENT)) {
						delayMode=2;

					}
					db_set_b(NULL, proto->m_szModuleName, "MailPopupDelayMode", delayMode);
					ApplyChanges(proto, 8);
					return TRUE;
				}
			}
			break;

	}
	return FALSE;
}

