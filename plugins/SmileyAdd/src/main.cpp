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

#include "general.h"

//globals
HINSTANCE g_hInst;
HANDLE    hEvent1;
HGENMENU  hContactMenuItem;

int hLangpack;

static const PLUGININFOEX pluginInfoEx =
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
	// {BD542BB4-5AE4-4D0E-A435-BA8DBE39607F}
	{0xbd542bb4, 0x5ae4, 0x4d0e, {0xa4, 0x35, 0xba, 0x8d, 0xbe, 0x39, 0x60, 0x7f}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD /* mirandaVersion */)
{
	return (PLUGININFOEX*)&pluginInfoEx;
}

static IconItem icon = { LPGEN("Button smiley"), "SmileyAdd_ButtonSmiley", IDI_SMILINGICON };

static int ModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTPOPUP;
	mi.popupPosition = 2000070050;
	mi.position = 2000070050;
	mi.icolibItem = icon.hIcolib;
	mi.pszPopupName = (char*)-1;
	mi.pszName = LPGEN("Assign smiley category");
	hContactMenuItem = Menu_AddContactMenuItem(&mi);

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

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS)) {
		ReportError(TranslateT("Only one instance of SmileyAdd could be executed.\nRemove duplicate instances from 'Plugins' directory"));

		return 1;
	}

	InitImageCache();

	Icon_Register(g_hInst, "SmileyAdd", &icon, 1);

	g_SmileyCategories.SetSmileyPackStore(&g_SmileyPacks);

	opt.Load();

	// create smiley events
	hEvent1 = CreateHookableEvent(ME_SMILEYADD_OPTIONSCHANGED);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MirandaShutdown);
	HookEvent(ME_OPT_INITIALISE, SmileysOptionsInitialize);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, RebuildContactMenu);
	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, UpdateSrmmDlg);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccountListChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DbSettingChanged);

	//create the smiley services
	CreateServiceFunction(MS_SMILEYADD_REPLACESMILEYS, ReplaceSmileysCommand);
	CreateServiceFunction(MS_SMILEYADD_SHOWSELECTION, ShowSmileySelectionCommand);
	CreateServiceFunction(MS_SMILEYADD_GETINFO2, GetInfoCommand2);
	CreateServiceFunction(MS_SMILEYADD_REGISTERCATEGORY, RegisterPack);
	CreateServiceFunction(MS_SMILEYADD_BATCHPARSE, ParseTextBatch);
	CreateServiceFunction(MS_SMILEYADD_BATCHFREE, FreeTextBatch);
	CreateServiceFunction(MS_SMILEYADD_CUSTOMCATMENU, CustomCatMenu);
	CreateServiceFunction(MS_SMILEYADD_RELOAD, ReloadPack);
	CreateServiceFunction(MS_SMILEYADD_LOADCONTACTSMILEYS, LoadContactSmileys);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	RemoveDialogBoxHook();

	DestroyHookableEvent(hEvent1);

	RichEditData_Destroy();
	DestroyAniSmileys();

	g_SmileyCategories.ClearAll();
	g_SmileyPackCStore.ClearAndFreeAll();

	DestroyImageCache();
	DestroyGdiPlus();

	DownloadClose();
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
	switch(fdwReason) {
	case DLL_PROCESS_ATTACH:
		g_hInst = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
