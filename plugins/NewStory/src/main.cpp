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

HANDLE hhkModulesLoaded = 0, hhkOptInitialise = 0, hhkTTBLoaded = 0;

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
	{ LPGEN("Main Icon"),         "main",      ICO_NEWSTORY   },
	{ LPGEN("User Info"),         "userinfo",  ICO_USERINFO   },
	{ LPGEN("User Menu"),         "usermenu",  ICO_USERMENU   },
	{ LPGEN("Search"),            "search",    ICO_SEARCH     },
	{ LPGEN("Options"),           "options",   ICO_OPTIONS    },
	{ LPGEN("Filter"),            "filter",    ICO_FILTER     },
	{ LPGEN("Export"),            "export",    ICO_EXPORT     },
	{ LPGEN("Copy"),              "copy",      ICO_COPY       },
	{ LPGEN("Send Message"),      "message",   ICO_SENDMSG    },
	{ LPGEN("Close"),             "close",     ICO_CLOSE      },

	{ LPGEN("Incoming Message"),  "msgin",     ICO_MSGIN      },
	{ LPGEN("Outgoing Message"),  "msgout",    ICO_MSGOUT     },
	{ LPGEN("User Signed In"),    "signin",    ICO_SIGNIN     },
	{ LPGEN("File"),              "file",      ICO_FILE       },
	{ LPGEN("URL"),               "url",       ICO_URL        },
	{ LPGEN("Unknown Event"),     "unknown",   ICO_UNKNOWN    },

	{ LPGEN("Find Previous"),     "findprev",  ICO_FINDPREV   },
	{ LPGEN("Find Next"),         "findnext",  ICO_FINDNEXT   },
	{ LPGEN("Password disabled"), "nopassword",ICO_NOPASSWORD },
	{ LPGEN("Password enabled"),  "password",  ICO_PASSWORD   },
	{ LPGEN("Jump to Date"),      "calendar",  ICO_CALENDAR   },

	{ LPGEN("Template Group"),    "tplgroup",  ICO_TPLGROUP   },
	{ LPGEN("Reset"),             "reset",     ICO_RESET      },
	{ LPGEN("Update Preview"),    "preview",   ICO_PREVIEW    },
	{ LPGEN("Help"),              "varhelp",   ICO_VARHELP    },
	{ LPGEN("Save Password"),     "savepass",  ICO_SAVEPASS   }
};

int evtModulesLoaded(WPARAM, LPARAM)
{
	InitFonts();
	InitNewstoryControl();
	InitHistory();
	InitOptions();
	LoadTemplates();

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	mi.name.w = L"User Newstory";
	mi.position = 1999990000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	Menu_AddContactMenuItem(&mi);

	mi.pszService = "Newstory/System";
	mi.name.w = L"System Newstory";
	mi.position = 1999990000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	Menu_AddMainMenuItem(&mi);
	return 0;
}

int CMPlugin::Load()
{
	g_plugin.registerIcon(MODULETITLE, icons);

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, svcShowNewstory);
	CreateServiceFunction("Newstory/System", svcShowSystemNewstory);

	hhkModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, evtModulesLoaded);
	return 0;
}

int CMPlugin::Unload()
{
	UnhookEvent(hhkModulesLoaded);
	FreeHistory();
	return 0;
}
