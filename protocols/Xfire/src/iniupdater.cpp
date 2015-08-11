//iniupdater.cpp by dufte, gro�en dank an xfireplus.com

#include "stdafx.h"

#include "iniupdater.h"
#include "baseProtocol.h"
#include "variables.h"

extern HANDLE XFireWorkingFolder;
extern HANDLE XFireIconFolder;

INT_PTR CALLBACK DlgUpdateDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			char *buf = NULL; //leeren zeiger f�r den empfangen buffer
			GetWWWContent2(INI_WHATSNEW, NULL, FALSE, &buf);

			if (buf != NULL) {
				SetDlgItemTextA(hwndDlg, IDC_UPDATEGAMES, buf);
				delete[] buf;
			}

			SetFocus(GetDlgItem(hwndDlg, IDOK));

			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

void UpdateMyXFireIni(LPVOID)
{
	char request[1024];
	char *inipath = XFireGetFoldersPath("IniFile");

	//ini pfad rausbekommen
	char file[1024], file2[1024], file3[1024];
	mir_snprintf(file, _countof(file), "%sxfire_games.new", inipath);
	mir_snprintf(file2, _countof(file2), "%sxfire_games.ini", inipath);
	mir_snprintf(file3, _countof(file3), "%sxfire_games.old", inipath);

	mir_snprintf(request, _countof(request), "%s%d", INI_URLREQUEST, getfilesize(file2));

	if (CheckWWWContent(request)) {
		if (db_get_b(NULL, protocolname, "dontaskforupdate", 0) == 1 || DialogBox(hinstance, MAKEINTRESOURCE(IDD_UPDATE), NULL, DlgUpdateDialogProc) == IDOK) {
			if (GetWWWContent2(request, file, FALSE)) {
				//altes backup l�schen
				remove(file3);
				//derzeitige ini und sichern
				rename(file2, file3);
				//l�sche .old, wenn aktiv
				if (db_get_b(NULL, protocolname, "nobackupini", 0))
					remove(file3);
				//neue aktiv schalten
				rename(file, file2);

				if (db_get_b(NULL, protocolname, "dontaskforupdate", 0) == 0) MSGBOX(Translate("The xfire_games.ini was updated."));
			}
			else MSGBOX(Translate("Error during xfire_games.ini update."));
		}
	}
}

void UpdateMyIcons(LPVOID)
{
	char request[1024];
	char *inipath = XFireGetFoldersPath("IconsFile");

	//ini pfad rausbekommen
	char file[1024], file2[1024], file3[1024];
	mir_snprintf(file, _countof(file), "%sicons.new", inipath);
	mir_snprintf(file2, _countof(file2), "%sicons.dll", inipath);
	mir_snprintf(file3, _countof(file3), "%sicons.old", inipath);

	mir_snprintf(request, _countof(request), "%s%d", ICO_URLREQUEST, getfilesize(file2));

	if (CheckWWWContent(request)) {
		if (db_get_b(NULL, protocolname, "dontaskforupdate", 0) == 1 || MessageBox(NULL, TranslateT("There is a new Icons.dll online, do you want to update now?"), TranslateT(PLUGIN_TITLE), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			if (GetWWWContent2(request, file, FALSE)) {
				//altes backup l�schen
				remove(file3);
				//derzeitige ini und sichern
				rename(file2, file3);
				//l�sche .old, wenn aktiv
				if (db_get_b(NULL, protocolname, "nobackupini", 0))
					remove(file3);
				//neue aktiv schalten
				rename(file, file2);

				if (db_get_b(NULL, protocolname, "dontaskforupdate", 0) == 0) MSGBOX(Translate("The Icons.dll was updated."));
			}
			else MSGBOX(Translate("Error during Icons.dll Update."));
		}
	}
}
