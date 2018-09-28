#include "headers.h"

MyIconDesriptor icons[] = 
{
	{ICO_NEWSTORY,  MODULENAME"/main",      MODULETITLE, "Main Icon", 0},
	{ICO_USERINFO,  MODULENAME"/userinfo",  MODULETITLE, "User Info", 0},
	{ICO_USERMENU,  MODULENAME"/usermenu",  MODULETITLE, "User Menu", 0},
	{ICO_SEARCH,    MODULENAME"/search",    MODULETITLE, "Search", 0},
	{ICO_OPTIONS,   MODULENAME"/options",   MODULETITLE, "Options", 0},
	{ICO_FILTER,    MODULENAME"/filter",    MODULETITLE, "Filter", 0},
	{ICO_EXPORT,    MODULENAME"/export",    MODULETITLE, "Export", 0},
	{ICO_COPY,      MODULENAME"/copy",      MODULETITLE, "Copy", 0},
	{ICO_SENDMSG,   MODULENAME"/message",   MODULETITLE, "Send Message", 0},
	{ICO_CLOSE,     MODULENAME"/close",     MODULETITLE, "Close", 0},

	{ICO_MSGIN,     MODULENAME"/msgin",     MODULETITLE, "Incoming Message", 0},
	{ICO_MSGOUT,    MODULENAME"/msgout",    MODULETITLE, "Outgoing Message", 0},
	{ICO_SIGNIN,    MODULENAME"/signin",    MODULETITLE, "User Signed In", 0},
	{ICO_FILE,      MODULENAME"/file",      MODULETITLE, "File", 0},
	{ICO_URL,       MODULENAME"/url",       MODULETITLE, "URL", 0},
	{ICO_UNKNOWN,   MODULENAME"/unknown",   MODULETITLE, "Unknown Event", 0},
	
	{ICO_FINDPREV,  MODULENAME"/findprev",  MODULETITLE, "Find Previous", 0},
	{ICO_FINDNEXT,  MODULENAME"/findnext",  MODULETITLE, "Find Next", 0},
	{ICO_NOPASSWORD,MODULENAME"/nopassword",MODULETITLE, "Password disabled", 0},
	{ICO_PASSWORD,  MODULENAME"/password",  MODULETITLE, "Password enabled", 0},
	{ICO_CALENDAR,  MODULENAME"/calendar",  MODULETITLE, "Jump to Date", 0},

	{ICO_TPLGROUP,  MODULENAME"/tplgroup",  MODULETITLE, "Template Group", 0},
	{ICO_RESET,     MODULENAME"/reset",     MODULETITLE, "Reset", 0},
	{ICO_PREVIEW,   MODULENAME"/preview",   MODULETITLE, "Update Preview", 0},
	{ICO_VARHELP,   MODULENAME"/varhelp",   MODULETITLE, "Help", 0},
	{ICO_SAVEPASS,  MODULENAME"/savepass",  MODULETITLE, "Save Password", 0}
};

int iconCount = sizeof(icons) / sizeof(icons[0]);

int evtIconsChanged(WPARAM, LPARAM)
{
	RefreshIcons(icons, ICO_COUNT);
	WindowList_Broadcast(hNewstoryWindows, UM_UPDATEICONS, 0, 0);
	return 0;
}

void InitIcons(MyIconDesriptor *icons, LPSTR szFilename, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (ServiceExists(MS_SKIN2_ADDICON))
		{
			SKINICONDESC sid;
			sid.cbSize = sizeof(sid);
			sid.pszName = icons[i].szId;
			sid.pszSection = icons[i].szGroup;
			sid.pszDescription = icons[i].szTitle;
			sid.pszDefaultFile = szFilename;
			sid.iDefaultIndex = -icons[i].dwIndex-1;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			icons[i].hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)icons[i].szId);
		} else
		{
			ExtractIconExA(szFilename, icons[i].dwIndex, 0, &icons[i].hIcon, 1);
		}
	}

	HookEvent(ME_SKIN2_ICONSCHANGED, evtIconsChanged);
}

void RefreshIcons(MyIconDesriptor *icons, int count)
{
	if (ServiceExists(MS_SKIN2_ADDICON))
		for (int i = 0; i < count; i++)
			icons[i].hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)icons[i].szId);
}