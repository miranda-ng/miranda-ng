#include "stdafx.h"
#include "version.h"

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
	// {4f1ff7fa-4d75-44b9-93b0-2ced2e4f9e3e}
	{ 0x4f1ff7fa, 0x4d75, 0x44b9, { 0x93, 0xb0, 0x2c, 0xed, 0x2e, 0x4f, 0x9e, 0x3e } }

};

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<WhatsAppProto>(MODULENAME, pluginInfo)
{
	SetUniqueId(WHATSAPP_KEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

int CMPlugin::Load()
{
	// InitIcons();
	// InitContactMenus();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

int CMPlugin::Unload()
{
	return 0;
}
