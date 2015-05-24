#include "headers.h"

HINSTANCE hInst = NULL;

MIDatabase *g_db;
HANDLE hTTBButt = NULL;
BOOL bServiceMode = FALSE;
BOOL usePopups;
HWND hwnd2watchedVarsWindow;
int hLangpack;
BYTE nameOrder[NAMEORDERCOUNT];
HGENMENU hUserMenu;
WatchListArrayStruct WatchListArray;
MCONTACT hRestore;
IconItem iconList[];

//========================
//  MirandaPluginInfo
//========================

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A8A417EF-07AA-4F37-869F-7BFD74886534}
	{ 0xa8a417ef, 0x7aa, 0x4f37, { 0x86, 0x9f, 0x7b, 0xfd, 0x74, 0x88, 0x65, 0x34 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

// we implement service mode interface
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

//========================
//  WINAPI DllMain
//========================
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

void settingChanged(HWND hwnd2Settings, MCONTACT hContact, char *module, char *setting);

int DBSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	char *setting;
	SettingListInfo *info;

	if (hwnd2mainWindow) {
		HWND hwnd2Settings = GetDlgItem(hwnd2mainWindow, IDC_SETTINGS);
		if (info = (SettingListInfo *)GetWindowLongPtr(hwnd2Settings, GWLP_USERDATA)) {
			if ((hContact == info->hContact) && !mir_strcmp(info->module, cws->szModule)) {
				setting = mir_strdup(cws->szSetting);
				if (cws->value.type == DBVT_DELETED) {
					LVFINDINFO lvfi;
					int index;
					lvfi.flags = LVFI_STRING;
					lvfi.psz = setting;
					lvfi.vkDirection = VK_DOWN;
					index = ListView_FindItem(hwnd2Settings, -1, &lvfi);
					if (index > -1)
						ListView_DeleteItem(hwnd2Settings, index);
					mir_free(setting);
					return 0;
				}
				settingChanged(hwnd2Settings, hContact, info->module, setting);
				mir_free(setting);
			}
		}
	}

	// watch list
	if (!hwnd2watchedVarsWindow && !usePopups)
		return 0;

	for (int i = 0; i < WatchListArray.count; i++) {
		if (WatchListArray.item[i].module && (hContact == WatchListArray.item[i].hContact)) {
			if (!mir_strcmp(cws->szModule, WatchListArray.item[i].module)) {
				if (!WatchListArray.item[i].setting || !mir_strcmp(cws->szSetting, WatchListArray.item[i].setting)) {
					if (usePopups)
						popupWatchedVar(hContact, cws->szModule, cws->szSetting);
					if (hwnd2watchedVarsWindow)
						PopulateWatchedWindow(GetDlgItem(hwnd2watchedVarsWindow, IDC_VARS));
					break;
				}
			}
		}
	}
	return 0;
}

INT_PTR DBEditorppMenuCommand(WPARAM wParam, LPARAM)
{
	if (!hwnd2mainWindow) { // so only opens 1 at a time
		hRestore = wParam;
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, MainDlgProc);
	}
	else {
		ShowWindow(hwnd2mainWindow, SW_RESTORE);
		SetForegroundWindow(hwnd2mainWindow);
		if (!hRestore && wParam) {
			hRestore = wParam;
			refreshTree(4);
		}
	}

	if (hTTBButt)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTBButt, (LPARAM)0);

	return 0;
}

BOOL IsCP_UTF8(void)
{
	CPINFO CPInfo;
	return GetCPInfo(CP_UTF8, &CPInfo);
}

static int OnTTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = "DBEditorpp/MenuCommand";
	ttb.name = LPGEN("Database Editor++");
	ttb.hIconHandleUp = iconList[0].hIcolib;
	ttb.pszTooltipUp = LPGEN("Open Database Editor");
	hTTBButt = TopToolbar_AddButton(&ttb);
	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	addIcons();

	// Register menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 1900000001;
	mi.icolibItem = iconList[0].hIcolib;
	mi.pszPopupName = "Database";
	mi.pszName = modFullname;
	mi.pszService = "DBEditorpp/MenuCommand";
	Menu_AddMainMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 1900000001;
	mi.flags = 0;
	mi.icolibItem = iconList[7].hIcolib;
	mi.pszName = LPGEN("Open user tree in DBE++");
	mi.pszService = "DBEditorpp/MenuCommand";
	hUserMenu = Menu_AddContactMenuItem(&mi);

	// Register hotkeys
	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszName = "hk_dbepp_open";
	hkd.pszService = "DBEditorpp/MenuCommand";
	hkd.ptszDescription = LPGEN("Open Database Editor");
	hkd.ptszSection = modFullname;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_EXT, 'D');
	Hotkey_Register(&hkd);

	usePopups = db_get_b(NULL, modname, "UsePopUps", 0);

	// Load the name order
	for (int i = 0; i < NAMEORDERCOUNT; i++)
		nameOrder[i] = i;

	DBVARIANT dbv;
	if (!db_get(NULL, "Contact", "NameOrder", &dbv)) {
		memcpy(nameOrder, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}

	HookEvent(ME_TTB_MODULELOADED, OnTTBLoaded);

	if (bServiceMode)
		CallService("DBEditorpp/MenuCommand", 0, 0);

	return 0;
}

int PreShutdown(WPARAM, LPARAM)
{
	if (hwnd2watchedVarsWindow) DestroyWindow(hwnd2watchedVarsWindow);
	if (hwnd2mainWindow) DestroyWindow(hwnd2mainWindow);
	if (hwnd2importWindow) DestroyWindow(hwnd2importWindow);
	return 0;
}

INT_PTR ServiceMode(WPARAM, LPARAM)
{
	addIcons();
	bServiceMode = TRUE;

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);

	return SERVICE_ONLYDB;  // load database and then call us
}

INT_PTR ImportFromFile(WPARAM wParam, LPARAM lParam)
{
	ImportSettingsFromFileMenuItem(wParam, (char *)lParam);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	hwnd2mainWindow = hwnd2watchedVarsWindow = hwnd2importWindow = 0;
	hRestore = NULL;
	g_db = GetCurrentDatabase();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	CreateServiceFunction("DBEditorpp/MenuCommand", DBEditorppMenuCommand);
	CreateServiceFunction("DBEditorpp/Import", ImportFromFile);

	CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceMode);

	// Ensure that the common control DLL is loaded.
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	memset(&WatchListArray, 0, sizeof(WatchListArray));
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	freeAllWatches();
	return 0;
}

//=======================================================
// db_get_s (prob shouldnt use this unless u know how big the string is gonna be..)
//=======================================================

int DBGetContactSettingStringStatic(MCONTACT hContact, char *szModule, char *szSetting, char *value, int maxLength)
{
	DBVARIANT dbv;
	if (!db_get(hContact, szModule, szSetting, &dbv)) {
		strncpy(value, dbv.pszVal, maxLength);
		db_free(&dbv);
		return 1;
	}
	else {
		db_free(&dbv);
		return 0;
	}
}

int WriteBlobFromString(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szValue, int len)
{
	int j = 0, i = 0;
	BYTE *data = NULL;
	BYTE b;
	int tmp;

	if (!(data = (BYTE *)_alloca(2 + len / 2))) {
		msg(Translate("Couldn't allocate enough memory!"), modFullname);
		return 0;
	}

	while (j < len) {
		b = szValue[j];

		if ((b >= '0' && b <= '9') ||
			(b >= 'A' && b <= 'F') ||
			(b >= 'a' && b <= 'f'))
		{
			if (sscanf(&szValue[j], "%02X", &tmp) == 1) {
				data[i++] = (BYTE)tmp;
				j++;
			}
		}
		j++;
	}

	if (i)
		return db_set_blob(hContact, szModule, szSetting, data, (WORD)i);

	return 0;
}

int GetSetting(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return db_get_s(hContact, szModule, szSetting, dbv, 0);
}

int GetValue(MCONTACT hContact, const char *szModule, const char *szSetting, char *Value, int length)
{
	DBVARIANT dbv = { 0 };

	if (Value && length >= 10 && !GetSetting(hContact, szModule, szSetting, &dbv)) {
		switch (dbv.type) {
		case DBVT_ASCIIZ:
			strncpy(Value, dbv.pszVal, length);
			break;
		case DBVT_DWORD:
			_itoa(dbv.dVal, Value, 10);
			break;
		case DBVT_BYTE:
			_itoa(dbv.bVal, Value, 10);
			break;
		case DBVT_WORD:
			_itoa(dbv.wVal, Value, 10);
			break;
		case DBVT_UTF8:
			int len = (int)mir_strlen(dbv.pszVal) + 1;
			char *sz = (char *)_alloca(len * 3);
			WCHAR *wc = (WCHAR *)_alloca(len * sizeof(WCHAR));
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, wc, len);
			WideCharToMultiByte(CP_ACP, 0, wc, -1, sz, len, NULL, NULL);
			strncpy(Value, sz, length);
			break;
		}

		db_free(&dbv);

		Value[length - 1] = 0;
		return 1;
	}

	if (Value)
		Value[0] = 0;

	return 0;
}

int GetValueW(MCONTACT hContact, const char *szModule, const char *szSetting, WCHAR *Value, int length)
{
	DBVARIANT dbv = { 0 };
	WCHAR *wc;
	int len;

	if (Value && length >= 10 && !GetSetting(hContact, szModule, szSetting, &dbv)) {
		switch (dbv.type) {
		case DBVT_UTF8:
			len = (int)mir_strlen(dbv.pszVal) + 1;
			wc = (WCHAR *)_alloca(length * sizeof(WCHAR));
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, wc, len);
			wcsncpy((WCHAR *)Value, wc, length);
			break;

		case DBVT_ASCIIZ:
			len = (int)mir_strlen(dbv.pszVal) + 1;
			wc = (WCHAR *)_alloca(len * sizeof(WCHAR));
			MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, wc, len);
			wcsncpy((WCHAR *)Value, wc, length);
			break;

		case DBVT_DWORD:
			_itow(dbv.dVal, Value, 10);
			break;

		case DBVT_BYTE:
			_itow(dbv.bVal, Value, 10);
			break;

		case DBVT_WORD:
			_itow(dbv.wVal, Value, 10);
			break;
		}

		db_free(&dbv);

		Value[length - 1] = 0;
		return 1;
	}

	if (Value)
		Value[0] = 0;

	return 0;
}

char *u2a(wchar_t *src)
{
	if (!src)
		return NULL;

	int cbLen = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	char *result = (char *)mir_calloc((cbLen + 1) * sizeof(char));
	if (result == NULL)
		return NULL;

	WideCharToMultiByte(CP_ACP, 0, src, -1, result, cbLen, NULL, NULL);
	result[cbLen] = 0;
	return result;
}

wchar_t *a2u(char *src, wchar_t *buffer, int len)
{
	wchar_t *result = buffer;
	if (result == NULL || len < 3)
		return NULL;

	MultiByteToWideChar(CP_ACP, 0, src, -1, result, len - 1);
	result[len - 1] = 0;

	return result;
}

int GetDatabaseString(MCONTACT hContact, const char *szModule, const char *szSetting, WCHAR *Value, int length, BOOL unicode)
{
	if (unicode)
		return GetValueW(hContact, szModule, szSetting, Value, length);
	else
		return GetValue(hContact, szModule, szSetting, (char *)Value, length);
}

WCHAR* GetContactName(MCONTACT hContact, const char *szProto, int unicode)
{
	int i, r = 0;
	static WCHAR res[512];
	char *proto = (char *)szProto;
	char name[256];

	if (hContact && !proto)
		if (GetValue(hContact, "Protocol", "p", name, SIZEOF(name)))
			proto = name;

	if (proto) {
		for (i = 0; i < NAMEORDERCOUNT - 1; i++) {
			switch (nameOrder[i]) {
			case 0: // custom name
				r = GetDatabaseString(hContact, "CList", "MyHandle", res, SIZEOF(res), unicode);
				break;

			case 1: // nick
				r = GetDatabaseString(hContact, proto, "Nick", res, SIZEOF(res), unicode);
				break;

			case 2: // First Name
				r = GetDatabaseString(hContact, proto, "FirstName", res, SIZEOF(res), unicode);
				break;

			case 3: // E-mail
				r = GetDatabaseString(hContact, proto, "e-mail", res, SIZEOF(res), unicode);
				break;

			case 4: // Last Name
				if (GetDatabaseString(hContact, proto, "LastName", res, SIZEOF(res), unicode))
					break;

			case 5: // Unique id
			{
				// protocol must define a PFLAG_UNIQUEIDSETTING
				char *uid = (char *)CallProtoService(proto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
				if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid)
					r = GetDatabaseString(hContact, proto, uid, res, SIZEOF(res), unicode);
			}
			break;
			case 6: // first + last name
			{
				int len = 0;

				if (r = GetDatabaseString(hContact, proto, "FirstName", res, SIZEOF(res), unicode)) {
					if (unicode)
						len = (int)mir_wstrlen(res);
					else
						len = (int)mir_strlen((char *)res);
				}
				else
					res[0] = 0;

				if (len && len < SIZEOF(res) - 2) {
					if (unicode)
						mir_wstrcat(res, L" ");
					else
						mir_strcat((char*)res, " ");

					len++;
				}

				if (SIZEOF(res) - len > 1)
					r |= GetDatabaseString(hContact, proto, "LastName", &res[len], SIZEOF(res) - len, unicode);

				break;
			}
			}

			if (r)
				return res;
		}
	}

	return (unicode) ? nick_unknownW : (WCHAR *)nick_unknown;
}
