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

#include "headers.h"

TCHAR szPath2Spash [MAX_PATH], szSoundFilePath[MAX_PATH];

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
	switch (msg)
	{
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
			} else {
				ReadDbConfig();
				TCHAR inBuf[80];
				DBVARIANT dbv = {0};
				if (!db_get_ts(NULL, MODNAME, "Path", &dbv))
				{
					_tcscpy_s(inBuf, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					_tcscpy_s(inBuf, _T("splash\\splash.png"));
				SetDlgItemText(hwndDlg, IDC_SPLASHPATH, inBuf);

				if (!db_get_ts(NULL, MODNAME, "Sound", &dbv))
				{
					_tcscpy_s(inBuf, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					_tcscpy_s(inBuf, _T("sounds\\startup.wav"));
				SetDlgItemText(hwndDlg, IDC_SNDPATH, inBuf);

				if (!db_get_ts(NULL, MODNAME, "VersionPrefix", &dbv))
				{
					_tcscpy_s(inBuf, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					_tcscpy_s(inBuf, _T(""));
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

				SetWindowText(GetDlgItem(hwndDlg, IDC_SHOWTIME), _itot(options.showtime, inBuf, 10));
				SetWindowText(GetDlgItem(hwndDlg, IDC_FISTEP), _itot(options.fisteps, inBuf, 10));
				SetWindowText(GetDlgItem(hwndDlg, IDC_FOSTEP), _itot(options.fosteps, inBuf, 10));

				SendDlgItemMessage(hwndDlg, IDC_SHOWTIME, EM_LIMITTEXT, 5, 0);
			}

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
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
					if (IsDlgButtonChecked(hwndDlg, IDC_FADEIN))
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_FISTEP), true);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FI_SPIN), true);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_FISTEP), false);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FI_SPIN), false);
					}
					if (IsDlgButtonChecked(hwndDlg, IDC_FADEOUT))
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_FOSTEP), true);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FO_SPIN), true);
					}
					else
					{
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
					TCHAR szTempPath[MAX_PATH], initDir[MAX_PATH];
					TCHAR *pos;

					if (Exists(szSplashFile))
					{
						_tcscpy_s(initDir, szSplashFile);
						pos = _tcsrchr(initDir, _T('\\'));
						if(pos != NULL) *pos = 0;
					}
					else
					{
						szMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));
						_tcscpy_s(initDir, szMirDir);
						mir_free(szMirDir);
					}

					OPENFILENAME ofn = {0};
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					TCHAR tmp[MAX_PATH];
					mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.png, *.bmp)%c*.png;*.bmp%c%c"), TranslateT("Graphic files"), 0, 0, 0);
					ofn.lpstrFilter = tmp;
					ofn.hwndOwner = 0;
					ofn.lpstrFile = szTempPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY;
					ofn.lpstrInitialDir = initDir;
					*szTempPath = '\0';
					ofn.lpstrDefExt = _T("");

					if (GetOpenFileName(&ofn)) 
					{
						_tcscpy_s(szSplashFile, szTempPath);

						#ifdef _DEBUG
							logMessage(_T("Set path"), szSplashFile);
						#endif

						// Make path relative
						int result = PathToRelativeT(szTempPath, szPath2Spash);			
						if(result && lstrlen(szPath2Spash) > 0)
						{
							if (options.random)
							{
								TCHAR *pos;
								pos = _tcsrchr(szPath2Spash, _T('\\'));
								if (pos != NULL) 
								{
									*pos = 0;
									_tcscat_s(szPath2Spash, _T("\\"));
								}
							}

							SetWindowText(GetDlgItem(hwndDlg, IDC_SPLASHPATH), szPath2Spash);
						}

						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

				break;
				}

				case IDC_CHOOSESND:
				{
					TCHAR szTempPath[MAX_PATH], initDir[MAX_PATH];
					TCHAR *pos;

					if (Exists(szSoundFile))
					{
						_tcscpy_s(initDir, szSoundFile);
						pos = _tcsrchr(initDir, _T('\\'));
						if(pos != NULL) *pos = 0;
					}
					else
					{
						szMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));
						_tcscpy_s(initDir, szMirDir);
						mir_free(szMirDir);
					}

					OPENFILENAME ofn = {0};
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					TCHAR tmp[MAX_PATH];
					mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.wav, *.mp3)%c*.wav;*.mp3%c%c"), TranslateT("Sound Files"), 0, 0, 0);
					ofn.lpstrFilter = tmp;
					ofn.hwndOwner = 0;
					ofn.lpstrFile = szTempPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY;
					ofn.lpstrInitialDir = initDir;
					*szTempPath = '\0';
					ofn.lpstrDefExt = _T("");

					if (GetOpenFileName(&ofn))
					{
						_tcscpy_s(szSoundFile,szTempPath);

						#ifdef _DEBUG
							logMessage(_T("Set sound path"), szSoundFile);
						#endif

						// Make path relative
						int result = PathToRelativeT(szTempPath, szSoundFilePath);			
						if(result && lstrlen(szSoundFile) > 0)
							SetWindowText(GetDlgItem(hwndDlg, IDC_SNDPATH),szSoundFilePath);

						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

				break;
				}
			}

				default:
				{
					if (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus())
						return 0;
					else
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			break;
		}

		case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->idFrom == 0)
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
					{
						TCHAR tmp[MAX_PATH];

						GetWindowText(GetDlgItem(hwndDlg, IDC_SPLASHPATH), tmp, SIZEOF(tmp));
						db_set_ts(NULL, MODNAME, "Path", tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_SNDPATH), tmp, SIZEOF(tmp));
						db_set_ts(NULL, MODNAME, "Sound", tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_VERSIONPREFIX), tmp, SIZEOF(tmp));
						db_set_ts(NULL, MODNAME, "VersionPrefix", tmp);
						_tcscpy_s(szPrefix, tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_SHOWTIME), tmp, SIZEOF(tmp));
						db_set_dw(NULL, MODNAME, "TimeToShow", _ttoi(tmp));
						options.showtime = _ttoi(tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_FISTEP), tmp, SIZEOF(tmp));
						db_set_dw(NULL, MODNAME, "FadeinSpeed", _ttoi(tmp));
						options.fisteps = _ttoi(tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_FOSTEP), tmp, SIZEOF(tmp));
						db_set_dw(NULL, MODNAME, "FadeoutSpeed", _ttoi(tmp));
						options.fosteps = _ttoi(tmp);

						if (IsDlgButtonChecked(hwndDlg, IDC_ACTIVE))
						{
							db_set_b(NULL, MODNAME, "Active", 1);
							options.active = 1;
						}
						else
						{
							db_set_b(NULL, MODNAME, "Active", 0);
							options.active = 0;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_PLAYSND))
						{
							db_set_b(NULL, MODNAME, "PlaySound", 1);
							options.playsnd = 1;
							db_set_b(NULL, MODNAME, "InheritGlobalSound", 1);
							options.inheritGS = 1;
						}
						else
						{
							db_set_b(NULL, MODNAME, "PlaySound", 0);
							options.playsnd = 0;
							db_set_b(NULL, MODNAME, "InheritGlobalSound", 0);
							options.inheritGS = 0;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_PLAYSND) == BST_INDETERMINATE)
						{
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

						if (IsDlgButtonChecked(hwndDlg, IDC_FADEIN))
						{
							db_set_b(NULL, MODNAME, "FadeIn", 1);
							options.fadein = 1;
						}
						else
						{
							db_set_b(NULL, MODNAME, "FadeIn", 0);
							options.fadein = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_FADEOUT))
						{
							db_set_b(NULL, MODNAME, "FadeOut", 1);
							options.fadeout = 1;
						}
						else
						{
							db_set_b(NULL, MODNAME, "FadeOut", 0);
							options.fadeout = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_RANDOM))
						{
							db_set_b(NULL, MODNAME, "Random", 1);
							options.random = 1;
						}
						else
						{
							db_set_b(NULL, MODNAME, "Random", 0);
							options.random = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_SHOWVERSION))
						{
							db_set_b(NULL, MODNAME, "ShowVersion", 1);
							options.showversion = 1;
						}
						else
						{
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
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszGroup = LPGEN("Skins");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPLASH_OPT);
	odp.pszTitle = LPGEN("Splash Screen");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
