/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

// TODO: Make following as "globals" structure?
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
LIST_INTERFACE li;
CLIST_INTERFACE* pcli;
UTF8_INTERFACE utfi;
MD5_INTERFACE md5i;
int hLangpack;

HINSTANCE g_hInstance;
std::string g_strUserAgent;
DWORD g_mirandaVersion;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
  #ifdef _WIN64
	  "Facebook Protocol RM x64",
  #else
	  "Facebook Protocol RM",
  #endif
	__VERSION_DWORD,
	"Provides basic support for Facebook Chat protocol. [Built: "__DATE__" "__TIME__"]",
	"Michal Zelinka, Robert Posel",
	"robyer@seznam.cz",
	"(c) 2009-11 Michal Zelinka, 2011-12 Robert Posel",
	"http://code.google.com/p/robyer/",
	UNICODE_AWARE, //not transient
	0,             //doesn't replace anything built-in
  #ifdef _WIN64
	// {8808C20C-5404-48A6-8390-232AAE5E793A}
	{ 0x8808c20c, 0x5404, 0x48a6, { 0x83, 0x90, 0x23, 0x2a, 0xae, 0x5e, 0x79, 0x3a } }
  #else
	// {8432B009-FF32-4727-AAE6-A9035038FD58}
	{ 0x8432b009, 0xff32, 0x4727, { 0xaa, 0xe6, 0xa9, 0x3, 0x50, 0x38, 0xfd, 0x58 } }
  #endif
};

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int compare_protos(const FacebookProto *p1, const FacebookProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<FacebookProto> g_Instances(1, compare_protos);

DWORD WINAPI DllMain(HINSTANCE hInstance,DWORD,LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if(mirandaVersion > PLUGIN_MAKE_VERSION(0,10,0,0) &&
	   mirandaVersion < PLUGIN_MAKE_VERSION(0,10,0,2))
	{
		MessageBox(0,_T("The Facebook protocol plugin cannot be loaded. ")
			_T("It requires Miranda IM 0.10 alpha build #2 or later."),_T("Miranda"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}
	else if(mirandaVersion < PLUGIN_MAKE_VERSION(0,9,43,0))
	{
		MessageBox(0,_T("The Facebook protocol plugin cannot be loaded. ")
			_T("It requires Miranda IM 0.9.43 or later."),_T("Miranda"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}

	g_mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char *proto_name,const TCHAR *username )
{
	FacebookProto *proto = new FacebookProto(proto_name,username);
	g_Instances.insert(proto);
	return proto;
}

static int protoUninit(PROTO_INTERFACE* proto)
{
	g_Instances.remove(( FacebookProto* )proto);
	return EXIT_SUCCESS;
}

int OnModulesLoaded(WPARAM,LPARAM)
{
	if ( ServiceExists( MS_UPDATE_REGISTER ) )
	{
		Update upd = {sizeof(upd)};
		char curr_version[30];

		upd.szComponentName = pluginInfo.shortName;
		upd.szUpdateURL = UPDATER_AUTOREGISTER;
		upd.szBetaVersionURL     = "http://robyer.info/miranda/facebookRM/version.html";
		upd.szBetaChangelogURL   = "http://robyer.info/miranda/facebookRM/changelog.html";
		upd.pbBetaVersionPrefix  = reinterpret_cast<BYTE*>("Facebook RM ");
		upd.cpbBetaVersionPrefix = (int)strlen(reinterpret_cast<char*>(upd.pbBetaVersionPrefix));
		#ifdef _WIN64
			upd.szBetaUpdateURL      = "http://robyer.info/stahni/facebookRM_x64.zip";    
		#else
			upd.szBetaUpdateURL      = "http://robyer.info/stahni/facebookRM.zip";  
		#endif    
		upd.pbVersion = reinterpret_cast<BYTE*>(CreateVersionStringPluginEx(&pluginInfo,curr_version) );
		upd.cpbVersion = (int)strlen(reinterpret_cast<char*>(upd.pbVersion));
		CallService(MS_UPDATE_REGISTER,0,(LPARAM)&upd);
	}

	return 0;
}

static HANDLE g_hEvents[1];

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getUTFI(&utfi);
	mir_getMD5I(&md5i);
	mir_getLP(&pluginInfo);

	pcli = reinterpret_cast<CLIST_INTERFACE*>( CallService(
	    MS_CLIST_RETRIEVE_INTERFACE,0,reinterpret_cast<LPARAM>(g_hInstance)) );

	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = "Facebook";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE,0,reinterpret_cast<LPARAM>(&pd));

	g_hEvents[0] = HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);

	InitIcons();
	InitContactMenus();

	// Init native User-Agent
	{
		std::stringstream agent;
//		DWORD mir_ver = ( DWORD )CallService( MS_SYSTEM_GETVERSION, NULL, NULL );
		agent << "MirandaIM/";
		agent << (( g_mirandaVersion >> 24) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion >> 16) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion >>  8) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion      ) & 0xFF);
	#ifdef _WIN64
		agent << " Facebook Protocol RM x64/";
	#else
		agent << " Facebook Protocol RM/";
	#endif
		agent << __VERSION_STRING;
		g_strUserAgent = agent.str( );
	}

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	UninitContactMenus();
	for(size_t i=1; i<SIZEOF(g_hEvents); i++)
		UnhookEvent(g_hEvents[i]);

	g_Instances.destroy();

	return 0;
}
