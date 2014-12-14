/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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

//////////////////////////////////////////////////////////////////////////////
// Account manager dialog

INT_PTR CALLBACK VKAccountProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CVkProto *ppro = (CVkProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CVkProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, 1));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon));
		{
			ptrT tszLogin(ppro->getTStringA("Login"));
			if (tszLogin != NULL)
				SetDlgItemText(hwndDlg, IDC_LOGIN, tszLogin);

			ptrT tszPassw(ppro->GetUserStoredPassword());
			if (tszPassw != NULL)
				SetDlgItemText(hwndDlg, IDC_PASSWORD, tszPassw);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_URL:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://www.vk.com");
			break;

		case IDC_LOGIN:
		case IDC_PASSWORD:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			TCHAR str[128];
			GetDlgItemText(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
			ppro->setTString("Login", str);
			
			GetDlgItemText(hwndDlg, IDC_PASSWORD, str, SIZEOF(str));
			ptrA szRawPasswd(mir_utf8encodeT(str));
			if (szRawPasswd != NULL)
				ppro->setString("Password", szRawPasswd);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;
	}

	return FALSE;
}

INT_PTR CVkProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, VKAccountProc, (LPARAM)this);
}

//////////////////////////////////////////////////////////////////////////////
// Options

INT_PTR CALLBACK CVkProto::OptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CVkProto *ppro = (CVkProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CVkProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, 1));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon));
		{
			ptrT tszLogin(ppro->getTStringA("Login"));
			if (tszLogin != NULL)
				SetDlgItemText(hwndDlg, IDC_LOGIN, tszLogin);

			ptrT tszPassw(ppro->GetUserStoredPassword());
			if (tszPassw != NULL)
				SetDlgItemText(hwndDlg, IDC_PASSWORD, tszPassw);

			SetDlgItemText(hwndDlg, IDC_GROUPNAME, ppro->getGroup());
		}

		CheckDlgButton(hwndDlg, IDC_DELIVERY, ppro->m_bServerDelivery ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USE_LOCAL_TIME, ppro->m_bUseLocalTime ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOCLEAN, ppro->getByte("AutoClean", 0) ? BST_CHECKED : BST_UNCHECKED);
		
		CheckDlgButton(hwndDlg, IDC_ONREAD, (ppro->m_iMarkMessageReadOn == markOnRead) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONRECEIVE, (ppro->m_iMarkMessageReadOn == markOnReceive) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONREPLY, (ppro->m_iMarkMessageReadOn == markOnReply) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONTYPING, (ppro->m_iMarkMessageReadOn == markOnTyping) ? BST_CHECKED : BST_UNCHECKED);
		
		CheckDlgButton(hwndDlg, IDC_SYNC_OFF, (ppro->m_iSyncHistoryMetod == syncOff) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SYNC_AUTO, (ppro->m_iSyncHistoryMetod == syncAuto) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SYNC_LAST1DAY, (ppro->m_iSyncHistoryMetod == sync1Days) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SYNC_LAST3DAY, (ppro->m_iSyncHistoryMetod == sync3Days) ? BST_CHECKED : BST_UNCHECKED);
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_URL:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://www.vk.com");
			break;

		case IDC_LOGIN:
		case IDC_PASSWORD:
		case IDC_GROUPNAME:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_DELIVERY: 
		case IDC_USE_LOCAL_TIME:
		case IDC_AUTOCLEAN:
		
		case IDC_ONREAD:
		case IDC_ONRECEIVE:
		case IDC_ONREPLY:
		case IDC_ONTYPING:
		
		case IDC_SYNC_OFF:
		case IDC_SYNC_AUTO:
		case IDC_SYNC_LAST1DAY:
		case IDC_SYNC_LAST3DAY:
			if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			TCHAR str[128];
			GetDlgItemText(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
			ppro->setTString("Login", str);

			GetDlgItemText(hwndDlg, IDC_GROUPNAME, str, SIZEOF(str));
			if (_tcscmp(ppro->getGroup(), str)) {
				ppro->setGroup(str);
				ppro->setTString("ProtoGroup", str);
			}
			
			GetDlgItemText(hwndDlg, IDC_PASSWORD, str, SIZEOF(str));
			ptrA szRawPasswd(mir_utf8encodeT(str));
			if (szRawPasswd != NULL)
				ppro->setString("Password", szRawPasswd);

			ppro->m_bServerDelivery = IsDlgButtonChecked(hwndDlg, IDC_DELIVERY) == BST_CHECKED;
			ppro->setByte("ServerDelivery", ppro->m_bServerDelivery);
						
			ppro->setByte("AutoClean", IsDlgButtonChecked(hwndDlg, IDC_AUTOCLEAN) == BST_CHECKED);

			ppro->m_bUseLocalTime = IsDlgButtonChecked(hwndDlg, IDC_USE_LOCAL_TIME) == BST_CHECKED;
			ppro->setByte("UseLocalTime", ppro->m_bUseLocalTime);

			if (IsDlgButtonChecked(hwndDlg, IDC_ONREAD) == BST_CHECKED)
				ppro->m_iMarkMessageReadOn = markOnRead;
			if (IsDlgButtonChecked(hwndDlg, IDC_ONRECEIVE) == BST_CHECKED)
				ppro->m_iMarkMessageReadOn = markOnReceive;
			if (IsDlgButtonChecked(hwndDlg, IDC_ONREPLY) == BST_CHECKED)
				ppro->m_iMarkMessageReadOn = markOnReply;
			if (IsDlgButtonChecked(hwndDlg, IDC_ONTYPING) == BST_CHECKED)
				ppro->m_iMarkMessageReadOn = markOnTyping;
			ppro->setByte("MarkMessageReadOn", ppro->m_iMarkMessageReadOn);

			if (IsDlgButtonChecked(hwndDlg, IDC_SYNC_OFF) == BST_CHECKED)
				ppro->m_iSyncHistoryMetod = syncOff;
			if (IsDlgButtonChecked(hwndDlg, IDC_SYNC_AUTO) == BST_CHECKED)
				ppro->m_iSyncHistoryMetod = syncAuto;
			if (IsDlgButtonChecked(hwndDlg, IDC_SYNC_LAST1DAY) == BST_CHECKED)
				ppro->m_iSyncHistoryMetod = sync1Days;
			if (IsDlgButtonChecked(hwndDlg, IDC_SYNC_LAST3DAY) == BST_CHECKED)
				ppro->m_iSyncHistoryMetod = sync3Days;

			ppro->setByte("SyncHistoryMetod", ppro->m_iSyncHistoryMetod);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CVkProto::OptionsAdvProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CVkProto *ppro = (CVkProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CVkProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, 1));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon));

		CheckDlgButton(hwndDlg, IDC_HIDECHATS, ppro->m_bHideChats ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MESASUREAD, ppro->m_bMesAsUnread ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ADD_IMG_BBC, ppro->m_bAddImgBbc ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STICKERS_AS_SMYLES, ppro->m_bStikersAsSmyles ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FORCE_ONLINE_ON_ACT, ppro->m_bUserForceOnlineOnActivity ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_REPORT_ABUSE, ppro->m_bReportAbuse ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CLEAR_SERVER_HISTORY, ppro->m_bClearServerHistory ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_REMOVE_FROM_FRENDLIST, ppro->m_bRemoveFromFrendlist ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_REMOVE_FROM_CLIST, ppro->m_bRemoveFromClist ? BST_CHECKED : BST_UNCHECKED);
		
		CheckDlgButton(hwndDlg, IDC_SEND_MUSIC_NONE, (ppro->m_iMusicSendMetod == sendNone) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SEND_MUSIC_BROADCAST, (ppro->m_iMusicSendMetod == sendBroadcastOnly) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SEND_MUSIC_STATUS, (ppro->m_iMusicSendMetod == sendStatusOnly) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SEND_MUSIC_BROADCAST_AND_STATUS, (ppro->m_iMusicSendMetod == sendBroadcastAndStatus) ? BST_CHECKED : BST_UNCHECKED);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_HIDECHATS:
		case IDC_MESASUREAD:
		case IDC_ADD_IMG_BBC:
		case IDC_FORCE_ONLINE_ON_ACT:
		case IDC_STICKERS_AS_SMYLES:
		case IDC_REPORT_ABUSE:
		case IDC_CLEAR_SERVER_HISTORY:
		case IDC_REMOVE_FROM_FRENDLIST:
		case IDC_REMOVE_FROM_CLIST:
		case IDC_SEND_MUSIC_NONE:
		case IDC_SEND_MUSIC_BROADCAST:
		case IDC_SEND_MUSIC_STATUS:
		case IDC_SEND_MUSIC_BROADCAST_AND_STATUS:
			if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			ppro->m_bHideChats = IsDlgButtonChecked(hwndDlg, IDC_HIDECHATS) == BST_CHECKED;
			ppro->setByte("HideChats", ppro->m_bHideChats);
			
			ppro->m_bMesAsUnread = IsDlgButtonChecked(hwndDlg, IDC_MESASUREAD) == BST_CHECKED;
			ppro->setByte("MesAsUnread", ppro->m_bMesAsUnread);

			ppro->m_bAddImgBbc = IsDlgButtonChecked(hwndDlg, IDC_ADD_IMG_BBC) == BST_CHECKED;
			ppro->setByte("AddImgBbc", ppro->m_bAddImgBbc);

			ppro->m_bStikersAsSmyles = IsDlgButtonChecked(hwndDlg, IDC_STICKERS_AS_SMYLES) == BST_CHECKED;
			ppro->setByte("StikersAsSmyles", ppro->m_bStikersAsSmyles);

			ppro->m_bUserForceOnlineOnActivity = IsDlgButtonChecked(hwndDlg, IDC_FORCE_ONLINE_ON_ACT) == BST_CHECKED;
			ppro->setByte("UserForceOnlineOnActivity", ppro->m_bUserForceOnlineOnActivity);

			ppro->m_bReportAbuse = IsDlgButtonChecked(hwndDlg, IDC_REPORT_ABUSE) == BST_CHECKED;
			ppro->setByte("ReportAbuseOnBanUser", ppro->m_bReportAbuse);

			ppro->m_bClearServerHistory = IsDlgButtonChecked(hwndDlg, IDC_CLEAR_SERVER_HISTORY) == BST_CHECKED;
			ppro->setByte("ClearServerHistoryOnBanUser", ppro->m_bClearServerHistory);

			ppro->m_bRemoveFromFrendlist = IsDlgButtonChecked(hwndDlg, IDC_REMOVE_FROM_FRENDLIST) == BST_CHECKED;
			ppro->setByte("RemoveFromFrendlistOnBanUser", ppro->m_bRemoveFromFrendlist);

			ppro->m_bRemoveFromClist = IsDlgButtonChecked(hwndDlg, IDC_REMOVE_FROM_CLIST) == BST_CHECKED;
			ppro->setByte("RemoveFromClistOnBanUser", ppro->m_bRemoveFromClist);

			if (IsDlgButtonChecked(hwndDlg, IDC_SEND_MUSIC_NONE) == BST_CHECKED)
				ppro->m_iMusicSendMetod = sendNone;
			if (IsDlgButtonChecked(hwndDlg, IDC_SEND_MUSIC_BROADCAST) == BST_CHECKED)
				ppro->m_iMusicSendMetod = sendBroadcastOnly;
			if (IsDlgButtonChecked(hwndDlg, IDC_SEND_MUSIC_STATUS) == BST_CHECKED)
				ppro->m_iMusicSendMetod = sendStatusOnly;
			if (IsDlgButtonChecked(hwndDlg, IDC_SEND_MUSIC_BROADCAST_AND_STATUS) == BST_CHECKED)
				ppro->m_iMusicSendMetod = sendBroadcastAndStatus;
			ppro->setByte("MusicSendMetod", ppro->m_iMusicSendMetod);
			CMStringA szListeningTo(ppro->m_szModuleName);
			szListeningTo += "Enabled";
			db_set_b(NULL, "ListeningTo", szListeningTo.GetBuffer(), ppro->m_iMusicSendMetod == 0 ? 0 : 1);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;
	}

	return FALSE;
}

int CVkProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance   = hInst;
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup   = LPGENT("Network");
	
	odp.ptszTab     = LPGENT("Account");
	odp.position = 1;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc  = &CVkProto::OptionsProc;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.position = 2;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ADV);
	odp.pfnDlgProc = &CVkProto::OptionsAdvProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
