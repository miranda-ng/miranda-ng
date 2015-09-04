/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"
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

INT_PTR GetCaps(WPARAM wParam, LPARAM)
{
	if (wParam == PFLAGNUM_2 && opt.ShowCustomIcon)
		return PF2_ONLINE | PF2_LIGHTDND | PF2_SHORTAWAY;

	return 0;
}

INT_PTR GetStatus(WPARAM, LPARAM)
{
	return ID_STATUS_ONLINE;
}

INT_PTR GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, MODULE_NAME, wParam);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
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
	mir_getLP(&pluginInfoEx);

	SkinAddNewSoundEx("Gmail", LPGEN("Other"), LPGEN("Gmail: New thread(s)"));
	HookEvent(ME_CLIST_DOUBLECLICKED, OpenBrowser);

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE_NAME;
	pd.type = PROTOTYPE_VIRTUAL;
	Proto_RegisterModule(&pd);

	CreateProtoServiceFunction(MODULE_NAME, PS_GETCAPS, GetCaps);
	CreateProtoServiceFunction(MODULE_NAME, PS_GETSTATUS, GetStatus);
	CreateProtoServiceFunction(MODULE_NAME, PS_GETNAME, GetName);
	CreateServiceFunction("GmailMNotifier/Notifying", Notifying);

	opt.circleTime = db_get_dw(NULL, MODULE_NAME, "circleTime", 30);
	opt.notifierOnTray = db_get_dw(NULL, MODULE_NAME, "notifierOnTray", TRUE);
	opt.notifierOnPop = db_get_dw(NULL, MODULE_NAME, "notifierOnPop", TRUE);
	opt.popupDuration = db_get_dw(NULL, MODULE_NAME, "popupDuration", -1);
	opt.popupBgColor = db_get_dw(NULL, MODULE_NAME, "popupBgColor", RGB(173, 206, 247));
	opt.popupTxtColor = db_get_dw(NULL, MODULE_NAME, "popupTxtColor", RGB(0, 0, 0));
	opt.OpenUsePrg = db_get_dw(NULL, MODULE_NAME, "OpenUsePrg", 0);
	opt.ShowCustomIcon = db_get_dw(NULL, MODULE_NAME, "ShowCustomIcon", FALSE);
	opt.UseOnline = db_get_dw(NULL, MODULE_NAME, "UseOnline", FALSE);
	opt.AutoLogin = db_get_dw(NULL, MODULE_NAME, "AutoLogin", TRUE);
	opt.LogThreads = db_get_dw(NULL, MODULE_NAME, "LogThreads", FALSE);

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
		db_set_dw(acc[i].hContact, MODULE_NAME, "Status", ID_STATUS_NONEW);

	hTimer = SetTimer(NULL, 0, opt.circleTime * 60000, TimerProc);
	hMirandaStarted = HookEvent(ME_SYSTEM_MODULESLOADED, OnMirandaStart);
	hOptionsInitial = HookEvent(ME_OPT_INITIALISE, OptInit);
	
	CreateServiceFunction(MODULE_NAME "/MenuCommand", PluginMenuCommand);

	CMenuItem mi;
	SET_UID(mi, 0xbe16f37, 0x17be, 0x4494, 0xaa, 0xb2, 0x3a, 0xa7, 0x38, 0xfa, 0xf9, 0xcc);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_LoadProtoIcon(MODULE_NAME, ID_STATUS_ONLINE);
	mi.name.a = LPGEN("&Check all Gmail inboxes");
	mi.pszService = MODULE_NAME "/MenuCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x22c6ace1, 0xba0c, 0x44b5, 0xa4, 0xd2, 0x1, 0x7d, 0xb1, 0xe0, 0x51, 0xeb);
	mi.name.a = LPGEN("&Check Gmail inbox");
	mi.pszService = "/MenuCommand";
	Menu_AddContactMenuItem(&mi, MODULE_NAME);
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
