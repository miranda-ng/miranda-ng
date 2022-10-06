/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"
#include "version.h"

#ifdef _DEBUG
#pragma comment(lib, "libprotobufd.lib")
#else
#pragma comment(lib, "libprotobuf.lib")
#endif

CMPlugin g_plugin;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	UNICODE_AWARE, //not transient
	// {008B9CE1-154B-44E4-9823-97C1AAB00C3C}
	{ 0x8b9ce1, 0x154b, 0x44e4, { 0x98, 0x23, 0x97, 0xc1, 0xaa, 0xb0, 0xc, 0x3c }}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<WhatsAppProto>(MODULENAME, pluginInfo)
{
	SetUniqueId(DBKEY_JID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

int CMPlugin::Load()
{
	// InitIcons();
	// InitContactMenus();

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = "WhatsApp";
	nlu.szDescriptiveName.w = TranslateT("WhatsApp (HTTP)");
	hAvatarUser = Netlib_RegisterUser(&nlu);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

int CMPlugin::Unload()
{
	Netlib_CloseHandle(hAvatarConn);
	Netlib_CloseHandle(hAvatarUser);
	return 0;
}
