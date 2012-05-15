
#include "commonheaders.h"
#include "resource.h"

#if defined( _UNICODE )
	// {4AECBC63-49B9-40da-9212-FF5EBC85360E}
	#define MIID_XML { 0x4aecbc63, 0x49b9, 0x40da, { 0x92, 0x12, 0xff, 0x5e, 0xbc, 0x85, 0x36, 0xe }}
#else
	// {6E2C9F7C-5F3F-4dd3-A999-ED64B70BFC79}
	#define MIID_XML { 0x6e2c9f7c, 0x5f3f, 0x4dd3, { 0xa9, 0x99, 0xed, 0x64, 0xb7, 0xb, 0xfc, 0x79 }}
#endif

PLUGINLINK *pluginLink;
HINSTANCE hInst;

void InitXmlApi( void );

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"Core XML driver for Miranda version earlier than 0.8",
	PLUGIN_MAKE_VERSION(0,1,0,0),
	"Provides XML parsing & creation services",
	"George Hazan",
	"ghazan@miranda.im",
	"é 2008 George Hazan",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	0
};

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	"Core XML driver for Miranda version earlier than 0.8",
	PLUGIN_MAKE_VERSION(0,1,0,0),
	"Provides XML parsing & creation services",
	"George Hazan",
	"ghazan@miranda.im",
	"é 2008 George Hazan",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	0,
	MIID_XML
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns an information about a plugin

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion >= PLUGIN_MAKE_VERSION(0, 8, 0, 0))
		return NULL;

	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion >= PLUGIN_MAKE_VERSION(0, 8, 0, 0))
		return NULL;

	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInterfaces - returns the protocol interface to the core

static const MUUID interfaces[] = { MIID_XML, MIID_LAST };

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Performs a primary set of actions upon plugin loading

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	InitXmlApi();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload a plugin

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
