#include "main.h"

DWORD settingDefault[] =
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
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		for (int indx = 0; indx < SIZEOF(settingId); indx++)
			if (settingId[indx] > 0)
				SendDlgItemMessage(hwndDlg, settingId[indx], CPM_SETCOLOUR, 0, db_get_dw(NULL, SERVICE_NAME, settingName[indx], settingDefault[indx]));
			else
				SetDlgItemInt(hwndDlg, -settingId[indx], db_get_dw(NULL, SERVICE_NAME, settingName[indx], settingDefault[indx]), FALSE);

		CheckDlgButton(hwndDlg, IDC_ALPHANUM, db_get_dw(NULL, SERVICE_NAME, "base64", 1) ? BST_CHECKED : BST_UNCHECKED);

		return TRUE;

	}
	case WM_COMMAND:
	{
		if (//MAKEWPARAM(IDC_AUTO, BN_CLICKED) != wParam || 
			MAKEWPARAM(IDC_ALPHANUM, BN_CLICKED) != wParam)
		{
			for (int indx = 0; indx < SIZEOF(settingId); indx++)
			{
				if (LOWORD(wParam) == abs(settingId[indx]))
				{
					if (settingId[indx] > 0)
					{
						if (HIWORD(wParam) != CPN_COLOURCHANGED) return FALSE;
					}
					else
					{
						if (HIWORD(wParam) != EN_CHANGE) return FALSE;
						if ((HWND)lParam != GetFocus()) return FALSE;
					}
				}
			}
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	}
	case WM_DESTROY:
		return FALSE;

	case WM_NOTIFY:
	{
		if ((((NMHDR*)lParam)->idFrom == 0) && (((LPNMHDR)lParam)->code == PSN_APPLY))
		{
			int value;
			BOOL succ;

			for (int indx = 0; indx < SIZEOF(settingId); indx++)
			{
				if (settingId[indx] > 0)
					value = SendDlgItemMessage(hwndDlg, settingId[indx], CPM_GETCOLOUR, 0, 0);
				else
				{
					value = GetDlgItemInt(hwndDlg, -settingId[indx], &succ, FALSE);
					if (!succ) value = settingDefault[indx];
				}
				db_set_dw(NULL, SERVICE_NAME, settingName[indx], value);
			}
			db_set_dw(NULL, SERVICE_NAME, "base64", (IsDlgButtonChecked(hwndDlg, IDC_ALPHANUM) == BST_CHECKED) ? 1 : 0);

			return TRUE;
		}
		break;
	}

	}

	return FALSE;
}
