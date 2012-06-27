#include "commonheaders.h"

HINSTANCE hInst;

HANDLE hModulesLoaded, hShowGuide;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"User Guide Plugin",
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"This plug-in adds the main menu item used to view miranda-im pack user guide.",
	"Yasnovidyashii",
	"yasnovidyashii@gmail.com",
	"© 2009 Mikhail Yuriev",
	"http://miranda-im.org/",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
    // Generate your own unique id for your plugin.
    // Do not use this UUID!
    // Use uuidgen.exe to generate the uuuid
    MIID_USERGUIDE
};

static INT_PTR ShowGuideFile(WPARAM wParam,LPARAM lParam)
{
	DBVARIANT dbv = {0};
	int iRes;

	LPCTSTR pszEmptySting = _T("");
	LPTSTR pszDirName, pszDirNameEx, pszFileName,pszDivider;

	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof( dat );
	dat.dwFlags = RVF_TCHAR;
	
	pszDirName = (LPTSTR)mir_alloc(250*sizeof(TCHAR));
	pszFileName = (LPTSTR)mir_alloc(250*sizeof(TCHAR));

	iRes = DBGetContactSettingTString(NULL, "UserGuide", "PathToHelpFile", &dbv);
	
	if (iRes!=0)
	{
			_tcscpy(pszDirName, _T("%miranda_path%\\Plugins"));
			_tcscpy(pszFileName, _T("UserGuide.chm"));			
	}
	else
	{
		if(!_tcscmp((dbv.ptszVal), pszEmptySting))
		{
			_tcscpy(pszDirName, _T("%miranda_path%\\Plugins"));
			_tcscpy(pszFileName, _T("UserGuide.chm"));
		}
		else 
		{
			pszDivider = _tcsrchr(dbv.ptszVal, '\\');
			if (pszDivider == NULL)
			{	
				pszDirName = _T("");
				_tcsncpy(pszFileName, dbv.ptszVal, _tcslen(dbv.ptszVal));
			}
			else
			{
				_tcsncpy(pszFileName, pszDivider + 1, _tcslen(dbv.ptszVal) - _tcslen(pszDivider) - 1);
				pszFileName[_tcslen(dbv.ptszVal) - _tcslen(pszDivider) - 1] = 0;
				_tcsncpy(pszDirName, dbv.ptszVal, pszDivider - dbv.ptszVal);
				pszDirName[pszDivider - dbv.ptszVal] = 0;
			}
		}
		DBFreeVariant(&dbv);
	}
	if (ServiceExists(MS_UTILS_REPLACEVARS))
		pszDirNameEx = (TCHAR *) CallService(MS_UTILS_REPLACEVARS, (WPARAM)pszDirName, (LPARAM)&dat);
	else
		pszDirNameEx = mir_tstrdup(pszDirName);

	ShellExecute(NULL, _T("open"), pszFileName, NULL, pszDirNameEx, SW_SHOW);
	mir_free(pszDirName);
	mir_free(pszFileName);
	mir_free(pszDirNameEx);
	return 0;
}

int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM mi;

	hShowGuide = CreateServiceFunction("UserGuide/ShowGuide", ShowGuideFile);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 500000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_HELP);
	mi.ptszName = LPGENT("User Guide");
	mi.pszService = "UserGuide/ShowGuide";
	Menu_AddMainMenuItem(&mi);
	
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{

	mir_getLP(&pluginInfo);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hModulesLoaded);
	DestroyServiceFunction(hShowGuide);
	return 0;
}