#include "stdafx.h"

SettingListInfo info = {};

int convertSetting(MCONTACT hContact, const char *module, const char *setting, int toType)
{
	DBVARIANT dbv = {};

	if (db_get_s(hContact, module, setting, &dbv, 0)) return 0;

	if (dbv.type == toType) {
		db_free(&dbv);
		return 1;
	}

	int res = 0;
	DWORD val = 0;
	wchar_t tmp[16];
	ptrW value;

	switch (dbv.type) {
	case DBVT_BYTE:
	case DBVT_WORD:
	case DBVT_DWORD:
		val = getNumericValue(&dbv);
		value = mir_wstrdup(_ultow(val, tmp, 10));
		break;

	case DBVT_WCHAR:
		if (!value)
			value = mir_wstrdup(dbv.pwszVal);
		__fallthrough;

	case DBVT_UTF8:
		if (!value)
			value = mir_utf8decodeW(dbv.pszVal);
		__fallthrough;

	case DBVT_ASCIIZ:
		if (!value)
			value = mir_a2u(dbv.pszVal);

		if (mir_wstrlen(value) < 11)
			val = wcstoul(value, nullptr, NULL);
	}

	switch (toType) {
	case DBVT_BYTE:
	case DBVT_WORD:
	case DBVT_DWORD:
		if (val != 0 || !mir_wstrcmp(value, L"0"))
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
		g_pMainWindow->msg(TranslateT("Unable to store value in this data type!"));

	db_free(&dbv);

	return res;
}

void EditFinish(int selected)
{
	if (info.hwnd2Edit) {
		SendMessage(info.hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
		info.hwnd2Edit = nullptr;
	}
	info.selectedItem = selected;
}

void CMainDlg::ClearListView()
{
	EditFinish(0);
	info.module[0] = 0;
	info.setting[0] = 0;
	info.hContact = 0;

	m_settings.DeleteAllItems();
}

void CMainDlg::DeleteSettingsFromList(MCONTACT hContact, const char *module, const char *setting)
{
	int count = m_settings.GetSelectedCount();
	if (!count)
		return;

	bool isContacts = (hContact == 0 && mir_strlen(module) == 0);

	if (g_plugin.bWarnOnDelete) {
		wchar_t text[MSG_SIZE];
		if (isContacts)
			mir_snwprintf(text, TranslateT("Are you sure you want to delete %d contact(s)?"), count);
		else
			mir_snwprintf(text, TranslateT("Are you sure you want to delete %d setting(s)?"), count);
		if (g_pMainWindow->dlg(text, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			return;
	}

	if (isContacts) {
		int items = m_settings.GetItemCount();
		for (int i = 0; i < items;) {
			if (m_settings.GetItemState(i, LVIS_SELECTED)) {
				LVITEM lvi = {};
				lvi.mask = LVIF_PARAM;
				lvi.iItem = i;
				if (m_settings.GetItem(&lvi)) {
					db_delete_contact(MCONTACT(lvi.lParam));
					m_settings.DeleteItem(i);
				}
				items--;
			}
			else i++;
		}
	}
	else {
		if (count == 1)
			db_unset(hContact, module, setting);
		else {
			int items = m_settings.GetItemCount();
			for (int i = 0; i < items;) {
				if (m_settings.GetItemState(i, LVIS_SELECTED)) {
					char text[FLD_SIZE];
					if (ListView_GetItemTextA(m_settings.GetHwnd(), i, 0, text, _countof(text)))
						db_unset(hContact, module, text);
					items--;
				}
				else i++;
			}
		}
	}

	if (m_settings.GetItemCount() == 0)
		replaceTreeItem(hContact, module, nullptr);
}

int CMainDlg::findListItem(const char *setting)
{
	if (!setting || !setting[0])
		return -1;

	int items = m_settings.GetItemCount();
	for (int i = 0; i < items; i++) {
		char text[FLD_SIZE];
		if (ListView_GetItemTextA(m_settings.GetHwnd(), i, 0, text, _countof(text)))
			if (!strcmp(setting, text))
				return i;
	}

	return -1;
}

void CMainDlg::deleteListItem(const char *setting)
{
	int item = findListItem(setting);
	if (item > -1)
		m_settings.DeleteItem(item);
}

void CMainDlg::updateListItem(int index, const char *setting, DBVARIANT *dbv, int resident)
{
	if (!dbv || !dbv->type) {
		m_settings.DeleteItem(index);
		return;
	}

	if (index < 0)
		return;

	LVITEM lvi = {};
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = index;

	m_settings.SetItemText(index, 4, resident ? L"[R]" : L"");

	if (g_db && g_db->IsSettingEncrypted(info.module, setting)) {
		lvi.iImage = IMAGE_UNICODE;
		m_settings.SetItem(&lvi);
		m_settings.SetItemText(index, 0, _A2T(setting));
		m_settings.SetItemText(index, 1, TranslateT("*** encrypted ***"));
		m_settings.SetItemText(index, 2, L"UNICODE");
		m_settings.SetItemText(index, 3, L"");
		return;
	}

	wchar_t data[32];
	int length;

	switch (dbv->type) {
	case DBVT_BLOB:
		lvi.iImage = IMAGE_BINARY;
		m_settings.SetItem(&lvi);

		m_settings.SetItemText(index, 1, _A2T(ptrA(StringFromBlob(dbv->pbVal, dbv->cpbVal))));

		mir_snwprintf(data, L"0x%04X (%u)", dbv->cpbVal, dbv->cpbVal);
		m_settings.SetItemText(index, 3, data);
		break;

	case DBVT_BYTE:
		lvi.iImage = IMAGE_BYTE;
		m_settings.SetItem(&lvi);

		mir_snwprintf(data, L"0x%02X (%u)", dbv->bVal, dbv->bVal);
		m_settings.SetItemText(index, 1, data);

		m_settings.SetItemText(index, 3, L"0x0001 (1)");
		break;

	case DBVT_WORD:
		lvi.iImage = IMAGE_WORD;
		m_settings.SetItem(&lvi);

		mir_snwprintf(data, L"0x%04X (%u)", dbv->wVal, dbv->wVal);
		m_settings.SetItemText(index, 1, data);

		m_settings.SetItemText(index, 3, L"0x0002 (2)");
		break;

	case DBVT_DWORD:
		lvi.iImage = IMAGE_DWORD;
		m_settings.SetItem(&lvi);

		mir_snwprintf(data, L"0x%08X (%u)", dbv->dVal, dbv->dVal);
		m_settings.SetItemText(index, 1, data);

		m_settings.SetItemText(index, 3, L"0x0004 (4)");
		break;

	case DBVT_ASCIIZ:
		lvi.iImage = IMAGE_STRING;
		m_settings.SetItem(&lvi);
		m_settings.SetItemText(index, 1, _A2T(dbv->pszVal));

		length = (int)mir_strlen(dbv->pszVal) + 1;
		mir_snwprintf(data, L"0x%04X (%u)", length, length);
		m_settings.SetItemText(index, 3, data);
		break;

	case DBVT_WCHAR:
		lvi.iImage = IMAGE_UNICODE;
		m_settings.SetItem(&lvi);
		m_settings.SetItemText(index, 1, dbv->pwszVal);

		length = (int)mir_wstrlen(dbv->pwszVal) + 1;
		mir_snwprintf(data, L"0x%04X (%u)", length, length);
		m_settings.SetItemText(index, 3, data);
		break;

	case DBVT_UTF8:
		lvi.iImage = IMAGE_UNICODE;
		m_settings.SetItem(&lvi);
		{
			ptrW wszText(mir_utf8decodeW(dbv->pszVal));
			m_settings.SetItemText(index, 1, wszText);
		}
		length = (int)mir_strlen(dbv->pszVal) + 1;
		mir_snwprintf(data, L"0x%04X (%u)", length, length);
		m_settings.SetItemText(index, 3, data);
		break;

	default:
		return;
	}

	m_settings.SetItemText(index, 2, DBVType(dbv->type));
}

void CMainDlg::addListHandle(MCONTACT hContact)
{
	wchar_t name[NAME_SIZE], data[32];
	LVITEM lvi = {};
	lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
	lvi.lParam = hContact;
	lvi.iImage = IMAGE_HANDLE;

	GetContactName(hContact, nullptr, name, _countof(name));
	lvi.pszText = name;

	int index = m_settings.InsertItem(&lvi);

	mir_snwprintf(data, L"0x%08X (%ld)", hContact, hContact);

	m_settings.SetItemText(index, 1, data);
	m_settings.SetItemText(index, 2, L"HANDLE");
	m_settings.SetItemText(index, 3, L"0x0004 (4)");
	if (db_mc_isEnabled()) {
		if (db_mc_isSub(hContact)) {
			m_settings.SetItemText(index, 4, L"[S]");
		}
		else if (db_mc_isMeta(hContact)) {
			m_settings.SetItemText(index, 4, L"[M]");
		}
	}
}

void CMainDlg::addListItem(const char *setting, int resident)
{
	DBVARIANT dbv;
	if (!db_get_s(info.hContact, info.module, setting, &dbv, 0)) {
		LVITEMA lvi = {};
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (char *)setting;
		int index = SendMessageA(m_settings.GetHwnd(), LVM_INSERTITEMA, 0, (LPARAM)&lvi);
		updateListItem(index, setting, &dbv, resident);
		db_free(&dbv);
	}
	else if (!resident) {
		LVITEMA lvi = {};
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (char *)setting;
		int index = SendMessageA(m_settings.GetHwnd(), LVM_INSERTITEMA, 0, (LPARAM)&lvi);
		m_settings.SetItemText(index, 1, TranslateT("*** buggy resident ***"));
	}
}

void CMainDlg::PopulateSettings(MCONTACT hContact, const char *module)
{
	// save module as it can be erased by ClearListView()
	char tmp[FLD_SIZE];
	mir_strncpy(tmp, module, _countof(tmp));

	// clear any settings that may be there...
	ClearListView();

	info.hContact = hContact;
	mir_strncpy(info.module, tmp, _countof(info.module));

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

void CMainDlg::SelectSetting(const char *setting)
{
	LVITEM lvItem = {};
	lvItem.mask = LVIF_STATE;
	lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

	int items = m_settings.GetItemCount();
	for (int i = 0; i < items; i++)
		if (m_settings.GetItemState(i, lvItem.stateMask))
			m_settings.SetItemState(i, 0, lvItem.stateMask);

	lvItem.iItem = findListItem(setting);
	if (lvItem.iItem != -1) {
		EditFinish(lvItem.iItem);
		
		lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
		m_settings.SetItem(&lvItem);

		m_settings.EnsureVisible(lvItem.iItem, false);
	}
}

void CMainDlg::onSettingChanged(MCONTACT hContact, DBCONTACTWRITESETTING *cws)
{
	// modules tree
	if (cws->value.type != DBVT_DELETED) {
		HTREEITEM hItem = findItemInTree(hContact, cws->szModule);
		if (!hItem) {
			HTREEITEM hParent = findItemInTree(hContact, nullptr);
			if (hParent)
				insertItem(hContact, cws->szModule, hParent);
		}
	}

	// settings list
	if (hContact != info.hContact || mir_strcmp(info.module, cws->szModule))
		return;

	if (cws->value.type != DBVT_DELETED) {
		int iItem = findListItem(cws->szSetting);
		if (iItem == -1)
			iItem = m_settings.AddItem(_A2T(cws->szSetting), 0);

		updateListItem(iItem, cws->szSetting, &cws->value, IsResidentSetting(cws->szModule, cws->szSetting));
	}
	else deleteListItem(cws->szSetting);
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
		SetFocus(hwnd);
		SendMessage(hwnd, WM_SETFONT, SendMessage(GetParent(hwnd), WM_GETFONT, 0, 0), 1);
		if (info.subitem)
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
				DBVARIANT dbv = {};
				int len = GetWindowTextLength(hwnd) + 1;

				if ((!info.subitem && len <= 1) || db_get_s(info.hContact, info.module, info.setting, &dbv, 0)) {
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
					return 0;
				}

				wchar_t *value = (wchar_t *)mir_alloc(len * sizeof(wchar_t));

				GetWindowTextW(hwnd, value, len);

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
					g_pMainWindow->deleteListItem(info.setting);
					break;

				case 1: // value
					DWORD val;
					int i = 0;

					if (dbv.type == DBVT_BLOB) {
						res = WriteBlobFromString(info.hContact, info.module, info.setting, szValue, (int)mir_strlen(szValue));
						break;
					}

					switch (value[0]) {
					case 'b':
					case 'B':
						val = wcstoul(&value[1], nullptr, 0);
						if (!val || value[1] == '0') {
							res = !db_set_b(info.hContact, info.module, info.setting, (BYTE)val);
						}
						else
							res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
						break;
					case 'w':
					case 'W':
						val = wcstoul(&value[1], nullptr, 0);
						if (!val || value[1] == '0')
							res = !db_set_w(info.hContact, info.module, info.setting, (WORD)val);
						else
							res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
						break;
					case 'd':
					case 'D':
						val = wcstoul(&value[1], nullptr, 0);
						if (!val || value[1] == '0')
							res = !db_set_dw(info.hContact, info.module, info.setting, val);
						else
							res = setTextValue(info.hContact, info.module, info.setting, value, dbv.type);
						break;

					case '0':
						i = 1;
						__fallthrough;

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
					case 'x':
					case 'X':
						if (value[i] == 'x' || value[i] == 'X')
							val = wcstoul(&value[i + 1], nullptr, 16);
						else
							val = wcstoul(value, nullptr, 10);

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
					case '\"':
					case '\'':
						{
							size_t nlen = mir_wstrlen(value);
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
					g_pMainWindow->msg(TranslateT("Unable to store value in this data type!"));
					break;
				}
			}
			__fallthrough;

		case IDCANCEL:
			DestroyWindow(hwnd);
			return 0;
		}

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_DESTROY:
		info.hwnd2Edit = nullptr;
		break;
	}
	return mir_callNextSubclass(hwnd, SettingLabelEditSubClassProc, msg, wParam, lParam);
}

void CMainDlg::EditLabel(int item, int subitem)
{
	if (info.hwnd2Edit) {
		SendMessage(info.hwnd2Edit, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0); // ignore the new value of the last edit
		info.hwnd2Edit = nullptr;
	}

	char setting[FLD_SIZE];
	if (!ListView_GetItemTextA(m_settings.GetHwnd(),item, 0, setting, _countof(setting)))
		return;
	
	RECT rc;
	if (!setting[0] || !m_settings.GetSubItemRect(item, subitem, LVIR_LABEL, &rc))
		return;

	DBVARIANT dbv;
	if (db_get_s(info.hContact, info.module, setting, &dbv, 0))
		return;

	mir_strcpy(info.setting, setting);
	info.subitem = subitem;

	if (!subitem)
		info.hwnd2Edit = CreateWindowW(L"EDIT", _A2T(setting), WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), m_settings.GetHwnd(), nullptr, g_plugin.getInst(), nullptr);
	else {
		wchar_t *str = nullptr, value[16] = {};

		switch (dbv.type) {
		case DBVT_ASCIIZ:
			str = mir_a2u(dbv.pszVal);
			break;
		case DBVT_BYTE:
			mir_snwprintf(value, (g_Hex & HEX_BYTE) ? L"0x%02X" : L"%u", dbv.bVal);
			break;
		case DBVT_WORD:
			mir_snwprintf(value, (g_Hex & HEX_WORD) ? L"0x%04X" : L"%u", dbv.wVal);
			break;
		case DBVT_DWORD:
			mir_snwprintf(value, (g_Hex & HEX_DWORD) ? L"0x%08X" : L"%u", dbv.dVal);
			break;
		case DBVT_WCHAR:
			str = mir_wstrdup(dbv.pwszVal);
			break;
		case DBVT_UTF8:
			str = mir_utf8decodeW(dbv.pszVal);
			break;
		case DBVT_BLOB:
			str = mir_a2u(ptrA(StringFromBlob(dbv.pbVal, dbv.cpbVal)));
			break;
		}

		if (str) {
			int height = (rc.bottom - rc.top) * 4;
			RECT rclist;
			GetClientRect(m_settings.GetHwnd(), &rclist);
			if (rc.top + height > rclist.bottom &&rclist.bottom - rclist.top > height)
				rc.top = rc.bottom - height;
			info.hwnd2Edit = CreateWindow(L"EDIT", str, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL, rc.left, rc.top, rc.right - rc.left, height, m_settings.GetHwnd(), nullptr, g_plugin.getInst(), nullptr);
			mir_free(str);
		}
		else if (dbv.type == DBVT_BYTE || dbv.type == DBVT_WORD || dbv.type == DBVT_DWORD)
			info.hwnd2Edit = CreateWindow(L"EDIT", value, WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), m_settings.GetHwnd(), nullptr, g_plugin.getInst(), nullptr);
	}

	db_free(&dbv);

	mir_subclassWindow(info.hwnd2Edit, SettingLabelEditSubClassProc);
	SendMessage(info.hwnd2Edit, WM_USER, 0, 0);
}

// hwnd here is to the main window, NOT the listview
void CMainDlg::onContextMenu_Settings(CContextMenuPos *pos)
{
	if (pos->iCurr == -1) {
		// nowhere.. new item menu
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXTMENU));
		HMENU hSubMenu = GetSubMenu(hMenu, 6);
		TranslateMenu(hSubMenu);

		if (!info.module[0]) {
			RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // new
			RemoveMenu(hSubMenu, 0, MF_BYPOSITION); // separator
		}

		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr)) {
		case MENU_ADD_BYTE:
			newSetting(info.hContact, info.module, DBVT_BYTE);
			break;

		case MENU_ADD_WORD:
			newSetting(info.hContact, info.module, DBVT_WORD);
			break;

		case MENU_ADD_DWORD:
			newSetting(info.hContact, info.module, DBVT_DWORD);
			break;

		case MENU_ADD_STRING:
			newSetting(info.hContact, info.module, DBVT_ASCIIZ);
			break;

		case MENU_ADD_UNICODE:
			newSetting(info.hContact, info.module, DBVT_WCHAR);
			break;

		case MENU_ADD_BLOB:
			newSetting(info.hContact, info.module, DBVT_BLOB);
			break;

		case MENU_REFRESH:
			PopulateSettings(info.hContact, info.module);
			break;
		}
		return;
	}

	// on item
	HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXTMENU));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	TranslateMenu(hSubMenu);

	LVITEM lvi = {};
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = pos->iCurr;
	m_settings.GetItem(&lvi);

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
		RemoveMenu(hSubMenu, MENU_WATCH_ITEM, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, 1, MF_BYPOSITION); // convert
		RemoveMenu(hSubMenu, 1, MF_BYPOSITION); // separator
		RemoveMenu(hSubMenu, 1, MF_BYPOSITION); // new
		break;
	}

	if (m_settings.GetSelectedCount() > 1) {
		RemoveMenu(hSubMenu, 3, MF_BYPOSITION); // convert
		RemoveMenu(hSubMenu, MENU_EDIT_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_COPY_SET, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, MENU_WATCH_ITEM, MF_BYCOMMAND);
	}

	char setting[FLD_SIZE];
	if (!ListView_GetItemTextA(m_settings.GetHwnd(), pos->iCurr, 0, setting, _countof(setting)))
		return;

	// check if the setting is being watched and if it is then check the menu item
	int watchIdx = WatchedArrayIndex(info.hContact, info.module, setting, 1);
	if (watchIdx >= 0)
		CheckMenuItem(hSubMenu, MENU_WATCH_ITEM, MF_CHECKED | MF_BYCOMMAND);

	switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr)) {
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

	case MENU_ADD_UNICODE:
		newSetting(info.hContact, info.module, DBVT_WCHAR);
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
