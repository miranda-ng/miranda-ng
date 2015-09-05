#include "stdafx.h"

HINSTANCE hInst;

HANDLE hShowGuide;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// 297EC1E7-41B7-41F9-BB91-EFA95028F16C
	{0x297ec1e7, 0x41b7, 0x41f9, {0xbb, 0x91, 0xef, 0xa9, 0x50, 0x28, 0xf1, 0x6c}}
};

static INT_PTR ShowGuideFile(WPARAM, LPARAM)
{
	LPTSTR pszDirName = (LPTSTR)mir_alloc(250*sizeof(TCHAR));
	LPTSTR pszFileName = (LPTSTR)mir_alloc(250*sizeof(TCHAR));

	TCHAR *ptszHelpFile = db_get_tsa(NULL, "UserGuide", "PathToHelpFile");
	
	if (ptszHelpFile==0)
	{
			mir_tstrcpy(pszDirName, _T("%miranda_path%\\Plugins"));
			mir_tstrcpy(pszFileName, _T("UserGuide.chm"));			
	}
	else
	{
		if(!mir_tstrcmp(ptszHelpFile, _T("")))
		{
			mir_tstrcpy(pszDirName, _T("%miranda_path%\\Plugins"));
			mir_tstrcpy(pszFileName, _T("UserGuide.chm"));
		}
		else 
		{
			LPTSTR pszDivider = _tcsrchr(ptszHelpFile, '\\');
			if (pszDivider == NULL)
			{	
				mir_tstrcpy(pszDirName, _T(""));
				_tcsncpy(pszFileName, ptszHelpFile, mir_tstrlen(ptszHelpFile));
			}
			else
			{
				_tcsncpy(pszFileName, pszDivider + 1, mir_tstrlen(ptszHelpFile) - mir_tstrlen(pszDivider) - 1);
				pszFileName[mir_tstrlen(ptszHelpFile) - mir_tstrlen(pszDivider) - 1] = 0;
				_tcsncpy(pszDirName, ptszHelpFile, pszDivider - ptszHelpFile);
				pszDirName[pszDivider - ptszHelpFile] = 0;
			}
		}
		mir_free(ptszHelpFile);
	}
	LPTSTR pszDirNameEx;
	pszDirNameEx = Utils_ReplaceVarsT(pszDirName);
	mir_free(pszDirName);

	ShellExecute(NULL, _T("open"), pszFileName, NULL, pszDirNameEx, SW_SHOW);
	mir_free(pszFileName);
	mir_free(pszDirNameEx);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	hShowGuide = CreateServiceFunction("UserGuide/ShowGuide", ShowGuideFile);

	CMenuItem mi;
	SET_UID(mi, 0x6787c12d, 0xdc85, 0x409d, 0xaa, 0x6c, 0x1f, 0xfe, 0x5f, 0xe8, 0xc1, 0x18);
	mi.position = 500000;
	mi.flags = CMIF_TCHAR;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_HELP);
	mi.name.t = LPGENT("User Guide");
	mi.pszService = "UserGuide/ShowGuide";
	Menu_AddMainMenuItem(&mi);
	
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyServiceFunction(hShowGuide);
	return 0;
}