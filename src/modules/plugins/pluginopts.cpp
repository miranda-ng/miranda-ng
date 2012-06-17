/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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

extern HANDLE hShutdownEvent, hPreShutdownEvent;
static HANDLE hevLoadModule, hevUnloadModule;

/////////////////////////////////////////////////////////////////////////////////////////
//   Plugins options page dialog

typedef struct
{
	HINSTANCE hInst;
	int   flags;
	char* author;
	char* authorEmail;
	char* description;
	char* copyright;
	char* homepage;
	MUUID uuid;
	TCHAR fileName[MAX_PATH];
}
	PluginListItemData;

static BOOL dialogListPlugins(WIN32_FIND_DATA* fd, TCHAR* path, WPARAM, LPARAM lParam)
{
	TCHAR buf[MAX_PATH];
	mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, fd->cFileName);
	HINSTANCE hInst = GetModuleHandle(buf);

	CharLower(fd->cFileName);

	BASIC_PLUGIN_INFO pi;
	if ( checkAPI(buf, &pi, MIRANDA_VERSION_CORE, CHECKAPI_NONE) == 0 )
		return TRUE;

	int isdb = pi.pluginInfo->replacesDefaultModule == DEFMOD_DB;
	int isclist = pi.pluginInfo->replacesDefaultModule == DEFMOD_CLISTALL;

	PluginListItemData* dat = (PluginListItemData*)mir_alloc( sizeof( PluginListItemData ));
	dat->hInst = hInst;
	_tcsncpy(dat->fileName, fd->cFileName, SIZEOF(dat->fileName));
	HWND hwndList = (HWND)lParam;

	LVITEM it = { 0 };
	it.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	it.iImage = ( pi.pluginInfo->flags & 1 ) ? 0 : 1;
	it.iItem = 100000; // add to the end
	it.lParam = (LPARAM)dat;
	int iRow = ListView_InsertItem( hwndList, &it );
	if ( isPluginOnWhiteList(fd->cFileName) )
		ListView_SetItemState(hwndList, iRow, !isdb ? 0x2000 : 0x3000, LVIS_STATEIMAGEMASK);
	if ( iRow != -1 ) {
		it.mask = LVIF_IMAGE;
		it.iItem = iRow;
		it.iSubItem = 1;
		it.iImage = ( hInst != NULL ) ? 2 : 3;
		if (isdb || isclist)
			it.iImage += 2;
		ListView_SetItem( hwndList, &it );

		ListView_SetItemText(hwndList, iRow, 2, fd->cFileName);

		dat->flags = pi.pluginInfo->replacesDefaultModule;
		dat->author = mir_strdup( pi.pluginInfo->author );
		dat->authorEmail = mir_strdup( pi.pluginInfo->authorEmail );
		dat->copyright = mir_strdup( pi.pluginInfo->copyright );
		dat->description = mir_strdup( pi.pluginInfo->description );
		dat->homepage = mir_strdup( pi.pluginInfo->homepage );
		if ( pi.pluginInfo->cbSize == sizeof( PLUGININFOEX ))
			dat->uuid = pi.pluginInfo->uuid;
		else
			memset( &dat->uuid, 0, sizeof(dat->uuid));

		TCHAR *shortNameT = mir_a2t(pi.pluginInfo->shortName);
		ListView_SetItemText(hwndList, iRow, 3, shortNameT);
		mir_free(shortNameT);

		DWORD unused, verInfoSize = GetFileVersionInfoSize(buf, &unused);
		if ( verInfoSize != 0 ) {
			UINT blockSize;
			VS_FIXEDFILEINFO* fi;
			void* pVerInfo = mir_alloc(verInfoSize);
			GetFileVersionInfo(buf, 0, verInfoSize, pVerInfo);
			VerQueryValue(pVerInfo, _T("\\"), (LPVOID*)&fi, &blockSize);
			mir_sntprintf(buf, SIZEOF(buf), _T("%d.%d.%d.%d"), HIWORD(fi->dwProductVersionMS),
				LOWORD(fi->dwProductVersionMS), HIWORD(fi->dwProductVersionLS), LOWORD(fi->dwProductVersionLS));
			mir_free( pVerInfo );
		}
		else
			mir_sntprintf(buf, SIZEOF(buf), _T("%d.%d.%d.%d"), HIBYTE(HIWORD(pi.pluginInfo->version)),
				LOBYTE(HIWORD(pi.pluginInfo->version)), HIBYTE(LOWORD(pi.pluginInfo->version)),
				LOBYTE(LOWORD(pi.pluginInfo->version)));

		ListView_SetItemText(hwndList, iRow, 4, buf);
	}
	else mir_free( dat );
	FreeLibrary(pi.hInst);
	return TRUE;
}

static int uuidToString(const MUUID uuid, char *szStr, int cbLen)
{
	if (cbLen<1||!szStr) return 0;
	mir_snprintf(szStr, cbLen, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
		uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
	return 1;
}

static void RemoveAllItems( HWND hwnd )
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = 0;
	while ( ListView_GetItem( hwnd, &lvi )) {
		PluginListItemData* dat = ( PluginListItemData* )lvi.lParam;
		mir_free( dat->author );
		mir_free( dat->authorEmail );
		mir_free( dat->copyright );
		mir_free( dat->description );
		mir_free( dat->homepage );
		mir_free( dat );
		lvi.iItem ++;
}	}

static int LoadPluginDynamically(PluginListItemData* dat)
{
	TCHAR exe[MAX_PATH];
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	TCHAR *p = _tcsrchr(exe, '\\'); if (p) *p = 0;

	pluginEntry* pPlug = OpenPlugin(dat->fileName, exe);
	if (pPlug->pclass & PCLASS_FAILED) {
LBL_Error:
		Plugin_Uninit(pPlug, true);
		return FALSE;
	}

	if ( !TryLoadPlugin(pPlug, true))
		goto LBL_Error;

	if ( CallPluginEventHook(pPlug->bpi.hInst, hModulesLoadedEvent, 0, 0) != 0)
		goto LBL_Error;

	dat->hInst = pPlug->bpi.hInst;
	CallHookSubscribers(hevLoadModule, (WPARAM)pPlug->bpi.InfoEx, 0);
	return TRUE;
}

static int UnloadPluginDynamically(PluginListItemData* dat)
{
	pluginEntry tmp;
	_tcsncpy(tmp.pluginname, dat->fileName, SIZEOF(tmp.pluginname)-1);

	int idx = pluginList.getIndex(&tmp);
	if (idx == -1)
		return FALSE;

	pluginEntry* pPlug = pluginList[idx];
	if ( CallPluginEventHook(pPlug->bpi.hInst, hOkToExitEvent, 0, 0) != 0)
		return FALSE;

	CallHookSubscribers(hevUnloadModule, (WPARAM)pPlug->bpi.InfoEx, 0);

	CallPluginEventHook(pPlug->bpi.hInst, hPreShutdownEvent, 0, 0);
	CallPluginEventHook(pPlug->bpi.hInst, hShutdownEvent, 0, 0);

	dat->hInst = NULL;
	Plugin_Uninit(pPlug, true);
	return TRUE;
}

static LRESULT CALLBACK PluginListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN) {
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam); hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwnd, &hi);
		if ( hi.iSubItem == 1 ) {
			LVITEM lvi = {0};
			lvi.mask = LVIF_IMAGE | LVIF_PARAM;
			lvi.stateMask = -1;
			lvi.iItem = hi.iItem;
			lvi.iSubItem = 1;
			if ( ListView_GetItem( hwnd, &lvi )) {
				lvi.mask = LVIF_IMAGE;

				PluginListItemData* dat = ( PluginListItemData* )lvi.lParam;
				if (lvi.iImage == 3) {
					if ( LoadPluginDynamically(dat)) {
						lvi.iImage = 2;
						ListView_SetItem(hwnd, &lvi);
					}
				}
				else if (lvi.iImage == 2) {
					if ( UnloadPluginDynamically(dat)) {
						lvi.iImage = 3;
						ListView_SetItem(hwnd, &lvi);
	}	}	}	}	}

	WNDPROC wnProc = ( WNDPROC )GetWindowLongPtr(hwnd, GWLP_USERDATA);
	return CallWindowProc(wnProc, hwnd, msg, wParam, lParam);
}

INT_PTR CALLBACK DlgPluginOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
			SetWindowLongPtr(hwndList, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtr(hwndList, GWLP_WNDPROC));
			SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)PluginListWndProc);

			HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | (IsWinVerXPPlus()? ILC_COLOR32 : ILC_COLOR16), 4, 0);
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

			col.pszText = _T("");
			col.cx = 20;
			ListView_InsertColumn(hwndList, 1, &col);

			col.pszText = TranslateT("Plugin");
			col.cx = 70;
			ListView_InsertColumn(hwndList, 2, &col);

			col.pszText = TranslateT("Name");
			col.cx = 70;//max = 220;
			ListView_InsertColumn(hwndList, 3, &col);

			col.pszText = TranslateT("Version");
			col.cx = 70;
			ListView_InsertColumn(hwndList, 4, &col);

			// XXX: Won't work on windows 95 without IE3+ or 4.70
			ListView_SetExtendedListViewStyleEx( hwndList, 0, LVS_EX_SUBITEMIMAGES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT );
			// scan the plugin dir for plugins, cos
			enumPlugins( dialogListPlugins, ( WPARAM )hwndDlg, ( LPARAM )hwndList );
			// sort out the headers

			ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE ); // dll name
			int w = ListView_GetColumnWidth( hwndList, 2 );
			if (w > 110) {
				ListView_SetColumnWidth( hwndList, 2, 110 );
				w = 110;
			}
			int max = w < 110 ? 199+110-w:199;
			ListView_SetColumnWidth( hwndList, 3, LVSCW_AUTOSIZE ); // short name
			w = ListView_GetColumnWidth( hwndList, 3 );
			if (w > max)
				ListView_SetColumnWidth( hwndList, 3, max );
		}
		return TRUE;

	case WM_NOTIFY:
		if ( lParam ) {
			NMLISTVIEW * hdr = (NMLISTVIEW *) lParam;
			if ( hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible(hdr->hdr.hwndFrom)) {
				if (hdr->uOldState != 0 && (hdr->uNewState == 0x1000 || hdr->uNewState == 0x2000 )) {
					HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);

					LVITEM it;
					it.mask = LVIF_PARAM | LVIF_STATE;
					it.iItem = hdr->iItem;
					if ( !ListView_GetItem( hwndList, &it ))
						break;

					PluginListItemData* dat = ( PluginListItemData* )it.lParam;
					if ( dat->flags == DEFMOD_DB ) {
						ListView_SetItemState(hwndList, hdr->iItem, 0x3000, LVIS_STATEIMAGEMASK);
						return FALSE;
					}
					// if enabling and replaces, find all other replaces and toggle off
					if (( hdr->uNewState & 0x2000) && dat->flags != 0 )  {
						for ( int iRow=0; iRow != -1; ) {
							if ( iRow != hdr->iItem ) {
								LVITEM dt;
								dt.mask = LVIF_PARAM;
								dt.iItem = iRow;
								if ( ListView_GetItem( hwndList, &dt )) {
									PluginListItemData* dat2 = ( PluginListItemData* )dt.lParam;
									if ( dat2->flags == dat->flags ) {
										// the lParam is unset, so when the check is unset the clist block doesnt trigger
										int lParam = dat2->flags;
										dat2->flags = 0;
										ListView_SetItemState(hwndList, iRow, 0x1000, LVIS_STATEIMAGEMASK );
										dat2->flags = lParam;
							}	}	}

							iRow = ListView_GetNextItem( hwndList, iRow, LVNI_ALL );
					}	}

					ShowWindow( GetDlgItem(hwndDlg, IDC_RESTART ), TRUE );
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					break;
				}

				if ( hdr->iItem != -1 ) {
					TCHAR buf[1024];
					int sel = hdr->uNewState & LVIS_SELECTED;
					HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
					LVITEM lvi = { 0 };
					lvi.mask = LVIF_PARAM;
					lvi.iItem = hdr->iItem;
					if ( ListView_GetItem( hwndList, &lvi )) {
						PluginListItemData* dat = ( PluginListItemData* )lvi.lParam;

						ListView_GetItemText(hwndList, hdr->iItem, 1, buf, SIZEOF(buf));
						SetWindowText(GetDlgItem(hwndDlg, IDC_PLUGININFOFRAME), sel ? buf : _T(""));

						SetWindowTextA(GetDlgItem(hwndDlg, IDC_PLUGINAUTHOR), sel ? dat->author : "" );
						SetWindowTextA(GetDlgItem(hwndDlg, IDC_PLUGINEMAIL), sel ? dat->authorEmail : "" );
						{
							TCHAR* p = LangPackPcharToTchar( dat->description );
							SetWindowText(GetDlgItem(hwndDlg, IDC_PLUGINLONGINFO), sel ? p : _T(""));
							mir_free( p );
						}
						SetWindowTextA(GetDlgItem(hwndDlg, IDC_PLUGINCPYR), sel ? dat->copyright : "" );
						SetWindowTextA(GetDlgItem(hwndDlg, IDC_PLUGINURL), sel ? dat->homepage : "" );
						if (equalUUID(miid_last, dat->uuid))
							SetWindowText(GetDlgItem(hwndDlg, IDC_PLUGINPID), sel ? TranslateT("<none>") : _T(""));
						else {
							char szUID[128];
							uuidToString( dat->uuid, szUID, sizeof(szUID));
							SetWindowTextA(GetDlgItem(hwndDlg, IDC_PLUGINPID), sel ? szUID : "" );
			}	}	}	}

			if ( hdr->hdr.code == PSN_APPLY ) {
				HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
				TCHAR buf[1024];
				for (int iRow=0; iRow != -1; ) {
					ListView_GetItemText(hwndList, iRow, 2, buf, SIZEOF(buf));
					int iState = ListView_GetItemState(hwndList, iRow, LVIS_STATEIMAGEMASK);
					SetPluginOnWhiteList(buf, (iState & 0x2000) ? 1 : 0);
					iRow = ListView_GetNextItem(hwndList, iRow, LVNI_ALL);
		}	}	}
		break;

	case WM_COMMAND:
		if ( HIWORD(wParam) == STN_CLICKED ) {
			switch (LOWORD(wParam)) {
			case IDC_PLUGINEMAIL:
			case IDC_PLUGINURL:
			{
				char buf[512];
				char *p = &buf[7];
				lstrcpyA(buf, "mailto:");
				if ( GetWindowTextA(GetDlgItem(hwndDlg, LOWORD(wParam)), p, SIZEOF(buf) - 7))
					CallService(MS_UTILS_OPENURL, 0, (LPARAM) (LOWORD(wParam) == IDC_PLUGINEMAIL ? buf : p) );
				break;
			}
			case IDC_GETMOREPLUGINS:
				CallService(MS_UTILS_OPENURL, 0, (LPARAM) "http://addons.miranda-im.org/index.php?action=display&id=1" );
				break;
		}	}
		break;

	case WM_DESTROY:
		RemoveAllItems( GetDlgItem( hwndDlg, IDC_PLUGLIST ));
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int PluginOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = hMirandaInst;
	odp.pfnDlgProc = DlgPluginOpt;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PLUGINS);
	odp.position = 1300000000;
	odp.pszTitle = LPGEN("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
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