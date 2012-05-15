/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "irc.h"
#include "version.h"

PLUGINLINK*    pluginLink;
HINSTANCE      hInst = NULL;
UTF8_INTERFACE utfi;
MM_INTERFACE   mmi;
LIST_INTERFACE li;

int mirVersion;
int hLangpack;

static int CompareServers( const SERVER_INFO* p1, const SERVER_INFO* p2 )
{
	return lstrcmpA( p1->m_name, p2->m_name );
}

OBJLIST<SERVER_INFO> g_servers( 20, CompareServers );

static int sttCompareProtocols(const CIrcProto *p1, const CIrcProto *p2)
{
	return strcmp(p1->m_szModuleName, p2->m_szModuleName);
}

LIST<CIrcProto> g_Instances(1, sttCompareProtocols);

void InitTimers( void );
void UninitTimers( void );

// Information about the plugin
PLUGININFOEX pluginInfo =
{
	sizeof( PLUGININFOEX ),
	__PLUGIN_NAME,
	__VERSION_DWORD,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
    #if defined( _UNICODE )
    {0x92382b4d, 0x5572, 0x48a0, {0xb0, 0xb9, 0x13, 0x36, 0xa6, 0x1, 0xd6, 0x89}} // {92382B4D-5572-48a0-B0B9-1336A601D689}
    #else
    {0xc0ef6b61, 0xb3b0, 0x4fb7, {0xa2, 0x9e, 0x56, 0x32, 0xdd, 0x98, 0xae, 0x1c}} // {C0EF6B61-B3B0-4fb7-A29E-5632DD98AE1C}
    #endif
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if ( mirandaVersion < MIRANDA_VERSION_CORE ) {
		MessageBoxA( NULL, 
			"The IRC protocol plugin cannot be loaded. It requires Miranda IM " MIRANDA_VERSION_CORE_STRING " or later.", 
			"IRC Protocol Plugin", MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}

	mirVersion = mirandaVersion;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////

static CIrcProto* ircProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	CIrcProto* ppro = new CIrcProto( pszProtoName, tszUserName );
	g_Instances.insert( ppro );
	return ppro;
}

static int ircProtoUninit( CIrcProto* ppro )
{
	g_Instances.remove(( CIrcProto* )ppro);
	delete ppro;
	return 0;
}

extern "C" int __declspec(dllexport) Load( PLUGINLINK *link )
{
	pluginLink = link;
	mir_getMMI( &mmi );
	mir_getUTFI( &utfi );
	mir_getLI( &li );
	mir_getLP( &pluginInfo );

	AddIcons();
	InitTimers();
	InitServers();
	InitContactMenus();

	// register protocol
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof( pd );
	pd.szName = "IRC";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = ( pfnInitProto )ircProtoInit;
	pd.fnUninit = ( pfnUninitProto )ircProtoUninit;
	CallService( MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload(void)
{
	UninitContactMenus();
	UninitIcons();
	UninitTimers();

	g_Instances.destroy();
	
	return 0;
}
