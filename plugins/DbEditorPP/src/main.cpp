#include "headers.h"

HINSTANCE hInst = NULL;

MIDatabase *g_db;

HANDLE hTTBButt = NULL;
BOOL bServiceMode = FALSE;
BOOL usePopups;

int hLangpack;
BYTE nameOrder[NAMEORDERCOUNT];
HGENMENU hUserMenu;
MCONTACT hRestore;

extern HWND hwnd2watchedVarsWindow;

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


int DBSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

	// setting list
	if (hwnd2mainWindow)
		settingChanged(hContact, cws->szModule, cws->szSetting, &(cws->value));
	
	// watch list
	if (!hwnd2watchedVarsWindow && !usePopups)
		return 0;

	if (WatchedArrayIndex(hContact, cws->szModule, cws->szSetting, 0) >= 0)
	{
		if (usePopups) popupWatchedVar(hContact, cws->szModule, cws->szSetting);
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
	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = "DBEditorpp/MenuCommand";
	ttb.name = LPGEN("Database Editor++");
	ttb.hIconHandleUp = GetIcoLibHandle(ICO_DBE_BUTT);
	ttb.pszTooltipUp = LPGEN("Open Database Editor");
	hTTBButt = TopToolbar_AddButton(&ttb);
	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	IcoLibRegister();

	// Register menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 1900000001;
	mi.icolibItem = GetIcoLibHandle(ICO_DBE_BUTT);
	mi.pszPopupName = "Database";
	mi.pszName = modFullname;
	mi.pszService = "DBEditorpp/MenuCommand";
	Menu_AddMainMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 1900000001;
	mi.flags = 0;
	mi.icolibItem = GetIcoLibHandle(ICO_REGUSER);
	mi.pszName = LPGEN("Open user tree in DBE++");
	mi.pszService = "DBEditorpp/MenuCommand";
	hUserMenu = Menu_AddContactMenuItem(&mi);

	// Register hotkeys
	_A2T text(modFullname);
	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszName = "hk_dbepp_open";
	hkd.pszService = "DBEditorpp/MenuCommand";
	hkd.ptszDescription = LPGENT("Open Database Editor");
	hkd.ptszSection = text;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_EXT, 'D');
	Hotkey_Register(&hkd);

	usePopups = db_get_b(NULL, modname, "UsePopUps", 0);

	// Load the name order
	for (int i = 0; i < NAMEORDERCOUNT; i++)
		nameOrder[i] = i;

	DBVARIANT dbv = {0};
	if (!db_get_s(NULL, "Contact", "NameOrder", &dbv, DBVT_BLOB)) {
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
	return 0;
}

INT_PTR ServiceMode(WPARAM, LPARAM)
{
	IcoLibRegister();
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

	hwnd2mainWindow = NULL;

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

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	freeAllWatches();
	return 0;
}


// ======================================================================================================================

char *StringFromBlob(BYTE *blob, WORD len)
{
	int j;
	char tmp[16];

	char *data = (char*)mir_alloc(3 * (len + 2));
	data[0] = 0;

	for (j = 0; j < len; j++)
	{
		mir_snprintf(tmp, SIZEOF(tmp), "%02X ", blob[j]);
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
			(b >= 'a' && b <= 'f'))
		{
			if (sscanf(&szValue[j], "%02X", &tmp) == 1) {
				data[i++] = (BYTE)(tmp&0xFF);
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

TCHAR *DBVType(BYTE type)
{
	switch (type) {
	case DBVT_BYTE:		return _T("BYTE");
	case DBVT_WORD:		return _T("WORD");
	case DBVT_DWORD:	return _T("DWORD");
	case DBVT_ASCIIZ:	return _T("STRING");
	case DBVT_WCHAR:
	case DBVT_UTF8:		return _T("UNICODE");
	case DBVT_BLOB:		return _T("BLOB");
	case DBVT_DELETED:	return _T("DELETED");
	}
	return _T("");
}


DWORD getNumericValue(DBVARIANT *dbv) {
	switch(dbv->type) {
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
	switch(type) {
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


int IsRealUnicode(TCHAR *value)
{
#ifdef _UNICODE
	BOOL nonascii = 0;
	WideCharToMultiByte(Langpack_GetDefaultCodePage(), WC_NO_BEST_FIT_CHARS, value, -1, NULL, 0, NULL, &nonascii);
	return nonascii;
#endif
	return 0;
}


int setTextValue(MCONTACT hContact, const char *module, const char *setting, TCHAR *value, int type)
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
	DBVARIANT dbv = { 0 };

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
		{
			ptrA str(mir_u2a(dbv.pwszVal));
			mir_strncpy(value, str, length);
			break;

		}
		case DBVT_UTF8:
		{
			ptrA str(mir_utf8decodeA(dbv.pszVal));
			mir_strncpy(value, str, length);
			break;
		}
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
	DBVARIANT dbv = { 0 };

	if (!module || !setting || !value)
		return 0;

	if (length >= 10 && !db_get_s(hContact, module, setting, &dbv, 0)) {
		switch (dbv.type) {

		case DBVT_ASCIIZ:
		{
			ptrW str(mir_a2u(dbv.pszVal));
			mir_wstrncpy(value, str, length);			        	
			break;
		}
		case DBVT_DWORD:
		case DBVT_WORD:
		case DBVT_BYTE:
			_ultow(getNumericValue(&dbv), value, 10);
			break;

		case DBVT_WCHAR:
			mir_wstrncpy(value, dbv.pwszVal, length);
			break;

		case DBVT_UTF8:
		{	
			ptrW str(mir_utf8decodeW(dbv.pszVal));
			mir_wstrncpy(value, str, length);
			break;
		}
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


int GetContactName(MCONTACT hContact, const char *proto, TCHAR *value, int maxlen)
{
	if (!value)
		return 0;

	if (!hContact) {
		mir_tstrncpy(value, TranslateT("Settings"), maxlen);
		return 1;
	}

	char *szProto = (char*)proto;
	char tmp[FLD_SIZE];
	TCHAR name[NAME_SIZE];
	name[0] = 0;

	if (hContact && (!proto || !proto[0])) {
		if (!db_get_static(hContact, "Protocol", "p", tmp, SIZEOF(tmp)))
			szProto = tmp;
	}

	for (int i = 0; i < NAMEORDERCOUNT - 1; i++) {
		switch (nameOrder[i]) {
		case 0: // custom name
			GetValue(hContact, "CList", "MyHandle", name, SIZEOF(name));
			break;

		case 1: // nick
			if (!szProto) break;
			GetValue(hContact, szProto, "Nick", name, SIZEOF(name));
			break;
/*
		case 2: // First Name
			if (!szProto) break;
			GetValue(hContact, szProto, "FirstName", name, SIZEOF(name));
			break;
*/
		case 3: // E-mail
			if (!szProto) break;
			GetValue(hContact, szProto, "e-mail", name, SIZEOF(name));
			break;
/*
		case 4: // Last Name
			GetValue(hContact, szProto, "LastName", name, SIZEOF(name));
			break;
*/
		case 5: // Unique id
		{
			if (!szProto) break;
			// protocol must define a PFLAG_UNIQUEIDSETTING
			char *uid = (char *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
			if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid)
				GetValue(hContact, szProto, uid, name, SIZEOF(name));
		}
		break;
		case 6: // first + last name
		{
			if (!szProto) break;

			GetValue(hContact, szProto, "FirstName", name, SIZEOF(name));

			int len = mir_tstrlen(name);
			if (len + 2 < SIZEOF(name)) {
			    if (len)
					mir_tstrncat(name, _T(" "), SIZEOF(name));
				len++;
				GetValue(hContact, szProto, "LastName", &name[len], SIZEOF(name) - len);
			}
		}
		break;
		}

		if (name[0])
			break;
	}

	if (!name[0])
		mir_tstrncpy(name, TranslateT("<UNKNOWN>"), SIZEOF(name));

	if (szProto && szProto[0]) {
		if (g_Order)
			mir_sntprintf(value, maxlen, _T("(%s) %s"), _A2T(szProto), name);
		else
			mir_sntprintf(value, maxlen , _T("%s (%s)"), name, _A2T(szProto));
	}
	else
		mir_tstrncpy(value, name, maxlen);

	if (!szProto || !Proto_IsProtocolLoaded(szProto)) {
		mir_tstrncat(value, _T(" "), maxlen);
		mir_tstrncat(value, TranslateT("[UNLOADED]"), maxlen);
	}

	return 1;
}


int ApplyProtoFilter(MCONTACT hContact)
{
	if (g_Mode == MODE_ALL)	return 0;

	int loaded = 0;
	char szProto[FLD_SIZE];

	if (!db_get_static(hContact, "Protocol", "p", szProto, SIZEOF(szProto)))
		loaded = Proto_IsProtocolLoaded(szProto) ? 1 : 0;

	if ((loaded && g_Mode == MODE_UNLOADED) || (!loaded && g_Mode == MODE_LOADED))
		return 1;

	return 0;
}


void loadListSettings(HWND hwnd, ColumnsSettings *cs)
{
	LVCOLUMN sLC = {0};
	sLC.fmt = LVCFMT_LEFT;
	sLC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	int i = 0; 
	while (cs[i].name) {
		sLC.pszText = TranslateTS(cs[i].name);
		sLC.cx = db_get_w(NULL, modname, cs[i].dbname, cs[i].defsize);
		ListView_InsertColumn(hwnd, cs[i].index, &sLC);
		i++;
	}
}


void saveListSettings(HWND hwnd, ColumnsSettings *cs)
{
	char tmp[FLD_SIZE];
	LVCOLUMN sLC = {0};
	sLC.mask = LVCF_WIDTH;
	int i = 0;
	while (cs[i].name) {
		if (ListView_GetColumn(hwnd, cs[i].index, &sLC)) {
			mir_snprintf(tmp, SIZEOF(tmp), cs[i].dbname, i);
			db_set_w(NULL, modname, tmp, (WORD)sLC.cx);
		}
		i++;
	}
}


INT_PTR CALLBACK ColumnsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	ColumnsSortParams params = *(ColumnsSortParams *)myParam;
	const int maxSize = 1024;
	TCHAR text1[maxSize];
	TCHAR text2[maxSize];
	ListView_GetItemText(params.hList, lParam1, params.column, text1, SIZEOF(text1));
	ListView_GetItemText(params.hList, lParam2, params.column, text2, SIZEOF(text2));

	int res = mir_tstrcmpi(text1, text2);
	return (params.column == params.last) ? -res : res;
}


