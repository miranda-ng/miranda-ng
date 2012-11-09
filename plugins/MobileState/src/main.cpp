/*
   Mobile State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2012 by Robert Pösel

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "commonheaders.h"

HINSTANCE g_hInst;
static HANDLE hHookModulesLoaded = NULL;
static HANDLE hHookExtraIconsApply = NULL, hContactSettingChanged = NULL;
HANDLE hExtraIcon = NULL;
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
	// {F0BA32D0-CD07-4A9C-926B-5A1FF21C3C10}
	{0xf0ba32d0, 0xcd07, 0x4a9c, { 0x92, 0x6b, 0x5a, 0x1f, 0xf2, 0x1c, 0x3c, 0x10 }}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

bool hasMobileClient(HANDLE hContact, LPARAM lParam)
{
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	bool ret = false;

	DBVARIANT dbv;
	if (!DBGetContactSettingTString(hContact, proto, "MirVer", &dbv))
	{		
		if (_tcsstr(dbv.ptszVal, _T("Android")) || _tcsstr(dbv.ptszVal, _T("android")) || _tcsstr(dbv.ptszVal, _T("Mobile")) || _tcsstr(dbv.ptszVal, _T("mobile")))
			ret = true;

		DBFreeVariant(&dbv);
	}
	
	return ret;
}

int ExtraIconsApply(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
		return 0;

	const char *icon = hasMobileClient((HANDLE) wParam, lParam) ? "mobile_icon" : NULL;	

	ExtraIcon_SetIcon(hExtraIcon, (HANDLE)wParam, icon);
	return 0;
}

int onContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (!proto)
		return 0;

	if (!lstrcmpA(cws->szModule,proto))
		if (!lstrcmpA(cws->szSetting, "MirVer"))
			ExtraIconsApply(wParam, 1);

	return 0;
}

int onModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	// IcoLib support
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;

	sid.ptszSection = _T("Mobile State");
	sid.ptszDefaultFile = szFile;

	sid.ptszDescription = _T("Mobile State");
	sid.pszName = "mobile_icon";
	sid.iDefaultIndex = -IDI_MOBILE;
	Skin_AddIcon(&sid);

	// extra icons
	hExtraIcon = ExtraIcon_Register("mobilestate", "Mobile State", "mobile_icon");

	// Set initial value for all contacts
	HANDLE hContact = db_find_first();
	while (hContact != NULL) {
		ExtraIconsApply((WPARAM)hContact, 1);
		hContact = db_find_next(hContact);
	}

	return 0;
}

int onSystemOKToExit(WPARAM wParam,LPARAM lParam)
{

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	hHookExtraIconsApply = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ExtraIconsApply);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hHookModulesLoaded);
	UnhookEvent(hHookExtraIconsApply);
	UnhookEvent(hContactSettingChanged);

	return 0;
}
