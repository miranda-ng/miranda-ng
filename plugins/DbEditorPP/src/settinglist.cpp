#include "headers.h"

void setupSettingsList(HWND hwnd2List)
{
	ListView_SetUnicodeFormat(hwnd2List, TRUE);

	LVCOLUMN sLC;
	sLC.fmt = LVCFMT_LEFT;
	ListView_SetExtendedListViewStyle(hwnd2List, 32 | LVS_EX_SUBITEMIMAGES); //LVS_EX_FULLROWSELECT
	sLC.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

	sLC.pszText = Translate("Name");
	sLC.cx = db_get_w(NULL, modname, "Column0width", 145);
	ListView_InsertColumn(hwnd2List, 0, &sLC);
	sLC.pszText = Translate("Data");
	sLC.cx = db_get_w(NULL, modname, "Column1width", 145);
	ListView_InsertColumn(hwnd2List, 1, &sLC);
	sLC.pszText = Translate("Type");
	sLC.cx = db_get_w(NULL, modname, "Column2width", 60);
	ListView_InsertColumn(hwnd2List, 2, &sLC);
	sLC.pszText = Translate("Size");
	sLC.cx = db_get_w(NULL, modname, "Column3width", 80);
	ListView_InsertColumn(hwnd2List, 3, &sLC);
}

void saveListSettings(HWND hwnd2List)
{
	LVCOLUMN sLC = {0};
	char tmp[33]; tmp[32] = 0;

	sLC.mask = LVCF_WIDTH;

	for (int i=0; i <= 3; i++)
		if (ListView_GetColumn(hwnd2List, i, &sLC)) {
			mir_snprintf(tmp, SIZEOF(tmp), "Column%dwidth", i);
			db_set_w(NULL, modname, tmp, (WORD)sLC.cx);
		}
}


void ClearListview(HWND hwnd2Settings)
{
	SettingListInfo *info = (SettingListInfo*)GetWindowLongPtr(hwnd2Settings,GWLP_USERDATA);
	if (info && ListView_GetItemCount(hwnd2Settings)) {
		mir_free(info->module);
		if (info->hwnd2Edit) {
			SendMessage(info->hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
			info->hwnd2Edit = NULL;
		}
		mir_free(info);
		SetWindowLongPtr(hwnd2Settings, GWLP_USERDATA, 0);
	}
	ListView_DeleteAllItems(hwnd2Settings);
}

void DeleteSettingsFromList(HWND hSettings, MCONTACT hContact, char *module, char *setting)
{
	int count = ListView_GetSelectedCount(hSettings);

	if (!count) return;

	if (count == 1)
		db_unset(hContact,module,setting);
	else {
		int items = ListView_GetItemCount(hSettings);
		int i = 0;
		char text[256];

		while (i < items) {
			if (ListView_GetItemState(hSettings, i, LVIS_SELECTED)) {
				ListView_GetItemText(hSettings, i, 0, text, SIZEOF(text));
				db_unset(hContact, module, text);
				items--;
			}
			else
				i++;
		}
	}

	if (ListView_GetItemCount(hSettings) == 0) {
		HWND hModules = GetDlgItem(hwnd2mainWindow, IDC_MODULES);
		TVITEM item;
		char text[265];
		item.mask = TVIF_PARAM | TVIF_TEXT;
		item.pszText = text;
		item.cchTextMax = SIZEOF(text);
		item.hItem = (HTREEITEM)findItemInTree(hModules, hContact, module);
		if ((int)item.hItem != -1)
			if (TreeView_GetItem(hModules, &item) && item.lParam) {
				mir_free((char*)item.lParam);
				TreeView_DeleteItem(hModules, item.hItem);
			}
	}
}

void additem(HWND hwnd2Settings,MCONTACT hContact, char* module, char* setting, int index)
{
	char *data = NULL;
	LVITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = index;
	lvi.iSubItem = 0;

	if (g_db && g_db->IsSettingEncrypted(module, setting)) {
		lvi.iImage = 5;
		ListView_SetItem(hwnd2Settings, &lvi);

		ListView_SetItemTextW(hwnd2Settings, index, 1, TranslateW(L"*** encrypted ***"));

		ListView_SetItemText(hwnd2Settings, index, 2, Translate("UNICODE"));
		ListView_SetItemText(hwnd2Settings, index, 3, "");
		return;
	}

	DBVARIANT dbv = { 0 };
	if (!GetSetting(hContact, module, setting, &dbv)) {
		switch (dbv.type) {
		case DBVT_BLOB:
			if (!(data = (char*)mir_realloc(data, 3 * (dbv.cpbVal + 1) + 10))) {
				msg(Translate("Couldn't allocate enough memory!"), modFullname); return;
			}
			data[0] = '\0';
			for (int j = 0; j < dbv.cpbVal; j++) {
				char tmp[16];
				mir_snprintf(tmp, SIZEOF(tmp), "%02X ", (BYTE)dbv.pbVal[j]);
				strcat(data, tmp);
			}
			lvi.iImage = 0;
			ListView_SetItem(hwnd2Settings, &lvi);
			ListView_SetItemText(hwnd2Settings, index, 1, data);
			ListView_SetItemText(hwnd2Settings, index, 2, Translate("BLOB"));
			mir_snprintf(data, 3 * (dbv.cpbVal + 1) + 10, "0x%04X (%d)", dbv.cpbVal, dbv.cpbVal);
			ListView_SetItemText(hwnd2Settings, index, 3, data);
			break;
		
		case DBVT_BYTE:
			if (!(data = (char*)mir_realloc(data, 16))) // 0x00 (000)
				return;
			lvi.iImage = 1;
			ListView_SetItem(hwnd2Settings, &lvi);
			mir_snprintf(data, 16, "0x%02X (%d)", dbv.bVal, dbv.bVal);
			ListView_SetItemText(hwnd2Settings, index, 1, data);
			ListView_SetItemText(hwnd2Settings, index, 2, Translate("BYTE"));
			ListView_SetItemText(hwnd2Settings, index, 3, "0x0001 (1)");
			break;
		
		case DBVT_WORD:
			if (!(data = (char*)mir_realloc(data, 16))) // 0x0000 (00000)
				return;

			lvi.iImage = 2;
			ListView_SetItem(hwnd2Settings, &lvi);
			mir_snprintf(data, 16, "0x%04X (%ld)", dbv.wVal, dbv.wVal);
			ListView_SetItemText(hwnd2Settings, index, 1, data);
			ListView_SetItemText(hwnd2Settings, index, 2, Translate("WORD"));
			ListView_SetItemText(hwnd2Settings, index, 3, "0x0002 (2)");
			break;
		
		case DBVT_DWORD:
			if (!(data = (char*)mir_realloc(data, 32))) // 0x00000000 (0000000000)
				return;

			lvi.iImage = 3;
			ListView_SetItem(hwnd2Settings, &lvi);
			mir_snprintf(data, 32, "0x%08X (%ld)", dbv.dVal, dbv.dVal);
			ListView_SetItemText(hwnd2Settings, index, 1, data);
			ListView_SetItemText(hwnd2Settings, index, 2, Translate("DWORD"));
			ListView_SetItemText(hwnd2Settings, index, 3, "0x0004 (4)");
			break;
		
		case DBVT_ASCIIZ:
			if ((data = (char*)mir_realloc(data, 512))) {
				lvi.iImage = 4;
				ListView_SetItem(hwnd2Settings, &lvi);
				ListView_SetItemText(hwnd2Settings, index, 1, dbv.pszVal);
				ListView_SetItemText(hwnd2Settings, index, 2, Translate("STRING"));
				int length = mir_strlen(dbv.pszVal) + 1;
				mir_snprintf(data, 512, "0x%04X (%d)", length, length);
				ListView_SetItemText(hwnd2Settings, index, 3, data);
			}
			break;
		
		case DBVT_UTF8:
			if ((data = (char*)mir_realloc(data, 512))) {
				lvi.iImage = 5;
				ListView_SetItem(hwnd2Settings, &lvi);

				int length = (int)strlen(dbv.pszVal) + 1;
				WCHAR *wc = (WCHAR*)_alloca(length*sizeof(WCHAR));
				MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, wc, length);
				ListView_SetItemTextW(hwnd2Settings, index, 1, wc);

				ListView_SetItemText(hwnd2Settings, index, 2, Translate("UNICODE"));
				mir_snprintf(data, 512, "0x%04X (%d)", length, length);
				ListView_SetItemText(hwnd2Settings, index, 3, data);
			}
			break;
		
		case DBVT_DELETED:
			return;
		}
	}
	else if (dbv.type == DBVT_UTF8) {
		lvi.iImage = 5;
		ListView_SetItem(hwnd2Settings, &lvi);
		ListView_SetItemText(hwnd2Settings, index, 1, Translate("<unsupported>"));
		ListView_SetItemText(hwnd2Settings, index, 2, Translate("UNICODE"));
		ListView_SetItemText(hwnd2Settings, index, 3, Translate("<unknown>"));
	}
	else ListView_DeleteItem(hwnd2Settings, index);

	db_free(&dbv);
	mir_free(data);
}

void PopulateSettings(HWND hwnd2Settings, MCONTACT hContact, char* module)
{
	SettingListInfo* info = (SettingListInfo*)mir_calloc(sizeof(SettingListInfo));
	LVITEM lvItem;

	ModuleSettingLL setlist;
	if (!EnumSettings(hContact, module, &setlist)) {
		msg(Translate("Error Loading Setting List"), modFullname);
		mir_free(info);
		return;
	}

	// clear any settings that may be there...
	ClearListview(hwnd2Settings);

	info->hContact = hContact;
	info->module = mir_tstrdup(module);
	SetWindowLongPtr(hwnd2Settings,GWLP_USERDATA, (LONG_PTR)info);

	// icons
	if (himl2)
		ListView_SetImageList(hwnd2Settings, himl2, LVSIL_SMALL);

	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;

	ModSetLinkLinkItem *setting = setlist.first;
	while (setting) {
		lvItem.pszText = setting->name;
		additem(hwnd2Settings, hContact, module, setting->name, ListView_InsertItem(hwnd2Settings, &lvItem));
		setting = (ModSetLinkLinkItem*)setting->next;
	}

	FreeModuleSettingLL(&setlist);
}

void SelectSetting(char *setting)
{
	LVITEM lvItem;
	LVFINDINFO lvfi;
	HWND hwnd2Settings = GetDlgItem(hwnd2mainWindow, IDC_SETTINGS);

	lvfi.flags = LVFI_STRING;
	lvfi.psz = setting;
	lvfi.vkDirection = VK_DOWN;

	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = ListView_FindItem(hwnd2Settings, -1, &lvfi);
	if (lvItem.iItem != -1) {
		lvItem.mask = LVIF_STATE;
		lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
		lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		ListView_SetItem(hwnd2Settings, &lvItem);
	}
}

void settingChanged(HWND hwnd2Settings, MCONTACT hContact, char* module, char* setting)
{
	LVITEM lvItem;
	LVFINDINFO lvfi;

	lvfi.flags = LVFI_STRING;
	lvfi.psz = setting;
	lvfi.vkDirection = VK_DOWN;

	lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
	lvItem.iItem = ListView_FindItem(hwnd2Settings, -1, &lvfi);
	lvItem.iSubItem = 0;

	if (lvItem.iItem == -1) {
		lvItem.iItem = 0;
		lvItem.pszText = setting;
		lvItem.iItem = ListView_InsertItem(hwnd2Settings, &lvItem);
	}
	additem(hwnd2Settings, hContact, module, setting, lvItem.iItem);
}

struct EditLabelInfoStruct
{
	MCONTACT hContact;
	char module[256];
	char setting[256];
	int item;
	int subitem;
	HWND hwnd;
	int unicode;
};

void writeStandardTextfromLabel(EditLabelInfoStruct* info, char* value, WCHAR *wc, int type)
{
	if (type != DBVT_ASCIIZ && type != DBVT_UTF8)
		db_unset(info->hContact, info->module, info->setting);

	if (type == DBVT_UTF8 && wc) {
		db_set_ws(info->hContact, info->module, info->setting, wc);
		mir_free(wc);
	}
	else db_set_s(info->hContact, info->module, info->setting, value);
}

static LRESULT CALLBACK SettingLabelEditSubClassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	EditLabelInfoStruct* info = (EditLabelInfoStruct*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
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
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		SetFocus(hwnd);
		SendMessage(hwnd, WM_SETFONT, SendMessage(GetParent(hwnd), WM_GETFONT, 0, 0), 1);
		info = ((EditLabelInfoStruct*)lParam);
		if (info->subitem)
			SendMessage(hwnd, EM_LIMITTEXT, (WPARAM)65535, 0);
		else
			SendMessage(hwnd, EM_LIMITTEXT, (WPARAM)255, 0);
		SendMessage(hwnd, EM_SETSEL, 0, -1);
		break;
	case WM_PAINT:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				int len = GetWindowTextLengthA(hwnd) + 1;
				char *value = (char*)_alloca(len);
				WCHAR *wc = NULL;
				DBVARIANT dbv = { 0 };

				GetWindowTextA(hwnd, value, len);

				if (info->unicode)
					wc = mir_a2u(value);

				if (len <= 1 || GetSetting(info->hContact, info->module, info->setting, &dbv)) {
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
					return 0;
				}

				switch (info->subitem) {
				case 0: // setting name
					if (!mir_strcmp(info->setting, value) || mir_strlen(value) > 255) {
						db_free(&dbv);
						SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
						return 0;
					}
					db_set(info->hContact, info->module, value, &dbv);
					db_unset(info->hContact, info->module, info->setting);
					{
						LVFINDINFO lvfi;
						lvfi.flags = LVFI_STRING;
						lvfi.psz = info->setting;
						lvfi.vkDirection = VK_DOWN;
						int item = ListView_FindItem(info->hwnd, -1, &lvfi);
						ListView_DeleteItem(info->hwnd, item);
					}
					break;
				
				case 1: // value
					int val;
					int i = 0;

					if (dbv.type == DBVT_BLOB) {
						WriteBlobFromString(info->hContact, info->module, info->setting, value, len);
						break;
					}

					switch (value[0]) {
					case 'b':
					case 'B':
						if (value[1] == '0' && (value[2] == 'x' || value[2] == 'X'))
							sscanf(&value[3], "%x", &val);
						else if (value[1] >= '0' && value[1] <= '9') {
							val = atoi(&value[1]);
							if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8)
								db_unset(info->hContact, info->module, info->setting);

							db_set_b(info->hContact, info->module, info->setting, (BYTE)val);
						}
						else writeStandardTextfromLabel(info, value, wc, dbv.type);
						break;
					case 'w':
					case 'W':
						if (value[1] == '0' && (value[2] == 'x' || value[2] == 'X'))
							sscanf(&value[3], "%x", &val);
						else if (value[1] >= '0' && value[1] <= '9') {
							val = atoi(&value[1]);
							if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8)
								db_unset(info->hContact, info->module, info->setting);
							db_set_w(info->hContact, info->module, info->setting, (WORD)val);
						}
						else writeStandardTextfromLabel(info, value, wc, dbv.type);
						break;
					case 'd':
					case 'D':
						if (value[1] == '0' && (value[2] == 'x' || value[2] == 'X'))
							sscanf(&value[3], "%x", &val);
						else if (value[1] >= '0' && value[1] <= '9') {
							val = atoi(&value[1]);
							if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8)
								db_unset(info->hContact, info->module, info->setting);
							db_set_dw(info->hContact, info->module, info->setting, val);
						}
						else writeStandardTextfromLabel(info, value, wc, dbv.type);
						break;
					case '0':
						i = 1;
					case 'x':
					case 'X':
						if (value[i] == 'x' || value[i] == 'X') {
							sscanf(&value[i + 1], "%x", &val);
							switch (dbv.type) {
							case DBVT_UTF8:
							case DBVT_ASCIIZ:
								writeStandardTextfromLabel(info, value, wc, dbv.type);
								break;
							case DBVT_BYTE:
								db_set_b(info->hContact, info->module, info->setting, (BYTE)val);
								break;
							case DBVT_WORD:
								db_set_w(info->hContact, info->module, info->setting, (WORD)val);
								break;
							case DBVT_DWORD:
								db_set_dw(info->hContact, info->module, info->setting, (DWORD)val);
								break;
							}
						}
						else {
							val = atoi(value);
							switch (dbv.type) {
							case DBVT_ASCIIZ:
							case DBVT_UTF8:
								writeStandardTextfromLabel(info, value, wc, dbv.type);
								break;
							case DBVT_BYTE:
								db_set_b(info->hContact, info->module, info->setting, (BYTE)val);
								break;
							case DBVT_WORD:
								db_set_w(info->hContact, info->module, info->setting, (WORD)val);
								break;
							case DBVT_DWORD:
								db_set_dw(info->hContact, info->module, info->setting, (DWORD)val);
								break;
							}
						}
						break;
					case '\"':
					case '\'':
						{
							int nlen = mir_strlen(value);
							int sh = 0;
							if (nlen > 3) {
								if (value[nlen - 1] == value[0]) {
									value[nlen - 1] = '\0';
									sh = 1;
								}
							}
							writeStandardTextfromLabel(info, &value[sh], wc, dbv.type);
						}
						break;
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '-':
						val = atoi(value);
						switch (dbv.type) {
						case DBVT_ASCIIZ:
						case DBVT_UTF8:
							writeStandardTextfromLabel(info, value, wc, dbv.type);
							break;
						case DBVT_BYTE:
							db_set_b(info->hContact, info->module, info->setting, (BYTE)val);
							break;
						case DBVT_WORD:
							db_set_w(info->hContact, info->module, info->setting, (WORD)val);
							break;
						case DBVT_DWORD:
							db_set_dw(info->hContact, info->module, info->setting, (DWORD)val);
							break;
						}
						break;
					
					default:
						writeStandardTextfromLabel(info, value, wc, dbv.type);
						break;
					}
					break;
				}
				db_free(&dbv);
			} // fall through
		case IDCANCEL:
			SettingListInfo *sli = (SettingListInfo*)GetWindowLongPtr(info->hwnd, GWLP_USERDATA);
			if (sli && sli->hwnd2Edit == hwnd)
				sli->hwnd2Edit = NULL;

			mir_free(info);
			DestroyWindow(hwnd);
			return 0;
		}
		break; // wm_command

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}
	return mir_callNextSubclass(hwnd, SettingLabelEditSubClassProc, msg, wParam, lParam);
}

void EditLabel(HWND hwnd2List, int item, int subitem)
{
	RECT rc;
	char setting[256], value[16] = { 0 };
	DBVARIANT dbv;
	SettingListInfo* info = (SettingListInfo*)GetWindowLongPtr(hwnd2List, GWLP_USERDATA);
	EditLabelInfoStruct *data = (EditLabelInfoStruct*)mir_calloc(sizeof(EditLabelInfoStruct));
	if (!data || !info)
		return;

	if (info->hwnd2Edit) {
		SendMessage(info->hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0); // ignore the new value of the last edit
		info->hwnd2Edit = NULL;
	}

	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.pszText = setting;
	lvi.cchTextMax = SIZEOF(setting);
	if (!ListView_GetItem(hwnd2List, &lvi) ||
		!ListView_GetSubItemRect
		(hwnd2List, item, subitem, LVIR_LABEL, &rc) ||
		GetSetting(info->hContact, info->module, setting, &dbv)) {
		mir_free(data);
		return;
	}

	data->hContact = info->hContact;
	strcpy(data->module, info->module);
	strcpy(data->setting, setting);
	data->item = item;
	data->subitem = subitem;
	data->hwnd = hwnd2List;

	// fix size for long strings

	switch (dbv.type) {
	case DBVT_UTF8:
		if (subitem) {
			int len = mir_strlen(dbv.pszVal) + 1;
			WCHAR *wc = (WCHAR*)_alloca(len*sizeof(WCHAR));
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, wc, len);
			data->unicode = 1;
			info->hwnd2Edit = CreateWindowW(L"EDIT", wc, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL, rc.left, rc.top, (int)((rc.right - rc.left)*1.5), (rc.bottom - rc.top) * 3, hwnd2List, 0, hInst, 0);
			break;
		}
		// fall through
	case DBVT_ASCIIZ:
		if (subitem) // convert from UTF8
			info->hwnd2Edit = CreateWindow("EDIT", dbv.pszVal, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL, rc.left, rc.top, (int)((rc.right - rc.left)*1.5), (rc.bottom - rc.top) * 3, hwnd2List, 0, hInst, 0);
		else
			info->hwnd2Edit = CreateWindow("EDIT", setting, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);
		break;
	case DBVT_BYTE:
		if (Hex & HEX_BYTE)
			mir_snprintf(value, SIZEOF(value), "0x%02X", dbv.bVal);
		else
			itoa(dbv.bVal, value, 10);
		info->hwnd2Edit = CreateWindow("EDIT", !subitem ? setting : value, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);
		break;
	case DBVT_WORD:
		if (Hex & HEX_WORD)
			mir_snprintf(value, SIZEOF(value), "0x%04X", dbv.wVal);
		else
			itoa(dbv.wVal, value, 10);
		info->hwnd2Edit = CreateWindow("EDIT", !subitem ? setting : value, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);
		break;
	case DBVT_DWORD:
		if (Hex & HEX_DWORD)
			mir_snprintf(value, SIZEOF(value), "0x%08X", dbv.dVal);
		else
			itoa(dbv.dVal, value, 10);
		info->hwnd2Edit = CreateWindow("EDIT", !subitem ? setting : value, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);
		break;
	case DBVT_BLOB:
		if (!subitem)
			info->hwnd2Edit = CreateWindow("EDIT", setting, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), hwnd2List, 0, hInst, 0);
		else {
			int j;
			char tmp[16];
			char *data = (char*)_alloca(3 * (dbv.cpbVal + 1) + 10);

			if (!data) { msg(Translate("Couldn't allocate enough memory!"), modFullname); return; }
			data[0] = '\0';

			for (j = 0; j < dbv.cpbVal; j++) {
				mir_snprintf(tmp, SIZEOF(tmp), "%02X ", (BYTE)dbv.pbVal[j]);
				strcat(data, tmp);
			}

			info->hwnd2Edit = CreateWindow("EDIT", data, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE, rc.left, rc.top, (int)((rc.right - rc.left)*1.5), (rc.bottom - rc.top) * 3, hwnd2List, 0, hInst, 0);
		}
		break;
	default:
		return;
	}

	db_free(&dbv);

	mir_subclassWindow(info->hwnd2Edit, SettingLabelEditSubClassProc);

	SendMessage(info->hwnd2Edit, WM_USER, 0, (LPARAM)data);
}

static int test;
void SettingsListRightClick(HWND hwnd, WPARAM wParam,LPARAM lParam);
static int lastColumn = -1;

struct SettingsSortParams
{
	HWND hList;
	int column;
};

INT_PTR CALLBACK SettingsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	SettingsSortParams params = *(SettingsSortParams *)myParam;
	const int maxSize = 1024;
	TCHAR text1[maxSize];
	TCHAR text2[maxSize];
	ListView_GetItemText(params.hList, lParam1, params.column, text1, maxSize);
	ListView_GetItemText(params.hList, lParam2, params.column, text2, maxSize);

	int res = _tcsicmp(text1, text2);
	res = (params.column == lastColumn) ? -res : res;
	return res;
}

void SettingsListWM_NOTIFY(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SettingListInfo* info = (SettingListInfo*)GetWindowLongPtr(GetDlgItem(hwnd, IDC_SETTINGS), GWLP_USERDATA);
	LVHITTESTINFO hti;

	switch (((NMHDR*)lParam)->code) {
	case NM_CLICK:

		hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
		if (db_get_b(NULL, modname, "DontAllowInLineEdit", 0))
			break;
		if (info && ListView_SubItemHitTest(GetDlgItem(hwnd, IDC_SETTINGS), &hti) > -1) {
			if (hti.iSubItem < 2 && hti.flags != LVHT_ONITEMICON) {
				if (info->selectedItem == hti.iItem)
					EditLabel(GetDlgItem(hwnd, IDC_SETTINGS), hti.iItem, hti.iSubItem);
				else if (info->hwnd2Edit) {
					SendMessage(info->hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
					info->hwnd2Edit = NULL;
					info->selectedItem = hti.iItem;
				}
				else info->selectedItem = hti.iItem;
			}
			else {
				if (info->hwnd2Edit) {
					SendMessage(info->hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
					info->hwnd2Edit = NULL;
				}
				info->selectedItem = hti.iItem;
			}
		}
		else if (info && info->hwnd2Edit) {
			SendMessage(info->hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
			info->hwnd2Edit = NULL;
			info->selectedItem = 0;
		}
		break;

	case NM_DBLCLK:
		hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
		if (info && ListView_SubItemHitTest(GetDlgItem(hwnd, IDC_SETTINGS), &hti) > -1) {
			if ((hti.iSubItem > 1 || hti.flags == LVHT_ONITEMICON) || (db_get_b(NULL, modname, "DontAllowInLineEdit", 0))) {
				char setting[256];
				SendMessage(info->hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
				info->hwnd2Edit = NULL;
				ListView_GetItemText(GetDlgItem(hwnd, IDC_SETTINGS), hti.iItem, 0, setting, 256);
				editSetting(info->hContact, info->module, setting);
			}
			else EditLabel(GetDlgItem(hwnd, IDC_SETTINGS), hti.iItem, hti.iSubItem);
		}
		break;

	case NM_RCLICK:
		SettingsListRightClick(hwnd, wParam, lParam);
		break;

	case LVN_COLUMNCLICK:
		LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
		SettingsSortParams params = { 0 };
		params.hList = GetDlgItem(hwnd, IDC_SETTINGS);
		params.column = lv->iSubItem;
		ListView_SortItemsEx(params.hList, SettingsCompare, (LPARAM)&params);
		lastColumn = (params.column == lastColumn) ? -1 : params.column;
		break;
	}
}

void SettingsListRightClick(HWND hwnd, WPARAM wParam, LPARAM lParam) // hwnd here is to the main window, NOT the listview
{
	HWND hSettings = GetDlgItem(hwnd, IDC_SETTINGS);
	SettingListInfo* info = (SettingListInfo*)GetWindowLongPtr(hSettings, GWLP_USERDATA);
	if (!info)
		return;

	char setting[256], *module;
	MCONTACT hContact;
	LVHITTESTINFO hti;
	POINT pt;
	HMENU hMenu, hSubMenu;
	DBsetting *dbsetting;
	DBVARIANT dbv = { 0 }; // freed in the dialog

	module = info->module;
	hContact = info->hContact;

	hti.pt = ((NMLISTVIEW*)lParam)->ptAction;
	if (ListView_SubItemHitTest(hSettings, &hti) == -1) {
		// nowhere.. new item menu
		GetCursorPos(&pt);
		hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXTMENU));
		hSubMenu = GetSubMenu(hMenu, 6);
		TranslateMenu(hSubMenu);

		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL)) {
		case MENU_ADD_BYTE:
			dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
			dbv.type = DBVT_BYTE;
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_tstrdup(module);
			dbsetting->setting = mir_tstrdup("");
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			break;

		case MENU_ADD_WORD:
			dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
			dbv.type = DBVT_WORD;
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_tstrdup(module);
			dbsetting->setting = mir_tstrdup("");
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			break;

		case MENU_ADD_DWORD:
			dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
			dbv.type = DBVT_DWORD;
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_tstrdup(module);
			dbsetting->setting = mir_tstrdup("");
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			break;

		case MENU_ADD_STRING:
			dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
			dbv.type = DBVT_ASCIIZ;
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_tstrdup(module);
			dbsetting->setting = mir_tstrdup("");
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			break;

		case MENU_ADD_UNICODE:
			dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
			dbv.type = DBVT_UTF8;
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_tstrdup(module);
			dbsetting->setting = mir_tstrdup("");
			CreateDialogParamW(hInst, MAKEINTRESOURCEW(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			break;

		case MENU_ADD_BLOB:
			dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
			dbv.type = DBVT_BLOB;
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_tstrdup(module);
			dbsetting->setting = mir_tstrdup("");
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			break;
		}
		return;
	}

	// on item
	char type[8];
	LVITEM lvi;
	int i;
	int watching = 0;
	GetCursorPos(&pt);
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXTMENU));
	hSubMenu = GetSubMenu(hMenu, 0);
	TranslateMenu(hSubMenu);

	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = hti.iItem;
	lvi.iSubItem = 0;
	lvi.pszText = setting;
	lvi.cchTextMax = SIZEOF(setting);

	ListView_GetItem(hSettings, &lvi);
	ListView_GetItemText(hSettings, hti.iItem, 2, type, 8);

	switch (lvi.iImage) {
	case 4: // STRING
		RemoveMenu(hSubMenu, MENU_CHANGE2STRING, MF_BYCOMMAND);
		break;
	case 1: // BYTE
		RemoveMenu(hSubMenu, 4, MF_BYPOSITION);
		RemoveMenu(hSubMenu, MENU_CHANGE2BYTE, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	case 2: // WORD
		RemoveMenu(hSubMenu, 4, MF_BYPOSITION);
		RemoveMenu(hSubMenu, MENU_CHANGE2WORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	case 3: // DWORD
		RemoveMenu(hSubMenu, 4, MF_BYPOSITION);
		RemoveMenu(hSubMenu, MENU_CHANGE2DWORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	case 0: // BLOB
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION);
		RemoveMenu(hSubMenu, 1, MF_BYPOSITION);
		RemoveMenu(hSubMenu, 2, MF_BYPOSITION);
		RemoveMenu(hSubMenu, MENU_EDIT_SET, MF_BYCOMMAND);
		break;
	case 5: // UTF8
		RemoveMenu(hSubMenu, 4, MF_BYPOSITION);
		RemoveMenu(hSubMenu, MENU_CHANGE2DWORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2WORD, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2BYTE, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_CHANGE2UNICODE, MF_BYCOMMAND);
		break;
	}

	// watch list stuff

	if (ListView_GetSelectedCount(hSettings) > 1) {
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION);
		RemoveMenu(hSubMenu, 1, MF_BYPOSITION);
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION);
		RemoveMenu(hSubMenu, MENU_EDIT_SET, MF_BYCOMMAND);
	}

	// check if the setting is being watched and if it is then check the menu item
	for (i = 0; i < WatchListArray.count; i++) {
		if (WatchListArray.item[i].module && (hContact == WatchListArray.item[i].hContact)) {
			if (WatchListArray.item[i].WatchModule == WATCH_MODULE)
				continue;

			if (!mir_strcmp(module, WatchListArray.item[i].module) && WatchListArray.item[i].setting[0]) {
				if (!mir_strcmp(setting, WatchListArray.item[i].setting)) {
					// yes so uncheck it
					CheckMenuItem(hSubMenu, MENU_WATCH_ITEM, MF_CHECKED | MF_BYCOMMAND);
					watching = 1;
					break;
				}
			}
		}
	}

	switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL)) {
	case MENU_EDIT_SET:
		editSetting(info->hContact, info->module, setting);
		break;

	case MENU_ADD_BYTE:
		dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
		dbv.type = DBVT_BYTE;
		dbsetting->dbv = dbv;
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup("");
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
		break;

	case MENU_ADD_WORD:
		dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
		dbv.type = DBVT_WORD;
		dbsetting->dbv = dbv;
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup("");
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
		break;

	case MENU_ADD_DWORD:
		dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
		dbv.type = DBVT_DWORD;
		dbsetting->dbv = dbv;
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup("");
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
		break;

	case MENU_ADD_STRING:
		dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
		dbv.type = DBVT_ASCIIZ;
		dbsetting->dbv = dbv;
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup("");
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
		break;

	case MENU_ADD_UNICODE:
		dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
		dbv.type = DBVT_UTF8;
		dbsetting->dbv = dbv;
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup("");
		CreateDialogParamW(hInst, MAKEINTRESOURCEW(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
		break;

	case MENU_ADD_BLOB:
		dbsetting = (DBsetting*)mir_alloc(sizeof(DBsetting)); // gets safe_free()ed in the window proc
		dbv.type = DBVT_BLOB;
		dbsetting->dbv = dbv;
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup("");
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
		break;

	///////////////////////// convert to submenu
	case MENU_CHANGE2BYTE:
		if (convertSetting(hContact, module, setting, 0)) {
			lvi.iImage = 1;
			ListView_SetItem(hSettings, &lvi);
		}
		break;

	case MENU_CHANGE2WORD:
		if (convertSetting(hContact, module, setting, 1)) {
			lvi.iImage = 2;
			ListView_SetItem(hSettings, &lvi);
		}
		break;

	case MENU_CHANGE2DWORD:
		if (convertSetting(hContact, module, setting, 2)) {
			lvi.iImage = 3;
			ListView_SetItem(hSettings, &lvi);
		}
		break;

	case MENU_CHANGE2STRING:
		if (convertSetting(hContact, module, setting, 3)) {
			lvi.iImage = 4;
			ListView_SetItem(hSettings, &lvi);
		}
		break;

	case MENU_CHANGE2UNICODE:
		if (convertSetting(hContact, module, setting, 4)) {
			lvi.iImage = 5;
			ListView_SetItem(hSettings, &lvi);
		}
		break;

	///////////////////////// convert to submenu
	case MENU_VIEWDECRYPT:
		if (!db_get(hContact, module, setting, &dbv) && dbv.type == DBVT_ASCIIZ) {
			if (mir_strcmp(setting, "LoginPassword"))
				msg(dbv.pszVal, Translate("Decoded string.."));
			else {
				char *str = mir_strdup(dbv.pszVal);
				char *str1 = str;
				for (; *str1; ++str1) {
					const char c = *str1 ^ 0xc3;
					if (c) *str1 = c;
				}

				WCHAR *res = mir_utf8decodeW(str);
				MessageBoxW(0, res, TranslateW(L"Decoded string.."), MB_OK);
				mir_free(res);
				mir_free(str);
			}
			db_free(&dbv);
		}
		break;

	case MENU_VIEWENCRYPT:
		if (!db_get(hContact, module, setting, &dbv) && dbv.type == DBVT_ASCIIZ)
			msg(dbv.pszVal, Translate("Encoded string.."));
		db_free(&dbv);
		break;

	case MENU_DECRYPT:
		if (!db_get_s(hContact, module, setting, &dbv))
			db_set_s(hContact, module, setting, dbv.pszVal);
		db_free(&dbv);
		break;

	case MENU_ENCRYPT:
		if (!db_get_s(hContact, module, setting, &dbv))
			db_set_s(hContact, module, setting, dbv.pszVal);
		db_free(&dbv);
		break;

	///////////////////////// divider
	case MENU_WATCH_ITEM:
		if (!watching)
			addSettingToWatchList(hContact, module, setting);
		else
			freeWatchListItem(i);
		if (hwnd2watchedVarsWindow)
			PopulateWatchedWindow(GetDlgItem(hwnd2watchedVarsWindow, IDC_VARS));
		break;

	case MENU_DELETE_SET:
		DeleteSettingsFromList(hSettings, hContact, module, setting);
		break;
	}
}
