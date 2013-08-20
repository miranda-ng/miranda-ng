#include "stdafx.h"

HINSTANCE hInst;
int hLangpack;

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
	// {7993AB24-1FDA-428C-A89B-BE377A10BE3A}
	{0x7993ab24, 0x1fda, 0x428c, {0xa8, 0x9b, 0xbe, 0x37, 0x7a, 0x10, 0xbe, 0x3a}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct HRegKey
{
	HRegKey(HKEY hRoot, const TCHAR *ptszKey) : m_key(NULL)
	{	RegCreateKeyEx(hRoot, ptszKey, 0, 0, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, 0, &m_key, 0);
	}
	
	~HRegKey() { if (m_key) RegCloseKey(m_key); }
	
	operator HKEY() const { return m_key; }

private:
	HKEY m_key;
};

char str1[] = "shlext " __VERSION_STRING " - shell context menu support for Miranda NG";
char str2[] = "{72013A26-A94C-11d6-8540-A5E62932711D}";
char str3[] = "miranda.shlext";
char str4[] = "Apartment";

TCHAR key1[] = _T("miranda.shlext\\{72013A26-A94C-11d6-8540-A5E62932711D}\\InprocServer32");
 
HRESULT __stdcall DllRegisterServer()
{
	if ( RegSetValueA(HKEY_CLASSES_ROOT, "miranda.shlext", REG_SZ, str1, sizeof(str1)-1))
		return E_FAIL;
	if ( RegSetValueA(HKEY_CLASSES_ROOT, "miranda.shlext\\CLSID", REG_SZ, str2, sizeof(str2)-1))
		return E_FAIL;
	if ( RegSetValueA(HKEY_CLASSES_ROOT, "miranda.shlext\\{72013A26-A94C-11d6-8540-A5E62932711D}", REG_SZ, str3, sizeof(str3)-1))
		return E_FAIL;
	if ( RegSetValueA(HKEY_CLASSES_ROOT, "miranda.shlext\\{72013A26-A94C-11d6-8540-A5E62932711D}\\ProgID", REG_SZ, str3, sizeof(str3)-1))
		return E_FAIL;

	TCHAR tszFileName[MAX_PATH];
	GetModuleFileName(hInst, tszFileName, SIZEOF(tszFileName));
	if ( RegSetValue(HKEY_CLASSES_ROOT, key1, REG_SZ, tszFileName, lstrlen(tszFileName)))
		return E_FAIL;

	HRegKey k1(HKEY_CLASSES_ROOT, key1);
	if (k1 == NULL)
		return E_FAIL;
	if ( RegSetValueA(k1, "ThreadingModel", REG_SZ, str4, sizeof(str4)))
		return E_FAIL;

	if ( RegSetValueA(HKEY_CLASSES_ROOT, "*\\shellex\\ContextMenuHandlers\\miranda.shlext", REG_SZ, str2, sizeof(str2)-1))
		return E_FAIL;
	if ( RegSetValueA(HKEY_CLASSES_ROOT, "Directory\\shellex\\ContextMenuHandlers\\miranda.shlext", REG_SZ, str2, sizeof(str2)-1))
		return E_FAIL;

	HRegKey k2(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));
	if (k2 == NULL)
		return E_FAIL;
	if ( RegSetValueA(k2, str2, REG_SZ, str1, sizeof(str1)-1))
		return E_FAIL;

	return S_OK;
}

HRESULT __stdcall DllUnregisterServer()
{
  return RemoveCOMRegistryEntries();
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);
	
	InvokeThreadServer();
	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	DllRegisterServer();
	CheckRegisterServer();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
