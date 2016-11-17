/*
Splash Screen Plugin for Miranda NG (www.miranda-ng.org)
(c) 2004-2007 nullbie, (c) 2005-2007 Thief

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

#include "stdafx.h"

wchar_t szPath2Spash[MAX_PATH], szSoundFilePath[MAX_PATH];

// Reads values from db
void ReadDbConfig()
{
	options.active = db_get_b(NULL, MODNAME, "Active", 1);
	options.playsnd = db_get_b(NULL, MODNAME, "PlaySound", 0);
	options.fadein = db_get_b(NULL, MODNAME, "FadeIn", 1);
	options.fadeout = db_get_b(NULL, MODNAME, "FadeOut", 1);
	options.showtime = db_get_dw(NULL, MODNAME, "TimeToShow", 2000);
	options.fisteps = db_get_dw(NULL, MODNAME, "FadeinSpeed", 5);
	options.fosteps = db_get_dw(NULL, MODNAME, "FadeoutSpeed", 5);
	options.inheritGS = db_get_b(NULL, MODNAME, "InheritGlobalSound", 1);
	options.showversion = db_get_b(NULL, MODNAME, "ShowVersion", 0);
	options.random = db_get_b(NULL, MODNAME, "Random", 0);
}

BOOL Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			if (!png2dibavail) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_PNG2DIBWARN), SW_SHOW);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVE), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RANDOM), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPLASHPATH), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSESPLASH), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWVERSION), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_VERSIONPREFIX), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWTIME), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ST_SPIN), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FADEIN), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FISTEP), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FI_SPIN), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FADEOUT), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FOSTEP), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FO_SPIN), false);
			}
			else {
				ReadDbConfig();
				wchar_t inBuf[80];
				DBVARIANT dbv = { 0 };
				if (!db_get_ws(NULL, MODNAME, "Path", &dbv)) {
					mir_wstrcpy(inBuf, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					mir_wstrcpy(inBuf, L"splash\\splash.png");
				SetDlgItemText(hwndDlg, IDC_SPLASHPATH, inBuf);

				if (!db_get_ws(NULL, MODNAME, "Sound", &dbv)) {
					mir_wstrcpy(inBuf, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					mir_wstrcpy(inBuf, L"sounds\\startup.wav");
				SetDlgItemText(hwndDlg, IDC_SNDPATH, inBuf);

				if (!db_get_ws(NULL, MODNAME, "VersionPrefix", &dbv)) {
					mir_wstrcpy(inBuf, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					mir_wstrcpy(inBuf, L"");
				SetDlgItemText(hwndDlg, IDC_VERSIONPREFIX, inBuf);

				if (options.active)
					CheckDlgButton(hwndDlg, IDC_ACTIVE, BST_CHECKED);
				if (options.playsnd && !options.inheritGS)
					CheckDlgButton(hwndDlg, IDC_PLAYSND, BST_INDETERMINATE);
				else if (options.playsnd)
					CheckDlgButton(hwndDlg, IDC_PLAYSND, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOOPSOUND), false);
				if (options.fadein)
					CheckDlgButton(hwndDlg, IDC_FADEIN, BST_CHECKED);
				if (options.fadeout)
					CheckDlgButton(hwndDlg, IDC_FADEOUT, BST_CHECKED);
				if (options.random)
					CheckDlgButton(hwndDlg, IDC_RANDOM, BST_CHECKED);
				if (options.showversion)
					CheckDlgButton(hwndDlg, IDC_SHOWVERSION, BST_CHECKED);

				SetDlgItemText(hwndDlg, IDC_SHOWTIME, _itow(options.showtime, inBuf, 10));
				SetDlgItemText(hwndDlg, IDC_FISTEP, _itow(options.fisteps, inBuf, 10));
				SetDlgItemText(hwndDlg, IDC_FOSTEP, _itow(options.fosteps, inBuf, 10));

				SendDlgItemMessage(hwndDlg, IDC_SHOWTIME, EM_LIMITTEXT, 5, 0);
			}

			return TRUE;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_PREVIEW:
				{
					ShowSplash(true);
					break;
				}

			case IDC_ACTIVE:
			case IDC_PLAYSND:
			case IDC_LOOPSOUND:
			case IDC_FADEIN:
			case IDC_FADEOUT:
			case IDC_SHOWTIME:
			case IDC_RANDOM:
			case IDC_SHOWVERSION:
			case IDC_FISTEP:
			case IDC_FOSTEP:
				{
					if (IsDlgButtonChecked(hwndDlg, IDC_FADEIN)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_FISTEP), true);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FI_SPIN), true);
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_FISTEP), false);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FI_SPIN), false);
					}
					if (IsDlgButtonChecked(hwndDlg, IDC_FADEOUT)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_FOSTEP), true);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FO_SPIN), true);
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_FOSTEP), false);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FO_SPIN), false);
					}

					if ((HWND)lParam != GetFocus())
						return 0;
					else {
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						break;
					}
					break;
				}

			case IDC_CHOOSESPLASH:
				{
					wchar_t szTempPath[MAX_PATH], initDir[MAX_PATH];

					if (Exists(szSplashFile)) {
						mir_wstrcpy(initDir, szSplashFile);
						wchar_t *pos = wcsrchr(initDir, '\\');
						if (pos != NULL)
							*pos = 0;
					}
					else {
						szMirDir = Utils_ReplaceVarsW(L"%miranda_path%");
						mir_wstrcpy(initDir, szMirDir);
						mir_free(szMirDir);
					}

					wchar_t tmp[MAX_PATH];
					mir_snwprintf(tmp, L"%s (*.png, *.bmp)%c*.png;*.bmp%c%c", TranslateT("Graphic files"), 0, 0, 0);

					OPENFILENAME ofn = { 0 };
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.lpstrFilter = tmp;
					ofn.lpstrFile = szTempPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY;
					ofn.lpstrInitialDir = initDir;
					*szTempPath = '\0';
					ofn.lpstrDefExt = L"";

					if (GetOpenFileName(&ofn)) {
						mir_wstrcpy(szSplashFile, szTempPath);

						#ifdef _DEBUG
						logMessage(L"Set path", szSplashFile);
						#endif
						// Make path relative
						int result = PathToRelativeW(szTempPath, szPath2Spash);
						if (result && mir_wstrlen(szPath2Spash) > 0) {
							if (options.random) {
								wchar_t *pos = wcsrchr(szPath2Spash, '\\');
								if (pos != NULL) {
									*pos = 0;
									mir_wstrcat(szPath2Spash, L"\\");
								}
							}

							SetDlgItemText(hwndDlg, IDC_SPLASHPATH, szPath2Spash);
						}

						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

					break;
				}

			case IDC_CHOOSESND:
				{
					wchar_t szTempPath[MAX_PATH], initDir[MAX_PATH];
					wchar_t *pos;

					if (Exists(szSoundFile)) {
						mir_wstrcpy(initDir, szSoundFile);
						pos = wcsrchr(initDir, '\\');
						if (pos != NULL) *pos = 0;
					}
					else {
						szMirDir = Utils_ReplaceVarsW(L"%miranda_path%");
						mir_wstrcpy(initDir, szMirDir);
						mir_free(szMirDir);
					}

					OPENFILENAME ofn = { 0 };
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					wchar_t tmp[MAX_PATH];
					mir_snwprintf(tmp, L"%s (*.wav, *.mp3)%c*.wav;*.mp3%c%c", TranslateT("Sound Files"), 0, 0, 0);
					ofn.lpstrFilter = tmp;
					ofn.hwndOwner = 0;
					ofn.lpstrFile = szTempPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY;
					ofn.lpstrInitialDir = initDir;
					*szTempPath = '\0';
					ofn.lpstrDefExt = L"";

					if (GetOpenFileName(&ofn)) {
						mir_wstrcpy(szSoundFile, szTempPath);

						#ifdef _DEBUG
						logMessage(L"Set sound path", szSoundFile);
						#endif

						// Make path relative
						int result = PathToRelativeW(szTempPath, szSoundFilePath);
						if (result && mir_wstrlen(szSoundFile) > 0)
							SetDlgItemText(hwndDlg, IDC_SNDPATH, szSoundFilePath);

						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

					break;
				}
			}

	default:
		{
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			else
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;
		}

	case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->idFrom == 0)
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					{
						wchar_t tmp[MAX_PATH];

						GetDlgItemText(hwndDlg, IDC_SPLASHPATH, tmp, _countof(tmp));
						db_set_ws(NULL, MODNAME, "Path", tmp);

						GetDlgItemText(hwndDlg, IDC_SNDPATH, tmp, _countof(tmp));
						db_set_ws(NULL, MODNAME, "Sound", tmp);

						GetDlgItemText(hwndDlg, IDC_VERSIONPREFIX, tmp, _countof(tmp));
						db_set_ws(NULL, MODNAME, "VersionPrefix", tmp);
						mir_wstrcpy(szPrefix, tmp);

						GetDlgItemText(hwndDlg, IDC_SHOWTIME, tmp, _countof(tmp));
						db_set_dw(NULL, MODNAME, "TimeToShow", _wtoi(tmp));
						options.showtime = _wtoi(tmp);

						GetDlgItemText(hwndDlg, IDC_FISTEP, tmp, _countof(tmp));
						db_set_dw(NULL, MODNAME, "FadeinSpeed", _wtoi(tmp));
						options.fisteps = _wtoi(tmp);

						GetDlgItemText(hwndDlg, IDC_FOSTEP, tmp, _countof(tmp));
						db_set_dw(NULL, MODNAME, "FadeoutSpeed", _wtoi(tmp));
						options.fosteps = _wtoi(tmp);

						if (IsDlgButtonChecked(hwndDlg, IDC_ACTIVE)) {
							db_set_b(NULL, MODNAME, "Active", 1);
							options.active = 1;
						}
						else {
							db_set_b(NULL, MODNAME, "Active", 0);
							options.active = 0;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_PLAYSND)) {
							db_set_b(NULL, MODNAME, "PlaySound", 1);
							options.playsnd = 1;
							db_set_b(NULL, MODNAME, "InheritGlobalSound", 1);
							options.inheritGS = 1;
						}
						else {
							db_set_b(NULL, MODNAME, "PlaySound", 0);
							options.playsnd = 0;
							db_set_b(NULL, MODNAME, "InheritGlobalSound", 0);
							options.inheritGS = 0;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_PLAYSND) == BST_INDETERMINATE) {
							db_set_b(NULL, MODNAME, "PlaySound", 1);
							options.playsnd = 1;
							db_set_b(NULL, MODNAME, "InheritGlobalSound", 0);
							options.inheritGS = 0;
						}

						/*
						if (IsDlgButtonChecked(hwndDlg, IDC_LOOPSOUND))
						{
						WritePrivateProfileString("Splash","LoopSound","1",szIniFile);
						options.loopsnd = 1;
						}
						else
						{
						WritePrivateProfileString("Splash","LoopSound","0",szIniFile);
						options.loopsnd = 0;
						}
						*/

						if (IsDlgButtonChecked(hwndDlg, IDC_FADEIN)) {
							db_set_b(NULL, MODNAME, "FadeIn", 1);
							options.fadein = 1;
						}
						else {
							db_set_b(NULL, MODNAME, "FadeIn", 0);
							options.fadein = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_FADEOUT)) {
							db_set_b(NULL, MODNAME, "FadeOut", 1);
							options.fadeout = 1;
						}
						else {
							db_set_b(NULL, MODNAME, "FadeOut", 0);
							options.fadeout = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_RANDOM)) {
							db_set_b(NULL, MODNAME, "Random", 1);
							options.random = 1;
						}
						else {
							db_set_b(NULL, MODNAME, "Random", 0);
							options.random = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_SHOWVERSION)) {
							db_set_b(NULL, MODNAME, "ShowVersion", 1);
							options.showversion = 1;
						}
						else {
							db_set_b(NULL, MODNAME, "ShowVersion", 0);
							options.showversion = 0;
						}
						return TRUE;
					}
				}
		}

	case WM_DESTROY:
		break;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.szGroup.a = LPGEN("Skins");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPLASH_OPT);
	odp.szTitle.a = LPGEN("Splash Screen");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
