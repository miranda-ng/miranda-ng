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
HINSTANCE   hInstance;
PLUGINLINK* pluginLink;

HANDLE g_hNetlibUser;

MM_INTERFACE   mmi;
UTF8_INTERFACE utfi;
MD5_INTERFACE  md5i;
LIST_INTERFACE li;
int hLangpack;

PLUGININFOEX pluginInfo={
		sizeof(PLUGININFOEX),
#ifdef YAHOO_CVSBUILD
		"Yahoo Protocol Beta/Nightly",
#else
		"Yahoo Protocol",
#endif
		__VERSION_DWORD,
		"Yahoo Protocol support via libyahoo2 library. [Built: "__DATE__" "__TIME__"]",
		"Gennady Feldman",
		"gena01@miranda-im.org",
		"© 2003-2010 Gennady Feldman, Laurent Marechal",
		"http://www.miranda-im.org",
		UNICODE_AWARE, //not transient
		0, //DEFMOD_PROTOCOLYAHOO - no core yahoo protocol
        #if defined( _UNICODE )
        {0xaa7bfea, 0x1fc7, 0x45f0, {0x90, 0x6e, 0x2a, 0x46, 0xb6, 0xe1, 0x19, 0xcf}} // {0AA7BFEA-1FC7-45f0-906E-2A46B6E119CF}
        #else
        {0x6151b6e6, 0x3a7b, 0x45e6, {0x9a, 0x51, 0x18, 0x72, 0xc1, 0x7b, 0x5, 0xa9}} // {6151B6E6-3A7B-45e6-9A51-1872C17B05A9}
        #endif
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
// Parameters: PLUGINLINK *link
// Returns : int
// Description : Called when plugin is loaded into Miranda
//=====================================================

static int CompareProtos( const CYahooProto* p1, const CYahooProto* p2 )
{
	return lstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CYahooProto> g_instances( 1, CompareProtos );

static CYahooProto* yahooProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	CYahooProto* ppro = new CYahooProto( pszProtoName, tszUserName );
	
	g_instances.insert( ppro );
	
	return ppro;
}

static int yahooProtoUninit( CYahooProto* ppro )
{
	g_instances.remove( ppro );
	delete ppro;
	
	return 0;
}

extern "C" int __declspec(dllexport)Load(PLUGINLINK *link)
{
 	pluginLink = link;
	
	/**
	 * Grab the interface handles (through pluginLink)
	 */
	int i = mir_getLI( &li );
		
	if (i) {
		MessageBox(NULL, _T("Can not retrieve the core List Interface."), _T("Yahoo Plugin Load Failed"), MB_ICONERROR | MB_OK);
		return 1;
	}
	
	mir_getMMI( &mmi );
	mir_getUTFI( &utfi );
	mir_getMD5I( &md5i );
	mir_getLP( &pluginInfo );
	
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = "YAHOO";
	pd.type   = PROTOTYPE_PROTOCOL;
	pd.fnInit = ( pfnInitProto )yahooProtoInit;
	pd.fnUninit = ( pfnUninitProto )yahooProtoUninit;
	CallService( MS_PROTO_REGISTERMODULE, 0, ( LPARAM )&pd );

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
  	nlu.flags = NUF_TCHAR | NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = "YAHOO/libyahoo2";
	nlu.ptszDescriptiveName = TranslateT("YAHOO plugin HTTP connections");
	g_hNetlibUser = ( HANDLE )YAHOO_CallService( MS_NETLIB_REGISTERUSER, 0, ( LPARAM )&nlu );

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
	LOG(( "Unload" ));
	
	YmsgrLinksUninit();
	Netlib_CloseHandle( g_hNetlibUser );
	g_instances.destroy();
	return 0;
}

/*
 * MirandaPluginInfoEx - Sets plugin info
 * Parameters: (DWORD mirandaVersion)
 */
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	/*
	 * This requires the latest trunk... [md5, sha, etc..]
	 */
    if (mirandaVersion < MIRANDA_VERSION_CORE ) {
		MessageBoxA( NULL, 
				"Yahoo plugin cannot be loaded. It requires Miranda IM " MIRANDA_VERSION_CORE_STRING " or later.",
				"Yahoo",
				MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );

        return NULL;
	}

    return &pluginInfo;
}

/*
 * MirandaPluginInterfaces - Notifies the core of interfaces implemented
 * Parameters: none
 */
static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}
