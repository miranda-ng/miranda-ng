/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"
#include "file.h"

static void SetControlToUnixTime(HWND hwndDlg, UINT idCtrl, time_t unixTime)
{
	LARGE_INTEGER liFiletime;
	FILETIME filetime;
	SYSTEMTIME st;
	char szTime[64], szDate[64], szOutput[128];

	liFiletime.QuadPart = (11644473600ll + (__int64)unixTime) * 10000000;
	filetime.dwHighDateTime = liFiletime.HighPart;
	filetime.dwLowDateTime = liFiletime.LowPart;
	FileTimeToSystemTime(&filetime, &st);
	GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, szTime, _countof(szTime));
	GetDateFormatA(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szDate, _countof(szDate));
	mir_snprintf(szOutput, "%s %s", szDate, szTime);
	SetDlgItemTextA(hwndDlg, idCtrl, szOutput);
}

#define C_CONTEXTMENU  0
#define C_PROPERTIES   1
// not defined in VC++ 6.0 SE
#ifndef CMF_EXTENDEDVERBS
#define CMF_EXTENDEDVERBS 0x00000100
#endif
static void DoAnnoyingShellCommand(HWND hwnd, const wchar_t *szFilename, int cmd, POINT *ptCursor)
{
	IShellFolder *pDesktopFolder;
	if (SHGetDesktopFolder(&pDesktopFolder) == NOERROR) {
		ITEMIDLIST *pCurrentIdl;
		wchar_t *wszFilename = (LPWSTR)szFilename;

		if (pDesktopFolder->ParseDisplayName(nullptr, nullptr, wszFilename, nullptr, &pCurrentIdl, nullptr) == NOERROR) {
			if (pCurrentIdl->mkid.cb) {
				ITEMIDLIST *pidl, *pidlNext, *pidlFilename;
				IShellFolder *pFileFolder;

				for (pidl = pCurrentIdl;;) {
					pidlNext = (ITEMIDLIST *)((uint8_t*)pidl + pidl->mkid.cb);
					if (pidlNext->mkid.cb == 0) {
						pidlFilename = (ITEMIDLIST *)CoTaskMemAlloc(pidl->mkid.cb + sizeof(pidl->mkid.cb));
						memcpy(pidlFilename, pidl, pidl->mkid.cb + sizeof(pidl->mkid.cb));
						pidl->mkid.cb = 0;
						break;
					}
					pidl = pidlNext;
				}
				if (pDesktopFolder->BindToObject(pCurrentIdl, nullptr, IID_IShellFolder, (void **)&pFileFolder) == NOERROR) {
					IContextMenu *pContextMenu;
					if (pFileFolder->GetUIObjectOf(nullptr, 1, (LPCITEMIDLIST *)&pidlFilename, IID_IContextMenu, nullptr, (void **)&pContextMenu) == NOERROR) {
						switch (cmd) {
						case C_PROPERTIES:
							{
								CMINVOKECOMMANDINFO ici = { 0 };
								ici.cbSize = sizeof(ici);
								ici.hwnd = hwnd;
								ici.lpVerb = "properties";
								ici.nShow = SW_SHOW;
								pContextMenu->InvokeCommand(&ici);
							}
							break;

						case C_CONTEXTMENU:
							HMENU hMenu = CreatePopupMenu();
							if (SUCCEEDED(pContextMenu->QueryContextMenu(hMenu, 0, 1000, 65535, (GetKeyState(VK_SHIFT) & 0x8000 ? CMF_EXTENDEDVERBS : 0) | CMF_NORMAL))) {
								int ret = TrackPopupMenu(hMenu, TPM_RETURNCMD, ptCursor->x, ptCursor->y, 0, hwnd, nullptr);
								if (ret) {
									CMINVOKECOMMANDINFO ici = { 0 };
									ici.cbSize = sizeof(ici);
									ici.hwnd = hwnd;
									ici.lpVerb = MAKEINTRESOURCEA(ret - 1000);
									ici.nShow = SW_SHOW;
									pContextMenu->InvokeCommand(&ici);
								}
							}
							DestroyMenu(hMenu);
							break;
						}
						pContextMenu->Release();
					}
					pFileFolder->Release();
				}
				CoTaskMemFree(pidlFilename);
			}
			CoTaskMemFree(pCurrentIdl);
		}
		pDesktopFolder->Release();
	}
}

static LRESULT CALLBACK IconCtrlSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PROTOFILETRANSFERSTATUS *pft = (PROTOFILETRANSFERSTATUS *)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	switch (msg) {
	case WM_LBUTTONDBLCLK:
		ShellExecute(hwnd, nullptr, pft->szCurrentFile.w, nullptr, nullptr, SW_SHOW);
		break;
	case WM_RBUTTONUP:
		POINT pt;
		pt.x = (short)LOWORD(lParam); pt.y = (short)HIWORD(lParam);
		ClientToScreen(hwnd, &pt);
		DoAnnoyingShellCommand(hwnd, pft->szCurrentFile.w, C_CONTEXTMENU, &pt);
		return 0;
	}
	return mir_callNextSubclass(hwnd, IconCtrlSubclassProc, msg, wParam, lParam);
}

struct loadiconsstartinfo
{
	HWND hwndDlg;
	wchar_t *szFilename;
};

void __cdecl LoadIconsAndTypesThread(void *param)
{
	loadiconsstartinfo *info = (loadiconsstartinfo *)param;
	SHFILEINFO fileInfo;

	if (SHGetFileInfo(info->szFilename, 0, &fileInfo, sizeof(fileInfo), SHGFI_TYPENAME | SHGFI_ICON | SHGFI_LARGEICON)) {
		wchar_t szExtension[64], szIconFile[MAX_PATH];

		wchar_t *pszFilename = wcsrchr(info->szFilename, '\\');
		if (pszFilename == nullptr)
			pszFilename = info->szFilename;

		wchar_t *pszExtension = wcsrchr(pszFilename, '.');
		if (pszExtension)
			mir_wstrncpy(szExtension, pszExtension + 1, _countof(szExtension));
		else {
			pszExtension = L".";
			szExtension[0] = '\0';
		}
		CharUpper(szExtension);
		if (fileInfo.szTypeName[0] == '\0')
			mir_snwprintf(fileInfo.szTypeName, TranslateT("%s file"), szExtension);
		SetDlgItemText(info->hwndDlg, IDC_EXISTINGTYPE, fileInfo.szTypeName);
		SetDlgItemText(info->hwndDlg, IDC_NEWTYPE, fileInfo.szTypeName);
		SendDlgItemMessage(info->hwndDlg, IDC_EXISTINGICON, STM_SETICON, (WPARAM)fileInfo.hIcon, 0);
		szIconFile[0] = '\0';
		if (!mir_wstrcmp(szExtension, L"EXE"))
			SRFile_GetRegValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons", L"2", szIconFile, _countof(szIconFile));
		else {
			wchar_t szTypeName[MAX_PATH];
			if (SRFile_GetRegValue(HKEY_CLASSES_ROOT, pszExtension, NULL, szTypeName, _countof(szTypeName))) {
				mir_wstrcat(szTypeName, L"\\DefaultIcon");
				if (SRFile_GetRegValue(HKEY_CLASSES_ROOT, szTypeName, NULL, szIconFile, _countof(szIconFile))) {
					if (wcsstr(szIconFile, L"%1"))
						SRFile_GetRegValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons", L"0", szIconFile, _countof(szIconFile));
					else szIconFile[0] = '\0';
				}
			}
		}

		if (szIconFile[0]) {
			wchar_t *pszComma = wcsrchr(szIconFile, ',');
			int iconIndex;
			if (pszComma) {
				iconIndex = _wtoi(pszComma + 1);
				*pszComma = '\0';
			}
			else
				iconIndex = 0;
			HICON hIcon = ExtractIcon(g_plugin.getInst(), szIconFile, iconIndex);
			if (hIcon)
				fileInfo.hIcon = hIcon;
		}
		SendDlgItemMessage(info->hwndDlg, IDC_NEWICON, STM_SETICON, (WPARAM)fileInfo.hIcon, 0);
	}
	mir_free(info->szFilename);
	mir_free(info);
}

INT_PTR CALLBACK DlgProcFileExists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PROTOFILETRANSFERSTATUS *fts = (PROTOFILETRANSFERSTATUS *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			wchar_t szSize[64];
			struct _stati64 statbuf;
			struct TDlgProcFileExistsParam *dat = (struct TDlgProcFileExistsParam *)lParam;

			SetPropA(hwndDlg, "Miranda.Preshutdown", HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwndDlg, M_PRESHUTDOWN));
			SetPropA(hwndDlg, "Miranda.ParentWnd", dat->hwndParent);

			fts = (PROTOFILETRANSFERSTATUS *)mir_alloc(sizeof(PROTOFILETRANSFERSTATUS));
			CopyProtoFileTransferStatus(fts, dat->fts);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)fts);
			SetDlgItemText(hwndDlg, IDC_FILENAME, fts->szCurrentFile.w);
			SetControlToUnixTime(hwndDlg, IDC_NEWDATE, fts->currentFileTime);
			GetSensiblyFormattedSize(fts->currentFileSize, szSize, _countof(szSize), 0, 1, NULL);
			SetDlgItemText(hwndDlg, IDC_NEWSIZE, szSize);

			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_EXISTINGICON), IconCtrlSubclassProc);

			HWND hwndFocus = GetDlgItem(hwndDlg, IDC_RESUME);
			if (_wstat64(fts->szCurrentFile.w, &statbuf) == 0) {
				SetControlToUnixTime(hwndDlg, IDC_EXISTINGDATE, statbuf.st_mtime);
				GetSensiblyFormattedSize(statbuf.st_size, szSize, _countof(szSize), 0, 1, NULL);
				SetDlgItemText(hwndDlg, IDC_EXISTINGSIZE, szSize);
				if (statbuf.st_size > (int)fts->currentFileSize) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_RESUME), FALSE);
					hwndFocus = GetDlgItem(hwndDlg, IDC_OVERWRITE);
				}
			}

			loadiconsstartinfo *lisi = (loadiconsstartinfo *)mir_alloc(sizeof(loadiconsstartinfo));
			lisi->hwndDlg = hwndDlg;
			lisi->szFilename = mir_wstrdup(fts->szCurrentFile.w);
			//can be a little slow, so why not?
			mir_forkthread(LoadIconsAndTypesThread, lisi);
			SetFocus(hwndFocus);
			SetWindowLongPtr(hwndFocus, GWL_STYLE, GetWindowLongPtr(hwndFocus, GWL_STYLE) | BS_DEFPUSHBUTTON);
		}
		return FALSE;

	case WM_COMMAND:
		{
			PROTOFILERESUME pfr = {};
			switch (LOWORD(wParam)) {
			case IDC_OPENFILE:
				ShellExecute(hwndDlg, NULL, fts->szCurrentFile.w, NULL, NULL, SW_SHOW);
				return FALSE;

			case IDC_OPENFOLDER:
				{
					wchar_t szFile[MAX_PATH];
					mir_wstrncpy(szFile, fts->szCurrentFile.w, _countof(szFile));
					wchar_t *pszLastBackslash = wcsrchr(szFile, '\\');
					if (pszLastBackslash)
						*pszLastBackslash = '\0';
					ShellExecute(hwndDlg, NULL, szFile, NULL, NULL, SW_SHOW);
				}
				return FALSE;
			case IDC_PROPERTIES:
				DoAnnoyingShellCommand(hwndDlg, fts->szCurrentFile.w, C_PROPERTIES, NULL);
				return FALSE;
			case IDC_RESUME:
				pfr.action = FILERESUME_RESUME;
				break;
			case IDC_RESUMEALL:
				pfr.action = FILERESUME_RESUMEALL;
				break;
			case IDC_OVERWRITE:
				pfr.action = FILERESUME_OVERWRITE;
				break;
			case IDC_OVERWRITEALL:
				pfr.action = FILERESUME_OVERWRITEALL;
				break;

			case IDC_AUTORENAME:
				pfr.action = FILERESUME_RENAMEALL;
				break;

			case IDC_SAVEAS:
				wchar_t str[MAX_PATH];
				mir_wstrncpy(str, fts->szCurrentFile.w, _countof(str));

				wchar_t filter[512];
				mir_snwprintf(filter, L"%s (*)%c*%c", TranslateT("All files"), 0, 0);
				{
					OPENFILENAME ofn = {};
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
					ofn.lpstrFilter = filter;
					ofn.lpstrFile = str;
					ofn.nMaxFile = _countof(str);
					ofn.nMaxFileTitle = MAX_PATH;
					if (!GetSaveFileName(&ofn))
						return FALSE;

					pfr.szFilename = mir_wstrdup(str);
					pfr.action = FILERESUME_RENAME;
				}
				break;

			case IDC_SKIP:
				pfr.action = FILERESUME_SKIP;
				break;

			case IDCANCEL:
				pfr.action = FILERESUME_CANCEL;
				break;

			default:
				return FALSE;
			}

			PostMessage((HWND)GetPropA(hwndDlg, "Miranda.ParentWnd"), M_FILEEXISTSDLGREPLY, (WPARAM)mir_wstrdup(fts->szCurrentFile.w), (LPARAM)new PROTOFILERESUME(pfr));
			DestroyWindow(hwndDlg);
		}
		break;

	case WM_CLOSE:
		PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, IDCANCEL));
		break;

	case M_PRESHUTDOWN:
		PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		break;

	case WM_DESTROY:
		UnhookEvent(GetPropA(hwndDlg, "Miranda.Preshutdown")); // GetProp() will return NULL if it couldnt find anything
		RemovePropA(hwndDlg, "Miranda.Preshutdown");
		RemovePropA(hwndDlg, "Miranda.ParentWnd");
		DestroyIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_EXISTINGICON, STM_GETICON, 0, 0));
		DestroyIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_NEWICON, STM_GETICON, 0, 0));
		FreeProtoFileTransferStatus(fts);
		mir_free(fts);
		break;
	}
	return FALSE;
}
