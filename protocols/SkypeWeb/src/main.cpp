/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

CMPlugin g_plugin;

char g_szMirVer[100];
HANDLE g_hCallEvent;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {57E90AC6-1067-423B-8CA3-70A39D200D4F}
	{ 0x57e90ac6, 0x1067, 0x423b, { 0x8c, 0xa3, 0x70, 0xa3, 0x9d, 0x20, 0xd, 0x4f } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CSkypeProto>("SKYPE", pluginInfoEx),
	szDefaultServer("azeus1-client-s.gateway.messenger.live.com")
{
	SetUniqueId(SKYPE_SETTINGS_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction(MODULE "/ParseUri", CSkypeProto::GlobalParseSkypeUriService);
		AssocMgr_AddNewUrlTypeW("skype:", TranslateT("Skype Link Protocol"), g_plugin.getInst(), IDI_SKYPE, MODULE "/ParseUri", 0);
	}
	return 0;
}

int CMPlugin::Load()
{
	Miranda_GetVersionText(g_szMirVer, sizeof(g_szMirVer));

	CSkypeProto::InitIcons();
	CSkypeProto::InitMenus();
	CSkypeProto::InitLanguages();

	CreateServiceFunction(MODULE "/GetEventIcon", &CSkypeProto::EventGetIcon);
	CreateServiceFunction(MODULE "/GetEventText", &CSkypeProto::GetEventText);

	g_hCallEvent = CreateHookableEvent(MODULE "/IncomingCall");

	HookEvent(ME_SYSTEM_MODULESLOADED, &CSkypeProto::OnModulesLoaded);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DestroyHookableEvent(g_hCallEvent);
	return 0;
}
