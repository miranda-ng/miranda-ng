//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speak.h"
#include "m_speak.h"

#include "resource.h"
#include "config/speak_config.h"
#include "announce/speak_announce.h"
#include "announce/dialog_announce.h"
#include "config/dialog_config_active.h"
#include "config/dialog_config_engine.h"

#include <general/debug/debug.h>
#include <general/debug/debug_window.h>
#include <general/debug/debug_file.h>
#include <general/optimise/aggressive_optimise.h>

//-----------------------------------------------------------------------------
PLUGINLINK    *pluginLink = 0;
HINSTANCE      g_hInst;

SpeakConfig   *g_speak_config = 0;
SpeakAnnounce *g_speak_announce = 0;
HANDLE         g_dialog_options_initialise;
HANDLE         g_event_status_change;
HANDLE         g_event_message_added;
HANDLE         g_protocol_ack;

PLUGININFO g_pluginInfo = 
{
	sizeof(PLUGININFO),
	"Speak",
	PLUGIN_MAKE_VERSION(0,9,7,0),
	"Miranda interface to the Microsoft Speech API",
	"Ryan Winter",
	"ryanwinter@hotmail.com",
	"© 2002 Ryan Winter",
	"",
	0,
	0
};

#ifdef _DEBUG
    DebugWindow g_debug_window;
//    DebugFile g_debug_file;
#endif


//-----------------------------------------------------------------------------
// Description : External hook
//-----------------------------------------------------------------------------
int say(WPARAM wParam, LPARAM lParam)
{
	return g_speak_config->say(
        reinterpret_cast<char *>(lParam), 
        reinterpret_cast<HANDLE>(wParam));
}

//-----------------------------------------------------------------------------
// Description : an status change event occured
//-----------------------------------------------------------------------------
int eventStatusChange(WPARAM wParam, LPARAM lParam)
{
	g_speak_announce->statusChange(
		reinterpret_cast<DBCONTACTWRITESETTING *>(lParam),
		reinterpret_cast<HANDLE>(wParam));

	return 0;
}

//-----------------------------------------------------------------------------
// Description : a message event occured
//-----------------------------------------------------------------------------
int eventMessageAdded(WPARAM wParam, LPARAM lParam)
{
	g_speak_announce->incomingEvent(
		reinterpret_cast<HANDLE>(wParam),
		reinterpret_cast<HANDLE>(lParam));

	return 0;
}

//-----------------------------------------------------------------------------
// Description : a messaging protocol changed state
//-----------------------------------------------------------------------------
int protocolAck(WPARAM wParam, LPARAM lParam)
{
    g_speak_announce->protocolAck(reinterpret_cast<ACKDATA *>(lParam));
    
    return 0;
}

//-----------------------------------------------------------------------------
// Description : request for dialog box
//-----------------------------------------------------------------------------
int dialogOptionsInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));

    odp.cbSize = sizeof(odp);
    odp.position = 100000000;
    odp.hInstance = g_hInst;
    odp.pszGroup = Translate("Speak");

    if (g_speak_config)
    {
	    odp.pszTemplate = MAKEINTRESOURCE(IDD_CONFIG);
	    odp.pszTitle = Translate("Engine/Voice");
	    odp.pfnDlgProc = DialogConfigEngine::process;
	    CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	    odp.pszTemplate = MAKEINTRESOURCE(IDD_ACTIVEMODES);
	    odp.pszTitle = Translate("Active Modes");
	    odp.pfnDlgProc = DialogConfigActive::process;
	    CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
    }

    if (g_speak_announce)
    {
	    odp.pszTemplate = MAKEINTRESOURCE(IDD_ANNOUNCE);
	    odp.pszTitle = Translate("Announce");
	    odp.pfnDlgProc = AnnounceDialog::process;
	    CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
    }

	return 0;
}

//-----------------------------------------------------------------------------
// Description : Return info about the plugin
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) PLUGININFO * 
MirandaPluginInfo(DWORD mirandaVersion)
{
	return &g_pluginInfo;
}

//-----------------------------------------------------------------------------
// Description : Construct the plugin
//-----------------------------------------------------------------------------
extern "C" int __declspec(dllexport) 
Load(PLUGINLINK *link)
{
  	pluginLink = link;

	if (!g_speak_config)
	{
		g_speak_config = new SpeakConfig(g_hInst);

		// expose to allow miranda + plugins to access my speak routines
		CreateServiceFunction("Speak/Say", say);
	}

    if (!g_speak_announce)
    {
        g_speak_announce = new SpeakAnnounce(g_hInst);

		// tap into contact setting change event
		g_event_status_change 
			= HookEvent(ME_DB_CONTACT_SETTINGCHANGED, eventStatusChange);

		// a new message event has occured
		g_event_message_added = HookEvent(ME_DB_EVENT_ADDED, eventMessageAdded);

        // a messaging protocol changed state
    	g_protocol_ack = HookEvent(ME_PROTO_ACK, protocolAck);

    }

	// a option dialog box request has occured
	g_dialog_options_initialise 
		= HookEvent(ME_OPT_INITIALISE, dialogOptionsInitialise);

	return 0; 
}

//-----------------------------------------------------------------------------
// Description : Destruct the plugin
//-----------------------------------------------------------------------------
extern "C" int __declspec(dllexport) 
Unload(void)
{
	UnhookEvent(g_dialog_options_initialise);

	if (g_speak_config)
	{
		delete g_speak_config;
        g_speak_config = 0;
	}

    if (g_speak_announce)
    {
		UnhookEvent(g_event_status_change);
		UnhookEvent(g_event_message_added);
        UnhookEvent(g_protocol_ack);

        delete g_speak_announce;
        g_speak_announce = 0;
    }

	return 0;
}

//-----------------------------------------------------------------------------
// Description : Main line
//-----------------------------------------------------------------------------
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID)
{
    CERR("================================================================");

	DisableThreadLibraryCalls(hinstDLL);
	g_hInst = hinstDLL;

    // dispatch application messages    
/*    MSG msg;

    while(::GetMessage(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }*/

	return TRUE;
}

//==============================================================================
//
//  Summary     : DLL interface
//
//  Description : Define the interface to the miranda program
//
//==============================================================================
