#include "common.h"

INT_PTR CToxProto::ToxProfileManagerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TCHAR *profilePath = (TCHAR*)GetWindowLongPtr(hwnd, DWLP_USER);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			profilePath = (TCHAR*)mir_calloc(sizeof(TCHAR)* MAX_PATH);
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)profilePath);

			CheckDlgButton(hwnd, IDC_CREATE_NEW, TRUE);
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;

	case WM_DESTROY:
		mir_free(profilePath);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_USE_EXISTING:
			EnableWindow(GetDlgItem(hwnd, IDC_PROFILE_PATH), IsDlgButtonChecked(hwnd, IDC_USE_EXISTING));
			EnableWindow(GetDlgItem(hwnd, IDC_BROWSE_PROFILE), IsDlgButtonChecked(hwnd, IDC_USE_EXISTING));
			break;

		case IDC_BROWSE_PROFILE:
		{
			TCHAR filter[MAX_PATH] = { 0 };
			mir_sntprintf(filter, MAX_PATH, _T("%s\0*.*"), TranslateT("All files (*.*)"));

			OPENFILENAME ofn = { sizeof(ofn) };
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = profilePath;
			ofn.lpstrTitle = TranslateT("Select tox profile");
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

			if (GetOpenFileName(&ofn) && profilePath)
			{
				SetDlgItemText(hwnd, IDC_PROFILE_PATH, profilePath);
			}
		}
			break;

		case IDOK:
		{
			if (IsDlgButtonChecked(hwnd, IDC_USE_EXISTING))
			{
				if (profilePath != NULL)
				{
					std::tstring toxProfilePath = proto->GetToxProfilePath();
					CopyFile(profilePath, toxProfilePath.c_str(), FALSE);
				}
			}
			EndDialog(hwnd, 1);
		}
			break;

		}
		break;
	}

	return FALSE;
}