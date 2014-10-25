/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

#include <m_version.h>

#include "plugins.h"

extern MUUID miid_clist, miid_database, miid_protocol;
HANDLE hevLoadModule, hevUnloadModule;
bool bOldMode = false;

static CMString szFilter;
static UINT_PTR timerID;

/////////////////////////////////////////////////////////////////////////////////////////
//   Plugins options page dialog

struct PluginListItemData
{
	TCHAR fileName[MAX_PATH];
	HINSTANCE hInst;
	int   flags, stdPlugin;
	char* author;
	char* authorEmail;
	char* description;
	char* copyright;
	char* homepage;
	MUUID uuid;
};

static int sttSortPlugins(const PluginListItemData *p1, const PluginListItemData *p2)
{
	return _tcscmp(p1->fileName, p2->fileName);
}

static LIST<PluginListItemData> arPluginList(10, sttSortPlugins);

static BOOL dialogListPlugins(WIN32_FIND_DATA *fd, TCHAR *path, WPARAM, LPARAM lParam)
{
	TCHAR buf[MAX_PATH];
	mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, fd->cFileName);
	HINSTANCE hInst = GetModuleHandle(buf);

	BASIC_PLUGIN_INFO pi;
	if (checkAPI(buf, &pi, MIRANDA_VERSION_CORE, CHECKAPI_NONE) == 0)
		return TRUE;

	PluginListItemData *dat = (PluginListItemData*)mir_alloc(sizeof(PluginListItemData));
	dat->hInst = hInst;
	dat->flags = pi.pluginInfo->flags;

	dat->stdPlugin = 0;
	if (pi.Interfaces) {
		MUUID *piface = pi.Interfaces;
		for (int i=0; !equalUUID(miid_last, piface[i]); i++) {
			int idx = getDefaultPluginIdx( piface[i] );
			if (idx != -1 ) {
				dat->stdPlugin |= (1 << idx);
				break;
			}
		}
	}

	CharLower(fd->cFileName);
	_tcsncpy(dat->fileName, fd->cFileName, SIZEOF(dat->fileName));

	HWND hwndList = (HWND)lParam;

	LVITEM it = { 0 };
	// column  1: Checkbox +  Enable/disabled icons
	it.mask = LVIF_PARAM | LVIF_IMAGE;
	it.iImage = (hInst != NULL) ? 2 : 3;
	bool bNoCheckbox = (dat->flags & STATIC_PLUGIN) != 0;
	if (bNoCheckbox|| hasMuuid(pi, miid_clist) || hasMuuid(pi, miid_protocol))
		it.iImage += 2;
	it.lParam = (LPARAM)dat;
	int iRow = ListView_InsertItem(hwndList, &it);

	if (isPluginOnWhiteList(fd->cFileName))
		ListView_SetItemState(hwndList, iRow, bNoCheckbox ? 0x3000 : 0x2000, LVIS_STATEIMAGEMASK);

	if (iRow != -1) {
		// column 2: Unicode/ANSI icon + filename
		it.mask = LVIF_IMAGE | LVIF_TEXT;
		it.iItem = iRow;
		it.iSubItem = 1;
		it.iImage = (dat->flags & UNICODE_AWARE) ? 0 : 1;
		it.pszText = fd->cFileName;
		ListView_SetItem(hwndList, &it);

		dat->author = mir_strdup(pi.pluginInfo->author);
		dat->authorEmail = mir_strdup(pi.pluginInfo->authorEmail);
		dat->copyright = mir_strdup(pi.pluginInfo->copyright);
		dat->description = mir_strdup(pi.pluginInfo->description);
		dat->homepage = mir_strdup(pi.pluginInfo->homepage);
		if (pi.pluginInfo->cbSize == sizeof(PLUGININFOEX))
			dat->uuid = pi.pluginInfo->uuid;
		else
			memset(&dat->uuid, 0, sizeof(dat->uuid));

		TCHAR *shortNameT = mir_a2t(pi.pluginInfo->shortName);
		// column 3: plugin short name
		if (shortNameT) {
			ListView_SetItemText(hwndList, iRow, 2, shortNameT);
			mir_free(shortNameT);
		}

		// column4: version number
		DWORD unused, verInfoSize = GetFileVersionInfoSize(buf, &unused);
		if (verInfoSize != 0) {
			UINT blockSize;
			VS_FIXEDFILEINFO *fi;
			void *pVerInfo = mir_alloc(verInfoSize);
			GetFileVersionInfo(buf, 0, verInfoSize, pVerInfo);
			VerQueryValue(pVerInfo, _T("\\"), (LPVOID*)&fi, &blockSize);
			mir_sntprintf(buf, SIZEOF(buf), _T("%d.%d.%d.%d"), HIWORD(fi->dwProductVersionMS),
				LOWORD(fi->dwProductVersionMS), HIWORD(fi->dwProductVersionLS), LOWORD(fi->dwProductVersionLS));
			mir_free(pVerInfo);
		}
		else
			mir_sntprintf(buf, SIZEOF(buf), _T("%d.%d.%d.%d"), HIBYTE(HIWORD(pi.pluginInfo->version)),
				LOBYTE(HIWORD(pi.pluginInfo->version)), HIBYTE(LOWORD(pi.pluginInfo->version)),
				LOBYTE(LOWORD(pi.pluginInfo->version)));

		ListView_SetItemText(hwndList, iRow, 3, buf);
		arPluginList.insert(dat);
	}
	else
		mir_free(dat);
	FreeLibrary(pi.hInst);
	return TRUE;
}

static int uuidToString(const MUUID uuid, char *szStr, int cbLen)
{
	if (cbLen < 1 || !szStr)
		return 0;

	mir_snprintf(szStr, cbLen, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
	return 1;
}

static void RemoveAllItems(HWND hwnd)
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = 0;
	while (ListView_GetItem(hwnd, &lvi)) {
		PluginListItemData *dat = (PluginListItemData*)lvi.lParam;
		mir_free(dat->author);
		mir_free(dat->authorEmail);
		mir_free(dat->copyright);
		mir_free(dat->description);
		mir_free(dat->homepage);
		mir_free(dat);
		lvi.iItem ++;
	}
}

static bool LoadPluginDynamically(PluginListItemData *dat)
{
	TCHAR exe[MAX_PATH];
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	TCHAR *p = _tcsrchr(exe, '\\'); if (p) *p = 0;

	pluginEntry* pPlug = OpenPlugin(dat->fileName, _T("Plugins"), exe);
	if (pPlug->pclass & PCLASS_FAILED) {
LBL_Error:
		Plugin_UnloadDyn(pPlug);
		return false;
	}

	if (!TryLoadPlugin(pPlug, true))
		goto LBL_Error;

	if (CallPluginEventHook(pPlug->bpi.hInst, hModulesLoadedEvent, 0, 0) != 0)
		goto LBL_Error;

	dat->hInst = pPlug->bpi.hInst;
	NotifyFastHook(hevLoadModule, (WPARAM)pPlug->bpi.pluginInfo, (LPARAM)pPlug->bpi.hInst);
	return true;
}

static bool UnloadPluginDynamically(PluginListItemData *dat)
{
	pluginEntry *p = pluginList.find((pluginEntry*)dat->fileName);
	if (p) {
		if (!Plugin_UnloadDyn(p))
			return false;

		dat->hInst = NULL;
	}
	return true;
}

static LRESULT CALLBACK PluginListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CHAR:
		if (wParam == '\b') {
			if (szFilter.GetLength() > 0)
				szFilter.Truncate(szFilter.GetLength() - 1);
		}
		else {
			szFilter.AppendChar(wParam);

			for (int i = 0; i < arPluginList.getCount(); i++) {
				PluginListItemData *p = arPluginList[i];
				if (!_tcsnicmp(szFilter, p->fileName, szFilter.GetLength())) {
					LVFINDINFO lvfi;
					lvfi.flags = LVFI_PARAM;
					lvfi.lParam = (LPARAM)p;
					int idx = ListView_FindItem(hwnd, 0, &lvfi);
					if (idx != -1) {
						ListView_SetItemState(hwnd, idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						ListView_EnsureVisible(hwnd, idx, FALSE);
						if (timerID != 0)
							KillTimer(hwnd, timerID);
						timerID = SetTimer(hwnd, 1, 1500, 0);
						return TRUE;
					}
				}
			}

			szFilter.Truncate(szFilter.GetLength() - 1);
			MessageBeep((UINT)-1);
		}
		return TRUE;

	case WM_TIMER:
		if (wParam == 1) {
			KillTimer(hwnd, timerID);
			timerID = 0;
			szFilter.Empty();
		}
		break;

	case WM_LBUTTONDOWN:
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam);
		hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwnd, &hi);
		// Dynamically load/unload a plugin
		if ((hi.iSubItem == 0) && (hi.flags & LVHT_ONITEMICON)) {
			LVITEM lvi = {0};
			lvi.mask = LVIF_IMAGE | LVIF_PARAM;
			lvi.stateMask = -1;
			lvi.iItem = hi.iItem;
			lvi.iSubItem = 0;
			if (ListView_GetItem(hwnd, &lvi)) {
				lvi.mask = LVIF_IMAGE;
				PluginListItemData *dat = (PluginListItemData*)lvi.lParam;
				if (lvi.iImage == 3) {
					// load plugin
					if (LoadPluginDynamically(dat)) {
						lvi.iImage = 2;
						ListView_SetItem(hwnd, &lvi);
					}
				}
				else if (lvi.iImage == 2) {
					// unload plugin
					if (UnloadPluginDynamically(dat)) {
						lvi.iImage = 3;
						ListView_SetItem(hwnd, &lvi);
					}
				}
				LoadStdPlugins();
			}
		}
	}

	return mir_callNextSubclass(hwnd, PluginListWndProc, msg, wParam, lParam);
}

static int CALLBACK SortPlugins(WPARAM i1, LPARAM i2, LPARAM)
{
	PluginListItemData *p1 = (PluginListItemData*)i1, *p2 = (PluginListItemData*)i2;
	return _tcscmp(p1->fileName, p2->fileName);
}

static TCHAR *latin2t(const char *p)
{
	if (p == NULL)
		return mir_tstrdup( _T(""));

	return mir_a2t_cp(p, 1250);
}

INT_PTR CALLBACK DlgPluginOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		timerID = 0;
		{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
			mir_subclassWindow(hwndList, PluginListWndProc);

			HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 4, 0);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_UNICODE);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_ANSI);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_LOADED);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_NOTLOADED);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_LOADEDGRAY);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_NOTLOADEDGRAY);
			ListView_SetImageList(hwndList, hIml, LVSIL_SMALL);

			LVCOLUMN col;
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = _T("");
			col.cx = 40;
			ListView_InsertColumn(hwndList, 0, &col);

			col.pszText = TranslateT("Plugin");
			col.cx = 180;
			ListView_InsertColumn(hwndList, 1, &col);

			col.pszText = TranslateT("Name");
			col.cx = 180;//max = 220;
			ListView_InsertColumn(hwndList, 2, &col);

			col.pszText = TranslateT("Version");
			col.cx = 75;
			ListView_InsertColumn(hwndList, 3, &col);

			ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_SUBITEMIMAGES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);
			// scan the plugin dir for plugins, cos
			arPluginList.destroy();
			szFilter.Empty();
			enumPlugins(dialogListPlugins, (WPARAM)hwndDlg, (LPARAM)hwndList);
			// sort out the headers

			ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE); // dll name
			int w = ListView_GetColumnWidth(hwndList, 1);
			if (w > 110) {
				ListView_SetColumnWidth(hwndList, 1, w = 110);
			}
			int max = w < 110 ? 189 + 110 - w : 189;
			ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE); // short name
			w = ListView_GetColumnWidth(hwndList, 2);
			if (w > max)
				ListView_SetColumnWidth(hwndList, 2, max);

			ListView_SortItems(hwndList, SortPlugins, (LPARAM)hwndDlg);
		}
		return TRUE;

	case WM_NOTIFY:
		if (lParam) {
			NMLISTVIEW *hdr = (NMLISTVIEW *)lParam;
			if (hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible(hdr->hdr.hwndFrom)) {
				if (hdr->uOldState != 0 && (hdr->uNewState == 0x1000 || hdr->uNewState == 0x2000)) {
					HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);

					LVITEM it;
					it.mask = LVIF_PARAM | LVIF_STATE;
					it.iItem = hdr->iItem;
					if (!ListView_GetItem(hwndList, &it))
						break;

					PluginListItemData *dat = (PluginListItemData*)it.lParam;
					if (dat->flags & STATIC_PLUGIN) {
						ListView_SetItemState(hwndList, hdr->iItem, 0x3000, LVIS_STATEIMAGEMASK);
						return FALSE;
					}
					// find all another standard plugins by mask and disable them
					if ((hdr->uNewState == 0x2000) && dat->stdPlugin != 0) {
						for (int iRow = 0; iRow != -1; iRow = ListView_GetNextItem(hwndList, iRow, LVNI_ALL)) {
							if (iRow != hdr->iItem) { // skip the plugin we're standing on
								LVITEM dt;
								dt.mask = LVIF_PARAM;
								dt.iItem = iRow;
								if (ListView_GetItem(hwndList, &dt)) {
									PluginListItemData *dat2 = (PluginListItemData*)dt.lParam;
									if (dat2->stdPlugin & dat->stdPlugin) {// mask differs
										// the lParam is unset, so when the check is unset the clist block doesnt trigger
										int lParam = dat2->stdPlugin;
										dat2->stdPlugin = 0;
										ListView_SetItemState(hwndList, iRow, 0x1000, LVIS_STATEIMAGEMASK);
										dat2->stdPlugin = lParam;
									}
								}
							}
						}
					}

					if (bOldMode)
						ShowWindow(GetDlgItem(hwndDlg, IDC_RESTART), TRUE); // this here only in "ghazan mode"
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}

				if (hdr->iItem != -1) {
					int sel = hdr->uNewState & LVIS_SELECTED;
					HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
					LVITEM lvi = { 0 };
					lvi.mask = LVIF_PARAM;
					lvi.iItem = hdr->iItem;
					if (ListView_GetItem(hwndList, &lvi)) {
						PluginListItemData *dat = (PluginListItemData*)lvi.lParam;
						
						TCHAR buf[1024];
						ListView_GetItemText(hwndList, hdr->iItem, 2, buf, SIZEOF(buf));
						SetDlgItemText(hwndDlg, IDC_PLUGININFOFRAME, sel ? buf : _T(""));

						ptrT tszAuthor(latin2t(sel ? dat->author : NULL));
						SetDlgItemText(hwndDlg, IDC_PLUGINAUTHOR, tszAuthor);

						ptrT tszEmail(latin2t(sel ? dat->authorEmail : NULL));
						SetDlgItemText(hwndDlg, IDC_PLUGINEMAIL, tszEmail);

						ptrT p(Langpack_PcharToTchar(dat->description));
						SetDlgItemText(hwndDlg, IDC_PLUGINLONGINFO, sel ? p : _T(""));

						ptrT tszCopyright(latin2t(sel ? dat->copyright : NULL));
						SetDlgItemText(hwndDlg, IDC_PLUGINCPYR, tszCopyright);

						ptrT tszUrl(latin2t(sel ? dat->homepage : NULL));
						SetDlgItemText(hwndDlg, IDC_PLUGINURL, tszUrl);

						if (!equalUUID(miid_last, dat->uuid)) {
							char szUID[128];
							uuidToString(dat->uuid, szUID, sizeof(szUID));
							SetDlgItemTextA(hwndDlg, IDC_PLUGINPID, sel ? szUID : "");
						}
						else
							SetDlgItemText(hwndDlg, IDC_PLUGINPID, sel ? TranslateT("<none>") : _T(""));
					}
				}
			}

			if (hdr->hdr.code == PSN_APPLY) {
				bool needRestart = false;
				TCHAR bufRestart[1024];
				int bufLen = mir_sntprintf(bufRestart, SIZEOF(bufRestart), _T("%s\n"), TranslateT("Miranda NG must be restarted to apply changes for these plugins:"));

				HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
				for (int iRow = 0; iRow != -1;) {
					TCHAR buf[1024];
					ListView_GetItemText(hwndList, iRow, 1, buf, SIZEOF(buf));
					int iState = ListView_GetItemState(hwndList, iRow, LVIS_STATEIMAGEMASK);
					SetPluginOnWhiteList(buf, (iState & 0x2000) ? 1 : 0);

					if (!bOldMode && iState != 0x3000) {
						LVITEM lvi = { 0 };
						lvi.mask = LVIF_IMAGE | LVIF_PARAM;
						lvi.stateMask = -1;
						lvi.iItem = iRow;
						lvi.iSubItem = 0;
						if (ListView_GetItem(hwndList, &lvi)) {
							lvi.mask = LVIF_IMAGE;

							PluginListItemData *dat = (PluginListItemData*)lvi.lParam;
							if (iState == 0x2000) {
								// enabling plugin
								if (lvi.iImage == 3 || lvi.iImage == 5) {
									if (lvi.iImage == 3 && LoadPluginDynamically(dat)) {
										lvi.iImage = 2;
										ListView_SetItem(hwndList, &lvi);
									}
									else {
										bufLen += mir_sntprintf(bufRestart + bufLen, SIZEOF(bufRestart) - bufLen, _T(" - %s\n"), buf);
										needRestart = true;
									}
								}
							}
							else {
								// disabling plugin
								if (lvi.iImage == 2 || lvi.iImage == 4) {
									if (lvi.iImage == 2 && UnloadPluginDynamically(dat)) {
										lvi.iImage = 3;
										ListView_SetItem(hwndList, &lvi);
									}
									else {
										bufLen += mir_sntprintf(bufRestart + bufLen, SIZEOF(bufRestart) - bufLen, _T(" - %s\n"), buf);
										needRestart = true;
									}
								}
							}
						}
					}

					iRow = ListView_GetNextItem(hwndList, iRow, LVNI_ALL);
				}
				LoadStdPlugins();

				ShowWindow(GetDlgItem(hwndDlg, IDC_RESTART), needRestart);
				if (needRestart) {
					mir_sntprintf(bufRestart + bufLen, SIZEOF(bufRestart) - bufLen, _T("\n%s"), TranslateT("Do you want to restart it now?"));
					if (MessageBox(NULL, bufRestart, _T("Miranda NG"), MB_ICONWARNING | MB_YESNO) == IDYES)
						CallService(MS_SYSTEM_RESTART, 1, 0);
				}
			}
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDC_GETMOREPLUGINS:
				CallService(MS_UTILS_OPENURL, 0, (LPARAM) "http://miranda-ng.org/downloads/");
				break;

			case IDC_PLUGINEMAIL:
			case IDC_PLUGINURL:
				char buf[512];
				char *p = &buf[7];
				lstrcpyA(buf, "mailto:");
				if (GetDlgItemTextA(hwndDlg, LOWORD(wParam), p, SIZEOF(buf) - 7))
					CallService(MS_UTILS_OPENURL, 0, (LPARAM)(LOWORD(wParam) == IDC_PLUGINEMAIL ? buf : p));
				break;
			}
		}
		break;

	case WM_DESTROY:
		arPluginList.destroy();
		RemoveAllItems(GetDlgItem(hwndDlg, IDC_PLUGLIST));
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int PluginOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pfnDlgProc = DlgPluginOpt;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PLUGINS);
	odp.position = 1300000000;
	odp.pszTitle = LPGEN("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

void LoadPluginOptions()
{
	hevLoadModule = CreateHookableEvent(ME_SYSTEM_MODULELOAD);
	hevUnloadModule = CreateHookableEvent(ME_SYSTEM_MODULEUNLOAD);
}

void UnloadPluginOptions()
{
	DestroyHookableEvent(hevLoadModule);
	DestroyHookableEvent(hevUnloadModule);
}
