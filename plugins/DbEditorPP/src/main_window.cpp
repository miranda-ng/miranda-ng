#include "stdafx.h"

HWND hwnd2mainWindow;

static HIMAGELIST hImg = 0;

int g_Hex;
int g_Mode;
int g_Order;
int g_Inline;

extern SettingListInfo info;
extern struct ColumnsSettings csSettingList[];
extern HWND hwnd2List;
extern HWND hwnd2Tree;

#define GC_SPLITTERMOVED		(WM_USER+101)

void moduleListWM_NOTIFY(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SettingsListWM_NOTIFY(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int DialogResize(HWND, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_MODULES:
		urc->rcItem.right = lParam - 3;
		urc->rcItem.top = 0;
		urc->rcItem.left = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SPLITTER:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.right = lParam;
		urc->rcItem.left = lParam - 3;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SETTINGS:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.left = lParam;
		urc->rcItem.right = urc->dlgNewSize.cx;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_VARS:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.left = 0;
		urc->rcItem.right = urc->dlgNewSize.cx;
		return RD_ANCHORY_CUSTOM | RD_ANCHORX_CUSTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			SetCursor(rc.right > rc.bottom ? LoadCursor(NULL, IDC_SIZENS) : LoadCursor(NULL, IDC_SIZEWE));
		}
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			RECT rc;
			GetClientRect(hwnd, &rc);
			SendMessage(GetParent(hwnd), GC_SPLITTERMOVED, rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	}

	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

LRESULT CALLBACK ModuleTreeSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONDOWN:
		{
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(GetMessagePos());
			hti.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(hwnd, &hti.pt);

			if (TreeView_HitTest(hwnd, &hti)) {
				if (hti.flags&TVHT_ONITEM)
					TreeView_SelectItem(hwnd, hti.hItem);
			}
		}
		break;

	case WM_CHAR:
		if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 6)
			newFindWindow();
		break;

	case WM_KEYUP:
		if (wParam == VK_DELETE || wParam == VK_F2 || wParam == VK_F5 || wParam == VK_F3) {
			TVITEM tvi;
			TCHAR text[FLD_SIZE];
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = TreeView_GetSelection(hwnd);
			tvi.pszText = text;
			tvi.cchTextMax = _countof(text);
			if (TreeView_GetItem(hwnd, &tvi) && tvi.lParam) {
				ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				MCONTACT hContact = mtis->hContact;
				if (wParam == VK_DELETE) {
					if ((mtis->type) & MODULE) {
						if (deleteModule(hContact, _T2A(text), 1)) {
							mir_free(mtis);
							TreeView_DeleteItem(hwnd, tvi.hItem);
						}
					}
					else if ((mtis->type == CONTACT) && hContact) {
						if (db_get_b(NULL, "CList", "ConfirmDelete", 1)) {
							TCHAR str[MSG_SIZE];
							mir_sntprintf(str, TranslateT("Are you sure you want to delete contact \"%s\"?"), text);
							if (dlg(str, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
								break;
						}
						CallService(MS_DB_CONTACT_DELETE, hContact, 0);
						freeTree(mtis->hContact);
						TreeView_DeleteItem(hwnd, tvi.hItem);
					}
				}
				else if (wParam == VK_F2 && mtis->type == MODULE)
					TreeView_EditLabel(hwnd, tvi.hItem);
				else if (wParam == VK_F5) {
					refreshTree(1);
					break;
				}
				else if (wParam == VK_F3) {
					newFindWindow();
					break;
				}
			}
		}
		break;
	}
	return mir_callNextSubclass(hwnd, ModuleTreeSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK SettingListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 6)
			newFindWindow();
		break;

	case WM_KEYDOWN:

		if (wParam == VK_F5) {
			PopulateSettings(info.hContact, info.module);
		}
		else if (wParam == VK_F3)
			newFindWindow();
		else
			if (wParam == VK_DELETE || (wParam == VK_F2 && ListView_GetSelectedCount(hwnd) == 1)) {

				char setting[FLD_SIZE];
				int idx = ListView_GetSelectionMark(hwnd);
				if (idx == -1) return 0;
				ListView_GetItemTextA(hwnd, idx, 0, setting, _countof(setting));

				if (wParam == VK_F2)
					editSetting(info.hContact, info.module, setting);
				else if (wParam == VK_DELETE)
					DeleteSettingsFromList(info.hContact, info.module, setting);

				return 0;
			}
		break;
	}
	return mir_callNextSubclass(hwnd, SettingListSubclassProc, msg, wParam, lParam);
}

INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			hwnd2mainWindow = hwnd;
			hwnd2Tree = GetDlgItem(hwnd, IDC_MODULES);
			hwnd2List = GetDlgItem(hwnd, IDC_SETTINGS);

			LoadResidentSettings();

			// image list
			hImg = LoadIcons();

			// do the icon
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(ICO_REGEDIT)));
			SetWindowText(hwnd, TranslateT("Database Editor++"));

			// setup the splitter
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA, (LONG_PTR)db_get_w(NULL, modname, "Splitter", 200));
			SendMessage(hwnd, GC_SPLITTERMOVED, 0, 0);
			mir_subclassWindow(GetDlgItem(hwnd, IDC_SPLITTER), SplitterSubclassProc);

			// module tree
			mir_subclassWindow(hwnd2Tree, ModuleTreeSubclassProc);
			TreeView_SetImageList(hwnd2Tree, hImg, TVSIL_NORMAL);

			//setting list
			mir_subclassWindow(hwnd2List, SettingListSubclassProc);
			ListView_SetExtendedListViewStyle(hwnd2List, 32 | LVS_EX_SUBITEMIMAGES | LVS_EX_LABELTIP); //LVS_EX_GRIDLINES
			loadListSettings(hwnd2List, csSettingList);
			ListView_SetImageList(hwnd2List, hImg, LVSIL_SMALL);

			HMENU hMenu = GetMenu(hwnd);
			TranslateMenu(hMenu);
			for (int i = 0; i < 6; i++)
				TranslateMenu(GetSubMenu(hMenu, i));

			Utils_RestoreWindowPosition(hwnd, NULL, modname, "Main_");
			if (db_get_b(NULL, modname, "Maximized", 0))
				ShowWindow(hwnd, SW_SHOWMAXIMIZED);

			g_Inline = !db_get_b(NULL, modname, "DontAllowInLineEdit", 1);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_INLINE_EDIT, MF_BYCOMMAND | (g_Inline ? MF_CHECKED : MF_UNCHECKED));

			g_Mode = MODE_ALL;
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_CHECKED);

			g_Hex = db_get_b(NULL, modname, "HexMode", 0);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_BYTE_HEX, MF_BYCOMMAND | ((g_Hex & HEX_BYTE) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_WORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_WORD) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_DWORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_DWORD) ? MF_CHECKED : MF_UNCHECKED));

			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SAVE_POSITION, MF_BYCOMMAND | (db_get_b(NULL, modname, "RestoreOnOpen", 1) ? MF_CHECKED : MF_UNCHECKED));

			g_Order = db_get_b(NULL, modname, "SortMode", 1);
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SORT_ORDER, MF_BYCOMMAND | (g_Order ? MF_CHECKED : MF_UNCHECKED));

			int restore;
			if (hRestore)
				restore = 3;
			else if (db_get_b(NULL, modname, "RestoreOnOpen", 1))
				restore = 2;
			else
				restore = 0;

			refreshTree(restore);
		}
		return TRUE;

	case GC_SPLITTERMOVED:
		{
			int splitterPos = GetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA);

			RECT rc2;
			GetWindowRect(hwnd, &rc2);

			if ((HWND)lParam == GetDlgItem(hwnd, IDC_SPLITTER)) {
				RECT rc;
				GetClientRect(hwnd, &rc);
				POINT pt = { wParam, 0 };
				ScreenToClient(hwnd, &pt);

				splitterPos = rc.left + pt.x + 1;
				if (splitterPos < 150)
					splitterPos = 150;
				if (splitterPos > rc2.right - rc2.left - 150)
					splitterPos = rc2.right - rc2.left - 150;
				SetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA, splitterPos);
				db_set_w(NULL, modname, "Splitter", (WORD)splitterPos);
			}
			PostMessage(hwnd, WM_SIZE, 0, 0);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			int splitterPos = GetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA);
			mmi->ptMinTrackSize.x = splitterPos + 150;
			mmi->ptMinTrackSize.y = 300;
		}
		return 0;

	case WM_MOVE:
	case WM_SIZE:
		Utils_ResizeDialog(hwnd, hInst, MAKEINTRESOURCEA(IDD_MAIN), DialogResize, GetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA));
		break;

	case WM_DESTROY: // free our shit!
		if (db_get_b(NULL, modname, "RestoreOnOpen", 1)) {
			HTREEITEM item;

			if (item = TreeView_GetSelection(hwnd2Tree)) {
				int type = MODULE;
				TCHAR text[FLD_SIZE];
				TVITEM tvi = { 0 };
				tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
				tvi.pszText = text;
				tvi.cchTextMax = _countof(text);
				tvi.hItem = item;
				if (TreeView_GetItem(hwnd2Tree, &tvi)) {
					MCONTACT hContact = 0;
					if (tvi.lParam) {
						ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
						hContact = mtis->hContact;
						type = mtis->type;
					}

					db_set_dw(NULL, modname, "LastContact", hContact);

					if (type == CONTACT)
						db_set_s(NULL, modname, "LastModule", "");
					else
						db_set_s(NULL, modname, "LastModule", _T2A(text));
				}
				else {
					db_unset(NULL, modname, "LastContact");
					db_unset(NULL, modname, "LastModule");
				}

				int pos = ListView_GetSelectionMark(hwnd2List);

				if (pos != -1) {
					char data[FLD_SIZE];
					ListView_GetItemTextA(hwnd2List, pos, 0, data, _countof(data));
					db_set_s(NULL, modname, "LastSetting", data);
				}
				else
					db_unset(NULL, modname, "LastSetting");
			}
		}

		db_set_b(NULL, modname, "HexMode", (byte)g_Hex);
		db_set_b(NULL, modname, "SortMode", (byte)g_Order);
		db_set_b(NULL, modname, "DontAllowInLineEdit", (byte)!g_Inline);

		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hwnd, &wp);
			if (wp.flags == WPF_RESTORETOMAXIMIZED) {
				db_set_b(NULL, modname, "Maximized", 1);
				ShowWindow(hwnd, SW_SHOWNOACTIVATE);
			}
			else db_set_b(NULL, modname, "Maximized", 0);
			
			Utils_SaveWindowPosition(hwnd, NULL, modname, "Main_");
			ShowWindow(hwnd, SW_HIDE);
		}

		saveListSettings(hwnd2List, csSettingList);
		ClearListView();

		freeTree(0);

		hwnd2mainWindow = NULL;
		hwnd2Tree = NULL;
		hwnd2List = NULL;

		if (hImg) {
			ImageList_Destroy(hImg);
			hImg = NULL;
		}

		FreeResidentSettings();

		if (g_bServiceMode)
			PostQuitMessage(0);
		return 0;

	case WM_COMMAND:
		if (GetKeyState(VK_ESCAPE) & 0x8000)
			return TRUE; // this needs to be changed to c if htere is a label edit happening..

		switch (LOWORD(wParam)) {
		case MENU_REFRESH_MODS:
			refreshTree(1);
			break;

		case MENU_REFRESH_SETS:
			TVITEM tvi;

			TCHAR text[FLD_SIZE];
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = TreeView_GetSelection(hwnd2Tree);
			tvi.pszText = text; // modulename
			tvi.cchTextMax = _countof(text);
			if (TreeView_GetItem(hwnd2Tree, &tvi)) {
				ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				if (mtis && (mtis->type == MODULE))
					PopulateSettings(mtis->hContact, _T2A(text));
				else
					ClearListView();
			}
			break;

		// watches
		case MENU_VIEW_WATCHES:
			openWatchedVarWindow();
			break;
		case MENU_REMALL_WATCHES:
			freeAllWatches();
			break;
		case MENU_EXPORTDB: // all db
			exportDB(INVALID_CONTACT_ID, 0);
			break;
		case MENU_EXPORTCONTACT: // all contacts
			exportDB(INVALID_CONTACT_ID, "");
			break;
		case MENU_EXPORTMODULE: // all settings
			exportDB(NULL, 0);
			break;
		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(NULL, NULL);
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(NULL);
			break;
		case MENU_EXIT:
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		case MENU_DELETE:
			deleteModuleDlg();
			break;
		case MENU_FINDANDREPLACE:
			newFindWindow();
			break;
		case MENU_FIX_RESIDENT:
			if (dlg(TranslateT("Delete resident settings from database?"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				int cnt = fixResidentSettings();
				TCHAR text[MSG_SIZE];
				mir_sntprintf(text, TranslateT("%d orphaned items deleted."), cnt);
				msg(text);
			}
			break;
		case MENU_FILTER_ALL:
			if (g_Mode != MODE_ALL) {
				HMENU hMenu = GetMenu(hwnd);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_UNCHECKED);
				g_Mode = MODE_ALL;
				refreshTree(1);
			}
			break;
		case MENU_FILTER_LOADED:
			if (g_Mode != MODE_LOADED) {
				HMENU hMenu = GetMenu(hwnd);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_UNCHECKED);
				g_Mode = MODE_LOADED;
				refreshTree(1);
			}
			break;
		case MENU_FILTER_UNLOADED:
			if (g_Mode != MODE_UNLOADED) {
				HMENU hMenu = GetMenu(hwnd);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_CHECKED);
				g_Mode = MODE_UNLOADED;
				refreshTree(1);
			}
			break;
		case MENU_BYTE_HEX:
			g_Hex ^= HEX_BYTE;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_BYTE_HEX, MF_BYCOMMAND | ((g_Hex & HEX_BYTE) ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_WORD_HEX:
			g_Hex ^= HEX_WORD;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_WORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_WORD) ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_DWORD_HEX:
			g_Hex ^= HEX_DWORD;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_DWORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_DWORD) ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_SAVE_POSITION:
			{
				BOOL save = !db_get_b(NULL, modname, "RestoreOnOpen", 1);
				CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SAVE_POSITION, MF_BYCOMMAND | (save ? MF_CHECKED : MF_UNCHECKED));
				db_set_b(NULL, modname, "RestoreOnOpen", (byte)save);
			}
			break;
		case MENU_INLINE_EDIT:
			g_Inline = !g_Inline;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_INLINE_EDIT, MF_BYCOMMAND | (g_Inline ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_SORT_ORDER:
			g_Order = !g_Order;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SORT_ORDER, MF_BYCOMMAND | (g_Order ? MF_CHECKED : MF_UNCHECKED));
			refreshTree(1);
			break;
		case MENU_OPEN_OPTIONS:
			OPENOPTIONSDIALOG odp = { 0 };
			odp.cbSize = sizeof(odp);
			odp.pszGroup = "Database";
			odp.pszPage = modFullname;
			odp.pszTab = 0;
			Options_Open(&odp);
			break;
		}
		return TRUE; // case WM_COMMAND

	case WM_NOTIFY:
		switch (LOWORD(wParam)) {
		case IDC_MODULES:
			moduleListWM_NOTIFY(hwnd, msg, wParam, lParam);
			break;
		case IDC_SETTINGS:
			SettingsListWM_NOTIFY(hwnd, msg, wParam, lParam);
			break;
		}
		return TRUE; // case WM_NOTIFY

	case WM_FINDITEM:
		ItemInfo *ii = (ItemInfo *)wParam;
		HTREEITEM hItem = findItemInTree(ii->hContact, ii->module);
		if (hItem) {
			TreeView_SelectItem(hwnd2Tree, hItem);
			TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
			if (ii->type != FW_MODULE)
				SelectSetting(ii->setting);
		}
		break;
	}
	return 0;
}


void openMainWindow()
{
	CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, MainDlgProc);
}
