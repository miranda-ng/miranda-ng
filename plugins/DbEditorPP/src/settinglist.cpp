#include "headers.h"


SettingListInfo info = {0};
HWND hwnd2List = 0;


static int lastColumn = -1;


struct ColumnsSettings csSettingList[] = {
	{ LPGENT("Name"),  0, "Column0width", 180 },
	{ LPGENT("Value"), 1, "Column1width", 250 },
	{ LPGENT("Type"),  2, "Column2width", 60 },
	{ LPGENT("Size"),  3, "Column3width", 80 },
	{ LPGENT("#"),     4, "Column4width", 30 },
	{0}
};


int ListView_GetItemTextA(HWND hwndLV, int i, int iSubItem, char *pszText, int cchTextMax)
{ 
	LV_ITEMA lvi;
	lvi.iSubItem = iSubItem;
	lvi.cchTextMax = cchTextMax;
	lvi.pszText = pszText;
	return SendMessageA((hwndLV), LVM_GETITEMTEXTA, (WPARAM)(i), (LPARAM)(LV_ITEMA *)&lvi);
}


int ListView_SetItemTextA(HWND hwndLV, int i, int iSubItem, const char *pszText) 
{ 
	LV_ITEMA lvi;
	lvi.iSubItem = iSubItem;
	lvi.pszText = (char*)pszText;
 	return SendMessageA((hwndLV), LVM_SETITEMTEXTA, (WPARAM)(i), (LPARAM)(LV_ITEMA *)&lvi);
}


int convertSetting(MCONTACT hContact, const char *module, const char *setting, int toType)
{
	DBVARIANT dbv = { 0 };

	if (db_get_s(hContact, module, setting, &dbv, 0)) return 0;

	if (dbv.type == toType) {
	    db_free(&dbv);
		return 1;
	}

	int res = 0;
	DWORD val = 0;
	TCHAR tmp[16]; 
	ptrT value;

	switch (dbv.type) {

	case DBVT_BYTE:
	case DBVT_WORD:
	case DBVT_DWORD:
		val = getNumericValue(&dbv);
		value = mir_tstrdup(_ultot(val, tmp, 10));
		break;

	case DBVT_WCHAR:
		if (!value) value = mir_u2t(dbv.pwszVal);
	case DBVT_UTF8:
		if (!value) value = mir_utf8decodeT(dbv.pszVal);
	case DBVT_ASCIIZ:
		if (!value) value = mir_a2t(dbv.pszVal);

		if (mir_tstrlen(value) < 11)
			val = _tcstoul(value, NULL, NULL);
		break;
	}

	switch (toType) {

	case DBVT_BYTE:
	case DBVT_WORD:
	case DBVT_DWORD:
		if (val != 0 || !mir_tstrcmp(value, _T("0")))
  			res = setNumericValue(hContact, module, setting, val, toType);
		break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
	case DBVT_WCHAR:
		if (value)
			res = setTextValue(hContact, module, setting, value, toType);
		break;

	}

	if (!res)
		msg(TranslateT("Unable to store value in this data type!"));

	db_free(&dbv);

	return res;
}


void EditFinish(int selected)
{
    if (info.hwnd2Edit) {
		SendMessage(info.hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
		info.hwnd2Edit = NULL;
	}
	info.selectedItem = selected;
}


void ClearListView()
{
	EditFinish(0);
	info.module[0] = 0;
	info.setting[0] = 0;
	info.hContact = 0;

	ListView_DeleteAllItems(hwnd2List);
}


void DeleteSettingsFromList(MCONTACT hContact, const char *module, const char *setting)
{
	int count = ListView_GetSelectedCount(hwnd2List);

	if (!count) return;

	if (db_get_b(NULL, modname, "WarnOnDelete", 1)) {
		TCHAR text[MSG_SIZE];
		mir_sntprintf(text, TranslateT("Are you sure you want to delete setting(s): %d?"), count);
		if (dlg(text, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			return;
	}

	if (count == 1)
		db_unset(hContact, module, setting);
	else {
		int items = ListView_GetItemCount(hwnd2List);
		int i = 0;
		char text[FLD_SIZE];

		while (i < items) {
			if (ListView_GetItemState(hwnd2List, i, LVIS_SELECTED)) {
				if (ListView_GetItemTextA(hwnd2List, i, 0, text, SIZEOF(text)))
					db_unset(hContact, module, text);
				items--;
			}
			else
				i++;
		}
	}

	if (ListView_GetItemCount(hwnd2List) == 0)
		replaceTreeItem(hContact, module, 0);
}


int findListItem(const char *setting) 
{
	if (!setting || !setting[0]) return -1;

	LVFINDINFOA lvfi;
	lvfi.flags = LVFI_STRING;
	lvfi.psz = setting;
	lvfi.vkDirection = VK_DOWN;
	return SendMessageA(hwnd2List, LVM_FINDITEMA, -1, (LPARAM)&lvfi);
}


void deleteListItem(const char *setting)
{
    int item = findListItem(setting);
	if (item > -1)
		ListView_DeleteItem(hwnd2List, item);
}


void updateListItem(int index, const char *setting, DBVARIANT *dbv, int resident)
{
	if (!dbv || !dbv->type) {
		ListView_DeleteItem(hwnd2List, index);
		return;
	}

	if (index < 0) return;

	LVITEM lvi = {0};
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = index;

	ListView_SetItemText(hwnd2List, index, 4, resident?_T("[R]"):_T(""));

	if (g_db && g_db->IsSettingEncrypted(info.module, setting)) {
		lvi.iImage = IMAGE_UNICODE;
		ListView_SetItem(hwnd2List, &lvi);
		ListView_SetItemTextA(hwnd2List, index, 0, setting);
		ListView_SetItemText(hwnd2List, index, 1, TranslateT("*** encrypted ***"));
		ListView_SetItemText(hwnd2List, index, 2, _T("UNICODE"));
		ListView_SetItemText(hwnd2List, index, 3, _T(""));
		return;
	}

	TCHAR data[32];
	int length;

	switch (dbv->type) {
	case DBVT_BLOB:
	{
		lvi.iImage = IMAGE_BINARY;
		ListView_SetItem(hwnd2List, &lvi);
		ptrA str(StringFromBlob(dbv->pbVal, dbv->cpbVal));		
		ListView_SetItemTextA(hwnd2List, index, 1, str);
		mir_sntprintf(data, _T("0x%04X (%u)"), dbv->cpbVal, dbv->cpbVal);
		ListView_SetItemText(hwnd2List, index, 3, data);
		break;
	}
	case DBVT_BYTE:
		lvi.iImage = IMAGE_BYTE;
		ListView_SetItem(hwnd2List, &lvi);
		mir_sntprintf(data, _T("0x%02X (%u)"), dbv->bVal, dbv->bVal);
		ListView_SetItemText(hwnd2List, index, 1, data);
		ListView_SetItemText(hwnd2List, index, 3, _T("0x0001 (1)"));
		break;

	case DBVT_WORD:
		lvi.iImage = IMAGE_WORD;
		ListView_SetItem(hwnd2List, &lvi);
		mir_sntprintf(data, _T("0x%04X (%u)"), dbv->wVal, dbv->wVal);
		ListView_SetItemText(hwnd2List, index, 1, data);
		ListView_SetItemText(hwnd2List, index, 3, _T("0x0002 (2)"));
		break;

	case DBVT_DWORD:
		lvi.iImage = IMAGE_DWORD;
		ListView_SetItem(hwnd2List, &lvi);
		mir_sntprintf(data, _T("0x%08X (%u)"), dbv->dVal, dbv->dVal);
		ListView_SetItemText(hwnd2List, index, 1, data);
		ListView_SetItemText(hwnd2List, index, 3, _T("0x0004 (4)"));
		break;

	case DBVT_ASCIIZ:
	{
		lvi.iImage = IMAGE_STRING;
		ListView_SetItem(hwnd2List, &lvi);
		ListView_SetItemTextA(hwnd2List, index, 1, dbv->pszVal);
		length = mir_strlen(dbv->pszVal) + 1;
		mir_sntprintf(data, _T("0x%04X (%u)"), length, length);
		ListView_SetItemText(hwnd2List, index, 3, data);
		break;
	}
	case DBVT_WCHAR:
	{
		lvi.iImage = IMAGE_UNICODE;
		ListView_SetItem(hwnd2List, &lvi);
		length = mir_wstrlen(dbv->pwszVal) + 1;
		ptrT str(mir_u2t(dbv->pwszVal));
		ListView_SetItemText(hwnd2List, index, 1, str);
		mir_sntprintf(data, _T("0x%04X (%u)"), length, length);
		ListView_SetItemText(hwnd2List, index, 3, data);
		break;
	}
	case DBVT_UTF8:
	{
		lvi.iImage = IMAGE_UNICODE;
		ListView_SetItem(hwnd2List, &lvi);
		length = mir_strlen(dbv->pszVal) + 1;
		ptrT str(mir_utf8decodeT(dbv->pszVal));
		ListView_SetItemText(hwnd2List, index, 1, str);
		mir_sntprintf(data, _T("0x%04X (%u)"), length, length);
		ListView_SetItemText(hwnd2List, index, 3, data);
		break;
	}
	case DBVT_DELETED:
//		ListView_DeleteItem(hwnd2List, index);
		return;
	}

	//ListView_SetItemTextA(hwnd2List, index, 0, setting);
	ListView_SetItemText(hwnd2List, index, 2, DBVType(dbv->type));
}


void addListHandle(MCONTACT hContact)
{
	TCHAR name[NAME_SIZE], data[32];
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
	lvi.lParam = hContact;
	lvi.iImage = IMAGE_HANDLE;

	GetContactName(hContact, NULL, name, SIZEOF(name));
	lvi.pszText = name;

	int index = ListView_InsertItem(hwnd2List, &lvi);

	mir_sntprintf(data, _T("0x%08X (%ld)"), hContact, hContact);

	ListView_SetItemText(hwnd2List, index, 1, data);
	ListView_SetItemText(hwnd2List, index, 2, _T("HANDLE"));
	ListView_SetItemText(hwnd2List, index, 3, _T("0x0004 (4)"));
	if (db_mc_isEnabled()) {
		if (db_mc_isSub(hContact)) {
			ListView_SetItemText(hwnd2List, index, 4, _T("[S]"));
		} else
		if (db_mc_isMeta(hContact)) {
			ListView_SetItemText(hwnd2List, index, 4, _T("[M]"));
		}
	}
}


void addListItem(const char *setting, int resident)
{
	DBVARIANT dbv;
	if (!db_get_s(info.hContact, info.module, setting, &dbv, 0)) {
		LVITEMA lvi = { 0 };
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (char*)setting;
		int index = SendMessageA(hwnd2List, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
		updateListItem(index, setting, &dbv, resident);
		db_free(&dbv);
	} 
	else
	if (!resident) { 
		LVITEMA lvi = { 0 };
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (char*)setting;
		int index = SendMessageA(hwnd2List, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
		ListView_SetItemText(hwnd2List, index, 1, TranslateT("*** buggy resident ***"));
		return;
	}
}


void PopulateSettings(MCONTACT hContact, const char *module)
{
    // save module as it can be erased by ClearListView()
    char tmp[FLD_SIZE];
    mir_strncpy(tmp, module, SIZEOF(tmp));

	// clear any settings that may be there...
	ClearListView();

	info.hContact = hContact;
	mir_strncpy(info.module, tmp, SIZEOF(info.module));

	ModuleSettingLL setlist;

	if (IsModuleEmpty(info.hContact, info.module) || !EnumSettings(info.hContact, info.module, &setlist))
		return;

	for (ModSetLinkLinkItem *setting = setlist.first; setting; setting = setting->next)
		addListItem(setting->name, 0);

	FreeModuleSettingLL(&setlist);

	// Resident settings 
	if (!EnumResidentSettings(module, &setlist))
		return;

	for (ModSetLinkLinkItem *setting = setlist.first; setting; setting = setting->next)
		addListItem(setting->name, 1);
	
	FreeModuleSettingLL(&setlist);
}


void SelectSetting(const char *setting)
{
	LVITEM lvItem = {0};

	lvItem.mask = LVIF_STATE;
	lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

	int items = ListView_GetItemCount(hwnd2List);
	for (int i = 0; i < items; i++) {
		if (ListView_GetItemState(hwnd2List, i, lvItem.stateMask))
			ListView_SetItemState(hwnd2List, i, 0, lvItem.stateMask);
	}

	lvItem.iItem = findListItem(setting);

	if (lvItem.iItem != -1) {
		EditFinish(lvItem.iItem);
		lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
		ListView_SetItem(hwnd2List, &lvItem);
	}
}


void settingChanged(MCONTACT hContact, const char *module, const char *setting, DBVARIANT *dbv)
{
	// modules tree
	if (dbv->type != DBVT_DELETED) {
		HTREEITEM hItem = findItemInTree(hContact, module);
		if (!hItem) {
			HTREEITEM hParent = findItemInTree(hContact, NULL);
			if (hParent)
				insertItem(hContact, module, hParent);
		}
	}

	// settings list
	if (hContact != info.hContact || mir_strcmp(info.module, module)) return;

	if (dbv->type == DBVT_DELETED) {
		deleteListItem(setting);
	} else {
		LVITEMA lvi = {0};
		lvi.iItem = findListItem(setting);
		if (lvi.iItem == -1) {
			lvi.iItem = 0;
			lvi.mask = LVIF_TEXT;
			lvi.pszText = (char*)setting;
			lvi.iItem = SendMessageA(hwnd2List, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
		}
		updateListItem(lvi.iItem, setting, dbv, IsResidentSetting(module, setting));
	}
}


static LRESULT CALLBACK SettingLabelEditSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			if (GetKeyState(VK_CONTROL) & 0x8000) // ctrl is pressed
				break;
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
			return 0;
		case VK_ESCAPE:
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
			return 0;
		}
		break;
	case WM_USER:
	{
		SetFocus(hwnd);
		SendMessage(hwnd, WM_SETFONT, SendMessage(GetParent(hwnd), WM_GETFONT, 0, 0), 1);
		if (info.subitem)
			SendMessage(hwnd, EM_LIMITTEXT, (WPARAM)65535, 0);
		else
			SendMessage(hwnd, EM_LIMITTEXT, (WPARAM)255, 0);
		SendMessage(hwnd, EM_SETSEL, 0, -1);
		break;
	}
	case WM_PAINT:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			DBVARIANT dbv = { 0 };
			int len = GetWindowTextLength(hwnd) + 1;

			if ((!info.subitem && len <= 1) || db_get_s(info.hContact, info.module, info.setting, &dbv, 0)) {
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
				return 0;
			}
			
			TCHAR *value = (TCHAR*)mir_alloc(len*sizeof(TCHAR));

			GetWindowText(hwnd, value, len);

			_T2A szValue(value);

			int res = 0;

			switch (info.subitem) {
			case 0: // setting name
				if (!mir_strcmp(info.setting, szValue) || mir_strlen(szValue) > 0xFE) {
					db_free(&dbv);
					mir_free(value);
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
					return 0;
				}
				if (db_set(info.hContact, info.module, szValue, &dbv))
					break;
				res = 1;
				db_unset(info.hContact, info.module, info.setting);
				deleteListItem(info.setting);
				break;

			case 1: // value
				DWORD val;
				int i = 0;

				if (dbv.type == DBVT_BLOB) {
					res = WriteBlobFromString(info.hContact, info.module, info.setting, szValue, mir_strlen(szValue));
					break;
				}

				switch (value[0]) {
				case _T('b'):
				case _T('B'):
					val = _tcstoul(&value[1], NULL, 0);
					if (!val || value[1] == _T('0')) {
						res = !db_set_b(info.hContact, info.module, info.setting, (BYTE)val);
					}
					else 
						res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
					break;
				case _T('w'):
				case _T('W'):
					val = _tcstoul(&value[1], NULL, 0);
					if (!val || value[1] == _T('0'))
						res = !db_set_w(info.hContact, info.module, info.setting, (WORD)val);
					else 
						res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
					break;
				case _T('d'):
				case _T('D'):
					val = _tcstoul(&value[1], NULL, 0);
					if (!val || value[1] == _T('0'))
						res = !db_set_dw(info.hContact, info.module, info.setting, val);
					else 
						res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
					break;

				case _T('0'):
					i = 1;
				case _T('1'):
				case _T('2'):
				case _T('3'):
				case _T('4'):
				case _T('5'):
				case _T('6'):
				case _T('7'):
				case _T('8'):
				case _T('9'):
				case _T('-'):
				case _T('x'):
				case _T('X'):
					if (value[i] == _T('x') || value[i] == _T('X'))
						val = _tcstoul(&value[i+1], NULL, 16);
					else
						val = _tcstoul(value, NULL, 10);

					switch (dbv.type) {
					case DBVT_BYTE:
					case DBVT_WORD:
					case DBVT_DWORD:
						res = setNumericValue(info.hContact, info.module, info.setting, val, dbv.type);
						break;
					case DBVT_ASCIIZ:
					case DBVT_WCHAR:
					case DBVT_UTF8:
						res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
						break;
					}
					break;
				case _T('\"'):
				case _T('\''):
				{
					int nlen = mir_tstrlen(value);
					int sh = 0;
					if (nlen > 3) {
						if (value[nlen - 1] == value[0]) {
							value[nlen - 1] = 0;
							sh = 1;
						}
					}
					res = setTextValue(info.hContact, info.module, info.setting, &value[sh], dbv.type);
				}
				break;

				default:
					res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
					break;
				}
				break;
			}

			mir_free(value);
			db_free(&dbv);

			if (!res) {
				msg(TranslateT("Unable to store value in this data type!"));
				break;
			}

		} // fall through
		case IDCANCEL:
		{
			DestroyWindow(hwnd);
			return 0;
		}
		break; // wm_command
		}
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_DESTROY:
		info.hwnd2Edit = NULL;
		break;		
	}
	return mir_callNextSubclass(hwnd, SettingLabelEditSubClassProc, msg, wParam, lParam);
}

void EditLabel(int item, int subitem)
{
	RECT rc;
	char setting[FLD_SIZE];
	DBVARIANT dbv;

	if (info.hwnd2Edit) {
		SendMessage(info.hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0); // ignore the new value of the last edit
		info.hwnd2Edit = NULL;
	}
	
	if (!ListView_GetItemTextA(hwnd2List, item, 0, setting, SIZEOF(setting))) return;
	if (!setting[0] || !ListView_GetSubItemRect(hwnd2List, item, subitem, LVIR_LABEL, &rc))
		return;
	
	if (db_get_s(info.hContact, info.module, setting, &dbv, 0)) 
		return;

	info.hContact = info.hContact;
	mir_strcpy(info.setting, setting);
	info.subitem = subitem;

	if (!subitem)
		info.hwnd2Edit = CreateWindow(_T("EDIT"), _A2T(setting), WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);
	else {
	    TCHAR *str = NULL, value[16] = { 0 };

		switch (dbv.type) {
		case DBVT_ASCIIZ:
			str = mir_a2t(dbv.pszVal);
			break;
		case DBVT_BYTE:
			mir_sntprintf(value, (g_Hex & HEX_BYTE) ? _T("0x%02X") : _T("%u"), dbv.bVal);
			break;
		case DBVT_WORD:
			mir_sntprintf(value, (g_Hex & HEX_WORD) ? _T("0x%04X") : _T("%u"), dbv.wVal);
			break;
		case DBVT_DWORD:
			mir_sntprintf(value, (g_Hex & HEX_DWORD) ? _T("0x%08X") : _T("%u"), dbv.dVal);
			break;
		case DBVT_WCHAR:
			str = mir_u2t(dbv.pwszVal);
			break;
		case DBVT_UTF8:
			str = mir_utf8decodeT(dbv.pszVal);
			break;
		case DBVT_BLOB:
		{
			ptrA tmp(StringFromBlob(dbv.pbVal, dbv.cpbVal));
			str = mir_a2t(tmp);
			break;
		}
		}

		if (str)
		{
		    int height = (rc.bottom - rc.top) * 4;
			RECT rclist;
			GetClientRect(hwnd2List, &rclist);
		    if (rc.top + height > rclist.bottom && rclist.bottom - rclist.top > height)
		    	rc.top = rc.bottom - height;
			info.hwnd2Edit = CreateWindow(_T("EDIT"), str, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL, rc.left, rc.top, rc.right - rc.left, height, hwnd2List, 0, hInst, 0);
			mir_free(str);
		}
		else
		if (dbv.type == DBVT_BYTE || dbv.type == DBVT_WORD || dbv.type == DBVT_DWORD)
			info.hwnd2Edit = CreateWindow(_T("EDIT"), value, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);

	}

	db_free(&dbv);

	mir_subclassWindow(info.hwnd2Edit, SettingLabelEditSubClassProc);
	SendMessage(info.hwnd2Edit, WM_USER, 0, 0);
}


void SettingsListRightClick(HWND hwnd, WPARAM wParam, LPARAM lParam);


void SettingsListWM_NOTIFY(HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)
{
	LVHITTESTINFO hti;

	switch (((NMHDR*)lParam)->code) {
	case NM_CLICK:
		hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
		if (ListView_SubItemHitTest(hwnd2List, &hti) > -1) {
			if (g_Inline && hti.iSubItem <= 1 && hti.flags != LVHT_ONITEMICON && info.selectedItem == hti.iItem)
				EditLabel(hti.iItem, hti.iSubItem);
			else 
				EditFinish(hti.iItem);
		}
		else 
			EditFinish(0);
		break;

	case NM_DBLCLK:
		hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
		if (ListView_SubItemHitTest(hwnd2List, &hti) > -1) {
			if (!info.module[0]) { // contact
				LVITEM lvi = { 0 };
				lvi.mask = LVIF_PARAM;
				lvi.iItem = hti.iItem;
				if (!ListView_GetItem(hwnd2List, &lvi)) break;

				ItemInfo ii = {0};
				ii.hContact = (MCONTACT)lvi.lParam;
				SendMessage(hwnd2mainWindow, WM_FINDITEM, (WPARAM)&ii, 0);
				break;
			}
			if (!g_Inline || hti.iSubItem > 1 || hti.flags == LVHT_ONITEMICON) {
				char setting[FLD_SIZE];
				EditFinish(hti.iItem);
				if (ListView_GetItemTextA(hwnd2List, hti.iItem, 0, setting, SIZEOF(setting)))
					editSetting(info.hContact, info.module, setting);
			}
			else 
				EditLabel(hti.iItem, hti.iSubItem);
		}
		break;

	case NM_RCLICK:
		SettingsListRightClick(hwnd, wParam, lParam);
		break;

	case LVN_COLUMNCLICK:
		LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
		ColumnsSortParams params = { 0 };
		params.hList = hwnd2List;
		params.column = lv->iSubItem;
		params.last = lastColumn;
		ListView_SortItemsEx(params.hList, ColumnsCompare, (LPARAM)&params);
		lastColumn = (params.column == lastColumn) ? -1 : params.column;
		break;
	}
}


void SettingsListRightClick(HWND hwnd, WPARAM, LPARAM lParam) // hwnd here is to the main window, NOT the listview
{
	LVHITTESTINFO hti;
	POINT pt;
	HMENU hMenu, hSubMenu;

	hti.pt = ((NMLISTVIEW*)lParam)->ptAction;

	if (ListView_SubItemHitTest(hwnd2List, &hti) == -1) {
		// nowhere.. new item menu
		GetCursorPos(&pt);
		hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXTMENU));
		hSubMenu = GetSubMenu(hMenu, 6);
		TranslateMenu(hSubMenu);

		if (!info.module[0]) {
			RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // new
			RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // separator
		}

		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL)) {
		case MENU_ADD_BYTE:
			newSetting(info.hContact, info.module, DBVT_BYTE);
			return;

		case MENU_ADD_WORD:
			newSetting(info.hContact, info.module, DBVT_WORD);
			return;

		case MENU_ADD_DWORD:
			newSetting(info.hContact, info.module, DBVT_DWORD);
			return;

		case MENU_ADD_STRING:
			newSetting(info.hContact, info.module, DBVT_ASCIIZ);
			return;

		case MENU_ADD_UNICODE:
			newSetting(info.hContact, info.module, DBVT_WCHAR);
			return;

		case MENU_ADD_BLOB:
			newSetting(info.hContact, info.module, DBVT_BLOB);
			return;

		case MENU_REFRESH:
    		PopulateSettings(info.hContact, info.module);
			return;
		}
		return;
	}

	// on item
	GetCursorPos(&pt);
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXTMENU));
	hSubMenu = GetSubMenu(hMenu, 0);
	TranslateMenu(hSubMenu);

	LVITEM lvi = { 0 };

	lvi.mask = LVIF_IMAGE;
	lvi.iItem = hti.iItem;
	lvi.iSubItem = 0;
	ListView_GetItem(hwnd2List, &lvi);

	switch (lvi.iImage) {
	case IMAGE_STRING:
		RemoveMenu(hSubMenu, MENU_CHANGE2STRING, MF_BYCOMMAND);
		break;
	case IMAGE_BYTE:
		RemoveMenu(hSubMenu, MENU_CHANGE2BYTE, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	case IMAGE_WORD:
		RemoveMenu(hSubMenu, MENU_CHANGE2WORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	case IMAGE_DWORD:
		RemoveMenu(hSubMenu, MENU_CHANGE2DWORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	case IMAGE_BINARY:
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION); // convert
		break;
	case IMAGE_UNICODE:
		RemoveMenu(hSubMenu, MENU_CHANGE2DWORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2WORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2BYTE, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2BYTE, MF_BYCOMMAND);
		break;
	case IMAGE_EMPTY: // resident
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION); // convert
		RemoveMenu(hSubMenu, MENU_COPY_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_DELETE_SET, MF_BYCOMMAND);
		break;

	case IMAGE_HANDLE:
		RemoveMenu(hSubMenu, MENU_EDIT_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_COPY_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_DELETE_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_WATCH_ITEM, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // convert
		RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // separator
		RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // new
		break;
	}

	if (ListView_GetSelectedCount(hwnd2List) > 1) {
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION); // convert
		RemoveMenu(hSubMenu, MENU_EDIT_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_COPY_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_WATCH_ITEM, MF_BYCOMMAND);
	}
	
	char setting[FLD_SIZE];
	if (!ListView_GetItemTextA(hwnd2List, hti.iItem, 0, setting, SIZEOF(setting))) return;

	// check if the setting is being watched and if it is then check the menu item
	int watchIdx = WatchedArrayIndex(info.hContact, info.module, setting, 1);
	if (watchIdx >= 0)
		CheckMenuItem(hSubMenu, MENU_WATCH_ITEM, MF_CHECKED | MF_BYCOMMAND);
	
	switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL)) {
	case MENU_EDIT_SET:
		editSetting(info.hContact, info.module, setting);
		break;

	case MENU_COPY_SET:
		copySetting(info.hContact, info.module, setting);
		break;

	case MENU_ADD_BYTE:
		newSetting(info.hContact, info.module, DBVT_BYTE);
		return;

	case MENU_ADD_WORD:
		newSetting(info.hContact, info.module, DBVT_WORD);
		return;

	case MENU_ADD_DWORD:
		newSetting(info.hContact, info.module, DBVT_DWORD);
		return;

	case MENU_ADD_STRING:
		newSetting(info.hContact, info.module, DBVT_ASCIIZ);
		return;

	case MENU_ADD_BLOB:
		newSetting(info.hContact, info.module, DBVT_BLOB);
		return;

	case MENU_CHANGE2BYTE:
		convertSetting(info.hContact, info.module, setting, DBVT_BYTE);
		break;

	case MENU_CHANGE2WORD:
		convertSetting(info.hContact, info.module, setting, DBVT_WORD);
		break;

	case MENU_CHANGE2DWORD:
		convertSetting(info.hContact, info.module, setting, DBVT_DWORD);
		break;

	case MENU_CHANGE2STRING:
		convertSetting(info.hContact, info.module, setting, DBVT_ASCIIZ);
		break;

	case MENU_CHANGE2UNICODE:
		convertSetting(info.hContact, info.module, setting, DBVT_UTF8);
		break;

	case MENU_WATCH_ITEM:
		if (watchIdx < 0)
			addSettingToWatchList(info.hContact, info.module, setting);
		else
			freeWatchListItem(watchIdx);
		PopulateWatchedWindow();
		break;

	case MENU_DELETE_SET:
		DeleteSettingsFromList(info.hContact, info.module, setting);
		break;

	case MENU_REFRESH:
		PopulateSettings(info.hContact, info.module);
		break;
	}
}
