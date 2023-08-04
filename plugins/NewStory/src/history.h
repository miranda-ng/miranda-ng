#ifndef __history_h__
#define __history_h__

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

	UM_SELECTED,

	UM_GETEVENTCOUNT,
	UM_GETEVENT,
	UM_GETEVENTTEXT,
	UM_GETEVENTCONTACT,
	UM_GETEVENTHANDLE
};

extern MWindowList g_hNewstoryWindows, g_hNewstoryLogs;

void InitMenus();

INT_PTR svcShowNewstory(WPARAM, LPARAM);
INT_PTR svcGlobalSearch(WPARAM, LPARAM);

CSrmmLogWindow* __cdecl NewStory_Stub(CMsgDialog &pDlg);

#endif // __history_h__