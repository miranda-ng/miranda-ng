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

CMOption<bool> g_bOptGrouping(MODULENAME, "MessageGrouping", false);
CMOption<bool> g_bOptDrawEdge(MODULENAME, "DrawEdge", true);

MWindowList g_hNewstoryWindows = 0, g_hNewstoryLogs = 0;

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
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bOptVScroll(MODULENAME, "VScroll", true),
	bSortAscending(MODULENAME, "SortAscending", true)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIHISTORY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem icons[] =
{
	{ LPGEN("Main icon"),         "main",      IDI_NEWSTORY   },
	{ LPGEN("User info"),         "userinfo",  IDI_USERINFO   },
	{ LPGEN("User menu"),         "usermenu",  IDI_USERMENU   },
	{ LPGEN("Search"),            "search",    IDI_SEARCH     },
	{ LPGEN("Options"),           "options",   IDI_OPTIONS    },
	{ LPGEN("Filter"),            "filter",    IDI_FILTER     },
	{ LPGEN("Export"),            "export",    IDI_EXPORT     },
	{ LPGEN("Copy"),              "copy",      IDI_COPY       },
	{ LPGEN("Bookmark"),          "bookmark",  IDI_BOOKMARK   },
	{ LPGEN("Send message"),      "message",   IDI_SENDMSG    },

	{ LPGEN("Incoming message"),  "msgin",     IDI_MSGIN      },
	{ LPGEN("Outgoing message"),  "msgout",    IDI_MSGOUT     },
	{ LPGEN("User signed in"),    "signin",    IDI_SIGNIN     },
	{ LPGEN("Unknown event"),     "unknown",   IDI_UNKNOWN    },

	{ LPGEN("Find previous"),     "findprev",  IDI_FINDPREV   },
	{ LPGEN("Find next"),         "findnext",  IDI_FINDNEXT   },
	{ LPGEN("Jump to date"),      "calendar",  IDI_CALENDAR   },
	{ LPGEN("Conversations"),     "timetree",  IDI_TIMETREE   },

	{ LPGEN("Template group"),    "tplgroup",  IDI_TPLGROUP   },
	{ LPGEN("Cancel edit"),       "reset",     IDI_RESET      },
	{ LPGEN("Update preview"),    "preview",   IDI_PREVIEW    },
	{ LPGEN("Help"),              "varhelp",   IDI_VARHELP    }
};

static int evtEventAdded(WPARAM hContact, LPARAM lParam)
{
	if (HWND hwnd = WindowList_Find(g_hNewstoryLogs, hContact))
		SendMessage(hwnd, UM_ADDEVENT, hContact, lParam);
	return 0;
}

static int evtEventDeleted(WPARAM hContact, LPARAM lParam)
{
	if (HWND hwnd = WindowList_Find(g_hNewstoryLogs, hContact))
		SendMessage(hwnd, UM_REMOVEEVENT, hContact, lParam);
	return 0;
}

static int evtEventEdited(WPARAM hContact, LPARAM lParam)
{
	if (HWND hwnd = WindowList_Find(g_hNewstoryLogs, hContact))
		SendMessage(hwnd, UM_EDITEVENT, hContact, lParam);
	return 0;
}

static int evtModulesLoaded(WPARAM, LPARAM)
{
	InitFonts();
	InitNewstoryControl();


	LoadTemplates();
	return 0;
}

static int evtPreShutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(g_hNewstoryWindows, WM_CLOSE, 0, 0);
	return 0;
}

int CMPlugin::Load()
{
	registerIcon(MODULETITLE, icons);

	bDrawEdge = g_bOptDrawEdge;
	bMsgGrouping = g_bOptGrouping;

	m_log = RegisterSrmmLog(this, MODULETITLE, _T(MODULENAME), NewStory_Stub);

	g_hNewstoryLogs = WindowList_Create();
	g_hNewstoryWindows = WindowList_Create();

	HookEvent(ME_DB_EVENT_ADDED, evtEventAdded);
	HookEvent(ME_DB_EVENT_DELETED, evtEventDeleted);
	HookEvent(ME_DB_EVENT_EDITED, evtEventEdited);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_MODULESLOADED, evtModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, evtPreShutdown);

	InitMenus();
	InitServices();
	return 0;
}

int CMPlugin::Unload()
{
	WindowList_Destroy(g_hNewstoryLogs);
	WindowList_Destroy(g_hNewstoryWindows);

	UnregisterSrmmLog(m_log);
	UnregisterClass(_T(NEWSTORYLIST_CLASS), g_plugin.getInst());
	DestroyFonts();
	return 0;
}
