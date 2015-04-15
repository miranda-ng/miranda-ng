#include "headers.h"

HWND hwnd2mainWindow;
int Order;
HIMAGELIST himl2;
int Hex;

#define GC_SPLITTERMOVED		(WM_USER+101)

extern BOOL bServiceMode;

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
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, FindWindowDlgProc);
		break;

	case WM_KEYUP:
		if (wParam == VK_DELETE || wParam == VK_F2 || wParam == VK_F5 || wParam == VK_F3) {
			TVITEM tvi;
			char module[256];
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = TreeView_GetSelection(hwnd);
			tvi.pszText = module;
			tvi.cchTextMax = SIZEOF(module);
			if (TreeView_GetItem(hwnd, &tvi) && tvi.lParam) {
				ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				MCONTACT hContact = mtis->hContact;
				if (wParam == VK_DELETE) {
					if ((mtis->type) & MODULE) {
						if (deleteModule(module, hContact, 0)) {
							mir_free(mtis);
							TreeView_DeleteItem(hwnd, tvi.hItem);
						}
					}
					else if ((mtis->type == CONTACT) && hContact) {
						if (db_get_b(NULL, "CList", "ConfirmDelete", 1)) {
							char msg[1024];
							mir_snprintf(msg, SIZEOF(msg), Translate("Are you sure you want to delete contact \"%s\"?"), module);
							if (MessageBox(0, msg, Translate("Confirm contact delete"), MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
								break;
						}
						CallService(MS_DB_CONTACT_DELETE, hContact, 0);
						freeTree(hwnd, mtis->hContact);
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
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, FindWindowDlgProc);
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
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, FindWindowDlgProc);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_DELETE || wParam == VK_F5 || (wParam == VK_F2 && ListView_GetSelectedCount(hwnd) == 1)) {
			char *module, setting[256];
			MCONTACT hContact;
			SettingListInfo* sli = (SettingListInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (!sli)
				break;
			hContact = sli->hContact;
			module = sli->module;
			ListView_GetItemText(hwnd, ListView_GetSelectionMark(hwnd), 0, setting, SIZEOF(setting));

			if (wParam == VK_F2)
				editSetting(hContact, module, setting);
			else if (wParam == VK_F5) {
				char *szModule = mir_tstrdup(module); // need to do this, otheriwse the setlist stays empty
				PopulateSettings(hwnd, hContact, szModule);
				mir_free(szModule);
			}
			else if (wParam == VK_DELETE)
				DeleteSettingsFromList(hwnd, hContact, module, setting);

			return 0;
		}
		else if (wParam == VK_F3)
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, FindWindowDlgProc);
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
			// do the icon
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(ICO_REGEDIT)));
			SetWindowText(hwnd, TranslateT("Database Editor++"));

			// setup the splitter
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA, (LONG_PTR)db_get_w(NULL, modname, "Splitter", 300));
			SendMessage(hwnd, GC_SPLITTERMOVED, 0, 0);
			mir_subclassWindow(GetDlgItem(hwnd, IDC_SPLITTER), SplitterSubclassProc);
			// module tree
			TreeView_SetUnicodeFormat(GetDlgItem(hwnd, IDC_MODULES), TRUE);
			mir_subclassWindow(GetDlgItem(hwnd, IDC_MODULES), ModuleTreeSubclassProc);
			//setting list
			setupSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
			mir_subclassWindow(GetDlgItem(hwnd, IDC_SETTINGS), SettingListSubclassProc);

			HMENU hMenu = GetMenu(hwnd);
			TranslateMenu(hMenu);
			for (int i = 0; i < 6; i++)
				TranslateMenu(GetSubMenu(hMenu, i));

			// move the dialog to the users position
			MoveWindow(hwnd, db_get_dw(NULL, modname, "x", 0), db_get_dw(NULL, modname, "y", 0), db_get_dw(NULL, modname, "width", 500), db_get_dw(NULL, modname, "height", 250), 0);
			if (db_get_b(NULL, modname, "Maximised", 0)) {
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				wp.flags = WPF_RESTORETOMAXIMIZED;
				wp.showCmd = SW_SHOWMAXIMIZED;

				SetWindowPlacement(hwnd, &wp);
			}
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			Mode = MODE_ALL;
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_CHECKED);

			Hex = db_get_b(NULL, modname, "HexMode", 0);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_BYTE_HEX, MF_BYCOMMAND | ((Hex & HEX_BYTE) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_WORD_HEX, MF_BYCOMMAND | ((Hex & HEX_WORD) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_DWORD_HEX, MF_BYCOMMAND | ((Hex & HEX_DWORD) ? MF_CHECKED : MF_UNCHECKED));

			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SAVE_POSITION, MF_BYCOMMAND | (db_get_b(NULL, modname, "RestoreOnOpen", 1) ? MF_CHECKED : MF_UNCHECKED));

			Order = db_get_b(NULL, modname, "SortMode", 1);
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SORT_ORDER, MF_BYCOMMAND | (Order ? MF_CHECKED : MF_UNCHECKED));

			// image list
			int numberOfIcons = 0;
			himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
			if (himl != NULL) {
				if (AddIconToList(himl, LoadSkinnedDBEIcon(ICO_SETTINGS)))
					numberOfIcons++;
				if (AddIconToList(himl, LoadSkinnedDBEIcon(ICO_KNOWN)))
					numberOfIcons++;
				if (AddIconToList(himl, LoadSkinnedDBEIcon(ICO_KNOWNOPEN)))
					numberOfIcons++;
				if (AddIconToList(himl, LoadSkinnedDBEIcon(ICO_CONTACTS)))
					numberOfIcons++;
				if (AddIconToList(himl, LoadSkinnedDBEIcon(ICO_OFFLINE)))
					numberOfIcons++;
				if (AddIconToList(himl, LoadSkinnedDBEIcon(ICO_ONLINE)))
					numberOfIcons++;

				if (numberOfIcons < DEF_ICON + 1) {
					if (numberOfIcons)
						ImageList_Destroy(himl);
					himl = NULL;
				}

				AddProtoIconsToList(himl, numberOfIcons);

				himl2 = NULL;
				numberOfIcons = 0;

				if (himl2 = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 5, 0)) {
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_BINARY)))
						numberOfIcons++;
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_BYTE)))
						numberOfIcons++;
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_WORD)))
						numberOfIcons++;
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_DWORD)))
						numberOfIcons++;
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_STRING)))
						numberOfIcons++;
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_UNICODE)))
						numberOfIcons++;
					if (AddIconToList(himl2, LoadSkinnedDBEIcon(ICO_HANDLE)))
						numberOfIcons++;

					if (numberOfIcons < 7) {
						if (numberOfIcons)
							ImageList_Destroy(himl2);
						himl2 = NULL;
					}
				}
			}

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
			if (splitterPos < 65)
				splitterPos = 65;
			if (splitterPos > rc2.right - rc2.left - 65)
				splitterPos = rc2.right - rc2.left - 65;
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

		if (splitterPos + 40 > 200)
			mmi->ptMinTrackSize.x = splitterPos + 65;
		else
			mmi->ptMinTrackSize.x = 200;
		mmi->ptMinTrackSize.y = 150;
	}
	return 0;

	case WM_MOVE:
	case WM_SIZE:
	{
		UTILRESIZEDIALOG urd;

		memset(&urd, 0, sizeof(urd));
		urd.cbSize = sizeof(urd);
		urd.hInstance = hInst;
		urd.hwndDlg = hwnd;
		urd.lParam = (LPARAM)GetWindowLongPtr(GetDlgItem(hwnd, IDC_SPLITTER), GWLP_USERDATA);
		urd.lpTemplate = MAKEINTRESOURCE(IDD_MAIN);
		urd.pfnResizer = DialogResize;
		CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

		if (msg == WM_SIZE && wParam == SIZE_MAXIMIZED || wParam == SIZE_MINIMIZED)
			db_set_b(NULL, modname, "Maximised", 1);
		else if (msg == WM_SIZE && wParam == SIZE_RESTORED)
			db_set_b(NULL, modname, "Maximised", 0);
	}
	break;

	case WM_DESTROY: // free our shit!
		if (db_get_b(NULL, modname, "RestoreOnOpen", 1)) {
			HTREEITEM item;
			HWND hwnd2Tree = GetDlgItem(hwnd, IDC_MODULES);
			char module[256] = { 0 };
			if (item = TreeView_GetSelection(hwnd2Tree)) {
				int type = MODULE;

				TVITEM tvi = { 0 };
				tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
				tvi.pszText = module;
				tvi.cchTextMax = SIZEOF(module);
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
						db_set_s(NULL, modname, "LastModule", module);
				}
				else {
					db_unset(NULL, modname, "LastContact");
					db_unset(NULL, modname, "LastModule");
				}

				HWND hwnd2Settings = GetDlgItem(hwnd, IDC_SETTINGS);
				int pos = ListView_GetSelectionMark(hwnd2Settings);

				if (pos != -1) {
					char text[256];
					ListView_GetItemText(hwnd2Settings, pos, 0, text, SIZEOF(text));
					db_set_s(NULL, modname, "LastSetting", text);
				}
				else
					db_unset(NULL, modname, "LastSetting");
			}
		}
		db_set_b(NULL, modname, "HexMode", (byte)Hex);
		db_set_b(NULL, modname, "SortMode", (byte)Order);
		saveListSettings(GetDlgItem(hwnd, IDC_SETTINGS));
		hwnd2mainWindow = 0;
		ClearListview(GetDlgItem(hwnd, IDC_SETTINGS));
		freeTree(GetDlgItem(hwnd, IDC_MODULES), 0);
		if (himl)
			ImageList_Destroy(himl);
		if (himl2)
			ImageList_Destroy(himl2);

		if (!db_get_b(NULL, modname, "Maximised", 0)) {
			RECT rc;
			GetWindowRect(hwnd, &rc);
			db_set_dw(NULL, modname, "x", rc.left);
			db_set_dw(NULL, modname, "y", rc.top);
			db_set_dw(NULL, modname, "width", rc.right - rc.left);
			db_set_dw(NULL, modname, "height", rc.bottom - rc.top);
		}

		if (hwnd2importWindow) {
			DestroyWindow(hwnd2importWindow);
			hwnd2importWindow = 0;
		}

		if (bServiceMode)
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
		{
			TVITEM tvi;
			ModuleTreeInfoStruct *mtis;
			char module[256];
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd, IDC_MODULES));
			tvi.pszText = module;
			tvi.cchTextMax = SIZEOF(module);
			if (!TreeView_GetItem(GetDlgItem(hwnd, IDC_MODULES), &tvi)) break;
			if (tvi.lParam) {
				mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				if (mtis->type == MODULE)
					PopulateSettings(GetDlgItem(hwnd, IDC_SETTINGS), mtis->hContact, module);
				else
					ClearListview(GetDlgItem(hwnd, IDC_SETTINGS));
			}
			else
				ClearListview(GetDlgItem(hwnd, IDC_SETTINGS));
		}
		break;
		///////////////////////// // watches
		case MENU_VIEW_WATCHES:
			if (!hwnd2watchedVarsWindow) // so only opens 1 at a time
				hwnd2watchedVarsWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_WATCH_DIAG), 0, WatchDlgProc);
			else
				SetForegroundWindow(hwnd2watchedVarsWindow);
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
			ImportSettingsFromFileMenuItem(NULL, "");
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(NULL);
			break;
		case MENU_EXIT:
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		case MENU_DELETE:
			deleteModuleGui();
			break;
		case MENU_FINDANDREPLACE:
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, FindWindowDlgProc);
			break;
		case MENU_FILTER_ALL:
			if (Mode != MODE_ALL) {
				HMENU hMenu = GetMenu(hwnd);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_UNCHECKED);
				Mode = MODE_ALL;
				refreshTree(1);
			}
			break;
		case MENU_FILTER_LOADED:
			if (Mode != MODE_LOADED) {
				HMENU hMenu = GetMenu(hwnd);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_UNCHECKED);
				Mode = MODE_LOADED;
				refreshTree(1);
			}
			break;
		case MENU_FILTER_UNLOADED:
			if (Mode != MODE_UNLOADED) {
				HMENU hMenu = GetMenu(hwnd);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_CHECKED);
				Mode = MODE_UNLOADED;
				refreshTree(1);
			}
			break;
		case MENU_BYTE_HEX:
			Hex ^= HEX_BYTE;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_BYTE_HEX, MF_BYCOMMAND | ((Hex & HEX_BYTE) ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_WORD_HEX:
			Hex ^= HEX_WORD;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_WORD_HEX, MF_BYCOMMAND | ((Hex & HEX_WORD) ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_DWORD_HEX:
			Hex ^= HEX_DWORD;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_DWORD_HEX, MF_BYCOMMAND | ((Hex & HEX_DWORD) ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MENU_SAVE_POSITION:
		{
			BOOL save = !db_get_b(NULL, modname, "RestoreOnOpen", 1);
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SAVE_POSITION, MF_BYCOMMAND | (save ? MF_CHECKED : MF_UNCHECKED));
			db_set_b(NULL, modname, "RestoreOnOpen", (byte)save);
		}
		break;
		case MENU_SORT_ORDER:
			Order = !Order;
			CheckMenuItem(GetSubMenu(GetMenu(hwnd), 5), MENU_SORT_ORDER, MF_BYCOMMAND | (Order ? MF_CHECKED : MF_UNCHECKED));
			refreshTree(1);
			break;
		case MENU_OPEN_OPTIONS:
			OPENOPTIONSDIALOG odp = { 0 };
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
		HWND hwnd2Settings = GetDlgItem(hwnd, IDC_SETTINGS);
		int hItem = findItemInTree(GetDlgItem(hwnd, IDC_MODULES), ii->hContact, ii->module);
		if (hItem != -1) {
			TreeView_SelectItem(GetDlgItem(hwnd, IDC_MODULES), (HTREEITEM)hItem);
			if (ii->type != FW_MODULE) {
				LVITEM lvItem;
				LVFINDINFO lvfi;

				lvfi.flags = LVFI_STRING;
				lvfi.psz = ii->setting;
				lvfi.vkDirection = VK_DOWN;

				lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
				lvItem.iItem = ListView_FindItem(hwnd2Settings, -1, &lvfi);
				lvItem.iSubItem = 0;
				if (lvItem.iItem >= 0)
					ListView_SetItemState(hwnd2Settings, lvItem.iItem, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
		break;
	}
	return 0;
}
