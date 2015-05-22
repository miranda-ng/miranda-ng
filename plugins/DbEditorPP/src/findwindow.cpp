#include "headers.h"

void __cdecl FindSettings(LPVOID di);

static int working;
static int replace;

#define FW_CASE    1
#define FW_EXACT   2
#define FW_MODNAME 4
#define FW_SETNAME 8
#define FW_SETVAL 16

#define FW_REPLACED 0x100
#define FW_DELETED  0x200

#define RW_MODULE  1
#define RW_SETNAME 2
#define RW_SETVAL  4
#define RW_FOUND   8

#define RW_FULL   0x100
#define RW_CASE   0x200

typedef struct {
	HWND hwnd; // hwnd 2 item list
	char* text; // text to find
	int options; // or'd about items
	char* replace; // text to replace
	int mode; // replace mode
} FindInfo;


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

void freeItems(HWND hwnd)
{
	for (int i = 0; i < SendMessage(hwnd, LB_GETCOUNT, 0, 0); i++) {
		ItemInfo *ii = (ItemInfo*)SendMessage(hwnd, LB_GETITEMDATA, i, 0);
		if ((LRESULT)ii != LB_ERR)
			mir_free(ii);
	}
}

INT_PTR CALLBACK FindWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		SendDlgItemMessage(hwnd, IDC_SBAR, SB_SETTEXT, 0, (LPARAM)Translate("Enter a string to search the database for"));
		CheckDlgButton(hwnd, IDC_MODNAME, BST_CHECKED);
		CheckDlgButton(hwnd, IDC_SETTINGNAME, BST_CHECKED);
		CheckDlgButton(hwnd, IDC_SETTINGVALUE, BST_CHECKED);
		CheckDlgButton(hwnd, IDC_FOUND, BST_CHECKED);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(ICO_REGEDIT)));
		SetWindowLongPtr(GetDlgItem(hwnd, IDC_REPLACE), GWLP_USERDATA, 0);
		SetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_REPLACE), GWLP_USERDATA, 1);

		case IDC_SEARCH:
			if (GetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA)) // stop the search
				SetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA, 0);
			else {
				char text[256];
				char replace[256] = "";

				if (!GetDlgItemText(hwnd, IDC_TEXT, text, SIZEOF(text))) break;

				if (GetWindowLongPtr(GetDlgItem(hwnd, IDC_REPLACE), GWLP_USERDATA) &&
					!GetDlgItemText(hwnd, IDC_REPLACE, replace, SIZEOF(replace)) &&
					BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_ENTIRELY))
					break;

				if (BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_MODNAME) &&
					BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_SETTINGNAME) &&
					BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_SETTINGVALUE))
					break;

				FindInfo *fi = (FindInfo*)mir_calloc(sizeof(FindInfo));
				if (!fi)
					break;

				fi->hwnd = GetDlgItem(hwnd, IDC_LIST);
				fi->options = (IsDlgButtonChecked(hwnd, IDC_CASESENSITIVE) ? FW_CASE : 0) |
					(IsDlgButtonChecked(hwnd, IDC_EXACT) ? FW_EXACT : 0) |
					(IsDlgButtonChecked(hwnd, IDC_MODNAME) ? FW_MODNAME : 0) |
					(IsDlgButtonChecked(hwnd, IDC_SETTINGNAME) ? FW_SETNAME : 0) |
					(IsDlgButtonChecked(hwnd, IDC_SETTINGVALUE) ? FW_SETVAL : 0);

				if (GetWindowLongPtr(GetDlgItem(hwnd, IDC_REPLACE), GWLP_USERDATA)) {
					if (IsDlgButtonChecked(hwnd, IDC_FOUND))
						fi->mode = RW_FOUND;
					else if (IsDlgButtonChecked(hwnd, IDC_MODNAME2))
						fi->mode = RW_MODULE;
					else if (IsDlgButtonChecked(hwnd, IDC_SETTINGNAME2))
						fi->mode = RW_SETNAME;
					else if (IsDlgButtonChecked(hwnd, IDC_SETTINGVALUE2))
						fi->mode = RW_SETVAL;

					if (IsDlgButtonChecked(hwnd, IDC_ENTIRELY))
						fi->mode |= RW_FULL;

					fi->replace = mir_tstrdup(replace);

					SetDlgItemText(hwnd, IDOK, Translate("Stop"));
					EnableWindow(GetDlgItem(hwnd, IDC_SEARCH), 0);

					if (IsDlgButtonChecked(hwnd, IDC_CASESENSITIVE))
						fi->mode |= RW_CASE;
				}
				else {
					SetDlgItemText(hwnd, IDC_SEARCH, Translate("Stop"));
					EnableWindow(GetDlgItem(hwnd, IDOK), 0);
				}

				fi->text = mir_tstrdup(text);

				SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
				SetWindowLongPtr(GetDlgItem(hwnd, IDC_SEARCH), GWLP_USERDATA, 1);

				EnableWindow(GetDlgItem(hwnd, IDCANCEL), 0);
				forkthread(FindSettings, 0, fi);
			}
			break;

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		case IDC_LIST:
			if (HIWORD(wParam) == LBN_DBLCLK) {
				int i = SendDlgItemMessage(hwnd, IDC_LIST, LB_GETCURSEL, 0, 0);
				ItemInfo *ii = (ItemInfo*)SendDlgItemMessage(hwnd, IDC_LIST, LB_GETITEMDATA, i, 0);
				if (!ii) break;
				SendMessage(GetParent(hwnd), WM_FINDITEM, (WPARAM)ii, 0);
			}
			break;
		}
		break;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *mmi = (MINMAXINFO*)lParam;
		mmi->ptMinTrackSize.x = 520;
		mmi->ptMinTrackSize.y = 300;
	}
	return 0;
	case WM_SIZE:
	{
		UTILRESIZEDIALOG urd;
		memset(&urd, 0, sizeof(urd));
		urd.cbSize = sizeof(urd);
		urd.hInstance = hInst;
		urd.hwndDlg = hwnd;
		urd.lpTemplate = MAKEINTRESOURCE(IDD_FIND);
		urd.pfnResizer = FindDialogResize;
		CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
	}
	break;

	case WM_DESTROY:
		freeItems(hwnd);
		break;
	}
	return 0;
}

void ItemFound(HWND hwnd, MCONTACT hContact, const char *module, const char *setting, const char* value, int type)
{
	ItemInfo *ii = (ItemInfo*)mir_calloc(sizeof(ItemInfo));
	if (!ii) return;

	int index;
	char text[256] = "";
	char szValue[256];
	char *name, *mode;

	if (type & FW_REPLACED)
		mode = Translate("Replaced with");
	else
		if (type & FW_DELETED)
			mode = Translate("Deleted");
		else
			mode = Translate("Found");

	name = hContact ? (char*)GetContactName(hContact, NULL, 0) : Translate("Settings");

	switch (type & 0xFF) {
	case FW_MODULE:
		ii->type = FW_MODULE;
		mir_snprintf(text, SIZEOF(text), Translate("%s Module \"%s\" in contact \"%s\""), mode, module, name);
		break;
	case FW_SETTINGNAME:
		mir_strncpy(ii->setting, setting, SIZEOF(ii->setting));
		ii->type = FW_SETTINGNAME;
		if (GetValue(hContact, module, setting, szValue, SIZEOF(szValue)))
			mir_snprintf(text, SIZEOF(text), Translate("%s Setting \"%s\" in module \"%s\" in contact \"%s\" - \"%s\""), mode, setting, module, name, szValue);
		else
			mir_snprintf(text, SIZEOF(text), Translate("%s Setting \"%s\" in module \"%s\" in contact \"%s\""), mode, setting, module, name);
		break;
	case FW_SETTINGVALUE:
		mir_strncpy(ii->setting, setting, SIZEOF(ii->setting));
		ii->type = FW_SETTINGVALUE;
		mir_snprintf(text, SIZEOF(text), Translate("%s \"%s\" in Setting \"%s\" in module \"%s\" in contact \"%s\""), mode, value, setting, module, name);
		break;
	}

	index = SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)text);
	if (type & FW_DELETED) {
		SendMessage(hwnd, LB_SETITEMDATA, index, 0);
		mir_free(ii);
	}
	else {
		ii->hContact = hContact;
		mir_strncpy(ii->module, module, SIZEOF(ii->module));
		SendMessage(hwnd, LB_SETITEMDATA, index, (LPARAM)ii);
	}
}

char* multiReplace(const char* value, const char *find, const char *replace, int cs)
{
	char *head, *temp, *string;

	int len = (int)mir_strlen(find);
	int replen = (int)mir_strlen(replace);

	// only should be 1 '=' sign there...
	if (head = (char*)(cs ? strstr(value, find) : StrStrI(value, find))) {
		string = (char*)value;
		temp = (char*)mir_alloc(1 * sizeof(char));
		temp[0] = '\0';

		while (head) {
			temp = (char*)mir_realloc(temp, mir_strlen(temp) + mir_strlen(string) + replen + 1);
			if (!temp) mir_tstrdup(value);

			strncat(temp, string, (head - string));
			string = head + len;
			strcat(temp, replace);

			head = (cs ? strstr(string, find) : StrStrI(string, find));
		}
		strcat(temp, string);

		return temp;
	}

	return mir_tstrdup(value);
}

int replaceValue(HWND hwnd, MCONTACT hContact, const char *module, const char *setting, DBVARIANT *dbv, const char *find, const char *replace, int mode)
{
	int count = 0;
	DWORD num = 0;
	int isNumeric;
	char *myreplace = NULL;
	DBVARIANT val = { 0 };

	if (!dbv->type || dbv->type == DBVT_BLOB)
		return 0;

	if (!replace[0])
		isNumeric = 1;
	else
		isNumeric = sscanf(replace, "%d", &num);

	val.type = dbv->type;

	switch (dbv->type) {
	case DBVT_UTF8:
	case DBVT_ASCIIZ:
		if (mode & RW_FULL)
			val.pszVal = (char*)replace;
		else {
			myreplace = multiReplace(dbv->pszVal, find, replace, mode & RW_CASE);
			val.pszVal = myreplace;
		}
		break;

	case DBVT_BYTE:
		if (isNumeric && num < 0x100)
			val.bVal = (BYTE)num;
		else
			return 0;
		break;

	case DBVT_WORD:
		if (isNumeric && num < 0x10000)
			val.wVal = (WORD)num;
		else
			return 0;
		break;

	case DBVT_DWORD:
		if (isNumeric)
			val.dVal = num;
		else
			return 0;
		break;

	default:
		return 0;
	}

	if ((!val.pszVal && !replace[0]) || (val.pszVal && !val.pszVal[0])) {
		ItemFound(hwnd, hContact, module, setting, NULL, FW_SETTINGNAME | FW_DELETED);
		db_unset(hContact, module, setting);
		mir_free(myreplace);
		return 1;
	}

	if (!db_set(hContact, module, setting, &val)) {
		count++;
		ItemFound(hwnd, hContact, module, setting, myreplace ? myreplace : (char*)replace, FW_SETTINGVALUE | FW_REPLACED);
	}

	mir_free(myreplace);

	return count;
}

int replaceSetting(HWND hwnd, MCONTACT hContact, const char *module, const char *setting, DBVARIANT *dbv, const char *find, const char *replace, int mode)
{
	char *szSetting;
	ptrA myreplace;
	int count = 0;
	DBVARIANT dbv2;

	if (!dbv->type)	return 0;

	if (mode & RW_FULL)
		szSetting = (char*)replace;
	else {
		myreplace = multiReplace(setting, find, replace, mode & RW_CASE);
		szSetting = myreplace;
	}

	if (szSetting[0] == 0) {
		ItemFound(hwnd, hContact, module, setting, NULL, FW_SETTINGNAME | FW_DELETED);
		db_unset(hContact, module, setting);
		return 1;
	}

	// check & write
	if (GetSetting(hContact, module, myreplace, &dbv2)) {
		if (!db_set(hContact, module, szSetting, &dbv2)) {
			count++;
			db_unset(hContact, module, setting);
			ItemFound(hwnd, hContact, module, szSetting, NULL, FW_SETTINGNAME | FW_REPLACED);
		}
	}
	else db_free(&dbv2);

	mir_free(myreplace);

	return count;
}


int replaceModule(HWND hwnd, MCONTACT hContact, const char *module, const char *find, const char *replace, int mode)
{
	ModuleSettingLL msll;
	ModSetLinkLinkItem *setting;
	char *myreplace = NULL;
	char *newModule;
	int count = 0;

	if (mode & RW_FULL)
		newModule = (char*)replace;
	else {
		myreplace = multiReplace(module, find, replace, mode & RW_CASE);
		newModule = myreplace;
	}

	if (newModule[0] == 0) {
		ItemFound(hwnd, hContact, module, NULL, NULL, FW_MODULE | FW_DELETED);
		deleteModule((char*)module, hContact, 1);
		replaceTreeItem(GetDlgItem(hwnd2mainWindow, IDC_MODULES), hContact, module, NULL);
		mir_free(myreplace);
		return 1;
	}

	if (!IsModuleEmpty(hContact, newModule))
		return 0;

	if (EnumSettings(hContact, (char*)module, &msll)) {
		setting = msll.first;

		while (setting) {
			DBVARIANT dbv;

			if (!GetSetting(hContact, module, setting->name, &dbv)) {
				switch (dbv.type) {
				case DBVT_BYTE:
					db_set_b(hContact, newModule, setting->name, dbv.bVal);
					break;
				case DBVT_WORD:
					db_set_w(hContact, newModule, setting->name, dbv.wVal);
					break;
				case DBVT_DWORD:
					db_set_dw(hContact, newModule, setting->name, dbv.dVal);
					break;
				case DBVT_ASCIIZ:
					db_set_s(hContact, newModule, setting->name, dbv.pszVal);
					break;
				case DBVT_UTF8:
					db_set_utf(hContact, newModule, setting->name, dbv.pszVal);
					break;
				case DBVT_BLOB:
					db_set_blob(hContact, newModule, setting->name, dbv.pbVal, dbv.cpbVal);
					break;
				}

				db_free(&dbv);
				db_unset(hContact, module, setting->name);
			}

			setting = setting->next;
		}
		FreeModuleSettingLL(&msll);

		replaceTreeItem(GetDlgItem(hwnd2mainWindow, IDC_MODULES), hContact, module, newModule);

		ItemFound(hwnd, hContact, newModule, NULL, NULL, FW_MODULE | FW_REPLACED);
		count++;
	}

	mir_free(myreplace);
	return count;
}

char* stringToUpper(char* in, char* out, int maxlen)
{
	int len;
	if (maxlen > 0)
		len = maxlen - 1;
	else
		len = 0x10000;

	int i;
	for (i = 0; in[i] && i < len; i++)
		out[i] = (in[i] >= 'a' && in[i] <= 'z') ? toupper(in[i]) : in[i];
	out[i] = '\0';
	return out;
}


void __cdecl FindSettings(LPVOID di)
{
	char* text = ((FindInfo*)di)->text;
	char* replace = ((FindInfo*)di)->replace;
	int mode = ((FindInfo*)di)->mode;
	HWND hwnd = ((FindInfo*)di)->hwnd;
	HWND prnthwnd = GetParent(hwnd);
	int options = ((FindInfo*)di)->options;
	ModuleSettingLL ModuleList, SettingList;
	ModSetLinkLinkItem *module, *setting;
	MCONTACT hContact;
	DBVARIANT dbv = { 0 };
	int caseSensitive = options & FW_CASE;
	int exactMatch = options & FW_EXACT;
	int inModuleName = options & FW_MODNAME;
	int inSettingName = options & FW_SETNAME;
	int inSettingValue = options & FW_SETVAL;
	int foundCount = 0;
	int replaceCount = 0;
	char szTmp[128];
	int settingValue, isNumber, NULLContactDone = 0;

	freeItems(hwnd);
	if (!text)
		return;

	if (!EnumModules(&ModuleList)) {
		msg(Translate("Error loading module list"), modFullname);
		mir_free(di);
		return;
	}

	SendDlgItemMessage(GetParent(hwnd), IDC_SBAR, SB_SETTEXT, 0, (LPARAM)Translate("Searching..."));

	hContact = 0;

	isNumber = sscanf(text, "%d", &settingValue);

	while (GetWindowLongPtr(GetDlgItem(prnthwnd, IDC_SEARCH), GWLP_USERDATA)) {
		if (!hContact) {
			if (NULLContactDone) break;
			else {
				NULLContactDone = 1;
				hContact = db_find_first();
			}
		}
		else hContact = db_find_next(hContact);

		module = ModuleList.first;
		while (module) {
			if (IsModuleEmpty(hContact, module->name)) {
				module = module->next;
				continue;
			}

			if (!EnumSettings(hContact, module->name, &SettingList)) {
				msg(Translate("Error Loading Setting List"), modFullname);
				mir_free(text);
				mir_free(di);
				FreeModuleSettingLL(&ModuleList);
				return;
			}
			setting = SettingList.first;

			// check in settings value
			while (setting) {
				if (inSettingValue) {
					dbv.type = 0;
					// check the setting value
					if (!GetSetting(hContact, module->name, setting->name, &dbv)) {
						switch (dbv.type) {
						case DBVT_UTF8: // no conversion atm
						case DBVT_ASCIIZ:
							if ((exactMatch && !(caseSensitive ? mir_strcmp(dbv.pszVal, text) : strcmpi(dbv.pszVal, text))) || (!exactMatch && (caseSensitive ? strstr(dbv.pszVal, text) : StrStrI(dbv.pszVal, text)))) {
								if ((mode & RW_FOUND) || (mode & RW_SETVAL))
									replaceCount += replaceValue(hwnd, hContact, module->name, setting->name, &dbv, text, replace, mode);
								else
									ItemFound(hwnd, hContact, module->name, setting->name, dbv.pszVal, FW_SETTINGVALUE);

								foundCount++;
							}
							break;

						case DBVT_BYTE:
							if (isNumber && settingValue == dbv.bVal) {
								if ((mode & RW_FOUND) || (mode & RW_SETVAL))
									replaceCount += replaceValue(hwnd, hContact, module->name, setting->name, &dbv, NULL, replace, mode);
								else
									ItemFound(hwnd, hContact, module->name, setting->name, text, FW_SETTINGVALUE);
								foundCount++;
							}
							break;

						case DBVT_WORD:
							if (isNumber && settingValue == dbv.wVal) {
								if ((mode & RW_FOUND) || (mode & RW_SETVAL))
									replaceCount += replaceValue(hwnd, hContact, module->name, setting->name, &dbv, NULL, replace, mode);
								else
									ItemFound(hwnd, hContact, module->name, setting->name, text, FW_SETTINGVALUE);
								foundCount++;
							}
							break;

						case DBVT_DWORD:
							if (isNumber && settingValue == (int)dbv.dVal) {
								if ((mode & RW_FOUND) || (mode & RW_SETVAL))
									replaceCount += replaceValue(hwnd, hContact, module->name, setting->name, &dbv, NULL, replace, mode);
								else
									ItemFound(hwnd, hContact, module->name, setting->name, text, FW_SETTINGVALUE);
								foundCount++;
							}
							break;

						}
						db_free(&dbv);
					}
				}

				// check in setting name
				if (inSettingName) {
					if ((exactMatch && !(caseSensitive ? mir_strcmp(setting->name, text) : strcmpi(setting->name, text))) || (!exactMatch && (caseSensitive ? StrStrI(setting->name, text) : StrStrI(setting->name, text)))) {
						if ((mode & RW_FOUND) || (mode & RW_SETNAME)) {
							if (!GetSetting(hContact, module->name, setting->name, &dbv)) {
								replaceCount += replaceSetting(hwnd, hContact, module->name, setting->name, &dbv, text, replace, mode);
								db_free(&dbv);
							}
						}
						else
							ItemFound(hwnd, hContact, module->name, setting->name, NULL, FW_SETTINGNAME);
						foundCount++;
					}
				}

				setting = (ModSetLinkLinkItem *)setting->next;
			}

			// check in module name
			if (inModuleName) {
				if ((exactMatch && !(caseSensitive ? mir_strcmp(module->name, text) : strcmpi(module->name, text))) || (!exactMatch && (caseSensitive ? strstr(module->name, text) : StrStrI(module->name, text)))) {
					if ((mode & RW_FOUND) || (mode & RW_MODULE))
						replaceCount += replaceModule(hwnd, hContact, module->name, text, replace, mode);
					else
						ItemFound(hwnd, hContact, module->name, 0, 0, FW_MODULE);
					foundCount++;
				}
			}

			FreeModuleSettingLL(&SettingList);
			module = (ModSetLinkLinkItem *)module->next;
		}
	}

	if (mode) {
		if (!replace[0])
			mir_snprintf(szTmp, SIZEOF(szTmp), Translate("Finished. %d items were found, %d items were deleted."), foundCount, replaceCount);
		else
			mir_snprintf(szTmp, SIZEOF(szTmp), Translate("Finished. %d items were found, %d items were replaced."), foundCount, replaceCount);
	}
	else mir_snprintf(szTmp, SIZEOF(szTmp), Translate("Finished. %d items were found."), foundCount);

	SendDlgItemMessage(prnthwnd, IDC_SBAR, SB_SETTEXT, 0, (LPARAM)szTmp);

	SetWindowLongPtr(GetDlgItem(prnthwnd, IDC_SEARCH), GWLP_USERDATA, 0);

	if (GetWindowLongPtr(GetDlgItem(prnthwnd, IDC_REPLACE), GWLP_USERDATA)) {
		SetWindowLongPtr(GetDlgItem(prnthwnd, IDC_REPLACE), GWLP_USERDATA, 0);
		EnableWindow(GetDlgItem(prnthwnd, IDC_SEARCH), 1);
		SetDlgItemText(prnthwnd, IDOK, Translate("&Replace"));
	}
	else {
		SetDlgItemText(prnthwnd, IDC_SEARCH, Translate("&Search"));
		EnableWindow(GetDlgItem(prnthwnd, IDOK), 1);
	}

	mir_free(replace);
	mir_free(text);
	mir_free(di);
	FreeModuleSettingLL(&ModuleList);

	EnableWindow(GetDlgItem(prnthwnd, IDCANCEL), 1);
}
