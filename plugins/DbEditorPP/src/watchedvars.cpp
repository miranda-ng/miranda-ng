#include "stdafx.h"

HWND hwnd2watchedVarsWindow = nullptr;

static WatchListArrayStruct WatchListArray = {};

static int lastColumn = -1;

ColumnsSettings csWatchList[] = {
	{ LPGENW("Contact"), 0, "Watch0width", 100 },
	{ LPGENW("Module"),  1, "Watch1width", 100 },
	{ LPGENW("Setting"), 2, "Watch2width", 100 },
	{ LPGENW("Value"),   3, "Watch3width", 200 },
	{ LPGENW("Type"),    4, "Watch4width", 75 },
	{nullptr}
};



int WatchedArrayIndex(MCONTACT hContact, const char *module, const char *setting, int strict)
{
	for (int i = 0; i < WatchListArray.count; i++) {
		if (hContact == WatchListArray.item[i].hContact)
			if (!mir_strcmp(module, WatchListArray.item[i].module))
				// empty setting = module watching
				if ((!strict && !WatchListArray.item[i].setting) || !mir_strcmp(setting, WatchListArray.item[i].setting))
					return i;
	}
	return -1;
}


int addSettingToWatchList(MCONTACT hContact, const char *module, const char *setting)
{
	if (WatchListArray.count == WatchListArray.size) {
		WatchListArray.size += 32;
		WatchListArray.item = (struct DBsetting*)mir_realloc(WatchListArray.item, sizeof(struct DBsetting)*WatchListArray.size);
	}
	if (!WatchListArray.item)
		return 0;

	db_get_s(hContact, module, setting, &(WatchListArray.item[WatchListArray.count].dbv), 0);

	WatchListArray.item[WatchListArray.count].hContact = hContact;
	WatchListArray.item[WatchListArray.count].module = mir_strdup(module);

	if (setting)
		WatchListArray.item[WatchListArray.count].setting = mir_strdup(setting);
	else
		WatchListArray.item[WatchListArray.count].setting = nullptr;

	WatchListArray.count++;
	return 1;
}


void freeWatchListItem(int item)
{
	if (item < 0 || item >= WatchListArray.count) return;

	if (WatchListArray.item[item].module)
		mir_free(WatchListArray.item[item].module);

	WatchListArray.item[item].module = nullptr;

	if (WatchListArray.item[item].setting)
		mir_free(WatchListArray.item[item].setting);

	WatchListArray.item[item].setting = nullptr;
	db_free(&(WatchListArray.item[item].dbv));
	WatchListArray.item[item].hContact = 0;
}


void addwatchtolist(HWND hwnd, struct DBsetting *lParam)
{
	DBVARIANT *dbv = &(lParam->dbv);

	if (!lParam->module) return;

	LVITEM lvItem;
	lvItem.lParam = (LPARAM)lParam->hContact;
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;

	if (!lParam->setting) // add every item in the module and return
	{
		ModuleSettingLL settinglist;
		if (IsModuleEmpty(lParam->hContact, lParam->module) || !EnumSettings(lParam->hContact, lParam->module, &settinglist))
			return;

		struct DBsetting dummy = {};
		dummy.hContact = lParam->hContact;
		dummy.module = lParam->module;

		for (ModSetLinkLinkItem *setting = settinglist.first; setting; setting = setting->next) {
			dummy.setting = setting->name;
			addwatchtolist(hwnd, &dummy);

		}
		FreeModuleSettingLL(&settinglist);
		return;
	}

	db_free(&(lParam->dbv));

	if (db_get_s(lParam->hContact, lParam->module, lParam->setting, &(lParam->dbv), 0))
		return;

	wchar_t data[32], tmp[16], name[NAME_SIZE];

	GetContactName(lParam->hContact, nullptr, name, _countof(name));
	lvItem.pszText = name;

	int index = ListView_InsertItem(hwnd, &lvItem);

	ListView_SetItemText(hwnd, index, 0, lvItem.pszText);
	ListView_SetItemTextA(hwnd, index, 1, lParam->module);
	ListView_SetItemTextA(hwnd, index, 2, lParam->setting);
	ListView_SetItemText(hwnd, index, 4, DBVType(dbv->type));

	data[0] = 0;

	switch (dbv->type) {
	case DBVT_BLOB:
		{
			ptrA str(StringFromBlob(dbv->pbVal, dbv->cpbVal));
			ListView_SetItemTextA(hwnd, index, 3, str);
			break;
		}
	case DBVT_BYTE:
		mir_snwprintf(data, L"0x%02X (%s)", dbv->bVal, _ultow(dbv->bVal, tmp, 10));
		ListView_SetItemText(hwnd, index, 3, data);
		break;
	case DBVT_WORD:
		mir_snwprintf(data, L"0x%04X (%s)", dbv->wVal, _ultow(dbv->wVal, tmp, 10));
		ListView_SetItemText(hwnd, index, 3, data);
		break;

	case DBVT_DWORD:
		mir_snwprintf(data, L"0x%08X (%s)", dbv->dVal, _ultow(dbv->dVal, tmp, 10));
		ListView_SetItemText(hwnd, index, 3, data);
		break;

	case DBVT_ASCIIZ:
		ListView_SetItemTextA(hwnd, index, 3, dbv->pszVal);
		break;

	case DBVT_WCHAR:
		{
			ptrW str(mir_wstrdup(dbv->pwszVal));
			ListView_SetItemText(hwnd, index, 3, str);
			break;
		}

	case DBVT_UTF8:
		{
			ptrW str(mir_utf8decodeW(dbv->pszVal));
			ListView_SetItemText(hwnd, index, 3, str);
			break;
		}
	case DBVT_DELETED:
		if (IsResidentSetting(lParam->module, lParam->setting))
			ListView_SetItemText(hwnd, index, 3, TranslateT("*** resident ***"));
		break;
	} // switch

}


void PopulateWatchedWindow()
{
	if (!hwnd2watchedVarsWindow) return;
	HWND hwnd = GetDlgItem(hwnd2watchedVarsWindow, IDC_VARS);
	ListView_DeleteAllItems(hwnd);
	for (int i = 0; i < WatchListArray.count; i++) {
		addwatchtolist(hwnd, &(WatchListArray.item[i]));
	}

	if (lastColumn != -1) {
		ColumnsSortParams params;
		params.hList = hwnd;
		params.column = lastColumn;
		params.last = -1;
		ListView_SortItemsEx(params.hList, ColumnsCompare, (LPARAM)&params);
	}
}


void freeAllWatches()
{
	ListView_DeleteAllItems(GetDlgItem(hwnd2watchedVarsWindow, IDC_VARS));
	for (int i = 0; i < WatchListArray.count; i++) {
		freeWatchListItem(i);
	}
	mir_free(WatchListArray.item);
	WatchListArray.item = nullptr;
	WatchListArray.count = 0;
}


int WatchDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_VARS:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.left = 0;
		urc->rcItem.right = urc->dlgNewSize.cx;
		return RD_ANCHORY_CUSTOM | RD_ANCHORX_CUSTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}


LRESULT CALLBACK WatchSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

	case WM_KEYUP:
		if (wParam == VK_F5) {
			PopulateWatchedWindow();
		}
		break;
	}
	return mir_callNextSubclass(hwnd, WatchSubclassProc, msg, wParam, lParam);
}


INT_PTR CALLBACK WatchDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		hwnd2watchedVarsWindow = hwnd;
		// do the icon
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(ICO_REGEDIT)));
		TranslateMenu(GetMenu(hwnd));
		TranslateMenu(GetSubMenu(GetMenu(hwnd), 0));
		TranslateDialogDefault(hwnd);

		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_APPWINDOW); // taskbar icon

		ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_VARS), 32 | LVS_EX_LABELTIP); // LVS_EX_GRIDLINES

		loadListSettings(GetDlgItem(hwnd, IDC_VARS), csWatchList);
		Utils_RestoreWindowPositionNoMove(hwnd, NULL, MODULENAME, "Watch_");

		mir_subclassWindow(GetDlgItem(hwnd, IDC_VARS), WatchSubclassProc);

		PopulateWatchedWindow();
		return TRUE;

		// for the resize
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = 500;
			mmi->ptMinTrackSize.y = 300;
		}
		return 0;

	case WM_SIZE:
		Utils_ResizeDialog(hwnd, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_WATCH_DIAG), WatchDialogResize);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case MENU_REMALL_WATCHES:
			freeAllWatches();
			break;
		case MENU_EXIT:
		case IDCANCEL:
			hwnd2watchedVarsWindow = nullptr;
			DestroyWindow(hwnd);
			break;
		case MENU_REFRESH:
			PopulateWatchedWindow();
		}
		break;

	case WM_NOTIFY:
		switch (LOWORD(wParam)) {
		case IDC_VARS:
			switch (((NMHDR*)lParam)->code) {
			case NM_DBLCLK:
				LVHITTESTINFO hti;
				LVITEM lvi;
				{
					HWND hwndVars = GetDlgItem(hwnd, IDC_VARS);
					hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
					if (ListView_SubItemHitTest(hwndVars, &hti) > -1) {
						if (hti.flags&LVHT_ONITEM) {
							lvi.mask = LVIF_PARAM;
							lvi.iItem = hti.iItem;
							lvi.iSubItem = 0;
							if (ListView_GetItem(hwndVars, &lvi)) {
								ItemInfo ii;
								ii.hContact = (MCONTACT)lvi.lParam;
								ListView_GetItemTextA(hwndVars, hti.iItem, 1, ii.module, _countof(ii.module));
								ListView_GetItemTextA(hwndVars, hti.iItem, 2, ii.setting, _countof(ii.setting));
								ii.type = FW_SETTINGNAME;
								SendMessage(hwnd2mainWindow, WM_FINDITEM, (WPARAM)&ii, 0);
							}
						}
					}
				}
				break;

			case LVN_COLUMNCLICK:
				LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
				ColumnsSortParams params;
				params.hList = GetDlgItem(hwnd, IDC_VARS);
				params.column = lv->iSubItem;
				params.last = lastColumn;
				ListView_SortItemsEx(params.hList, ColumnsCompare, (LPARAM)&params);
				lastColumn = (params.column == lastColumn) ? -1 : params.column;
				break;
			}
			break;
		}
		break;

	case WM_DESTROY:
		ListView_DeleteAllItems(GetDlgItem(hwnd, IDC_VARS));
		saveListSettings(GetDlgItem(hwnd, IDC_VARS), csWatchList);
		Utils_SaveWindowPosition(hwnd, NULL, MODULENAME, "Watch_");
		hwnd2watchedVarsWindow = nullptr;
		break;
	}
	return 0;
}

void openWatchedVarWindow()
{
	if (!hwnd2watchedVarsWindow)
		CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WATCH_DIAG), nullptr, WatchDlgProc);
	else
		SetForegroundWindow(hwnd2watchedVarsWindow);
}


void popupWatchedVar(MCONTACT hContact, const char *module, const char *setting)
{
	int timeout = g_plugin.iPopupDelay;
	
	wchar_t value[MAX_SECONDLINE];
	int type = GetValue(hContact, module, setting, value, _countof(value));

	POPUPDATAW ppd;
	GetContactName(hContact, nullptr, ppd.lpwzContactName, _countof(ppd.lpwzContactName));
	mir_snwprintf(ppd.lpwzText, TranslateT("Database Setting Changed: \nModule: \"%s\", Setting: \"%s\"\nNew Value (%s): \"%s\""), 
		_A2T(module).get(), _A2T(setting).get(), DBVType(type), value);

	ppd.lchContact = hContact;
	ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(ICO_REGEDIT));
	ppd.colorBack = g_plugin.iPopupBkColor;
	ppd.colorText = g_plugin.iPopupTxtColor;
	ppd.iSeconds = timeout ? timeout : -1;
	PUAddPopupW(&ppd);
}
