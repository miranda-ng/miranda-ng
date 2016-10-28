#include "stdafx.h"

HINSTANCE hInst;
int hLangpack;
HKEY ROOT_KEY = HKEY_CURRENT_USER;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {EB0465E2-CEEE-11DB-83EF-C1BF55D89593}
	{0xeb0465e2, 0xceee, 0x11db, {0x83, 0xef, 0xc1, 0xbf, 0x55, 0xd8, 0x95, 0x93}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

void GetProfilePath(wchar_t *res, size_t resLen)
{
	wchar_t dbname[MAX_PATH], exename[MAX_PATH];
	Profile_GetNameW(_countof(dbname), dbname);
	GetModuleFileName(NULL, exename, _countof(exename));

	wchar_t *p = wcsrchr(dbname, '.');
	if (p) *p = 0;

	mir_snwprintf(res, resLen, L"\"%s\" \"/profile:%s\"", exename, dbname);
}

static void SetAutorun(BOOL autorun)
{
	HKEY hKey;
	DWORD dw;
	switch (autorun) {
	case TRUE:
		if ( RegCreateKeyEx(ROOT_KEY, SUB_KEY, 0, NULL, 0, KEY_CREATE_SUB_KEY|KEY_SET_VALUE,NULL,&hKey,&dw) == ERROR_SUCCESS) {
			wchar_t result[MAX_PATH];
			GetProfilePath(result, _countof(result));
			RegSetValueEx(hKey, L"MirandaNG", 0, REG_SZ, (BYTE*)result, sizeof(wchar_t)*(DWORD)mir_wstrlen(result));
			RegCloseKey(hKey);
		}
		break;
	case FALSE:
		if ( RegOpenKey(ROOT_KEY, SUB_KEY, &hKey) == ERROR_SUCCESS) {
			RegDeleteValue(hKey, L"MirandaNG");
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

	wchar_t result[MAX_PATH], dbpath[MAX_PATH];
	DWORD dwBufLen = MAX_PATH;
	if ( RegQueryValueEx(hKey, L"MirandaNG", NULL, NULL, (LPBYTE)dbpath, &dwBufLen) != ERROR_SUCCESS)
		return FALSE;
	
	GetProfilePath(result, _countof(result));
	return mir_wstrcmpi(result, dbpath) == 0;
}

static INT_PTR CALLBACK DlgProcAutorunOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_AUTORUN, CmpCurrentAndRegistry() ? BST_CHECKED : BST_UNCHECKED); // Check chekbox if Registry value exists
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

static int AutorunOptInitialise(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100100000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTORUN);
	odp.szTitle.a = ModuleName;
	odp.szGroup.a = LPGEN("Services");
	odp.pfnDlgProc = DlgProcAutorunOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
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
