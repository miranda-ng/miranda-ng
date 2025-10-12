/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"
#include "version.h"

CMPlugin	g_plugin;

UINT hTimer;
HNETLIBUSER hNetlibUser;
NOTIFYICONDATA niData;

OBJLIST<Account> g_accs(1);
int ID_STATUS_NONEW, g_iStatus;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {243955E0-75D9-4CC3-9B28-6F9C5AF4532D}
	{ 0x243955e0, 0x75d9, 0x4cc3, { 0x9b, 0x28, 0x6f, 0x9c, 0x5a, 0xf4, 0x53, 0x2d } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	circleTime(MODULENAME, "circleTime", 30),
	bNotifierOnTray(MODULENAME, "bNotifierOnTray", true),
	bNotifierOnPop(MODULENAME, "bNotifierOnPop", true),
	popupDuration(MODULENAME, "popupDuration", -1),
	popupBgColor(MODULENAME, "popupBgColor", RGB(173, 206, 247)),
	popupTxtColor(MODULENAME, "popupTxtColor", RGB(0, 0, 0)),
	bUseOnline(MODULENAME, "bUseOnline", false),
	AutoLogin(MODULENAME, "AutoLogin", true),
	OpenUsePrg(MODULENAME, "OpenUsePrg", 0),
	bLogThreads(MODULENAME, "bLogThreads", false)
{
	RegisterProtocol(PROTOTYPE_PROTOCOL);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR GetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_LIGHTDND | PF2_SHORTAWAY;

	case PFLAGNUM_5:
		return PF2_LIGHTDND | PF2_SHORTAWAY;
	}

	return 0;
}

static INT_PTR GetStatus(WPARAM, LPARAM)
{
	return g_iStatus;
}

static INT_PTR SetStatus(WPARAM iStatus, LPARAM)
{
	g_iStatus = (iStatus == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE;
	return 0;
}

static INT_PTR PluginMenuCommand(WPARAM hContact = 0, LPARAM = 0)
{
	mir_forkthread(Check_ThreadFunc, GetAccountByContact(hContact));
	return 0;
}

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if (g_iStatus == ID_STATUS_ONLINE)
		PluginMenuCommand();
}

static int OnMirandaStart(WPARAM, LPARAM)
{
	PluginMenuCommand();
	return 0;
}

int CMPlugin::Load()
{
	g_plugin.addSound("Gmail", LPGENW("Other"), LPGENW("Gmail: New thread(s)"));
	HookEvent(ME_CLIST_DOUBLECLICKED, OpenBrowser);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE | NUF_INCOMING;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("GmailNotifier");
	hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateProtoServiceFunction(MODULENAME, PS_GETCAPS, GetCaps);
	CreateProtoServiceFunction(MODULENAME, PS_GETSTATUS, GetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_SETSTATUS, SetStatus);
	CreateServiceFunction("GmailMNotifier/Notifying", Notifying);

	DBVARIANT dbv;
	if (db_get_s(0, "SkinIcons", "core_status_" MODULENAME "4", &dbv)) {
		db_set_s(0, "SkinIcons", "core_status_" MODULENAME "0", "plugins\\GmailNotifier.dll,2");
		db_set_s(0, "SkinIcons", "core_status_" MODULENAME "1", "plugins\\GmailNotifier.dll,2");
		db_set_s(0, "SkinIcons", "core_status_" MODULENAME "2", "plugins\\GmailNotifier.dll,0");
		db_set_s(0, "SkinIcons", "core_status_" MODULENAME "4", "plugins\\GmailNotifier.dll,1");
	}
	else db_free(&dbv);

	for (auto &hContact : Contacts(MODULENAME))
		g_accs.insert(new Account(hContact));

	ID_STATUS_NONEW = g_plugin.bUseOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
	for (auto &it : g_accs)
		db_set_w(it->hContact, MODULENAME, "Status", ID_STATUS_NONEW);

	hTimer = SetTimer(nullptr, 0, g_plugin.circleTime * 60000, TimerProc);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnMirandaStart);
	HookEvent(ME_OPT_INITIALISE, OptInit);

	CreateServiceFunction(MODULENAME "/MenuCommand", PluginMenuCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xbe16f37, 0x17be, 0x4494, 0xaa, 0xb2, 0x3a, 0xa7, 0x38, 0xfa, 0xf9, 0xcc);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_LoadProtoIcon(MODULENAME, ID_STATUS_ONLINE);
	mi.name.a = LPGEN("&Check all Gmail inboxes");
	mi.pszService = MODULENAME "/MenuCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x22c6ace1, 0xba0c, 0x44b5, 0xa4, 0xd2, 0x1, 0x7d, 0xb1, 0xe0, 0x51, 0xeb);
	mi.name.a = LPGEN("&Check Gmail inbox");
	mi.pszService = "/MenuCommand";
	Menu_AddContactMenuItem(&mi, MODULENAME);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (hTimer)
		KillTimer(nullptr, hTimer);
	
	for (auto &it : g_accs) {
		if (it->popUpHwnd)
			PUDeletePopup(it->popUpHwnd);
		Clist_RemoveEvent(it->hContact, 1);
	}
	g_accs.destroy();

	Netlib_CloseHandle(hNetlibUser);
	return 0;
}
