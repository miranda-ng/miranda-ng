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
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, true));
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
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://www.vk.com");
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
			ptrA szRawPasswd( mir_utf8encodeT(str));
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

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, true));
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
		CheckDlgButton(hwndDlg, IDC_DELIVERY, ppro->m_bServerDelivery);
		CheckDlgButton(hwndDlg, IDC_HIDECHATS, ppro->m_bHideChats);
		CheckDlgButton(hwndDlg, IDC_AUTOCLEAN, ppro->getByte("AutoClean", 0));
		CheckDlgButton(hwndDlg, IDC_MESASUREAD, ppro->m_bMesAsUnread);
		CheckDlgButton(hwndDlg, IDC_MARKREADONREPLY, ppro->m_bMarkReadOnReply);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MARKREADONTYPING), ppro->m_bMarkReadOnReply);
		CheckDlgButton(hwndDlg, IDC_MARKREADONTYPING, ppro->m_bMarkReadOnTyping);
		CheckDlgButton(hwndDlg, IDC_SYNCHISTOTYONONLINE, ppro->m_bAutoSyncHistory);
		CheckDlgButton(hwndDlg, IDC_USE_LOCAL_TIME, ppro->m_bUseLocalTime);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_URL:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://www.vk.com");
			break;

		case IDC_LOGIN:
		case IDC_PASSWORD:
		case IDC_GROUPNAME:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_DELIVERY:
		case IDC_HIDECHATS:
		case IDC_AUTOCLEAN:
		case IDC_MESASUREAD: 
		case IDC_MARKREADONTYPING:
		case IDC_SYNCHISTOTYONONLINE:
		case IDC_USE_LOCAL_TIME:
			if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_MARKREADONREPLY:
			if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == GetFocus()){
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_MARKREADONTYPING), (IsDlgButtonChecked(hwndDlg, IDC_MARKREADONREPLY) == BST_CHECKED));
			}
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

			ppro->m_bHideChats = IsDlgButtonChecked(hwndDlg, IDC_HIDECHATS) == BST_CHECKED;
			ppro->setByte("HideChats", ppro->m_bHideChats);

			ppro->setByte("AutoClean", IsDlgButtonChecked(hwndDlg, IDC_AUTOCLEAN) == BST_CHECKED);
			
			ppro->m_bMesAsUnread = IsDlgButtonChecked(hwndDlg, IDC_MESASUREAD) == BST_CHECKED;
			ppro->setByte("MesAsUnread", ppro->m_bMesAsUnread);

			ppro->m_bMarkReadOnReply = IsDlgButtonChecked(hwndDlg, IDC_MARKREADONREPLY) == BST_CHECKED;
			ppro->setByte("MarkReadOnReply", ppro->m_bMarkReadOnReply);

			ppro->m_bMarkReadOnTyping = (IsDlgButtonChecked(hwndDlg, IDC_MARKREADONREPLY) == BST_CHECKED) && (IsDlgButtonChecked(hwndDlg, IDC_MARKREADONTYPING) == BST_CHECKED);
			ppro->setByte("MarkReadOnTyping", ppro->m_bMarkReadOnTyping);

			ppro->m_bAutoSyncHistory = IsDlgButtonChecked(hwndDlg, IDC_SYNCHISTOTYONONLINE) == BST_CHECKED;
			ppro->setByte("AutoSyncHistory", ppro->m_bAutoSyncHistory);

			ppro->m_bUseLocalTime = IsDlgButtonChecked(hwndDlg, IDC_USE_LOCAL_TIME) == BST_CHECKED;
			ppro->setByte("UseLocalTime", ppro->m_bUseLocalTime);
						
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
	odp.position    = 1;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTab     = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc  = &CVkProto::OptionsProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
