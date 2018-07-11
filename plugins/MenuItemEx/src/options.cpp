#include "stdafx.h"

extern BOOL bPopupService;

struct {
	int idc;
	int flag;
}
static const checkboxes[] = {
	{ IDC_COPYID, VF_CID },
	{ IDC_STATUSMSG, VF_STAT },
	{ IDC_COPYIP, VF_CIP },
	{ IDC_COPYMIRVER, VF_CMV },
	{ IDC_VIS, VF_VS },
	{ IDC_SHOWALPHAICONS, VF_SAI },
	{ IDC_HIDE, VF_HFL },
	{ IDC_IGNORE, VF_IGN },
	{ IDC_IGNOREHIDE, VF_IGNH },
	{ IDC_PROTOS, VF_PROTO },
	{ IDC_ADDED, VF_ADD },
	{ IDC_AUTHREQ, VF_REQ },
	{ IDC_SHOWID, VF_SHOWID },
	{ IDC_COPYIDNAME, VF_CIDN },
	{ IDC_RECVFILES, VF_RECV },
	{ IDC_SMNAME, VF_SMNAME },
	{ IDC_TRIMID, VF_TRIMID }
};

INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	DWORD flags = db_get_dw(NULL, MODULENAME, "flags", vf_default);
	wchar_t buffer[64] = { 0 };
	int i;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		for (auto &it : checkboxes)
			CheckDlgButton(hdlg, it.idc, (flags & it.flag) ? BST_CHECKED : BST_UNCHECKED);

		if (bPopupService) {
			for (i = 0; i < 4; i++) {
				GetDlgItemText(hdlg, checkboxes[i].idc, buffer, (_countof(buffer) - 3));
				mir_wstrcat(buffer, L" *");
				SetDlgItemText(hdlg, checkboxes[i].idc, buffer);
			}
		}
		else ShowWindow(GetDlgItem(hdlg, IDC_HINT1), SW_HIDE);

		SendMessage(hdlg, WM_USER + 50, 0, 0);
		return 0;

	case WM_USER + 50:
		EnableWindow(GetDlgItem(hdlg, IDC_SHOWALPHAICONS), IsDlgButtonChecked(hdlg, IDC_VIS) == BST_CHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_IGNOREHIDE), IsDlgButtonChecked(hdlg, IDC_IGNORE) == BST_CHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_COPYIDNAME), IsDlgButtonChecked(hdlg, IDC_COPYID) == BST_CHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_SHOWID), IsDlgButtonChecked(hdlg, IDC_COPYID) == BST_CHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_TRIMID), (IsDlgButtonChecked(hdlg, IDC_COPYID) == BST_CHECKED && IsDlgButtonChecked(hdlg, IDC_SHOWID) == BST_CHECKED));
		EnableWindow(GetDlgItem(hdlg, IDC_SMNAME), IsDlgButtonChecked(hdlg, IDC_STATUSMSG) == BST_CHECKED);
		return 1;

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->code) {
		case PSN_APPLY:
			DWORD mod_flags = 0;

			for (auto &it : checkboxes)
				mod_flags |= IsDlgButtonChecked(hdlg, it.idc) ? it.flag : 0;

			db_set_dw(NULL, MODULENAME, "flags", mod_flags);
			return 1;
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wparam) == BN_CLICKED && GetFocus() == (HWND)lparam) {
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			if (LOWORD(wparam) == IDC_VIS ||
				LOWORD(wparam) == IDC_IGNORE ||
				LOWORD(wparam) == IDC_COPYID ||
				LOWORD(wparam) == IDC_STATUSMSG ||
				LOWORD(wparam) == IDC_SHOWID)
			{
				SendMessage(hdlg, WM_USER + 50, 0, 0);
			}
		}
		return 0;

	case WM_CLOSE:
		EndDialog(hdlg, 0);
		return 0;
	}
	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 955000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.a = MODULENAME;
	odp.pfnDlgProc = OptionsProc;
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wparam, &odp);
	return 0;
}
