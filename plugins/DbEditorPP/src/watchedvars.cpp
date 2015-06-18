#include "headers.h"

HWND hwnd2watchedVarsWindow = NULL;

static WatchListArrayStruct WatchListArray = {0};

static int lastColumn = -1;

ColumnsSettings csWatchList[] = {
	{ LPGENT("Contact"), 0, "Watch0width", 100 },
	{ LPGENT("Module"),  1, "Watch1width", 100 },
	{ LPGENT("Setting"), 2, "Watch2width", 100 },
	{ LPGENT("Value"),   3, "Watch3width", 200 },
	{ LPGENT("Type"),    4, "Watch4width", 75 },
	{0}
};



int WatchedArrayIndex(MCONTACT hContact, const char *module, const char *setting, int strict) {
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
	if (WatchListArray.count == WatchListArray.size)
	{
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
		WatchListArray.item[WatchListArray.count].setting = 0;

	WatchListArray.count++;
	return 1;
}


void freeWatchListItem(int item)
{
    if (item < 0 || item >= WatchListArray.count) return;
    
	if (WatchListArray.item[item].module) 
		mir_free(WatchListArray.item[item].module);

	WatchListArray.item[item].module = 0;

	if (WatchListArray.item[item].setting) 
		mir_free(WatchListArray.item[item].setting);

	WatchListArray.item[item].setting = 0;
	db_free(&(WatchListArray.item[item].dbv));
	WatchListArray.item[item].hContact = 0;
}


void addwatchtolist(HWND hwnd, struct DBsetting *lParam)
{
	LVITEM lvItem;
	int index;

	DBVARIANT *dbv = &(lParam->dbv);
	MCONTACT hContact = lParam->hContact;
	
	char *module = lParam->module;
	char *setting = lParam->setting;

	if (!module) return;
	
	lvItem.lParam = (LPARAM)lParam->hContact;
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;

	if (!setting) // add every item in the module and return
	{
		ModuleSettingLL settinglist;
		struct DBsetting dummy = {0};
		ModSetLinkLinkItem *setting;
		if (IsModuleEmpty(hContact, module) || !EnumSettings(hContact, module, &settinglist)) return;

		dummy.hContact = hContact;
		dummy.module = module;
		setting = settinglist.first;
		while (setting)
		{
			dummy.setting = setting->name;
			addwatchtolist(hwnd, &dummy);
			setting = (ModSetLinkLinkItem *)setting->next;
		}
		FreeModuleSettingLL(&settinglist);
		return;
	}
	
	db_free(&(lParam->dbv));

	db_get_s(hContact, module, setting, &(lParam->dbv), 0);

	TCHAR data[32], tmp[16], name[NAME_SIZE];

	GetContactName(hContact, NULL, name, SIZEOF(name));
	lvItem.pszText = name;

	index = ListView_InsertItem(hwnd, &lvItem);

	ListView_SetItemText(hwnd, index, 0, lvItem.pszText);
	ListView_SetItemTextA(hwnd, index, 1, module);
	ListView_SetItemTextA(hwnd, index, 2, setting);
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
		mir_sntprintf(data, _T("0x%02X (%s)"), dbv->bVal, _ultot(dbv->bVal, tmp, 10));
		ListView_SetItemText(hwnd, index, 3, data);
		break;
	case DBVT_WORD:
		mir_sntprintf(data, _T("0x%04X (%s)"), dbv->wVal, _ultot(dbv->wVal, tmp, 10));
		ListView_SetItemText(hwnd, index, 3, data);
		break;

	case DBVT_DWORD:
		mir_sntprintf(data, _T("0x%08X (%s)"), dbv->dVal, _ultot(dbv->dVal, tmp, 10));
		ListView_SetItemText(hwnd, index, 3, data);
		break;

	case DBVT_ASCIIZ:
		ListView_SetItemTextA(hwnd, index, 3, dbv->pszVal);
		break;

	case DBVT_WCHAR:
	{
		ptrT str(mir_u2t(dbv->pwszVal));
		ListView_SetItemText(hwnd, index, 3, str);
		break;
	}

	case DBVT_UTF8:
	{
		ptrT str(mir_utf8decodeT(dbv->pszVal));
		ListView_SetItemText(hwnd, index, 3, str);
		break;
	}
	case DBVT_DELETED:
		if (IsResidentSetting(module, setting))
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
	for (int i = 0; i < WatchListArray.count; i++)
	{
		freeWatchListItem(i);
	}
	mir_free(WatchListArray.item);
	WatchListArray.item = 0;
	WatchListArray.count = 0;
}


int WatchDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId)
	{
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
	switch (msg)
	{
	case WM_INITDIALOG:
	{
	    hwnd2watchedVarsWindow = hwnd;
		// do the icon
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(ICO_REGEDIT)));
		TranslateMenu(GetMenu(hwnd));
		TranslateMenu(GetSubMenu(GetMenu(hwnd), 0));
		TranslateDialogDefault(hwnd);
	
	    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_APPWINDOW); // taskbar icon

		ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_VARS), 32 | LVS_EX_LABELTIP); // LVS_EX_GRIDLINES

		loadListSettings(GetDlgItem(hwnd, IDC_VARS), csWatchList);
		Utils_RestoreWindowPositionNoMove(hwnd, NULL, modname, "Watch_");

		mir_subclassWindow(GetDlgItem(hwnd, IDC_VARS), WatchSubclassProc);

		PopulateWatchedWindow();
	}
	return TRUE;
	// for the resize
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *mmi = (MINMAXINFO*)lParam;
		mmi->ptMinTrackSize.x = 500;
		mmi->ptMinTrackSize.y = 300;
		return 0;
	}
	case WM_SIZE:
	{
		UTILRESIZEDIALOG urd;
		memset(&urd, 0, sizeof(urd));
		urd.cbSize = sizeof(urd);
		urd.hInstance = hInst;
		urd.hwndDlg = hwnd;
		urd.lParam = 0;
		urd.lpTemplate = MAKEINTRESOURCEA(IDD_WATCH_DIAG);
		urd.pfnResizer = WatchDialogResize;
		CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case MENU_REMALL_WATCHES:
			freeAllWatches();
			break;
		case MENU_EXIT:
		case IDCANCEL:
			hwnd2watchedVarsWindow = NULL;
			DestroyWindow(hwnd);
			break;
		case MENU_REFRESH:
			PopulateWatchedWindow();
		}
		break;

	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_VARS:
			switch (((NMHDR*)lParam)->code) {
			case NM_DBLCLK:
				{
					LVHITTESTINFO hti;
					LVITEM lvi;
					HWND hwndVars = GetDlgItem(hwnd, IDC_VARS);
					hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
					if (ListView_SubItemHitTest(hwndVars, &hti) > -1)
					{
						if (hti.flags&LVHT_ONITEM)
						{
							lvi.mask = LVIF_PARAM;
							lvi.iItem = hti.iItem;
							lvi.iSubItem = 0;
							if (ListView_GetItem(hwndVars, &lvi))
							{
								ItemInfo ii;
								ii.hContact = (MCONTACT)lvi.lParam;
								ListView_GetItemTextA(hwndVars, hti.iItem, 1, ii.module, SIZEOF(ii.module));
								ListView_GetItemTextA(hwndVars, hti.iItem, 2, ii.setting, SIZEOF(ii.setting));
								ii.type = FW_SETTINGNAME;
								SendMessage(hwnd2mainWindow, WM_FINDITEM, (WPARAM)&ii, 0);
							}
						}
					}
				}
				break;
			case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
					ColumnsSortParams params;
					params.hList = GetDlgItem(hwnd, IDC_VARS);
					params.column = lv->iSubItem;
					params.last = lastColumn;
					ListView_SortItemsEx(params.hList, ColumnsCompare, (LPARAM)&params);
					lastColumn = (params.column == lastColumn) ? -1 : params.column;
					break;
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
		ListView_DeleteAllItems(GetDlgItem(hwnd, IDC_VARS));
		saveListSettings(GetDlgItem(hwnd, IDC_VARS), csWatchList);
		Utils_SaveWindowPosition(hwnd, NULL, modname, "Watch_");
		hwnd2watchedVarsWindow = NULL;
		break;
	}
	return 0;
}


void openWatchedVarWindow() {

	if (!hwnd2watchedVarsWindow) 
		CreateDialog(hInst, MAKEINTRESOURCE(IDD_WATCH_DIAG), NULL, WatchDlgProc);
	else
		SetForegroundWindow(hwnd2watchedVarsWindow);
}


void popupWatchedVar(MCONTACT hContact, const char *module, const char *setting)
{
	COLORREF colorBack = db_get_dw(NULL, modname, "PopupColour", RGB(255, 0, 0));
	COLORREF colorText = db_get_dw(NULL, modname, "PopupTextColour", RGB(0, 0, 0));
	int timeout = db_get_b(NULL, modname, "PopupDelay", 3);

	TCHAR name[NAME_SIZE], text[MAX_SECONDLINE], value[MAX_SECONDLINE];
	int res = 0;

	GetContactName(hContact, NULL, name, SIZEOF(name));

	// 2nd line
	int type = GetValue(hContact, module, setting, value, SIZEOF(value));
//	if (!type) value = _T("NULL");

	mir_sntprintf(text, TranslateT("Database Setting Changed: \nModule: \"%s\", Setting: \"%s\"\nNew Value (%s): \"%s\""), _A2T(module), _A2T(setting), DBVType(type), value);

	POPUPDATAT ppd = { 0 };
	ppd.lchContact = (MCONTACT)hContact;
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(ICO_REGEDIT));
	mir_tstrncpy(ppd.lptzContactName, name, SIZEOF(ppd.lptzContactName));
	mir_tstrncpy(ppd.lptzText, text, SIZEOF(ppd.lptzText));
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.iSeconds = timeout ? timeout : -1;
	PUAddPopupT(&ppd);
}
