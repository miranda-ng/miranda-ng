#include "stdafx.h"

uint32_t settingDefault[] =
{
	RGB(64, 255, 64),
	RGB(255, 255, 64),
	RGB(128, 128, 128),
	RGB(192, 192, 192),

	6000,
	5000
};
char *settingName[] =
{
	"colorRecv",
	"colorSent",
	"colorUnsent",
	"colorTosend",

	"SendDelay",
	"ChunkSize"
};
int settingId[] =
{
	IDC_RECV,
	IDC_SENT,
	IDC_UNSENT,
	IDC_TOSEND,

	-IDC_SENDDELAY,
	-IDC_CHUNKSIZE
};
//
// OptionsDlgProc()
// this handles the options page
// verwaltet die Optionsseite
//
INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		for (int indx = 0; indx < _countof(settingId); indx++)
			if (settingId[indx] > 0)
				SendDlgItemMessage(hwndDlg, settingId[indx], CPM_SETCOLOUR, 0, g_plugin.getDword(settingName[indx], settingDefault[indx]));
			else
				SetDlgItemInt(hwndDlg, -settingId[indx], g_plugin.getDword(settingName[indx], settingDefault[indx]), FALSE);

		CheckDlgButton(hwndDlg, IDC_ALPHANUM, g_plugin.getDword("base64", 1) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		if (MAKEWPARAM(IDC_ALPHANUM, BN_CLICKED) != wParam) {
			for (int indx = 0; indx < _countof(settingId); indx++) {
				if (LOWORD(wParam) == abs(settingId[indx])) {
					if (settingId[indx] > 0) {
						if (HIWORD(wParam) != CPN_COLOURCHANGED) return FALSE;
					}
					else {
						if (HIWORD(wParam) != EN_CHANGE) return FALSE;
						if ((HWND)lParam != GetFocus()) return FALSE;
					}
				}
			}
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	
	case WM_DESTROY:
		return FALSE;

	case WM_NOTIFY:
		if ((((NMHDR *)lParam)->idFrom == 0) && (((LPNMHDR)lParam)->code == PSN_APPLY)) {
			int value;
			BOOL succ;

			for (int indx = 0; indx < _countof(settingId); indx++) {
				if (settingId[indx] > 0)
					value = SendDlgItemMessage(hwndDlg, settingId[indx], CPM_GETCOLOUR, 0, 0);
				else {
					value = GetDlgItemInt(hwndDlg, -settingId[indx], &succ, FALSE);
					if (!succ)
						value = settingDefault[indx];
				}
				g_plugin.setDword(settingName[indx], value);
			}
			g_plugin.setDword("base64", (IsDlgButtonChecked(hwndDlg, IDC_ALPHANUM) == BST_CHECKED) ? 1 : 0);

			return TRUE;
		}
		break;
	}

	return FALSE;
}
