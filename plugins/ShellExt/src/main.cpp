#include "stdafx.h"
#include "shlcom.h"

HINSTANCE hInst;
int hLangpack;
bool bIsVistaPlus;

TCHAR tszLogPath[MAX_PATH];

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
	if (fdwReason == DLL_PROCESS_ATTACH) {
		bIsVistaPlus = GetProcAddress( GetModuleHandleA("kernel32.dll"), "GetProductInfo") != NULL;

		GetTempPath(SIZEOF(tszLogPath), tszLogPath);
		_tcscat_s(tszLogPath, SIZEOF(tszLogPath), _T("shlext.log"));

		hInst = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
	}

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// exported functions

const IID CLSID_ISHLCOM = { 0x72013A26, 0xA94C, 0x11d6, {0x85, 0x40, 0xA5, 0xE6, 0x29, 0x32, 0x71, 0x1D }};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if (rclsid == CLSID_ISHLCOM) {
		TClassFactoryRec *p = new TClassFactoryRec();
		HRESULT hr = p->QueryInterface(riid, ppv);
		if ( FAILED(hr)) {
			delete p;
			return hr;
		}
		logA("DllGetClassObject succeeded\n");
		return S_OK;
	}

	#ifdef LOG_ENABLED
		RPC_CSTR szGuid;
		UuidToStringA(&riid, &szGuid);
		logA("DllGetClassObject {%08x-%04x-%04x-%08x%08x} failed\n", szGuid);
		RpcStringFreeA(&szGuid);
	#endif

	*ppv = NULL;
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow()
{
	logA("DllCanUnloadNow: %d %d\n", DllFactoryCount, DllObjectCount);
	if (DllFactoryCount == 0 && DllObjectCount == 0)
		return S_OK;
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct HRegKey
{
	HRegKey(HKEY hRoot, const char *ptszKey) : m_key(NULL)
	{	RegCreateKeyExA(hRoot, ptszKey, 0, 0, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, 0, &m_key, 0);
	}
	
	~HRegKey() { if (m_key) RegCloseKey(m_key); }
	
	operator HKEY() const { return m_key; }

private:
	HKEY m_key;
};

char str1[100];
char str2[] = "{72013A26-A94C-11d6-8540-A5E62932711D}";
char str3[] = "miranda.shlext";
char str4[] = "Apartment";
 
STDAPI DllRegisterServer()
{
	HRegKey k1(HKEY_CLASSES_ROOT, "miranda.shlext");
	if (k1 == NULL)
		return E_FAIL;

	int str1len = sprintf_s(str1, sizeof(str1), "shlext %d.%d.%d.%d - shell context menu support for Miranda NG", __FILEVERSION_STRING);
	if ( RegSetValueA(k1, NULL, REG_SZ, str1, str1len))
		return E_FAIL;
	if ( RegSetValueA(k1, "CLSID", REG_SZ, str2, sizeof(str2)))
		return E_FAIL;

	//////////////////////////////////////////////////////////////////////////////////////

	HRegKey kClsid(HKEY_CLASSES_ROOT, "CLSID\\{72013A26-A94C-11d6-8540-A5E62932711D}");
	if (kClsid == NULL)
		return E_FAIL;

	if ( RegSetValueA(kClsid, NULL, REG_SZ, str3, sizeof(str3)))
		return E_FAIL;
	if ( RegSetValueA(kClsid, "ProgID", REG_SZ, str3, sizeof(str3)))
		return E_FAIL;

	HRegKey kInprocServer(kClsid, "InprocServer32");
	if (kInprocServer == NULL)
		return E_FAIL;

	TCHAR tszFileName[MAX_PATH];
	GetModuleFileName(hInst, tszFileName, SIZEOF(tszFileName));
	if ( RegSetValueEx(kInprocServer, NULL, 0, REG_SZ, (LPBYTE)tszFileName, sizeof(TCHAR)*(lstrlen(tszFileName)+1)))
		return E_FAIL;
	if ( RegSetValueExA(kInprocServer, "ThreadingModel", 0, REG_SZ, (PBYTE)str4, sizeof(str4)))
		return E_FAIL;

	//////////////////////////////////////////////////////////////////////////////////////

	if ( RegSetValueA(HKEY_CLASSES_ROOT, "*\\shellex\\ContextMenuHandlers\\miranda.shlext", REG_SZ, str2, sizeof(str2)))
		return E_FAIL;
	if ( RegSetValueA(HKEY_CLASSES_ROOT, "Directory\\shellex\\ContextMenuHandlers\\miranda.shlext", REG_SZ, str2, sizeof(str2)))
		return E_FAIL;

	//////////////////////////////////////////////////////////////////////////////////////

	HRegKey k2(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");
	if (k2 == NULL)
		return E_FAIL;
	if ( RegSetValueExA(k2, str2, 0, REG_SZ, (PBYTE)str1, str1len))
		return E_FAIL;

	return S_OK;
}

STDAPI DllUnregisterServer()
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
