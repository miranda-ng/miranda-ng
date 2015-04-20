/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

int hLangpack;
XML_API  xi;
TIME_API tmi = { 0 };
HINSTANCE g_hInstance;
CLIST_INTERFACE *pcli;

PLUGININFOEX pluginInfo =
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
	// {57E90AC6-1067-423B-8CA3-70A39D200D4F}
	{ 0x57e90ac6, 0x1067, 0x423b, {0x8c, 0xa3, 0x70, 0xa3, 0x9d, 0x20, 0xd, 0x4f}}
};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);
	mir_getXI(&xi);
	mir_getCLI();

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitAccount;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitAccount;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CSkypeProto::InitIcons();
	CSkypeProto::InitMenus();
	CSkypeProto::InitLanguages();

	HookEvent(ME_SYSTEM_MODULESLOADED, &CSkypeProto::OnModulesLoaded);


	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	CSkypeProto::UninitIcons();
	CSkypeProto::UninitMenus();

	return 0;
}


int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM) 
{ 
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
	{
		CreateServiceFunction(SKYPE_PARSEURI, ParseSkypeURI);
		AssocMgr_AddNewUrlTypeT("skype:", TranslateT("Skype Link Protocol"), g_hInstance, IDI_SKYPE, SKYPE_PARSEURI, 0);
	}
	return 0; 
}