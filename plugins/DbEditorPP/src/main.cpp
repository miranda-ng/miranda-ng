#include "stdafx.h"

MIDatabase *g_db;

HANDLE hTTBButt = nullptr;
bool g_bServiceMode = false;
bool g_bUsePopups;

CMPlugin g_plugin;
BYTE nameOrder[NAMEORDERCOUNT];
HGENMENU hUserMenu;
MCONTACT hRestore;

extern HWND hwnd2watchedVarsWindow;

#pragma comment(lib, "shlwapi.lib")

/////////////////////////////////////////////////////////////////////////////////////////
//  MirandaPluginInfo

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A8A417EF-07AA-4F37-869F-7BFD74886534}
	{ 0xa8a417ef, 0x7aa, 0x4f37, { 0x86, 0x9f, 0x7b, 0xfd, 0x74, 0x88, 0x65, 0x34 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	iPopupDelay(MODULENAME, "PopupDelay", 4),
	iPopupBkColor(MODULENAME, "PopupColour", RGB(255, 0, 0)),
	iPopupTxtColor(MODULENAME, "PopupTextColour", RGB(0, 0, 0)),
	bWarnOnDelete(MODULENAME, "WarnOnDelete", true),
	bRestoreOnOpen(MODULENAME, "RestoreOnOpen", true),
	bExpandSettingsOnOpen(MODULENAME, "ExpandSettingsOnOpen", false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// we implement service mode interface

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static int DBSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

	// setting list
	if (hwnd2mainWindow)
		settingChanged(hContact, cws->szModule, cws->szSetting, &(cws->value));

	// watch list
	if (!hwnd2watchedVarsWindow && !g_bUsePopups)
		return 0;

	if (WatchedArrayIndex(hContact, cws->szModule, cws->szSetting, 0) >= 0) {
		if (g_bUsePopups) popupWatchedVar(hContact, cws->szModule, cws->szSetting);
		PopulateWatchedWindow();
	}

	return 0;
}

INT_PTR DBEditorppMenuCommand(WPARAM wParam, LPARAM)
{
	if (!hwnd2mainWindow) { // so only opens 1 at a time
		hRestore = wParam;
		openMainWindow();
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

static int OnTTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = "DBEditorpp/MenuCommand";
	ttb.name = LPGEN("Database Editor++");
	ttb.hIconHandleUp = g_plugin.getIconHandle(ICO_DBE_BUTT);
	ttb.pszTooltipUp = LPGEN("Open Database Editor");
	hTTBButt = g_plugin.addTTB(&ttb);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	IcoLibRegister();

	// Register menu item
	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000);

	SET_UID(mi, 0xe298849c, 0x1a8c, 0x4fc7, 0xa0, 0xf4, 0x78, 0x18, 0xf, 0xe2, 0xf7, 0xc9);
	mi.position = 1900000001;
	mi.hIcolibItem = g_plugin.getIconHandle(ICO_DBE_BUTT);
	mi.name.a = modFullname;
	mi.pszService = "DBEditorpp/MenuCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x2fed8613, 0xac43, 0x4148, 0xbd, 0x5c, 0x44, 0x88, 0xaf, 0x68, 0x69, 0x10);
	mi.root = nullptr;
	mi.hIcolibItem = g_plugin.getIconHandle(ICO_REGUSER);
	mi.name.a = LPGEN("Open user tree in DBE++");
	mi.pszService = "DBEditorpp/MenuCommand";
	hUserMenu = Menu_AddContactMenuItem(&mi);

	// Register hotkeys
	HOTKEYDESC hkd = {};
	hkd.pszName = "hk_dbepp_open";
	hkd.pszService = "DBEditorpp/MenuCommand";
	hkd.szSection.a = modFullname;
	hkd.szDescription.a = LPGEN("Open Database Editor");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_EXT, 'D');
	g_plugin.addHotkey(&hkd);

	g_bUsePopups = g_plugin.getByte("UsePopUps", 0) != 0;

	// Load the name order
	for (int i = 0; i < NAMEORDERCOUNT; i++)
		nameOrder[i] = i;

	DBVARIANT dbv = {};
	if (!db_get_s(0, "Contact", "NameOrder", &dbv, DBVT_BLOB)) {
		memcpy(nameOrder, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}

	HookEvent(ME_TTB_MODULELOADED, OnTTBLoaded);

	if (g_bServiceMode)
		CallService("DBEditorpp/MenuCommand", 0, 0);
	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	if (hwnd2watchedVarsWindow) DestroyWindow(hwnd2watchedVarsWindow);
	if (hwnd2mainWindow) DestroyWindow(hwnd2mainWindow);
	return 0;
}

static INT_PTR ServiceMode(WPARAM, LPARAM)
{
	g_bServiceMode = true;

	IcoLibRegister();
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);

	return SERVICE_ONLYDB;  // load database and then call us
}

static INT_PTR ImportFromFile(WPARAM wParam, LPARAM lParam)
{
	ImportSettingsFromFileMenuItem(wParam, (char *)lParam);
	return 0;
}

int CMPlugin::Load()
{
	hwnd2mainWindow = nullptr;

	hRestore = NULL;
	g_db = db_get_current();

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

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	freeAllWatches();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

char *StringFromBlob(BYTE *blob, WORD len)
{
	int j;
	char tmp[16];

	char *data = (char*)mir_alloc(3 * (len + 2));
	data[0] = 0;

	for (j = 0; j < len; j++) {
		mir_snprintf(tmp, "%02X ", blob[j]);
		mir_strcat(data, tmp);
	}
	return data;
}

int WriteBlobFromString(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szValue, int len)
{
	int j = 0, i = 0;
	BYTE b;
	int tmp, res = 0;
	BYTE *data = (BYTE*)mir_alloc(2 + len / 2);

	if (!data)
		return 0;

	while (j < len) {
		b = szValue[j];

		if ((b >= '0' && b <= '9') ||
			(b >= 'A' && b <= 'F') ||
			(b >= 'a' && b <= 'f')) {
			if (sscanf(&szValue[j], "%02X", &tmp) == 1) {
				data[i++] = (BYTE)(tmp & 0xFF);
				j++;
			}
		}
		j++;
	}


	if (i)
		res = !db_set_blob(hContact, szModule, szSetting, data, (WORD)i);

	mir_free(data);
	return res;
}

wchar_t *DBVType(BYTE type)
{
	switch (type) {
	case DBVT_BYTE:		return L"BYTE";
	case DBVT_WORD:		return L"WORD";
	case DBVT_DWORD:	return L"DWORD";
	case DBVT_ASCIIZ:	return L"STRING";
	case DBVT_WCHAR:
	case DBVT_UTF8:		return L"UNICODE";
	case DBVT_BLOB:		return L"BLOB";
	case DBVT_DELETED:	return L"DELETED";
	}
	return L"";
}

DWORD getNumericValue(DBVARIANT *dbv)
{
	switch (dbv->type) {
	case DBVT_DWORD:
		return dbv->dVal;
	case DBVT_WORD:
		return dbv->wVal;
	case DBVT_BYTE:
		return dbv->bVal;
	}
	return 0;
}

int setNumericValue(MCONTACT hContact, const char *module, const char *setting, DWORD value, int type)
{
	switch (type) {
	case DBVT_BYTE:
		if (value <= 0xFF)
			return !db_set_b(hContact, module, setting, (BYTE)value);
		break;

	case DBVT_WORD:
		if (value <= 0xFFFF)
			return !db_set_w(hContact, module, setting, (WORD)value);
		break;

	case DBVT_DWORD:
		return !db_set_dw(hContact, module, setting, value);
	}
	return 0;
}

int IsRealUnicode(wchar_t *value)
{
	BOOL nonascii = 0;
	WideCharToMultiByte(Langpack_GetDefaultCodePage(), WC_NO_BEST_FIT_CHARS, value, -1, nullptr, 0, nullptr, &nonascii);
	return nonascii;
}

int setTextValue(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int type)
{
#ifdef _UNICODE
	if (type == DBVT_UTF8 || type == DBVT_WCHAR)
		return !db_set_ws(hContact, module, setting, value);

	if (type == DBVT_ASCIIZ && IsRealUnicode(value))
		return 0;
#endif
	return !db_set_s(hContact, module, setting, _T2A(value));
}

int GetValueA(MCONTACT hContact, const char *module, const char *setting, char *value, int length)
{
	DBVARIANT dbv = {};

	if (!module || !setting || !value)
		return 0;

	if (length >= 10 && !db_get_s(hContact, module, setting, &dbv, 0)) {
		switch (dbv.type) {

		case DBVT_ASCIIZ:
			mir_strncpy(value, dbv.pszVal, length);
			break;

		case DBVT_DWORD:
		case DBVT_WORD:
		case DBVT_BYTE:
			_ultoa(getNumericValue(&dbv), value, 10);
			break;

		case DBVT_WCHAR:
			mir_strncpy(value, ptrA(mir_u2a(dbv.pwszVal)), length);
			break;

		case DBVT_UTF8:
			mir_strncpy(value, ptrA(mir_utf8decodeA(dbv.pszVal)), length);
			break;

		case DBVT_DELETED:
			value[0] = 0;
			return 0;
		}

		int type = dbv.type;
		db_free(&dbv);
		return type;
	}

	value[0] = 0;
	return 0;
}

int GetValueW(MCONTACT hContact, const char *module, const char *setting, WCHAR *value, int length)
{
	DBVARIANT dbv = {};

	if (!module || !setting || !value)
		return 0;

	if (length >= 10 && !db_get_s(hContact, module, setting, &dbv, 0)) {
		switch (dbv.type) {

		case DBVT_ASCIIZ:
			mir_wstrncpy(value, ptrW(mir_a2u(dbv.pszVal)), length);
			break;

		case DBVT_DWORD:
		case DBVT_WORD:
		case DBVT_BYTE:
			_ultow(getNumericValue(&dbv), value, 10);
			break;

		case DBVT_WCHAR:
			mir_wstrncpy(value, dbv.pwszVal, length);
			break;

		case DBVT_UTF8:
			mir_wstrncpy(value, ptrW(mir_utf8decodeW(dbv.pszVal)), length);
			break;

		case DBVT_DELETED:
			value[0] = 0;
			return 0;
		}

		int type = dbv.type;
		db_free(&dbv);
		return type;
	}

	value[0] = 0;
	return 0;
}

int GetContactName(MCONTACT hContact, const char *proto, wchar_t *value, int maxlen)
{
	if (!value)
		return 0;

	if (!hContact) {
		mir_wstrncpy(value, TranslateT("Settings"), maxlen);
		return 1;
	}

	char *szProto = (char*)proto;
	char tmp[FLD_SIZE];
	wchar_t name[NAME_SIZE]; name[0] = 0;

	if (hContact && (!proto || !proto[0]))
		if (!db_get_static(hContact, "Protocol", "p", tmp, _countof(tmp)))
			szProto = tmp;

	for (int i = 0; i < NAMEORDERCOUNT - 1; i++) {
		switch (nameOrder[i]) {
		case 0: // custom name
			GetValue(hContact, "CList", "MyHandle", name, _countof(name));
			break;

		case 1: // nick
			if (!szProto) break;
			GetValue(hContact, szProto, "Nick", name, _countof(name));
			break;

		case 2: // First Name
			// if (!szProto) break;
			// GetValue(hContact, szProto, "FirstName", name, _countof(name));
			break;

		case 3: // E-mail
			if (!szProto) break;
			GetValue(hContact, szProto, "e-mail", name, _countof(name));
			break;

		case 4: // Last Name
			// GetValue(hContact, szProto, "LastName", name, _countof(name));
			break;

		case 5: // Unique id
			if (szProto) {
				// protocol must define a PFLAG_UNIQUEIDSETTING
				const char *uid = Proto_GetUniqueId(szProto);
				if (uid)
					GetValue(hContact, szProto, uid, name, _countof(name));
			}
			break;

		case 6: // first + last name
			if (szProto) {
				GetValue(hContact, szProto, "FirstName", name, _countof(name));

				int len = (int)mir_wstrlen(name);
				if (len + 2 < _countof(name)) {
					if (len)
						mir_wstrncat(name, L" ", _countof(name));
					len++;
					GetValue(hContact, szProto, "LastName", &name[len], _countof(name) - len);
				}
			}
			break;
		}

		if (name[0])
			break;
	}

	if (!name[0])
		mir_wstrncpy(name, TranslateT("<UNKNOWN>"), _countof(name));

	if (szProto && szProto[0]) {
		if (g_Order)
			mir_snwprintf(value, maxlen, L"(%S) %s", szProto, name);
		else
			mir_snwprintf(value, maxlen, L"%s (%S)", name, szProto);
	}
	else mir_wstrncpy(value, name, maxlen);

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (!pa->IsEnabled()) {
		mir_wstrncat(value, L" ", maxlen);
		mir_wstrncat(value, TranslateT("[UNLOADED]"), maxlen);
	}

	return 1;
}

int ApplyProtoFilter(MCONTACT hContact)
{
	if (g_Mode == MODE_ALL)	return 0;

	int loaded = 0;
	char szProto[FLD_SIZE];

	if (!db_get_static(hContact, "Protocol", "p", szProto, _countof(szProto)))
		loaded = Proto_GetAccount(szProto) ? 1 : 0;

	if ((loaded && g_Mode == MODE_UNLOADED) || (!loaded && g_Mode == MODE_LOADED))
		return 1;

	return 0;
}

void loadListSettings(HWND hwnd, ColumnsSettings *cs)
{
	LVCOLUMN sLC = {};
	sLC.fmt = LVCFMT_LEFT;
	sLC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	int i = 0;
	while (cs[i].name) {
		sLC.pszText = TranslateW(cs[i].name);
		sLC.cx = g_plugin.getWord(cs[i].dbname, cs[i].defsize);
		ListView_InsertColumn(hwnd, cs[i].index, &sLC);
		i++;
	}
}

void saveListSettings(HWND hwnd, ColumnsSettings *cs)
{
	char tmp[FLD_SIZE];
	LVCOLUMN sLC = {};
	sLC.mask = LVCF_WIDTH;
	int i = 0;
	while (cs[i].name) {
		if (ListView_GetColumn(hwnd, cs[i].index, &sLC)) {
			mir_snprintf(tmp, cs[i].dbname, i);
			g_plugin.setWord(tmp, (WORD)sLC.cx);
		}
		i++;
	}
}

INT_PTR CALLBACK ColumnsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	ColumnsSortParams params = *(ColumnsSortParams *)myParam;
	const int maxSize = 1024;
	wchar_t text1[maxSize];
	wchar_t text2[maxSize];
	ListView_GetItemText(params.hList, lParam1, params.column, text1, _countof(text1));
	ListView_GetItemText(params.hList, lParam2, params.column, text2, _countof(text2));

	int res = mir_wstrcmpi(text1, text2);
	return (params.column == params.last) ? -res : res;
}
