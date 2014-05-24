/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "gmail.h"
#include "version.h"

HINSTANCE hInst;
int hLangpack;
UINT hTimer;
HANDLE hMirandaStarted, hOptionsInitial;
NOTIFYICONDATA niData;
optionSettings opt;
int acc_num = 0;
Account *acc;
BOOL optionWindowIsOpen = FALSE;
short ID_STATUS_NONEW;

static PLUGININFOEX pluginInfoEx =
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
	// {243955E0-75D9-4CC3-9B28-6F9C5AF4532D}
	{ 0x243955e0, 0x75d9, 0x4cc3, { 0x9b, 0x28, 0x6f, 0x9c, 0x5a, 0xf4, 0x53, 0x2d } }
};

INT_PTR GetCaps(WPARAM wParam, LPARAM lParam)
{
	if (wParam == PFLAGNUM_2 && opt.ShowCustomIcon)
		return PF2_ONLINE | PF2_LIGHTDND | PF2_SHORTAWAY;

	return 0;
}

INT_PTR GetStatus(WPARAM wParam, LPARAM lParam)
{
	return ID_STATUS_ONLINE;
}

INT_PTR GetName(WPARAM wParam, LPARAM lParam)
{
	lstrcpynA((char*)lParam, pluginName, wParam);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	PluginMenuCommand(0, 0);
}

INT_PTR PluginMenuCommand(WPARAM hContact, LPARAM)
{
	if (!optionWindowIsOpen)
		mir_forkthread(Check_ThreadFunc, GetAccountByContact(hContact));

	return 0;
}

static int OnMirandaStart(WPARAM, LPARAM)
{
	PluginMenuCommand(0, 0);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern "C" int __declspec(dllexport) Load()
{
	SkinAddNewSoundEx("Gmail", LPGEN("Other"), LPGEN("Gmail: New thread(s)"));
	HookEvent(ME_CLIST_DOUBLECLICKED, OpenBrowser);

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = pluginName;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CreateServiceFunction(pluginName PS_GETCAPS, GetCaps);
	CreateServiceFunction(pluginName PS_GETSTATUS, GetStatus);
	CreateServiceFunction(pluginName PS_GETNAME, GetName);
	CreateServiceFunction("GmailMNotifier/Notifying", Notifying);

	opt.circleTime = db_get_dw(NULL, pluginName, "circleTime", 30);
	opt.notifierOnTray = db_get_dw(NULL, pluginName, "notifierOnTray", TRUE);
	opt.notifierOnPop = db_get_dw(NULL, pluginName, "notifierOnPop", TRUE);
	opt.popupDuration = db_get_dw(NULL, pluginName, "popupDuration", -1);
	opt.popupBgColor = db_get_dw(NULL, pluginName, "popupBgColor", RGB(173, 206, 247));
	opt.popupTxtColor = db_get_dw(NULL, pluginName, "popupTxtColor", RGB(0, 0, 0));
	opt.OpenUsePrg = db_get_dw(NULL, pluginName, "OpenUsePrg", 0);
	opt.ShowCustomIcon = db_get_dw(NULL, pluginName, "ShowCustomIcon", FALSE);
	opt.UseOnline = db_get_dw(NULL, pluginName, "UseOnline", FALSE);
	opt.AutoLogin = db_get_dw(NULL, pluginName, "AutoLogin", TRUE);
	opt.LogThreads = db_get_dw(NULL, pluginName, "LogThreads", FALSE);

	DBVARIANT dbv;
	if (db_get_s(NULL, "Icons", "GmailMNotifier40076", &dbv)) {
		db_set_s(NULL, "Icons", "GmailMNotifier40071", "plugins\\gmailm.dll,2");
		db_set_s(NULL, "Icons", "GmailMNotifier40072", "plugins\\gmailm.dll,2");
		db_set_s(NULL, "Icons", "GmailMNotifier40076", "plugins\\gmailm.dll,0");
		db_set_s(NULL, "Icons", "GmailMNotifier40073", "plugins\\gmailm.dll,1");
	}
	else db_free(&dbv);

	BuildList();
	ID_STATUS_NONEW = opt.UseOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
	for (int i = 0; i < acc_num; i++)
		db_set_dw(acc[i].hContact, pluginName, "Status", ID_STATUS_NONEW);

	hTimer = SetTimer(NULL, 0, opt.circleTime * 60000, TimerProc);
	hMirandaStarted = HookEvent(ME_SYSTEM_MODULESLOADED, OnMirandaStart);
	hOptionsInitial = HookEvent(ME_OPT_INITIALISE, OptInit);
	CreateServiceFunction("GmailMNotifier/MenuCommand", PluginMenuCommand);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -0x7FFFFFFF;
	mi.flags = 0;
	mi.hIcon = LoadSkinnedProtoIcon(pluginName, ID_STATUS_ONLINE);
	mi.pszName = LPGEN("&Check all Gmail inboxes");
	mi.pszContactOwner = pluginName;
	mi.pszService = "GmailMNotifier/MenuCommand";
	Menu_AddMainMenuItem(&mi);
	mi.pszName = LPGEN("&Check Gmail inbox");
	Menu_AddContactMenuItem(&mi);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hTimer)
		KillTimer(NULL, hTimer);
	for (int i = 0; i < acc_num; i++)
		DeleteResults(acc[i].results.next);
	free(acc);
	UnhookEvent(hMirandaStarted);
	UnhookEvent(hOptionsInitial);
	return 0;
}
