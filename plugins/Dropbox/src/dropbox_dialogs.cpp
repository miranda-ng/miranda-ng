#include "common.h"

INT_PTR CALLBACK CDropbox::TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *token = reinterpret_cast<char*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		token = (char*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			//::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)CSkypeProto::IconList[0].Handle);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)CSkypeProto::IconList[0].Handle);

			/*wchar_t title[MAX_PATH];
			::mir_sntprintf(
			title, 
			MAX_PATH, 
			::TranslateT("Enter a password for %s:"), 
			param->login);*/
			//::SetDlgItemText(hwndDlg, IDC_INSTRUCTION, title);

			SendDlgItemMessage(hwndDlg, IDC_TOKEN, EM_LIMITTEXT, 128 - 1, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					char data[128];
					GetDlgItemTextA(hwndDlg, IDC_TOKEN, data, SIZEOF(data));
					strcpy(token, data);

					EndDialog(hwndDlg, IDOK);
				}
				break;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CDropbox::MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}