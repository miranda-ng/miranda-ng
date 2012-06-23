#include "commonheaders.h"

struct MM_INTERFACE mmi;
HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hModulesLoaded;
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
	0,		//not transient
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

	LPCSTR pszEmptySting="";
	LPSTR pszDirName, pszDirNameEx, pszFileName,pszDivider;

	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof( dat );
	dat.dwFlags = 0;
	
	pszDirName=(LPSTR)mir_alloc(250*sizeof(CHAR));
	pszFileName=(LPSTR)mir_alloc(250*sizeof(CHAR));

	iRes = DBGetContactSettingString(NULL,"UserGuide","PathToHelpFile",&dbv);
	
	if (iRes!=0)
	{
			strcpy(pszDirName, "%miranda_path%\\Plugins");
			strcpy(pszFileName, "UserGuide.chm");			
	}
	else
			if(strcmp((dbv.pszVal),pszEmptySting)==0)
			{
				strcpy(pszDirName, "%miranda_path%\\Plugins");
				strcpy(pszFileName, "UserGuide.chm");			
				mir_free(dbv.pszVal);
			}
			else 
			{
				pszDivider = strrchr(dbv.pszVal, '\\');
				if (pszDivider == NULL)
				{	
					pszDirName = "";
					strncpy(pszFileName, dbv.pszVal, strlen(dbv.pszVal));
				}
				else
				{
					strncpy(pszFileName, pszDivider+1, strlen(dbv.pszVal)-strlen(pszDivider)-1);
					pszFileName[strlen(dbv.pszVal)-strlen(pszDivider)-1] = 0;
					strncpy(pszDirName, dbv.pszVal, pszDivider-dbv.pszVal);
					pszDirName[pszDivider-dbv.pszVal] = 0;
				}
				mir_free(dbv.pszVal);
			}
	if (ServiceExists(MS_UTILS_REPLACEVARS))
		pszDirNameEx = (char *) CallService(MS_UTILS_REPLACEVARS,(WPARAM)pszDirName,(LPARAM)&dat);
	else
		pszDirNameEx = mir_strdup(pszDirName);

	ShellExecuteA(NULL,"open",pszFileName,NULL,pszDirNameEx,SW_SHOW);
	mir_free(pszDirName);
	mir_free(pszFileName);
	mir_free(pszDirNameEx);
	return 0;
}

int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM mi;

	CreateServiceFunction("UserGuide/ShowGuide",ShowGuideFile);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=500000;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_HELP);
	mi.pszName=LPGEN("User Guide");
	mi.pszService="UserGuide/ShowGuide";
	Menu_AddMainMenuItem(&mi);
	
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}


__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getMMI(&mmi);
	mir_getLP(&pluginInfo);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hModulesLoaded);
	return 0;
}