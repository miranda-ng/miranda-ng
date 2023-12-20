#ifndef __history_h__
#define __history_h__

#define EVENTTYPE_OTHER 12345
#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_JABBER_PRESENCE 2001

enum
{
	UM_LOADCONTACT = WM_USER + 1,

	UM_REBUILDLIST,
	UM_FILTERLIST,
	UM_REDRAWLIST,
	UM_REDRAWLISTH,

	UM_ADDEVENT,
	UM_ADDEVENTFILTER,
	UM_REMOVEEVENT,
	UM_EDITEVENT,
	UM_MARKREAD,

	UM_SELECTED,

	UM_GETEVENTCOUNT,
	UM_GETEVENT,
	UM_GETEVENTTEXT,
	UM_GETEVENTCONTACT,
	UM_GETEVENTHANDLE,

	UM_BOOKMARKS = WM_USER + 0x601,
};

extern MWindowList g_hNewstoryWindows, g_hNewstoryLogs;

void InitMenus();
void InitHotkeys();

INT_PTR svcShowNewstory(WPARAM, LPARAM);
INT_PTR svcGlobalSearch(WPARAM, LPARAM);

CSrmmLogWindow* __cdecl NewStory_Stub(CMsgDialog &pDlg);

#endif // __history_h__