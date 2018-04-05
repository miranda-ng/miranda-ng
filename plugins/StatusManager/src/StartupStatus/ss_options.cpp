/*
	StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "..\stdafx.h"

char* OptName(int i, const char* setting)
{
	static char buf[100];
	mir_snprintf(buf, "%d_%s", i, setting);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Command line processing

static char* GetStatusDesc(int status)
{
	switch (status) {
	case ID_STATUS_AWAY:       return "away";
	case ID_STATUS_NA:         return "na";
	case ID_STATUS_DND:        return "dnd";
	case ID_STATUS_OCCUPIED:   return "occupied";
	case ID_STATUS_FREECHAT:   return "freechat";
	case ID_STATUS_ONLINE:     return "online";
	case ID_STATUS_OFFLINE:    return "offline";
	case ID_STATUS_INVISIBLE:  return "invisible";
	case ID_STATUS_ONTHEPHONE: return "onthephone";
	case ID_STATUS_OUTTOLUNCH: return "outtolunch";
	case ID_STATUS_LAST:       return "last";
	}
	return "offline";
}

static char* GetCMDLArguments(TProtoSettings& protoSettings)
{
	if (protoSettings.getCount() == 0)
		return nullptr;

	char *cmdl, *pnt;
	pnt = cmdl = (char*)mir_alloc(mir_strlen(protoSettings[0].m_szName) + mir_strlen(GetStatusDesc(protoSettings[0].m_status)) + 4);

	for (int i = 0; i < protoSettings.getCount(); i++) {
		*pnt++ = '/';
		mir_strcpy(pnt, protoSettings[i].m_szName);
		pnt += mir_strlen(protoSettings[i].m_szName);
		*pnt++ = '=';
		mir_strcpy(pnt, GetStatusDesc(protoSettings[i].m_status));
		pnt += mir_strlen(GetStatusDesc(protoSettings[i].m_status));
		if (i != protoSettings.getCount() - 1) {
			*pnt++ = ' ';
			*pnt++ = '\0';
			cmdl = (char*)mir_realloc(cmdl, mir_strlen(cmdl) + mir_strlen(protoSettings[i + 1].m_szName) + mir_strlen(GetStatusDesc(protoSettings[i + 1].m_status)) + 4);
			pnt = cmdl + mir_strlen(cmdl);
		}
	}

	if (db_get_b(0, SSMODULENAME, SETTING_SHOWDIALOG, FALSE) == TRUE) {
		*pnt++ = ' ';
		*pnt++ = '\0';
		cmdl = (char*)mir_realloc(cmdl, mir_strlen(cmdl) + 12);
		pnt = cmdl + mir_strlen(cmdl);
		mir_strcpy(pnt, "/showdialog");
		pnt += 11;
		*pnt = '\0';
	}

	return cmdl;
}

static char* GetCMDL(TProtoSettings& protoSettings)
{
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);

	char* cmdl = (char*)mir_alloc(mir_strlen(path) + 4);
	mir_snprintf(cmdl, mir_strlen(path) + 4, "\"%s\" ", path);

	char* args = GetCMDLArguments(protoSettings);
	if (args) {
		cmdl = (char*)mir_realloc(cmdl, mir_strlen(cmdl) + mir_strlen(args) + 1);
		mir_strcat(cmdl, args);
		mir_free(args);
	}
	return cmdl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Link processing

static wchar_t* GetLinkDescription(TProtoSettings& protoSettings)
{
	if (protoSettings.getCount() == 0)
		return nullptr;

	CMStringW result(SHORTCUT_DESC);
	for (auto &p : protoSettings) {
		wchar_t *status;
		if (p->m_status == ID_STATUS_LAST)
			status = TranslateT("<last>");
		else if (p->m_status == ID_STATUS_CURRENT)
			status = TranslateT("<current>");
		else if (p->m_status >= MIN_STATUS && p->m_status <= MAX_STATUS)
			status = Clist_GetStatusModeDescription(p->m_status, 0);
		else
			status = nullptr;
		if (status == nullptr)
			status = TranslateT("<unknown>");

		result.AppendChar('\r');
		result.Append(p->m_tszAccName);
		result.AppendChar(':');
		result.AppendChar(' ');
		result.Append(status);
	}

	return mir_wstrndup(result, result.GetLength());
}

class CCmdlDlg : public CDlgBase
{
	TProtoSettings ps;

	CCtrlButton btnLink, btnCopy;

public:
	CCmdlDlg(int iProfileNo)
		: CDlgBase(hInst, IDD_CMDLOPTIONS),
		btnCopy(this, IDC_COPY),
		btnLink(this, IDC_SHORTCUT),
		ps(protoList)
	{
		GetProfile(iProfileNo, ps);
	}

	void OnInitDialog() override
	{
		char* cmdl = GetCMDL(ps);
		SetDlgItemTextA(m_hwnd, IDC_CMDL, cmdl);
		mir_free(cmdl);
	}

	void onClick_Copy(CCtrlButton*)
	{
		if (OpenClipboard(m_hwnd)) {
			EmptyClipboard();

			char cmdl[2048];
			GetDlgItemTextA(m_hwnd, IDC_CMDL, cmdl, _countof(cmdl));
			HGLOBAL cmdlGlob = GlobalAlloc(GMEM_MOVEABLE, sizeof(cmdl));
			if (cmdlGlob != nullptr) {
				LPTSTR cmdlStr = (LPTSTR)GlobalLock(cmdlGlob);
				memcpy(cmdlStr, &cmdl, sizeof(cmdl));
				GlobalUnlock(cmdlGlob);
				SetClipboardData(CF_TEXT, cmdlGlob);
			}
			CloseClipboard();
		}
	}

	void onClick_Link(CCtrlButton*)
	{
		wchar_t savePath[MAX_PATH];
		if (SHGetSpecialFolderPath(nullptr, savePath, CSIDL_DESKTOPDIRECTORY, FALSE))
			wcsncat_s(savePath, SHORTCUT_FILENAME, _countof(savePath) - mir_wstrlen(savePath));
		else
			mir_snwprintf(savePath, L".\\%s", SHORTCUT_FILENAME);

		// Get a pointer to the IShellLink interface.
		IShellLink *psl;
		HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
		if (SUCCEEDED(hres)) {
			ptrA args(GetCMDLArguments(ps));
			ptrW desc(GetLinkDescription(ps));

			// Set the path to the shortcut target, and add the
			// description.
			wchar_t path[MAX_PATH];
			GetModuleFileName(nullptr, path, _countof(path));
			psl->SetPath(path);
			psl->SetDescription(desc);
			psl->SetArguments(_A2T(args));

			// Query IShellLink for the IPersistFile interface for saving the
			// shortcut in persistent storage.
			IPersistFile *ppf;
			hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

			if (SUCCEEDED(hres)) {
				// Save the link by calling IPersistFile::Save.
				hres = ppf->Save(savePath, TRUE);
				ppf->Release();
			}
			psl->Release();
		}
	}
};

static INT_PTR CALLBACK StartupStatusOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bInitDone = FALSE;

	switch (msg) {
	case WM_INITDIALOG:
		bInitDone = FALSE;

		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SETPROFILE, db_get_b(0, SSMODULENAME, SETTING_SETPROFILE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OVERRIDE, db_get_b(0, SSMODULENAME, SETTING_OVERRIDE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWDIALOG, db_get_b(0, SSMODULENAME, SETTING_SHOWDIALOG, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETWINSTATE, db_get_b(0, SSMODULENAME, SETTING_SETWINSTATE, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETWINLOCATION, db_get_b(0, SSMODULENAME, SETTING_SETWINLOCATION, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETDOCKED, db_get_b(0, SSMODULENAME, SETTING_SETDOCKED, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETWINSIZE, db_get_b(0, SSMODULENAME, SETTING_SETWINSIZE, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OFFLINECLOSE, db_get_b(0, SSMODULENAME, SETTING_OFFLINECLOSE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTODIAL, db_get_b(0, SSMODULENAME, SETTING_AUTODIAL, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOHANGUP, db_get_b(0, SSMODULENAME, SETTING_AUTOHANGUP, 0) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, db_get_dw(0, SSMODULENAME, SETTING_SETPROFILEDELAY, 500), FALSE);
		SetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, db_get_dw(0, SSMODULENAME, SETTING_DLGTIMEOUT, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_XPOS, db_get_dw(0, SSMODULENAME, SETTING_XPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_YPOS, db_get_dw(0, SSMODULENAME, SETTING_YPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_WIDTH, db_get_dw(0, SSMODULENAME, SETTING_WIDTH, 0), FALSE);
		SetDlgItemInt(hwndDlg, IDC_HEIGHT, db_get_dw(0, SSMODULENAME, SETTING_HEIGHT, 0), FALSE);
		{
			int val = db_get_b(0, SSMODULENAME, SETTING_DOCKED, DOCKED_NONE);
			int item = SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
			SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_LEFT);
			if (val == DOCKED_LEFT)
				SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

			item = SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
			SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_RIGHT);
			if (val == DOCKED_RIGHT)
				SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

			item = SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));
			SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_NONE);
			if (val == DOCKED_NONE)
				SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILEDELAY), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIMEOUT), IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
		EnableWindow(GetDlgItem(hwndDlg, IDC_WINSTATE), IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_XPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
		EnableWindow(GetDlgItem(hwndDlg, IDC_YPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		SendMessage(hwndDlg, UM_REINITDOCKED, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSTATE, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSIZE, 0, 0);
		SetTimer(hwndDlg, 0, 100, nullptr);
		bInitDone = TRUE;
		break;

	case WM_TIMER:
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
			SetDlgItemTextA(hwndDlg, IDC_CURWINSIZE, "");
			SetDlgItemTextA(hwndDlg, IDC_CURWINLOC, "");
			break;
		}
		else {
			wchar_t text[128];
			mir_snwprintf(text, TranslateT("size: %d x %d"),
				db_get_dw(0, MODULE_CLIST, SETTING_WIDTH, 0),
				db_get_dw(0, MODULE_CLIST, SETTING_HEIGHT, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINSIZE, text);

			mir_snwprintf(text, TranslateT("loc: %d x %d"),
				db_get_dw(0, MODULE_CLIST, SETTING_XPOS, 0),
				db_get_dw(0, MODULE_CLIST, SETTING_YPOS, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINLOC, text);
		}
		break;

	case UM_REINITPROFILES:
		// creates profile combo box according to 'dat'
		SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_RESETCONTENT, 0, 0);
		{
			int defProfile;
			int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
			for (int i = 0; i < profileCount; i++) {
				wchar_t profileName[128];
				if (GetProfileName(i, (LPARAM)profileName))
					continue;

				int item = SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)profileName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETITEMDATA, item, i);
			}
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETCURSEL, defProfile, 0);
		}
		break;

	case UM_REINITDOCKED:
		EnableWindow(GetDlgItem(hwndDlg, IDC_SETDOCKED), db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 1));
		if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_SETDOCKED)))
			CheckDlgButton(hwndDlg, IDC_SETDOCKED, BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_DOCKED), IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
		break;

	case UM_REINITWINSTATE:
		{
			int val = db_get_b(0, SSMODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_RESETCONTENT, 0, 0);

			int item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hidden"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, (LPARAM)SETTING_STATE_HIDDEN);
			if (val == SETTING_STATE_HIDDEN)
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);

			if (!db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0)) {
				item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Minimized"));
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_MINIMIZED);
				if (val == SETTING_STATE_MINIMIZED)
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);
			}
			item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Normal"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_NORMAL);
			if (val == SETTING_STATE_NORMAL || (val == SETTING_STATE_MINIMIZED) && db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);
		}
		break;

	case UM_REINITWINSIZE:
		EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTH), IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HEIGHT), !db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0) && IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == LBN_SELCHANGE || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == EN_CHANGE)
			if (bInitDone)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_SETPROFILE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILEDELAY), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			break;
		case IDC_SHOWDIALOG:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIMEOUT), IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			break;
		case IDC_SETWINSTATE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_WINSTATE), IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
			break;
		case IDC_SETDOCKED:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DOCKED), IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
			break;
		case IDC_SETWINLOCATION:
			EnableWindow(GetDlgItem(hwndDlg, IDC_XPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_YPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			break;
		case IDC_SETWINSIZE:
			SendMessage(hwndDlg, UM_REINITWINSIZE, 0, 0);
			break;
		case IDC_SHOWCMDL:
			{
				int defProfile = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				CCmdlDlg *pDlg = new CCmdlDlg(defProfile);
				pDlg->Show();
				break;
			}
		}
		break;

	case WM_SHOWWINDOW:
		if (wParam == FALSE)
			break;

		bInitDone = FALSE;
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		SendMessage(hwndDlg, UM_REINITDOCKED, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSTATE, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSIZE, 0, 0);
		bInitDone = TRUE;
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_WIZFINISH:
			SSLoadMainOptions();
			break;

		case PSN_APPLY:
			db_set_b(0, SSMODULENAME, SETTING_SETPROFILE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE))
				db_set_dw(0, SSMODULENAME, SETTING_SETPROFILEDELAY, GetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, nullptr, FALSE));

			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG)) {
				int val = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				db_set_w(0, SSMODULENAME, SETTING_DEFAULTPROFILE, (WORD)val);
			}
			db_set_b(0, SSMODULENAME, SETTING_OVERRIDE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE));
			db_set_b(0, SSMODULENAME, SETTING_SHOWDIALOG, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG))
				db_set_dw(0, SSMODULENAME, SETTING_DLGTIMEOUT, GetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, nullptr, FALSE));

			db_set_b(0, SSMODULENAME, SETTING_SETWINSTATE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE)) {
				int val = (int)SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETCURSEL, 0, 0), 0);
				db_set_b(0, SSMODULENAME, SETTING_WINSTATE, (BYTE)val);
			}
			db_set_b(0, SSMODULENAME, SETTING_SETDOCKED, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED)) {
				int val = (int)SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETCURSEL, 0, 0), 0);
				db_set_b(0, SSMODULENAME, SETTING_DOCKED, (BYTE)val);
			}
			db_set_b(0, SSMODULENAME, SETTING_SETWINLOCATION, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION)) {
				db_set_dw(0, SSMODULENAME, SETTING_XPOS, GetDlgItemInt(hwndDlg, IDC_XPOS, nullptr, TRUE));
				db_set_dw(0, SSMODULENAME, SETTING_YPOS, GetDlgItemInt(hwndDlg, IDC_YPOS, nullptr, TRUE));
			}
			db_set_b(0, SSMODULENAME, SETTING_SETWINSIZE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
				db_set_dw(0, SSMODULENAME, SETTING_WIDTH, GetDlgItemInt(hwndDlg, IDC_WIDTH, nullptr, FALSE));
				db_set_dw(0, SSMODULENAME, SETTING_HEIGHT, GetDlgItemInt(hwndDlg, IDC_HEIGHT, nullptr, FALSE));
			}
			db_set_b(0, SSMODULENAME, SETTING_OFFLINECLOSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OFFLINECLOSE));
			db_set_b(0, SSMODULENAME, SETTING_AUTODIAL, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTODIAL));
			db_set_b(0, SSMODULENAME, SETTING_AUTOHANGUP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOHANGUP));
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// new profile dialog

class CAddProfileDlg : public CDlgBase
{
	CCtrlEdit edtProfile;
	CCtrlButton btnOk;

public:
	CAddProfileDlg()
		: CDlgBase(hInst, IDD_ADDPROFILE),
		btnOk(this, IDOK),
		edtProfile(this, IDC_PROFILENAME)
	{
		edtProfile.OnChange = Callback(this, &CAddProfileDlg::onChange_Edit);
	}

	void OnInitDialog() override
	{
		btnOk.Disable();
	}

	void OnApply() override
	{
		ptrW profileName(edtProfile.GetText());
		SendMessage(m_hwndParent, UM_ADDPROFILE, 0, (LPARAM)profileName.get());
	}

	void OnDestroy() override
	{
		EnableWindow(m_hwndParent, TRUE);
	}

	void onChange_Edit(CCtrlEdit*)
	{
		btnOk.Enable(edtProfile.SendMsg(EM_LINELENGTH, 0, 0) > 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// advanced options dialog

static int DeleteSetting(const char *szSetting, void *lParam)
{
	LIST<char> *p = (LIST<char> *)lParam;
	p->insert(mir_strdup(szSetting));
	return 0;
}

class CSSAdvancedOptDlg : public CDlgBase
{
	bool bNeedRebuildMenu;

	OBJLIST<PROFILEOPTIONS> arProfiles;

	void ReinitProfiles()
	{
		// creates profile combo box according to 'dat'
		cmbProfile.ResetContent();
		for (auto &it : arProfiles) 
			cmbProfile.AddString(it->tszName, arProfiles.indexOf(&it));

		cmbProfile.SetCurSel(0);
		SetProfile();
	}

	void SetProfile()
	{
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		chkCreateTTB.SetState(arProfiles[sel].createTtb);
		chkShowDialog.SetState(arProfiles[sel].showDialog);
		chkCreateMMI.SetState(arProfiles[sel].createMmi);
		chkInSubmenu.SetState(arProfiles[sel].inSubMenu);
		chkInSubmenu.Enable(arProfiles[sel].createMmi);
		chkRegHotkey.SetState(arProfiles[sel].regHotkey);
		edtHotkey.SendMsg(HKM_SETHOTKEY, arProfiles[sel].hotKey, 0);
		edtHotkey.Enable(arProfiles[sel].regHotkey);

		// fill proto list
		lstAccount.ResetContent();
		for (auto &it : arProfiles[sel].ps)
			lstAccount.AddString(it->m_tszAccName, (LPARAM)it);
		lstAccount.SetCurSel(0);

		SetProtocol();
	}

	void SetProtocol()
	{
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			// fill status box
			SMProto* ps = (SMProto*)lstAccount.GetItemData(idx);

			int flags = (CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_2, 0))&~(CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_5, 0));
			lstStatus.ResetContent();
			for (int i = 0; i < _countof(statusModeList); i++) {
				if ((flags & statusModePf2List[i]) || (statusModeList[i] == ID_STATUS_OFFLINE)) {
					int item = lstStatus.AddString(Clist_GetStatusModeDescription(statusModeList[i], 0), statusModeList[i]);
					if (ps->m_status == statusModeList[i])
						lstStatus.SetCurSel(item);
				}
			}

			int item = lstStatus.AddString(TranslateT("<current>"), ID_STATUS_CURRENT);
			if (ps->m_status == ID_STATUS_CURRENT)
				lstStatus.SetCurSel(item);

			item = lstStatus.AddString(TranslateT("<last>"), ID_STATUS_LAST);
			if (ps->m_status == ID_STATUS_LAST)
				lstStatus.SetCurSel(item);
		}

		SetStatusMsg();
	}

	// set status message
	void SetStatusMsg()
	{
		bool bStatusMsg = false;
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			SMProto *ps = (SMProto*)lstAccount.GetItemData(idx);

			CheckRadioButton(m_hwnd, IDC_MIRANDAMSG, IDC_CUSTOMMSG, ps->m_szMsg != nullptr ? IDC_CUSTOMMSG : IDC_MIRANDAMSG);
			if (ps->m_szMsg != nullptr)
				edtStatusMsg.SetText(ps->m_szMsg);

			bStatusMsg = ((((CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG)) &&
				(CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(ps->m_status))) || (ps->m_status == ID_STATUS_CURRENT) || (ps->m_status == ID_STATUS_LAST));
		}
		chkMiranda.Enable(bStatusMsg);
		chkCustom.Enable(bStatusMsg);
		btnHelp.Enable(bStatusMsg && chkCustom.GetState());
		edtStatusMsg.Enable(bStatusMsg && chkCustom.GetState());
	}

	CCtrlEdit edtStatusMsg, edtHotkey;
	CCtrlCombo cmbProfile;
	CCtrlCheck chkMiranda, chkCustom, chkCreateMMI, chkInSubmenu, chkRegHotkey, chkCreateTTB, chkShowDialog;
	CCtrlButton btnAdd, btnDelete, btnHelp;
	CCtrlListBox lstStatus, lstAccount;

public:
	CSSAdvancedOptDlg() :
		CDlgBase(hInst, IDD_OPT_STATUSPROFILES),
		cmbProfile(this, IDC_PROFILE),
		btnAdd(this, IDC_ADDPROFILE),
		btnHelp(this, IDC_VARIABLESHELP),
		btnDelete(this, IDC_DELPROFILE),
		chkCustom(this, IDC_CUSTOMMSG),
		chkMiranda(this, IDC_MIRANDAMSG),
		chkCreateMMI(this, IDC_CREATEMMI),
		chkInSubmenu(this, IDC_INSUBMENU),
		chkRegHotkey(this, IDC_REGHOTKEY),
		chkCreateTTB(this, IDC_CREATETTB),
		chkShowDialog(this, IDC_SHOWDIALOG),
		edtHotkey(this, IDC_HOTKEY),
		edtStatusMsg(this, IDC_STATUSMSG),
		lstStatus(this, IDC_STATUS),
		lstAccount(this, IDC_PROTOCOL),
		arProfiles(5),
		bNeedRebuildMenu(false)
	{
		btnAdd.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Add);
		btnHelp.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Help);
		btnDelete.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Delete);

		cmbProfile.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Profile);

		chkCreateMMI.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_CreateMMI);
		chkInSubmenu.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_InSubmenu);
		chkCustom.OnChange = chkMiranda.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_StatusMsg);
		chkRegHotkey.OnChange = chkCreateTTB.OnChange = chkShowDialog.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Option);
		
		edtHotkey.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Hotkey);
		edtStatusMsg.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_EdtStatusMsg);

		lstStatus.OnSelChange = Callback(this, &CSSAdvancedOptDlg::onChange_Status);
		lstAccount.OnSelChange = Callback(this, &CSSAdvancedOptDlg::onChange_Account);

		m_OnFinishWizard = Callback(this, &CSSAdvancedOptDlg::OnFinishWizard);
	}

	void OnFinishWizard(void*)
	{
		SSLoadMainOptions();
	}

	virtual void OnInitDialog() override
	{
		chkCreateMMI.SetText(TranslateT("Create a status menu item"));

		int defProfile;
		int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
		if (profileCount == 0) {
			profileCount = 1;
			defProfile = 0;
			NotifyChange();
		}

		for (int i = 0; i < profileCount; i++) {
			PROFILEOPTIONS *ppo = new PROFILEOPTIONS;
			TProtoSettings &ar = ppo->ps;

			if (GetProfile(i, ar) == -1) {
				/* create an empty profile */
				if (i == defProfile)
					ppo->tszName = mir_wstrdup(TranslateT("default"));
				else
					ppo->tszName = mir_wstrdup(TranslateT("unknown"));
			}
			else {
				ppo->tszName = db_get_wsa(0, SSMODULENAME, OptName(i, SETTING_PROFILENAME));
				if (ppo->tszName == nullptr) {
					if (i == defProfile)
						ppo->tszName = mir_wstrdup(TranslateT("default"));
					else
						ppo->tszName = mir_wstrdup(TranslateT("unknown"));
				}
				ppo->createTtb = db_get_b(0, SSMODULENAME, OptName(i, SETTING_CREATETTBBUTTON), 0);
				ppo->showDialog = db_get_b(0, SSMODULENAME, OptName(i, SETTING_SHOWCONFIRMDIALOG), 0);
				ppo->createMmi = db_get_b(0, SSMODULENAME, OptName(i, SETTING_CREATEMMITEM), 0);
				ppo->inSubMenu = db_get_b(0, SSMODULENAME, OptName(i, SETTING_INSUBMENU), 1);
				ppo->regHotkey = db_get_b(0, SSMODULENAME, OptName(i, SETTING_REGHOTKEY), 0);
				ppo->hotKey = db_get_w(0, SSMODULENAME, OptName(i, SETTING_HOTKEY), MAKEWORD((char)('0' + i), HOTKEYF_CONTROL | HOTKEYF_SHIFT));
			}
			arProfiles.insert(ppo);
		}
		if (hTTBModuleLoadedHook == nullptr)
			chkCreateTTB.Disable();

		ReinitProfiles();
		ShowWindow(GetDlgItem(m_hwnd, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELPEX) ? SW_SHOW : SW_HIDE);
	}

	virtual void OnApply() override
	{
		int oldCount = db_get_w(0, SSMODULENAME, SETTING_PROFILECOUNT, 0);
		for (int i = 0; i < oldCount; i++) {
			LIST<char> arSettings(10);
			db_enum_settings(0, DeleteSetting, SSMODULENAME, &arSettings);

			char setting[128];
			int len = mir_snprintf(setting, "%d_", i);
			for (auto &it : arSettings) {
				if (!strncmp(setting, it, len))
					db_unset(0, SSMODULENAME, it);
				mir_free(it);
			}
		}

		for (auto &it : arProfiles) {
			int i = arProfiles.indexOf(&it);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_SHOWCONFIRMDIALOG), it->showDialog);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_CREATETTBBUTTON), it->createTtb);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_CREATEMMITEM), it->createMmi);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_INSUBMENU), it->inSubMenu);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_REGHOTKEY), it->regHotkey);
			db_set_w(0, SSMODULENAME, OptName(i, SETTING_HOTKEY), it->hotKey);
			db_set_ws(0, SSMODULENAME, OptName(i, SETTING_PROFILENAME), it->tszName);

			for (auto jt : it->ps) {
				if (jt->m_szMsg != nullptr) {
					char setting[128];
					mir_snprintf(setting, "%s_%s", jt->m_szName, SETTING_PROFILE_STSMSG);
					db_set_ws(0, SSMODULENAME, OptName(i, setting), jt->m_szMsg);
				}
				db_set_w(0, SSMODULENAME, OptName(i, jt->m_szName), jt->m_status);
			}
		}
		db_set_w(0, SSMODULENAME, SETTING_PROFILECOUNT, (WORD)arProfiles.getCount());

		// Rebuild status menu
		if (bNeedRebuildMenu)
			Menu_ReloadProtoMenus();
	}

	// add a profile
	void onClick_Add(CCtrlButton*)
	{
		CAddProfileDlg *pDlg = new CAddProfileDlg();
		pDlg->SetParent(m_hwnd);
		pDlg->Show();
		EnableWindow(m_hwnd, FALSE);
	}

	void onClick_Delete(CCtrlButton*)
	{
		// wparam == profile no
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		if (arProfiles.getCount() == 1) {
			MessageBox(nullptr, TranslateT("At least one profile must exist"), TranslateT("Status manager"), MB_OK);
			return;
		}

		arProfiles.remove(sel);

		int defProfile;
		GetProfileCount((WPARAM)&defProfile, 0);
		if (sel == defProfile) {
			MessageBox(nullptr, TranslateT("Your default profile will be changed"), TranslateT("Status manager"), MB_OK);
			db_set_w(0, SSMODULENAME, SETTING_DEFAULTPROFILE, 0);
		}
		ReinitProfiles();;
	}

	void onClick_Help(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, IDC_STATUSMSG, VHF_INPUT | VHF_EXTRATEXT | VHF_HELP | VHF_FULLFILLSTRUCT | VHF_HIDESUBJECTTOKEN, nullptr, "Protocol ID");
	}

	void onChange_Status(CCtrlListBox*)
	{
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			SMProto* ps = (SMProto*)lstAccount.GetItemData(idx);
			ps->m_status = lstStatus.GetItemData(lstStatus.GetCurSel());
			NotifyChange();
		}
		SetStatusMsg();
	}

	void onChange_Account(CCtrlListBox*)
	{
		SetProtocol();
	}

	void onChange_Profile(CCtrlCombo*)
	{
		SetProfile();
	}

	void onChange_StatusMsg(CCtrlCheck*)
	{
		int len;
		SMProto* ps = (SMProto*)lstAccount.GetItemData(lstAccount.GetCurSel());
		if (ps->m_szMsg != nullptr)
			mir_free(ps->m_szMsg);

		ps->m_szMsg = nullptr;
		if (chkCustom.GetState()) {
			len = edtStatusMsg.SendMsg(WM_GETTEXTLENGTH, 0, 0);
			ps->m_szMsg = (wchar_t*)mir_calloc(sizeof(wchar_t) * (len + 1));
			GetDlgItemText(m_hwnd, IDC_STATUSMSG, ps->m_szMsg, (len + 1));
		}
		SetStatusMsg();
	}

	void onChange_Hotkey(CCtrlEdit*)
	{
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		arProfiles[sel].hotKey = edtHotkey.SendMsg(HKM_GETHOTKEY, 0, 0);
	}

	void onChange_EdtStatusMsg(CCtrlEdit*)
	{
		// update the status message in memory, this is done on each character tick, not nice
		// but it works
		SMProto* ps = (SMProto*)lstAccount.GetItemData(lstAccount.GetCurSel());
		if (ps->m_szMsg != nullptr) {
			if (*ps->m_szMsg)
				mir_free(ps->m_szMsg);
			ps->m_szMsg = nullptr;
		}
		int len = edtStatusMsg.SendMsg(WM_GETTEXTLENGTH, 0, 0);
		ps->m_szMsg = (wchar_t*)mir_calloc(sizeof(wchar_t) * (len + 1));
		GetDlgItemText(m_hwnd, IDC_STATUSMSG, ps->m_szMsg, (len + 1));
	}

	void onChange_CreateMMI(CCtrlCheck*)
	{
		chkInSubmenu.Enable(chkCreateMMI.GetState());
		onChange_InSubmenu(nullptr);
	}

	void onChange_InSubmenu(CCtrlCheck*)
	{
		bNeedRebuildMenu = TRUE;
		onChange_Option(nullptr);
	}

	void onChange_Option(CCtrlCheck*)
	{
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		PROFILEOPTIONS &po = arProfiles[sel];
		po.createMmi = chkCreateMMI.GetState();
		po.inSubMenu = chkInSubmenu.GetState();
		po.createTtb = chkCreateTTB.GetState();
		po.regHotkey = chkRegHotkey.GetState();
		po.showDialog = chkShowDialog.GetState();
		edtHotkey.Enable(chkRegHotkey.GetState());
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case UM_ADDPROFILE:
			wchar_t *tszName = (wchar_t*)lParam;
			if (tszName == nullptr)
				break;

			PROFILEOPTIONS* ppo = new PROFILEOPTIONS;
			ppo->tszName = mir_wstrdup(tszName);
			arProfiles.insert(ppo);

			ReinitProfiles();
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

int StartupStatusOptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.hInstance = hInst;
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Startup status");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STARTUPSTATUS);
	odp.pfnDlgProc = StartupStatusOptDlgProc;
	Options_AddPage(wparam, &odp, SSLangPack);

	odp.szTab.a = LPGEN("Status profiles");
	odp.pszTemplate = nullptr;
	odp.pfnDlgProc = nullptr;
	odp.pDialog = new CSSAdvancedOptDlg();
	Options_AddPage(wparam, &odp, SSLangPack);
	return 0;
}
