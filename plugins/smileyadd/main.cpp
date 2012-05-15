/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "smileys.h"
#include "customsmiley.h"
#include "services.h"
#include "options.h"
#include "download.h"
#include "imagecache.h"
#include "version.h"
#include "m_updater.h"
#include "m_metacontacts.h"

//globals
HINSTANCE g_hInst;
HANDLE hEvent1, hContactMenuItem;
extern LIST<HANDLE> menuHandleArray;

char* metaProtoName;

PLUGINLINK *pluginLink;

//static globals
static HANDLE hHooks[7];
static HANDLE hService[13];
int hLangpack;

MM_INTERFACE   mmi;
LIST_INTERFACE li;
UTF8_INTERFACE utfi;

static const PLUGININFOEX pluginInfoEx = 
{
	sizeof(PLUGININFOEX), 
#if defined(_WIN64)
	"SmileyAdd x64",
#elif defined(UNICODE) | defined(_UNICODE)
	"SmileyAdd Unicode",
#else
	"SmileyAdd",
#endif
	__VERSION_DWORD,
	"Smiley support for Miranda Instant Messanger",
	"Peacow, nightwish, bid, borkra",
	"borkra@miranda-im.org",
	"Copyright© 2004 - 2011 Boris Krasnovskiy, portions by Rein-Peter de Boer",
	"http://code.google.com/p/mirandaimplugins/downloads/list",
	//	"http://addons.miranda-im.org/index.php?action=display&id=2152",
#if defined(UNICODE) | defined(_UNICODE)
	UNICODE_AWARE,		//not transient
#else
	0,
#endif
	0,		//doesn't replace anything built-in
#if defined(UNICODE) | defined(_UNICODE)
	// {BD542BB4-5AE4-4d0e-A435-BA8DBE39607F}
	{ 0xbd542bb4, 0x5ae4, 0x4d0e, { 0xa4, 0x35, 0xba, 0x8d, 0xbe, 0x39, 0x60, 0x7f } }
#else
	// {9817B76A-603A-4616-806C-86ECB97EA05B}
	{ 0x9817b76a, 0x603a, 0x4616, { 0x80, 0x6c, 0x86, 0xec, 0xb9, 0x7e, 0xa0, 0x5b } }
#endif

};

static SKINICONDESC skinDesc = 
{  
	SKINICONDESC_SIZE_V1, "SmileyAdd", NULL, 
	"SmileyAdd_ButtonSmiley", NULL, -IDI_SMILINGICON 
};


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD /* mirandaVersion */) 
{
	return (PLUGININFOEX*)&pluginInfoEx;
}

// MirandaPluginInterfaces - returns the protocol interface to the core
static const MUUID interfaces[] = {MIID_SMILEY, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	char path[MAX_PATH];
	GetModuleFileNameA(g_hInst, path, MAX_PATH);

	skinDesc.pszDefaultFile = path;
	skinDesc.pszDescription = LPGEN("Button Smiley");
	HANDLE hSkinIcon = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&skinDesc);

	INT_PTR temp = CallService(MS_MC_GETPROTOCOLNAME, 0, 0);
	metaProtoName = mir_strdup(temp == CALLSERVICE_NOTFOUND ? NULL : (char*)temp); 

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ROOTPOPUP | CMIF_ICONFROMICOLIB;
	mi.popupPosition = 2000070050;
	mi.position = 2000070050;
	mi.icolibItem = hSkinIcon;
	mi.pszPopupName = (char*)-1;
	mi.pszName = "Assign Smiley Category";
	hContactMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	DownloadInit();

	//install hooks if enabled
	InstallDialogBoxHook(); 

	g_SmileyCategories.AddAllProtocolsAsCategory();
	g_SmileyCategories.ClearAndLoadAll();

	return 0;
}

static int MirandaShutdown(WPARAM, LPARAM)
{
	CloseSmileys();
	return 0;
}


extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;

	mir_getLI(&li);
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLP(&pluginInfoEx);

	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS))
	{
		static const TCHAR errmsg[] = _T("Only one instance of SmileyAdd could be executed.\n")
			_T("Remove duplicate instances from 'Plugins' directory"); 
		ReportError(TranslateTS(errmsg));

		return 1;
	}

	char temp[80];
	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)SIZEOF(temp), (LPARAM)temp);

#ifdef _UNICODE
	if (strstr(temp, "Unicode") == NULL)
	{
		ReportError(TranslateT("Please update SmileyAdd to ANSI Version"));
		return 1;
	}
#else
	if (strstr(temp, "Unicode") != NULL)
	{
		ReportError(Translate("Please update SmileyAdd to Unicode Version"));
		return 1;
	}
#endif

	InitImageCache();

	g_SmileyCategories.SetSmileyPackStore(&g_SmileyPacks);

	opt.Load();

	// create smiley events
	hEvent1 = CreateHookableEvent(ME_SMILEYADD_OPTIONSCHANGED);

	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, MirandaShutdown);
	hHooks[2] = HookEvent(ME_OPT_INITIALISE, SmileysOptionsInitialize);
	hHooks[3] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, RebuildContactMenu);
	hHooks[4] = HookEvent(ME_SMILEYADD_OPTIONSCHANGED, UpdateSrmmDlg);
	hHooks[5] = HookEvent(ME_PROTO_ACCLISTCHANGED, AccountListChanged);
	hHooks[6] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DbSettingChanged);

	//create the smiley services
	hService[0] = CreateServiceFunction(MS_SMILEYADD_REPLACESMILEYS, ReplaceSmileysCommand);
	hService[1] = CreateServiceFunction(MS_SMILEYADD_GETSMILEYICON, GetSmileyIconCommand);
	hService[2] = CreateServiceFunction(MS_SMILEYADD_SHOWSELECTION, ShowSmileySelectionCommand);
	hService[3] = CreateServiceFunction(MS_SMILEYADD_GETINFO, GetInfoCommand);
	hService[4] = CreateServiceFunction(MS_SMILEYADD_GETINFO2, GetInfoCommand2);
	hService[5] = CreateServiceFunction(MS_SMILEYADD_PARSE, ParseText);
	hService[6] = CreateServiceFunction(MS_SMILEYADD_REGISTERCATEGORY, RegisterPack);
	hService[7] = CreateServiceFunction(MS_SMILEYADD_BATCHPARSE, ParseTextBatch);
	hService[8] = CreateServiceFunction(MS_SMILEYADD_BATCHFREE, FreeTextBatch);
	hService[9] = CreateServiceFunction(MS_SMILEYADD_CUSTOMCATMENU, CustomCatMenu);
	hService[10] = CreateServiceFunction(MS_SMILEYADD_RELOAD, ReloadPack);
	hService[11] = CreateServiceFunction(MS_SMILEYADD_LOADCONTACTSMILEYS, LoadContactSmileys); 

#if defined(UNICODE) | defined(_UNICODE)
	hService[12] = CreateServiceFunction(MS_SMILEYADD_PARSEW, ParseTextW);
#endif

	return 0;
}


extern "C" __declspec(dllexport) int Unload(void)
{
	int i;

	RemoveDialogBoxHook();

	for (i=0; i<SIZEOF(hHooks); i++)
		UnhookEvent(hHooks[i]);

	for (i=0; i<SIZEOF(hService); i++)
		DestroyServiceFunction(hService[i]);

	DestroyHookableEvent(hEvent1);

	RichEditData_Destroy();
	DestroyAniSmileys();
	DestroySmileyBase();

	g_SmileyCategories.ClearAll();
	g_SmileyPackCStore.ClearAndFreeAll();

	DestroyImageCache();
	DestroyGdiPlus();

	DownloadClose();
	menuHandleArray.destroy();

	mir_free(metaProtoName);

	return 0;
}


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
