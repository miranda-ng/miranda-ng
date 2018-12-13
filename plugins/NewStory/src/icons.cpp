#include "stdafx.h"

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

int evtIconsChanged(WPARAM, LPARAM)
{
	//RefreshIcons(icons, ICO_COUNT);
	//WindowList_Broadcast(hNewstoryWindows, UM_UPDATEICONS, 0, 0);
	return 0;
}

void InitIcons()
{
	g_plugin.registerIcon(MODULETITLE, icons);
	HookEvent(ME_SKIN2_ICONSCHANGED, evtIconsChanged);
}

/*void RefreshIcons(IconItem *icons, int count)
{
	//if (ServiceExists(MS_SKIN2_ADDICON))
	//	for (int i = 0; i < count; i++)
	//		icons[i].hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)icons[i].szId);
}*/

HICON GetIcon(int iconId)
{
	for (auto &it : icons)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib);
	return nullptr;
}
