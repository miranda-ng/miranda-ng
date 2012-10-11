#include "autorun.h"

HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfoEx=
{ 	// about plugin
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	MIID_AUTORUN
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

void GetProfilePath(TCHAR *res, size_t resLen)
{
	TCHAR dbname[MAX_PATH], dbpath[MAX_PATH], exename[MAX_PATH];
	CallService(MS_DB_GETPROFILENAMET, SIZEOF(dbname), (LPARAM)(TCHAR*) dbname);
	CallService(MS_DB_GETPROFILEPATHT, SIZEOF(dbpath), (LPARAM)(TCHAR*) dbpath);
	GetModuleFileName(NULL,exename, SIZEOF(exename));
	lstrcat(dbpath, _T("\\"));		
	lstrcpyn(dbpath + lstrlen(dbpath), dbname, lstrlen(dbname)-3);
	lstrcat(dbpath, _T("\\"));
	lstrcat(dbpath, dbname);		// path + profile name
	mir_sntprintf(res, resLen, _T("\"%s\" \"%s\""), exename, dbpath);
}

static void SetAutorun(BOOL autorun)
{
	HKEY hKey;
	DWORD dw;
	switch (autorun) {
	case TRUE:
		if ( RegCreateKeyEx(ROOT_KEY, SUB_KEY, 0, NULL, 0, KEY_CREATE_SUB_KEY|KEY_SET_VALUE,NULL,&hKey,&dw) == ERROR_SUCCESS) {
			TCHAR result[MAX_PATH];
			GetProfilePath(result, SIZEOF(result));
			RegSetValueEx(hKey, _T("MirandaNG"), 0, REG_SZ, (BYTE*)result, sizeof(TCHAR)*lstrlen(result));
			RegCloseKey(hKey);
		}
		break;
	case FALSE:
		if ( RegOpenKey(ROOT_KEY, SUB_KEY, &hKey) == ERROR_SUCCESS) {
			RegDeleteValue(hKey, _T("MirandaNG"));
			RegCloseKey(hKey);
		}
		break;
	}
}

static BOOL CmpCurrentAndRegistry()
{
	HKEY hKey;
	if ( RegOpenKeyEx(ROOT_KEY, SUB_KEY, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		return FALSE;

	TCHAR result[MAX_PATH], dbpath[MAX_PATH];
	DWORD dwBufLen = MAX_PATH;
	if ( RegQueryValueEx(hKey, _T("MirandaNG"), NULL, NULL, (LPBYTE)dbpath, &dwBufLen) != ERROR_SUCCESS)
		return FALSE;
	
	GetProfilePath(result, SIZEOF(result));
	return lstrcmpi(result, dbpath) == 0;
}

static INT_PTR CALLBACK DlgProcAutorunOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg,IDC_AUTORUN,CmpCurrentAndRegistry()); // Check chekbox if Registry value exists
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0); // Send message to activate "Apply" button
		return TRUE;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY: // if "Apply" pressed then...
				SetAutorun(IsDlgButtonChecked(hwndDlg,IDC_AUTORUN)); //Save changes to registry;
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

static int AutorunOptInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = 100100000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTORUN);
	odp.pszTitle = LPGEN(ModuleName);
	odp.pszGroup = LPGEN("Events");
	odp.pfnDlgProc = DlgProcAutorunOpts;
	odp.flags = ODPF_BOLDGROUPS;	
	Options_AddPage(wParam, &odp);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);
	HookEvent(ME_OPT_INITIALISE, AutorunOptInitialise);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
