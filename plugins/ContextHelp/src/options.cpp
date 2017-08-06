/*
Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

typedef PLUGININFOEX* (__cdecl *pfnMirandaPluginInfo)(DWORD);

WORD settingAutoTipDelay;      /* auto show help after those milliseconds, 0 if disabled */
extern HINSTANCE hInst;
static HANDLE hHookOptInit;
static HWND hwndLangOpt;

/**************************** LANGUAGE OPTS ***********************/

/* these are only there for a short time to
* debug those radio buttons an Win9x */
#define BOX(str)        BOX2("%s (err:%i)", str, GetLastError())
#define BOX2(fmt,p1,p2) { char str[256]; mir_snprintf(str, fmt, p1, p2); MessageBoxA(NULL, str, "dbg", 0); }

// ImageList_Destroy() the return value
// refresh on WM_THEMECHANGED
static HIMAGELIST CreateRadioImages(COLORREF clrBk, COLORREF clrText)
{
	/* draw bitmap */
	HDC hdcScreen = GetDC(NULL);
	if (hdcScreen == NULL)
		return NULL;

	HIMAGELIST himl = NULL;
	HDC hdc = CreateCompatibleDC(NULL); // compatible to screen
	if (hdc != NULL) {
		SIZE size = { GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON) };
		RECT rc;
		SetRect(&rc, 0, 0, 2 * size.cx, size.cy);
		HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, rc.right, rc.bottom);
		if (hbm != NULL) {
			HBITMAP hbmPrev = (HBITMAP)SelectObject(hdc, hbm);
			if (hbmPrev != NULL) { // error on select?
				HTHEME hTheme = OpenThemeData(NULL, L"Button");
				SetRect(&rc, 0, 0, size.cx, size.cy);
				/* unchecked */
				if (!DrawThemeBackground(hTheme, hdc, BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL, &rc, NULL)) {
					/* checked */
					OffsetRect(&rc, size.cx, 0);
					if (!DrawThemeBackground(hTheme, hdc, BP_RADIOBUTTON, RBS_CHECKEDNORMAL, &rc, NULL))
						himl = ImageList_Create(size.cx, size.cy, ILC_COLOR32 | ILC_MASK, 3, 0);
				}
				CloseThemeData(hTheme);

				/* the classic way */
				if (himl == NULL) {
					HBRUSH hbrBk = CreateSolidBrush(clrBk);
					if (hbrBk != NULL) {
						FillRect(hdc, &rc, hbrBk);
						DeleteObject(hbrBk);
						HDC hdcMono = CreateCompatibleDC(hdc);
						if (hdcMono != NULL) {
							HBITMAP hbmMono = CreateBitmap(rc.right, rc.bottom, 1, 1, NULL);
							if (hbmMono != NULL) {
								HBITMAP hbmPrevMono = (HBITMAP)SelectObject(hdcMono, hbmMono);
								if (hbmPrevMono != NULL) { /* error on select? */
									/* draws a black-and-white mask (see docs)
									* we need to colorize it using BitBlt with text and background color */
									COLORREF clrPrevText = SetTextColor(hdc, clrText);
									COLORREF clrPrevBk = SetBkColor(hdc, clrBk);
									/* check mark is slightly smaller than icon size */
									RECT rcRadio;
									SetRect(&rcRadio, 0, 0, GetSystemMetrics(SM_CXMENUCHECK), GetSystemMetrics(SM_CYMENUCHECK));
									if (rcRadio.right>size.cx)
										rcRadio.right = size.cx;
									if (rcRadio.bottom>size.cy)
										rcRadio.bottom = size.cy;
									SetRect(&rc, ((size.cx - rcRadio.right) / 2) + 1, ((size.cy - rcRadio.bottom) / 2) + 1, rcRadio.right + 1, rcRadio.bottom + 1);
									/* unchecked */
									if (BitBlt(hdcMono, 0, 0, rcRadio.right, rcRadio.bottom, NULL, 0, 0, WHITENESS)) { /* white back */
										if (DrawFrameControl(hdcMono, &rcRadio, DFC_BUTTON, DFCS_BUTTONRADIO | DFCS_FLAT)) {
											if (BitBlt(hdc, rc.left, rc.top, rcRadio.right, rcRadio.bottom, hdcMono, 0, 0, SRCCOPY | NOMIRRORBITMAP)) {
												/* checked */
												OffsetRect(&rc, size.cx, 0);
												if (BitBlt(hdcMono, 0, 0, rcRadio.right, rcRadio.bottom, NULL, 0, 0, WHITENESS)) {/* white back */
													if (DrawFrameControl(hdcMono, &rcRadio, DFC_BUTTON, DFCS_BUTTONRADIO | DFCS_FLAT | DFCS_CHECKED)) {
														if (BitBlt(hdc, rc.left, rc.top, rcRadio.right, rcRadio.bottom, hdcMono, 0, 0, SRCCOPY | NOMIRRORBITMAP)) {
															himl = ImageList_Create(size.cx, size.cy, ILC_COLOR | ILC_MASK, 3, 0);
															if (himl == NULL)
																BOX("img list creation failed");
														}
													}
													else
														BOX("second DrawFrameControl() failed");
												}
												else
													BOX("second BitBlt() failed");
											}
											else
												BOX("intermediate BitBlt() failed");
										}
										else
											BOX("DrawFrameControl() failed");
									}
									else
										BOX("first BitBlt() failed");
									/* restore */
									SetBkColor(hdc, clrPrevBk);
									SetTextColor(hdc, clrPrevText);
									SelectObject(hdcMono, hbmPrevMono);
								}
								else
									BOX("hbmPrevMono==NULL");
								DeleteObject(hbmMono);
							}
							else
								BOX("hbmMono==NULL");
							DeleteDC(hdcMono);
						}
						else
							BOX("hdcMono==NULL");
					}
				}
				SelectObject(hdc, hbmPrev);
				/* create imagelist */
				if (himl != NULL) {
					if (ImageList_AddMasked(himl, hbm, clrBk) == -1)
						BOX("addmasked failed");
				}
				else
					BOX("Win9x: drawing code not reached");
			}
			DeleteObject(hbm);
		}
		DeleteDC(hdc);
	}
	ReleaseDC(NULL, hdcScreen);

	return himl;
}

static void CleanupPluginName(char *szShortName)
{
	char *p;
	/* strip-off anything in brackets */
	for (p = szShortName; *p != '\0'; ++p)
		if (*p == '(' || *p == '[') {
			*p = '\0';
			break;
		}
	/* remove trailing space */
	int len = lstrlenA(szShortName);
	while (szShortName[0] != '\0' && szShortName[len - 1] == ' ')
		szShortName[--len] = 0;
}

static void DisplayNotIncludedPlugins(HWND hwndListBox, const HELPPACK_INFO *pack)
{
	/* enum plugins */
	TCHAR szDir[MAX_PATH];
	if (GetModuleFileName(NULL, szDir, _countof(szDir))) {
		TCHAR *p = _tcsrchr(szDir, _T('\\'));
		if (p != NULL)
			*p = _T('\0');
		TCHAR szSearch[MAX_PATH];
		mir_sntprintf(szSearch, L"%s\\Plugins\\*.dll", szDir);
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile(szSearch, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			DWORD mirandaVersion = CallService(MS_SYSTEM_GETVERSION, 0, 0);
			SendMessage(hwndListBox, LB_SETLOCALE, Langpack_GetDefaultLocale(), 0); /* for sort order */
			SendMessage(hwndListBox, LB_INITSTORAGE, 128, lstrlenA(pack->szPluginsIncluded)); /* speed up */
			do {
				if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					continue;
				if ((lstrlen(wfd.cFileName) < 4) || (wfd.cFileName[lstrlen(wfd.cFileName) - 4] != _T('.')))
					continue;
				/* file name */
				lstrcpy(szSearch, wfd.cFileName); /* buffer safe */
				p = _tcsrchr(szSearch, _T('.'));
				if (p != NULL)
					*p = _T('\0');
				char cFileNameA[MAX_PATH];
				cFileNameA[0] = '\0';
				WideCharToMultiByte(CP_ACP, 0, szSearch, -1, cFileNameA, sizeof(cFileNameA), NULL, NULL);
				if (IsPluginIncluded(pack, cFileNameA))
					continue;

				/* friendly name of the plugin */
				mir_sntprintf(szSearch, L"%s\\Plugins\\%s", szDir, wfd.cFileName);
				HMODULE hModule = GetModuleHandle(szSearch);
				BOOL fNeedsFree = (hModule == NULL);
				if (hModule == NULL) {
					hModule = LoadLibrary(szSearch);
					if (hModule == NULL)
						continue;
				}

				/* plugin info */
				pfnMirandaPluginInfo pFunc = (pfnMirandaPluginInfo)GetProcAddress(hModule, "MirandaPluginInfoEx");
				if (pFunc != NULL) { /* both structs have the same header */
					PLUGININFOEX *pluginInfo = pFunc(mirandaVersion);
					if (pluginInfo != NULL && pluginInfo->cbSize >= sizeof(PLUGININFOEX) && pluginInfo->shortName != NULL) {
						char buf[128];
						lstrcpynA(buf, pluginInfo->shortName, sizeof(buf)); /* buffer safe */
						CleanupPluginName(buf);

						TCHAR buf2[128];
						mir_sntprintf(buf2, L"%hs (%s)", buf, CharLower(wfd.cFileName));
						SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM)buf2);
					}
				}
				if (fNeedsFree)
					FreeLibrary(hModule);
			} while (FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
	}
	/* all are included? */
	if (!SendMessage(hwndListBox, LB_GETCOUNT, 0, 0))
		SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM)TranslateT("All installed plugins are included."));
}

// pack is allowed to be NULL
static void DisplayPackInfo(HWND hwndDlg, const HELPPACK_INFO *pack)
{
	/* show/hide controls */
	if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_NOPACK)) == (pack != NULL)) {
		const int controls[] = { IDC_LANGNOTINCLUDEDLABEL, IDC_LANGNOTINCLUDED, IDC_LANGDATELABEL, IDC_LANGDATE,
			IDC_LANGLOCALELABEL, IDC_LANGLOCALE, IDC_LANGVERSIONLABEL, IDC_LANGVERSION,
			IDC_LANGMODUSINGLABEL, IDC_LANGMODUSING, IDC_LANGAUTHORSLABEL, IDC_LANGAUTHORS,
			IDC_LANGEMAILLABEL, IDC_LANGEMAIL };
		for (int i = 0; i < _countof(controls); i++)
			ShowWindow(GetDlgItem(hwndDlg, controls[i]), (pack != NULL) ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_NOPACK), (pack != NULL) ? SW_HIDE : SW_SHOW);
		SetDlgItemText(hwndDlg, IDC_LANGINFOFRAME, NULL);
	}
	if (pack == NULL)
		return;

	/* compute not-included from included list */
	SendDlgItemMessage(hwndDlg, IDC_LANGNOTINCLUDED, LB_RESETCONTENT, 0, 0);
	DisplayNotIncludedPlugins(GetDlgItem(hwndDlg, IDC_LANGNOTINCLUDED), pack);
	/* locale string */
	if (!(pack->flags&HPF_NOLOCALE)) {
		TCHAR szLocaleName[128];
		szLocaleName[0] = _T('\0');
		/* can't use LOCALE_SNAME as it is not present on pre WinVista */
		if (!GetLocaleInfo(pack->Locale, LOCALE_SISO639LANGNAME, szLocaleName, _countof(szLocaleName))) { /* Win98/NT4+ */
			if (!GetLocaleInfo(pack->Locale, LOCALE_SLANGUAGE, szLocaleName, _countof(szLocaleName))) /* not unique! */
				szLocaleName[0] = _T('\0');
		}
		else {
			if (GetLocaleInfo(pack->Locale, LOCALE_SISO3166CTRYNAME, &szLocaleName[3], _countof(szLocaleName) - 3)) /* Win98/NT4+ */
				szLocaleName[2] = _T('-');
		}
		/* add some note if its incompatible */
		if (szLocaleName[0]) {
			if (!IsValidLocale(pack->Locale, LCID_INSTALLED)) {
				TCHAR *pszIncompat;
				pszIncompat = TranslateT("(incompatible)");
				szLocaleName[_countof(szLocaleName) - lstrlen(pszIncompat) - 1] = 0;
				lstrcat(lstrcat(szLocaleName, L" "), pszIncompat); /* buffer safe */
			}
			SetDlgItemText(hwndDlg, IDC_LANGLOCALE, szLocaleName);
		}
		else
			SetDlgItemText(hwndDlg, IDC_LANGLOCALE, TranslateT("Unknown"));
	}
	else
		SetDlgItemText(hwndDlg, IDC_LANGLOCALE, TranslateT("Current"));

	/* file date */
	{
		SYSTEMTIME stFileDate;
		TCHAR szDate[128];
		szDate[0] = _T('\0');
		if (FileTimeToSystemTime(&pack->ftFileDate, &stFileDate))
			GetDateFormat(Langpack_GetDefaultLocale(), DATE_SHORTDATE, &stFileDate, NULL, szDate, _countof(szDate));
		SetDlgItemText(hwndDlg, IDC_LANGDATE, szDate);
	}

	/* version */
	SetDlgItemTextA(hwndDlg, IDC_LANGVERSION, pack->szVersion);
	if (pack->szVersion[0] && pack->szFLName[0]) {
		if (!IsWindowVisible(GetDlgItem(hwndDlg, IDC_LANGVERSIONLABEL))) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_LANGVERSIONLABEL), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_LANGVERSION), SW_SHOW);
		}
	}
	else {
		ShowWindow(GetDlgItem(hwndDlg, IDC_LANGVERSIONLABEL), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LANGVERSION), SW_HIDE);
	}

	/* general */
	SetDlgItemTextA(hwndDlg, IDC_LANGMODUSING, pack->szLastModifiedUsing);
	SetDlgItemTextA(hwndDlg, IDC_LANGAUTHORS, pack->szAuthors);
	SetDlgItemTextA(hwndDlg, IDC_LANGEMAIL, pack->szAuthorEmail);
	SetDlgItemText(hwndDlg, IDC_LANGINFOFRAME, TranslateTS(pack->szLanguage));
}

#define M_RELOADLIST   (WM_APP+1)
#define M_SHOWFILECOL  (WM_APP+2)

static void DeletePackFile(HWND hwndDlg, HWND hwndList, int iItem, HELPPACK_INFO *pack)
{
	SHFILEOPSTRUCT sfo = { 0 };
	sfo.hwnd = hwndDlg;
	sfo.wFunc = FO_DELETE;
	sfo.fFlags = FOF_SIMPLEPROGRESS | FOF_SILENT; /* silent = no progress */
	/* double zero terminated */
	TCHAR szFileName[MAX_PATH];
	if (GetPackPath(szFileName, _countof(szFileName) - 1, pack->flags&HPF_ENABLED, pack->szFileName)) {
		szFileName[lstrlen(szFileName) + 1] = _T('\0');
		sfo.pFrom = szFileName;
		/* ask to delete file */
		if (!SHFileOperation(&sfo) && !sfo.fAnyOperationsAborted) {
			LVITEM lvi;
			lvi.iItem = iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_STATE;
			lvi.stateMask = LVIS_STATEIMAGEMASK | LVIS_SELECTED | LVIS_FOCUSED;
			if (ListView_GetItem(hwndList, &lvi)) {
				ListView_DeleteItem(hwndList, iItem);
				/* enable/select next item at same position */
				int nCount = ListView_GetItemCount(hwndList);
				lvi.iItem = (iItem < nCount) ? iItem : iItem - 1;
				if (lvi.iItem != -1) {
					ListView_SetItemState(hwndList, lvi.iItem, lvi.state, lvi.stateMask);
					/* enable pack file */
					lvi.mask = LVIF_PARAM;
					lvi.iSubItem = 0;
					if (ListView_GetItem(hwndList, &lvi))
						EnablePack((HELPPACK_INFO*)lvi.lParam, L"helppack_*.txt");
				}
				else
					DisplayPackInfo(hwndDlg, NULL);
				if (nCount == 1)
					SendMessage(hwndDlg, M_SHOWFILECOL, 0, FALSE);
			}
		}
	}
}

static INT_PTR CALLBACK InsertPackItemEnumProc(HELPPACK_INFO *pack, WPARAM wParam, LPARAM lParam)
{
	HELPPACK_INFO *pack2 = (HELPPACK_INFO*)mir_alloc(sizeof(HELPPACK_INFO));
	if (pack2 == NULL)
		return FALSE;
	CopyMemory(pack2, pack, sizeof(HELPPACK_INFO));

	/* country flag icon */
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	if ((HIMAGELIST)lParam != NULL) {
		HICON hIcon;
		if (pack->flags&HPF_DEFAULT) /* not really needed, but to be consistent with LangMan */
			hIcon = (HICON)CallService(MS_FLAGS_CREATEMERGEDFLAGICON, CTRY_UNITED_STATES, CTRY_UNITED_KINGDOM);
		else {
			int countryId = 0xFFFF; /* Unknown */
			TCHAR szBuf[6];
			/* get country id from locale */
			if (!(pack->flags&HPF_NOLOCALE))
				if (GetLocaleInfo(pack->Locale, LOCALE_ICOUNTRY, szBuf, _countof(szBuf)))
					countryId = _ttoi(szBuf);
			hIcon = (HICON)CallService(MS_FLAGS_LOADFLAGICON, countryId, 0);
		}
		lvi.iImage = (hIcon == NULL) ? -1 : ImageList_AddIcon((HIMAGELIST)lParam, hIcon);
		lvi.mask |= LVIF_IMAGE;
	}

	/* insert */
	lvi.iItem = lvi.iSubItem = 0;
	lvi.stateMask = LVIS_STATEIMAGEMASK | LVIS_SELECTED;
	lvi.state = INDEXTOSTATEIMAGEMASK((pack->flags&HPF_ENABLED) ? 2 : 1);
	if (pack->flags&HPF_ENABLED)
		lvi.state |= LVIS_SELECTED | LVIS_FOCUSED;
	lvi.pszText = TranslateTS(pack->szLanguage);
	lvi.lParam = (LPARAM)pack2;
	ListView_InsertItem((HWND)wParam, &lvi);

	return TRUE;
}

static int CALLBACK CompareListItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int cmp = CompareString((LCID)lParamSort, 0, ((HELPPACK_INFO*)lParam1)->szLanguage, -1, ((HELPPACK_INFO*)lParam2)->szLanguage, -1);
	if (cmp)
		cmp -= 2;
	if (!cmp) {
		if (((HELPPACK_INFO*)lParam1)->flags&HPF_ENABLED)
			cmp = -1;
		else if (((HELPPACK_INFO*)lParam2)->flags&HPF_ENABLED)
			cmp = 1;
	}

	return cmp;
}

static INT_PTR CALLBACK LangOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LANGLIST);
	LVITEM lvi;

	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			hwndLangOpt = hwndDlg;
			ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
			ListView_SetImageList(hwndList, CreateRadioImages(ListView_GetBkColor(hwndList), ListView_GetTextColor(hwndList)), LVSIL_STATE); /* auto-destroyed */
			{
				LVCOLUMN lvc;
				lvc.mask = LVCF_TEXT;
				lvc.pszText = TranslateT("Installed languages");
				ListView_InsertColumn(hwndList, 0, &lvc);
			}
			if (ServiceExists(MS_FLAGS_LOADFLAGICON))
				ListView_SetImageList(hwndList, ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR24, 8, 8), LVSIL_SMALL);
			CorrectPacks(L"helppack_*.txt", L"helppack_english.txt", FALSE);
			SendMessage(hwndDlg, M_RELOADLIST, 0, 0);
			SendMessage(hwndDlg, M_SHOWFILECOL, 0, 1);
			return TRUE;
		}
	case M_RELOADLIST:
		{
			/* init list */
			ListView_DeleteAllItems(hwndList);
			ListView_DeleteColumn(hwndList, 1); /* if present */
			{
				HIMAGELIST himl = ListView_GetImageList(hwndList, LVSIL_SMALL);
				ImageList_RemoveAll(himl);
				/* enum all packs */
				EnumPacks(InsertPackItemEnumProc, L"helppack_*.txt", "Miranda Help Pack Version 1", (WPARAM)hwndList, (LPARAM)himl);
				/* nothing installed */
				if (!ListView_GetItemCount(hwndList))
					DisplayPackInfo(hwndDlg, NULL);
				/* make it use current langpack locale for sort */
				ListView_SortItems(hwndList, CompareListItem, Langpack_GetDefaultLocale());
				/* show selection */
				int iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
				if (iItem != -1)
					ListView_EnsureVisible(hwndList, iItem, FALSE);
			}
			return TRUE;
		}
	case M_SHOWFILECOL:
		if ((BOOL)lParam && ListView_GetItemCount(hwndList)>1) {
			/* add column */
			LVCOLUMN lvc;
			ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);
			lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lvc.pszText = TranslateT("File");
			lvc.cx = 160;
			ListView_InsertColumn(hwndList, lvc.iSubItem = 1, &lvc);
			ListView_SetColumnWidth(hwndList, 0, ListView_GetColumnWidth(hwndList, 0) - lvc.cx);

			/* add text */
			lvi.mask = LVIF_PARAM;
			lvi.iSubItem = 0;
			for (lvi.iItem = 0; ListView_GetItem(hwndList, &lvi); ++lvi.iItem) {
				HELPPACK_INFO *pack = (HELPPACK_INFO*)lvi.lParam;
				ListView_SetItemText(hwndList, lvi.iItem, 1, pack->szFileName);
			}
		}
		else {
			ListView_DeleteColumn(hwndList, 1);
			ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);
		}
		return TRUE;

	case WM_THEMECHANGED:
	case WM_SETTINGCHANGE:
		{
			HIMAGELIST himl = ListView_SetImageList(hwndList, CreateRadioImages(ListView_GetBkColor(hwndList), ListView_GetTextColor(hwndList)), LVSIL_STATE); /* auto-destroyed */
			if (himl != NULL)
				ImageList_Destroy(himl);
		}
		break;

	case WM_CTLCOLORLISTBOX: /* mimic readonly edit */
		return (BOOL)SendMessage(hwndDlg, WM_CTLCOLORSTATIC, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LANGEMAIL:
			{
				char buf[512];
				mir_strcpy(buf, "mailto:");
				if (GetDlgItemTextA(hwndDlg, LOWORD(wParam), &buf[7], _countof(buf) - 7))
					Utils_OpenUrl(buf);
				return TRUE;
			}

		case IDC_MORELANG:
			Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Download");
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU:
		if (GetDlgCtrlID((HWND)wParam) == IDC_LANGLIST) {
			/* get item */
			LVHITTESTINFO hti;
			POINTSTOPOINT(hti.pt, MAKEPOINTS(lParam));
			if (hti.pt.x == -1 && hti.pt.y == -1) {
				/* keyboard invoked */
				hti.iItem = ListView_GetNextItem((HWND)wParam, -1, LVNI_SELECTED);
				if (hti.iItem != -1)
					break;

				RECT rc;
				if (!ListView_GetItemRect((HWND)wParam, hti.iItem, &rc, LVIR_SELECTBOUNDS))
					break;

				hti.pt.x = rc.left + (rc.right - rc.left) / 2;
				hti.pt.y = rc.top + (rc.bottom - rc.top) / 2;
				ClientToScreen((HWND)wParam, &hti.pt);
			}
			else {
				ScreenToClient((HWND)wParam, &hti.pt);
				if (ListView_HitTest((HWND)wParam, &hti) == -1 || !(hti.flags&LVHT_ONITEM))
					break;
				POINTSTOPOINT(hti.pt, MAKEPOINTS(lParam));
			}
			/* param */
			lvi.iItem = hti.iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_PARAM;
			if (!ListView_GetItem((HWND)wParam, &lvi))
				break;

			/* context menu */
			HELPPACK_INFO *pack = (HELPPACK_INFO*)lvi.lParam;
			HMENU hContextMenu = CreatePopupMenu();
			if (hContextMenu != NULL) {
				AppendMenu(hContextMenu, MF_STRING, 2, TranslateT("&Remove..."));
				if (TrackPopupMenuEx(hContextMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION, hti.pt.x, hti.pt.y, (HWND)wParam, NULL))
					DeletePackFile(hwndDlg, (HWND)wParam, hti.iItem, pack);
				DestroyMenu(hContextMenu);
			}
			return TRUE;
		}
		break;

	case WM_NOTIFYFORMAT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, NFR_UNICODE);
		return TRUE;

	case WM_NOTIFY:
		NMHDR *nmhdr = (NMHDR*)lParam;
		switch (nmhdr->idFrom) {
		case IDC_LANGLIST:
			switch (nmhdr->code) {
			case LVN_DELETEITEM:
				LVITEM lvi;
				lvi.iItem = ((NMLISTVIEW*)lParam)->iItem; /* nmlv->lParam is invalid */
				lvi.iSubItem = 0;
				lvi.mask = LVIF_PARAM;
				if (ListView_GetItem(nmhdr->hwndFrom, &lvi))
					mir_free((HELPPACK_INFO*)lvi.lParam);
				break;

			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW*)lParam;
					if (!(nmlv->uChanged&LVIF_STATE))
						break;

					/* display info and check radio item */
					if (nmlv->uNewState&LVIS_SELECTED && !(nmlv->uOldState&LVIS_SELECTED)) {
						ListView_SetItemState(nmhdr->hwndFrom, nmlv->iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
						DisplayPackInfo(hwndDlg, (HELPPACK_INFO*)nmlv->lParam);
					}
					/* disable all other radio items */
					else if (nmlv->uNewState&INDEXTOSTATEIMAGEMASK(2)) {
						for (int iItem = ListView_GetItemCount(nmhdr->hwndFrom) - 1; iItem != -1; --iItem)
							if (iItem != nmlv->iItem)
								ListView_SetItemState(nmhdr->hwndFrom, iItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);

						/* enable apply */
						if (nmlv->uOldState)
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				break;

			case LVN_KEYDOWN:
				{
					int iItem = ListView_GetNextItem(nmhdr->hwndFrom, -1, LVNI_SELECTED);
					switch (((NMLVKEYDOWN*)lParam)->wVKey) {
					case VK_SPACE:
						ListView_SetItemState(nmhdr->hwndFrom, iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
						break;

					case VK_DELETE:
						lvi.iItem = iItem;
						lvi.iSubItem = 0;
						lvi.mask = LVIF_PARAM;
						if (ListView_GetItem(nmhdr->hwndFrom, &lvi))
							DeletePackFile(hwndDlg, nmhdr->hwndFrom, iItem, (HELPPACK_INFO*)lvi.lParam);
						break;
					}
				}
				break;

			case NM_CLICK:
				LVHITTESTINFO hti;
				lParam = GetMessagePos();
				POINTSTOPOINT(hti.pt, MAKEPOINTS(lParam));
				ScreenToClient(nmhdr->hwndFrom, &hti.pt);
				if (ListView_HitTest(nmhdr->hwndFrom, &hti) != -1)
					if (hti.flags&(LVHT_ONITEMSTATEICON | LVHT_ONITEMICON)) /* one of them */
						ListView_SetItemState(nmhdr->hwndFrom, hti.iItem, LVIS_SELECTED, LVIS_SELECTED);
			} /* switch nmhdr->code */
			break;

		case 0:
			switch (nmhdr->code) {
			case PSN_APPLY:
				lvi.mask = LVIF_STATE | LVIF_PARAM;
				lvi.stateMask = LVIS_STATEIMAGEMASK;
				lvi.iSubItem = 0;
				for (lvi.iItem = 0; ListView_GetItem(hwndList, &lvi); ++lvi.iItem) {
					HELPPACK_INFO *pack = (HELPPACK_INFO*)lvi.lParam;
					if (lvi.state&INDEXTOSTATEIMAGEMASK(2)) {
						EnablePack(pack, L"helppack_*.txt");
						pack->flags |= HPF_ENABLED;
					}
					else
						pack->flags &= ~HPF_ENABLED;
				}
				return TRUE;
			}
		} /* switch nmhdr->idFrom */
		break;
	}
	return FALSE;
}

void ReloadLangOptList(void)
{
	if (hwndLangOpt != NULL) {
		SendMessage(hwndLangOpt, M_RELOADLIST, 0, 0);
		SendMessage(hwndLangOpt, M_SHOWFILECOL, 0, 1);
	}
}

/**************************** ADV OPTS ****************************/

static INT_PTR CALLBACK AdvOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			WORD val = settingAutoTipDelay;
			if (!val)
				val = db_get_w(NULL, "HelpPlugin", "AutoTipDelay", SETTING_AUTOTIPDELAY_DEFAULT);
			CheckDlgButton(hwndDlg, IDC_AUTOTIPSENABLED, settingAutoTipDelay != 0);
			SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_SETRANGE, 0, MAKELPARAM(10000, 500));
			SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAY, EM_SETLIMITTEXT, (WPARAM)5, 0);
			{
				UDACCEL uda = { 0, 500 };
				SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_SETACCEL, 1, (LPARAM)&uda);
			}
			SetDlgItemInt(hwndDlg, IDC_AUTOTIPDELAY, val, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_SETPOS, 0, MAKELPARAM(val, 0));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAYLABEL), settingAutoTipDelay != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAY), settingAutoTipDelay != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAYSPIN), settingAutoTipDelay != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAYMS), settingAutoTipDelay != 0);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AUTOTIPSENABLED:
			{
				BOOL checked = IsDlgButtonChecked(hwndDlg, IDC_AUTOTIPSENABLED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAYLABEL), checked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAY), checked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAYSPIN), checked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOTIPDELAYMS), checked);
				break;
			}
		case IDC_AUTOTIPDELAY:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
				WORD minVal = HIWORD(SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_GETRANGE, 0, 0));
				if (GetDlgItemInt(hwndDlg, IDC_AUTOTIPDELAY, NULL, FALSE)<minVal) {
					SetDlgItemInt(hwndDlg, IDC_AUTOTIPDELAY, minVal, FALSE);
					SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_SETPOS, 0, MAKELPARAM(minVal, 0));
				}
			}
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		}
		PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0); // enable apply
		break;
	case WM_NOTIFYFORMAT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, NFR_UNICODE);
		return TRUE;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					WORD minVal;
					minVal = HIWORD(SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_GETRANGE, 0, 0));
					if (GetDlgItemInt(hwndDlg, IDC_AUTOTIPDELAY, NULL, FALSE) < minVal) {
						SetDlgItemInt(hwndDlg, IDC_AUTOTIPDELAY, minVal, FALSE);
						SendDlgItemMessage(hwndDlg, IDC_AUTOTIPDELAYSPIN, UDM_SETPOS, 0, MAKELPARAM(minVal, 0));
					}
					settingAutoTipDelay = (WORD)GetDlgItemInt(hwndDlg, IDC_AUTOTIPDELAY, NULL, FALSE);
					db_set_w(NULL, "HelpPlugin", "AutoTipDelay", settingAutoTipDelay);

					if (!IsDlgButtonChecked(hwndDlg, IDC_AUTOTIPSENABLED))
						settingAutoTipDelay = 0;
					db_set_b(NULL, "HelpPlugin", "AutoTipsEnabled", (BYTE)(settingAutoTipDelay != 0));
					return TRUE;
				}
			}
			break;
		}
		break;
	}
	return FALSE;
}

/**************************** INIT ********************************/

static int HelpOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 1200000080;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LANG);
	odp.ptszGroup = LPGENT("Customize"); // autotranslated
	odp.ptszTitle = LPGENT("Help");      // autotranslated

	odp.ptszTab = LPGENT("Language");    // autotranslated
	odp.flags = ODPF_TCHAR | ODPF_BOLDGROUPS;
	odp.pfnDlgProc = LangOptDlgProc;
	Options_AddPage(wParam, &odp);

#ifdef EDITOR
	odp.ptszTab = L"Help editor"; /* autotranslated */
#else
	odp.ptszTab = LPGENT("Advanced");    /* autotranslated */
#endif
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ADV);
	odp.pfnDlgProc = AdvOptDlgProc;
	Options_AddPage(wParam, &odp);

	return 0;
}

void InitOptions(void)
{
	settingAutoTipDelay = 0;
	if (db_get_b(NULL, "HelpPlugin", "AutoTipsEnabled", SETTING_AUTOTIPSENABLED_DEFAULT))
		settingAutoTipDelay = db_get_w(NULL, "HelpPlugin", "AutoTipDelay", SETTING_AUTOTIPDELAY_DEFAULT);

	hwndLangOpt = NULL;
	hHookOptInit = HookEvent(ME_OPT_INITIALISE, HelpOptInit);
	CorrectPacks(L"helppack_*.txt", L"helppack_english.txt", FALSE);
}

void UninitOptions(void)
{
	UnhookEvent(hHookOptInit);
}