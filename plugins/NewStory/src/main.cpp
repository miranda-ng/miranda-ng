////////////////////////////////////////////////////////////////////////
// NewStory -- new history viewer for Miranda IM
// (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
// Visit http://miranda-im.org/ for details on Miranda Instant Messenger
////////////////////////////////////////////////////////////////////////
// File: main.cpp
// Created by: Victor Pavlychko
// Description:
//    Main module. Responsible for startup/cleanup and Miranda bindings
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

CMPlugin g_plugin;

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
	// {1AD11C86-CAAB-4372-A0A4-8B1168D51B9E}
	{ 0x1ad11c86, 0xcaab, 0x4372, { 0xa0, 0xa4, 0x8b, 0x11, 0x68, 0xd5, 0x1b, 0x9e } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIHISTORY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem icons[] =
{
	{ LPGEN("Main icon"),         "main",      ICO_NEWSTORY   },
	{ LPGEN("User info"),         "userinfo",  ICO_USERINFO   },
	{ LPGEN("User menu"),         "usermenu",  ICO_USERMENU   },
	{ LPGEN("Search"),            "search",    ICO_SEARCH     },
	{ LPGEN("Options"),           "options",   ICO_OPTIONS    },
	{ LPGEN("Filter"),            "filter",    ICO_FILTER     },
	{ LPGEN("Export"),            "export",    ICO_EXPORT     },
	{ LPGEN("Copy"),              "copy",      ICO_COPY       },
	{ LPGEN("Send message"),      "message",   ICO_SENDMSG    },

	{ LPGEN("Incoming message"),  "msgin",     ICO_MSGIN      },
	{ LPGEN("Outgoing message"),  "msgout",    ICO_MSGOUT     },
	{ LPGEN("User signed in"),    "signin",    ICO_SIGNIN     },
	{ LPGEN("File"),              "file",      ICO_FILE       },
	{ LPGEN("URL"),               "url",       ICO_URL        },
	{ LPGEN("Unknown event"),     "unknown",   ICO_UNKNOWN    },

	{ LPGEN("Find previous"),     "findprev",  ICO_FINDPREV   },
	{ LPGEN("Find next"),         "findnext",  ICO_FINDNEXT   },
	{ LPGEN("Jump to date"),      "calendar",  ICO_CALENDAR   },

	{ LPGEN("Template group"),    "tplgroup",  ICO_TPLGROUP   },
	{ LPGEN("Reset"),             "reset",     ICO_RESET      },
	{ LPGEN("Update preview"),    "preview",   ICO_PREVIEW    },
	{ LPGEN("Help"),              "varhelp",   ICO_VARHELP    }
};

static int evtModulesLoaded(WPARAM, LPARAM)
{
	InitFonts();
	InitNewstoryControl();
	InitHistory();

	LoadTemplates();

	CMenuItem mi(&g_plugin);

	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	mi.name.a = LPGEN("User history");
	mi.position = 1999990000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	Menu_AddContactMenuItem(&mi);

	mi.pszService = "Newstory/System";
	mi.name.a = LPGEN("System history");
	mi.position = 1999990000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	Menu_AddMainMenuItem(&mi);
	return 0;
}

static int evtPreShutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(hNewstoryWindows, WM_CLOSE, 0, 0);
	return 0;
}

int CMPlugin::Load()
{
	g_plugin.registerIcon(MODULETITLE, icons);

	m_log = RegisterSrmmLog(MODULETITLE, _T(MODULENAME), NewStory_Stub);

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, svcShowNewstory);
	CreateServiceFunction("Newstory/System", svcShowSystemNewstory);

	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_MODULESLOADED, evtModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, evtPreShutdown);
	return 0;
}

int CMPlugin::Unload()
{
	UnregisterSrmmLog(m_log);
	UnregisterClass(_T(NEWSTORYLIST_CLASS), g_plugin.getInst());
	DestroyFonts();

	return 0;
}