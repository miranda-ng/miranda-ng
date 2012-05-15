/*
Splash Screen Plugin for Miranda-IM (www.miranda-im.org)
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

File name      : $URL: http://svn.miranda.im/mainrepo/splashscreen/trunk/src/options.cpp $
Revision       : $Rev: 951 $
Last change on : $Date: 2007-10-16 18:46:53 +0400 (Вт, 16 окт 2007) $
Last change by : $Author: Thief $

DESCRIPTION: Options dialog handling code

*/

#include "headers.h"

TCHAR szPath2Spash [MAX_PATH], szSoundFilePath[MAX_PATH];

// Reads values from db
void ReadIniConfig()
{
	options.active = DBGetContactSettingByte(NULL, MODNAME, "Active", 1);
	options.playsnd = DBGetContactSettingByte(NULL, MODNAME, "PlaySound", 0);
	options.fadein = DBGetContactSettingByte(NULL, MODNAME, "FadeIn", 1);
	options.fadeout = DBGetContactSettingByte(NULL, MODNAME, "FadeOut", 1);
	options.showtime = DBGetContactSettingDword(NULL, MODNAME, "TimeToShow", 2000);
	options.fisteps = DBGetContactSettingDword(NULL, MODNAME, "FadeinSpeed", 5);
	options.fosteps = DBGetContactSettingDword(NULL, MODNAME, "FadeoutSpeed", 5);
	options.inheritGS = DBGetContactSettingByte(NULL, MODNAME, "InheritGlobalSound", 1);
	options.showversion = DBGetContactSettingByte(NULL, MODNAME, "ShowVersion", 0);
	options.random = DBGetContactSettingByte(NULL, MODNAME, "Random", 0);
	options.runonce = DBGetContactSettingByte(NULL, MODNAME, "DisableAfterStartup", 0);
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
			if (!png2dibavail)
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_PNG2DIBWARN), SW_SHOW);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVE), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RANDOM), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPLASHPATH), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSESPLASH), false);
			}
			ReadIniConfig();
			TCHAR inBuf[80];
			DBVARIANT dbv = {0};
			DBGetContactSettingTString(NULL, MODNAME, "Path", &dbv);
			if (lstrcmp(dbv.ptszVal, NULL) == 0)
			{
				_tcscpy_s(inBuf, _T("splash\\splash.png"));
				DBFreeVariant(&dbv);
			}
			else
				_tcscpy_s(inBuf, dbv.ptszVal);
			dbv.ptszVal = NULL;
			SetWindowText(GetDlgItem(hwndDlg, IDC_SPLASHPATH),inBuf);
			DBGetContactSettingTString(NULL, MODNAME, "Sound", &dbv);
			if (lstrcmp(dbv.ptszVal, NULL) == 0)
			{
				_tcscpy_s(inBuf, _T("sounds\\startup.wav"));
				DBFreeVariant(&dbv);
			}
			else
				_tcscpy_s(inBuf, dbv.ptszVal);
			dbv.ptszVal = NULL;
			SetWindowText(GetDlgItem(hwndDlg, IDC_SNDPATH),inBuf);
			DBGetContactSettingTString(NULL, MODNAME, "VersionPrefix", &dbv);
			if (lstrcmp(dbv.ptszVal, NULL) == 0)
			{
				_tcscpy_s(inBuf, _T(""));
				DBFreeVariant(&dbv);
			}
			else
				_tcscpy_s(inBuf, dbv.ptszVal);
			dbv.ptszVal = NULL;
			SetWindowText(GetDlgItem(hwndDlg, IDC_VERSIONPREFIX), inBuf);
			if (options.active)	CheckDlgButton(hwndDlg, IDC_ACTIVE, BST_CHECKED);
			if (options.playsnd && !options.inheritGS) CheckDlgButton(hwndDlg, IDC_PLAYSND, BST_INDETERMINATE);
			else if (options.playsnd) CheckDlgButton(hwndDlg, IDC_PLAYSND, BST_CHECKED);
			//if (options.loopsnd) CheckDlgButton(hwndDlg, IDC_LOOPSOUND, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOOPSOUND), false);
			if (options.fadein)	CheckDlgButton(hwndDlg, IDC_FADEIN, BST_CHECKED);
			if (options.fadeout) CheckDlgButton(hwndDlg, IDC_FADEOUT, BST_CHECKED);
			if (options.random)	CheckDlgButton(hwndDlg, IDC_RANDOM, BST_CHECKED);
			if (options.showversion) CheckDlgButton(hwndDlg, IDC_SHOWVERSION, BST_CHECKED);

			SetWindowText(GetDlgItem(hwndDlg, IDC_SHOWTIME), _itot(options.showtime, inBuf, 10));
			SetWindowText(GetDlgItem(hwndDlg, IDC_FISTEP), _itot(options.fisteps, inBuf, 10));
			SetWindowText(GetDlgItem(hwndDlg, IDC_FOSTEP), _itot(options.fosteps, inBuf, 10));

			SendDlgItemMessage(hwndDlg, IDC_SHOWTIME, EM_LIMITTEXT, 5, 0);
			/*
			SendDlgItemMessage(hwndDlg, IDC_ST_SPIN, UDM_SETRANGE32, 0, 20000);
			SendDlgItemMessage(hwndDlg, IDC_FI_SPIN, UDM_SETRANGE32, 1, 7);
			SendDlgItemMessage(hwndDlg, IDC_FO_SPIN, UDM_SETRANGE32, 1, 7);
			*/

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
						lstrcpy(initDir, szSplashFile);
						pos = _tcsrchr(initDir, _T('\\'));
						if(pos != NULL) *pos = 0;
					}
					else
					{
						szMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));
						lstrcpy(initDir, szMirDir);
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
						lstrcpy(szSplashFile, szTempPath);

						#ifdef _DEBUG
							logMessage(_T("Set path"), szSplashFile);
						#endif

						// Make path relative
						int result = CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)szTempPath, (LPARAM)szPath2Spash);			

						if(result && lstrlen(szPath2Spash) > 0)
						{
							if (options.random)
							{
								TCHAR *pos;
								pos = _tcsrchr(szPath2Spash, _T('\\'));
								if (pos != NULL) 
								{
									*pos = 0;
									lstrcat(szPath2Spash, _T("\\"));
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
						lstrcpy(initDir, szSoundFile);
						pos = _tcsrchr(initDir, _T('\\'));
						if(pos != NULL) *pos = 0;
					}
					else
					{
						szMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));
						lstrcpy(initDir, szMirDir);
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
						lstrcpy(szSoundFile,szTempPath);

						#ifdef _DEBUG
							logMessage(_T("Set sound path"), szSoundFile);
						#endif

						// Make path relative
						int result = CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)szTempPath, (LPARAM)szSoundFilePath);			

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

						GetWindowText(GetDlgItem(hwndDlg, IDC_SPLASHPATH), tmp, MAX_PATH);
						DBWriteContactSettingTString(NULL, MODNAME, "Path", tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_SNDPATH), tmp, MAX_PATH);
						DBWriteContactSettingTString(NULL, MODNAME, "Sound", tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_VERSIONPREFIX), tmp, MAX_PATH);
						DBWriteContactSettingTString(NULL, MODNAME, "VersionPrefix", tmp);
						lstrcpy(szPrefix, tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_SHOWTIME), tmp, MAX_PATH);
						DBWriteContactSettingDword(NULL, MODNAME, "TimeToShow", _ttoi(tmp));
						options.showtime = _ttoi(tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_FISTEP), tmp, MAX_PATH);
						DBWriteContactSettingDword(NULL, MODNAME, "FadeinSpeed", _ttoi(tmp));
						options.fisteps = _ttoi(tmp);

						GetWindowText(GetDlgItem(hwndDlg, IDC_FOSTEP), tmp, MAX_PATH);
						DBWriteContactSettingDword(NULL, MODNAME, "FadeoutSpeed", _ttoi(tmp));
						options.fosteps = _ttoi(tmp);

						if (IsDlgButtonChecked(hwndDlg, IDC_ACTIVE))
						{
							DBWriteContactSettingByte(NULL, MODNAME, "Active", 1);
							options.active = 1;
						}
						else
						{
							DBWriteContactSettingByte(NULL, MODNAME, "Active", 0);
							options.active = 0;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_PLAYSND))
						{
							DBWriteContactSettingByte(NULL, MODNAME, "PlaySound", 1);
							options.playsnd = 1;
							DBWriteContactSettingByte(NULL, MODNAME, "InheritGlobalSound", 1);
							options.inheritGS = 1;
						}
						else
						{
							DBWriteContactSettingByte(NULL, MODNAME, "PlaySound", 0);
							options.playsnd = 0;
							DBWriteContactSettingByte(NULL, MODNAME, "InheritGlobalSound", 0);
							options.inheritGS = 0;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_PLAYSND) == BST_INDETERMINATE)
						{
							DBWriteContactSettingByte(NULL, MODNAME, "PlaySound", 1);
							options.playsnd = 1;
							DBWriteContactSettingByte(NULL, MODNAME, "InheritGlobalSound", 0);
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
							DBWriteContactSettingByte(NULL, MODNAME, "FadeIn", 1);
							options.fadein = 1;
						}
						else
						{
							DBWriteContactSettingByte(NULL, MODNAME, "FadeIn", 0);
							options.fadein = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_FADEOUT))
						{
							DBWriteContactSettingByte(NULL, MODNAME, "FadeOut", 1);
							options.fadeout = 1;
						}
						else
						{
							DBWriteContactSettingByte(NULL, MODNAME, "FadeOut", 0);
							options.fadeout = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_RANDOM))
						{
							DBWriteContactSettingByte(NULL, MODNAME, "Random", 1);
							options.random = 1;
						}
						else
						{
							DBWriteContactSettingByte(NULL, MODNAME, "Random", 0);
							options.random = 0;
						}
						if (IsDlgButtonChecked(hwndDlg, IDC_SHOWVERSION))
						{
							DBWriteContactSettingByte(NULL, MODNAME, "ShowVersion", 1);
							options.showversion = 1;
						}
						else
						{
							DBWriteContactSettingByte(NULL, MODNAME, "ShowVersion", 0);
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
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.ptszGroup = _T("Skins");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPLASH_OPT);
	odp.ptszTitle = _T("Splash Screen");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_TCHAR | ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);
	return 0;
}
