#include "stdafx.h"

#define F_CASE		1
#define F_EXACT		2
#define F_MODNAME	4
#define F_SETNAME	8
#define F_SETVAL	0x10

#define F_ENTIRE 	0x20

#define F_NUMSRCH	0x40
#define F_NUMREPL	0x80
#define F_UNICODE	0x100

#define F_REPLACED	0x200
#define F_DELETED	0x400

static int lastColumn = -1;

struct FindInfo
{
	HWND hwnd; // hwnd to item list
	wchar_t* search; // text to find
	wchar_t* replace; // text to replace
	int options; // or'd about items
};

static ColumnsSettings csResultList[] =
{
	{ LPGENW("Result"),  0, "Search0width", 100 },
	{ LPGENW("Contact"), 1, "Search1width", 100 },
	{ LPGENW("Module"),  2, "Search2width", 100 },
	{ LPGENW("Setting"), 3, "Search3width", 100 },
	{ LPGENW("Value"),   4, "Search4width", 150 },
	{ nullptr }
};

void __cdecl FindSettings(LPVOID di);

int FindDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_LIST:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	case IDC_SBAR:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
	default:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}
}

INT_PTR CALLBACK FindWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndResults = GetDlgItem(hwnd, IDC_LIST);

	switch (msg) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDC_SBAR, SB_SETTEXT, 0, (LPARAM)TranslateT("Enter a string to search the database for"));
		CheckDlgButton(hwnd, IDC_MODNAME, BST_CHECKED);
		CheckDlgButton(hwnd, IDC_SETTINGNAME, BST_CHECKED);
		CheckDlgButton(hwnd, IDC_SETTINGVALUE, BST_CHECKED);
		CheckDlgButton(hwnd, IDC_FOUND, BST_CHECKED);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(ICO_REGEDIT)));
		SetWindowLongPtr(GetDlgItem(hwnd, IDC_REPLACE), GWLP_USERDATA, 0);
		SetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA, 0);
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_APPWINDOW); // taskbar icon
		TranslateDialogDefault(hwnd);
		ListView_SetExtendedListViewStyle(hwndResults, 32 | LVS_EX_LABELTIP); // LVS_EX_GRIDLINES
		loadListSettings(hwndResults, csResultList);
		Utils_RestoreWindowPositionNoMove(hwnd, NULL, MODULENAME, "Search_");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDC_SEARCH:
			if (GetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA)) // stop the search
				SetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA, 0);
			else {
				wchar_t text[FLD_SIZE];
				wchar_t replace[FLD_SIZE] = {};

				if (!GetDlgItemText(hwnd, IDC_TEXT, text, _countof(text)) && !IsDlgButtonChecked(hwnd, IDC_EXACT)) break;

				// empty replace is done only for exact match or entire replace
				if (LOWORD(wParam) == IDOK &&
					!GetDlgItemText(hwnd, IDC_REPLACE, replace, _countof(replace)) &&
					(!IsDlgButtonChecked(hwnd, IDC_ENTIRELY) && !IsDlgButtonChecked(hwnd, IDC_EXACT)))
					break;

				if (BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_MODNAME) &&
					BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_SETTINGNAME) &&
					BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_SETTINGVALUE))
					break;

				FindInfo *fi = (FindInfo *)mir_calloc(sizeof(FindInfo));
				if (!fi)
					break;

				fi->hwnd = hwndResults;
				fi->options = (IsDlgButtonChecked(hwnd, IDC_CASESENSITIVE) ? F_CASE : 0) |
					(IsDlgButtonChecked(hwnd, IDC_EXACT) ? F_EXACT : 0) |
					(IsDlgButtonChecked(hwnd, IDC_MODNAME) ? F_MODNAME : 0) |
					(IsDlgButtonChecked(hwnd, IDC_SETTINGNAME) ? F_SETNAME : 0) |
					(IsDlgButtonChecked(hwnd, IDC_SETTINGVALUE) ? F_SETVAL : 0);

				if (LOWORD(wParam) == IDOK) {

					if (IsDlgButtonChecked(hwnd, IDC_ENTIRELY))
						fi->options |= F_ENTIRE;

					fi->replace = mir_wstrdup(replace);

					SetDlgItemText(hwnd, IDOK, TranslateT("Stop"));
					EnableWindow(GetDlgItem(hwnd, IDC_SEARCH), 0);

				}
				else {
					SetDlgItemText(hwnd, IDC_SEARCH, TranslateT("Stop"));
					EnableWindow(GetDlgItem(hwnd, IDOK), 0);
				}

				fi->search = mir_wstrdup(text);

				ListView_DeleteAllItems(fi->hwnd);
				SetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA, 1);

				EnableWindow(GetDlgItem(hwnd, IDCANCEL), 0);
				mir_forkthread(FindSettings, fi);
			}
			break;

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			mmi->ptMinTrackSize.x = 610;
			mmi->ptMinTrackSize.y = 300;
		}
		return 0;

	case WM_SIZE:
		Utils_ResizeDialog(hwnd, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_FIND), FindDialogResize);
		break;

	case WM_NOTIFY:
		if (LOWORD(wParam) != IDC_LIST) break;
		switch (((NMHDR *)lParam)->code) {
		case NM_DBLCLK:
			LVHITTESTINFO hti;
			LVITEM lvi;
			hti.pt = ((NMLISTVIEW *)lParam)->ptAction;
			if (ListView_SubItemHitTest(hwndResults, &hti) > -1) {
				if (hti.flags & LVHT_ONITEM) {
					lvi.mask = LVIF_PARAM;
					lvi.iItem = hti.iItem;
					lvi.iSubItem = 0;
					if (ListView_GetItem(hwndResults, &lvi)) {
						ItemInfo ii = {};
						ii.hContact = (MCONTACT)lvi.lParam;
						ListView_GetItemTextA(hwndResults, hti.iItem, 2, ii.module, _countof(ii.module));
						ListView_GetItemTextA(hwndResults, hti.iItem, 3, ii.setting, _countof(ii.setting));
						if (ii.setting[0])
							ii.type = FW_SETTINGNAME;
						else if (ii.module[0])
							ii.type = FW_MODULE;

						SendMessage(hwnd2mainWindow, WM_FINDITEM, (WPARAM)&ii, 0);
					}
				}
			}
			break;

		case LVN_COLUMNCLICK:
			LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
			ColumnsSortParams params;
			params.hList = hwndResults;
			params.column = lv->iSubItem;
			params.last = lastColumn;
			ListView_SortItemsEx(params.hList, ColumnsCompare, (LPARAM)&params);
			lastColumn = (params.column == lastColumn) ? -1 : params.column;
			break;
		} // switch
		break;

	case WM_DESTROY:
		ListView_DeleteAllItems(hwndResults);
		saveListSettings(hwndResults, csResultList);
		Utils_SaveWindowPosition(hwnd, NULL, MODULENAME, "Search_");
		break;
	}
	return 0;
}


void newFindWindow()
{
	CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FIND), hwnd2mainWindow, FindWindowDlgProc);
}

void ItemFound(HWND hwnd, MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int type)
{
	wchar_t name[NAME_SIZE];
	wchar_t *mode;

	if (type & F_REPLACED)
		mode = TranslateT("Replaced");
	else if (type & F_DELETED)
		mode = TranslateT("Deleted");
	else
		mode = TranslateT("Found");

	GetContactName(hContact, nullptr, name, _countof(name));

	LVITEM lvi = {};
	lvi.mask = LVIF_PARAM;
	lvi.lParam = (LPARAM)hContact;

	int index = ListView_InsertItem(hwnd, &lvi);
	ListView_SetItemText(hwnd, index, 0, mode);
	ListView_SetItemText(hwnd, index, 1, name);

	ListView_SetItemTextA(hwnd, index, 2, module);

	int F_type = type & 0xFF;

	if (F_type == F_SETNAME || F_type == F_SETVAL)
		ListView_SetItemTextA(hwnd, index, 3, setting);

	if (F_type == F_SETVAL && value)
		ListView_SetItemText(hwnd, index, 4, value);
}


char *multiReplaceA(const char *value, const char *search, const char *replace, int cs)
{
	int slen = (int)mir_strlen(search);
	int rlen = (int)mir_strlen(replace);
	int vlen = (int)mir_strlen(value);
	int ci = slen ? cs : 1; // on empty string strstr() returns full string while StrStrI() returns NULL 
	// let's try to calculate maximum length for result string
	int newlen = (!slen) ? rlen + 1 : ((rlen <= slen) ? vlen + 1 : vlen * rlen / slen + 1);

	char *head;
	char *in = (char *)value;
	char *out = (char *)mir_alloc(newlen * sizeof(char));
	out[0] = 0;

	while (head = ci ? strstr(in, search) : StrStrIA(in, search)) {
		if (head != in)
			mir_strncat(out, in, head - in + 1);
		in = head + slen;
		mir_strcat(out, replace);
	}

	mir_strcat(out, in);
	return out;
}

WCHAR *multiReplaceW(const WCHAR *value, const WCHAR *search, const WCHAR *replace, int cs)
{
	int slen = (int)mir_wstrlen(search);
	int rlen = (int)mir_wstrlen(replace);
	int vlen = (int)mir_wstrlen(value);
	int ci = slen ? cs : 1; // on empty string strstr() returns full string while StrStrI() returns NULL 
	// let's try to calculate maximum length for result string
	int newlen = (!slen) ? rlen + 1 : ((rlen <= slen) ? vlen + 1 : vlen * rlen / slen + 1);

	WCHAR *head;
	WCHAR *in = (WCHAR *)value;
	WCHAR *out = (WCHAR *)mir_alloc(newlen * sizeof(WCHAR));
	out[0] = 0;

	while (head = ci ? wcsstr(in, search) : StrStrIW(in, search)) {
		if (head != in)
			mir_wstrncat(out, in, head - in + 1);
		in = head + slen;
		mir_wstrcat(out, replace);
	}

	mir_wstrcat(out, in);
	return out;
}


int FindMatchA(const char *text, char *search, int options)
{
	if (!search[0] && (!(options & F_EXACT) || !text[0]))
		return 1;

	if (options & F_EXACT)
		return (options & F_CASE) ? !strcmp(text, search) : !stricmp(text, search);

	// on empty string strstr() returns full string while StrStrI() returns NULL 	
	return (options & F_CASE) ? (INT_PTR)strstr(text, search) : (INT_PTR)StrStrIA(text, search);
}


int FindMatchW(const WCHAR *text, WCHAR *search, int options)
{
	if (!search[0] && (!(options & F_EXACT) || !text[0]))
		return 1;

	if (options & F_EXACT)
		return (options & F_CASE) ? !wcscmp(text, search) : !wcsicmp(text, search);

	// on empty string strstr() returns full string while StrStrI() returns NULL 	
	return (options & F_CASE) ? (INT_PTR)wcsstr(text, search) : (INT_PTR)StrStrIW(text, search);
}


void fi_free(FindInfo *fi)
{
	mir_free(fi->search);
	mir_free(fi->replace);
	mir_free(fi);
}


void __cdecl FindSettings(LPVOID param)
{
	FindInfo *fi = (FindInfo *)param;
	HWND hwndParent = GetParent(fi->hwnd);

	ModuleSettingLL ModuleList, SettingList;
	ModSetLinkLinkItem *module, *setting;

	MCONTACT hContact;
	DBVARIANT dbv = {};

	int foundCount = 0, replaceCount = 0, deleteCount = 0;

	DWORD numsearch = 0, numreplace = 0;
	int NULLContactDone = 0;

	if (!fi->search || !EnumModules(&ModuleList)) {
		fi_free(fi);
		return;
	}

	_T2A search(fi->search);
	_T2A replace(fi->replace);

	// skip modules and setting names on unicode search or replace
	if (IsRealUnicode(fi->search) || IsRealUnicode(fi->replace)) {
		fi->options &= ~(F_SETNAME | F_MODNAME);
		fi->options |= F_UNICODE;
	}

	if (!(fi->options & F_UNICODE) && (fi->options & F_SETVAL)) {
		char val[16];
		numsearch = strtoul(search, nullptr, 10);
		_ultoa(numsearch, val, 10);
		if (!mir_strcmp(search, val)) {
			fi->options |= F_NUMSRCH;
			// replace numeric values only entirely
			if (replace && (fi->options & F_ENTIRE)) {
				numreplace = strtoul(replace, nullptr, 10);
				_ultoa(numreplace, val, 10);
				if (!replace[0] || !mir_strcmp(replace, val))
					fi->options |= F_NUMREPL;
			}
		}
	}

	SendDlgItemMessage(hwndParent, IDC_SBAR, SB_SETTEXT, 0, (LPARAM)TranslateT("Searching..."));

	hContact = 0;

	while (GetWindowLongPtr(GetDlgItem(hwndParent, IDC_SEARCH), GWLP_USERDATA)) {

		if (!hContact) {
			if (NULLContactDone)
				break;
			else {
				NULLContactDone = 1;
				hContact = db_find_first();
			}
		}
		else hContact = db_find_next(hContact);

		for (module = ModuleList.first; module; module = module->next) {

			if (IsModuleEmpty(hContact, module->name))
				continue;

			if (fi->options & (F_SETVAL | F_SETNAME)) {

				if (!EnumSettings(hContact, module->name, &SettingList)) {
					fi_free(fi);
					FreeModuleSettingLL(&ModuleList);
					return;
				}

				for (setting = SettingList.first; setting; setting = setting->next) {

					dbv.type = 0;
					if (db_get_s(hContact, module->name, setting->name, &dbv, 0))
						continue;

					// check in settings value				
					if (fi->options & F_SETVAL) {

						wchar_t *value = nullptr;

						switch (dbv.type) {

						case DBVT_BYTE:
						case DBVT_WORD:
						case DBVT_DWORD:
							if ((fi->options & F_NUMSRCH) && numsearch == getNumericValue(&dbv)) {
								wchar_t *val = fi->search;
								int flag = F_SETVAL;

								if (fi->options & F_NUMREPL) {
									if (replace[0]) {
										db_unset(hContact, module->name, setting->name);
										flag |= F_DELETED;
										deleteCount++;
									}
									else
										if (setNumericValue(hContact, module->name, setting->name, numreplace, dbv.type)) {
											val = fi->replace;
											flag |= F_REPLACED;
											replaceCount++;
										}
								}

								ItemFound(fi->hwnd, hContact, module->name, setting->name, val, flag);
							}
							break;

						case DBVT_WCHAR:
							if (!value) value = mir_wstrdup(dbv.pwszVal);
						case DBVT_UTF8:
							if (!value) value = mir_utf8decodeW(dbv.pszVal);
						case DBVT_ASCIIZ:
							if (!value) value = mir_a2u(dbv.pszVal);

							if (FindMatchW(value, fi->search, fi->options)) {
								foundCount++;
								ptrW ptr;
								wchar_t *newValue = value;
								int flag = F_SETVAL;

								if (fi->replace) {
									newValue = (fi->options & F_ENTIRE) ? fi->replace : ptr = multiReplaceW(value, fi->search, fi->replace, fi->options & F_CASE);
									// !!!! delete or make empty ?
									if (!newValue[0]) {
										db_unset(hContact, module->name, setting->name);
										flag |= F_DELETED;
										newValue = value;
										deleteCount++;
									}
									else {
										// save as unicode if needed
										if (dbv.type != DBVT_ASCIIZ || IsRealUnicode(newValue))
											db_set_ws(hContact, module->name, setting->name, newValue);
										else
											db_set_s(hContact, module->name, setting->name, _T2A(newValue));
										flag |= F_REPLACED;
										replaceCount++;
									}
								}

								ItemFound(fi->hwnd, hContact, module->name, setting->name, newValue, flag);
							}
							mir_free(value);
							break;
						} // switch
					}

					// check in setting name
					if ((fi->options & F_SETNAME) && FindMatchA(setting->name, search, fi->options)) {
						foundCount++;
						ptrA ptr;
						char *newSetting = setting->name;
						int flag = F_SETNAME;

						if (replace) {
							newSetting = (fi->options & F_ENTIRE) ? replace : ptr = multiReplaceA(setting->name, search, replace, fi->options & F_CASE);

							if (!newSetting[0]) {
								db_unset(hContact, module->name, setting->name);
								flag |= F_DELETED;
								newSetting = setting->name;
								deleteCount++;
							}
							else {
								DBVARIANT dbv2;
								// skip if exist
								if (!db_get_s(hContact, module->name, newSetting, &dbv2, 0))
									db_free(&dbv2);
								else if (!db_set(hContact, module->name, newSetting, &dbv)) {
									db_unset(hContact, module->name, setting->name);
									flag |= F_REPLACED;
									replaceCount++;
								}
							}
						}

						ItemFound(fi->hwnd, hContact, module->name, newSetting, nullptr, flag);
					}

					db_free(&dbv);

				} // for(setting)

				FreeModuleSettingLL(&SettingList);
			}

			// check in module name
			if ((fi->options & F_MODNAME) && FindMatchA(module->name, search, fi->options)) {
				foundCount++;
				char *newModule = module->name;
				int flag = F_MODNAME;
				ptrA ptr;

				if (replace) {
					newModule = (fi->options & F_ENTIRE) ? replace : ptr = multiReplaceA(module->name, search, replace, fi->options & F_CASE);

					if (!newModule[0]) {
						deleteModule(hContact, module->name, 0);
						replaceTreeItem(hContact, module->name, nullptr);
						flag |= F_DELETED;
						newModule = module->name;
						deleteCount++;
					}
					else if (renameModule(hContact, module->name, newModule)) {
						replaceTreeItem(hContact, module->name, nullptr);
						flag |= F_REPLACED;
						replaceCount++;
					}
				}

				ItemFound(fi->hwnd, hContact, newModule, nullptr, nullptr, flag);
			}
		} // for(module)
	}

	wchar_t msg[MSG_SIZE];
	mir_snwprintf(msg, TranslateT("Finished. Items found: %d / replaced: %d / deleted: %d"), foundCount, replaceCount, deleteCount);
	SendDlgItemMessage(hwndParent, IDC_SBAR, SB_SETTEXT, 0, (LPARAM)msg);

	if (fi->replace) {
		EnableWindow(GetDlgItem(hwndParent, IDC_SEARCH), 1);
		SetDlgItemText(hwndParent, IDOK, TranslateT("&Replace"));
	}
	else {
		SetDlgItemText(hwndParent, IDC_SEARCH, TranslateT("&Search"));
		EnableWindow(GetDlgItem(hwndParent, IDOK), 1);
	}

	fi_free(fi);
	FreeModuleSettingLL(&ModuleList);

	SetWindowLongPtr(GetDlgItem(hwndParent, IDC_SEARCH), GWLP_USERDATA, 0);
	EnableWindow(GetDlgItem(hwndParent, IDCANCEL), 1);
}
