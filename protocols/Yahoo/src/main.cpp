/*
 * $Id: main.cpp 13596 2011-04-15 19:07:23Z george.hazan $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#include "yahoo.h"
#include "http_gateway.h"
#include "version.h"
#include "resource.h"

#include <m_langpack.h>

/*
 * Global Variables
 */
HINSTANCE hInstance;

HANDLE g_hNetlibUser;

CLIST_INTERFACE *pcli;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE, //not transient
	{0xaa7bfea, 0x1fc7, 0x45f0, {0x90, 0x6e, 0x2a, 0x46, 0xb6, 0xe1, 0x19, 0xcf}} // {0AA7BFEA-1FC7-45f0-906E-2A46B6E119CF}
};

void YmsgrLinksInit(void);
void YmsgrLinksUninit(void);

/*
 * WINAPI DllMain - main entry point into a DLL
 * Parameters: 
 *          HINSTANCE hinst,
 *          DWORD fdwReason,
 *          LPVOID lpvReserved
 * Returns : 
 *           BOOL
 * 
 */
extern "C" BOOL WINAPI DllMain(HINSTANCE hinst,DWORD /*fdwReason*/,LPVOID /*lpvReserved*/)
{
	hInstance = hinst;
	return TRUE;
}

/*
 *	Load - loads plugin into memory
 */
 
//=====================================================
// Name : Load
// Parameters: 
// Returns : int
// Description : Called when plugin is loaded into Miranda
//=====================================================

static int CompareProtos( const CYahooProto* p1, const CYahooProto* p2 )
{
	return mir_tstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CYahooProto> g_instances( 1, CompareProtos );

static CYahooProto* yahooProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	CYahooProto* ppro = new CYahooProto(pszProtoName, tszUserName);
	
	g_instances.insert( ppro );
	
	return ppro;
}

static int yahooProtoUninit( CYahooProto* ppro )
{
	g_instances.remove( ppro );
	delete ppro;
	
	return 0;
}

extern "C" int __declspec(dllexport)Load(void)
{
	mir_getLP( &pluginInfo );
	mir_getCLI();
	
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "YAHOO";
	pd.type   = PROTOTYPE_PROTOCOL;
	pd.fnInit = ( pfnInitProto )yahooProtoInit;
	pd.fnUninit = ( pfnUninitProto )yahooProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_TCHAR | NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = "YAHOO/libyahoo2";
	nlu.ptszDescriptiveName = TranslateT("YAHOO plugin HTTP connections");
	g_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	YmsgrLinksInit();
	/**
	 * Register LibYahoo2 callback functions
	 */
	register_callbacks();
	
	return 0;
}

/*
 * Unload - Unloads plugin
 * Parameters: void
 */

extern "C" int __declspec(dllexport) Unload(void)
{
	LOG(("Unload"));
	
	YmsgrLinksUninit();
	Netlib_CloseHandle( g_hNetlibUser );
	return 0;
}

/*
 * MirandaPluginInfoEx - Sets plugin info
 * Parameters: (DWORD mirandaVersion)
 */
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/*
 * MirandaInterfaces - Notifies the core of interfaces implemented
 * Parameters: none
 */
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};
